/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2009 STMicroelectronics
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * CEC driver.
 */
#include "omv_boardconfig.h"

#if OMV_DISPLAY_CEC_ENABLE

#include <stdint.h>
#include "py/obj.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include "omv_gpio.h"
#include "cec.h"

/* Start bit timings */
#define Sbit_Nom_LD               37 /* Start Bit Nominal Low duration: 37 x 100µs = 3.7ms */
#define Sbit_Nom_HD               8 /* Start Bit Nominal High Duration: 8 x 100µs = 0.8ms */
#define Sbit_Min_LD               35 /* Start Bit Minimum Low duration: 35 x 100µs = 3.5ms */
#define Sbit_Max_LD               39 /* Start Bit Maximum Low duration: 39 x 100µs = 3.9ms */
#define Sbit_Min_TD               43 /* Start Bit Minimum Total duration: 43 x 100µs = 4.3ms */
#define Sbit_Max_TD               47 /* Start Bit Maximum Total duration: 47 x 100µs = 4.7ms */

/* Data bit logical "0" timings */
#define Dbit0_Nom_LD              15 /* Data Bit "0" Nominal Low duration: 15 x 100µs = 1.5ms */
#define Dbit0_Nom_HD              9 /* Data Bit "0" Nominal High Duration: 9 x 100µs = 0.9ms */
#define Dbit0_Min_LD              13 /* Data Bit "0" Minimum Low duration: 13 x 100µs = 1.3ms */
#define Dbit0_Max_LD              17 /* Data Bit "0" Maximum Low duration: 17 x 100µs = 1.7ms */

/* Data bit logical "1" timings */
#define Dbit1_Nom_LD              6 /* Data Bit (logical "1") Nominal Low duration: 0.6ms  */
#define Dbit1_Nom_HD              18 /* Data Bit (logical "1") Nominal High Duration: 1.8ms */
#define Dbit1_Min_LD              4 /* Data Bit "1" Minimum Low duration: 0.4ms */
#define Dbit1_Max_LD              8 /* Data Bit "1" Maximum Low duration: 0.8ms */

/* Data bit duration */
#define DbitX_Min_TD              20 /* Data Bit Minimum Total duration: 2ms   */
#define DbitX_Max_TD              27 /* Data Bit Maximum Total duration: 2.7ms */

/* Header or Data block definition */
#define DataBlock                 0
#define HeaderBlock               1

/* Masks */
#define ReceiveFrameStatusMask    0x00010000
#define FrameSendToMeMask         0x00020000
#define InitiatorAddressMask      0x000000FF

static uint8_t cec_byte;
static uint8_t cec_bit;
static uint8_t cec_eom;
static uint32_t cec_counter;
static uint8_t cec_last_byte;

static void CEC_Wait100us(uint32_t nTime) {
    mp_hal_delay_us(nTime * 100);
}

static void CEC_SendStartBit() {
    /* Start Low Period */
    omv_gpio_write(OMV_CEC_PIN, 0);

    /* Wait 3.7ms*/
    CEC_Wait100us(Sbit_Nom_LD);

    /* Start High Period */
    omv_gpio_write(OMV_CEC_PIN, 1);

    /* Wait 0.8ms for a total of 3.7 + 0.8 = 4.5ms */
    CEC_Wait100us(Sbit_Nom_HD);
}

static uint8_t CEC_ReceiveStartBit() {
    /* Initialize cec_counter */
    cec_counter = 0;

    /* Go to high impedance: CEC bus state = VDD */
    omv_gpio_write(OMV_CEC_PIN, 1);

    /* Wait for rising edge of the start bit */
    while (!omv_gpio_read(OMV_CEC_PIN)) {
        /* Wait 100us */
        CEC_Wait100us(1);

        /* Increment cec_counter that contains the duration of low level duration */
        cec_counter++;

        /* If too long low level for start bit */
        if (cec_counter > Sbit_Max_LD) {
            /* Exit: it's an error: 0 */
            return 0;
        }
    }

    /* If too short duration of low level for start bit */
    if (cec_counter < Sbit_Min_LD) {
        /* Exit: it's an error: 0 */
        return 0;
    }

    /* Wait for falling edge of the start bit (end of start bit) */
    while (omv_gpio_read(OMV_CEC_PIN)) {
        /* Wait 100us */
        CEC_Wait100us(1);

        /* Increment cec_counter that contains the duration of high level duration */
        cec_counter++;

        /* If too long total duration for start bit */
        if (cec_counter > Sbit_Max_TD) {
            /* Exit: it's an error: 0 */
            return 0;
        }
    }

    /* If too short total duration for start bit */
    if (cec_counter < Sbit_Min_TD) {
        /* Exit: it's an error: 0 */
        return 0;
    }

    /* Exit: start bit received correctly */
    return 1;
}

static void CEC_SendDataBit(uint8_t bit) {
    /* Start Low Period: the duration depends on the Logical Level "0" or "1" */
    omv_gpio_write(OMV_CEC_PIN, 0);

    /* Wait 0.6 ms if Logical Level is "1" and 1.5ms if Logical Level is "0" */
    CEC_Wait100us(bit ? Dbit1_Nom_LD : Dbit0_Nom_LD);

    /* Start High Period: the duration depends on the Logical Level "0" or "1" */
    omv_gpio_write(OMV_CEC_PIN, 1);

    /* Wait 1.8 ms if Logical Level is "1" and 0.9 ms if Logical Level is "0" */
    CEC_Wait100us(bit ? Dbit1_Nom_HD : Dbit0_Nom_HD);
}

static uint8_t CEC_ReceiveDataBit() {
    uint8_t bit = 0xFF;

    /* Initialize cec_counter */
    cec_counter = 0;

    /* Wait for rising edge of the data bit */
    while (!omv_gpio_read(OMV_CEC_PIN)) {
        /* Wait 100us */
        CEC_Wait100us(1);

        /* Increment cec_counter that contains the duration of low level duration */
        cec_counter++;

        if (cec_counter > Dbit0_Max_LD) {
            /* Exit: it's an error: 0xFF */
            return 0xFF;
        }
    }

    /* If the measured duration of the low level is greater than the minimum low duration
       of "logical 0" i.e > 1.3 ms */
    if (cec_counter > Dbit0_Min_LD) {
        /* The received bit is "logical 0" */
        bit = 0;
    } else {
        /* If the measured duration of the low level is greater than the maximum low duration
           of "logical 1" i.e > 0.8 ms */
        if (cec_counter > Dbit1_Max_LD) {
            /* Exit: it's an error: 0xFF */
            return 0xFF;
        }

        /* If the measured duration of the low level is greater than the minimum low duration
           of "logical 1" i.e > 0.4 ms */
        if (cec_counter > Dbit1_Min_LD) {
            /* The received bit is "logical 1" */
            bit = 1;
        } else {
            /* Exit: it's an error: 0xFF */
            return 0xFF;
        }
    }

    /* Wait for falling edge of the data bit */
    while (omv_gpio_read(OMV_CEC_PIN)) {
        /* Wait 100us */
        CEC_Wait100us(1);

        /* Increment cec_counter that contains the duration of high level duration */
        cec_counter++;

        /* If too long total duration for the data bit is detected */
        if (cec_counter > DbitX_Max_TD) {
            /* Exit: it's an error: 0xFF */
            return 0xFF;
        }
    }

    /* If too short total duration for the data bit is detected	*/
    if (cec_counter < DbitX_Min_TD) {
        /* Exit: it's an error: 0xFF */
        return 0xFF;
    }

    /* The data bit is received correctly. Return its value */
    return bit;
}

static void CEC_SendAckBit() {
    mp_uint_t start = mp_hal_ticks_ms();
    /* Wait for falling edge: end of EOM bit sent by the initiator */
    while (omv_gpio_read(OMV_CEC_PIN)) {
        if ((mp_hal_ticks_ms() - start) > 10) {
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Ack timeout!"));
        }
    }

    /* Send ACK bit */
    CEC_SendDataBit(0);

    /* Force the bus to 0: for ACK bit delimiting */
    omv_gpio_write(OMV_CEC_PIN, 0);

    /* Wait 100us to allow the initiator to detect the end of ACK bit */
    CEC_Wait100us(1);

    /* Go to high impedance: CEC bus state = VDD */
    omv_gpio_write(OMV_CEC_PIN, 1);
}

static uint8_t CEC_ReceiveAckBit() {
    uint8_t AckValue = 0xFF;

    /* Get the ACK bit */
    AckValue = CEC_ReceiveDataBit();

    /* If the byte has been acknowledged by the Follower (ACK = 0) */
    if (AckValue != 0xFF) {
        mp_uint_t start = mp_hal_ticks_ms();
        /* Wait for falling edge of ACK bit (the end of ACK bit)*/
        while (omv_gpio_read(OMV_CEC_PIN)) {
            if ((mp_hal_ticks_ms() - start) > 10) {
                mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Ack timeout!"));
            }
        }
    }

    return AckValue;
}

static bool CEC_SendByte(uint8_t byte) {
    /* Send the data byte: bit by bit (MSB first) */
    for (cec_bit = 0; cec_bit <= 7; cec_bit++) {
        CEC_SendDataBit(byte & 0x80);
        byte <<= 1;
    }

    /* Send EOM bit after sending the data byte */
    CEC_SendDataBit(cec_last_byte);

    /* Force the bus to 0: for EOM bit delimiting */
    omv_gpio_write(OMV_CEC_PIN, 0);

    /* Wait 100us to allow the follower to detect the end of EOM bit */
    CEC_Wait100us(1);

    /* Go to high impedance: CEC bus state = VDD */
    omv_gpio_write(OMV_CEC_PIN, 1);

    /* If the byte is acknowledged by the receiver */
    if (CEC_ReceiveAckBit() == 0) {
        /* Exit: the data byte has been received by the follower */
        return true;
    } else {
        /* The data byte is not acknowledged by the follower */
        /* Exit: the data byte has not been received by the follower */
        return false;
    }
}

static uint8_t CEC_ReceiveByte(uint8_t HeaderDataIndicator) {
    uint8_t TempReceiveBit = 0;
    bool ReceiveByteStatus = true;

    /* Initialize cec_counter */
    cec_byte = 0;

    for (cec_bit = 0; cec_bit <= 7; cec_bit++) {
        /* Shift the data byte */
        cec_byte <<= 1;

        /* Receive the data bit */
        TempReceiveBit = CEC_ReceiveDataBit();

        /* If the received bit was received incorrectly */
        if (TempReceiveBit == 0xFF) {
            /* If the received bit is wrong */
            /* Store the error status */
            ReceiveByteStatus = false;
        }

        /* Build the data byte (MSB first) */
        cec_byte |= TempReceiveBit;
    }

    /* Read EOM bit */
    cec_eom = CEC_ReceiveDataBit();

    /* If the EOM bit was received incorrectly */
    if (cec_eom == 0xFF) {
        /* If the received bit is wrong */
        /* Store the error status */
        ReceiveByteStatus = false;
    }

    /* If the byte to send is a "Data" block */
    if (HeaderDataIndicator == DataBlock) {
        /* If the bits has been received correctly, acknowledge the data byte */
        if (ReceiveByteStatus != false) {
            /* Send the Ack bit */
            CEC_SendAckBit();

            /* Byte received correctly: return 1 */
            return 1;
        } else {
            /* Otherwise do not acknowledge the received byte */
            /* Byte received incorrectly: return 0 */
            return 0;
        }
    } else {
        /* If the byte is a "Header" block */
        /* If the bits has been received correctly and do not acknowledge the byte */
        if (ReceiveByteStatus != false) {
            /* Byte received correctly: return 1 */
            return 1;
        } else {
            /* If the bits has been received incorrectly and do not acknowledge the byte */
            /* Byte received incorrectly: return 0 */
            return 0;
        }
    }
}

int cec_init(void) {
    omv_gpio_config(OMV_CEC_PIN, OMV_GPIO_MODE_OUTPUT_OD, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_CEC_PIN, 1);
    mp_hal_delay_ms(1);
    return 0;
}

void cec_deinit(void) {
    omv_gpio_irq_enable(OMV_CEC_PIN, false);
    omv_gpio_deinit(OMV_CEC_PIN);
}

int cec_send_frame(uint8_t InitiatorAddress, uint8_t FollowerAddress, uint8_t MessageLength, uint8_t *Message) {
    uint8_t i = 0;
    uint8_t HeaderBlockValueToSend = 0;

    cec_last_byte = 0;

    /* Build the Header block to send */
    HeaderBlockValueToSend = (((InitiatorAddress & 0xF) << 4) | (FollowerAddress & 0xF));

    /* Send start bit */
    CEC_SendStartBit();

    /* Send initiator and follower addresses. If the Header block is not
       transmitted successfully then exit and return error */
    if (CEC_SendByte(HeaderBlockValueToSend) == false) {
        /* Exit and return send failed */
        return -1;
    }

    /* Send data bytes */
    for (i = 0; i < MessageLength; i++) {
        if (i == (MessageLength - 1)) {
            cec_last_byte = 1;
        }

        /* Send data byte and check if the follower sent the ACK bit = 0 */
        if (CEC_SendByte(Message[i]) == false) {
            /* Exit and return send failed */
            return -1;
        }
    }

    return 0;
}

int cec_receive_frame(uint8_t *Message, size_t *MessageLength,
                      uint8_t FollowerAddress, uint8_t *InitiatorAddress) {
    uint32_t i = 0;
    uint32_t InitAddress = 0;
    uint32_t ReceiveStatus = 1;
    uint32_t FrameSendToMe = 0;
    cec_byte = 0;
    cec_eom = 0;

    /* If the start bit has been received successfully */
    if (CEC_ReceiveStartBit()) {
        /* Get the first byte and check if the byte has been received correctly */
        if (CEC_ReceiveByte(HeaderBlock)) {
            /* Get the initiator address */
            InitAddress = cec_byte >> 4;

            /* If the frame was sent to me */
            if ((cec_byte & 0x0F) == FollowerAddress) {
                CEC_SendAckBit(); /* Send Acknowledge bit = 0 */
                FrameSendToMe = 1;

                /* Continue to read the data byte since the frame is not completed */
                while (!cec_eom) {
                    /* Check if the byte has been received correctly */
                    if (CEC_ReceiveByte(DataBlock)) {
                        /* Build the frame */
                        Message[i] = cec_byte;
                        i++;
                    } else {
                        /* If the byte has not been received correctly */
                        /* Set receive status bit (Error) */
                        ReceiveStatus = 0;
                    }
                }
            }
        }
    } else {
        /* If the start bit has not been received successfully */
        /* Set receive status bit (Error) */
        ReceiveStatus = 0;
    }

    if (ReceiveStatus && FrameSendToMe) {
        *MessageLength = i;
        *InitiatorAddress = InitAddress;
        return 0;
    }
    return -1;
}
#endif // OMV_DISPLAY_CEC_ENABLE

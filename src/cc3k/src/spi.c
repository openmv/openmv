/*****************************************************************************
 *
 *  spi.c - CC3000 Host Driver Implementation.
 *  Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
#include <string.h>
#include <mdefs.h>
#include <pincfg.h>
#include <stm32f4xx_hal.h>
#include "hci.h"
#include "spi.h"
#include "evnt_handler.h"

#define READ                3
#define WRITE               1

#define HI(value)           (((value) & 0xFF00) >> 8)
#define LO(value)           ((value) & 0x00FF)

#define SPI_TIMEOUT         (1000)
#define HEADERS_SIZE_EVNT   (SPI_HEADER_SIZE + 5)

/* SPI bus states */
#define eSPI_STATE_POWERUP                  (0)
#define eSPI_STATE_INITIALIZED              (1)
#define eSPI_STATE_IDLE                     (2)
#define eSPI_STATE_WRITE_IRQ                (3)
#define eSPI_STATE_WRITE_FIRST_PORTION      (4)
#define eSPI_STATE_WRITE_EOT                (5)
#define eSPI_STATE_READ_IRQ                 (6)
#define eSPI_STATE_READ_FIRST_PORTION       (7)
#define eSPI_STATE_READ_EOT                 (8)

typedef struct {
    gcSpiHandleRx  SPIRxHandler;
    unsigned short usTxPacketLength;
    unsigned short usRxPacketLength;
    unsigned long  ulSpiState;
    unsigned char *pTxPacket;
    unsigned char *pRxPacket;
} tSpiInformation;
tSpiInformation sSpiInformation;

// The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
// for the purpose of detection of the overrun. The location of the memory where the magic number
// resides shall never be written. In case it is written - the overrun occured and either recevie function
// or send function will stuck forever.
#define CC3000_BUFFER_MAGIC_NUMBER (0xDE)

char spi_buffer[CC3000_RX_BUFFER_SIZE];
unsigned char wlan_tx_buffer[CC3000_TX_BUFFER_SIZE];

static SPI_HandleTypeDef SPIHandle;

void SpiWriteDataSynchronous(unsigned char *data, unsigned short size);
void SpiReadDataSynchronous(unsigned char *data, unsigned short size);

void SpiClose(void)
{
    if (sSpiInformation.pRxPacket) {
        sSpiInformation.pRxPacket = 0;
    }

    tSLInformation.WlanInterruptDisable();

    //HAL_SPI_DeInit(&SPIHandle);
}

void SpiOpen(gcSpiHandleRx pfRxHandler)
{
    /* initialize SPI state */
    sSpiInformation.ulSpiState = eSPI_STATE_POWERUP;
    sSpiInformation.SPIRxHandler = pfRxHandler;
    sSpiInformation.usTxPacketLength = 0;
    sSpiInformation.pTxPacket = NULL;
    sSpiInformation.pRxPacket = (unsigned char *)spi_buffer;
    sSpiInformation.usRxPacketLength = 0;
    spi_buffer[CC3000_RX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;
    wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;

    /* SPI configuration */
    SPIHandle.Instance               = WLAN_SPI;
    SPIHandle.Init.Mode              = SPI_MODE_MASTER;
    SPIHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SPIHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SPIHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SPIHandle.Init.CLKPhase          = SPI_PHASE_2EDGE;
    SPIHandle.Init.NSS               = SPI_NSS_SOFT;
    SPIHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    SPIHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SPIHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
    SPIHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    SPIHandle.Init.CRCPolynomial     = 7;

    /* Initialize the WLAN SPI */
    if (HAL_SPI_Init(&SPIHandle) != HAL_OK) {
        /* Initialization Error */
        BREAK();
    }

    uint8_t buf[1];
    HAL_SPI_Receive(&SPIHandle, buf, sizeof(buf), SPI_TIMEOUT);

    /* Configure and enable IRQ Channel */
    HAL_NVIC_SetPriority(WLAN_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(WLAN_IRQn);
    HAL_Delay(500);

    tSLInformation.WlanInterruptEnable();
}


void SpiPauseSpi(void)
{
    HAL_NVIC_DisableIRQ(WLAN_IRQn);
}

void SpiResumeSpi(void)
{
    HAL_NVIC_EnableIRQ(WLAN_IRQn);
}

long ReadWlanInterruptPin(void)
{
    return HAL_GPIO_ReadPin(WLAN_IRQ_PORT, WLAN_IRQ_PIN);
}

void WriteWlanPin(unsigned char val)
{
    if (val == WLAN_ENABLE) {
        /* Set WLAN EN high */
        HAL_GPIO_WritePin(WLAN_EN_PORT, WLAN_EN_PIN, GPIO_PIN_SET);
    } else {
        /* Set WLAN EN LOW */
        HAL_GPIO_WritePin(WLAN_EN_PORT, WLAN_EN_PIN, GPIO_PIN_RESET);
    }
}

void __delay_cycles(volatile int x)
{
    while (x--);
}

long SpiFirstWrite(unsigned char *ucBuf, unsigned short usLength)
{
    WLAN_SELECT();

    // Assuming we are running on 24 MHz ~50 micro delay is 1200 cycles;
    __delay_cycles(1200);

    // SPI writes first 4 bytes of data
    SpiWriteDataSynchronous(ucBuf, 4);

    __delay_cycles(1200);

    SpiWriteDataSynchronous(ucBuf + 4, usLength - 4);

    // From this point on - operate in a regular way
    sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

    WLAN_DESELECT();

    return(0);
}

long SpiWrite(unsigned char *pUserBuffer, unsigned short usLength)
{
    unsigned char ucPad = 0;

    // Figure out the total length of the packet in order to figure out if there 
    // is padding or not
    if(!(usLength & 0x0001)) {
        ucPad++;
    }

    pUserBuffer[0] = WRITE;
    pUserBuffer[1] = HI(usLength + ucPad);
    pUserBuffer[2] = LO(usLength + ucPad);
    pUserBuffer[3] = 0;
    pUserBuffer[4] = 0;

    usLength += (SPI_HEADER_SIZE + ucPad);

    // The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
    // for the purpose of detection of the overrun. If the magic number is overriten - buffer overrun
    // occurred - and we will stuck here forever!
    if (wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER) {
        while (1);
    }

    if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP) {
        while (sSpiInformation.ulSpiState != eSPI_STATE_INITIALIZED);
    }

    if (sSpiInformation.ulSpiState == eSPI_STATE_INITIALIZED) {
        // This is time for first TX/RX transactions over SPI:
        // the IRQ is down - so need to send read buffer size command
        SpiFirstWrite(pUserBuffer, usLength);
    } else {
        //
        // We need to prevent here race that can occur in case 2 back to back packets are sent to the
        // device, so the state will move to IDLE and once again to not IDLE due to IRQ
        //
        tSLInformation.WlanInterruptDisable();

        while (sSpiInformation.ulSpiState != eSPI_STATE_IDLE);

        sSpiInformation.ulSpiState = eSPI_STATE_WRITE_IRQ;
        sSpiInformation.pTxPacket = pUserBuffer;
        sSpiInformation.usTxPacketLength = usLength;

        // Assert the CS line and wait till SSI IRQ line is active and then initialize write operation
        WLAN_SELECT();

        // Re-enable IRQ - if it was not disabled - this is not a problem...
        tSLInformation.WlanInterruptEnable();

        // check for a missing interrupt between the CS assertion and enabling back the interrupts
        if (tSLInformation.ReadWlanInterruptPin() == 0) {
            SpiWriteDataSynchronous(sSpiInformation.pTxPacket, sSpiInformation.usTxPacketLength);

            sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

            WLAN_DESELECT();
        }
    }

    // Due to the fact that we are currently implementing a blocking situation
    // here we will wait till end of transaction
    while (eSPI_STATE_IDLE != sSpiInformation.ulSpiState);

    return(0);
}

void SpiWriteDataSynchronous(unsigned char *data, unsigned short size)
{
    __disable_irq();
    if (HAL_SPI_TransmitReceive(&SPIHandle, data, data, size, SPI_TIMEOUT) != HAL_OK) {
        BREAK();
    }
    __enable_irq();
}

void SpiReadDataSynchronous(unsigned char *data, unsigned short size)
{
    memset(data, READ, size);
    __disable_irq();
    if (HAL_SPI_TransmitReceive(&SPIHandle, data, data, size, SPI_TIMEOUT) != HAL_OK) {
        BREAK();
    }
    __enable_irq();
}

void SpiReadPacket(void)
{
    int length;

    /* read SPI header */
    SpiReadDataSynchronous(sSpiInformation.pRxPacket, SPI_HEADER_SIZE);

    /* parse data length  */
    STREAM_TO_UINT8(sSpiInformation.pRxPacket, SPI_HEADER_SIZE-1, length);

    /* read the remainder of the packet */
    SpiReadDataSynchronous(sSpiInformation.pRxPacket + SPI_HEADER_SIZE, length);

    sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
}

void SpiReadHeader(void)
{
    SpiReadDataSynchronous(sSpiInformation.pRxPacket, 10);
}

void SpiTriggerRxProcessing(void)
{
    SpiPauseSpi();
    WLAN_DESELECT();

    // The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
    // for the purpose of detection of the overrun. If the magic number is overriten - buffer overrun
    // occurred - and we will stuck here forever!
    if (sSpiInformation.pRxPacket[CC3000_RX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER) {
        while (1);
    }

    sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
    sSpiInformation.SPIRxHandler(sSpiInformation.pRxPacket + SPI_HEADER_SIZE);
}


long SpiReadDataCont(void)
{
    long data_to_recv=0;
    unsigned char *evnt_buff, type;

    //determine what type of packet we have
    evnt_buff =  sSpiInformation.pRxPacket;
    STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE), HCI_PACKET_TYPE_OFFSET, type);

    switch (type) {
        case HCI_TYPE_DATA:{
                // We need to read the rest of data..
                STREAM_TO_UINT16((char *)(evnt_buff + SPI_HEADER_SIZE),
                        HCI_DATA_LENGTH_OFFSET, data_to_recv);
                if (!((HEADERS_SIZE_EVNT + data_to_recv) & 1)) {
                    data_to_recv++;
                }

                if (data_to_recv) {
                    SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
                }
                break;
            }
        case HCI_TYPE_EVNT: {
                // Calculate the rest length of the data
                STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE),
                        HCI_EVENT_LENGTH_OFFSET, data_to_recv);
                data_to_recv -= 1;

                // Add padding byte if needed
                if ((HEADERS_SIZE_EVNT + data_to_recv) & 1) {
                    data_to_recv++;
                }

                if (data_to_recv) {
                    SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
                }

                sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
                break;
            }
    }

    return 0;
}

void SSIContReadOperation(void)
{
    // The header was read - continue with  the payload read
    if (!SpiReadDataCont()) {
        /* All the data was read - finalize handling by switching
           to the task and calling from task Event Handler */
        SpiTriggerRxProcessing();
    }
}

void WLAN_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_FLAG(WLAN_EXTI_LINE)) {
        /* Clear the EXTI pending bit */
        __HAL_GPIO_EXTI_CLEAR_FLAG(WLAN_EXTI_LINE);

        switch (sSpiInformation.ulSpiState) {
            case eSPI_STATE_POWERUP:
                /* This means IRQ line was low call a callback of HCI Layer to inform on event */
                sSpiInformation.ulSpiState = eSPI_STATE_INITIALIZED;
                break;
            case eSPI_STATE_IDLE:
                sSpiInformation.ulSpiState = eSPI_STATE_READ_IRQ;

                /* IRQ line goes down - we are start reception */
                WLAN_SELECT();

                // Wait for TX/RX Compete which will come as DMA interrupt
                SpiReadHeader();

                sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;

                SSIContReadOperation();
                break;
            case eSPI_STATE_WRITE_IRQ:
                SpiWriteDataSynchronous(sSpiInformation.pTxPacket, sSpiInformation.usTxPacketLength);

                sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

                WLAN_DESELECT();
                break;
        }
    }
}

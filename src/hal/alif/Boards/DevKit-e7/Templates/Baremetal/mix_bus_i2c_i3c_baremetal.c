/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     mix_bus_i2c_i3c_testApp.c
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     04-April-2022
 * @brief    Baremetal testapp to verify Mix Bus i2c and i3c communication with
 *            i2c + i3c slave devices using i3c IP
 *
 *           Hardware setup
 *            TestApp will communicate with Accelerometer and BMI Slave,
 *             which are on-board connected with the I3C_D.
 *             (so no any external hardware connection are required).
 *              Pins used:
 *               P7_6 (SDA)
 *               P7_7 (SCL)
 *               GND
 *
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* System Includes */
#include <stdio.h>
#include <string.h>

/* Project Includes */
#include "Driver_I3C.h"
#include "system_utils.h"

/* PINMUX Driver */
#include "pinconf.h"
#include "Driver_GPIO.h"

#include "RTE_Device.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


/* i3c Driver instance 0 */
extern ARM_DRIVER_I3C Driver_I3C;
static ARM_DRIVER_I3C *I3Cdrv = &Driver_I3C;

void mix_bus_i2c_i3c_demo_entry();

/* i3c callback events */
typedef enum _I3C_CB_EVENT{
    I3C_CB_EVENT_SUCCESS        = (1 << 0),
    I3C_CB_EVENT_ERROR          = (1 << 1)
}I3C_CB_EVENT;

volatile int32_t cb_event_flag = 0;

/**
  \fn          int32_t hardware_init(void)
  \brief       i3c hardware pin initialization:
                - PIN-MUX configuration
                - PIN-PAD configuration
  \param[in]   void
  \return      0:success; -1:failure
*/
int32_t hardware_init(void)
{
    /* for I3C_D(PORT_7 PIN_6(SDA)/PIN_7(SCL)) instance,
     *  for I3C in I3C mode (not required for I3C in I2C mode)
     *  GPIO voltage level(flex) has to be change to 1.8-V power supply.
     *
     *  GPIO_CTRL Register field VOLT:
     *   Select voltage level for the 1.8-V/3.3-V (flex) I/O pins
     *    0x0: I/O pin will be used with a 3.3-V power supply
     *    0x1: I/O pin will be used with a 1.8-V power supply
     */

    /* Configure GPIO flex I/O pins to 1.8-V:
     *  P7_6 and P7_7 pins are part of GPIO flex I/O pins,
     *   so we can use any one of the pin to configure flex I/O.
     */
#define GPIO7_PORT          7

    extern  ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO7_PORT);
    ARM_DRIVER_GPIO *gpioDrv = &ARM_Driver_GPIO_(GPIO7_PORT);

    int32_t  ret = 0;
    uint32_t arg = 0;

    ret = gpioDrv->Initialize(PIN_6, NULL);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to initialize GPIO \n");
        return ARM_DRIVER_ERROR;
    }

    ret = gpioDrv->PowerControl(PIN_6, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to powered full GPIO \n");
        return ARM_DRIVER_ERROR;
    }

    /* select control argument as flex 1.8-V */
    arg = ARM_GPIO_FLEXIO_VOLT_1V8;
    ret = gpioDrv->Control(PIN_6, ARM_GPIO_CONFIG_FLEXIO, &arg);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to control GPIO Flex \n");
        return ARM_DRIVER_ERROR;
    }

    /* I3C_SDA_D */
    pinconf_set(PORT_7, PIN_6, PINMUX_ALTERNATE_FUNCTION_6,
                PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP | \
                PADCTRL_OUTPUT_DRIVE_STRENGTH_4MA);

    /* I3C_SCL_D */
    pinconf_set(PORT_7, PIN_7, PINMUX_ALTERNATE_FUNCTION_6,
                PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP | \
                PADCTRL_OUTPUT_DRIVE_STRENGTH_4MA);

    return ARM_DRIVER_OK;
}

/**
  \fn          void I3C_callback(uint32_t event)
  \brief       i3c isr callback
  \param[in]   event: i3c Event
  \return      none
*/
void I3C_callback(uint32_t event)
{
    if (event & ARM_I3C_EVENT_TRANSFER_DONE)
    {
        /* Transfer Success */
        cb_event_flag = I3C_CB_EVENT_SUCCESS;
    }

    if (event & ARM_I3C_EVENT_TRANSFER_ERROR)
    {
        /* Transfer Error */
        cb_event_flag = I3C_CB_EVENT_ERROR;
    }
}

/**
  \fn          void mix_bus_i2c_i3c_demo_entry()
  \brief       Baremetal testApp to verify mix bus i2c and i3c communication with
                i2c + i3c slave devices using i3c IP.

               This demo function does:
                 - initialize i3c driver;
                 - set i3c speed mode to Mixed bus i2c/i3c Fast Mode 400 Kbps;
                 - assign dynamic address and attach all i3c slave devices to i3c;
                 - send/receive i3c CCC (Common Command Codes) only for i3c slaves
                 - attach all i2c slave devices to i3c;
                 - continuously read from specific register address(chip-id)
                    for all the attached slaves;
                 - display result depending on whether
                   slave has given ACK or NACK.
  \param[in]   none
  \return      none
*/
void mix_bus_i2c_i3c_demo_entry()
{

/* Maximum 8 Slave Devices are supported */
#define MAX_SLAVE_SUPPORTED   8

/* Added 2 slaves for demo purpose
 *   i3c : Accelerometer
 *   i2c : BMI
 */
#define TOTAL_SLAVE           2

/* ICM-42670-P Accelerometer Slave address(On-chip attached to Board) */
#define I3C_ACCERO_ADDR       0x68

/* BMI323 Slave address(On-chip attached to Board) */
#define I2C_BMI_ADDR          0x69

/* ICM-42670-P Accelerometer Slave chip-id register(WHO AM I) address and value
 *  as per datasheet
 */
#define I3C_ACCERO_REG_WHO_AM_I_ADDR        0x75
#define I3C_ACCERO_REG_WHO_AM_I_VAL         0x67

/* BMI323 Slave Chip-id register address and value as per datasheet. */
#define I2C_BMI_REG_CHIP_ID_ADDR            0x00
#define I2C_BMI_REG_CHIP_ID_VAL             0x43

    uint32_t  i         = 0;
    uint32_t  len       = 0;
    uint32_t  retry_cnt = 0;
    int32_t   ret       = 0;

    /* Array of slave address :
     *       Dynamic Address for i3c and
     *       Static  Address for i2c
     */
    uint8_t slave_addr[TOTAL_SLAVE] =
    {
        0, /* I3C Accero  Dynamic Address: To be updated later using MasterAssignDA */
        I2C_BMI_ADDR /* I2C BMI Slave Address. */
    };

    /* Slave Register Address */
    uint8_t slave_reg_addr[TOTAL_SLAVE] =
    {
        I3C_ACCERO_REG_WHO_AM_I_ADDR,
        I2C_BMI_REG_CHIP_ID_ADDR
    };

    /* @NOTE:
     *  I3C expects data to be aligned in 4-bytes (multiple of 4) for DMA.
     */

    /* transmit data to i3c */
    uint8_t __ALIGNED(4) tx_data[4] = {0};

    /* receive data from i3c */
    uint8_t __ALIGNED(4) rx_data[4] = {0};

    /* receive data used for comparison. */
    uint8_t cmp_rx_data = 0;

    /* actual receive data as per slave datasheet */
    uint8_t actual_rx_data[TOTAL_SLAVE] =
    {
        I3C_ACCERO_REG_WHO_AM_I_VAL,
        I2C_BMI_REG_CHIP_ID_VAL
    };

    ARM_DRIVER_VERSION version;

    /* I3C CCC (Common Command Codes) */
    ARM_I3C_CMD i3c_cmd;
    uint8_t i3c_cmd_tx_data[4] = {0x0F};
    uint8_t i3c_cmd_rx_data[4] = {0};

    printf("\r\n>> mix bus i2c and i3c communication demo starting up <<\r\n");

    /* Get i3c driver version. */
    version = I3Cdrv->GetVersion();
    printf("\r\n i3c version api:0x%X driver:0x%X \r\n",
            version.api, version.drv);

    if((version.api < ARM_DRIVER_VERSION_MAJOR_MINOR(7U, 0U))       ||
       (version.drv < ARM_DRIVER_VERSION_MAJOR_MINOR(7U, 0U)))
    {
        printf("\r\n Error: >>>Old driver<<< Please use new one \r\n");
        return;
    }

    /* Initialize i3c hardware pins using PinMux Driver. */
    ret = hardware_init();
    if(ret != 0)
    {
        printf("\r\n Error: i3c hardware_init failed.\r\n");
        return;
    }

    /* Initialize I3C driver */
#if RTE_I3C_BLOCKING_MODE_ENABLE
    ret = I3Cdrv->Initialize(NULL);
#else
    ret = I3Cdrv->Initialize(I3C_callback);
#endif
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Initialize failed.\r\n");
        return;
    }

    /* Power up I3C peripheral */
    ret = I3Cdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Power Up failed.\r\n");
        goto error_uninitialize;
    }

    /* Initialize I3C master */
    ret = I3Cdrv->Control(I3C_MASTER_INIT, 0);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: Master Init control failed.\r\n");
        goto error_uninitialize;
    }

    /* i3c Speed Mode Configuration for i2c comm:
     *  I3C_BUS_MODE_MIXED_FAST_I2C_FMP_SPEED_1_MBPS  : Fast Mode Plus   1 Mbps
     *  I3C_BUS_MODE_MIXED_FAST_I2C_FM_SPEED_400_KBPS : Fast Mode      400 Kbps
     *  I3C_BUS_MODE_MIXED_SLOW_I2C_SS_SPEED_100_KBPS : Standard Mode  100 Kbps
     */
    ret = I3Cdrv->Control(I3C_MASTER_SET_BUS_MODE,
                          I3C_BUS_MODE_MIXED_FAST_I2C_FMP_SPEED_1_MBPS);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Control failed.\r\n");
        goto error_poweroff;
    }

    /* Reject Hot-Join request */
    ret = I3Cdrv->Control(I3C_MASTER_SETUP_HOT_JOIN_ACCEPTANCE, 0);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: Hot Join control failed.\r\n");
        goto error_uninitialize;
    }

    /* Reject Master request */
    ret = I3Cdrv->Control(I3C_MASTER_SETUP_MR_ACCEPTANCE, 0);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: Master Request control failed.\r\n");
        goto error_uninitialize;
    }

    /* Reject Slave Interrupt request */
    ret = I3Cdrv->Control(I3C_MASTER_SETUP_SIR_ACCEPTANCE, 0);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: Slave Interrupt Request control failed.\r\n");
        goto error_uninitialize;
    }

    /* Delay for 1000 micro second.
     *  @Note: Minor delay is required if prints are disable.
     */
    sys_busy_loop_us(1000);

    /* Assign Dynamic Address for Accelerometer */
    printf("\r\n >> i3c: Get dynamic addr for static addr:0x%X.\r\n",
            I3C_ACCERO_ADDR);

    /* clear callback event flag. */
    cb_event_flag = 0;

    i3c_cmd.rw            = 0U;
    i3c_cmd.cmd_id        = I3C_CCC_SETDASA;
    i3c_cmd.len           = 1U;
    /* Assign Slave's Static address */
    i3c_cmd.addr          = I3C_ACCERO_ADDR;
    i3c_cmd.data          = NULL;
    i3c_cmd.def_byte      = 0U;

    ret = I3Cdrv->MasterAssignDA(&i3c_cmd);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C MasterAssignDA failed.\r\n");
        goto error_poweroff;
    }

#if !RTE_I3C_BLOCKING_MODE_ENABLE
    while(!((cb_event_flag == I3C_CB_EVENT_SUCCESS) ||
            (cb_event_flag == I3C_CB_EVENT_ERROR)));

    if(cb_event_flag == I3C_CB_EVENT_ERROR)
    {
        printf("\nError: I3C MasterAssignDA failed\n");
    }
#endif

    /* Get assigned dynamic address for the static address */
    ret = I3Cdrv->GetSlaveDynAddr(I3C_ACCERO_ADDR, &slave_addr[0]);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Failed to get Dynamic Address.\r\n");
    }
    else
    {
        printf("\r\n >> i3c: Rcvd dyn_addr:0x%X for static addr:0x%X\r\n",
                slave_addr[0],I3C_ACCERO_ADDR);
    }

    /* Delay for n micro second. */
    sys_busy_loop_us(1000);

    /* i3c Speed Mode Configuration: I3C_BUS_NORMAL_MODE */
    ret = I3Cdrv->Control(I3C_MASTER_SET_BUS_MODE,
                          I3C_BUS_NORMAL_MODE);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Control failed.\r\n");
        goto error_poweroff;
    }

    /* clear callback event flag. */
    cb_event_flag = 0;

    /* demo for I3C CCC (Common Command Codes) APIs */

    /* write I3C_CCC_SETMWL (Set Max Write Length)
     * command to Accelerometer slave */
    i3c_cmd.rw     = 0;
    i3c_cmd.cmd_id = I3C_CCC_SETMWL(false);
    i3c_cmd.len    = 1;
    i3c_cmd.addr   = slave_addr[0];
    i3c_cmd.data   = i3c_cmd_tx_data;

    ret = I3Cdrv->MasterSendCommand(&i3c_cmd);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C MasterSendCommand failed.\r\n");
        goto error_detach;
    }

#if !RTE_I3C_BLOCKING_MODE_ENABLE
    while(!((cb_event_flag == I3C_CB_EVENT_SUCCESS) ||
            (cb_event_flag == I3C_CB_EVENT_ERROR)));

    if(cb_event_flag == I3C_CB_EVENT_ERROR)
    {
        printf("\nError: I3C MasterSendCommand failed\n");
    }
#endif

    /* clear callback event flag. */
    cb_event_flag = 0;

    /* Delay for 1000 micro second. */
    sys_busy_loop_us(1000);

    /* read I3C_CCC_GETMWL (Get Max Write Length) command
     * from Accelerometer slave */
    i3c_cmd.rw     = 1;
    i3c_cmd.cmd_id = I3C_CCC_GETMWL;
    i3c_cmd.len    = 1;
    i3c_cmd.addr   = slave_addr[0];
    i3c_cmd.data   = i3c_cmd_rx_data;

    ret = I3Cdrv->MasterSendCommand(&i3c_cmd);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C MasterSendCommand failed.\r\n");
        goto error_detach;
    }

#if !RTE_I3C_BLOCKING_MODE_ENABLE
    while(!((cb_event_flag == I3C_CB_EVENT_SUCCESS) ||
            (cb_event_flag == I3C_CB_EVENT_ERROR)));

    if(cb_event_flag == I3C_CB_EVENT_ERROR)
    {
        printf("\nError: I3C MasterSendCommand failed\n");
    }
#endif

    /* compare tx and rx command data for Accelerometer slave */
    if( memcmp(i3c_cmd_rx_data, i3c_cmd_tx_data, 1) == 0 )
    {
        printf("\r\n>> i3c Accelerometer SendCommand Success.\r\n");
    }
    else
    {
        printf("\r\n>> i3c Accelerometer SendCommand failed.\r\n");
    }

    /* Delay for 1000 micro second. */
    sys_busy_loop_us(1000);

    /* Attach i2c BMI slave using static address */
    printf("\r\n >> Attaching i2c BMI slave addr: 0x%X to i3c...\r\n",
            slave_addr[1]);

    ret = I3Cdrv->AttachSlvDev(ARM_I3C_DEVICE_TYPE_I2C, slave_addr[1]);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Attach I2C device failed.\r\n");
        goto error_poweroff;
    }


    /*
     * @Note:
     *  How much data(register address + actual data) user has to Transmit/Receive ?
     *   it depends on Slave's register address location bytes.
     *
     *  Generally, Camera Slave supports       16-bit(2 Byte) reg-addr and (8/16/32 bit) data
     *   Others Accero/BMI/EEPROM supports      8-bit(1 Byte) reg-addr and (8/16/32 bit) data
     *
     *  First LSB[7-0] will be added to TX FIFO and first transmitted on the i3c bus;
     *   remaining bytes will be added in LSB -> MSB order.
     *
     *  For Slave who supports 16-bit(2 Byte) register address and data:
     *   Register Address[15:8] : Needs to be Transmit First  to the i3c
     *   Register Address[07:0] : Needs to be Transmit Second to the i3c
     *
     *  That means,
     *
     *  While transmitting to TX FIFO,
     *   MSB of TX data needs to be added first to the TX FIFO.
     *
     *  While receiving from RX FIFO,
     *   First MSB will be received from RX FIFO.
     *
     *  START          I3C FIFO           END
     *  MSB                               LSB
     *  24-31 bit | 16-23 bit | 8-15 bit | 0-7 bit
     *
     *  So, USER has to modify
     *  Transmit/Receive data (Little Endian <-> Big Endian and vice versa)
     *  before sending/after receiving to/from i3c TX/RX FIFO.
     */


    /* Let's Continuously read from chip-id register address for
     *  all the attached slaves and display received data depending on
     *  whether slave has given ACK or NACK.
    */
    while(1)
    {
        for(i=0; i<TOTAL_SLAVE; i++)
        {
            /* To Read from any register address:
             *  First write register address using MasterTransmit and
             *   then Read data using MasterReceive
             */

            /* TX/RX length is 1 Byte
             * (assume slave requires 8-bit data for TX/RX).
             */
            len = 1;

            printf("\r\n -------------------------------------------- \r\n");
            printf("\r\n >> i=%d TX slave addr:0x%X reg_addr:[0]0x%X \r\n",
                    i, slave_addr[i], slave_reg_addr[i]);

            /* Delay for 1000 micro second. */
            sys_busy_loop_us(1000);

            /* clear callback event flag. */
            cb_event_flag = 0;

            /* For TX, User has to pass
             * Slave Address + TX data + length of the TX data.
             */
            tx_data[0] = slave_reg_addr[i];

            ret = I3Cdrv->MasterTransmit(slave_addr[i], tx_data, len);
            if(ret != ARM_DRIVER_OK)
            {
                printf("\r\n Error: I3C Master Transmit failed. \r\n");
                goto error_detach;
            }

#if RTE_I3C_BLOCKING_MODE_ENABLE
            printf("\r\n>> i=%d TX Success: Got ACK from slave :0x%X\r\n",
                    i, slave_addr[i]);
#else
            /* wait till any event success/error comes in isr callback */
            retry_cnt = 1000;
            while (retry_cnt)
            {
                retry_cnt--;
                /* Delay for 1000 micro second. */
                sys_busy_loop_us(1000);

                if(cb_event_flag == I3C_CB_EVENT_SUCCESS)
                {
                    printf("\r\n>> i=%d TX Success: Got ACK from slave :0x%X\r\n",
                            i, slave_addr[i]);
                    break;
                }
                if(cb_event_flag == I3C_CB_EVENT_ERROR)
                {
                    /* TX Error: Got NACK from slave */
                    printf("\r\n>> i=%d TX Error: Got NACK from slave:0x%X\r\n",
                            i, slave_addr[i]);
                    break;
                }
            }

            if ( (!(retry_cnt)) && (!(cb_event_flag)) )
            {
                printf("Error: event retry_cnt \r\n");
                goto error_detach;
            }
#endif
            /* RX */
            printf("\r\n\r\n >> i=%d RX slave addr:0x%X \r\n",i, slave_addr[i]);

            /* clear rx data buffer. */
            rx_data[0] = 0;
            rx_data[1] = 0;
            rx_data[2] = 0;

            /* TX/RX length is 1 Byte
             * (assume slave requires 8-bit data for TX/RX).
             */
            len = 1;

            if(slave_addr[i] == I2C_BMI_ADDR)
            {
                /* BMI slave supports(as per datasheet ch-4):
                 *  - 8-bit addressing and 16-bit data
                 *  - Each register read operation required
                 *     2 bytes of dummy data (for i2c/i3c) before the payload.
                 *  - if only LSB of an register needed,
                 *     only the first byte has to be read and
                 *     second one will be discarded by slave automatically.
                 *     (so RX length = 3 (2 dummy bytes + LSB byte))
                 */
                len = 3;
            }

            /* clear callback event flag. */
            cb_event_flag = 0;

            /* For RX, User has to pass
             * Slave Address + Pointer to RX data + length of the RX data.
             */
            ret = I3Cdrv->MasterReceive(slave_addr[i], rx_data, len);
            if(ret != ARM_DRIVER_OK)
            {
                printf("\r\n Error: I3C Master Receive failed. \r\n");
                goto error_detach;;
            }
#if RTE_I3C_BLOCKING_MODE_ENABLE

            cmp_rx_data = rx_data[0];

            if(slave_addr[i] == I2C_BMI_ADDR)
            {
                /* BMI read: gives 2 bytes of dummy data[0][1] + LSB[2] */
                cmp_rx_data = rx_data[2];
            }

            /* RX Success: Got ACK from slave */
            printf("\r\n>> i=%d RX Success: Got ACK from slave addr:0x%X.\r\n",
                    i, slave_addr[i]);

            printf("\r\n>> i=%d RX rcvd data from slv: [0]0x%X.", i,cmp_rx_data);
            printf("\t\t Actual data:0x%X\r\n",actual_rx_data[i]);

            if(cmp_rx_data == actual_rx_data[i])
            {
                printf("\r\n>> i=%d RX rcvd Data from slave is VALID\r\n",i);
            }
#else
            /* wait till any event success/error comes in isr callback */
            retry_cnt =1000;
            while (retry_cnt)
            {
                retry_cnt--;
                /* Delay for 1000 micro second. */
                sys_busy_loop_us(1000);

                if(cb_event_flag == I3C_CB_EVENT_SUCCESS)
                {
                    cmp_rx_data = rx_data[0];

                    if(slave_addr[i] == I2C_BMI_ADDR)
                    {
                        /* BMI read: gives 2 bytes of dummy data[0][1] + LSB[2] */
                        cmp_rx_data = rx_data[2];
                    }

                    /* RX Success: Got ACK from slave */
                    printf("\r\n>> i=%d RX Success: Got ACK from slv:0x%X\r\n",
                            i, slave_addr[i]);

                    printf("\r\n>> i=%d Rcvd data from slave: [0]0x%X.",
                            i,cmp_rx_data);
                    printf("\t\t Actual data:0x%X\r\n", actual_rx_data[i]);

                    if(cmp_rx_data == actual_rx_data[i])
                    {
                        printf("\r\n>> i=%d RX rcvd VALID data \r\n",i);
                    }
                    else
                    {
                        printf("\r\n >> i=%d RX rcvd INVALID data\r\n",i);
                    }

                    break;
                }

                if(cb_event_flag == I3C_CB_EVENT_ERROR)
                {
                    /* RX Error: Got NACK from slave */
                    printf("\r\n>> i=%d RX Error: Got NACK from slave:0x%X \r\n",
                            i, slave_addr[i]);
                    break;
                }
            }

            if ( (!(retry_cnt)) && (!(cb_event_flag)) )
            {
                printf("Error: event retry_cnt \r\n");
                goto error_detach;
            }
#endif
            printf("\r\n -----------------XXX----------------- \r\n");
        }
    }


error_detach:

    /* Detach all attached i2c/i3c slave device. */
    for(i=0; i<TOTAL_SLAVE; i++)
    {
        printf("\r\n i=%d detaching slave:0x%X from i3c.\r\n",i, slave_addr[i]);
        ret = I3Cdrv->Detachdev(slave_addr[i]);
        if(ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error: I3C Detach device failed.\r\n");
        }
    }

error_poweroff:

    /* Power off I3C peripheral */
    ret = I3Cdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Power OFF failed.\r\n");
    }

error_uninitialize:

    /* Un-initialize I3C driver */
    ret = I3Cdrv->Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I3C Uninitialize failed.\r\n");
    }

    printf("\r\n XXX I3C demo testapp exiting XXX...\r\n");
}

/* Define main entry point.  */
int main()
{
#if defined(RTE_Compiler_IO_STDOUT_User)
    int32_t ret;
    ret = stdout_init();
    if(ret != ARM_DRIVER_OK)
    {
        while(1)
        {
        }
    }
#endif

    mix_bus_i2c_i3c_demo_entry();
    return 0;
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/

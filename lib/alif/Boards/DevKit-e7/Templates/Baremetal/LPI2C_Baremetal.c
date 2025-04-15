/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     LPI2C_Baremetal.c
 * @brief    TestApp to verify I2C Master and LPI2C Slave functionality
 *           using Baremetal without any operating system.
 * @note     LPI2C cannot transmit or receive more than 8 byte of memory.
 *           Code will verify:
 *            1.)Master transmit and Slave receive
 *            2.)Master receive  and Slave transmit
 *                I2C0 instance is taken as Master and
 *                LPI2C(Slave-only) instance is taken as Slave.
 *
 *           Hardware Connection:
 *           I2C0 SDA(P0_2) -> LPI2C SDA(P7_5)
 *           I2C0 SCL(P0_3) -> LPI2C SCL(P7_4)
 ******************************************************************************/

/* Include */
#include <stdio.h>
#include <string.h>

#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_I2C.h"
#include "pinconf.h"
#include "lpi2c.h"

#if !defined(M55_HE)
#error "This Demo application works only on RTSS_HE"
#endif

#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

/* I2C Driver instance */
extern ARM_DRIVER_I2C Driver_I2C0;
static ARM_DRIVER_I2C *I2C_mstdrv = &Driver_I2C0;

extern ARM_DRIVER_I2C Driver_LPI2C;
static ARM_DRIVER_I2C *LPI2C_slvdrv = &Driver_LPI2C;

volatile uint32_t mst_cb_status = 0;
volatile uint32_t slv_cb_status = 0;
int slv_rec_compare;
int slv_xfer_compare;

#define TAR_ADDRS         (0X40)   /* Target(Slave) Address, use by Master */
#define RESTART           (0X01)
#define STOP              (0X00)

/* master transmit and slave receive */
#define MST_BYTE_TO_TRANSMIT            3

/* slave transmit and master receive */
#define SLV_BYTE_TO_TRANSMIT            3

/* Master parameter set */

/* Master TX Data (Any random value). */
uint8_t MST_TX_BUF[MST_BYTE_TO_TRANSMIT] ={0x92,0x42,0x74};

/* master receive buffer */
uint8_t MST_RX_BUF[SLV_BYTE_TO_TRANSMIT];

/*  */
uint8_t MST_REC_DATA[SLV_BYTE_TO_TRANSMIT];

/* Master parameter set END  */


/* Slave parameter set */

/* slave receive buffer */
uint8_t SLV_RX_BUF[MST_BYTE_TO_TRANSMIT];

/* Slave TX Data (Any random value). */
uint8_t SLV_TX_BUF[SLV_BYTE_TO_TRANSMIT]={0x56,0x78,0x88};

/* Slave parameter set END */


static void i2c_mst_tranfer_callback(uint32_t event)
{

    if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
    /* Transfer or receive is finished */
    mst_cb_status = 1;
    }

}

static void i2c_slv_transfer_callback(uint32_t event)
{
    if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
    /* Transfer or receive is finished */
    slv_cb_status = 1;
    }
}

/* Pinmux for B0 */
void pinmux_config()
{
    /* LPI2C_SCL_A */
    pinconf_set(PORT_7, PIN_4, PINMUX_ALTERNATE_FUNCTION_5, \
         (PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP| PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA));

    /* LPI2C_SDA_A */
    pinconf_set(PORT_7, PIN_5, PINMUX_ALTERNATE_FUNCTION_6, \
         (PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP | PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA));

    /* I2C0_SDA_A */
    pinconf_set(PORT_0, PIN_2, PINMUX_ALTERNATE_FUNCTION_3, \
         (PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP | PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA));

    /* I2C0_SCL_A */
    pinconf_set(PORT_0, PIN_3, PINMUX_ALTERNATE_FUNCTION_3, \
         (PADCTRL_READ_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP | PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA));

}

void LPI2C_demo()
{
    int32_t   ret      = 0;
    ARM_DRIVER_VERSION version;

    printf("\r\n >>> LPI2C demo starting up !!! <<< \r\n");

    /* Pinmux */
    pinmux_config();

    version = I2C_mstdrv->GetVersion();
    version = LPI2C_slvdrv->GetVersion();
    printf("\r\n I2C version api:0x%X driver:0x%X...\r\n",version.api, version.drv);

    /* Initialize I2C driver */
    ret = I2C_mstdrv->Initialize(i2c_mst_tranfer_callback);
    if (ret != ARM_DRIVER_OK){
        printf("\r\n Error: I2C master init failed\n");
        return;
    }

    /* Initialize I2C driver */
    ret = LPI2C_slvdrv->Initialize(i2c_slv_transfer_callback);
    if (ret != ARM_DRIVER_OK){
        printf("\r\n Error: I2C slave init failed\n");
        return;
    }

    /* Power control I2C */
    ret = I2C_mstdrv->PowerControl(ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK){
        printf("\r\n Error: I2C Power up failed\n");
        goto error_uninitialize;
    }

    /* Power control I2C */
    ret = LPI2C_slvdrv->PowerControl(ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK){
        printf("\r\n Error: I2C Power up failed\n");
        goto error_uninitialize;
    }

    ret = I2C_mstdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
    if (ret != ARM_DRIVER_OK){
        printf("\r\n Error: I2C master init failed\n");
        goto error_poweroff;
    }

     printf("\n----------------Master transmit/slave receive-----------------------\n");

     LPI2C_slvdrv->SlaveReceive(SLV_RX_BUF, MST_BYTE_TO_TRANSMIT);

     /* delay */
     sys_busy_loop_us(1000);

     I2C_mstdrv->MasterTransmit(TAR_ADDRS, MST_TX_BUF, MST_BYTE_TO_TRANSMIT, STOP);

     /* wait for master/slave callback. */
     while (mst_cb_status == 0);
     mst_cb_status = 0;

     while (slv_cb_status == 0);
     slv_cb_status = 0;

     /* Compare received data. */
     if (memcmp(&SLV_RX_BUF, &MST_TX_BUF, MST_BYTE_TO_TRANSMIT))
     {
         printf("\n Error: Master transmit/slave receive failed \n");
         printf("\n ---Stop--- \r\n wait forever >>> \n");
         while(1);
     }

     printf("\n----------------Master receive/slave transmit-----------------------\n");

     LPI2C_slvdrv->SlaveTransmit(SLV_TX_BUF, 3);

     /* Delay */
     sys_busy_loop_us(500);

     I2C_mstdrv->MasterReceive(TAR_ADDRS, MST_REC_DATA, 1, STOP);

     /* wait for master/slave callback. */
     while (slv_cb_status == 0);
     slv_cb_status = 0;

     while (mst_cb_status == 0);
     mst_cb_status = 0;

     /* Store Receive data */
     MST_RX_BUF[0] = MST_REC_DATA[0];

     I2C_mstdrv->MasterReceive(TAR_ADDRS, MST_REC_DATA, 1, STOP);

     while (mst_cb_status == 0);
     mst_cb_status = 0;

     /* Store Receive data */
     MST_RX_BUF[1] = MST_REC_DATA[0];

     I2C_mstdrv->MasterReceive(TAR_ADDRS, MST_REC_DATA, 1, STOP);

     /* wait for master callback. */
     while (mst_cb_status == 0);
     mst_cb_status = 0;

     /* Store Receive data */
     MST_RX_BUF[2] = MST_REC_DATA[0];

     /* Compare received data. */
     if (memcmp(&SLV_TX_BUF, &MST_RX_BUF, SLV_BYTE_TO_TRANSMIT))
     {
         printf("\n Error: Master receive/slave transmit failed\n");
         printf("\n ---Stop--- \r\n wait forever >>> \n");
         while(1);
     }

     ret = I2C_mstdrv->Uninitialize();
     if (ret == ARM_DRIVER_OK)
     {
         printf("\r\n I2C Master Uninitialized\n");
         goto error_uninitialize;
     }
     ret = LPI2C_slvdrv->Uninitialize();
     if (ret == ARM_DRIVER_OK)
     {
         printf("\r\n I2C Slave Uninitialized\n");
         goto error_uninitialize;
     }

     printf("\n >>> LPI2C transfer completed without any error\n");
     printf("\n ---END--- \r\n wait forever >>> \n");
     while(1);

error_poweroff:
    /* Power off peripheral */
    ret = I2C_mstdrv->PowerControl(ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK)
    {
       printf("\r\n Error: I2C Power OFF failed.\r\n");
    }
    ret = LPI2C_slvdrv->PowerControl(ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK)
    {
       printf("\r\n Error: LPI2C Power OFF failed.\r\n");
    }

error_uninitialize:
    /* Un-initialize I2C driver */
    ret = I2C_mstdrv->Uninitialize();
    ret = LPI2C_slvdrv->Uninitialize();
    if (ret != ARM_DRIVER_OK)
    {
      printf("\r\n Error: I2C Uninitialize failed.\r\n");
    }
    printf("\r\n  LPI2C demo exiting...\r\n");
}

/* Define main entry point.  */
int main (void)
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
    /* Enter the demo Application.  */
    LPI2C_demo();
}

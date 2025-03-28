/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     MW_Baremetal.c
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     28-Jan-2024
 * @brief    Baremetal demo app configures the SPI2 instance in master
 *           mode with Microwire frame format and configures the SPI3 instance
 *           in slave mode with Microwire frame format. The data transfer
 *           occurs between master and slave or slave and master based on the
 *           MASTER_TO_SLAVE_TRANSFER macro.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include <stdio.h>
#include "stdint.h"
#include "string.h"
#include "pinconf.h"
#include "Driver_SPI.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

/* assign 1: To enable master to slave transfer
 *        0: To enable slave to master transfer */
#define MASTER_TO_SLAVE_TRANSFER   1

#define SPI2               2        /* SPI instance 2 configured Master */
#define SPI3               3        /* SPI instance 3 configured Slave */

static volatile uint32_t cb_spi2_status, cb_spi3_status;

extern ARM_DRIVER_SPI ARM_Driver_SPI_(SPI2);
static ARM_DRIVER_SPI *masterDrv = &ARM_Driver_SPI_(SPI2);

extern ARM_DRIVER_SPI ARM_Driver_SPI_(SPI3);
static ARM_DRIVER_SPI *slaveDrv = &ARM_Driver_SPI_(SPI3);

/**
 * @fn      int32_t pinmux_config(void)
 * @brief   pinmux config for master and slave SPI instances.
 * @note    none.
 * @param   none.
 * @retval  status
 */
static int32_t pinmux_config(void)
{
    int32_t ret = 0;

    /* pinmux configurations for SPI2 pins (using B version pins) */
    ret = pinconf_set(PORT_9, PIN_2, PINMUX_ALTERNATE_FUNCTION_3, PADCTRL_READ_ENABLE);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for SPI2_MISO_PIN\n");
        return ret;
    }
    ret = pinconf_set(PORT_9, PIN_3, PINMUX_ALTERNATE_FUNCTION_4, 0);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for SPI2_MOSI_PIN\n");
        return ret;
    }
    ret = pinconf_set(PORT_9, PIN_4, PINMUX_ALTERNATE_FUNCTION_3, 0);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for SPI2_CLK_PIN\n");
        return ret;
    }
    ret = pinconf_set(PORT_9, PIN_5, PINMUX_ALTERNATE_FUNCTION_3, 0);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for SPI2_SS_PIN\n");
        return ret;
    }

    /* pinmux configurations for SPI3 pins (using B version pins) */
    ret = pinconf_set(PORT_12, PIN_4, PINMUX_ALTERNATE_FUNCTION_2, 0);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for SPI3_MISO_PIN\n");
        return ret;
    }
    ret = pinconf_set(PORT_12, PIN_5, PINMUX_ALTERNATE_FUNCTION_2, PADCTRL_READ_ENABLE);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for SPI3_MOSI_PIN\n");
        return ret;
    }
    ret = pinconf_set(PORT_12, PIN_6, PINMUX_ALTERNATE_FUNCTION_2, PADCTRL_READ_ENABLE);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for SPI3_CLK_PIN\n");
        return ret;
    }
    ret = pinconf_set(PORT_12, PIN_7, PINMUX_ALTERNATE_FUNCTION_3, PADCTRL_READ_ENABLE);
    if (ret)
    {
        printf("ERROR: Failed to configure PINMUX for SPI3_SS_PIN\n");
        return ret;
    }

    return 0;
}

/**
 * @fn      void SPI2_Callback_func (uint32_t event)
 * @brief   SPI2 call back function.
 * @note    none.
 * @param   event: SPI event.
 * @retval  None
 */
static void SPI2_Callback_func (uint32_t event)
{
    if (event == ARM_SPI_EVENT_TRANSFER_COMPLETE)
    {
        cb_spi2_status = 1;
    }
}

/**
 * @fn      void SPI3_Callback_func (uint32_t event)
 * @brief   SPI3 call back function.
 * @note    none.
 * @param   event: SPI event.
 * @retval  None
 */
static void SPI3_Callback_func (uint32_t event)
{
    if (event == ARM_SPI_EVENT_TRANSFER_COMPLETE)
    {
        cb_spi3_status = 1;
    }
}

/**
 * @fn      void MW_Demo_func(void)
 * @brief   Microwire application function.
 * @note    none.
 * @param   none.
 * @retval  None
 */
static void MW_Demo_func(void)
{
    uint32_t master_control, slave_control, baudrate = 1000000;
    int32_t status;
    ARM_SPI_STATUS spi2_status, spi3_status;

    /* Single buffer is used to store both control code and data.
     * Therefore, Buffer width size should be of 4 bytes irrespective of frame format size
     * and control code size (control code size can be configured in RTE_Device.h) */
    uint32_t master_tx_buff[4], slave_rx_buff[4] = {0};
#if !MASTER_TO_SLAVE_TRANSFER
    uint32_t master_rx_buff[4] = {0}, slave_tx_buff[4];
#endif

    printf("Start of MicroWire demo application \n");

    /* SPI instances pinmux initialization */
    status = pinmux_config();
    if (status)
    {
        printf("ERROR: Failed in SPI pinmux and pinpad config \n");
        return;
    }

    /* SPI2 master instance initialization */
    status = masterDrv->Initialize(SPI2_Callback_func);
    if (status != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to initialize SPI2 \n");
        return;
    }

    status = masterDrv->PowerControl(ARM_POWER_FULL);
    if (status != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to power SPI2 \n");
        goto error_spi2_uninitialize;
    }

    master_control = (ARM_SPI_MODE_MASTER | ARM_SPI_SS_MASTER_HW_OUTPUT | ARM_SPI_MICROWIRE | ARM_SPI_DATA_BITS(32));

    status = masterDrv->Control(master_control, baudrate);
    if (status != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to configure SPI2\n");
        goto error_spi2_power_off;
    }

    /* SPI3 slave instance initialization */
    status = slaveDrv->Initialize(SPI3_Callback_func);
    if (status != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to initialize SPI3 \n");
        return;
    }

    status = slaveDrv->PowerControl(ARM_POWER_FULL);
    if (status != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to power SPI3 \n");
        goto error_spi3_uninitialize;
    }

    slave_control = (ARM_SPI_MODE_SLAVE | ARM_SPI_MICROWIRE | ARM_SPI_DATA_BITS(32));

    status = slaveDrv->Control(slave_control, NULL);
    if (status != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to configure SPI3\n");
        goto error_spi3_power_off;
    }

    status = masterDrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
    if (status != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to configure SPI2\n");
        goto error_spi2_power_off;
    }

#if MASTER_TO_SLAVE_TRANSFER

    printf("\nData transfer from master to slave \n");

    master_tx_buff[0] = 0x1111;       /* control word 1 */
    master_tx_buff[1] = 0x11111111;   /* data 1 */
    master_tx_buff[2] = 0x2222;       /* control word 2 */
    master_tx_buff[3] = 0x22222222;   /* data 2 */

    /* The second parameter should be the total number of data to be transferred,
     * excluding the control frame number. */
    status = slaveDrv->Receive(slave_rx_buff, 2);
    if (status != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed SPI3 to configure as rx\n");
        goto error_spi3_power_off;
    }

    /* The second parameter should be the total number of data to be transferred,
     * excluding the control frame number. */
    status = masterDrv->Send(master_tx_buff, 2);
    if (status != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed SPI2 to configure as tx\n");
        goto error_spi2_power_off;
    }

#else

    /* assign control codes to master tx buffer */
    master_tx_buff[0] = 0x1111;       /* control word 1 */
    master_tx_buff[1] = 0x2222;       /* control word 2 */

    /* assign slave tx buffer */
    slave_tx_buff[0] = 0x11111111;   /* data 1 */
    slave_tx_buff[1] = 0x22222222;   /* data 2 */

    printf("\nData transfer from slave to master \n");

    /* The third parameter should be the total number of data to be transferred,
     * excluding the control frame number. */
    status = slaveDrv->Transfer(slave_tx_buff, slave_rx_buff, 2);
    if (status != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed SPI3 to configure as tx\n");
        goto error_spi3_power_off;
    }

    /* The third parameter should be the total number of data to be transferred,
     * excluding the control frame number. */
    status = masterDrv->Transfer(master_tx_buff, master_rx_buff, 2);
    if (status != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed SPI2 to configure as rx\n");
        goto error_spi2_power_off;
    }

#endif

    /* waiting for a transfer complete interrupt */
    while(!(cb_spi2_status && cb_spi3_status));
    printf("Data Transfer is completed\n");

    spi2_status = masterDrv->GetStatus();
    spi3_status = slaveDrv->GetStatus();
    while((spi2_status.busy == 1) || (spi3_status.busy == 1))
    {
        spi2_status = masterDrv->GetStatus();
        spi3_status = slaveDrv->GetStatus();
    }

    masterDrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
    if (status != ARM_DRIVER_OK)
    {
        printf("ERROR: Failed to configure SPI2\n");
        goto error_spi2_power_off;
    }

#if MASTER_TO_SLAVE_TRANSFER
    (memcmp(master_tx_buff, slave_rx_buff, 4) == 0) ? printf("Master tx & Slave rx buffers are same\n") : \
                                                      printf("Master tx & Slave rx rx buffers are different\n");

#else
    (memcmp(master_tx_buff, slave_rx_buff, 2) == 0) ? printf("Master tx & Slave rx buffers are same\n") : \
                                                      printf("Master tx & Slave rx buffers are different\n");
    (memcmp(slave_tx_buff, master_rx_buff, 2) == 0) ? printf("Slave tx & Master rx buffers are same\n") : \
                                                      printf("Slave tx & Master rx buffers are different\n");

#endif

error_spi2_power_off :
    status = masterDrv->PowerControl(ARM_POWER_OFF);
    if (status != ARM_DRIVER_OK)
        printf("Error in SPI2 Power Off \n");

error_spi2_uninitialize :
    status = masterDrv->Uninitialize();
    if (status != ARM_DRIVER_OK)
        printf("Error in SPI2 Uninitialize \n");

error_spi3_power_off :
    status = slaveDrv->PowerControl(ARM_POWER_OFF);
    if (status != ARM_DRIVER_OK)
        printf("Error in SPI3 Power Off \n");

error_spi3_uninitialize :
    status = slaveDrv->Uninitialize();
    if (status != ARM_DRIVER_OK)
        printf("Error in SPI3 Uninitialize \n");

    printf("\nEnd of MicroWire demo application \n");
}

int main(void)
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
    MW_Demo_func();
}

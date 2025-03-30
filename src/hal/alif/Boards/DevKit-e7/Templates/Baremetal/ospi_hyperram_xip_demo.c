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
 * @file     ospi_hyperram_xip_demo.c
 * @author   Silesh C V
 * @email    silesh@alifsemi.com
 * @version  V1.0.0
 * @date     20-Jul-2023
 * @brief    Demo program for the OSPI hyperram XIP library API.
 ******************************************************************************/

#include "ospi_hyperram_xip.h"
#include "pinconf.h"
#include "Driver_GPIO.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


#include <stdio.h>
#include <stddef.h>

#define OSPI_RESET_PORT     LP
#define OSPI_RESET_PIN      6
#define OSPI0_XIP_BASE      0xA0000000

extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(OSPI_RESET_PORT);
ARM_DRIVER_GPIO *GPIODrv = &ARM_Driver_GPIO_(OSPI_RESET_PORT);

#define DDR_DRIVE_EDGE      0
#define RXDS_DELAY          11
#define OSPI_BUS_SPEED      100000000           /* 100MHz */
#define ISSI_WAIT_CYCLES    6

#define HRAM_SIZE_BYTES     (32 * 1024 * 1024)  /* 32MB */

static const ospi_hyperram_xip_config issi_config = {
    .instance       = OSPI_INSTANCE_0,
    .bus_speed      = OSPI_BUS_SPEED,
    .hyperram_init  = NULL, /* No special initialization needed by the hyperram device */
    .ddr_drive_edge = DDR_DRIVE_EDGE,
    .rxds_delay     = RXDS_DELAY,
    .wait_cycles    = ISSI_WAIT_CYCLES,
    .slave_select   = 0,
};

static int32_t pinmux_setup()
{
    int32_t ret;

    ret = pinconf_set(PORT_2, PIN_0, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_1, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_2, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE);
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_3, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_4, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_5, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE);
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_6, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_2, PIN_7, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_3, PIN_0, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_3, PIN_1, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_3, PIN_2, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_1, PIN_6, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE );
    if (ret)
    {
        return -1;
    }

    ret = pinconf_set(PORT_15, PIN_6, PINMUX_ALTERNATE_FUNCTION_0, 0);
    if (ret)
    {
        return -1;
    }

    ret = GPIODrv->Initialize(OSPI_RESET_PIN, NULL);
    if (ret != ARM_DRIVER_OK)
    {
        return -1;
    }

    ret = GPIODrv->PowerControl(OSPI_RESET_PIN, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK)
    {
        return -1;
    }

    ret = GPIODrv->SetDirection(OSPI_RESET_PIN, GPIO_PIN_DIRECTION_OUTPUT);
    if (ret != ARM_DRIVER_OK)
    {
        return -1;
    }

    ret = GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_LOW);
    if (ret != ARM_DRIVER_OK)
    {
        return -1;
    }

    ret = GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_HIGH);
    if (ret != ARM_DRIVER_OK)
    {
        return -1;
    }

    return 0;
}

int main(void)
{
    uint8_t *const ptr = (uint8_t *) OSPI0_XIP_BASE;
    uint32_t total_errors = 0;

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

    if (pinmux_setup() < 0)
    {
        printf("Pinmux/GPIO setup failed\n");
        goto error_exit;
    }

    if (ospi_hyperram_xip_init(&issi_config) < 0)
    {
        printf("Hyperram XIP init failed\n");
        goto error_exit;
    }

    printf("Writing data to the XIP region:\n");

    for (int i = 0; i < HRAM_SIZE_BYTES; i++)
    {
        ptr[i] = (i % 255);
    }

    printf("Reading back:\n");

    for (int i = 0; i < HRAM_SIZE_BYTES; i++)
    {
        if (ptr[i] != (i % 255))
        {
            printf("Data error at addr %x, got %x, expected %x\n", i, ptr[i], i % 255);
            total_errors++;
        }
    }

    printf("Done, total errors = %d\n", total_errors);

error_exit:

    while(1);

    return 0;
}

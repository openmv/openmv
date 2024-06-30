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
 * @file     LPTimer_Baremetal.c
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     22-March-2022
 * @brief    baremetal demo application for lptimer.
 *           - Configuring the lptimer channel 0 for 5 seconds.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "Driver_LPTIMER.h"


#include "RTE_Components.h"
#include CMSIS_device_header
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

extern ARM_DRIVER_LPTIMER DRIVER_LPTIMER0;
ARM_DRIVER_LPTIMER *ptrDrv = &DRIVER_LPTIMER0;

#define LPTIMER_CHANNEL_0    0

volatile uint32_t Cb_status  =  0;

static void lptimer_cb_fun (uint8_t event)
{
    if (event == ARM_LPTIMER_EVENT_UNDERFLOW)
    {
        Cb_status++;
    }
}

static void lptimer_Thread ()
{
    /* Configuring the lptimer channel 0 for 5 seconds
     *Clock Source is depends on RTE_LPTIMER_CHANNEL_CLK_SRC in RTE_Device.h
     *RTE_LPTIMER_CHANNEL_CLK_SRC = 0 : 32.768KHz freq (Default)
     *RTE_LPTIMER_CHANNEL_CLK_SRC = 1 : 128KHz freq.
     *
     * Selected clock frequency (F)= 32.768KHz
     *
     * time for 1 count T = 1/F = 1/(32.768*10^3) = 30.51 * 10^-6
     *
     * To increment timer by 1 count, takes 30.51 micro sec
     *
     * So count for 5sec = 5/(30.51 *(10^-6)) = 163880
     *
     * DEC = 163880
     * HEX = 0x28028
    */

    /* Timer channel configured 5 sec */

    int32_t ret;
    uint32_t count = 0x28028;
    uint8_t channel = LPTIMER_CHANNEL_0;

    ret = ptrDrv->Initialize (channel, lptimer_cb_fun);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: channel '%d'failed to initialize\r\n", channel);
        return;
    }

    ret = ptrDrv->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: channel '%d'failed to power up\r\n", channel);
        goto error_uninstall;
    }

    /**< Loading the counter value >*/
    ret = ptrDrv->Control (channel, ARM_LPTIMER_SET_COUNT1, &count);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: channel '%d'failed to load count\r\n", channel);
        goto error_poweroff;
    }

    printf("demo application: lptimer channel '%d' configured for 5 sec \r\n\n", channel);

    ret = ptrDrv->Start (channel);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: failed to start channel '%d' timer\n", channel);
        goto error_poweroff;
    }
    else
    {
        printf("timer started\r\n");
    }

    /* delay for 6sec */
    for(uint32_t count = 0; count < 60; count++)
        sys_busy_loop_us(100000);

    if (Cb_status)
    {
        printf("5 seconds timer expired \r\n");
        Cb_status = 0;
    }
    else
    {

        printf("Error: timer is failed \r\n");
    }

    ret = ptrDrv->Stop(channel);
    if(ret != ARM_DRIVER_OK)
    {
        printf("ERROR: failed to stop channel %d\n", channel);
    }
    else
    {
        printf("timer stopped\r\n\n");
    }

error_poweroff:

    ret = ptrDrv->PowerControl(channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: failed to power off channel '%d'\n", channel);
    }

error_uninstall:

    ret = ptrDrv->Uninitialize(channel);
    if (ret != ARM_DRIVER_OK)
    {
        printf("ERROR: failed to un-initialize channel %d\n", channel);
    }

    printf("demo application: completed \r\n");
}

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
    lptimer_Thread();
    return 0;
}

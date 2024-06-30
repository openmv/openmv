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
 * @file     RTC_Baremetal.c
 * @author   Mnaoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     12-April-2022
 * @brief    Baremetal testApp to verify RTC Driver.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* System Includes */
#include <stdio.h>
#include "Driver_RTC.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


/* RTC Driver instance 0 */
extern ARM_DRIVER_RTC Driver_RTC0;
static ARM_DRIVER_RTC *RTCdrv = &Driver_RTC0;


void rtc_demo_Thread_entry();

#define CB_EVENT_SET     1
#define CB_EVENT_CLEAR   0

volatile uint32_t cb_status = 0;

/**
  \fn           void alarm_callback(uint32_t event)
  \brief        rtc alarm callback
  \return       none
*/
static void rtc_callback(uint32_t event)
{
    if (event & ARM_RTC_EVENT_ALARM_TRIGGER)
    {
        /* Received RTC Alarm. */
        cb_status = CB_EVENT_SET;
    }
}

/**
  \fn           void rtc_demo_Thread_entry()
  \brief        RTC demo Thread:
                    This thread initializes the RTC. And then in a loop,
                    reads the current counter value and sets up an alarm to ring in the future.
                    The alarms are setup with increasing timeouts starting with 5 counts and
                    ending with 25 counts.
  \param[in]    none
  \return       none
*/
void rtc_demo_Thread_entry()
{
    uint32_t  val      = 0;
    uint32_t  iter     = 1;
    uint32_t  timeout  = 5;
    int32_t   ret      = 0;
    ARM_DRIVER_VERSION version;
    ARM_RTC_CAPABILITIES capabilities;

    printf("\r\n >>> RTC demo thread is starting up!!! <<< \r\n");

    version = RTCdrv->GetVersion();
    printf("\r\n RTC version api:%X driver:%X...\r\n",version.api, version.drv);

    capabilities = RTCdrv->GetCapabilities();
    if(!capabilities.alarm){
        printf("\r\n Error: RTC alarm capability is not available.\n");
        return;
    }

    /* Initialize RTC driver */
    ret = RTCdrv->Initialize(rtc_callback);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: RTC init failed\n");
        return;
    }

    /* Enable the power for RTC */
    ret = RTCdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: RTC Power up failed\n");
        goto error_uninitialize;
    }

    while (iter <= 5)
    {
        ret = RTCdrv->ReadCounter(&val);
        if(ret != ARM_DRIVER_OK){
            printf("\r\n Error: RTC read failed\n");
            goto error_poweroff;
        }

        printf("\r\n Setting alarm after %d counts \r\n", timeout);
        ret = RTCdrv->Control(ARM_RTC_SET_ALARM, val + timeout);
        if(ret != ARM_DRIVER_OK){
            printf("\r\n Error: RTC Could not set alarm\n");
            goto error_poweroff;
        }

        /* wait till alarm event comes in isr callback */
        while (1)
        {
            if (cb_status)
            {
                printf("\r\n Received alarm \r\n");
                cb_status = CB_EVENT_CLEAR;
                break;
            }
        }
        timeout += 5;
        iter ++;
    }

error_poweroff:

        /* Power off RTC peripheral */
        ret = RTCdrv->PowerControl(ARM_POWER_OFF);
        if(ret != ARM_DRIVER_OK){
            printf("\r\n Error: RTC Power OFF failed.\r\n");
        }

error_uninitialize:

        /* Un-initialize RTC driver */
        ret = RTCdrv->Uninitialize();
        if(ret != ARM_DRIVER_OK){
            printf("\r\n Error: RTC Uninitialize failed.\r\n");
        }

    printf("\r\n XXX RTC demo thread is exiting XXX...\r\n");
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
    rtc_demo_Thread_entry();
    return 0;
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/

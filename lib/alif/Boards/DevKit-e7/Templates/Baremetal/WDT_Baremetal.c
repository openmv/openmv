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
 * @file     WDT_Baremetal.c
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     26-April-2022
 * @brief    TestApp to verify Wachdog Driver.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* Project Includes */
#include <stdio.h>
#include "Driver_WDT.h"

#include <RTE_Components.h>
#include CMSIS_device_header
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


/* watchdog Driver instance 0 */
extern ARM_DRIVER_WDT Driver_WDT0;
static ARM_DRIVER_WDT *WDTdrv = &Driver_WDT0;

void watchdog_demo_entry();


void NMI_Handler(void)
{
	printf("\r\n NMI_Handler: Received Interrupt from Watchdog! \r\n");
	while(1);
}

/**
  \fn          void watchdog_demo_entry()
  \brief       TestApp to verify watchdog peripheral,
               This demo thread does:
                 - initialize watchdog with timeout value
                 - start the watchdog;
                 - feed  the watchdog;
                 - stop feeding after n iterations;
                 - wait for system to RESET on timeout
  \param[in]   none
  \return      none
*/
void watchdog_demo_entry()
{
	uint32_t wdog_timeout_msec = 0;   /* watchdog timeout value in msec        */
	uint32_t time_to_reset = 0;       /* watchdog remaining time before reset. */
	uint32_t iter = 3;
	int32_t  ret = 0;
	ARM_DRIVER_VERSION version;

	printf("\r\n >>> watchdog demo starting up!!! <<< \r\n");

	version = WDTdrv->GetVersion();
	printf("\r\n watchdog version api:%X driver:%X...\r\n",version.api, version.drv);

	/* Watchdog timeout is set to 5000 msec (5 sec). */
	wdog_timeout_msec = 5000;

	/* Initialize watchdog driver */
	ret = WDTdrv->Initialize(wdog_timeout_msec);
	if(ret != ARM_DRIVER_OK){
		printf("\r\n Error: watchdog init failed\n");
		return;
	}

	/* Power up watchdog peripheral */
	ret = WDTdrv->PowerControl(ARM_POWER_FULL);
	if(ret != ARM_DRIVER_OK){
		printf("\r\n Error: watchdog Power up failed\n");
		goto error_uninitialize;
	}

	/* Watchdog initialize will lock the timer, unlock it to change the register value. */
	ret = WDTdrv->Control(ARM_WATCHDOG_UNLOCK, 0);
	if(ret != ARM_DRIVER_OK){
		printf("\r\n Error: watchdog unlock failed\n");
		goto error_poweroff;
	}

	/* Start the watchDog Timer. */
	ret = WDTdrv->Start();
	if(ret != ARM_DRIVER_OK){
		printf("\r\n Error: watchdog start failed\n");
		goto error_stop;
	}

	while(iter--)
	{
		/* Delay for 3 sec. */
        for(uint32_t count = 0; count < 30; count++)
            sys_busy_loop_us(100000);

		/* Get watchdog remaining time before reset. */
		ret = WDTdrv->GetRemainingTime(&time_to_reset);
		if(ret != ARM_DRIVER_OK){
			printf("\r\n Error: watchdog get remaining time failed\n");
			goto error_stop;
		}

		printf("\r\n Feed the WatchDog: %d...\r\n",iter);
		ret = WDTdrv->Feed();
		if(ret != ARM_DRIVER_OK){
			printf("\r\n Error: watchdog feed failed\n");
			goto error_stop;
		}
	}

	printf("\r\n now stop feeding to the watchdog, system will RESET on timeout. \r\n");
	while(1);


error_stop:
	/* First Unlock and then Stop watchdog peripheral. */
	ret = WDTdrv->Control(ARM_WATCHDOG_UNLOCK, 0);
	if(ret != ARM_DRIVER_OK){
		printf("\r\n Error: watchdog unlock failed\n");
	}

	/* Stop watchdog peripheral */
	ret = WDTdrv->Stop();
	if(ret != ARM_DRIVER_OK){
		printf("\r\n Error: watchdog Stop failed.\r\n");
	}

error_poweroff:
	/* Power off watchdog peripheral */
	ret = WDTdrv->PowerControl(ARM_POWER_OFF);
	if(ret != ARM_DRIVER_OK){
		printf("\r\n Error: watchdog Power OFF failed.\r\n");
	}

error_uninitialize:
	/* Un-initialize watchdog driver */
	ret = WDTdrv->Uninitialize();
	if(ret != ARM_DRIVER_OK){
		printf("\r\n Error: watchdog Uninitialize failed.\r\n");
	}

	printf("\r\n XXX watchdog demo thread exiting XXX...\r\n");
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

    watchdog_demo_entry();
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/

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
 * @file     rtc_testApp.c
 * @author   Sudarshan Iyengar
 * @email    sudarshan.iyengar@alifsemi.com
 * @version  V1.0.0
 * @date     18-Sep-2021
 * @brief    TestApp to verify RTC Driver using FreeRTOS as an operating system.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* Includes ----------------------------------------------------------------- */

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
/*RTOS Includes */
#include "RTE_Components.h"
#include CMSIS_device_header

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


/* Project Includes */

/* include for RTC Driver */
#include "Driver_RTC.h"

/*Define for FreeRTOS*/
#define STACK_SIZE     1024
#define TIMER_SERVICE_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH // 512
#define IDLE_TASK_STACK_SIZE          configMINIMAL_STACK_SIZE // 1024

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;


/****************************** FreeRTOS functions **********************/

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
      StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
   *ppxIdleTaskTCBBuffer = &IdleTcb;
   *ppxIdleTaskStackBuffer = IdleStack;
   *pulIdleTaskStackSize = IDLE_TASK_STACK_SIZE;
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
   (void) pxTask;

   for (;;);
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
      StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
   *ppxTimerTaskTCBBuffer = &TimerTcb;
   *ppxTimerTaskStackBuffer = TimerStack;
   *pulTimerTaskStackSize = TIMER_SERVICE_TASK_STACK_SIZE;
}

void vApplicationIdleHook(void)
{
   for (;;);
}

/*****************Only for FreeRTOS use *************************/
/* RTC Driver instance 0 */
extern ARM_DRIVER_RTC Driver_RTC0;
static ARM_DRIVER_RTC *RTCdrv = &Driver_RTC0;

void rtc_demo_Thread(void *pvParameters);

/* Define the FreeRTOS object control blocks...  */
#define DEMO_STACK_SIZE                 1024
#define RTC_ALARM_EVENT                 0x01

/* Thread id of thread */
TaskHandle_t rtc_xHandle;

uint32_t    event_flags_rtc;

/**
  \fn           void alarm_callback(uint32_t event)
  \brief        rtc alarm callback
  \return       none
*/
static void alarm_callback(uint32_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event & ARM_RTC_EVENT_ALARM_TRIGGER)
    {
        /* Received RTC Alarm: Wake-up Thread. */
        xTaskNotifyFromISR(rtc_xHandle, RTC_ALARM_EVENT,eSetBits, &xHigherPriorityTaskWoken);

        if (xResult == pdTRUE)        {    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );    }
    }
}

/**
  \fn           void rtc_demo_Thread_entry(void *pvParameters)
  \brief       RTC demo Thread:
            This thread initializes the RTC. And then in a loop,
            reads the current counter value and sets up an alarm to ring in the future.
            The alarms are setup with increasing timeouts starting with 5 counts and
            ending with 25 counts.
  \param[in]        thread_input : thread input
  \return        none
*/
void rtc_demo_Thread(void *pvParameters)
{
    uint32_t  val      = 0;
    uint32_t  iter     = 5;
    uint32_t  timeout  = 5;
    int       ret      = 0;
    BaseType_t xReturned;

    ARM_DRIVER_VERSION version;
    ARM_RTC_CAPABILITIES capabilities;

    printf("\r\n >>> RTC demo thread starting up!!! <<< \r\n");

    version = RTCdrv->GetVersion();
    printf("\r\n RTC version api:%X driver:%X...\r\n",version.api, version.drv);

    capabilities = RTCdrv->GetCapabilities();
    if(!capabilities.alarm)
    {
        printf("\r\n Error: RTC alarm capability is not available.\n");
        return;
    }

    /* Initialize RTC driver */
    ret = RTCdrv->Initialize(alarm_callback);
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

    while (iter--)
    {
        ret = RTCdrv->ReadCounter(&val);
        if(ret != ARM_DRIVER_OK){
            printf("\r\n Error: RTC read failed\n");
            goto error_poweroff;
        }

        printf("\r\n Setting alarm after %d counts into the future: \r\n", timeout);
        ret = RTCdrv->Control(ARM_RTC_SET_ALARM, val + timeout);
        if(ret != ARM_DRIVER_OK){
            printf("\r\n Error: RTC Could not set alarm\n");
            goto error_poweroff;
        }

        /* wait till alarm event comes in isr callback */
        xReturned = xTaskNotifyWait(NULL,RTC_ALARM_EVENT,NULL, portMAX_DELAY);
        if (xReturned != pdTRUE) {
            printf("Error: RTC tx_event_flags_get\n");
            goto error_poweroff;
        }

        printf("\r\n Received alarm \r\n");
        timeout += 5;
    }

error_poweroff:

    /* Power off RTC peripheral */
    ret = RTCdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: RTC Power OFF failed.\r\n");
    }

error_uninitialize:

    /* Un-initialize RTC driver */
    ret = RTCdrv->Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: RTC Uninitialize failed.\r\n");
    }

    printf("\r\n XXX RTC demo thread exiting XXX...\r\n");

    /* thread delete */
    vTaskDelete( NULL );
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
int main( void )
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
   /* System Initialization */
   SystemCoreClockUpdate();
   /* Create application main thread */
   BaseType_t xReturned = xTaskCreate(rtc_demo_Thread, "rtc_demo_Thread", 256, NULL,configMAX_PRIORITIES-1, &rtc_xHandle);
   if (xReturned != pdPASS)
   {
      vTaskDelete(rtc_xHandle);
      return -1;
   }

   /* Start thread execution */
   vTaskStartScheduler();

}
/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/

/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     LPTimer_testApp.c
 * @author   Sudarshan Iyengar
 * @email    sudarshan.iyengar@alifsemi.com
 * @version  V1.0.0
 * @date     28-August-2021
 * @brief    demo application for lptimer.
 *           - Configuring the lptimer channel 0 for 5 seconds.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
/* System Includes */
#include <stdio.h>
#include <stdlib.h>
/* include for LPTIMER Driver */
#include "Driver_LPTIMER.h"
/*RTOS Includes */
#include "RTE_Components.h"
#include CMSIS_device_header

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


/*Define for the FreeRTOS objects*/
#define LPTIMER_CALLBACK_EVENT     0x01
#define LPTIMER_CHANNEL_0          0    /* lptimer have 0-3 channels, using channel zero for demo app */
#define LPTIMER_EVENT_WAIT_TIME    pdMS_TO_TICKS(6000) /* interrupt wait time:6 seconds */


/*Define for FreeRTOS*/
#define STACK_SIZE     1024
#define TIMER_SERVICE_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH // 512
#define IDLE_TASK_STACK_SIZE          configMINIMAL_STACK_SIZE // 1024

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

/* Thread id of thread */
TaskHandle_t lptimer_xHandle;


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


void lptimer_cb_fun (uint8_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event & ARM_LPTIMER_EVENT_UNDERFLOW) {
        xTaskNotifyFromISR(lptimer_xHandle,LPTIMER_CALLBACK_EVENT,eSetBits, &xHigherPriorityTaskWoken);

        if (xResult == pdTRUE)        {    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );    }
    }
}

void lptimer_Thread (void *pvParameters)
{
    extern ARM_DRIVER_LPTIMER DRIVER_LPTIMER0;
    ARM_DRIVER_LPTIMER *ptrDrv = &DRIVER_LPTIMER0;

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
    uint32_t count = 0x28028;
    uint8_t channel = LPTIMER_CHANNEL_0;
    BaseType_t xReturned;
    uint32_t ret;

    ret = ptrDrv->Initialize (channel, lptimer_cb_fun);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: channel '%d'failed to initialize\r\n", channel);
        return;
    }

    ret = ptrDrv->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: channel '%d'failed to power up\r\n", channel);
        goto error_uninstall;
    }

    /**< Loading the counter value >*/
    ret = ptrDrv->Control (channel, ARM_LPTIMER_SET_COUNT1, &count);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: channel '%d'failed to load count\r\n", channel);
        goto error_poweroff;
    }

    printf("demo application: lptimer channel '%d'configured for 5 sec \r\n\n", channel);

    ret = ptrDrv->Start (channel);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: failed to start channel '%d' timer\n", channel);
        goto error_poweroff;
    } else {
        printf("timer started\r\n");
    }

    xReturned = xTaskNotifyWait(NULL,LPTIMER_CALLBACK_EVENT,NULL, LPTIMER_EVENT_WAIT_TIME);
    if (xReturned != pdTRUE )
    {
        printf("\n\r Task Wait Time out expired \n\r");
        goto error_poweroff;
    }

    printf("5 sec timer expired \r\n");

    ret = ptrDrv->Stop(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("ERROR: failed to stop channel %d\n", channel);
    } else {
        printf("timer stopped\r\n\n");
    }

error_poweroff:

    ret = ptrDrv->PowerControl(channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: failed to power off channel '%d'\n", channel);
    }

error_uninstall:

	ret = ptrDrv->Uninitialize(channel);
	if (ret != ARM_DRIVER_OK) {
        printf("ERROR: failed to un-initialize channel %d\n", channel);
    }

    printf("demo application: completed \r\n");

    /* thread delete  */
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
   BaseType_t xReturned = xTaskCreate(lptimer_Thread, "lpTimer_Thread", 512, NULL,configMAX_PRIORITIES-1, &lptimer_xHandle);
   if (xReturned != pdPASS)
   {

       vTaskDelete(lptimer_xHandle);
       return -1;
    }

    /* Start thread execution */
    vTaskStartScheduler();

}

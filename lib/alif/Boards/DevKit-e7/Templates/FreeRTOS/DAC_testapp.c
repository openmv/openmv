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
 * @file     DAC_testapp.c
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     06-June-2023
 * @brief    TestApp to verify DAC(Digital to Analog converter) interface using
 *           FreeRTOS as an operating system.
 *           -As DAC is 12 bit resolution, If the input value is maximum than the
                maximum DAC input value(0xFFF)then the input value will be set to
                DAC maximum input value.
             -And If the input value is equal to maximum dac input value then
                input value will be set to 0.
             -If the input value is not greater than the maximum dac input value
               then input value will be incremented by 1000.

             Hardware Setup :
              -when the application uses DAC0 channel,then in Engineering board
               connect DAC0 to P2_2 pin,according to DAC input the output will be
               observed in P2_2 pin through the logic analyzer.

              -And when the application uses DAC1 channel,then in Engineering board
               connect DAC1 to P2_3 pin,according to DAC input the output will be
               observed in P2_3 pin through the logic analyzer.
 ******************************************************************************/
/* System Includes */
#include <stdio.h>

#include "RTE_Components.h"
#include CMSIS_device_header

/* Project Includes */
/* include for DAC Driver */
#include "Driver_DAC.h"
#include "pinconf.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


/* DAC Driver instance */
extern ARM_DRIVER_DAC Driver_DAC0;
static ARM_DRIVER_DAC *DACdrv = &Driver_DAC0;

/*Define for FreeRTOS*/
#define STACK_SIZE           1024
#define TIMER_SERVICE_TASK_STACK_SIZE   configTIMER_TASK_STACK_DEPTH
#define IDLE_TASK_STACK_SIZE            configMINIMAL_STACK_SIZE

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

TaskHandle_t dac_xHandle = NULL;

/* DAC maximum resolution is 12-bit */
#define DAC_MAX_INPUT_VALUE   (0xFFF)

#define ERROR    -1
#define SUCCESS   0

/**
 * @fn          void dac_pinmux_config(void)
 * @brief       Initialize the pinmux for DAC output
 * @return      status
*/
int32_t dac_pinmux_config(void)
{
    int32_t status;

    /* Configure DAC0 output */
    status = pinconf_set(PORT_2, PIN_2, PINMUX_ALTERNATE_FUNCTION_7, PADCTRL_OUTPUT_DRIVE_STRENGTH_2MA);
    if(status)
        return ERROR;

    /* Configure DAC1 output */
    status = pinconf_set(PORT_2, PIN_3, PINMUX_ALTERNATE_FUNCTION_7, PADCTRL_OUTPUT_DRIVE_STRENGTH_2MA);
    if(status)
        return ERROR;

    return SUCCESS;
}

/**
 @fn           void dac_demo_Thread_entry()
 @brief        DAC demo :
               This initializes the DAC. And then in a loop,
               according to the input value, the output will change.
               If the input value is maximum than the maximum DAC input
               value then the input value will be set to DAC maximum input value.
               And If the input value is equal to maximum dac input value then
               input value will be set to 0.If the input value is not greater
               than the maximum dac input value then input value will be incremented by 1000.
 @return       none
*/
void dac_demo_Thread_entry()
{
    uint32_t input_value = 0;
    int32_t  ret         = 0;
    ARM_DRIVER_VERSION version;

    printf("\r\n >>> DAC demo starting up!!! <<< \r\n");

    /* Configure the DAC output pins */
    if(dac_pinmux_config())
    {
        printf("DAC pinmux failed\n");
        return;
    }

    version = DACdrv->GetVersion();
    printf("\r\n DAC version api:%X driver:%X...\r\n",version.api, version.drv);

    /* Initialize DAC driver */
    ret = DACdrv->Initialize();
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: DAC init failed\n");
        return;
    }

    /* Enable the power for DAC */
    ret = DACdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: DAC Power up failed\n");
        goto error_uninitialize;
    }

    /* Set DAC IBAIS output current */
    ret = DACdrv->Control(ARM_DAC_SELECT_IBIAS_OUTPUT, ARM_DAC_1500UA_OUT_CUR);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: Setting DAC output current failed failed\n");
        goto error_uninitialize;
    }

        /* Set DAC capacitance  */
    ret = DACdrv->Control(ARM_DAC_CAPACITANCE_HP_MODE, ARM_DAC_8PF_CAPACITANCE);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: Setting DAC capacitance failed\n");
        goto error_uninitialize;
    }

    /* start dac */
    ret = DACdrv->Start();
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: DAC Start failed\n");
        goto error_uninitialize;
    }

    input_value = 0;

    while(1)
    {
        /* set dac input */
        ret = DACdrv->SetInput(input_value);
        if(ret != ARM_DRIVER_OK){
            printf("\r\n Error: DAC Set Input failed\n");
            goto error_stop;
        }

        /* Sleep for n micro second */
        sys_busy_loop_us(1000);

        /* If the input value is equal to maximum dac input value then input
           value will be set to 0 */
        if(input_value == DAC_MAX_INPUT_VALUE)
        {
            input_value = 0;
        }

        /* If the input value is not greater than the maximum dac input value then input
           value will be incremented by 1000 */
        else
        {
            input_value += 1000;
        }

        /* If the input value is maximum than the maximum DAC input value then the input
           value will be set to DAC maximum input value */
        if(input_value > DAC_MAX_INPUT_VALUE)
        {
            input_value = DAC_MAX_INPUT_VALUE;
        }

    }

error_stop :

    /* Stop the DAC driver */
    ret = DACdrv->Stop();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: DAC Stop failed.\r\n");
    }

error_poweroff:

    /* Power off DAC peripheral */
    ret = DACdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: DAC Power OFF failed.\r\n");
    }

error_uninitialize:

    /* Un-initialize DAC driver */
    ret = DACdrv->Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: DAC Uninitialize failed.\r\n");
    }

    printf("\r\n XXX DAC demo exiting XXX...\r\n");
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

    /* System Initialization */
    SystemCoreClockUpdate();

    /* Create application main thread */
    BaseType_t xReturned = xTaskCreate(dac_demo_Thread_entry, "DACFreertos", 256, NULL,configMAX_PRIORITIES-1, &dac_xHandle);
    if (xReturned != pdPASS)
    {
        vTaskDelete(dac_xHandle);
        return -1;
    }

    /* Start thread execution */
    vTaskStartScheduler();
}

/********************** (c) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/

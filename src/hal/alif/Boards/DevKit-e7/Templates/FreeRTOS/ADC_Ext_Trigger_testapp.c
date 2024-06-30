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
 * @file     : ADC_Ext_Trigger_testapp.c
 * @author   : Prabhakar kumar
 * @email    : prabhakar.kumar@alifsemi.com
 * @version  : V1.0.0
 * @date     : 04-Sept-2023
 * @brief    : Testapp demo application code for testing the external trigger
 *             feature for the ADC12 & ADC24
 *              - Generating the pulse from the Utimer channel 0 Driver A for starting
 *                the ADC conversion.
 *              - Input in analog signal corresponding output is digital value.
 *              - Converted digital value are stored in user provided memory
 *                address.
 *              ADC configurations for Demo testApp:
 *              Single channel scan(Default scan)
 *              - GPIO pin P1_4 are connected to Regulated DC Power supply.
 *                DC Power supply:
 *                - +ve connected to P1_4 (ADC2 channel 0) at 1.0V
 *                - -ve connect to GND.
 *
 * @Note     : From RTE_Device.h file set following
 *             - RTE_UTIMER_CHANNEL0_DRIVER_A set marco to 1.
 *             - RTE_UTIMER_CHANNEL0_DRV_A_OP_AT_MATCH_COUNTset to 3.
 ******************************************************************************/

/* Include */
#include <stdio.h>
#include "string.h"
#include "system_utils.h"

/* Project include */
#include "Driver_UTIMER.h"
#include "Driver_ADC.h"
#include "pinconf.h"

/* Rtos include */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "se_services_port.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

/* UTIMER0 Driver instance */
extern ARM_DRIVER_UTIMER DRIVER_UTIMER0;
ARM_DRIVER_UTIMER *pxUTIMERDrv = &DRIVER_UTIMER0;

/* Macro for ADC12 and ADC24 */
#define ADC12    1
#define ADC24    0

/* For ADC12 use ADC_INSTANCE ADC12  */
/* For ADC24 use ADC_INSTANCE ADC24  */

#define ADC_INSTANCE         ADC12
//#define ADC_INSTANCE         ADC24

#if (ADC_INSTANCE == ADC12)
/* Instance for ADC12 */
extern ARM_DRIVER_ADC Driver_ADC122;
static ARM_DRIVER_ADC *pxADCDrv = &Driver_ADC122;
#else
/* Instance for ADC24 */
extern ARM_DRIVER_ADC Driver_ADC24;
static ARM_DRIVER_ADC *pxADCDrv = &Driver_ADC24;
#endif

#define NUM_CHANNELS                8
#define NUM_TEST_SAMPLES            3
#define NUM_PULSE_GENERATE          3

/* Flags to set */
#define ADC_INT_AVG_SAMPLE_RDY      0x01
#define UTIMER_COMPARE_A            0X02
#define UTIMER_TASK_START           0X04

/* Adc Function dec */
static void prvAdcExtTriggerDemo(void *pvParameters);

/*Define for FreeRTOS*/
#define TIMER_SERVICE_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH
#define IDLE_TASK_STACK_SIZE          configMINIMAL_STACK_SIZE

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

TaskHandle_t utimer_xHandle;
TaskHandle_t adc_xHandle;

/****************************** FreeRTOS functions **********************/

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
      StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
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

/* Demo purpose adc_sample*/
uint32_t ulAdcSample[NUM_CHANNELS];

volatile uint8_t ucNumSamples = 0;

volatile uint8_t ucNumPulses  = 0;

/*
 * @func   : static void prvAdcConversionCallBack(uint32_t ulEvent,
 *                                                uint8_t  ucChannel,
 *                                                uint32_t ulSampleOutput)
 * @brief  : adc conversion isr callback
 * @return : NONE
*/
static void prvAdcConversionCallBack(uint32_t ulEvent,
                                     uint8_t  ucChannel,
                                     uint32_t ulSampleOutput)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (ulEvent & ARM_ADC_EVENT_CONVERSION_COMPLETE)
    {
        ucNumSamples += 1;

        /* Store the value for the respected channels */
        ulAdcSample[ucChannel] = ulSampleOutput;

        if (ucNumSamples == NUM_TEST_SAMPLES)
        {
            /* conversion completed */
            xResult = xTaskNotifyFromISR(adc_xHandle,ADC_INT_AVG_SAMPLE_RDY,
                                         eSetBits, &xHigherPriorityTaskWoken);
            if (xResult == pdTRUE)
            {
                portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
            }
        }
    }
}

/**
 * @function    void prvUtimerCompareModeCallBack(uint8_t ucEvent)
 * @brief       utimer compare mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void prvUtimerCompareModeCallBack(uint8_t ucEvent)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (ucEvent == ARM_UTIMER_EVENT_CAPTURE_A)
    {
        if (ucNumPulses++ == NUM_PULSE_GENERATE)
        {
            /* conversion completed */
            xResult = xTaskNotifyFromISR(utimer_xHandle,UTIMER_COMPARE_A,
                                         eSetBits, &xHigherPriorityTaskWoken);
            if (xResult == pdTRUE)
            {
                portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
            }
        }
    }
}

/**
 * @function    void prvUtimerCompareModeDemo(void *pvParameters)
 * @brief       utimer compare mode application
 * @note        none
 * @param       none
 * @retval      none
 */
static void prvUtimerCompareModeDemo(void *pvParameters)
{
    int32_t    lRet      = 0;
    uint8_t    ucChannel = 0;
    uint32_t   ulCountArray[3];
    BaseType_t xReturned;

    /*
     * utimer channel 0 is configured for utimer compare mode (driver A).
     */
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 1 sec = 1/(0.0025*(10^-6)) = 400000000
     * DEC = 400000000
     * HEX = 0x17D78400
     *
     * So count for 500ms = (500*(10^-3)/(0.0025*(10^-6)) = 200000000
     * DEC = 200000000
     * HEX = 0xBEBC200
     */
    ulCountArray[0] =  0x000000000;       /*< initial counter value >*/
    ulCountArray[1] =  0x17D78400;        /*< over flow count value >*/
    ulCountArray[2] =  0xBEBC200;         /*< compare a/b value>*/

    xReturned = xTaskNotifyWait(NULL,UTIMER_TASK_START,
                                NULL, portMAX_DELAY);
    if (xReturned != pdTRUE )
    {
        printf("\n\r Task Wait Time out expired \n\r");
        while(1);
    }

    lRet = pxUTIMERDrv->Initialize (ucChannel, prvUtimerCompareModeCallBack);
    if (lRet != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", ucChannel);
        return;
    }

    lRet = pxUTIMERDrv->PowerControl (ucChannel, ARM_POWER_FULL);
    if (lRet != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", ucChannel);
        goto error_compare_mode_uninstall;
    }

    lRet = pxUTIMERDrv->ConfigCounter (ucChannel, ARM_UTIMER_MODE_COMPARING, ARM_UTIMER_COUNTER_UP);
    if (lRet != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", ucChannel);
        goto error_compare_mode_poweroff;
    }

    lRet = pxUTIMERDrv->SetCount (ucChannel, ARM_UTIMER_CNTR, ulCountArray[0]);
    if (lRet != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", ucChannel);
        goto error_compare_mode_poweroff;
    }

    lRet = pxUTIMERDrv->SetCount (ucChannel, ARM_UTIMER_CNTR_PTR, ulCountArray[1]);
    if (lRet != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", ucChannel);
        goto error_compare_mode_poweroff;
    }

    lRet = pxUTIMERDrv->SetCount (ucChannel, ARM_UTIMER_COMPARE_A, ulCountArray[2]);
    if (lRet != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", ucChannel);
        goto error_compare_mode_poweroff;
    }

    lRet = pxUTIMERDrv->Start(ucChannel);
    if (lRet != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", ucChannel);
        goto error_compare_mode_poweroff;
    }

    /* Waiting for the callback */
    xReturned = xTaskNotifyWait(NULL,UTIMER_COMPARE_A,
                                NULL, portMAX_DELAY);
    if (xReturned != pdTRUE )
    {
        printf("\n\r Task Wait Time out expired \n\r");
        while(1);
    }

    lRet = pxUTIMERDrv->Stop (ucChannel, ARM_UTIMER_COUNTER_CLEAR);
    if (lRet != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", ucChannel);
    }

error_compare_mode_poweroff:

    lRet = pxUTIMERDrv->PowerControl (ucChannel, ARM_POWER_OFF);
    if (lRet != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", ucChannel);
    }

error_compare_mode_uninstall:

    lRet = pxUTIMERDrv->Uninitialize (ucChannel);
    if(lRet != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", ucChannel);
    }

    /* Task delete */
    vTaskDelete(utimer_xHandle);
}

/**
 * @function    static void prvAdcExtTriggerDemo(void *pvParameters)
 * @brief       adc external trigger application, it start conversion
 *              of the input signal once the pulse is generated by
 *              the UTIMER or QEC.
 * @note        none
 * @param       none
 * @retval      none
 */
static void prvAdcExtTriggerDemo(void *pvParameters)
{
    int32_t    lRet               = 0;
    uint32_t   ulErrorCode        = SERVICES_REQ_SUCCESS;
    uint32_t   ulServiceErrorCode;
    BaseType_t xReturned;
    BaseType_t xResult = pdFALSE;
    ARM_DRIVER_VERSION xVersion;

    /* Initialize the SE services */
    se_services_port_init();

    /* enable the 160 MHz clock */
    ulErrorCode = SERVICES_clocks_enable_clock(se_services_s_handle,
                           /*clock_enable_t*/ CLKEN_CLK_160M,
                           /*bool enable   */ true,
                                              &ulServiceErrorCode);
    if(ulErrorCode)
    {
        printf("SE Error: 160 MHz clk enable = %d\n", ulErrorCode);
        return;
    }

    printf("\r\n >>> ADC demo starting up!!! <<< \r\n");

    xVersion = pxADCDrv->GetVersion();
    printf("\r\n ADC version api:%X driver:%X...\r\n",xVersion.api, xVersion.drv);

    /* Initialize ADC driver */
    lRet = pxADCDrv->Initialize(prvAdcConversionCallBack);
    if (lRet != ARM_DRIVER_OK) {
        printf("\r\n Error: ADC init failed\n");
        return;
    }

    /* Power control ADC */
    lRet = pxADCDrv->PowerControl(ARM_POWER_FULL);
    if (lRet != ARM_DRIVER_OK) {
        printf("\r\n Error: ADC Power up failed\n");
        goto error_uninitialize;
    }

    /* set initial channel */
    lRet = pxADCDrv->Control(ARM_ADC_CHANNEL_INIT_VAL, ARM_ADC_CHANNEL_0);
    if (lRet != ARM_DRIVER_OK) {
        printf("\r\n Error: ADC select initial channel failed\n");
        goto error_poweroff;
    }

    printf(">>> Allocated memory buffer Address is 0x%X <<<\n",(uint32_t)ulAdcSample);

    /* Start ADC from External trigger pulse */
    lRet = pxADCDrv->Control(ARM_ADC_EXTERNAL_TRIGGER_ENABLE, ARM_ADC_EXTERNAL_TRIGGER_SRC_0);
    if (lRet != ARM_DRIVER_OK) {
        printf("\r\n Error: ADC External trigger enable failed\n");
        goto error_poweroff;
    }

    /* Start ADC */
    lRet = pxADCDrv->Start();
    if(lRet != ARM_DRIVER_OK){
        printf("\r\n Error: ADC Start failed\n");
        goto error_poweroff;
    }

    /* Set flag to wake up utimer task */
    xResult = xTaskNotify(utimer_xHandle,UTIMER_TASK_START,
                                 eSetBits);
    if (xResult == pdTRUE)
    {
        portYIELD();
    }
    else
    {
        printf("Error in invoking Utimer Task \r\n");
    }

    xReturned = xTaskNotifyWait(NULL,ADC_INT_AVG_SAMPLE_RDY,
                                NULL, portMAX_DELAY);
    if (xReturned != pdTRUE )
    {
        printf("\n\r Task Wait Time out expired \n\r");
        while(1);
    }

    /* Stop ADC external trigger conversion */
    lRet = pxADCDrv->Control(ARM_ADC_EXTERNAL_TRIGGER_DISABLE, ARM_ADC_EXTERNAL_TRIGGER_SRC_0);
    if (lRet != ARM_DRIVER_OK) {
        printf("\r\n Error: ADC External trigger disable failed\n");
        goto error_poweroff;
    }

    /* Stop ADC */
    lRet = pxADCDrv->Stop();
    if(lRet != ARM_DRIVER_OK){
        printf("\r\n Error: ADC Start failed\n");
        goto error_poweroff;
    }

    printf("\n >>> ADC conversion completed \n");
    printf(" Converted value are stored in user allocated memory address.\n");
    printf("\n ---END--- \r\n wait forever >>> \n");
    while(1);

error_poweroff:
    /* Power off ADC peripheral */
    lRet = pxADCDrv->PowerControl(ARM_POWER_OFF);
    if (lRet != ARM_DRIVER_OK)
    {
        printf("\r\n Error: ADC Power OFF failed.\r\n");
    }

error_uninitialize:
    /* Un-initialize ADC driver */
    lRet = pxADCDrv->Uninitialize();
    if (lRet != ARM_DRIVER_OK)
    {
        printf("\r\n Error: ADC Uninitialize failed.\r\n");
    }
    /* disable the 160 MHz clock */
    ulErrorCode = SERVICES_clocks_enable_clock(se_services_s_handle,
                           /*clock_enable_t*/ CLKEN_CLK_160M,
                           /*bool enable   */ false,
                                              &ulServiceErrorCode);
    if(ulErrorCode)
    {
        printf("SE Error: 160 MHz clk disable = %d\n", ulErrorCode);
        return;
    }

    printf("\r\n ADC demo exiting...\r\n");

    /* Task delete */
    vTaskDelete(adc_xHandle);
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
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

    /* System Initialization */
    SystemCoreClockUpdate();

    BaseType_t xReturned;

    /* Create application main thread */
    xReturned = xTaskCreate(prvAdcExtTriggerDemo,
                            "adc_ext_trigger_demo",
                            256, NULL,configMAX_PRIORITIES-2,
                            &adc_xHandle);
    if (xReturned != pdPASS)
    {
      vTaskDelete(adc_xHandle);
      return -1;
    }

    /* Create application main thread */
    xReturned = xTaskCreate(prvUtimerCompareModeDemo,
                            "utimer_compare_mode_demo",
                            256, NULL,configMAX_PRIORITIES-1,
                            &utimer_xHandle);
    if (xReturned != pdPASS)
    {
      vTaskDelete(utimer_xHandle);
      return -1;
    }

    /* Start thread execution */
    vTaskStartScheduler();
}

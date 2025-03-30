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
 * @file     UTIMER_app.c
 * @author   Sudarshan Iyengar, Manoj A Murudi
 * @email    sudarshan.iyengar@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     17-July-2023
 * @brief    FreeRTOS demo application for UTIMER.
 *           - Configuring the UTIMER Channel 0 for 500ms basic mode.
 *           - Configuring the UTIMER Channel 1 for 500ms, 1000ms, 1500ms buffering mode.
 *           - Configuring the UTIMER Channel 3 for counter start triggering mode.
 *           - Configuring the UTIMER Channel 4 for driver A, double buffering capture mode.
 *           - Configuring the UTIMER Channel 5 for driver A, double buffering compare mode.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

/* System Includes */
#include <stdio.h>
#include "Driver_UTIMER.h"
#include "Driver_GPIO.h"
#include "pinconf.h"

/*RTOS Includes */
#include "RTE_Components.h"
#include CMSIS_device_header

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


/* GPIO related definitions */
#define GPIO3                          3
#define GPIO3_PIN5                     5U
#define GPIO3_PIN6                     6U
#define GPIO3_PIN3                     3U
#define GPIO3_PIN4                     4U

/*Define for the FreeRTOS objects*/
/* UTIMER callback events */
#define UTIMER_OVERFLOW_CB_EVENT                 (1U << 0U)
#define UTIMER_CAPTURE_A_CB_EVENT                (1U << 1U)
#define UTIMER_COMPARE_A_CB_EVENT                (1U << 2U)
#define UTIMER_COMPARE_A_BUF1_CB_EVENT           (1U << 3U)
#define UTIMER_COMPARE_A_BUF2_CB_EVENT           (1U << 4U)

#define UTIMER_BASIC_MODE_WAIT_TIME              pdMS_TO_TICKS(1000U) /* interrupt wait time:1 second */
#define UTIMER_BUFFERING_MODE_WAIT_TIME          pdMS_TO_TICKS(3000U) /* interrupt wait time:3 seconds */
#define UTIMER_TRIGGER_MODE_WAIT_TIME            pdMS_TO_TICKS(1000U) /* interrupt wait time:1 second */
#define UTIMER_CAPTURE_MODE_WAIT_TIME            pdMS_TO_TICKS(1000U) /* interrupt wait time:1 second */
#define UTIMER_COMPARE_MODE_WAIT_TIME            pdMS_TO_TICKS(3000U) /* interrupt wait time:3 seconds */

/* UTIMER0 Driver instance */
extern ARM_DRIVER_UTIMER DRIVER_UTIMER0;
ARM_DRIVER_UTIMER *ptrUTIMER = &DRIVER_UTIMER0;

/* GPIO3 Driver instance */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO3);
ARM_DRIVER_GPIO *ptrDrv = &ARM_Driver_GPIO_(GPIO3);

/* Thread id of thread */
TaskHandle_t  utimer_basic_xHandle;
TaskHandle_t  utimer_buffer_xHandle;
TaskHandle_t  utimer_trigger_xHandle;
TaskHandle_t  utimer_capture_xHandle;
TaskHandle_t  utimer_compare_xHandle;

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

/**
 * @function    int gpio_init(ARM_UTIMER_MODE mode)
 * @brief       GPIO initialization using gpio driver
 * @note        none
 * @param       mode
 * @retval      execution status
 */
static int32_t gpio_init(ARM_UTIMER_MODE mode)
{
    int32_t ret;

    if (mode == ARM_UTIMER_MODE_TRIGGERING)
    {
        /* init P3_5 as GPIO */
        ret = pinconf_set(PORT_3, PIN_5, PINMUX_ALTERNATE_FUNCTION_0, 0);
        if (ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }

        ret = ptrDrv->Initialize(GPIO3_PIN5, NULL);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to initialize GPIO3_PIN5 as GPIO\n");
            return -1;
        }

        ret = ptrDrv->PowerControl(GPIO3_PIN5, ARM_POWER_FULL);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to Power up GPIO3_PIN5\n");
            return -1;
        }

        ret = ptrDrv->SetDirection(GPIO3_PIN5, GPIO_PIN_DIRECTION_OUTPUT);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set direction for GPIO3_PIN5\n");
            return -1;
        }

        ret = ptrDrv->SetValue(GPIO3_PIN5, GPIO_PIN_OUTPUT_STATE_LOW);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set value for GPIO3_PIN5\n");
            return -1;
        }

        /* init P3_6 as GPIO */
        ret = pinconf_set(PORT_3, PIN_6, PINMUX_ALTERNATE_FUNCTION_0, 0);
        if (ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }

        ret = ptrDrv->Initialize(GPIO3_PIN6, NULL);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to initialize GPIO3_PIN6 as GPIO\n");
            return -1;
        }

        ret = ptrDrv->PowerControl(GPIO3_PIN6, ARM_POWER_FULL);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to Power up GPIO3_PIN6\n");
            return -1;
        }

        ret = ptrDrv->SetDirection(GPIO3_PIN6, GPIO_PIN_DIRECTION_OUTPUT);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set direction for GPIO3_PIN6\n");
            return -1;
        }

        ret = ptrDrv->SetValue(GPIO3_PIN6, GPIO_PIN_OUTPUT_STATE_LOW);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set value for GPIO3_PIN6\n");
            return -1;
        }
    }

    else if (mode == ARM_UTIMER_MODE_CAPTURING)
    {
        /* init P3_3 as GPIO */
        ret = pinconf_set(PORT_3, PIN_3, PINMUX_ALTERNATE_FUNCTION_0, 0);
        if (ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }

        ret = ptrDrv->Initialize(GPIO3_PIN3, NULL);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to initialize GPIO3_PIN3 as GPIO\n");
            return -1;
        }

        ret = ptrDrv->PowerControl(GPIO3_PIN3, ARM_POWER_FULL);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to Power up GPIO3_PIN3\n");
            return -1;
        }

        ret = ptrDrv->SetDirection(GPIO3_PIN3, GPIO_PIN_DIRECTION_OUTPUT);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set direction for GPIO3_PIN3\n");
            return -1;
        }

        ret = ptrDrv->SetValue(GPIO3_PIN3, GPIO_PIN_OUTPUT_STATE_LOW);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set value for GPIO3_PIN3\n");
            return -1;
        }

        /* init P3_4 as GPIO */
        ret = pinconf_set(PORT_3, PIN_4, PINMUX_ALTERNATE_FUNCTION_0, 0);
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }

        ret = ptrDrv->Initialize(GPIO3_PIN4, NULL);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to initialize GPIO3_PIN4 as GPIO\n");
            return -1;
        }

        ret = ptrDrv->PowerControl(GPIO3_PIN4, ARM_POWER_FULL);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to Power up GPIO3_PIN4\n");
            return -1;
        }

        ret = ptrDrv->SetDirection(GPIO3_PIN4, GPIO_PIN_DIRECTION_OUTPUT);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set direction for GPIO3_PIN4\n");
            return -1;
        }

        ret = ptrDrv->SetValue(GPIO3_PIN4, GPIO_PIN_OUTPUT_STATE_LOW);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set value for GPIO3_PIN4\n");
            return -1;
        }
    }

    else
    {
       return -1;
    }

    return 0;
}

/**
 * @function    void utimer_basic_mode_cb_func(uint8_t event)
 * @brief       utimer basic mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_basic_mode_cb_func(uint8_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event & ARM_UTIMER_EVENT_OVER_FLOW)
    {
        xResult = xTaskNotifyFromISR(utimer_basic_xHandle, UTIMER_OVERFLOW_CB_EVENT, eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/**
 * @function    void utimer_basic_mode_app(void *pvParameters)
 * @brief       utimer basic mode application
 * @note        none
 * @param       pointer to thread input param
 * @retval      none
 */
static void utimer_basic_mode_app(void *pvParameters)
{
    int32_t ret;
    uint8_t channel = 0;
    uint32_t count_array[2];
    BaseType_t xReturned;

    /* utimer channel 0 is configured for utimer basic mode (config counter ptr reg for 500ms) */

    printf("*** utimer demo application for basic mode started ***\n");
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 500ms = (500*(10^-3))/(0.0025*(10^-6)) = 20000000
     *
     * DEC = 20000000
     * HEX = 0xBEBC200
     *
     */
    count_array[0] = 0x00000000;   /*< initial counter value >*/
    count_array[1] = 0xBEBC200;    /*< over flow count value >*/

    ret = ptrUTIMER->Initialize(channel, utimer_basic_mode_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl(channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_basic_mode_uninstall;
    }

    ret = ptrUTIMER->ConfigCounter(channel, ARM_UTIMER_MODE_BASIC, ARM_UTIMER_COUNTER_UP);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_basic_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR, count_array[0]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_basic_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR_PTR, count_array[1]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_basic_mode_poweroff;
    }

    printf("utimer channel '%d'configured on basic mode for 500 ms\r\n", channel);

    ret = ptrUTIMER->Start(channel);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", channel);
        goto error_basic_mode_poweroff;
    } else {
        printf("utimer channel '%d': timer started\n", channel);
    }

    xReturned = xTaskNotifyWait(NULL, UTIMER_OVERFLOW_CB_EVENT, NULL, UTIMER_BASIC_MODE_WAIT_TIME);
    if( xReturned != pdTRUE )
    {
        printf("\r\n Task notify wait timeout expired\r\n");
        goto error_basic_mode_poweroff;
    }
    printf("utimer channel %d :500ms timer expired \n", channel);

    ret = ptrUTIMER->Stop(channel, ARM_UTIMER_COUNTER_CLEAR);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d :timer stopped\n", channel);
    }

error_basic_mode_poweroff:
    ret = ptrUTIMER->PowerControl(channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_basic_mode_uninstall:

    ret = ptrUTIMER->Uninitialize(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }
    printf("*** demo application: basic mode completed *** \r\n\n");

    /* thread delete */
    vTaskDelete(NULL);
}

/**
 * @function    void utimer_buffering_mode_cb_func(uint8_t event)
 * @brief       utimer buffer mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_buffering_mode_cb_func(uint8_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event & ARM_UTIMER_EVENT_OVER_FLOW)
    {
        xResult = xTaskNotifyFromISR(utimer_buffer_xHandle, UTIMER_OVERFLOW_CB_EVENT, eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/**
 * @function    void utimer_buffering_mode_app(void *pvParameters)
 * @brief       utimer buffer mode application
 * @note        none
 * @param       pointer to thread input param
 * @retval      none
 */
static void utimer_buffering_mode_app (void *pvParameters)
{
    int32_t ret;
    uint8_t channel = 1;
    uint8_t index;
    uint32_t count_array[4];
    BaseType_t xReturned;

    /* utimer channel 1 is configured for utimer buffer mode (selected double buffering)
     * configuring counter ptr, buf1, buf2 reg's as 500ms, 1 sec, 1.5 sec respectively */

    printf("*** utimer demo application for buffering mode started ***\n");
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 500ms = (500*(10^-3))/(0.0025*(10^-6))
     * DEC = 200000000
     * HEX = 0xBEBC200
     *
     * So count for 1000ms = (1000*(10^-3))/(0.0025*(10^-6))
     * DEC = 400000000
     * HEX = 0x17D78400
     *
     * So count for 1500ms = (1500*(10^-3))/(0.0025*(10^-6))
     * DEC = 60000000
     * HEX = 0x23C34600
     *
     */

    count_array[0] = 0x00000000;     /*< Initial counter value>*/
    count_array[1] = 0xBEBC200;      /*< Over flow count value for First Iteration>*/
    count_array[2] = 0x17D78400;     /*< Over flow count value for Second Iteration>*/
    count_array[3] = 0x23C34600;     /*< Over flow count value for Third Iteration>*/

    ret = ptrUTIMER->Initialize(channel, utimer_buffering_mode_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl(channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_buffering_mode_uninstall;
    }

    ret = ptrUTIMER->ConfigCounter(channel, ARM_UTIMER_MODE_BUFFERING, ARM_UTIMER_COUNTER_UP);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_buffering_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR, count_array[0]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_buffering_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR_PTR, count_array[1]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_buffering_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR_PTR_BUF1, count_array[2]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_buffering_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR_PTR_BUF2, count_array[3]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_buffering_mode_poweroff;
    }

    printf("channel '%d'configured on buffering mode for 500, 1000, and 1500 ms \r\n", channel);

    ret = ptrUTIMER->Start(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", channel);
        goto error_buffering_mode_poweroff;
    } else {
        printf("utimer channel %d :timer started\n", channel);
    }

    for (index=0; index<3; index++)
    {
        xReturned = xTaskNotifyWait(NULL,UTIMER_OVERFLOW_CB_EVENT,NULL, UTIMER_BUFFERING_MODE_WAIT_TIME);
        if ( xReturned != pdTRUE )
        {
             printf("\r\n Task notify wait timeout expired\r\n");
             goto error_buffering_mode_poweroff;
        }

        printf("utimer channel %d: %d ms timer expired\n", channel, (500 + (500 * index)));
    }

    ret = ptrUTIMER->Stop(channel, ARM_UTIMER_COUNTER_CLEAR);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d: timer stopped\n", channel);
    }

error_buffering_mode_poweroff:

    ret = ptrUTIMER->PowerControl(channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("uTIMER channel %d failed power off \n", channel);
    }

error_buffering_mode_uninstall:

    ret = ptrUTIMER->Uninitialize(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }

    printf("*** demo application: buffering mode completed *** \r\n\n");

    /* thread delete */
    vTaskDelete(NULL);
}

/**
 * @function    void utimer_trigger_mode_cb_func(uint8_t event)
 * @brief       utimer trigger mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_trigger_mode_cb_func(uint8_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event & ARM_UTIMER_EVENT_OVER_FLOW)
    {
        xResult = xTaskNotifyFromISR(utimer_trigger_xHandle, UTIMER_OVERFLOW_CB_EVENT, eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/**
 * @function    void utimer_trigger_mode_app(void *pvParameters)
 * @brief       utimer trigger mode application
 * @note        none
 * @param       pointer to thread input param
 * @retval      none
 */
static void utimer_trigger_mode_app(void *pvParameters)
{
    int32_t ret;
    uint8_t channel = 3;
    uint32_t count_array[2], value;
    BaseType_t xReturned;

    ARM_UTIMER_TRIGGER_CONFIG trig_config = {
        .triggerTarget = ARM_UTIMER_TRIGGER_START,
        .triggerSrc = ARM_UTIMER_SRC_1,
        .trigger = ARM_UTIMER_SRC1_DRIVE_A_RISING_B_0
    };

    /*
     * utimer channel 3 is configured for utimer trigger mode.
     * chan_event_a_rising_b_0 event from pinmux is used for triggering counter start.
     * H/W connection : short P3_5 and P0_6, short P3_6 and P0_7.
     **/

    printf("*** utimer demo application for trigger mode started ***\n");
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 500ms = (500*(10^-3))/(0.0025*(10^-6)) = 200000000
     *
     * DEC = 200000000
     * HEX = 0xBEBC200
     */

    count_array[0] = 0;            /*< initial counter value >*/
    count_array[1] = 0xBEBC200;    /*< over flow count value >*/

    /* trigger mode pin config */
    ret = pinconf_set (PORT_0, PIN_6, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
    }

    ret = pinconf_set (PORT_0, PIN_7, PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
    }

    ret = gpio_init(ARM_UTIMER_MODE_TRIGGERING);
    if (ret) {
        printf("gpio init failed\n");
    }

    ret = ptrUTIMER->Initialize(channel, utimer_trigger_mode_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl(channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_trigger_mode_uninstall;
    }

    ret = ptrUTIMER->ConfigCounter(channel, ARM_UTIMER_MODE_TRIGGERING, ARM_UTIMER_COUNTER_UP);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_trigger_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR, count_array[0]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_trigger_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR_PTR, count_array[1]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_trigger_mode_poweroff;
    }

    /* Config Trigger for counter start using chan_event_a_rising_b_0 */
    ret = ptrUTIMER->ConfigTrigger(channel, &trig_config);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d trigger configuration failed \n", channel);
        goto error_trigger_mode_poweroff;
    } else {
        printf("utimer channel %d triggered for counter start \n", channel);
    }

    value = ptrUTIMER->GetCount(channel, ARM_UTIMER_CNTR);
    printf("counter value before triggering : %d\n",value);

    ret = ptrDrv->SetValue(GPIO3_PIN5, GPIO_PIN_OUTPUT_STATE_HIGH);
    if ((ret != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to set value for GPIO3_PIN5\n");
    }

    value = ptrUTIMER->GetCount(channel, ARM_UTIMER_CNTR);
    printf("counter value immediately after triggering : %d\n",value);

    xReturned = xTaskNotifyWait(NULL, UTIMER_OVERFLOW_CB_EVENT, NULL, UTIMER_TRIGGER_MODE_WAIT_TIME);
    if ( xReturned != pdTRUE )
    {
         printf("\r\n Task notify wait timeout expired\r\n");
         goto error_trigger_mode_poweroff;
    }
    printf("overflow interrupt is generated\n");

    ret = ptrUTIMER->Stop(channel, ARM_UTIMER_COUNTER_CLEAR);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d :timer stopped\n", channel);
    }

error_trigger_mode_poweroff:
    ret = ptrUTIMER->PowerControl(channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_trigger_mode_uninstall:

    ret = ptrUTIMER->Uninitialize(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }
    printf("*** demo application: trigger mode completed *** \r\n\n");

    /* thread delete */
    vTaskDelete(NULL);
}

/**
 * @function    void utimer_capture_mode_cb_funcc(event)
 * @brief       utimer capture mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_capture_mode_cb_func(uint8_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event & ARM_UTIMER_EVENT_CAPTURE_A)
    {
        ptrDrv->SetValue(GPIO3_PIN3, GPIO_PIN_OUTPUT_STATE_LOW);

        xResult = xTaskNotifyFromISR(utimer_capture_xHandle, UTIMER_CAPTURE_A_CB_EVENT, eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/**
 * @function    void utimer_capture_mode_app(void *pvParameters)
 * @brief       utimer capture mode application
 * @note        none
 * @param       pointer to thread input param
 * @retval      none
 */
static void utimer_capture_mode_app(void *pvParameters)
{
    int32_t ret;
    uint8_t channel = 4;
    uint32_t count_array[2];
    BaseType_t xReturned;

    ARM_UTIMER_TRIGGER_CONFIG trig_config = {
        .triggerTarget = ARM_UTIMER_TRIGGER_CAPTURE_A,
        .triggerSrc = ARM_UTIMER_SRC_1,
        .trigger = ARM_UTIMER_SRC1_DRIVE_A_RISING_B_0
    };

    /*
     * utimer channel 4 is configured for utimer input capture mode (selected driver A, double buffer).
     * chan_event_a_rising_b_0 event from pinmux is used to trigger input capture counter value.
     * H/W connection : short P3_3 and P1_0, short P3_4 and P1_1.
     */

    printf("*** utimer demo application for capture mode started ***\n");
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 1 sec = 1/(0.0025*(10^-6)) = 400000000
     *
     * DEC = 400000000
     * HEX = 0x17D78400
     */
    count_array[0] = 0;             /*< initial counter value >*/
    count_array[1] = 0x17D78400;    /*< over flow count value >*/

    /* capture mode pin config */
    ret = pinconf_set(PORT_1, PIN_0, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
    }

    ret = pinconf_set(PORT_1, PIN_1, PINMUX_ALTERNATE_FUNCTION_4, PADCTRL_READ_ENABLE);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
    }

    /* GPIO pin confg */
    ret = gpio_init(ARM_UTIMER_MODE_CAPTURING);
    if (ret) {
        printf("gpio init failed\n");
    }

    ret = ptrUTIMER->Initialize(channel, utimer_capture_mode_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl(channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_capture_mode_uninstall;
    }

    ret = ptrUTIMER->ConfigCounter(channel, ARM_UTIMER_MODE_CAPTURING, ARM_UTIMER_COUNTER_UP);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_capture_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR, count_array[0]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_capture_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR_PTR, count_array[1]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_capture_mode_poweroff;
    }

    /* Config Trigger for capture counter value for chan_event_a_rising_b_0 */
    ret = ptrUTIMER->ConfigTrigger(channel, &trig_config);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d trigger configuration failed \n", channel);
        goto error_capture_mode_poweroff;
    } else {
        printf("utimer channel %d triggered for capture counter value \n", channel);
    }

    ret = ptrUTIMER->Start(channel);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", channel);
        goto error_capture_mode_poweroff;
    } else {
        printf("utimer channel '%d': timer started\n", channel);
    }

    for(int index=0; index<3; index++)
    {
        /* Delay of 100 ms */
        vTaskDelay(100/portTICK_PERIOD_MS);

        ret = ptrDrv->SetValue(GPIO3_PIN3, GPIO_PIN_OUTPUT_STATE_HIGH);
        if ((ret != ARM_DRIVER_OK)) {
            printf("ERROR: Failed to set value for GPIO3_PIN3\n");
        }

        xReturned = xTaskNotifyWait(NULL,UTIMER_CAPTURE_A_CB_EVENT,NULL,UTIMER_CAPTURE_MODE_WAIT_TIME);
        if( xReturned != pdTRUE )
        {
             printf("\r\n Task notify wait timeout expired\r\n");
             goto error_capture_mode_poweroff;
        }
        printf("current counter value is captured\n");
    }

    ret = ptrUTIMER->Stop(channel, ARM_UTIMER_COUNTER_CLEAR);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d :timer stopped \n", channel);
    }

    printf("counter value at capture a : 0x%x \n", ptrUTIMER->GetCount(channel, ARM_UTIMER_CAPTURE_A));
    printf("counter value at capture a buf1 : 0x%x \n", ptrUTIMER->GetCount(channel, ARM_UTIMER_CAPTURE_A_BUF1));
    printf("counter value at capture a buf2 : 0x%x \n", ptrUTIMER->GetCount(channel, ARM_UTIMER_CAPTURE_A_BUF2));

    error_capture_mode_poweroff:
    ret = ptrUTIMER->PowerControl(channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_capture_mode_uninstall:

    ret = ptrUTIMER->Uninitialize(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }

    printf("*** demo application: capture mode completed *** \r\n\n");

    /* thread delete */
    vTaskDelete( NULL );
}

/**
 * @function    void utimer_compare_mode_cb_func(uint8_t event)
 * @brief       utimer compare mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_compare_mode_cb_func(uint8_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult = pdFALSE;

    if (event & ARM_UTIMER_EVENT_COMPARE_A) {
        xResult = xTaskNotifyFromISR(utimer_compare_xHandle, UTIMER_COMPARE_A_CB_EVENT, eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if (event & ARM_UTIMER_EVENT_COMPARE_A_BUF1) {
        xResult = xTaskNotifyFromISR(utimer_compare_xHandle, UTIMER_COMPARE_A_BUF1_CB_EVENT, eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if (event & ARM_UTIMER_EVENT_COMPARE_A_BUF2) {
        xResult = xTaskNotifyFromISR(utimer_compare_xHandle, UTIMER_COMPARE_A_BUF2_CB_EVENT, eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if (event & ARM_UTIMER_EVENT_OVER_FLOW) {
        xResult = xTaskNotifyFromISR(utimer_compare_xHandle, UTIMER_OVERFLOW_CB_EVENT, eSetBits, &xHigherPriorityTaskWoken);
        if (xResult == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/**
 * @function    void utimer_compare_mode_app(void *pvParameters)
 * @brief       utimer compare mode application
 * @note        none
 * @param       pointer to thread input param
 * @retval      none
 */
static void utimer_compare_mode_app(void *pvParameters)
{
    int32_t ret;
    uint8_t channel = 5;
    uint32_t count_array[5], NotificationValue = 0;
    BaseType_t xReturned;

    /*
     * utimer channel 5 is configured for utimer compare mode (driver A, double buffer is enabled).
     * observe driver A output signal from P1_2.
     */
    printf("*** utimer demo application for compare mode started ***\n");
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
     * So count for 250ms = (250*(10^-3)/(0.0025*(10^-6)) = 100000000
     * DEC = 100000000
     * HEX = 0x5F5E100
     *
     * So count for 500ms = (500*(10^-3)/(0.0025*(10^-6)) = 200000000
     * DEC = 200000000
     * HEX = 0xBEBC200
     *
     * So count for 750ms = (750*(10^-3)/(0.0025*(10^-6)) = 300000000
     * DEC = 300000000
     * HEX = 0x11E1A300
     */
    count_array[0] =  0x000000000;       /*< initial counter value >*/
    count_array[1] =  0x17D78400;        /*< over flow count value >*/
    count_array[2] =  0x5F5E100;         /*< compare a/b value>*/
    count_array[3] =  0xBEBC200;         /*< compare a/b buf1 value>*/
    count_array[4] =  0x11E1A300;        /*< compare a/b buf2 value>*/

    /* compare mode pin confg */
    ret = pinconf_set (PORT_1, PIN_2, PINMUX_ALTERNATE_FUNCTION_4, 0);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
    }

    ret = ptrUTIMER->Initialize(channel, utimer_compare_mode_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl(channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_compare_mode_uninstall;
    }

    ret = ptrUTIMER->ConfigCounter(channel, ARM_UTIMER_MODE_COMPARING, ARM_UTIMER_COUNTER_UP);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_compare_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR, count_array[0]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_compare_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR_PTR, count_array[1]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_compare_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_COMPARE_A, count_array[2]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_compare_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_COMPARE_A_BUF1, count_array[3]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_compare_mode_poweroff;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_COMPARE_A_BUF2, count_array[4]);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d set count failed \n", channel);
        goto error_compare_mode_poweroff;
    }

    ret = ptrUTIMER->Start(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", channel);
        goto error_compare_mode_poweroff;
    } else {
        printf("utimer channel %d :timer started\n", channel);
    }

    for (int index = 0; index <= 8; index++)
    {
        xReturned = xTaskNotifyWait(NULL, UTIMER_OVERFLOW_CB_EVENT | UTIMER_COMPARE_A_CB_EVENT |
                UTIMER_COMPARE_A_BUF1_CB_EVENT | UTIMER_COMPARE_A_BUF2_CB_EVENT, &NotificationValue, UTIMER_COMPARE_MODE_WAIT_TIME);
        if(xReturned != pdTRUE)
        {
             printf("\r\n Task notify wait timeout expired\r\n");
             goto error_compare_mode_poweroff;
        }

        if (NotificationValue & UTIMER_COMPARE_A_CB_EVENT) {
            printf("compare_a reg value is matched to counter value\n");
        }
        if (NotificationValue & UTIMER_COMPARE_A_BUF1_CB_EVENT) {
            printf("compare_a_buf1 reg value is matched to counter value\n");
        }
        if (NotificationValue & UTIMER_COMPARE_A_BUF2_CB_EVENT) {
            printf("compare_a_buf2 reg value is matched to counter value\n");
        }
        if (NotificationValue & UTIMER_OVERFLOW_CB_EVENT) {
            printf("Interrupt: Overflow occurred\n");
        }

        NotificationValue = 0;
    }

    ret = ptrUTIMER->Stop(channel, ARM_UTIMER_COUNTER_CLEAR);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d: timer stopped\n", channel);
    }

error_compare_mode_poweroff:

    ret = ptrUTIMER->PowerControl(channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_compare_mode_uninstall:

    ret = ptrUTIMER->Uninitialize(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }

    printf("*** demo application: compare mode completed *** \r\n\n");

    /* thread delete */
    vTaskDelete(NULL);
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
int main( void )
{
    BaseType_t xReturned;

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
    xReturned = xTaskCreate(utimer_basic_mode_app, "utimer_basic_mode_app", 256, NULL, configMAX_PRIORITIES-1, &utimer_basic_xHandle);
    if (xReturned != pdPASS)
    {
       vTaskDelete(utimer_basic_xHandle);
       return -1;
    }

   /* Create application main thread */
   xReturned = xTaskCreate(utimer_buffering_mode_app, "utimer_buffering_mode_app", 256, NULL, configMAX_PRIORITIES-1, &utimer_buffer_xHandle);
   if (xReturned != pdPASS)
   {
       vTaskDelete(utimer_buffer_xHandle);
       return -1;
    }

   /* Create application main thread */
    xReturned = xTaskCreate(utimer_trigger_mode_app, "utimer_trigger_mode_app", 256, NULL, configMAX_PRIORITIES-1, &utimer_trigger_xHandle);
    if (xReturned != pdPASS)
    {
       vTaskDelete(utimer_trigger_xHandle);
       return -1;
    }

   /* Create application main thread */
   xReturned = xTaskCreate(utimer_capture_mode_app, "utimer_capture_mode_app", 256, NULL, configMAX_PRIORITIES-1, &utimer_capture_xHandle);
   if (xReturned != pdPASS)
   {
       vTaskDelete(utimer_capture_xHandle);
       return -1;
    }

   /* Create application main thread */
    xReturned = xTaskCreate(utimer_compare_mode_app, "utimer_compare_mode_app", 256, NULL, configMAX_PRIORITIES-1, &utimer_compare_xHandle);
    if (xReturned != pdPASS)
    {
       vTaskDelete(utimer_compare_xHandle);
       return -1;
    }

    /* Start thread execution */
    vTaskStartScheduler();
}

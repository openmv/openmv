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
 * @file     gt911_touchscreen_testapp.c
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     07-Sept-2023
 * @brief    TestApp to verify GT911 touch screen with
 *            FREERTOS as an Operating System.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* System Includes */
#include "stdio.h"
#include "stdint.h"
#include "string.h"

/* PINMUX Driver */
#include "pinconf.h"
#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


/*touch screen driver */
#include "Driver_Touch_Screen.h"

/* Rtos include */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

/* Touch screen driver instance */
extern ARM_DRIVER_TOUCH_SCREEN GT911;
static ARM_DRIVER_TOUCH_SCREEN *Drv_Touchscreen = &GT911;

#define GT911_TOUCH_INT_GPIO_PORT        PORT_9
#define GT911_TOUCH_INT_PIN_NO           PIN_4
#define GT911_TOUCH_I2C_SDA_PORT         PORT_7
#define GT911_TOUCH_I2C_SDA_PIN_NO       PIN_2
#define GT911_TOUCH_I2C_SCL_PORT         PORT_7
#define GT911_TOUCH_I2C_SCL_PIN_NO       PIN_3
#define ACTIVE_TOUCH_POINTS              5

/*Define for FreeRTOS*/
#define STACK_SIZE                       1024
#define TIMER_SERVICE_TASK_STACK_SIZE    configTIMER_TASK_STACK_DEPTH
#define IDLE_TASK_STACK_SIZE             configMINIMAL_STACK_SIZE

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

/* Thread id of thread */
TaskHandle_t touch_xHandle;

/****************************** FreeRTOS functions **********************/

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
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
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
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
  \fn          int32_t hardware_cfg(void)
  \brief       -i2c hardware pin initialization:
                   -  PIN-MUX configuration
                   -  PIN-PAD configuration
               -GPIO9 initialization:
                   -  PIN-MUX configuration
                   -  PIN-PAD configuration
  \param[in]   none
  \return      ARM_DRIVER_OK: success; 0: failure
  */
int32_t hardware_cfg(void)
{
    int8_t ret = 0;
    /* gpio9 config for interrupt
     * Pad function: PADCTRL_READ_ENABLE |
     *               PADCTRL_DRIVER_DISABLED_PULL_UP |
     *               PADCTRL_SCHMITT_TRIGGER_ENABLE
     */
    ret = pinconf_set(GT911_TOUCH_INT_GPIO_PORT, GT911_TOUCH_INT_PIN_NO,
                      PINMUX_ALTERNATE_FUNCTION_0, PADCTRL_READ_ENABLE |
                      PADCTRL_SCHMITT_TRIGGER_ENABLE | PADCTRL_DRIVER_DISABLED_PULL_UP);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: GPIO PINMUX failed.\r\n");
        return ret;
    }

    /* Configure GPIO Pin : P7_2 as i2c1_sda_c
     * Pad function: PADCTRL_READ_ENABLE |
     *               PADCTRL_DRIVER_DISABLED_PULL_UP
     */
    ret = pinconf_set(GT911_TOUCH_I2C_SDA_PORT, GT911_TOUCH_I2C_SDA_PIN_NO,
                      PINMUX_ALTERNATE_FUNCTION_5, PADCTRL_READ_ENABLE |
                      PADCTRL_DRIVER_DISABLED_PULL_UP);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I2C SDA PINMUX failed.\r\n");
        return ret;
    }

    /* Configure GPIO Pin : P7_3 as i2c1_scl_c
     * Pad function: PADCTRL_READ_ENABLE |
     *               PADCTRL_DRIVER_DISABLED_PULL_UP
     */
    ret = pinconf_set(GT911_TOUCH_I2C_SCL_PORT, GT911_TOUCH_I2C_SCL_PIN_NO,
                      PINMUX_ALTERNATE_FUNCTION_5,PADCTRL_READ_ENABLE |
                      PADCTRL_DRIVER_DISABLED_PULL_UP);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error: I2C SCL PINMUX failed.\r\n");
        return ret;
    }

    return ARM_DRIVER_OK;
}

/**
  * @function    void touchscreen_demo_thread_entry(void *pvParameters)
  \brief         TestApp to verify GT911 touch screen with
                 FREERTOS as an Operating System.
                 This demo thread does:
                    - initialize i2c AND gpio9 port hardware pins
                    - initialize GT911 Touch screen driver with call back function.
                    - check if touch screen is pressed or not
                    - if pressed then print up to 5 coordinate positions where display touch screen was touched.
  @param       pvParameters.
  \return      none
  */
void touchscreen_demo_thread_entry(void *pvParameters)
{
    int8_t ret = 0;
    int8_t count = 0;
    ARM_DRIVER_VERSION version;
    ARM_TOUCH_STATE state;

    printf("\r\n \t\t >>> GT911 Touchscreen demo with FREERTOS is starting up!!! <<< \r\n");

    /* Initialize i2c and GPIO9 hardware pins using PinMux Driver. */
    /* Initialize UART4 hardware pins using PinMux driver if printf redirection to UART is selected */
    ret = hardware_cfg();
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in hardware configuration */
        printf("\r\n Error: Hardware configuration failed.\r\n");
    }

    /* Touch screen version */
    version = Drv_Touchscreen->GetVersion();
    printf("\r\n Touchscreen driver version api:0x%X driver:0x%X \r\n",version.api, version.drv);

    /* Initialize GT911 touch screen */
    ret = Drv_Touchscreen->Initialize();
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in GT911 touch screen initialize */
        printf("\r\n Error: GT911 touch screen initialization failed.\r\n");
        goto error_GT911_uninitialize;
    }

    /* Power ON GT911 touch screen */
    ret = Drv_Touchscreen->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in GT911 touch screen power up */
        printf("\r\n Error: GT911 touch screen Power Up failed.\r\n");
        goto error_GT911_uninitialize;
    }

    while(1)
    {
        /* Reading GT911 touch screen press status */
        ret =Drv_Touchscreen->GetState(&state);
        if(ret != ARM_DRIVER_OK)
        {
            /* Error in GT911 touch screen read status */
            printf("\r\n Error: GT911 touch screen read  status failed.\r\n");
            goto error_GT911_poweroff;
        }

        if(state.numtouches)
        {
            for(count = 1; count <= ACTIVE_TOUCH_POINTS; count++)
            {
                /* Print coordinates positions of pressing on GT911 touch screen up to max touch points set */
                printf("x%d: %d y%d: %d \r\n",count, state.coordinates[count - 1].x, count, state.coordinates[count - 1].y);
            }
            memset(state.coordinates,0,sizeof(state.coordinates));
        }
    }

error_GT911_poweroff:
    /* Received error Power off GT911 touch screen peripheral */
    ret = Drv_Touchscreen->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        /* Error in GT911 Touch screen Power OFF. */
        printf("ERROR: Could not power OFF touch screen\n");
        return;
    }

error_GT911_uninitialize:
    /* Received error Un-initialize Touch screen driver */
    ret = Drv_Touchscreen->Uninitialize();
    if (ret != ARM_DRIVER_OK)
    {
        /* Error in GT911 Touch screen uninitialize. */
        printf("ERROR: Could not unintialize touch screen\n");
        return;
    }

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
    BaseType_t xReturned = xTaskCreate(touchscreen_demo_thread_entry, "touchscreen_demo_thread_entry",
                                       216, NULL,configMAX_PRIORITIES-1, &touch_xHandle);
    if (xReturned != pdPASS)
    {

        vTaskDelete(touch_xHandle);
        return -1;
    }

    /* Start thread execution */
    vTaskStartScheduler();
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/


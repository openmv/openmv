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
 * @file     LED_Blinky_testapp.c
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     25-May-2023
 * @brief    FreeRTOS DEMO application for LED blink.
 *           - Toggle LED0_R and LED1_B alternately
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <RTE_Components.h>
#include CMSIS_device_header

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "Driver_GPIO.h"
#include "pinconf.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


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

/* Define the FreeRTOS object control blocks...  */
#define DEMO_STACK_SIZE                 1024

TaskHandle_t led_xHandle;

/* LED0 gpio pins */
#define GPIO12_PORT                     12  /*< Use LED0_R,LED0_B GPIO port >*/
#define GPIO7_PORT                      7   /*< Use LED0_G GPIO port >*/
#define PIN3                            3   /*< LED0_R gpio pin >*/
#define PIN4                            4   /*< LED0_G gpio pin >*/
#define PIN0                            0   /*< LED0_B gpio pin >*/

/* LED1 gpio pins */
#define GPIO6_PORT                      6   /*< Use LED1_R,LED1_B,LED1_R GPIO port >*/
#define PIN2                            2   /*< LED1_R gpio pin >*/
#define PIN4                            4   /*< LED1_G gpio pin >*/
#define PIN6                            6   /*< LED1_B gpio pin >*/

/* GPIO port used for LED0_R & LED0_B */
extern  ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO12_PORT);
ARM_DRIVER_GPIO *gpioDrv12 = &ARM_Driver_GPIO_(GPIO12_PORT);

/* GPIO port used for LED0_G */
extern  ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO7_PORT);
ARM_DRIVER_GPIO *gpioDrv7 = &ARM_Driver_GPIO_(GPIO7_PORT);

/* GPIO port used for LED1_R, LED1_B & LED1_G */
extern  ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO6_PORT);
ARM_DRIVER_GPIO *gpioDrv6 = &ARM_Driver_GPIO_(GPIO6_PORT);

/**
  \fn         void led_demo_Thread(void *pvParameters)
  \brief      LED blinky demo Thread
  \param[in]  pvParameters : thread input
  \return     none
*/
void led_demo_Thread(void *pvParameters)
{
  /*
   * gpio12 pin3 can be used as Red LED of LED0.
   * gpio7 pin4 can be used as Green LED of LED0.
   * gpio12 pin0 can be used as Blue LED of LED0.
   *
   * gpio6 pin2 can be used as Red LED of LED1.
   * gpio6 pin4 can be used as Green LED of LED1.
   * gpio6 pin6 can be used as Blue LED of LED1.
   *
   * This demo application is about.
   *   - Blink LED0_R and LED1_R, then LED0_B and LED1_B, then LED0_G and LED1_G simultaneously in rotation.
   */

    int32_t ret1 = 0;
    int32_t ret2 = 0;
    uint8_t LED0_R = PIN3;
    uint8_t LED0_G = PIN4;
    uint8_t LED0_B = PIN0;
    uint8_t LED1_R = PIN2;
    uint8_t LED1_G = PIN4;
    uint8_t LED1_B = PIN6;
    const TickType_t xDelay = (1000/portTICK_PERIOD_MS);

    printf("led blink demo application for FreeRTOS started\n\n");

    /* pinmux configurations for all GPIOs */
    pinconf_set(GPIO12_PORT, LED0_R, PINMUX_ALTERNATE_FUNCTION_0, 0);
    pinconf_set(GPIO7_PORT, LED0_G, PINMUX_ALTERNATE_FUNCTION_0, 0);
    pinconf_set(GPIO12_PORT, LED0_B, PINMUX_ALTERNATE_FUNCTION_0, 0);
    pinconf_set(GPIO6_PORT, LED1_R, PINMUX_ALTERNATE_FUNCTION_0, 0);
    pinconf_set(GPIO6_PORT, LED1_G, PINMUX_ALTERNATE_FUNCTION_0, 0);
    pinconf_set(GPIO6_PORT, LED1_B, PINMUX_ALTERNATE_FUNCTION_0, 0);

    ret1 = gpioDrv12->Initialize(LED0_R, NULL);
    ret2 = gpioDrv6->Initialize(LED1_R, NULL);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to initialize\n");
        return;
    }
    ret1 = gpioDrv7->Initialize(LED0_G, NULL);
    ret2 = gpioDrv6->Initialize(LED1_G, NULL);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to initialize\n");
        return;
    }
    ret1 = gpioDrv12->Initialize(LED0_B, NULL);
    ret2 = gpioDrv6->Initialize(LED1_B, NULL);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to initialize\n");
        return;
    }

    ret1 = gpioDrv12->PowerControl(LED0_R, ARM_POWER_FULL);
    ret2 = gpioDrv6->PowerControl(LED1_R, ARM_POWER_FULL);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to powered full\n");
        goto error_uninitialize;
    }
    ret1 = gpioDrv7->PowerControl(LED0_G, ARM_POWER_FULL);
    ret2 = gpioDrv6->PowerControl(LED1_G, ARM_POWER_FULL);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to powered full\n");
        goto error_uninitialize;
    }
    ret1 = gpioDrv12->PowerControl(LED0_B, ARM_POWER_FULL);
    ret2 = gpioDrv6->PowerControl(LED1_B, ARM_POWER_FULL);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to powered full\n");
        goto error_uninitialize;
    }

    ret1 = gpioDrv12->SetDirection(LED0_R, GPIO_PIN_DIRECTION_OUTPUT);
    ret2 = gpioDrv6->SetDirection(LED1_R, GPIO_PIN_DIRECTION_OUTPUT);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to configure\n");
        goto error_power_off;
    }
    ret1 = gpioDrv7->SetDirection(LED0_G, GPIO_PIN_DIRECTION_OUTPUT);
    ret2 = gpioDrv6->SetDirection(LED1_G, GPIO_PIN_DIRECTION_OUTPUT);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to configure\n");
        goto error_power_off;
    }
    ret1 = gpioDrv12->SetDirection(LED0_B, GPIO_PIN_DIRECTION_OUTPUT);
    ret2 = gpioDrv6->SetDirection(LED1_B, GPIO_PIN_DIRECTION_OUTPUT);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to configure\n");
        goto error_power_off;
    }

    while (1)
    {
        /* Toggle Red LED */
        ret1 = gpioDrv12->SetValue(LED0_R, GPIO_PIN_OUTPUT_STATE_HIGH);
        ret2 = gpioDrv6->SetValue(LED1_R, GPIO_PIN_OUTPUT_STATE_HIGH);
        if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
            printf("ERROR: Failed to toggle LEDs\n");
            goto error_power_off;
        }

        /* delay for 1 Sec */
        vTaskDelay(xDelay);

        ret1 = gpioDrv12->SetValue(LED0_R, GPIO_PIN_OUTPUT_STATE_LOW);
        ret2 = gpioDrv6->SetValue(LED1_R, GPIO_PIN_OUTPUT_STATE_LOW);
        if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
            printf("ERROR: Failed to toggle LEDs\n");
            goto error_power_off;
        }

        /* delay for 1 Sec */
        vTaskDelay(xDelay);


        /* Toggle Green LED */
        ret1 = gpioDrv7->SetValue(LED0_G, GPIO_PIN_OUTPUT_STATE_HIGH);
        ret2 = gpioDrv6->SetValue(LED1_G, GPIO_PIN_OUTPUT_STATE_HIGH);
        if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
            printf("ERROR: Failed to toggle LEDs\n");
            goto error_power_off;
        }

        /* delay for 1 Sec */
        vTaskDelay(xDelay);

        ret1 = gpioDrv7->SetValue(LED0_G, GPIO_PIN_OUTPUT_STATE_LOW);
        ret2 = gpioDrv6->SetValue(LED1_G, GPIO_PIN_OUTPUT_STATE_LOW);
        if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
            printf("ERROR: Failed to toggle LEDs\n");
            goto error_power_off;
        }

        /* delay for 1 Sec */
        vTaskDelay(xDelay);


        /* Toggle Blue LED */
        ret1 = gpioDrv12->SetValue(LED0_B, GPIO_PIN_OUTPUT_STATE_HIGH);
        ret2 = gpioDrv6->SetValue(LED1_B, GPIO_PIN_OUTPUT_STATE_HIGH);
        if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
            printf("ERROR: Failed to toggle LEDs\n");
            goto error_power_off;
        }

        /* delay for 1 Sec */
        vTaskDelay(xDelay);

        ret1 = gpioDrv12->SetValue(LED0_B, GPIO_PIN_OUTPUT_STATE_LOW);
        ret2 = gpioDrv6->SetValue(LED1_B, GPIO_PIN_OUTPUT_STATE_LOW);
        if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
            printf("ERROR: Failed to toggle LEDs\n");
            goto error_power_off;
        }

        /* delay for 1 Sec */
        vTaskDelay(xDelay);
    };

error_power_off:

    ret1 = gpioDrv12->PowerControl(LED0_R, ARM_POWER_OFF);
    ret2 = gpioDrv6->PowerControl(LED1_R, ARM_POWER_OFF);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to power off \n");
    } else {
        printf("LEDs power off \n");
    }
    ret1 = gpioDrv7->PowerControl(LED0_G, ARM_POWER_OFF);
    ret2 = gpioDrv6->PowerControl(LED1_G, ARM_POWER_OFF);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to power off \n");
    } else {
        printf("LEDs power off \n");
    }
    ret1 = gpioDrv12->PowerControl(LED0_B, ARM_POWER_OFF);
    ret2 = gpioDrv6->PowerControl(LED1_B, ARM_POWER_OFF);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to power off \n");
    } else {
        printf("LEDs power off \n");
    }

error_uninitialize:

    ret1 = gpioDrv12->Uninitialize(LED0_R);
    ret2 = gpioDrv6->Uninitialize(LED1_R);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("Failed to Un-initialize \n");
    } else {
        printf("Un-initialized \n");
    }
    ret1 = gpioDrv7->Uninitialize(LED0_G);
    ret2 = gpioDrv6->Uninitialize(LED1_G);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("Failed to Un-initialize \n");
    } else {
        printf("Un-initialized \n");
    }
    ret1 = gpioDrv12->Uninitialize(LED0_B);
    ret2 = gpioDrv6->Uninitialize(LED1_B);
    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("Failed to Un-initialize \n");
    } else {
        printf("Un-initialized \n");
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
   BaseType_t xReturned = xTaskCreate(led_demo_Thread, "led_demo_Thread", 216, NULL,configMAX_PRIORITIES-1, &led_xHandle);
   if (xReturned != pdPASS)
   {
      vTaskDelete(led_xHandle);
      return -1;
   }

   /* Start thread execution */
   vTaskStartScheduler();

}
/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/


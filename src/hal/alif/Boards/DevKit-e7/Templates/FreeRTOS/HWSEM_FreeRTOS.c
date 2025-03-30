/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     HWSEM_FreeRTOS.c
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     27-June-2022
 * @brief    TestApp to verify HWSEM interface with FreeRTOS.
 *             It locks the HWSEM, sends the message through UART and then unlocks.
 *               This application will run on two cores
 *               to demonstrate the working of the Hardware Semaphore.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "RTE_Components.h"
#include CMSIS_device_header

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "Driver_USART.h"
#include <pinconf.h>
#include "Driver_HWSEM.h"

#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

#ifdef M55_HP
const char * msg = "\nPrinting from M55_HP";
const char * acq_msg = "\nM55_HP acquiring the semaphore, printing 10 messages\r\n";
const char * rel_msg = "\nM55_HP releasing the semaphore\r\n\n";
#else
const char * msg = "\nPrinting from M55_HE";
const char * acq_msg = "\nM55_HE acquiring the semaphore, printing 10 messages\r\n";
const char * rel_msg = "\nM55_HE releasing the semaphore\r\n\n";
#endif

/*Define for FreeRTOS*/
#define STACK_SIZE     1024
#define TIMER_SERVICE_TASK_STACK_SIZE configTIMER_TASK_STACK_DEPTH // 512
#define IDLE_TASK_STACK_SIZE          configMINIMAL_STACK_SIZE // 1024

StackType_t IdleStack[2 * IDLE_TASK_STACK_SIZE];
StaticTask_t IdleTcb;
StackType_t TimerStack[2 * TIMER_SERVICE_TASK_STACK_SIZE];
StaticTask_t TimerTcb;

#define UART_CB_TX_EVENT          1U << 0
#define UART_CB_RX_EVENT          1U << 1
#define UART_CB_RX_TIMEOUT        1U << 2

#define HWSEM_CB_EVENT            1U << 3

#define SEMID                     0

/* HWSEM Driver instance */
#define HWSEM                     0
/* Uart Driver instance */
#define UART                      4

/* UART Driver */
extern ARM_DRIVER_USART ARM_Driver_USART_(UART);
static ARM_DRIVER_USART *USARTdrv = &ARM_Driver_USART_(UART);

/* HWSEM Driver */
extern ARM_DRIVER_HWSEM ARM_Driver_HWSEM_(HWSEM);
static ARM_DRIVER_HWSEM *HWSEMdrv = &ARM_Driver_HWSEM_(HWSEM);

static TaskHandle_t Hwsem_xHandle;

/* ****************************** FreeRTOS functions **********************/
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
 * @function    void myUART_callback(uint32_t event)
 * @brief       UART isr callabck
 * @note        none
 * @param       event: USART Event
 * @retval      none
 */
static void myUART_callback(uint32_t event)
{
    if (event & ARM_USART_EVENT_SEND_COMPLETE)
    {
        xTaskNotifyFromISR(Hwsem_xHandle, UART_CB_TX_EVENT, eSetBits, NULL);
    }
}

/**
 * @function   void HWSEM_callback(int32_t event, uint8_t sem_id)
 * @brief      HWSEM isr callabck
 * @note       none
 * @param      event: HWSEM Event
 * @retval     none
 */
static void myHWSEM_callback(int32_t event, uint8_t sem_id)
{
    (void) sem_id;

    if (event & HWSEM_AVAILABLE_CB_EVENT)
    {
        xTaskNotifyFromISR(Hwsem_xHandle,HWSEM_CB_EVENT, eSetBits, NULL);
    }
}

/**
 * @function    static int32_t pinmux_init(void)
 * @brief       pinmux initialization
 * @note        none
 * @param       void
 * @retval      execution status
 */
static int32_t pinmux_init(void)
{
    int32_t ret;

    /* UART4_RX_B */
    ret = pinconf_set(PORT_12, PIN_1, PINMUX_ALTERNATE_FUNCTION_2, PADCTRL_READ_ENABLE);

    if (ret)
    {
        return ret;
    }

    /* UART4_TX_B */
    ret = pinconf_set(PORT_12, PIN_2, PINMUX_ALTERNATE_FUNCTION_2, 0);

    return ret;
}

/**
 * @function    static void Hwsem_Thread(void *pvParameters)
 * @brief       TestApp to verify HWSEM interface
 *              Get the lock, send message through UART
 *              and then unlock
 * @note        none
 * @param       none
 * @retval      none
 */
static void Hwsem_Thread(void *pvParameters)
{
    int32_t ret, len, init = 1;
    char uart_msg[30];
    ARM_DRIVER_VERSION version;

    (void) pvParameters;

    version = HWSEMdrv->GetVersion();

    printf("\r\n HWSEM version api:%X driver:%X...\r\n",version.api, version.drv);

    /* Initialize HWSEM driver */
    ret = HWSEMdrv->Initialize(myHWSEM_callback);

    if (ret != ARM_DRIVER_OK)
    {
        printf("\r\n HWSEM initialization failed\r\n");
        goto error_exit;
    }

    while (1)
    {
        /* Acquire the lock */
        ret = HWSEMdrv->TryLock();

        if (ret == ARM_DRIVER_ERROR)
        {
            printf("\r\n HWSEM lock failed\r\n");
            goto error_uninitialize;
        }

        while (ret == ARM_DRIVER_ERROR_BUSY)
        {
            ret = xTaskNotifyWait(NULL, HWSEM_CB_EVENT, NULL, portMAX_DELAY);

            if (ret == pdTRUE)
            {
                ret = HWSEMdrv->TryLock();
            }
            else
            {
                goto error_uninitialize;
            }
        }

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\n HWSEM lock failed\n");
            goto error_uninitialize;
        }

        /* Initialize the pinmux in the first iteration */
        if (init)
        {
            ret = pinmux_init();

            if (ret != 0)
            {
                printf("\r\n Error in UART hardware_init.\r\n");
                goto error_unlock;
            }

            init = 0;
        }

        /* Initialize UART driver */
        ret = USARTdrv->Initialize(myUART_callback);

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error in UART Initialize.\r\n");
            goto error_unlock;
        }

        /* Power up UART peripheral */
        ret = USARTdrv->PowerControl(ARM_POWER_FULL);

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error in UART Power Up.\r\n");
            goto error_unlock;
        }

        /* Configure UART */
        ret =  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                                      ARM_USART_DATA_BITS_8 |
                                      ARM_USART_PARITY_NONE |
                                      ARM_USART_STOP_BITS_1 |
                                      ARM_USART_FLOW_CONTROL_NONE,
                                      115200);

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error in UART Control.\r\n");
            goto error_unlock;
        }

        /* Enable UART tx */
        ret =  USARTdrv->Control(ARM_USART_CONTROL_TX, 1);

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error in UART Control.\r\n");
            goto error_unlock;
        }


        ret = USARTdrv->Send(acq_msg, strlen(acq_msg));

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\nError in UART Send.\r\n");
            goto error_unlock;
        }

        /* wait for event flag after UART call */
        xTaskNotifyWait(NULL, UART_CB_TX_EVENT, NULL, portMAX_DELAY);

        /* Print 10 messages */
        for (int iter = 1; iter <= 10; iter++)
        {
            len = sprintf(uart_msg, "%s %d\r\n", msg, iter);

            ret = USARTdrv->Send(uart_msg, len);

            if (ret != ARM_DRIVER_OK)
            {
                printf("\r\nError in UART Send.\r\n");
                goto error_unlock;
            }

            /* wait for event flag after UART call */
            xTaskNotifyWait(NULL, UART_CB_TX_EVENT, NULL, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        ret = USARTdrv->Send(rel_msg, strlen(rel_msg));

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\nError in UART Send.\r\n");
            goto error_unlock;
        }

        /* wait for event flag after UART call */
        xTaskNotifyWait(NULL, UART_CB_TX_EVENT, NULL, portMAX_DELAY);

        ret = USARTdrv->PowerControl(ARM_POWER_OFF);

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error in UART Power OFF.\r\n");
            goto error_unlock;
        }

        ret = USARTdrv->Uninitialize();

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error in UART Uninitialize.\r\n");
            goto error_unlock;
        }

        /* Unlock the HW Semaphore */
        ret = HWSEMdrv->Unlock();

        if(ret == ARM_DRIVER_ERROR)
        {
            printf("\r\n HWSEM unlock failed\r\n");
            goto error_uninitialize;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    /* Uninitialize the HWSEM Driver */
    HWSEMdrv->Uninitialize();

    return;

error_unlock:
    /* Unlock the HW Semaphore */
    HWSEMdrv->Unlock();
error_uninitialize:
    /* Uninitialize the HWSEM Driver */
    HWSEMdrv->Uninitialize();
error_exit:

    return;
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start the FreeRTOS Kernel
 *---------------------------------------------------------------------------*/
int main(void)
{
    #if defined(RTE_Compiler_IO_STDOUT_User)
    int32_t ret;
    ret = stdout_init();
    if (ret != ARM_DRIVER_OK)
    {
        while (1)
        {
        }
    }
    #endif

    /* System Initialization */
    SystemCoreClockUpdate();

    /* Create application main thread */
    BaseType_t xReturned = xTaskCreate(Hwsem_Thread, "HwsemThread", 1024, NULL, configMAX_PRIORITIES-1, &Hwsem_xHandle);

    if (xReturned != pdPASS)
    {
        vTaskDelete(Hwsem_xHandle);
        return -1;
    }

    /* Start thread execution */
    vTaskStartScheduler();
}

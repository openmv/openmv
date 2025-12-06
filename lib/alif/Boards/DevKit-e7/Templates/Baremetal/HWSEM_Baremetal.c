/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     HWSEM0_Baremetal.c
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     27-June-2022
 * @brief    TestApp to verify HWSEM interface using baremetal, no operating system.
 *             It locks the HWSEM, sends the message through UART and then unlocks.
 *               This application will run on two cores
 *               to demonstrate the working of the Hardware Semaphore.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#include <RTE_Components.h>
#include CMSIS_device_header

#include <stdio.h>
#include <string.h>

#include "RTE_Device.h"
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

/* HWSEM Driver instance (HWSEM0-HWSEM15) */
#define HWSEM      0
#define HWSEM_CB_EVENT            1U << 0

/* Uart driver instance */
#define UART    4

volatile uint32_t event_flags_hwsem = 0;

/* UART Driver */
extern ARM_DRIVER_USART ARM_Driver_USART_(UART);
static ARM_DRIVER_USART *USARTdrv = &ARM_Driver_USART_(UART);

/* HWSEM Driver */
extern ARM_DRIVER_HWSEM ARM_Driver_HWSEM_(HWSEM);
static ARM_DRIVER_HWSEM *HWSEMdrv = &ARM_Driver_HWSEM_(HWSEM);

#if !RTE_UART4_BLOCKING_MODE_ENABLE

#define UART_CB_TX_EVENT          1U << 0
#define UART_CB_RX_EVENT          1U << 1
#define UART_CB_RX_TIMEOUT        1U << 2

volatile uint32_t event_flags_uart = 0;

/**
 * @function    static void myUART_callback(uint32_t event)
 * @brief       UART isr callabck
 * @note        none
 * @param       event: USART Event
 * @retval      none
 */
static void myUART_callback(uint32_t event)
{
    if (event & ARM_USART_EVENT_SEND_COMPLETE)
    {
        /* Send Success */
        event_flags_uart |= UART_CB_TX_EVENT;
    }

    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        /* Receive Success */
        event_flags_uart |= UART_CB_RX_EVENT;
    }

    if (event & ARM_USART_EVENT_RX_TIMEOUT)
    {
        /* Receive Success with rx timeout */
        event_flags_uart |= UART_CB_RX_TIMEOUT;
    }
}
#endif

/**
 * @function    static void myHWSEM_callback(int32_t event, uint8_t sem_id)
 * @brief       HWSEM isr callabck
 * @note        none
 * @param       event: HWSEM Available Event
 * @retval      none
 */
static void myHWSEM_callback(int32_t event, uint8_t sem_id)
{
    (void) sem_id;

    if (event & HWSEM_AVAILABLE_CB_EVENT)
    {
        /* HWSEM available */
        event_flags_hwsem |= HWSEM_CB_EVENT;
    }
}

/**
 * @function    static int32_t pinmux_setup(void)
 * @brief       pinmux setup
 * @note        none
 * @param       void
 * @retval      execution status
 */
static int32_t pinmux_setup(void)
{
    int32_t ret;

    /* UART4_RX_B */
    ret = pinconf_set(PORT_12, PIN_1, PINMUX_ALTERNATE_FUNCTION_2, PADCTRL_READ_ENABLE);

    if (ret)
    {
        return -1;
    }

    /* UART4_TX_B */
    ret = pinconf_set(PORT_12, PIN_2, PINMUX_ALTERNATE_FUNCTION_2, 0);

    if (ret)
    {
        return -1;
    }

    return 0;
}

/**
 * @function    void uart_hwsem_demo()
 * @brief       TestApp to verify HWSEM interface
 *              Get the lock, send message through UART
 *              and then unlock
 * @note        none
 * @param       none
 * @retval      none
 */
void uart_hwsem_demo()
{
    int32_t ret, len;
    char uart_msg[30];

    /* Initialize the HWSEM driver */
    ret = HWSEMdrv->Initialize(myHWSEM_callback);

    if (ret == ARM_DRIVER_ERROR)
    {
        printf("\r\n HWSEM initialization failed\r\n");
        goto error_exit;
    }

    while(1)
    {
        /* Try to get the lock */
        ret = HWSEMdrv->TryLock();

        if (ret == ARM_DRIVER_ERROR)
        {
            printf("\r\n HWSEM lock failed\r\n");
            goto error_uninitialize;
        }

        /* Loop until the lock becomes available */
        while (ret == ARM_DRIVER_ERROR_BUSY)
        {
            /* Wait for the interrupt callback event */
            while (!(event_flags_hwsem & HWSEM_CB_EVENT));

            /* Reset the flag */
            event_flags_hwsem &= ~HWSEM_CB_EVENT;

            /* Try to acquire the lock again */
            ret = HWSEMdrv->TryLock();
        }

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\n HWSEM lock failed\r\n");
            goto error_uninitialize;
        }

#if !RTE_UART4_BLOCKING_MODE_ENABLE
        /* Initialize UART driver */
        ret = USARTdrv->Initialize(myUART_callback);
#else
        /* Initialize UART driver */
        ret = USARTdrv->Initialize(NULL);
#endif
        if (ret)
        {
            printf("\r\n Error in UART Initialize.\r\n");
            goto error_exit;
        }

        /* Power up UART peripheral */
        ret = USARTdrv->PowerControl(ARM_POWER_FULL);

        if (ret)
        {
            printf("\r\n Error in UART Power Up.\r\n");
            goto error_exit;
        }

        /* Configure UART */
        ret =  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                                      ARM_USART_DATA_BITS_8 |
                                      ARM_USART_PARITY_NONE |
                                      ARM_USART_STOP_BITS_1 |
                                      ARM_USART_FLOW_CONTROL_NONE,
                                      115200);

        if (ret)
        {
            printf("\r\n Error in UART Control.\r\n");
            goto error_exit;
        }

        /* Enable UART tx */
        ret =  USARTdrv->Control(ARM_USART_CONTROL_TX, 1);

        if (ret)
        {
            printf("\r\n Error in UART Control.\r\n");
            goto error_exit;
        }

#if !RTE_UART4_BLOCKING_MODE_ENABLE
        event_flags_uart &= ~UART_CB_TX_EVENT;
#endif
        ret = USARTdrv->Send(acq_msg, strlen(acq_msg));

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\nError in UART Send.\r\n");
            goto error_exit;
        }

#if !RTE_UART4_BLOCKING_MODE_ENABLE
        /* wait for event flag after UART call */
        while (!(event_flags_uart & (UART_CB_TX_EVENT)));
#endif
        /* Print 10 messages */
        for (int iter = 1; iter <= 10; iter++)
        {
#if !RTE_UART4_BLOCKING_MODE_ENABLE
            event_flags_uart &= ~UART_CB_TX_EVENT;
#endif
            len = sprintf(uart_msg, "%s %d\r\n", msg, iter);

            ret = USARTdrv->Send(uart_msg, len);

            if (ret != ARM_DRIVER_OK)
            {
                printf("\r\nError in UART Send.\r\n");
                goto error_exit;
            }

#if !RTE_UART4_BLOCKING_MODE_ENABLE
            /* wait for event flag after UART call */
            while (!(event_flags_uart & (UART_CB_TX_EVENT)));
#endif
            for(uint32_t count = 0; count < 5; count++)
                sys_busy_loop_us(100000);
        }

#if !RTE_UART4_BLOCKING_MODE_ENABLE
        event_flags_uart &= ~UART_CB_TX_EVENT;
#endif
        ret = USARTdrv->Send(rel_msg, strlen(rel_msg));

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\nError in UART Send.\r\n");
            goto error_exit;
        }

#if !RTE_UART4_BLOCKING_MODE_ENABLE
        /* wait for event flag after UART call */
        while (!(event_flags_uart & (UART_CB_TX_EVENT)));
#endif
        ret = USARTdrv->PowerControl(ARM_POWER_OFF);

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error in UART Power OFF.\r\n");
            goto error_exit;
        }

        ret = USARTdrv->Uninitialize();

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error in UART Uninitialize.\r\n");
            goto error_exit;
        }

        /* Unlock the hw semaphore */
        ret = HWSEMdrv->Unlock();

        if (ret != ARM_DRIVER_OK)
        {
            printf("\r\nError in HWSEM Unlock.\r\n");
            goto error_uninitialize;
        }

        for(uint32_t count = 0; count < 5; count++)
            sys_busy_loop_us(100000);
    }

error_uninitialize:
    /* Uninitialize the HWSEM Driver */
    HWSEMdrv->Uninitialize();
error_exit:
    while(1);
}

int main()
{
    int32_t ret;
    ARM_DRIVER_VERSION version;

    #if defined(RTE_Compiler_IO_STDOUT_User)
    ret = stdout_init();
    if(ret != ARM_DRIVER_OK)
    {
        while(1)
        {
        }
    }
    #endif

    version = HWSEMdrv->GetVersion();
    printf("\r\n HWSEM version api:%X driver:%X...\r\n",version.api, version.drv);

    /* Initialize the HWSEM Driver */
    ret = HWSEMdrv->Initialize(myHWSEM_callback);

    if (ret != ARM_DRIVER_OK)
    {
        printf("\r\n HWSEM initialization failed\r\n");
        goto error_exit;
    }

    /* Try to get the lock */
    ret = HWSEMdrv->TryLock();

    if (ret == ARM_DRIVER_ERROR)
    {
        printf("\r\n HWSEM lock failed\r\n");
        goto error_uninitialize;
    }

    /* Loop until the lock becomes available */
    while (ret == ARM_DRIVER_ERROR_BUSY)
    {
        /* Wait for the interrupt callback event */
        while (!(event_flags_hwsem & HWSEM_CB_EVENT));

        /* Reset the flag */
        event_flags_hwsem &= ~HWSEM_CB_EVENT;

        /* Try to acquire the lock again */
        ret = HWSEMdrv->TryLock();
    }

    if (ret != ARM_DRIVER_OK)
    {
        printf("\r\n HWSEM lock failed\r\n");
        goto error_uninitialize;
    }

    ret = pinmux_setup();

    if (ret != 0)
    {
        printf("\r\n Error in pinmux_setup.\r\n");
        goto error_unlock;
    }

    /* Unlock the hw semaphore */
    ret = HWSEMdrv->Unlock();

    if (ret == ARM_DRIVER_ERROR)
    {
        printf("\r\n HWSEM unlock failed\r\n");
        goto error_uninitialize;
    }

    /* Uninitialize the HWSEM Driver */
    HWSEMdrv->Uninitialize();

    /* Print UART messages under hwsem lock */
    uart_hwsem_demo();

    return 0;

error_unlock:
    HWSEMdrv->Unlock();
error_uninitialize:
    HWSEMdrv->Uninitialize();
error_exit:
    while (1);
}

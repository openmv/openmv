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
 * @file     UART4_Baremetal.c
 * @author   Ahmad Rashed
 * @email    ahmad.rashed@alifsemi.com
 * @version  V1.0.1
 * @date     31-March-2023
 * @brief    TestApp to verify UART interface using baremetal, no operating system.
 *           UART interactive console application (using UART4 instance):
 *             UART waits for a char on serial terminal;
 *               if 'Enter' key is received; UART Sends back "Hello World!".
 *               else, it sends back received character.
 * @bug      None.
 * @Note     Updated for B0
 ******************************************************************************/

/* System Includes */
#include <stdio.h>

/* Project Includes */
/* include for UART Driver */
#include "Driver_USART.h"

/* PINMUX Driver */
#include "pinconf.h"

#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

#include "RTE_Device.h"
/*
 * UART4 CLK SOURCE: Defines UART4 clock source.
 *    <0=> CLK_38.4MHz (CLKEN_HFOSC)
 *    <1=> CLK_100MHz
 */
#if (RTE_UART4_CLK_SOURCE == 0) /* CLK_38.4MHz */
#include "se_services_port.h"
#endif


/* UART Driver instance (UART0-UART7) */
#define UART      4

/* UART Driver */
extern ARM_DRIVER_USART ARM_Driver_USART_(UART);

/* UART Driver instance */
static ARM_DRIVER_USART *USARTdrv = &ARM_Driver_USART_(UART);

void myUART_Thread_entry();

#define UART_CB_TX_EVENT          1U << 0
#define UART_CB_RX_EVENT          1U << 1
#define UART_CB_RX_TIMEOUT        1U << 2
volatile uint32_t event_flags_uart;


/**
 * @function    int hardware_init(void)
 * @brief       UART hardware pin initialization using PIN-MUX driver
 * @note        none
 * @param       void
 * @retval      0:success, -1:failure
 */
int hardware_init(void)
{
    /* UART4_RX_B */
    pinconf_set( PORT_12, PIN_1, PINMUX_ALTERNATE_FUNCTION_2, PADCTRL_READ_ENABLE);

    /* UART4_TX_B */
    pinconf_set( PORT_12, PIN_2, PINMUX_ALTERNATE_FUNCTION_2, 0);

    return 0;
}

/**
 * @function    void myUART_callback(uint32_t event)
 * @brief       UART isr callabck
 * @note        none
 * @param       event: USART Event
 * @retval      none
 */
void myUART_callback(uint32_t event)
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

/**
 * @function    void myUART_Thread_entry()
 * @brief       TestApp to verify UART interface
 *              UART interactive console application:
 *                  UART waits for a char on serial terminal;
 *                  if 'Enter' key is received; UART Sends back "Hello World!".
 *                  else, it sends back received character.
 * @note        none
 * @param       none
 * @retval      none
 */
void myUART_Thread_entry()
{
    char cmd;
    int32_t ret;
    ARM_DRIVER_VERSION version;
    event_flags_uart = 0;

#if (RTE_UART4_CLK_SOURCE == 0) /* CLK_38.4MHz */
    uint32_t service_error_code;
    uint32_t error_code = SERVICES_REQ_SUCCESS;

    /* Initialize the SE services */
    se_services_port_init();

    /* enable the HFOSC clock */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle,
                           /*clock_enable_t*/ CLKEN_HFOSC,
                           /*bool enable   */ true,
                                              &service_error_code);
    if(error_code)
        printf("SE: clk enable = %d\n", error_code);
#endif /* CLK_38.4MHz */

    printf("\r\n >>> UART testApp starting up!!!...<<< \r\n");

    version = USARTdrv->GetVersion();
    printf("\r\n UART version api:%X driver:%X...\r\n",version.api, version.drv);

    /* Initialize UART hardware pins using PinMux Driver. */
    ret = hardware_init();
    if(ret != 0)
    {
        printf("\r\n Error in UART hardware_init.\r\n");
        return;
    }

    /* Initialize UART driver */
    ret = USARTdrv->Initialize(myUART_callback);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Initialize.\r\n");
        return;
    }

    /* Power up UART peripheral */
    ret = USARTdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Power Up.\r\n");
        goto error_uninitialize;
    }

    /* Configure UART to 115200 Bits/sec */
    ret =  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                             ARM_USART_DATA_BITS_8       |
                             ARM_USART_PARITY_NONE       |
                             ARM_USART_STOP_BITS_1       |
                             ARM_USART_FLOW_CONTROL_NONE, 115200);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Control.\r\n");
        goto error_poweroff;
    }

    /* Enable Receiver and Transmitter lines */
    ret =  USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Control TX.\r\n");
        goto error_poweroff;
    }

    ret =  USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Control RX.\r\n");
        goto error_poweroff;
    }

    printf("\r\n Press Enter or any character on serial terminal to receive a message:\r\n");

    event_flags_uart &= ~UART_CB_TX_EVENT;
    ret = USARTdrv->Send("\r\nPress Enter or any character to receive a message\r\n", 53);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Send.\r\n");
        goto error_poweroff;
    }

    /* wait for event flag after UART call */
    while(!(event_flags_uart & (UART_CB_TX_EVENT)));

    while(1)
    {
        /* Get byte from UART - clear event flag before UART call */
        event_flags_uart &= ~(UART_CB_RX_EVENT | UART_CB_RX_TIMEOUT);
        ret = USARTdrv->Receive(&cmd, 1);
        if(ret != ARM_DRIVER_OK)
        {
            printf("\r\n Error in UART Receive.\r\n");
            goto error_poweroff;
        }

        /* wait for event flag after UART call */
        while(!(event_flags_uart & (UART_CB_RX_EVENT | UART_CB_RX_TIMEOUT)));

        if (event_flags_uart & (UART_CB_RX_EVENT))
        {
            /* Write byte(s) to UART - again, set event flag before UART call */
            event_flags_uart &= ~UART_CB_TX_EVENT;
            if (cmd == 13)
            {
                /* received 'Enter', send back "Hello World!". */
                ret = USARTdrv->Send("\r\nHello World!\r\n", 16);
            }
            else
            {
                /* else send back received character. */
                ret = USARTdrv->Send(&cmd, 1);
            }

            /* wait for event flag after UART call */
            while(!(event_flags_uart & (UART_CB_TX_EVENT)));
        }
    }

error_poweroff:
    /* Received error Power off UART peripheral */
    ret = USARTdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Power OFF.\r\n");
    }

error_uninitialize:
    /* Received error Un-initialize UART driver */
    ret = USARTdrv->Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        printf("\r\n Error in UART Uninitialize.\r\n");
    }

#if (RTE_UART4_CLK_SOURCE == 0) /* CLK_38.4MHz */
    /* disable the HFOSC clock */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle,
                       /*clock_enable_t*/ CLKEN_HFOSC,
                       /*bool enable   */ false,
                                          &service_error_code);
    if(error_code)
        printf("SE: clk enable = %d\n", error_code);
#endif

    printf("\r\n XXX UART demo thread exiting XXX...\r\n");
}

/* Define main entry point */
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
    hardware_init();
    myUART_Thread_entry();
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/

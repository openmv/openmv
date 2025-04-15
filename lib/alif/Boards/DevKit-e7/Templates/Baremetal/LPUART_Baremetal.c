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
 * @file     LPUART_Baremetal.c
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.1
 * @date     31-March-2023
 * @brief    Taken reference from UART4_Baremetal.c
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


/* LPUART Driver */
#define UART      LP

/* UART Driver */
extern ARM_DRIVER_USART ARM_Driver_USART_(UART);

/* UART Driver instance */
static ARM_DRIVER_USART *USARTdrv = &ARM_Driver_USART_(UART);

void myUART_Thread_entry();

#define UART_CB_TX_EVENT           (1U << 0)
#define UART_CB_RX_EVENT           (1U << 1)
#define UART_CB_RX_TIMEOUT         (1U << 2)
#define UART_CB_RX_BREAK           (1U << 3)
#define UART_CB_RX_FRAMING_ERROR   (1U << 4)
#define UART_CB_RX_PARITY_ERROR    (1U << 5)
#define UART_CB_RX_OVERFLOW        (1U << 6)

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
    /* LPUART_RX_A */
    pinconf_set( PORT_7, PIN_6, PINMUX_ALTERNATE_FUNCTION_2, PADCTRL_READ_ENABLE);

    /* LPUART_TX_A */
    pinconf_set( PORT_7, PIN_7, PINMUX_ALTERNATE_FUNCTION_2, 0);

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

    if (event & ARM_USART_EVENT_RX_BREAK)
    {
        /* Receive Break */
        event_flags_uart |= UART_CB_RX_BREAK;
    }

    if (event & ARM_USART_EVENT_RX_FRAMING_ERROR)
    {
        /* Receive Framing Error */
        event_flags_uart |= UART_CB_RX_FRAMING_ERROR;
    }

    if (event & ARM_USART_EVENT_RX_PARITY_ERROR)
    {
        /* Receive Parity Error */
        event_flags_uart |= UART_CB_RX_PARITY_ERROR;
    }

    if (event & ARM_USART_EVENT_RX_OVERFLOW)
    {
        /* Receive Overflow */
        event_flags_uart |= UART_CB_RX_OVERFLOW;
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

    printf("\r\n >>> LPUART testApp starting up!!!...<<< \r\n");

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
        while(!(event_flags_uart & (UART_CB_RX_EVENT | UART_CB_RX_TIMEOUT | UART_CB_RX_BREAK)));

        /* check for any RX error. */
        if (event_flags_uart & (UART_CB_RX_BREAK | UART_CB_RX_FRAMING_ERROR | \
                                UART_CB_RX_PARITY_ERROR | UART_CB_RX_OVERFLOW))
        {
            /* clear error flags. */
            if (event_flags_uart & (UART_CB_RX_BREAK))
            {
                printf("\r\n RX Break sequence detected.\r\n");
                event_flags_uart = event_flags_uart & (~(UART_CB_RX_BREAK));
            }

            if (event_flags_uart & (UART_CB_RX_FRAMING_ERROR))
            {
                printf("\r\n RX Framing Error.\r\n");
                event_flags_uart = event_flags_uart & (~(UART_CB_RX_FRAMING_ERROR));
            }

            if (event_flags_uart & (UART_CB_RX_PARITY_ERROR))
            {
                printf("\r\n RX Parity Error.\r\n");
                event_flags_uart = event_flags_uart & (~(UART_CB_RX_PARITY_ERROR));
            }

            if (event_flags_uart & (UART_CB_RX_OVERFLOW))
            {
                printf("\r\n RX Overflow Error.\r\n");
                event_flags_uart = event_flags_uart & (~(UART_CB_RX_OVERFLOW));
            }
        }

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
    myUART_Thread_entry();
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/

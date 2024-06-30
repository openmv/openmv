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
 * @file     stdin_USART.c
 * @author   Raj Ranjan
 * @email    raj.ranjan@alifsemi.com
 * @version  V1.0.0
 * @date     24-Aug-2023
 * @brief    STDIN USART Template
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#if defined(RTE_Compiler_IO_STDIN)
#include "retarget_stdin.h"
#endif  /* RTE_Compiler_IO_STDIN */
#include <RTE_Components.h>
#include CMSIS_device_header

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h>STDIN USART Interface

#if defined(RTE_Compiler_IO_STDIN_User)

/* UART Includes */
#include "retarget_config.h"
#include "Driver_USART.h"
#include "pinconf.h"

/* UART Driver */
extern ARM_DRIVER_USART ARM_Driver_USART_(PRINTF_UART_CONSOLE)  ;
static ARM_DRIVER_USART *USARTdrv   = &ARM_Driver_USART_(PRINTF_UART_CONSOLE);

/**
  @fn           void stdin_uart_error_uninitialize()
  @brief        UART un-initializtion:
  @return       none
*/
static int stdin_uart_error_uninitialize()
{
    int ret = -1;

    /* Un-initialize UART driver */
    ret = USARTdrv->Uninitialize();
    return ret;
}

/**
  @fn           int stdin_uart_error_power_off()
  @brief        UART power-off:
  @return       none
*/
static int stdin_uart_error_power_off()
{
    int ret = -1;

    /* Power off UART peripheral */
    ret = USARTdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK){
        return ret;
    }

    ret = stdin_uart_error_uninitialize();

    return ret;
}

/**
  Initialize stdin

  \return          0 on success, or -1 on error.
*/
int stdin_init(void)
{
    int32_t ret = -1;

    /*Initialize the USART driver */
    /* RX PIN */
    ret = pinconf_set(PRINTF_UART_CONSOLE_PORT_NUM,  PRINTF_UART_CONSOLE_RX_PIN,  \
            PRINTF_UART_CONSOLE_RX_PINMUX_FUNCTION, PRINTF_UART_CONSOLE_RX_PADCTRL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /*Initialize the USART driver */
    ret = USARTdrv->Initialize(NULL);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Enable the power for UART */
    ret = USARTdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        ret = stdin_uart_error_uninitialize();
        return ret;
    }

    ret =  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS            |
                ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE       |
                ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE,\
                PRINTF_UART_CONSOLE_BAUD_RATE);
    if(ret != ARM_DRIVER_OK)
    {
        ret = stdin_uart_error_power_off();
        return ret;
    }

    ret =  USARTdrv->Control(ARM_USART_CONTROL_RX, 1);  /* RX must be enable. */
    if(ret != ARM_DRIVER_OK)
    {
        ret = stdin_uart_error_power_off();
        return ret;
    }

    return ret;
}

/**
  Get a character from stdin

  \return     The next character from the input, or -1 on read error.
*/
int stdin_getchar(void)
{
    uint8_t buf[1];

    if (USARTdrv->Receive(buf, 1) != ARM_DRIVER_OK)
    {
        return (-1);
    }
    while (USARTdrv->GetRxCount() != 1);
    return (buf[0]);
}
#endif  /* defined(RTE_Compiler_IO_STDIN_User)  */
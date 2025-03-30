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
 * @file     stdout_USART.c
 * @author   Raj Ranjan
 * @email    raj.ranjan@alifsemi.com
 * @version  V1.0.0
 * @date     24-Aug-2023
 * @brief    STDOUT USART Template
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "RTE_Components.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */
#include CMSIS_device_header

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h>STDOUT USART Interface

#if defined(RTE_Compiler_IO_STDOUT_User)

/* UART Includes */
#include "retarget_config.h"
#include "Driver_USART.h"
#include "pinconf.h"

/* UART Driver */
extern ARM_DRIVER_USART ARM_Driver_USART_(PRINTF_UART_CONSOLE)  ;
static ARM_DRIVER_USART *USARTdrv   = &ARM_Driver_USART_(PRINTF_UART_CONSOLE);

/**
  @fn           void stdout_uart_error_uninitialize()
  @brief        UART un-initializtion:
  @return       none
*/
static int stdout_uart_error_uninitialize()
{
    int ret = -1;

    /* Un-initialize UART driver */
    ret = USARTdrv->Uninitialize();
    return ret;
}

/**
  @fn           int stdout_uart_error_power_off()
  @brief        UART power-off:
  @return       none
*/
static int stdout_uart_error_power_off()
{
    int ret = -1;

    /* Power off UART peripheral */
    ret = USARTdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK){
        return ret;
    }

    ret = stdout_uart_error_uninitialize();

    return ret;
}

/**
  Initialize stdout

  \return          0 on success, or -1 on error.
*/
int stdout_init()
{
    int32_t ret = -1;

    /*Initialize the USART driver */

    /* TX_PIN */
    ret = pinconf_set(PRINTF_UART_CONSOLE_PORT_NUM,  PRINTF_UART_CONSOLE_TX_PIN,  \
            PRINTF_UART_CONSOLE_TX_PINMUX_FUNCTION, PRINTF_UART_CONSOLE_TX_PADCTRL);
    if(ret != ARM_DRIVER_OK)
        return ret;

    /*Initialize the USART driver */
    ret = USARTdrv->Initialize(NULL); //polling without isr callback
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Enable the power for UART */
    ret = USARTdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        ret = stdout_uart_error_uninitialize();
        return ret;
    }

    ret =  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS            |
                ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE       |
                ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE,\
                PRINTF_UART_CONSOLE_BAUD_RATE);

    if(ret != ARM_DRIVER_OK)
    {
        ret = stdout_uart_error_power_off();
        return ret;
    }

    ret =  USARTdrv->Control(ARM_USART_CONTROL_TX, 1);  /* TX must be enable. */
    if(ret != ARM_DRIVER_OK)
    {
        ret = stdout_uart_error_power_off();
        return ret;
    }

    return ret;
}

/**
  Put a character to the stdout

  \param[in]   ch  Character to output
  \return          The character written, or -1 on write error.
*/
int stdout_putchar (int ch)
{
    uint8_t buf[1];

    buf[0] = (uint8_t) ch;
    if (USARTdrv->Send(buf, 1) != ARM_DRIVER_OK) {
        return (-1);
    }
    while (USARTdrv->GetTxCount() != 1);
    return (ch);
}
#endif /* defined(RTE_Compiler_IO_STDOUT_User) */

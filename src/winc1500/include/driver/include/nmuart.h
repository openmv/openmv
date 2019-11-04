/**
 *
 * \file
 *
 * \brief This module contains NMC1000 UART protocol bus APIs implementation.
 *
 * Copyright (c) 2016-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

#ifndef _NMUART_H_
#define _NMUART_H_

#include "common/include/nm_common.h"

/*
*	@fn			nm_uart_sync_cmd
*	@brief		Check COM Port
*	@return		ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
sint8 nm_uart_sync_cmd(void);
/**
*	@fn			nm_uart_read_reg
*	@brief		Read register
*	@param [in]	u32Addr
*				Register address
*	@return		Register value
*/
uint32 nm_uart_read_reg(uint32 u32Addr);

/**
*	@fn			nm_uart_read_reg_with_ret
*	@brief		Read register with error code return
*	@param [in]	u32Addr
*				Register address
*	@param [out]	pu32RetVal
*				Pointer to u32 variable used to return the read value
*	@return		ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
sint8 nm_uart_read_reg_with_ret(uint32 u32Addr, uint32* pu32RetVal);

/**
*	@fn			nm_uart_write_reg
*	@brief		write register
*	@param [in]	u32Addr
*				Register address
*	@param [in]	u32Val
*				Value to be written to the register
*	@return		ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
sint8 nm_uart_write_reg(uint32 u32Addr, uint32 u32Val);

/**
*	@fn			nm_uart_read_block
*	@brief		Read block of data
*	@param [in]	u32Addr
*				Start address
*	@param [out]	puBuf
*				Pointer to a buffer used to return the read data
*	@param [in]	u16Sz
*				Number of bytes to read. The buffer size must be >= u16Sz
*	@return		ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
sint8 nm_uart_read_block(uint32 u32Addr, uint8 *puBuf, uint16 u16Sz);

/**
*	@fn			nm_uart_write_block
*	@brief		Write block of data
*	@param [in]	u32Addr
*				Start address
*	@param [in]	puBuf
*				Pointer to the buffer holding the data to be written
*	@param [in]	u16Sz
*				Number of bytes to write. The buffer size must be >= u16Sz
*	@return		ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
sint8 nm_uart_write_block(uint32 u32Addr, uint8 *puBuf, uint16 u16Sz);

/**
*	@fn			nm_uart_reconfigure
*	@brief		Reconfigures the UART interface
*	@param [in]	ptr
*				Pointer to a DWORD containing baudrate at this moment.
*	@return		ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
sint8 nm_uart_reconfigure(void *ptr);
#endif /* _NMI2C_H_ */

/**
 *
 * \file
 *
 * \brief This module contains NMC1000 bus APIs implementation.
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
#ifndef CORTUS_APP

#include "driver/include/nmbus.h"
#include "driver/include/nmi2c.h"
#include "driver/include/nmspi.h"
#include "driver/include/nmuart.h"

#define MAX_TRX_CFG_SZ		8

/**
*	@fn		nm_bus_iface_init
*	@brief	Initialize bus interface
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@author	M. Abdelmawla
*	@date	11 July 2012
*	@version	1.0
*/
sint8 nm_bus_iface_init(void *pvInitVal)
{
	sint8 ret = M2M_SUCCESS;
	ret = nm_bus_init(pvInitVal);
	return ret;
}

/**
*	@fn		nm_bus_iface_deinit
*	@brief	Deinitialize bus interface
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@author	Samer Sarhan
*	@date	07 April 2014
*	@version	1.0
*/
sint8 nm_bus_iface_deinit(void)
{
	sint8 ret = M2M_SUCCESS;
	ret = nm_bus_deinit();

	return ret;
}

/**
*	@fn		nm_bus_reset
*	@brief	reset bus interface
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@version	1.0
*/
sint8 nm_bus_reset(void)
{
	sint8 ret = M2M_SUCCESS;
#ifdef CONF_WINC_USE_UART
#elif defined (CONF_WINC_USE_SPI)
	return nm_spi_reset();
#elif defined (CONF_WINC_USE_I2C)
#else
#error "Please define bus usage"
#endif

	return ret;
}

/**
*	@fn		nm_bus_iface_reconfigure
*	@brief	reconfigure bus interface
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@author	Viswanathan Murugesan
*	@date	22 Oct 2014
*	@version	1.0
*/
sint8 nm_bus_iface_reconfigure(void *ptr)
{
	sint8 ret = M2M_SUCCESS;
#ifdef CONF_WINC_USE_UART
	ret = nm_uart_reconfigure(ptr);
#endif
	return ret;
}
/*
*	@fn		nm_read_reg
*	@brief	Read register
*	@param [in]	u32Addr
*				Register address
*	@return	Register value
*	@author	M. Abdelmawla
*	@date	11 July 2012
*	@version	1.0
*/
uint32 nm_read_reg(uint32 u32Addr)
{
#ifdef CONF_WINC_USE_UART
	return nm_uart_read_reg(u32Addr);
#elif defined (CONF_WINC_USE_SPI)
	return nm_spi_read_reg(u32Addr);
#elif defined (CONF_WINC_USE_I2C)
	return nm_i2c_read_reg(u32Addr);
#else
#error "Please define bus usage"
#endif

}

/*
*	@fn		nm_read_reg_with_ret
*	@brief	Read register with error code return
*	@param [in]	u32Addr
*				Register address
*	@param [out]	pu32RetVal
*				Pointer to u32 variable used to return the read value
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@author	M. Abdelmawla
*	@date	11 July 2012
*	@version	1.0
*/
sint8 nm_read_reg_with_ret(uint32 u32Addr, uint32* pu32RetVal)
{
#ifdef CONF_WINC_USE_UART
	return nm_uart_read_reg_with_ret(u32Addr,pu32RetVal);
#elif defined (CONF_WINC_USE_SPI)
	return nm_spi_read_reg_with_ret(u32Addr,pu32RetVal);
#elif defined (CONF_WINC_USE_I2C)
	return nm_i2c_read_reg_with_ret(u32Addr,pu32RetVal);
#else
#error "Please define bus usage"
#endif
}

/*
*	@fn		nm_write_reg
*	@brief	write register
*	@param [in]	u32Addr
*				Register address
*	@param [in]	u32Val
*				Value to be written to the register
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@author	M. Abdelmawla
*	@date	11 July 2012
*	@version	1.0
*/
sint8 nm_write_reg(uint32 u32Addr, uint32 u32Val)
{
#ifdef CONF_WINC_USE_UART
	return nm_uart_write_reg(u32Addr,u32Val);
#elif defined (CONF_WINC_USE_SPI)
	return nm_spi_write_reg(u32Addr,u32Val);
#elif defined (CONF_WINC_USE_I2C)
	return nm_i2c_write_reg(u32Addr,u32Val);
#else
#error "Please define bus usage"
#endif
}

static sint8 p_nm_read_block(uint32 u32Addr, uint8 *puBuf, uint16 u16Sz)
{
#ifdef CONF_WINC_USE_UART
	return nm_uart_read_block(u32Addr,puBuf,u16Sz);
#elif defined (CONF_WINC_USE_SPI)
	return nm_spi_read_block(u32Addr,puBuf,u16Sz);
#elif defined (CONF_WINC_USE_I2C)
	return nm_i2c_read_block(u32Addr,puBuf,u16Sz);
#else
#error "Please define bus usage"
#endif

}
/*
*	@fn		nm_read_block
*	@brief	Read block of data
*	@param [in]	u32Addr
*				Start address
*	@param [out]	puBuf
*				Pointer to a buffer used to return the read data
*	@param [in]	u32Sz
*				Number of bytes to read. The buffer size must be >= u32Sz
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@author	M. Abdelmawla
*	@date	11 July 2012
*	@version	1.0
*/ 
sint8 nm_read_block(uint32 u32Addr, uint8 *puBuf, uint32 u32Sz)
{
	uint16 u16MaxTrxSz = egstrNmBusCapabilities.u16MaxTrxSz - MAX_TRX_CFG_SZ;
	uint32 off = 0;
	sint8 s8Ret = M2M_SUCCESS;

	for(;;)
	{
		if(u32Sz <= u16MaxTrxSz)
		{
			s8Ret += p_nm_read_block(u32Addr, &puBuf[off], (uint16)u32Sz);	
			break;
		}
		else
		{
			s8Ret += p_nm_read_block(u32Addr, &puBuf[off], u16MaxTrxSz);
			if(M2M_SUCCESS != s8Ret) break;
			u32Sz -= u16MaxTrxSz;
			off += u16MaxTrxSz;
			u32Addr += u16MaxTrxSz;
		}
	}

	return s8Ret;
}

static sint8 p_nm_write_block(uint32 u32Addr, uint8 *puBuf, uint16 u16Sz)
{
#ifdef CONF_WINC_USE_UART
	return nm_uart_write_block(u32Addr,puBuf,u16Sz);
#elif defined (CONF_WINC_USE_SPI)
	return nm_spi_write_block(u32Addr,puBuf,u16Sz);
#elif defined (CONF_WINC_USE_I2C)
	return nm_i2c_write_block(u32Addr,puBuf,u16Sz);
#else
#error "Please define bus usage"
#endif

}
/**
*	@fn		nm_write_block
*	@brief	Write block of data
*	@param [in]	u32Addr
*				Start address
*	@param [in]	puBuf
*				Pointer to the buffer holding the data to be written
*	@param [in]	u32Sz
*				Number of bytes to write. The buffer size must be >= u32Sz
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@author	M. Abdelmawla
*	@date	11 July 2012
*	@version	1.0
*/ 
sint8 nm_write_block(uint32 u32Addr, uint8 *puBuf, uint32 u32Sz)
{
	uint16 u16MaxTrxSz = egstrNmBusCapabilities.u16MaxTrxSz - MAX_TRX_CFG_SZ;
	uint32 off = 0;
	sint8 s8Ret = M2M_SUCCESS;

	for(;;)
	{
		if(u32Sz <= u16MaxTrxSz)
		{
			s8Ret += p_nm_write_block(u32Addr, &puBuf[off], (uint16)u32Sz);	
			break;
		}
		else
		{
			s8Ret += p_nm_write_block(u32Addr, &puBuf[off], u16MaxTrxSz);
			if(M2M_SUCCESS != s8Ret) break;
			u32Sz -= u16MaxTrxSz;
			off += u16MaxTrxSz;
			u32Addr += u16MaxTrxSz;
		}
	}

	return s8Ret;
}

#endif


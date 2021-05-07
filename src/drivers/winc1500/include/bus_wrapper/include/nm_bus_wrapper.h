/**
 *
 * \file
 *
 * \brief This module contains NMC1000 bus wrapper APIs declarations.
 *
 * Copyright (c) 2016-2021 Microchip Technology Inc. and its subsidiaries.
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

#ifndef _NM_BUS_WRAPPER_H_
#define _NM_BUS_WRAPPER_H_

#include "common/include/nm_common.h"

/**
	BUS Type
**/
#define  NM_BUS_TYPE_I2C	((uint8)0)
#define  NM_BUS_TYPE_SPI	((uint8)1)
#define  NM_BUS_TYPE_UART	((uint8)2)
/**
	IOCTL commands
**/
#define NM_BUS_IOCTL_R			((uint8)0)	/*!< Read only ==> I2C/UART. Parameter:tstrNmI2cDefault/tstrNmUartDefault */
#define NM_BUS_IOCTL_W			((uint8)1)	/*!< Write only ==> I2C/UART. Parameter type tstrNmI2cDefault/tstrNmUartDefault*/
#define NM_BUS_IOCTL_W_SPECIAL	((uint8)2)	/*!< Write two buffers within the same transaction
												(same start/stop conditions) ==> I2C only. Parameter:tstrNmI2cSpecial */
#define NM_BUS_IOCTL_RW			((uint8)3)	/*!< Read/Write at the same time ==> SPI only. Parameter:tstrNmSpiRw */

#define NM_BUS_IOCTL_WR_RESTART	((uint8)4)				/*!< Write buffer then made restart condition then read ==> I2C only. parameter:tstrNmI2cSpecial */ 
/**
*	@struct	tstrNmBusCapabilities
*	@brief	Structure holding bus capabilities information
*	@sa	NM_BUS_TYPE_I2C, NM_BUS_TYPE_SPI
*/ 
typedef struct
{
	uint16	u16MaxTrxSz;	/*!< Maximum transfer size. Must be >= 16 bytes*/
} tstrNmBusCapabilities;

/**
*	@struct	tstrNmI2cDefault
*	@brief	Structure holding I2C default operation parameters
*	@sa		NM_BUS_IOCTL_R, NM_BUS_IOCTL_W
*/ 
typedef struct
{
	uint8 u8SlaveAdr;
	uint8	*pu8Buf;	/*!< Operation buffer */
	uint16	u16Sz;		/*!< Operation size */
} tstrNmI2cDefault;

/**
*	@struct	tstrNmI2cSpecial
*	@brief	Structure holding I2C special operation parameters
*	@sa		NM_BUS_IOCTL_W_SPECIAL
*/ 
typedef struct
{
	uint8 u8SlaveAdr;
	uint8	*pu8Buf1;	/*!< pointer to the 1st buffer */
	uint8	*pu8Buf2;	/*!< pointer to the 2nd buffer */	
	uint16	u16Sz1;		/*!< 1st buffer size */
	uint16	u16Sz2;		/*!< 2nd buffer size */
} tstrNmI2cSpecial;

/**
*	@struct	tstrNmSpiRw
*	@brief	Structure holding SPI R/W parameters
*	@sa		NM_BUS_IOCTL_RW
*/ 
typedef struct
{
	uint8	*pu8InBuf;		/*!< pointer to input buffer. 
							Can be set to null and in this case zeros should be sent at MOSI */
	uint8	*pu8OutBuf;		/*!< pointer to output buffer. 
							Can be set to null and in this case data from MISO can be ignored  */
	uint16	u16Sz;			/*!< Transfere size */	
} tstrNmSpiRw;


/**
*	@struct	tstrNmUartDefault
*	@brief	Structure holding UART default operation parameters
*	@sa		NM_BUS_IOCTL_R, NM_BUS_IOCTL_W
*/ 
typedef struct
{
	uint8	*pu8Buf;	/*!< Operation buffer */
	uint16	u16Sz;		/*!< Operation size */
} tstrNmUartDefault;
/*!< Bus capabilities. This structure must be declared at platform specific bus wrapper */
extern tstrNmBusCapabilities egstrNmBusCapabilities;


#ifdef __cplusplus
     extern "C" {
 #endif
/**
*	@fn		nm_bus_init
*	@brief	Initialize the bus wrapper
*	@return	ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/ 
sint8 nm_bus_init(void *);

/**
*	@fn		nm_bus_ioctl
*	@brief	send/receive from the bus
*	@param [in]	u8Cmd
*					IOCTL command for the operation
*	@param [in]	pvParameter
*					Arbitrary parameter depending on IOCTL
*	@return	ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@note	For SPI only, it's important to be able to send/receive at the same time
*/ 
sint8 nm_bus_ioctl(uint8 u8Cmd, void* pvParameter);

/**
*	@fn		nm_bus_deinit
*	@brief	De-initialize the bus wrapper
*	@return	ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/ 
sint8 nm_bus_deinit(void);

/*
*	@fn			nm_bus_reinit
*	@brief		re-initialize the bus wrapper
*	@param [in]	void *config
*					re-init configuration data
*	@return		ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
sint8 nm_bus_reinit(void *);
/*
*	@fn			nm_bus_get_chip_type
*	@brief		get chip type
*	@return		ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
*/ 
#ifdef CONF_WINC_USE_UART
uint8 nm_bus_get_chip_type(void);
sint8 nm_bus_break(void);
#endif

/**
 *  @fn         spi_rw
 *  @brief      Process SPI Read/Write operation
 *  @param      pu8Mosi TX Data buffer
 *  @param      pu8Miso RX Data buffer
 *  @param      u16Sz Transfer length
 *  @return     ZERO in case of success and M2M_ERR_BUS_FAIL in case of failure
 */
#ifdef CONF_WINC_USE_SPI
sint8 nm_spi_rw(uint8* pu8Mosi, uint8* pu8Miso, uint16 u16Sz);
#endif

#ifdef __cplusplus
	 }
 #endif

#endif	/*_NM_BUS_WRAPPER_H_*/

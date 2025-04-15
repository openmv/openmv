/**
 *
 * \file
 *
 * \brief This module contains NMC1500 M2M driver APIs declarations.
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

#ifndef _NMDRV_H_
#define _NMDRV_H_

#include "common/include/nm_common.h"

/**
*  @struct		tstrM2mRev
*  @brief		Structure holding firmware version parameters and build date/time
*/
typedef struct {
	uint32 u32Chipid; /* HW revision which will be basically the chip ID */
	uint8 u8FirmwareMajor; /* Version Major Number which represents the official release base */
	uint8 u8FirmwareMinor; /* Version Minor Number which represents the engineering release base */
	uint8 u8FirmwarePatch;	/* Version patch Number which represents the patches release base */
	uint8 u8DriverMajor; /* Version Major Number which represents the official release base */
	uint8 u8DriverMinor; /* Version Minor Number which represents the engineering release base */
	uint8 u8DriverPatch; /* Version Patch Number which represents the patches release base */
	uint8 BuildDate[sizeof(__DATE__)];
	uint8 BuildTime[sizeof(__TIME__)];
	uint8 _PAD8_;
	uint16 u16FirmwareSvnNum;
	uint16 _PAD16_[2];
} tstrM2mRev;

/**
*  @struct		tstrM2mBinaryHeader
*  @brief		Structure holding compatibility version info for firmware binaries
*/
typedef struct {
	tstrM2mRev binVerInfo;
    uint32	   flashOffset;
	uint32     payloadSize;
} tstrM2mBinaryHeader;

#ifdef __cplusplus
     extern "C" {
 #endif
/**
*	@fn		nm_get_firmware_info(tstrM2mRev* M2mRev)
*	@brief	Get Firmware version info
*	@param [out]	M2mRev
*			    pointer holds address of structure "tstrM2mRev" that contains the firmware version parameters
*	@version	1.0
*/
sint8 nm_get_firmware_info(tstrM2mRev* M2mRev);
/**
*	@fn		nm_get_firmware_full_info(tstrM2mRev* pstrRev)
*	@brief	Get Firmware version info
*	@param [out]	M2mRev
*			    pointer holds address of structure "tstrM2mRev" that contains the firmware version parameters
*	@version	1.0
*/
sint8 nm_get_firmware_full_info(tstrM2mRev* pstrRev);
/**
*	@fn		nm_get_ota_firmware_info(tstrM2mRev* pstrRev)
*	@brief	Get Firmware version info
*	@param [out]	M2mRev
*			    pointer holds address of structure "tstrM2mRev" that contains the firmware version parameters
			
*	@version	1.0
*/
sint8 nm_get_ota_firmware_info(tstrM2mRev* pstrRev);
/*
*	@fn		nm_drv_init
*	@brief	Initialize NMC1000 driver
*	@return	ZERO in case of success and Negative error code in case of failure
*/
sint8 nm_drv_init_download_mode(void);

/*
*	@fn		nm_drv_init
*	@brief	Initialize NMC1000 driver
*	@return	M2M_SUCCESS in case of success and Negative error code in case of failure
*   @param [in]	arg
*				Generic argument TBD
*	@return	ZERO in case of success and Negative error code in case of failure

*/
sint8 nm_drv_init(void * arg);

/*
*	@fn		nm_drv_init_hold
*	@brief	First part of nm_drv_init, up to the point of initializing spi for flash access.
*	@see	nm_drv_init
*	@return	M2M_SUCCESS in case of success and Negative error code in case of failure
*	@param [in]	req_serial_number
*				Parameter inherited from nm_drv_init
*	@return	ZERO in case of success and Negative error code in case of failure
*/
sint8 nm_drv_init_hold(void);

/*
*	@fn		nm_drv_init_start
*	@brief	Second part of nm_drv_init, continuing from where nm_drv_init_hold left off.
*	@see	nm_drv_init
*	@return	M2M_SUCCESS in case of success and Negative error code in case of failure
*	@param [in]	arg
*				Parameter inherited from nm_drv_init
*	@return	ZERO in case of success and Negative error code in case of failure
*/
sint8 nm_drv_init_start(void * arg);

/**
*	@fn		nm_drv_deinit
*	@brief	Deinitialize NMC1000 driver
*	@author	M. Abdelmawla
*   @param [in]	arg
*				Generic argument TBD
*	@return	ZERO in case of success and Negative error code in case of failure
*/
sint8 nm_drv_deinit(void * arg);

#ifdef __cplusplus
	 }
 #endif

#endif	/*_NMDRV_H_*/



/*******************************************************************************
  This module contains WINC1500 M2M driver APIs implementation.

  File Name:
    nmdrv.h

  Summary:
    This module contains WINC1500 M2M driver APIs implementation.

  Description:
    This module contains WINC1500 M2M driver APIs implementation.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2022, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/
// DOM-IGNORE-END

#ifndef _NMDRV_H_
#define _NMDRV_H_

#include "nm_common.h"

/**
*  @struct      tstrM2mRev
*  @brief       Structure holding firmware version parameters and build date/time
*/
typedef struct {
    uint32_t u32Chipid; /* HW revision which will be basically the chip ID */
    uint8_t u8FirmwareMajor; /* Version Major Number which represents the official release base */
    uint8_t u8FirmwareMinor; /* Version Minor Number which represents the engineering release base */
    uint8_t u8FirmwarePatch;    /* Version patch Number which represents the patches release base */
    uint8_t u8DriverMajor; /* Version Major Number which represents the official release base */
    uint8_t u8DriverMinor; /* Version Minor Number which represents the engineering release base */
    uint8_t u8DriverPatch; /* Version Patch Number which represents the patches release base */
    uint8_t BuildDate[sizeof(__DATE__)];
    uint8_t BuildTime[sizeof(__TIME__)];
    uint8_t _PAD8_;
    uint16_t u16FirmwareSvnNum;
    uint16_t _PAD16_[2];
} tstrM2mRev;

/**
*  @struct      tstrM2mBinaryHeader
*  @brief       Structure holding compatibility version info for firmware binaries
*/
typedef struct {
    tstrM2mRev binVerInfo;
    uint32_t       flashOffset;
    uint32_t     payloadSize;
} tstrM2mBinaryHeader;

#ifdef __cplusplus
     extern "C" {
 #endif
/**
*   @fn     nm_get_firmware_info(tstrM2mRev* M2mRev)
*   @brief  Get Firmware version info
*   @param [out]    M2mRev
*               pointer holds address of structure "tstrM2mRev" that contains the firmware version parameters
*   @version    1.0
*/
int8_t nm_get_firmware_info(tstrM2mRev* M2mRev);
/**
*   @fn     nm_get_firmware_full_info(tstrM2mRev* pstrRev)
*   @brief  Get Firmware version info
*   @param [out]    M2mRev
*               pointer holds address of structure "tstrM2mRev" that contains the firmware version parameters
*   @version    1.0
*/
int8_t nm_get_firmware_full_info(tstrM2mRev* pstrRev);
/**
*   @fn     nm_get_ota_firmware_info(tstrM2mRev* pstrRev)
*   @brief  Get Firmware version info
*   @param [out]    M2mRev
*               pointer holds address of structure "tstrM2mRev" that contains the firmware version parameters

*   @version    1.0
*/
int8_t nm_get_ota_firmware_info(tstrM2mRev* pstrRev);
/*
*   @fn     nm_drv_init
*   @brief  Initialize NMC1000 driver
*   @return ZERO in case of success and Negative error code in case of failure
*/
int8_t nm_drv_init_download_mode(void);

/*
*   @fn     nm_drv_init
*   @brief  Initialize NMC1000 driver
*   @return M2M_SUCCESS in case of success and Negative error code in case of failure
*   @param [in] arg
*               Generic argument TBD
*   @return ZERO in case of success and Negative error code in case of failure

*/
int8_t nm_drv_init(void * arg);

/*
*	@fn		nm_drv_init_hold
*	@brief	First part of nm_drv_init, up to the point of initializing spi for flash access.
*	@see	nm_drv_init
*	@return	M2M_SUCCESS in case of success and Negative error code in case of failure
*	@param [in]	req_serial_number
*				Parameter inherited from nm_drv_init
*	@return	ZERO in case of success and Negative error code in case of failure
*/
int8_t nm_drv_init_hold(void);

/*
*	@fn		nm_drv_init_start
*	@brief	Second part of nm_drv_init, continuing from where nm_drv_init_hold left off.
*	@see	nm_drv_init
*	@return	M2M_SUCCESS in case of success and Negative error code in case of failure
*	@param [in]	arg
*				Parameter inherited from nm_drv_init
*	@return	ZERO in case of success and Negative error code in case of failure
*/
int8_t nm_drv_init_start(void * arg);

/**
*   @fn     nm_drv_deinit
*   @brief  Deinitialize NMC1000 driver
*   @author M. Abdelmawla
*   @param [in] arg
*               Generic argument TBD
*   @return ZERO in case of success and Negative error code in case of failure
*/
int8_t nm_drv_deinit(void * arg);

#ifdef __cplusplus
     }
 #endif

#endif  /*_NMDRV_H_*/



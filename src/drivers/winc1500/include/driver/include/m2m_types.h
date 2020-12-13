/**
 *
 * \file
 *
 * \brief WINC Application Interface Internal Types.
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

#ifndef __M2M_WIFI_TYPES_H__
#define __M2M_WIFI_TYPES_H__


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
#ifndef	_BOOT_
#ifndef _FIRMWARE_
#include "common/include/nm_common.h"
#else
#ifndef LINT
#include "m2m_common.h"
#else
#include "../../../firmware/wifi_v111/src/m2m/include/m2m_common.h"
#endif
#endif
#endif


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/**@addtogroup  VERSIONDEF
 */
/**@{*/
#define M2M_MAJOR_SHIFT (8)
#define M2M_MINOR_SHIFT (4)
#define M2M_PATCH_SHIFT (0)

#define M2M_DRV_VERSION_SHIFT (16)
#define M2M_FW_VERSION_SHIFT (0)

#define M2M_GET_MAJOR(ver_info_hword) ((uint8)((ver_info_hword) >> M2M_MAJOR_SHIFT) & 0xff)
#define M2M_GET_MINOR(ver_info_hword) ((uint8)((ver_info_hword) >> M2M_MINOR_SHIFT) & 0x0f)
#define M2M_GET_PATCH(ver_info_hword) ((uint8)((ver_info_hword) >> M2M_PATCH_SHIFT) & 0x0f)

#define M2M_GET_FW_VER(ver_info_word)  ((uint16) ((ver_info_word) >> M2M_FW_VERSION_SHIFT))
#define M2M_GET_DRV_VER(ver_info_word) ((uint16) ((ver_info_word) >> M2M_DRV_VERSION_SHIFT))

#define M2M_GET_DRV_MAJOR(ver_info_word) M2M_GET_MAJOR(M2M_GET_DRV_VER(ver_info_word))
#define M2M_GET_DRV_MINOR(ver_info_word) M2M_GET_MINOR(M2M_GET_DRV_VER(ver_info_word))
#define M2M_GET_DRV_PATCH(ver_info_word) M2M_GET_PATCH(M2M_GET_DRV_VER(ver_info_word))

#define M2M_GET_FW_MAJOR(ver_info_word) M2M_GET_MAJOR(M2M_GET_FW_VER(ver_info_word))
#define M2M_GET_FW_MINOR(ver_info_word) M2M_GET_MINOR(M2M_GET_FW_VER(ver_info_word))
#define M2M_GET_FW_PATCH(ver_info_word) M2M_GET_PATCH(M2M_GET_FW_VER(ver_info_word))

#define M2M_MAKE_VERSION(major, minor, patch) ( \
	((uint16)((major)  & 0xff)  << M2M_MAJOR_SHIFT) | \
	((uint16)((minor)  & 0x0f)  << M2M_MINOR_SHIFT) | \
	((uint16)((patch)  & 0x0f)  << M2M_PATCH_SHIFT))

#define M2M_MAKE_VERSION_INFO(fw_major, fw_minor, fw_patch, drv_major, drv_minor, drv_patch) \
	( \
	( ((uint32)M2M_MAKE_VERSION((fw_major),  (fw_minor),  (fw_patch)))  << M2M_FW_VERSION_SHIFT) | \
	( ((uint32)M2M_MAKE_VERSION((drv_major), (drv_minor), (drv_patch))) << M2M_DRV_VERSION_SHIFT))

#define REL_19_7_0_VER			M2M_MAKE_VERSION_INFO(19,7,0,19,3,0)
#define REL_19_6_0_VER			M2M_MAKE_VERSION_INFO(19,6,0,19,3,0)
#define REL_19_5_3_VER			M2M_MAKE_VERSION_INFO(19,5,3,19,3,0)
#define REL_19_5_2_VER			M2M_MAKE_VERSION_INFO(19,5,2,19,3,0)
#define REL_19_5_1_VER			M2M_MAKE_VERSION_INFO(19,5,1,19,3,0)
#define REL_19_5_0_VER			M2M_MAKE_VERSION_INFO(19,5,0,19,3,0)
#define REL_19_4_6_VER			M2M_MAKE_VERSION_INFO(19,4,6,19,3,0)
#define REL_19_4_5_VER			M2M_MAKE_VERSION_INFO(19,4,5,19,3,0)
#define REL_19_4_4_VER			M2M_MAKE_VERSION_INFO(19,4,4,19,3,0)
#define REL_19_4_3_VER			M2M_MAKE_VERSION_INFO(19,4,3,19,3,0)
#define REL_19_4_2_VER			M2M_MAKE_VERSION_INFO(19,4,2,19,3,0)
#define REL_19_4_1_VER			M2M_MAKE_VERSION_INFO(19,4,1,19,3,0)
#define REL_19_4_0_VER			M2M_MAKE_VERSION_INFO(19,4,0,19,3,0)
#define REL_19_3_1_VER			M2M_MAKE_VERSION_INFO(19,3,1,19,3,0)
#define REL_19_3_0_VER			M2M_MAKE_VERSION_INFO(19,3,0,19,3,0)
#define REL_19_2_2_VER			M2M_MAKE_VERSION_INFO(19,2,2,19,2,0)
#define REL_19_2_1_VER			M2M_MAKE_VERSION_INFO(19,2,1,19,2,0)
#define REL_19_2_0_VER			M2M_MAKE_VERSION_INFO(19,2,0,19,2,0)
#define REL_19_1_0_VER			M2M_MAKE_VERSION_INFO(19,1,0,18,2,0)
#define REL_19_0_0_VER			M2M_MAKE_VERSION_INFO(19,0,0,18,1,1)

/*======*======*======*======*
		FIRMWARE VERSION NO INFO
 *======*======*======*======*/

#define M2M_RELEASE_VERSION_MAJOR_NO 						(19)
/*!< Firmware Major release version number.
*/


#ifndef BLDTESTVERSION1
// the real version number must appear first as the release script greps it out to create folder.
#define M2M_RELEASE_VERSION_MINOR_NO						(6)
#else
#define M2M_RELEASE_VERSION_MINOR_NO						(8)
#endif
/*!< Firmware Minor release version number.
*/

#define M2M_RELEASE_VERSION_PATCH_NO                        (1)
/*!< Firmware patch release version number.
*/

#define M2M_RELEASE_VERSION_SVN_VERSION						(SVN_REVISION)
/*!< Firmware SVN release version number.
 */

/*======*======*======*======*
  SUPPORTED DRIVER VERSION NO INFO
 *======*======*======*======*/

#define	M2M_MIN_REQ_DRV_VERSION_MAJOR_NO 						(19)
/*!< Driver Major release version number.
*/


#define M2M_MIN_REQ_DRV_VERSION_MINOR_NO						(3)
/*!< Driver Minor release version number.
*/

#define M2M_MIN_REQ_DRV_VERSION_PATCH_NO						(0)
/*!< Driver patch release version number.
*/

#define M2M_MIN_REQ_DRV_SVN_VERSION								(0)
/*!< Driver svn version.
*/

#if !defined(M2M_RELEASE_VERSION_MAJOR_NO) || !defined(M2M_RELEASE_VERSION_MINOR_NO)
#error Undefined version number
#endif

/**@}*/     //VERSIONDEF

/**@addtogroup  WlanDefines
 * @{
 */

#define M2M_BUFFER_MAX_SIZE								(1600UL - 4)
/*!< Maximum size for the shared packet buffer.
 */


#define M2M_MAC_ADDRES_LEN                               6
/*!< The size fo 802 MAC address.
 */

#define M2M_ETHERNET_HDR_OFFSET							34
/*!< The offset of the Ethernet header within the WLAN Tx Buffer.
 */


#define M2M_ETHERNET_HDR_LEN							14
/*!< Length of the Ethernet header in bytes.
*/


#define M2M_MAX_SSID_LEN 								33
/*!< 1 more than the max SSID length.
	This matches the size of SSID buffers (max SSID length + 1-byte length field).
 */


#define M2M_MAX_PSK_LEN           						65
/*!< 1 more than the WPA PSK length (in ASCII format).
	This matches the size of the WPA PSK/Passphrase buffer (max ASCII contents + 1-byte length field).
	Alternatively it matches the WPA PSK length (in ASCII format) + 1 byte NULL termination.
 */

#define M2M_MIN_PSK_LEN           						9
/*!< 1 more than the minimum WPA PSK Passphrase length.
	It matches the minimum WPA PSK Passphrase length + 1 byte NULL termination.
 */

#define M2M_DEVICE_NAME_MAX								48
/*!< Maximum Size for the device name including the NULL termination.
 */

#define M2M_NTP_MAX_SERVER_NAME_LENGTH					32
/*!< Maximum NTP server name length
*/

#define M2M_LISTEN_INTERVAL 							1
/*!< The STA uses the Listen Interval parameter to indicate to the AP how
	many beacon intervals it shall sleep before it retrieves the queued frames
	from the AP. 
*/

#define MAX_HIDDEN_SITES 								4
/*!<
	max number of hidden SSID supported by scan request
*/

#define M2M_CUST_IE_LEN_MAX								252
/*!< The maximum size of IE (Information Element).
*/

#define M2M_CRED_STORE_FLAG								0x01
/*!< Flag used in @ref tstrM2mConnCredHdr to indicate that Wi-Fi connection
	credentials should be stored in WINC flash.
*/
#define M2M_CRED_ENCRYPT_FLAG							0x02
/*!< Flag used in @ref tstrM2mConnCredHdr to indicate that Wi-Fi connection
	credentials should be encrypted when stored in WINC flash.
*/
#define M2M_CRED_IS_STORED_FLAG							0x10
/*!< Flag used in @ref tstrM2mConnCredHdr to indicate that Wi-Fi connection
	credentials are stored in WINC flash. May only be set by WINC firmware.
*/
#define M2M_CRED_IS_ENCRYPTED_FLAG						0x20
/*!< Flag used in @ref tstrM2mConnCredHdr to indicate that Wi-Fi connection
	credentials are encrypted in WINC flash. May only be set by WINC firmware.
*/

#define M2M_WIFI_CONN_BSSID_FLAG						0x01
/*!< Flag used in @ref tstrM2mConnCredCmn to indicate that Wi-Fi connection
	must be restricted to an AP with a certain BSSID.
*/

#define M2M_AUTH_1X_USER_LEN_MAX						132
/*!< The maximum length (in ASCII characters) of domain name + username (including '@' or '\')
	for authentication with Enterprise methods.
*/
#define M2M_AUTH_1X_PASSWORD_LEN_MAX					256
/*!< The maximum length (in ASCII characters) of password for authentication with Enterprise MSCHAPv2 methods.
*/
#define M2M_AUTH_1X_PRIVATEKEY_LEN_MAX					256
/*!< The maximum length (in bytes) of private key modulus for authentication with Enterprise TLS methods.
	Private key exponent must be the same length as modulus, pre-padded with 0s if necessary.
*/
#define M2M_AUTH_1X_CERT_LEN_MAX						1584
/*!< The maximum length (in bytes) of certificate for authentication with Enterprise TLS methods.
*/
#define M2M_802_1X_UNENCRYPTED_USERNAME_FLAG			0x80
/*!< Flag to indicate that the 802.1x user-name should be sent (unencrypted) in the initial EAP
	identity response. Intended for use with EAP-TLS only.
*/
#define M2M_802_1X_PREPEND_DOMAIN_FLAG					0x40
/*!< Flag to indicate that the 802.1x domain name should be prepended to the user-name:
	"Domain\Username". If the flag is not set then domain name is appended to the user-name:
	"Username@Domain". (Note that the '@' or '\' must be included in the domain name.)
*/
#define M2M_802_1X_MSCHAP2_FLAG							0x01
/*!< Flag to indicate 802.1x MsChapV2 credentials: domain/user-name/password.
*/
#define M2M_802_1X_TLS_FLAG								0x02
/*!< Flag to indicate 802.1x TLS credentials: domain/user-name/private-key/certificate.
*/
#define M2M_802_1X_TLS_CLIENT_CERTIFICATE				1
/*!< Info type used in @ref tstrM2mWifiAuthInfoHdr to indicate Enterprise TLS client certificate.
*/

#define PSK_CALC_LEN									40
/*!< PSK is 32 bytes generated either:
    - from 64 ASCII characters
    - by SHA1 operations on up to 63 ASCII characters
    40 byte array is required during SHA1 operations, so we define PSK_CALC_LEN as 40.
*/

#define PWR_DEFAULT                                        PWR_HIGH
/*********************
 *
 * WIFI GROUP requests
 */

#define M2M_CONFIG_CMD_BASE									1
/*!< The base value of all the host configuration commands opcodes.
*/
#define M2M_STA_CMD_BASE									40
/*!< The base value of all the station mode host commands opcodes.
*/
#define M2M_AP_CMD_BASE										70
/*!< The base value of all the Access Point mode host commands opcodes.
*/
/**@}*/     //WlanDefines

#define M2M_P2P_CMD_BASE									90
/*!< The base value of all the P2P mode host commands opcodes.
*/
/**@addtogroup  WlanDefines
 * @{
 */
#define M2M_SERVER_CMD_BASE									100
/*!< The base value of all the power save mode host commands codes.
*/
#define M2M_GEN_CMD_BASE									105
/*!< The base value of additional host wifi command opcodes.
 * Usage restrictions (eg STA mode only) should always be made clear at the API layer in any case.
*/
/**********************
 * OTA GROUP requests
 */
#define M2M_OTA_CMD_BASE									100
/*!< The base value of all the OTA mode host commands opcodes.
 * The OTA Have special group so can extended from 1-M2M_MAX_GRP_NUM_REQ
*/
/***********************
 *
 * CRYPTO group requests
 */
#define M2M_CRYPTO_CMD_BASE									1
/*!< The base value of all the crypto mode host commands opcodes.
 * The crypto Have special group so can extended from 1-M2M_MAX_GRP_NUM_REQ
*/

#define M2M_MAX_GRP_NUM_REQ									(127)
/*!< max number of request in one group equal to 127 as the last bit reserved for config or data pkt
*/

#define WEP_40_KEY_SIZE										((uint8)5)
/*!< The size in bytes of a 40-bit wep key.
*/
#define WEP_104_KEY_SIZE									((uint8)13)
/*!< The size in bytes of a 104-bit wep key.
*/

#define WEP_40_KEY_STRING_SIZE 								((uint8)10)
/*!< The string length of a 40-bit wep key.
*/
#define WEP_104_KEY_STRING_SIZE 							((uint8)26)
/*!< The string length of a 104-bit wep key.
*/

#define WEP_KEY_MAX_INDEX									((uint8)4)
/*!< WEP key index is in the range 1 to 4 inclusive. (This is decremented to
 * result in an index in the range 0 to 3 on air.)
*/
#define M2M_SHA256_CONTEXT_BUFF_LEN							(128)
/*!< sha256 context size
*/
#define M2M_SCAN_DEFAULT_NUM_SLOTS							(2)
/*!< The default number of scan slots used by the WINC board.
*/
#define M2M_SCAN_DEFAULT_SLOT_TIME							(30)
/*!< The default duration in miliseconds of an active scan slot used by the WINC board.
*/
#define M2M_SCAN_DEFAULT_PASSIVE_SLOT_TIME					(300)
/*!< The passive scan slot default duration in ms.
*/
#define M2M_SCAN_DEFAULT_NUM_PROBE							(2)
/*!< The default number of probes per scan slot.
*/
#define M2M_FASTCONNECT_DEFAULT_RSSI_THRESH					(-45)
/*!< The default threshold RSSI for fast reconnection to an AP.
*/

/*======*======*======*======*
    TLS DEFINITIONS
 *======*======*======*======*/
#define TLS_FILE_NAME_MAX                               48
/*!<  Maximum length for each TLS certificate file name including null terminator.
*/
#define TLS_SRV_SEC_MAX_FILES                           8
/*!<  Maximum number of certificates allowed in TLS_SRV section.
*/
#define TLS_SRV_SEC_START_PATTERN_LEN                   8
/*!<  Length of certificate struct start pattern.
*/

/*======*======*======*======*
    SSL DEFINITIONS
 *======*======*======*======*/

#define TLS_CRL_DATA_MAX_LEN    64
/*<!
    Maximum data length in a CRL entry (= Hash length for SHA512)
	*/
#define TLS_CRL_MAX_ENTRIES     10
/*<!
    Maximum number of entries in a CRL
	*/

#define TLS_CRL_TYPE_NONE       0
/*<!
    No CRL check
	*/
#define TLS_CRL_TYPE_CERT_HASH  1
/*<!
    CRL contains certificate hashes
	*/

#define TLS_CERTS_CHUNKED_SIG_VALUE 0x6ec8

/* Commonly used initializers for rate lists for B, G, N or mixed modes for iteration on rates. */
#define WLAN_11B_RATES_INITIALIZER { \
    TX_RATE_1, TX_RATE_2, TX_RATE_5_5, \
    TX_RATE_11 \
}

#define WLAN_11G_RATES_INITIALIZER  { \
    TX_RATE_6, TX_RATE_9, TX_RATE_12, \
    TX_RATE_18, TX_RATE_24, TX_RATE_36, \
    TX_RATE_48, TX_RATE_54 \
}

#define WLAN_11N_RATES_INITIALIZER { \
    TX_RATE_MCS_0, TX_RATE_MCS_1, TX_RATE_MCS_2, \
    TX_RATE_MCS_3, TX_RATE_MCS_4, TX_RATE_MCS_5, \
    TX_RATE_MCS_6, TX_RATE_MCS_7 \
}

#define WLAN_11BGN_RATES_ASC_INITIALIZER { \
    TX_RATE_1, TX_RATE_2, TX_RATE_5_5, \
    TX_RATE_6, TX_RATE_MCS_0, TX_RATE_9, \
    TX_RATE_11, TX_RATE_12, TX_RATE_MCS_1, \
    TX_RATE_18, TX_RATE_MCS_2, TX_RATE_24, \
    TX_RATE_MCS_3, TX_RATE_36, TX_RATE_MCS_4, \
    TX_RATE_48, TX_RATE_MCS_5, TX_RATE_54, \
    TX_RATE_MCS_6, TX_RATE_MCS_7, \
}

#define WLAN_11BG_RATES_ASC_INITIALIZER { \
     TX_RATE_1, TX_RATE_2, TX_RATE_5_5, \
     TX_RATE_6, TX_RATE_9, TX_RATE_11, \
     TX_RATE_12, TX_RATE_18, TX_RATE_24, \
     TX_RATE_36, TX_RATE_48, TX_RATE_54 \
}

#define DEFAULT_CONF_AR_INITIALIZER { 5, 1, TX_RATE_AUTO, TX_RATE_AUTO, 10, 5, 3 }

/**@}*/     //WlanDefines

/**@addtogroup OTADEFINE
 * @{
*/

/*======*======*======*======*
	OTA DEFINITIONS
 *======*======*======*======*/
 
#define OTA_STATUS_VALID					(0x12526285)
/*!< 
	Magic value updated in the Control structure in case of ROLLBACK image Valid
*/
#define OTA_STATUS_INVALID					(0x23987718)
/*!< 
	Magic value updated in the Control structure in case of ROLLBACK image InValid
*/
#define OTA_MAGIC_VALUE						(0x1ABCDEF9)
/*!< 
	Magic value set at the beginning of the OTA image header
*/
#ifdef CORTUS_APP
#define M2M_MAGIC_APP 						(0xef522f61UL)
/*!< 
	Magic value set at the beginning of the Cortus OTA image header
*/
#endif

#define OTA_FORMAT_VER_0                    (0)
/*!<
    Control structure format version 0.\n
    Format used until version 19.2.2.
*/
#define OTA_FORMAT_VER_1                    (1)
/*!<
    Control structure format version 1.\n
    Starting from 19.3.0 CRC is used and sequence number is used.
*/
#define OTA_FORMAT_VER_2                    (2)
/*!<
    Control structure format version 2.\n
    Starting from 19.6.1 a flexible flash map is used.
*/
#define OTA_SHA256_DIGEST_SIZE 				(32)
/*!< 
 Sha256 digest size in the OTA image,
 the sha256 digest is set at the beginning of image before the OTA header
 */

#define MAX_FILE_READ_STEP               128
/*!<
 Max amount of bytes to read a file via HIF messages.
*/

#define HFD_INVALID_HANDLER                 (0xff)
/*!<
 Defines an ID which symbolizes an invalid handler.
*/
/**@}*/     //OTADEFINE

/**@addtogroup  OTATYPEDEF
*/
 /**@{*/

/*!
@enum   \
    tenuOtaError

@brief
    OTA Error codes.
*/
typedef enum {
	OTA_SUCCESS = (0),
	/*!<
	 OTA Success status
	 */
	OTA_ERR_WORKING_IMAGE_LOAD_FAIL = ((sint8) -1),
	/*!<
	 Failure to load the firmware image
	 */
	OTA_ERR_INVALID_CONTROL_SEC = ((sint8) -2),
	/*!<
     Control structure is corrupted
	 */
	M2M_ERR_OTA_SWITCH_FAIL = ((sint8) -3),
	/*!<
	 switching to the updated image failed as may be the image is invalid
	 */
	M2M_ERR_OTA_START_UPDATE_FAIL = ((sint8) -4),
	/*!<
     OTA update fail due to multiple reasons:
	 - Connection failure
	 - Image integrity fail

	 */
	M2M_ERR_OTA_ROLLBACK_FAIL = ((sint8) -5),
	/*!<
	 Roll-back failed due to Roll-back image is not valid
	 */
	M2M_ERR_OTA_INVALID_FLASH_SIZE = ((sint8) -6),
	/*!<
     The OTA Support at least 4MB flash size, this error code will appear if the current flash is less than 4M
	 */
	M2M_ERR_OTA_INVALID_ARG = ((sint8) -7),
	/*!<
	 * Ota still in progress
	 */
	M2M_ERR_OTA_INPROGRESS = ((sint8) -8)
/*!<
 Invalid argument in any OTA Function
 */
} tenuOtaError;

/**@}*/     //OTATYPEDEF

/**@addtogroup WlanEnums
 * @{
 */

/*======*======*======*======*
 *   FLASH ID DEFINITIONS    *
 *======*======*======*======*/
typedef enum {
    ENTRY_ID_FW         = 0x0011,
    ENTRY_ID_PLLGAIN    = 0x0021,
    ENTRY_ID_TLSROOT    = 0x0031,
    ENTRY_ID_TLSCLIENT  = 0x0032,
    ENTRY_ID_TLSSERVER  = 0x0033,
    ENTRY_ID_CONNPARAMS = 0x0034,
    ENTRY_ID_HTTPFILES  = 0x0035,
    ENTRY_ID_TLSCOMMON  = 0x0036,
    ENTRY_ID_HOSTFILE   = 0x0041
} tenuFlashLUTEntryID;

/*======*======*======*======*
    CONNECTION ERROR DEFINITIONS
 *======*======*======*======*/
typedef enum {
    M2M_DEFAULT_CONN_INPROGRESS = ((sint8)-23),
    /*!<
    A failure that indicates that a default connection or forced connection is in progress
    */
    M2M_DEFAULT_CONN_FAIL,
    /*!<
    A failure response that indicates that the winc failed to connect to the cached network
    */
    M2M_DEFAULT_CONN_SCAN_MISMATCH,
    /*!<
    A failure response that indicates that no one of the cached networks
    was found in the scan results, as a result to the function call m2m_default_connect.
    */
    M2M_DEFAULT_CONN_EMPTY_LIST
    /*!<
    A failure response that indicates an empty network list as
    a result to the function call m2m_default_connect.
    */

} tenuM2mDefaultConnErrcode;

/*!
@enum	\
	tenuM2mConnChangedErrcode
	
@brief
	
*/
typedef enum {
	 M2M_ERR_SCAN_FAIL = ((uint8)1),
	/*!< Indicate that the WINC board has failed to perform the scan operation.
	*/
	 M2M_ERR_JOIN_FAIL,	 								
	/*!< Indicate that the WINC board has failed to join the BSS .
	*/
	 M2M_ERR_AUTH_FAIL, 									
	/*!< Indicate that the WINC board has failed to authenticate with the AP.
	*/
	 M2M_ERR_ASSOC_FAIL,
	/*!< Indicate that the WINC board has failed to associate with the AP.
	*/
	 M2M_ERR_CONN_INPROGRESS
	 /*!< Indicate that the WINC board has another connection request in progress.
	*/
}tenuM2mConnChangedErrcode;
/*!
@enum	\
	tenuM2mWepKeyIndex
	
@brief
	
*/
typedef enum {
	M2M_WIFI_WEP_KEY_INDEX_1 = ((uint8) 1),
	M2M_WIFI_WEP_KEY_INDEX_2,
	M2M_WIFI_WEP_KEY_INDEX_3,
	M2M_WIFI_WEP_KEY_INDEX_4
	/*!< Index for WEP key Authentication
	*/
}tenuM2mWepKeyIndex;

/*!
@enum	\
	tenuM2mPwrMode
	
@brief
	
*/
typedef enum {
	PWR_AUTO = ((uint8) 1),
	/*!< FW will decide the best power mode to use internally. */
	PWR_LOW1,
    /*!< Low power mode #1 */
	PWR_LOW2,
    /*!< Low power mode #2 */
	PWR_HIGH
    /*!< High power mode */
}tenuM2mPwrMode;

/*!
@struct	\	
    tstrM2mPwrMode

@brief
	Power Mode
*/
typedef struct {
	uint8	u8PwrMode; 
	/*!< power Save Mode
	*/
	uint8	__PAD24__[3];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mPwrMode;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mPwrMode)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mPwrMode)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@enum	\
	tenuM2mTxPwrLevel
	
@brief
	
*/
typedef enum {
	TX_PWR_HIGH = ((uint8) 1),
	/*!< PPA Gain 6dbm	PA Gain 18dbm */
	TX_PWR_MED,
	/*!< PPA Gain 6dbm	PA Gain 12dbm */
	TX_PWR_LOW
	/*!< PPA Gain 6dbm	PA Gain 6dbm */
}tenuM2mTxPwrLevel;

/*!
@struct	\	
	tstrM2mTxPwrLevel

@brief
	Tx power level 
*/
typedef struct {
	uint8	u8TxPwrLevel; 
	/*!< Tx power level
	*/
	uint8	__PAD24__[3];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mTxPwrLevel;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mTxPwrLevel)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mTxPwrLevel)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\	
	tstrM2mWiFiGainIdx

@brief
	Gain Table index selection corresponding to specific WiFi region.
*/
typedef struct {
	uint8	u8GainTableIdx; 
	/*!< Gain table index
	*/
	uint8	__PAD24__[3];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mWiFiGainIdx;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mWiFiGainIdx)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mWiFiGainIdx)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\	
	tstrM2mEnableLogs

@brief
	Enable Firmware logs
*/
typedef struct {
	uint8	u8Enable; 
	/*!< Enable/Disable firmware logs
	*/
	uint8	__PAD24__[3];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mEnableLogs;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mEnableLogs)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mEnableLogs)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\	
	tstrM2mBatteryVoltage

@brief
	Battery Voltage
*/
typedef struct {
	//Note: on SAMD D21 the size of double is 8 Bytes
	uint16	u16BattVolt; 
	/*!< Battery Voltage
	*/
	uint8	__PAD16__[2];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mBatteryVoltage;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mBatteryVoltage)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mBatteryVoltage)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\	
	tstrM2mWiFiRoaming

@brief
	Roaming related information .
*/
typedef struct {
	uint8	u8EnableRoaming; 
	/*!< Enable/Disable Roaming
	*/
	uint8 	u8EnableDhcp;
	/*!< Enable/Disable DHCP client when u8EnableRoaming is true
	*/
	uint8	__PAD16__[2];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mWiFiRoaming;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mWiFiRoaming)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mWiFiRoaming)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@enum	\
	tenuM2mReqGroup

@brief
*/
typedef enum{
	M2M_REQ_GROUP_MAIN = 0,
	M2M_REQ_GROUP_WIFI,
	M2M_REQ_GROUP_IP,
	M2M_REQ_GROUP_HIF,
	M2M_REQ_GROUP_OTA,
	M2M_REQ_GROUP_SSL,
	M2M_REQ_GROUP_CRYPTO,
	M2M_REQ_GROUP_SIGMA,
	M2M_REQ_GROUP_INTERNAL
}tenuM2mReqGroup;

/*!
@enum	\
	tenuM2mReqpkt

@brief
*/
typedef enum{
	M2M_REQ_CONFIG_PKT,
	M2M_REQ_DATA_PKT = 0x80 /*BIT7*/
}tenuM2mReqpkt;
/*!
@enum	\
	tenuM2mConfigCmd

@brief
	This enum contains host commands used to configure the WINC board.

*/
typedef enum {
	M2M_WIFI_REQ_RESTART = M2M_CONFIG_CMD_BASE,
	/*!<
		Restart the WINC MAC layer, it's doesn't restart the IP layer.
	*/
	M2M_WIFI_REQ_SET_MAC_ADDRESS,
	/*!<
		Set the WINC mac address (not possible for production effused boards).
	*/
	M2M_WIFI_REQ_CURRENT_RSSI,
	/*!<
		Request the current connected AP RSSI.
	*/
	M2M_WIFI_RESP_CURRENT_RSSI,
	/*!<
		Response to M2M_WIFI_REQ_CURRENT_RSSI with the RSSI value.
	*/
	M2M_WIFI_REQ_GET_CONN_INFO,
	/*!< Request connection information command.
	*/
	M2M_WIFI_RESP_CONN_INFO,

	/*!< Connect with default AP response.
	*/
	M2M_WIFI_REQ_SET_DEVICE_NAME,
	/*!<
		Set the WINC device name property.
	*/
	M2M_WIFI_REQ_START_PROVISION_MODE,
	/*!<
		Start the provisioning mode for the M2M Device.
	*/
	M2M_WIFI_RESP_PROVISION_INFO,
	/*!<
		Send the provisioning information to the host.
	*/
	M2M_WIFI_REQ_STOP_PROVISION_MODE,
	/*!<
		Stop the current running provision mode.
	*/
	M2M_WIFI_REQ_SET_SYS_TIME,
	/*!<
		Set time of day from host.
	*/
	M2M_WIFI_REQ_ENABLE_SNTP_CLIENT,
	/*!<
		Enable the simple network time protocol to get the
		time from the Internet. this is required for security purposes.
	*/
	M2M_WIFI_REQ_DISABLE_SNTP_CLIENT,
	/*!<
		Disable the simple network time protocol for applications that
		do not need it.
	*/
	M2M_WIFI_RESP_MEMORY_RECOVER,
	/*!<
	 * Reserved for debugging
	 * */
	M2M_WIFI_REQ_CUST_INFO_ELEMENT,
	/*!< Add Custom Element to Beacon Management Frame.
	*/
	M2M_WIFI_REQ_SCAN,
	/*!< Request scan command.
	*/
	M2M_WIFI_RESP_SCAN_DONE,
	/*!< Scan complete notification response.
	*/
	M2M_WIFI_REQ_SCAN_RESULT,
	/*!< Request Scan results command.
	*/
	M2M_WIFI_RESP_SCAN_RESULT,
	/*!< Request Scan results response.
	*/
	M2M_WIFI_REQ_SET_SCAN_OPTION,
	/*!< Set Scan options "slot time, slot number .. etc" .
	*/
	M2M_WIFI_REQ_SET_SCAN_REGION,
	/*!< Set scan region.
	*/
	M2M_WIFI_REQ_SET_POWER_PROFILE,
	/*!< The API shall set power mode to one of 3 modes
	*/
	M2M_WIFI_REQ_SET_TX_POWER,
	/*!<  API to set TX power. 
	*/
	M2M_WIFI_REQ_SET_BATTERY_VOLTAGE,
	/*!<  API to set Battery Voltage. 
	*/
	M2M_WIFI_REQ_SET_ENABLE_LOGS,
	/*!<  API to set Battery Voltage. 
	*/
	M2M_WIFI_REQ_GET_SYS_TIME,
	/*!<
		REQ GET time of day from WINC.
	*/
	M2M_WIFI_RESP_GET_SYS_TIME,
	/*!<
		RESP time of day from host.
	*/
	M2M_WIFI_REQ_SEND_ETHERNET_PACKET,
	/*!< Send Ethernet packet in bypass mode.
	*/
	M2M_WIFI_RESP_ETHERNET_RX_PACKET,
	/*!< Receive Ethernet packet in bypass mode.
	*/	
	M2M_WIFI_REQ_SET_MAC_MCAST,
	/*!< Set the WINC multicast filters.
	*/
	M2M_WIFI_REQ_GET_PRNG,
	/*!< Request PRNG.
	*/
	M2M_WIFI_RESP_GET_PRNG,
	/*!< Response for PRNG.
	*/
	M2M_WIFI_REQ_SCAN_SSID_LIST,
	/*!< Request scan with list of hidden SSID plus the broadcast scan.
	*/
	M2M_WIFI_REQ_SET_GAINS,
	/*!< Request set the PPA gain
	*/
	M2M_WIFI_REQ_PASSIVE_SCAN,
	/*!< Request a passivr scan command.
	*/
	M2M_WIFI_REQ_CONG_AUTO_RATE,
	/*!< Configure auto TX rate selection algorithm.
	*/
	M2M_WIFI_REQ_CONFIG_SNTP,
	/*!< Configure NTP servers.
	*/
	M2M_WIFI_REQ_SET_GAIN_TABLE_IDX,
	/*!<  API to set Gain table index. 
	*/
	M2M_WIFI_REQRSP_DELETE_APID,
	/*!< Request/response to delete AP security credentials from WINC flash.
	*/
	/* This enum is now 'full' in the sense that (M2M_WIFI_REQRSP_DELETE_APID+1) == M2M_STA_CMD_BASE.
	 * Any new config values should be placed in tenuM2mGenCmd. */
	M2M_WIFI_MAX_CONFIG_ALL
}tenuM2mConfigCmd;

/*!
@enum	\
	tenuM2mStaCmd
	
@brief
	This enum contains WINC commands while in Station mode.
*/
typedef enum {
	M2M_WIFI_REQ_CONNECT = M2M_STA_CMD_BASE,
	/*!< Connect with AP command. This command is deprecated in favour of @ref M2M_WIFI_REQ_CONN.
	*/
	M2M_WIFI_REQ_DEFAULT_CONNECT,
	/*!< Connect with default AP command.
	*/
	M2M_WIFI_RESP_DEFAULT_CONNECT,
	/*!< Request connection information response.
	*/
	M2M_WIFI_REQ_DISCONNECT,
	/*!< Request to disconnect from AP command.
	*/
	M2M_WIFI_RESP_CON_STATE_CHANGED,
	/*!< Connection state changed response.
	*/
	M2M_WIFI_REQ_SLEEP,
	/*!< Set PS mode command.
	*/
	M2M_WIFI_REQ_WPS_SCAN,
	/*!< Request WPS scan command.
	*/
	M2M_WIFI_REQ_WPS,
	/*!< Request WPS start command.
	*/
	M2M_WIFI_REQ_START_WPS,
	/*!< This command is for internal use by the WINC and 
		should not be used by the host driver.
	*/
	M2M_WIFI_REQ_DISABLE_WPS,
	/*!< Request to disable WPS command.
	*/
	M2M_WIFI_REQ_DHCP_CONF,
	/*!< Response indicating that IP address was obtained.
	*/
	M2M_WIFI_RESP_IP_CONFIGURED,
	/*!< This command is for internal use by the WINC and 
		should not be used by the host driver.
	*/
	M2M_WIFI_RESP_IP_CONFLICT,
	/*!< Response indicating a conflict in obtained IP address.
		The user should re attempt the DHCP request.
	*/
	M2M_WIFI_REQ_ENABLE_MONITORING,
	/*!< Request to enable monitor mode  command.
	*/
	M2M_WIFI_REQ_DISABLE_MONITORING,
	/*!< Request to disable monitor mode  command.
	*/
	M2M_WIFI_RESP_WIFI_RX_PACKET,
	/*!< Indicate that a packet was received in monitor mode.
	*/
	M2M_WIFI_REQ_SEND_WIFI_PACKET,
	/*!< Send packet in monitor mode.
	*/
	M2M_WIFI_REQ_LSN_INT,
	/*!< Set WiFi listen interval.
	*/
	M2M_WIFI_REQ_DOZE,
	/*!< Used to force the WINC to sleep in manual PS mode.
	*/
	M2M_WIFI_REQ_CONN,
	/*!< New command to connect with AP.
		This replaces M2M_WIFI_REQ_CONNECT. (Firmware continues to handle
		M2M_WIFI_REQ_CONNECT for backwards compatibility purposes.)
	*/
	M2M_WIFI_IND_CONN_PARAM,
	/*!< Provide extra information (such as Enterprise client certificate) required for connection.
	*/
	M2M_WIFI_REQ_DHCP_FAILURE,
	/*!< Response indicating that IP address could not be obtained or renewed. If the IP could not be renewed then the previous IP will continue to be used.
	*/
	M2M_WIFI_MAX_STA_ALL
} tenuM2mStaCmd;

/*!
@enum	\
	tenuM2mApCmd

@brief
	This enum contains WINC commands while in AP mode.
*/
typedef enum {
	M2M_WIFI_REQ_ENABLE_AP = M2M_AP_CMD_BASE,
	/*!< Enable AP mode command.
	*/
	M2M_WIFI_REQ_DISABLE_AP,
	/*!< Disable AP mode command.
	*/
	M2M_WIFI_REQ_RESTART_AP,
	/*!<
	*/
	M2M_WIFI_MAX_AP_ALL
}tenuM2mApCmd;

/**@}*/     //WlanEnums

/*!
@enum	\
	tenuM2mP2pCmd

@brief
	This enum contains WINC commands while in P2P mode.
*/
typedef enum {
	M2M_WIFI_REQ_P2P_INT_CONNECT = M2M_P2P_CMD_BASE,
	/*!< This command is for internal use by the WINC and 
		should not be used by the host driver.
	*/
	M2M_WIFI_REQ_ENABLE_P2P,
	/*!< Enable P2P mode command.
	*/
	M2M_WIFI_REQ_DISABLE_P2P,
	/*!< Disable P2P mode command.
	*/
	M2M_WIFI_REQ_P2P_REPOST,
	/*!< This command is for internal use by the WINC and 
		should not be used by the host driver.
	*/
	M2M_WIFI_MAX_P2P_ALL
}tenuM2mP2pCmd;

/**@addtogroup WlanEnums
 * @{
 */

/*!
@enum	\
	tenuM2mServerCmd

@brief
	This enum contains WINC commands while in PS mode.
	These command are currently not supported.
*/
typedef enum {
	M2M_WIFI_REQ_CLIENT_CTRL = M2M_SERVER_CMD_BASE,
	M2M_WIFI_RESP_CLIENT_INFO,
	M2M_WIFI_REQ_SERVER_INIT,
	M2M_WIFI_MAX_SERVER_ALL
}tenuM2mServerCmd;

/*!
@enum	\
	tenuM2mGenCmd

@brief
	This enum contains additional WINC commands (overflow of previous enums).
*/
typedef enum {
	M2M_WIFI_REQ_ROAMING = M2M_GEN_CMD_BASE,
	/*!< Request to enable/disable wifi roaming.
		(Processing matches @ref tenuM2mConfigCmd.)
	*/
	M2M_WIFI_MAX_GEN_ALL
}tenuM2mGenCmd;

/*!
@enum	\
	tenuM2mCryptoCmd

@brief

*/
typedef enum {
	M2M_CRYPTO_REQ_SHA256_INIT = M2M_CRYPTO_CMD_BASE,
	M2M_CRYPTO_RESP_SHA256_INIT,
	M2M_CRYPTO_REQ_SHA256_UPDATE,
	M2M_CRYPTO_RESP_SHA256_UPDATE,
	M2M_CRYPTO_REQ_SHA256_FINISH,
	M2M_CRYPTO_RESP_SHA256_FINISH,
	M2M_CRYPTO_REQ_RSA_SIGN_GEN,
	M2M_CRYPTO_RESP_RSA_SIGN_GEN,
	M2M_CRYPTO_REQ_RSA_SIGN_VERIFY,
	M2M_CRYPTO_RESP_RSA_SIGN_VERIFY,
	M2M_CRYPTO_MAX_ALL
}tenuM2mCryptoCmd;

/*!
@enum	\
	tenuM2mIpCmd

@brief

*/
typedef enum {
	/* Request IDs corresponding to the IP GROUP. */
	M2M_IP_REQ_STATIC_IP_CONF = ((uint8) 10),
	M2M_IP_REQ_ENABLE_DHCP,
	M2M_IP_REQ_DISABLE_DHCP
} tenuM2mIpCmd;

/*!
@enum	\
	tenuM2mSigmaCmd
	
@brief

*/
typedef enum {
	/* Request IDs corresponding to the IP GROUP. */
	M2M_SIGMA_ENABLE = ((uint8) 3),
	M2M_SIGMA_TA_START,
	M2M_SIGMA_TA_STATS,
	M2M_SIGMA_TA_RECEIVE_STOP,
	M2M_SIGMA_ICMP_ARP,
	M2M_SIGMA_ICMP_RX,
	M2M_SIGMA_ICMP_TX,
	M2M_SIGMA_UDP_TX,
	M2M_SIGMA_UDP_TX_DEFER,
	M2M_SIGMA_SECURITY_POLICY,
	M2M_SIGMA_SET_SYSTIME
} tenuM2mSigmaCmd;

/*!
@enum	\
	tenuM2mConnState

@brief
	Wi-Fi Connection State.
*/
typedef enum {
	M2M_WIFI_DISCONNECTED = 0,
	/*!< Wi-Fi state is disconnected.
	*/
	M2M_WIFI_CONNECTED,
	/*!< Wi-Fi state is connected.
	*/
	M2M_WIFI_ROAMED,
	/*!< Wi-Fi state is roamed to new AP.
	*/
	M2M_WIFI_UNDEF = 0xff
	/*!< Undefined Wi-Fi State.
	*/
}tenuM2mConnState;

/*!
@enum	\
	tenuM2mSecType

@brief
	Wi-Fi Supported Security types.
*/
typedef enum {
	M2M_WIFI_SEC_INVALID = 0,
	/*!< Invalid security type.
	*/
	M2M_WIFI_SEC_OPEN,
	/*!< Wi-Fi network is not secured.
	*/
	M2M_WIFI_SEC_WPA_PSK,
	/*!< Wi-Fi network is secured with WPA/WPA2 personal(PSK).
	*/
	M2M_WIFI_SEC_WEP,
	/*!< Security type WEP (40 or 104) OPEN OR SHARED.
	*/
	M2M_WIFI_SEC_802_1X,
	/*!< Wi-Fi network is secured with WPA/WPA2 Enterprise.IEEE802.1x.
	 */
	M2M_WIFI_NUM_AUTH_TYPES
	/*!< Upper limit for enum value.
	 */
}tenuM2mSecType;


/*!
@enum	\
	tenuM2mSecType

@brief
	Wi-Fi Supported SSID types.
*/
typedef enum {
	SSID_MODE_VISIBLE = 0,
	/*!< SSID is visible to others.
	*/
	SSID_MODE_HIDDEN
	/*!< SSID is hidden.
	*/
}tenuM2mSsidMode;

/*!
@enum	\
	tenuM2mScanCh

@brief
	Wi-Fi RF Channels.
@sa
	tstrM2MScan
	tstrM2MScanOption
*/
typedef enum {
	M2M_WIFI_CH_1 = ((uint8) 1),
	M2M_WIFI_CH_2,
	M2M_WIFI_CH_3,
	M2M_WIFI_CH_4,
	M2M_WIFI_CH_5,
	M2M_WIFI_CH_6,
	M2M_WIFI_CH_7,
	M2M_WIFI_CH_8,
	M2M_WIFI_CH_9,
	M2M_WIFI_CH_10,
	M2M_WIFI_CH_11,
	M2M_WIFI_CH_12,
	M2M_WIFI_CH_13,
	M2M_WIFI_CH_14,
	M2M_WIFI_CH_ALL = ((uint8) 255)
}tenuM2mScanCh;

/*!
@enum	\
	tenuM2mScanRegion

@brief
	Wi-Fi RF Channels.
*/
typedef enum {

	REG_CH_1 = ((uint16) 1 << 0),
	REG_CH_2 = ((uint16) 1 << 1),
	REG_CH_3 = ((uint16) 1 << 2),
	REG_CH_4 = ((uint16) 1 << 3),
	REG_CH_5 = ((uint16) 1 << 4),
	REG_CH_6 = ((uint16) 1 << 5),
	REG_CH_7 = ((uint16) 1 << 6),
	REG_CH_8 = ((uint16) 1 << 7),
	REG_CH_9 = ((uint16) 1 << 8),
	REG_CH_10 = ((uint16) 1 << 9),
	REG_CH_11 = ((uint16) 1 << 10),
	REG_CH_12 = ((uint16) 1 << 11),
	REG_CH_13 = ((uint16) 1 << 12),
	REG_CH_14 = ((uint16) 1 << 13),
	REG_CH_ALL = ((uint16) 0x3FFF),
	NORTH_AMERICA = ((uint16) 0x7FF),
	/** 11 channel
	*/
	EUROPE		=   ((uint16) 0x1FFF),
	/** 13 channel
	*/
	ASIA		=   ((uint16) 0x3FFF)
	/* 14 channel
	*/
}tenuM2mScanRegion;


/*!
@enum	\
	tenuPowerSaveModes

@brief
	Power Save Modes.
*/
typedef enum {
	M2M_NO_PS,
	/*!< Power save is disabled.
	*/
	M2M_PS_AUTOMATIC,
	/*!< Power save is done automatically by the WINC.
		This mode doesn't disable all of the WINC modules and 
		use higher amount of power than the H_AUTOMATIC and 
		the DEEP_AUTOMATIC modes..
	*/
	M2M_PS_H_AUTOMATIC,
	/*!< Power save is done automatically by the WINC.
		Achieve higher power save than the AUTOMATIC mode
		by shutting down more parts of the WINC board.
	*/
	M2M_PS_DEEP_AUTOMATIC,
	/*!< Power save is done automatically by the WINC.
		Achieve the highest possible power save.
	*/
	M2M_PS_MANUAL
	/*!< Power save is done manually by the user.
	*/
}tenuPowerSaveModes;

/*!
@enum	\
	tenuM2mWifiMode
	
@brief
	Wi-Fi Operation Mode.
*/
typedef enum {
	M2M_WIFI_MODE_NORMAL = ((uint8) 1),
	/*!< Normal Mode means to run customer firmware version.
	 */
	M2M_WIFI_MODE_ATE_HIGH,
	/*!< Config Mode in HIGH POWER means to run production test firmware version which is known as ATE (Burst) firmware.
	 */
	M2M_WIFI_MODE_ATE_LOW,
	/*!< Config Mode in LOW POWER means to run production test firmware version which is known as ATE (Burst) firmware.
	 */
	M2M_WIFI_MODE_ETHERNET,
	/*!< ethernet Mode
	 */
	M2M_WIFI_MODE_MAX
}tenuM2mWifiMode;

/*!
@enum	\
	tenuWPSTrigger

@brief
	WPS Triggering Methods.
*/
typedef enum{
	WPS_PIN_TRIGGER = 0,
	/*!< WPS is triggered in PIN method.
	*/
	WPS_PBC_TRIGGER = 4
	/*!< WPS is triggered via push button.
	*/
}tenuWPSTrigger;

/*!
@enum	\
	tenuSNTPUseDHCP

@brief
	Use NTP server provided by the DHCP server.
*/
typedef enum{
	SNTP_DISABLE_DHCP = 0,
	/*!< Don't use the NTP server provided by the DHCP server when falling back.
	*/
	SNTP_ENABLE_DHCP = 1
	/*!< Use the NTP server provided by the DHCP server when falling back.
	*/
}tenuSNTPUseDHCP;

/*! 
@struct	\
	tstrM2mWifiGainsParams

@brief
	Gain Values 
*/
typedef struct{
	uint16	u8PPAGFor11B;
	/*!< PPA gain for 11B (as the RF document representation)
	PPA_AGC<0:2> Every bit have 3dB gain control each.
	for example:
	1 ->3db
	3 ->6db
	7 ->9db
	*/
	uint16	u8PPAGFor11GN;
	/*!< PPA gain for 11GN (as the RF document represented)
	PPA_AGC<0:2> Every bit have 3dB gain control each.
		for example:
	1 ->3db
	3 ->6db
	7 ->9db
	*/
}tstrM2mWifiGainsParams;

/*!
@struct	\
	tstrM2mConnCredHdr

@brief
	Wi-Fi Connect Credentials Header
*/
typedef struct{
	uint16	u16CredSize;
	/*!< Total size of connect credentials, not including tstrM2mConnCredHdr:
			tstrM2mConnCredCmn
			Auth details (variable size)
	*/
	uint8	u8CredStoreFlags;
	/*!< Credential storage options represented with flags:
			@ref M2M_CRED_STORE_FLAG
			@ref M2M_CRED_ENCRYPT_FLAG
			@ref M2M_CRED_IS_STORED_FLAG
			@ref M2M_CRED_IS_ENCRYPTED_FLAG
	*/
	uint8	u8Channel;
	/*!< Wi-Fi channel(s) on which to attempt connection. */
}tstrM2mConnCredHdr;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mConnCredHdr)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mConnCredHdr)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mConnCredCmn

@brief
	Wi-Fi Connect Credentials Common section
*/
typedef struct{
	uint8	u8SsidLen;
	/*!< SSID length. */
	uint8	au8Ssid[M2M_MAX_SSID_LEN-1];
	/*!< SSID. */
	uint8	u8Options;
	/*!< Common flags:
			@ref M2M_WIFI_CONN_BSSID_FLAG
	*/
	uint8	au8Bssid[M2M_MAC_ADDRES_LEN];
	/*!< BSSID to restrict on, or all 0 if M2M_WIFI_CONN_BSSID_FLAG is not set in u8Options. */
	uint8	u8AuthType;
	/*!< Connection auth type. See @ref tenuM2mSecType. */
	uint8	au8Rsv[3];
	/*!< Reserved for future use. Set to 0. */
}tstrM2mConnCredCmn;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mConnCredCmn)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mConnCredCmn)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mWifiWep

@brief
	WEP security key header.
	*/
typedef struct{
	uint8	u8KeyIndex;
	/*!< WEP Key Index. */
	uint8	u8KeyLen;
	/*!< WEP Key Size. */
	uint8	au8WepKey[WEP_104_KEY_SIZE];
	/*!< WEP Key represented in bytes (padded with 0's if WEP-40). */
	uint8	u8Rsv;
	/*!< Reserved for future use. Set to 0. */
}tstrM2mWifiWep;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mWifiWep)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mWifiWep)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
    tstrM2mWifiPsk

@brief
	Passphrase and PSK for WPA(2) PSK.
*/
typedef struct{
	uint8	u8PassphraseLen;
	/*!< Length of passphrase (8 to 63) or 64 if au8Passphrase contains ASCII representation of PSK. */
	uint8	au8Passphrase[M2M_MAX_PSK_LEN-1];
	/*!< Passphrase, or ASCII representation of PSK if u8PassphraseLen is 64. */
	uint8	au8Psk[PSK_CALC_LEN];
	/*!< PSK calculated by firmware. Driver sets this to 0. */
	uint8	u8PskCalculated;
	/*!< Flag used by firmware to avoid unnecessary recalculation of PSK. Driver sets this to 0. */
	uint8	au8Rsv[2];
	/*!< Reserved for future use. Set to 0. */
}tstrM2mWifiPsk;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mWifiPsk)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mWifiPsk)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mWifi1xHdr

@brief
	Wi-Fi Authentication 802.1x header for parameters.
	The parameters (Domain, UserName, PrivateKey/Password) are appended to this structure.
*/
typedef struct{
	uint8	u8Flags;
	/*!< 802.1x-specific flags:
			@ref M2M_802_1X_MSCHAP2_FLAG
			@ref M2M_802_1X_TLS_FLAG
			@ref M2M_802_1X_UNENCRYPTED_USERNAME_FLAG
			@ref M2M_802_1X_PREPEND_DOMAIN_FLAG
	*/
	uint8	u8DomainLength;
	/*!< Length of Domain. (Offset of Domain understood to be 0.) */
	uint16	u16UserNameLength;
	/*!< Length of UserName. (Offset of UserName understood to be u8DomainLength.) */
	uint16	u16PrivateKeyOffset;
	/*!< Offset within au81xAuthDetails of PrivateKey/Password. */
	uint16	u16PrivateKeyLength;
	/*!< Length of PrivateKey/Password. In the case of PrivateKey, this is the length of the modulus. */
	uint16	u16CertificateOffset;
	/*!< Offset within au81xAuthDetails of Certificate. */
	uint16	u16CertificateLength;
	/*!< Length of Certificate. */
	uint8	au81xAuthDetails[];
	/*!< Placeholder for concatenation of Domain, UserName, PrivateKey/Password, Certificate.
			Padding (0s) is placed between UserName and PrivateKey/Password so that
			PrivateKey/Password begins on a 4-byte boundary.
			Certificate (for 1x Tls only) is sent over HIF separately from the other parameters. */
}tstrM2mWifi1xHdr;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mWifi1xHdr)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mWifi1xHdr)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mWifiAuthInfoHdr

@brief
	Generic Wi-Fi authentication information to be sent in a separate HIF message of type
	@ref M2M_WIFI_IND_CONN_PARAM (preceding @ref M2M_WIFI_REQ_CONN).
	*/
typedef struct{
	uint8	u8Type;
	/*!< Type of info:
			@ref M2M_802_1X_TLS_CLIENT_CERTIFICATE
	*/
	uint8	au8Rsv[3];
	/*!< Reserved for future use. Set to 0. */
	uint16  u16InfoPos;
	/*!< Information about positioning of the Info. The interpretation depends on u8Type. */
	uint16	u16InfoLen;
	/*!< Info length (not including this header). */
	uint8	au8Info[];
	/*!< Placeholder for info. */
}tstrM2mWifiAuthInfoHdr;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mWifiAuthInfoHdr)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mWifiAuthInfoHdr)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mWifiConnHdr

@brief
	Wi-Fi Connect Request (new format) for use with @ref M2M_WIFI_REQ_CONN.
	This structure is sent across the HIF along with the relevant auth details. One of:
		@ref tstrM2mWifiPsk
		@ref tstrM2mWifiWep
		@ref tstrM2mWifi1xHdr
	If further authentication details need to be sent (such as client certificate for 1x TLS), they
	are sent with header @ref tstrM2mWifiAuthInfoHdr in a preceding HIF message of type
	@ref M2M_WIFI_IND_CONN_PARAM
*/
typedef struct{
	tstrM2mConnCredHdr	strConnCredHdr;
	/*!< Credentials header. */
	tstrM2mConnCredCmn	strConnCredCmn;
	/*!< Credentials common section, including auth type and SSID. */
}tstrM2mWifiConnHdr;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mWifiConnHdr)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mWifiConnHdr)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mWifiApId

@brief
	Specify an access point (by SSID)
*/
typedef struct{
	uint8				au8SSID[M2M_MAX_SSID_LEN];
	/*!<
		SSID of the desired AP, prefixed by length byte.
		First byte 0xFF used to mean all access points.
	*/
	uint8	__PAD__[3];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mWifiApId;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mWifiApId)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mWifiApId)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2MGenericResp

@brief
	Generic success/error response
*/
typedef struct{
	sint8		s8ErrorCode;
	/*!<
		Generic success/error code. Possible values are:
		- @ref M2M_SUCCESS
		- @ref M2M_ERR_FAIL
	*/
	uint8	__PAD24__[3];
}tstrM2MGenericResp;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MGenericResp)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MGenericResp)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2MWPSConnect

@brief
	WPS Configuration parameters

@sa
	tenuWPSTrigger
*/
typedef struct {
	uint8 	u8TriggerType;
	/*!< WPS triggering method (Push button or PIN)
	*/
	char         acPinNumber[8];
	/*!< WPS PIN No (for PIN method)
	*/
	uint8	__PAD24__[3];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2MWPSConnect;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MWPSConnect)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MWPSConnect)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2MWPSInfo

@brief	WPS Result

	This structure is passed to the application in response to a WPS request. If the WPS session is completed successfully, the
	structure will have Non-ZERO authentication type. If the WPS Session fails (due to error or timeout) the authentication type
	is set to ZERO.

@sa
	tenuM2mSecType
*/
typedef struct{
	uint8	u8AuthType;
	/*!< Network authentication type.
	*/
	uint8   	u8Ch;
	/*!< RF Channel for the AP.
	*/
	uint8	au8SSID[M2M_MAX_SSID_LEN];
	/*!< SSID obtained from WPS.
	*/
	uint8	au8PSK[M2M_MAX_PSK_LEN];
	/*!< PSK for the network obtained from WPS.
	*/
}tstrM2MWPSInfo;


/*!
@struct	\
	tstrM2MDefaultConnResp

@brief
	Response error of the m2m_default_connect

@sa
	M2M_DEFAULT_CONN_SCAN_MISMATCH
	M2M_DEFAULT_CONN_EMPTY_LIST
*/
typedef struct{
	sint8		s8ErrorCode;
	/*!<
		Default connect error code. possible values are:
		- M2M_DEFAULT_CONN_EMPTY_LIST
		- M2M_DEFAULT_CONN_SCAN_MISMATCH
	*/
	uint8	__PAD24__[3];
}tstrM2MDefaultConnResp;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MDefaultConnResp)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MDefaultConnResp)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2MScanOption

@brief
    This struct contains the configuration options for Wi-Fi scan.

@sa
	tenuM2mScanCh
	tstrM2MScan
*/
typedef struct {
	uint8   u8NumOfSlot;
    /*!< The number of scan slots per channel. Refers to both active and passive scan.
         Valid settings are in the range 0<Slots<=255.
         Default setting is @ref M2M_SCAN_DEFAULT_NUM_SLOTS.
	*/
	uint8   u8SlotTime;
    /*!< The length of each scan slot in milliseconds. Refers to active scan only.
         The device listens for probe responses and beacons during this time.
         Valid settings are in the range 10<=SlotTime<=250.
         Default setting is @ref M2M_SCAN_DEFAULT_SLOT_TIME.
	*/
	uint8  u8ProbesPerSlot;
    /*!< Number of probe requests to be sent each scan slot. Refers to active scan only.
         Valid settings are in the range 0<Probes<=2.
         Default setting is @ref M2M_SCAN_DEFAULT_NUM_PROBE.
	*/
	sint8   s8RssiThresh;
    /*!< The Received Signal Strength Indicator threshold required for (fast) reconnection to an AP without scanning all channels first.
         Refers to active scan as part of reconnection to a previously connected AP.
         The device connects to the target AP immediately if it receives a sufficiently strong probe response on the expected channel.
         Low thresholds facilitate fast reconnection. High thresholds facilitate connection to the strongest signal.
         Valid settings are in the range -128<=Thresh<0.
         Default setting is @ref M2M_FASTCONNECT_DEFAULT_RSSI_THRESH.
	*/

}tstrM2MScanOption;

/*!
@struct	\
	tstrM2MScanRegion

@brief
	Wi-Fi channel regulation region information.

@sa
	tenuM2mScanRegion
*/
typedef struct {
	uint16   u16ScanRegion;
	/*|< Specifies the number of channels allowed in the region (e.g. North America = 11 ... etc.).
	*/
	uint8 __PAD16__[2];
	/*!< Padding bytes for forcing 4-byte alignment */
}tstrM2MScanRegion;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MScanRegion)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MScanRegion)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2MScan

@brief
	Wi-Fi Scan Request

@sa
	tenuM2mScanCh
	tstrM2MScanOption
*/
typedef struct {
	uint8 	u8ChNum;
	/*!< The Wi-Fi RF Channel number
	*/
	uint8	__RSVD8__[1];
	/*!< Reserved for future use.
	*/
	uint16 	u16PassiveScanTime;
    /*!< The length of each scan slot in milliseconds. Refers to passive scan only.
         The device listens for beacons during this time.
         Valid settings are in the range 10<=PassiveScanTime<=1200.
         Default setting is @ref M2M_SCAN_DEFAULT_PASSIVE_SLOT_TIME.
	*/
}tstrM2MScan;

/*!
@struct	\
	tstrCyptoResp

@brief
	crypto response
*/
typedef struct {
	sint8 s8Resp;
	/***/
	uint8 __PAD24__[3];
	/*
	*/
}tstrCyptoResp;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrCyptoResp)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrCyptoResp)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mScanDone

@brief
	Wi-Fi Scan Result
*/
typedef struct{
	uint8 	u8NumofCh;
	/*!< Number of found APs
	*/
	sint8 	s8ScanState;
	/*!< Scan status
	*/
	uint8	__PAD16__[2];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mScanDone;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mScanDone)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mScanDone)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mReqScanResult

@brief	Scan Result Request

	The Wi-Fi Scan results list is stored in Firmware. The application can request a certain scan result by its index.
*/
typedef struct {
	uint8 	u8Index;
	/*!< Index of the desired scan result
	*/
	uint8	__PAD24__[3];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mReqScanResult;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mReqScanResult)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mReqScanResult)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mWifiscanResult

@brief	Wi-Fi Scan Result

	Information corresponding to an AP in the Scan Result list identified by its order (index) in the list.
*/
typedef struct {
	uint8 	u8index;
	/*!< AP index in the scan result list.
	*/
	sint8 	s8rssi;
	/*!< AP signal strength.
	*/
	uint8 	u8AuthType;
	/*!< AP authentication type.
	*/
	uint8 	u8ch;
	/*!< AP RF channel.
	*/
	uint8	au8BSSID[6];
	/*!< BSSID of the AP.
	*/
	uint8 	au8SSID[M2M_MAX_SSID_LEN];
	/*!< AP ssid.
	*/
	uint8 	_PAD8_;
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mWifiscanResult;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mWifiscanResult)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mWifiscanResult)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mWifiStateChanged

@brief
	Wi-Fi Connection State

@sa
	M2M_WIFI_DISCONNECTED, M2M_WIFI_CONNECTED, M2M_WIFI_REQ_CON_STATE_CHANGED,tenuM2mConnChangedErrcode
*/
typedef struct {
	uint8	u8CurrState;
	/*!< Current Wi-Fi connection state
	*/
	uint8  u8ErrCode;
	/*!< Error type review tenuM2mConnChangedErrcode
	*/
	uint8	__PAD16__[2];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mWifiStateChanged;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mWifiStateChanged)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mWifiStateChanged)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mPsType

@brief
	Power Save Configuration

@sa
	tenuPowerSaveModes
*/
typedef struct{
	uint8 	u8PsType;
	/*!< Power save operating mode
	*/
	uint8 	u8BcastEn;
	/*!<
	*/
	uint8	__PAD16__[2];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mPsType;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mPsType)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mPsType)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mSlpReqTime

@brief
	Manual power save request sleep time

*/
typedef struct {
	/*!< Sleep time in ms
	*/
	uint32 u32SleepTime;

} tstrM2mSlpReqTime;

/*!
@struct	\
	tstrM2mLsnInt

@brief	Listen interval

	It is the value of the Wi-Fi STA listen interval for power saving. It is given in units of Beacon period. 
	Periodically after the listen interval fires, the WINC is wakeup and listen to the beacon and check for any buffered frames for it from the AP.
*/
typedef struct {
	uint16 	u16LsnInt;
	/*!< Listen interval in Beacon period count.
	*/
	uint8	__PAD16__[2];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mLsnInt;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mLsnInt)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mLsnInt)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2MWifiMonitorModeCtrl

@brief	Wi-Fi Monitor Mode Filter

	This structure sets the filtering criteria for WLAN packets when monitoring mode is enable. 
	The received packets matching the filtering parameters, are passed directly to the application.
*/
typedef struct{
	uint8	u8ChannelID;
	/* !< RF Channel ID. It must use values from tenuM2mScanCh
	*/
	uint8	u8FrameType;
	/*!< It must use values from tenuWifiFrameType.
	*/
	uint8	u8FrameSubtype;
	/*!< It must use values from tenuSubTypes.
	*/
	uint8	au8SrcMacAddress[6];
	/* ZERO means DO NOT FILTER Source address.
	*/
	uint8	au8DstMacAddress[6];
	/* ZERO means DO NOT FILTER Destination address.
	*/
	uint8	au8BSSID[6];
	/* ZERO means DO NOT FILTER BSSID.
	*/
	uint8 u8EnRecvHdr;
	/*
	 Enable receive the full header before the payload
	*/
	uint8	__PAD16__[2];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2MWifiMonitorModeCtrl;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MWifiMonitorModeCtrl)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MWifiMonitorModeCtrl)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2MWifiRxPacketInfo

@brief	Wi-Fi RX Frame Header

	The M2M application has the ability to allow Wi-Fi monitoring mode for receiving all Wi-Fi Raw frames matching a well defined filtering criteria.
	When a target Wi-Fi packet is received, the header information are extracted and assigned in this structure. 
*/
typedef struct{
	uint8	u8FrameType;
	/*!< It must use values from tenuWifiFrameType.
	*/
	uint8	u8FrameSubtype;
	/*!< It must use values from tenuSubTypes.
	*/
	uint8	u8ServiceClass;
	/*!< Service class from Wi-Fi header.
	*/
	uint8	u8Priority;
	/*!< Priority from Wi-Fi header.
	*/
	uint8	u8HeaderLength;
	/*!< Frame Header length.
	*/
	uint8	u8CipherType;
	/*!< Encryption type for the rx packet.
	*/
	uint8	au8SrcMacAddress[6];
	/* ZERO means DO NOT FILTER Source address.
	*/
	uint8	au8DstMacAddress[6];
	/* ZERO means DO NOT FILTER Destination address.
	*/
	uint8	au8BSSID[6];
	/* ZERO means DO NOT FILTER BSSID.
	*/
	uint16	u16DataLength;
	/*!< Data payload length (Header excluded).
	*/
	uint16	u16FrameLength;
	/*!< Total frame length (Header + Data).
	*/
	uint32	u32DataRateKbps;
	/*!< Data Rate in Kbps.
	*/
	sint8		s8RSSI;
	/*!< RSSI.
	*/
	uint8	__PAD24__[3];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2MWifiRxPacketInfo;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MWifiRxPacketInfo)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MWifiRxPacketInfo)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2MWifiTxPacketInfo

@brief	Wi-Fi TX Packet Info

	The M2M Application has the ability to compose a RAW Wi-Fi frames (under the application responsibility).
	When transmitting a Wi-Fi packet, the application must supply the firmware with this structure for sending the target frame.
*/
typedef struct{
	uint16	u16PacketSize;
	/*!< Wlan frame length.
	*/
	uint16	u16HeaderLength;
	/*!< Wlan frame header length.
	*/
}tstrM2MWifiTxPacketInfo;

/**@}*/     //WlanEnums

/**@cond P2P_DOC
 * @addtogroup WlanEnums
 * @{
 */
/*!
 @struct	\
 	tstrM2MP2PConnect

 @brief
 	Set the device to operate in the Wi-Fi Direct (P2P) mode.
*/
typedef struct {
	uint8 	u8ListenChannel;
	/*!< P2P Listen Channel (1, 6 or 11)
	*/
	uint8	__PAD24__[3];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2MP2PConnect;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MP2PConnect)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MP2PConnect)%4)==0, "Structure alignment error");
#endif
#endif

/**@}*/
/**@endcond*/

/**@addtogroup WlanEnums
 * @{
 */

/*!
@struct	\
	tstrM2MAPConfig

@brief	AP Configuration

	This structure holds the configuration parameters for the M2M AP mode. It should be set by the application when
	it requests to enable the M2M AP operation mode. The M2M AP mode currently supports only WEP security (with
	the NO Security option available of course).
*/
typedef struct {
	/*!<
		Configuration parameters for the WiFi AP.
	*/
	uint8 	au8SSID[M2M_MAX_SSID_LEN];
	/*!< AP SSID
	*/
	uint8 	u8ListenChannel;
	/*!< Wi-Fi RF Channel which the AP will operate on
	*/
	uint8	u8KeyIndx;
	/*!< Wep key Index
	*/
	uint8	u8KeySz;
	/*!< Wep/WPA key Size
	*/
	uint8	au8WepKey[WEP_104_KEY_STRING_SIZE + 1];
	/*!< Wep key
	*/
	uint8 	u8SecType;
	/*!< Security type: Open or WEP or WPA in the current implementation
	*/
	uint8 	u8SsidHide;
	/*!< SSID Status "Hidden(1)/Visible(0)"
	*/
	uint8	au8DHCPServerIP[4];
	/*!< Ap IP server address
	*/
	uint8	au8Key[M2M_MAX_PSK_LEN];
	/*!< WPA key
	*/
	uint8	__PAD16__[2];
	/*!< Padding bytes for forcing alignment
	*/
}tstrM2MAPConfig;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MAPConfig)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MAPConfig)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2MAPConfigExt

@brief	AP Configuration Extension

	This structure holds additional configuration parameters for the M2M AP mode. If modification of the extended parameters
	in AP mode is desired then @ref tstrM2MAPModeConfig should be set by the application, which contains the main AP configuration
	structure as well as this extended parameters structure.
	When configuring provisioning mode then @ref tstrM2MProvisionModeConfig should be used, which also contains the main AP configuration
	structure, this extended parameters structure and additional provisioning parameters.
*/
typedef struct {
	uint8	au8DefRouterIP[4];
	/*!< Ap Default Router address
	*/
	uint8	au8DNSServerIP[4];
	/*!< Ap DNS server address
	*/
    uint8   au8SubnetMask[4];
	/*!< Network Subnet Mask
	*/
}tstrM2MAPConfigExt;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MAPConfigExt)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MAPConfigExt)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2MAPModeConfig

@brief	AP Configuration

	This structure holds the AP configuration parameters plus the extended AP configuration parameters for the M2M AP mode.
	It should be set by the application when it requests to enable the M2M AP operation mode. The M2M AP mode currently
	supports only WEP security (with the NO Security option available of course).
	*/
typedef struct {
	tstrM2MAPConfig		strApConfig;
	/*!<
		Configuration parameters for the WiFi AP.
	*/
	tstrM2MAPConfigExt		strApConfigExt;
	/*!<
		Additional configuration parameters for the WiFi AP.
	*/
}tstrM2MAPModeConfig;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MAPModeConfig)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MAPModeConfig)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mServerInit

@brief
	PS Server initialization.
*/
typedef struct {
	uint8 	u8Channel;
	/*!< Server Listen channel
	*/
	uint8	__PAD24__[3];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mServerInit;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mServerInit)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mServerInit)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mClientState

@brief
	PS Client State.
*/
typedef struct {
	uint8 	u8State;
	/*!< PS Client State
	*/
	uint8	__PAD24__[3];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mClientState;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mClientState)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mClientState)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2Mservercmd

@brief
	PS Server CMD
*/
typedef struct {
	uint8	u8cmd;
	/*!< PS Server Cmd
	*/
	uint8	__PAD24__[3];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2Mservercmd;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2Mservercmd)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2Mservercmd)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrM2mSetMacAddress

@brief
	Sets the MAC address from application. The WINC load the mac address from the effuse by default to the WINC configuration memory, 
	but that function is used to let the application overwrite the configuration memory with the mac address from the host.

@note
	It's recommended to call this only once before calling connect request and after the m2m_wifi_init
*/
typedef struct {
	uint8 	au8Mac[6];
	/*!< MAC address array
	*/
	uint8	__PAD16__[2];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2mSetMacAddress;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2mSetMacAddress)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2mSetMacAddress)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
 	tstrM2MDeviceNameConfig

@brief	Device name

	It is assigned by the application. It is used mainly for Wi-Fi Direct device
	discovery and WPS device information.
*/
typedef struct {
	uint8 	au8DeviceName[M2M_DEVICE_NAME_MAX];
	/*!< NULL terminated device name
	*/
}tstrM2MDeviceNameConfig;


/*!
@struct	\
 	tstrM2MIPConfig

@brief
 	IP configuration (static/DHCP). The same structure is used for DCHP callback as well as static IP configuration.

@note
 	All member IP addresses are expressed in Network Byte Order (eg. "192.168.10.1" will be expressed as 0x010AA8C0).
 */
typedef struct {
	uint32 	u32StaticIP;
	/*!< If DHCP callback, this is the IP address obtained from the DHCP. In static IP config, this is the assigned to the device from the application.
	*/
	uint32 	u32Gateway;
	/*!< IP of the default internet gateway.
	*/
	uint32 	u32DNS;
	/*!< IP for the DNS server.
	*/
	uint32 	u32AlternateDNS;
	/*!< IP for the secondary DNS server (if any). Must set to zero if not provided in static IP configuration from the application.
	*/
	uint32 	u32SubnetMask;
	/*!< Subnet mask for the local area network.
	*/
	uint32 u32DhcpLeaseTime;
	/*!< DHCP Lease Time in sec. This field is is ignored in static IP configuration.
	*/
} tstrM2MIPConfig;

/*!
@struct	\
 	tstrM2mIpRsvdPkt

@brief
 	Received Packet Size and Data Offset

 */
typedef struct{
	uint16	u16PktSz;
	uint16	u16PktOffset;
} tstrM2mIpRsvdPkt;


/*!
@struct	\
 	tstrM2MProvisionModeConfig

@brief
 	M2M Provisioning Mode Configuration
 */

typedef struct {
	tstrM2MAPConfig		strApConfig;
	/*!<
		Configuration parameters for the WiFi AP.
	*/
	char				acHttpServerDomainName[64];
	/*!<
		The device domain name for HTTP provisioning.
	*/
	uint8				u8EnableRedirect;
	/*!<
		A flag to enable/disable HTTP redirect feature for the HTTP Provisioning server. If the Redirect is enabled,
		all HTTP traffic (http://URL) from the device associated with WINC AP will be redirected to the HTTP Provisioning Web page.
		- 0 : Disable HTTP Redirect.
		- 1 : Enable HTTP Redirect.
	*/
	tstrM2MAPConfigExt		strApConfigExt;
	/*!<
		Additional configuration parameters for the WiFi AP.
	*/
	uint8			__PAD24__[3];
}tstrM2MProvisionModeConfig;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MProvisionModeConfig)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MProvisionModeConfig)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
 	tstrM2MProvisionInfo

@brief
 	M2M Provisioning Information obtained from the HTTP Provisioning server.
 */
typedef struct{
	uint8	au8SSID[M2M_MAX_SSID_LEN];
	/*!<
		Provisioned SSID.
	*/
	uint8	au8Password[M2M_MAX_PSK_LEN];
	/*!<
		Provisioned Password.
	*/
	uint8	u8SecType;
	/*!<
		Wifi Security type.
	*/
	uint8	u8Status;
	/*!<
		Provisioning status. It must be checked before reading the provisioning information. It may be
		- M2M_SUCCESS 	: Provision successful.
		- M2M_FAIL		: Provision Failed.
	*/
}tstrM2MProvisionInfo;


/*!
@struct	\
 	tstrM2MConnInfo

@brief
 	M2M Provisioning Information obtained from the HTTP Provisioning server.
 */
typedef struct{
	char		acSSID[M2M_MAX_SSID_LEN];
	/*!< AP connection SSID name  */
	uint8	u8SecType;
	/*!< Security type */
	uint8	au8IPAddr[4];
	/*!< Connection IP address */
	uint8	au8MACAddress[6];
	/*!< MAC address of the peer Wi-Fi station */ 
	sint8	s8RSSI;
	/*!< Connection RSSI signal */
	uint8	u8CurrChannel; 
	/*!< Wi-Fi RF channel number  1,2,... 14.  */
	uint8	__PAD16__[2];
	/*!< Padding bytes for forcing 4-byte alignment */
}tstrM2MConnInfo;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MConnInfo)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MConnInfo)%4)==0, "Structure alignment error");
#endif
#endif

/**@}*/     //WlanEnums

/**@addtogroup  SSLEnums
 * @{
 */

typedef enum {
    M2M_SSL_REQ_CERT_VERIF,
    M2M_SSL_REQ_ECC,
    M2M_SSL_RESP_ECC,
    M2M_SSL_IND_CRL,
    M2M_SSL_REQ_WRITE_OWN_CERTS,
    M2M_SSL_REQ_SET_CS_LIST,
    M2M_SSL_RESP_SET_CS_LIST,
    M2M_SSL_RESP_WRITE_OWN_CERTS
} tenuM2mSslCmd;

/*
 * TLS certificate revocation list
 * Typedefs common between fw and host
 */

/*!
@struct \
    tstrTlsCrlEntry

@brief
    Certificate data for inclusion in a revocation list (CRL)
*/
typedef struct {
    uint8   u8DataLen;
    /*!<
        Length of certificate data (maximum possible is @ref TLS_CRL_DATA_MAX_LEN)
    */
    uint8   au8Data[TLS_CRL_DATA_MAX_LEN];
    /*!<
        Certificate data
    */
    uint8   __PAD24__[3];
    /*!<
        Padding bytes for forcing 4-byte alignment
    */
} tstrTlsCrlEntry;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrTlsCrlEntry)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrTlsCrlEntry)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct \
    tstrTlsCrlInfo

@brief
    Certificate revocation list details
*/
typedef struct {
    uint8           u8CrlType;
    /*!<
        Type of certificate data contained in list
    */
    uint8           u8Rsv1;
    /*!<
        Reserved for future use
    */
    uint8           u8Rsv2;
    /*!<
        Reserved for future use
    */
    uint8           u8Rsv3;
    /*!<
        Reserved for future use
    */
    tstrTlsCrlEntry astrTlsCrl[TLS_CRL_MAX_ENTRIES];
    /*!<
        List entries
    */
} tstrTlsCrlInfo;

/*!
@enum\
tenuSslCertExpSettings

@brief  SSL Certificate Expiry Validation Options
*/
typedef enum {
    SSL_CERT_EXP_CHECK_DISABLE,
    /*!<
        ALWAYS OFF.
        Ignore certificate expiration date validation. If a certificate is
        expired or there is no configured system time, the SSL connection SUCCEEDs.
    */
    SSL_CERT_EXP_CHECK_ENABLE,
    /*!<
        ALWAYS ON.
        Validate certificate expiration date. If a certificate is expired or
        there is no configured system time, the SSL connection FAILs.
    */
    SSL_CERT_EXP_CHECK_EN_IF_SYS_TIME
    /*!<
        CONDITIONAL VALIDATION (Default setting at startup).
        Validate the certificate expiration date only if there is a configured system time.
        If there is no configured system time, the certificate expiration is bypassed and the
        SSL connection SUCCEEDs.
    */
} tenuSslCertExpSettings;


/*!
@struct \
    tstrTlsSrvSecFileEntry

@brief
    This struct contains a TLS certificate.
 */
typedef struct {
    char    acFileName[TLS_FILE_NAME_MAX];
    /*!< Name of the certificate.   */
    uint32  u32FileSize;
    /*!< Size of the certificate.   */
    uint32  u32FileAddr;
    /*!< Error Code.    */
} tstrTlsSrvSecFileEntry;

/*!
@struct \
    tstrTlsSrvSecHdr

@brief
    This struct contains a set of TLS certificates.
 */
typedef struct {
    uint8                   au8SecStartPattern[TLS_SRV_SEC_START_PATTERN_LEN];
    /*!< Start pattern. */
    uint32                  u32nEntries;
    /*!< Number of certificates stored in the struct.   */
    uint32                  u32NextWriteAddr;
    /*  */
    tstrTlsSrvSecFileEntry  astrEntries[TLS_SRV_SEC_MAX_FILES];
    /*!< TLS Certificate headers.   */
    uint32                  u32CRC;
    /*!< CRC32 of entire cert block, only the cert writer computes this, the FW just does a compare with replacement blocks.    */
} tstrTlsSrvSecHdr;

typedef enum {
    TLS_FLASH_OK,
    /*!< Operation succeeded. Flash modified. */
    TLS_FLASH_OK_NO_CHANGE,
    /*!< Operation was unnecessary. Flash not modified. */
    TLS_FLASH_ERR_CORRUPT,
    /*!< Operation failed. Flash modified. */
    TLS_FLASH_ERR_NO_CHANGE,
    /*!< Operation failed. Flash not modified. */
    TLS_FLASH_ERR_UNKNOWN
    /*!< Operation failed. Flash status unknown. */
} tenuTlsFlashStatus;

typedef struct {
    uint16  u16Sig;
    uint16  u16TotalSize32;
    uint16  u16Offset32;
    uint16  u16Size32;
} tstrTlsSrvChunkHdr;

typedef struct {
    uint32  u32CsBMP;
} tstrSslSetActiveCsList;

/**@}*/     //SSLEnums

#define tstrM2MSNTPConfig_PAD (4 - ((M2M_NTP_MAX_SERVER_NAME_LENGTH + 1 + 1) % 4))

/**@addtogroup WlanEnums
 * @{
 */

/*!
@struct	\
	tstrM2MSNTPConfig

@brief	SNTP Client Configuration

	Configuration structure for the SNTP client.
*/
typedef struct {
	/*!<
		Configuration parameters for the NTP Client.
	*/
	char 					acNTPServer[M2M_NTP_MAX_SERVER_NAME_LENGTH + 1];
	/*!< Custom NTP server name.
	*/
	tenuSNTPUseDHCP			enuUseDHCP;
	/*!< Use NTP server provided by the DHCP server when falling back
	*/
#if tstrM2MSNTPConfig_PAD != 4
	uint8					__PAD8__[tstrM2MSNTPConfig_PAD];
	/*!< Padding bytes for forcing 4-byte alignment
	*/
#endif
}tstrM2MSNTPConfig;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MSNTPConfig)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MSNTPConfig)%4)==0, "Structure alignment error");
#endif
#endif

/**@}*/     //WlanEnums

/**@addtogroup OTATYPEDEF
 * @{
 */

/*!
@enum   \
    tenuM2mOtaCmd

@brief
    OTA Command IDs.
*/
typedef enum {
    M2M_OTA_REQ_NOTIF_SET_URL = M2M_OTA_CMD_BASE,
    M2M_OTA_REQ_NOTIF_CHECK_FOR_UPDATE,
    M2M_OTA_REQ_NOTIF_SCHED,
    M2M_OTA_REQ_START_FW_UPDATE,
    M2M_OTA_REQ_SWITCH_FIRMWARE,
    M2M_OTA_REQ_ROLLBACK_FW,
    M2M_OTA_RESP_NOTIF_UPDATE_INFO,
    M2M_OTA_RESP_UPDATE_STATUS,
    M2M_OTA_REQ_TEST,
    M2M_OTA_REQ_START_CRT_UPDATE,
    M2M_OTA_REQ_SWITCH_CRT_IMG,
    M2M_OTA_REQ_ROLLBACK_CRT,
    M2M_OTA_REQ_ABORT,
    M2M_OTA_REQ_HOST_FILE_STATUS,
    M2M_OTA_RESP_HOST_FILE_STATUS,
    M2M_OTA_REQ_HOST_FILE_DOWNLOAD,
    M2M_OTA_RESP_HOST_FILE_DOWNLOAD,
    M2M_OTA_REQ_HOST_FILE_READ,
    M2M_OTA_RESP_HOST_FILE_READ,
    M2M_OTA_REQ_HOST_FILE_ERASE,
    M2M_OTA_RESP_HOST_FILE_ERASE,
    M2M_OTA_MAX_ALL,
} tenuM2mOtaCmd;

/*!
@enum   \
    tenuOtaUpdateStatus

@brief
    OTA return status
*/
typedef enum {
    OTA_STATUS_SUCCESS        = 0,
    /*!< OTA Success with no errors */
    OTA_STATUS_FAIL           = 1,
    /*!< OTA generic fail */
    OTA_STATUS_INVALID_ARG    = 2,
    /*!< Invalid or malformed download URL */
    OTA_STATUS_INVALID_RB_IMAGE    = 3,
    /*!< Invalid rollback image */
    OTA_STATUS_INVALID_FLASH_SIZE    = 4,
    /*!< Flash size on device is not enough for OTA */
    OTA_STATUS_ALREADY_ENABLED    = 5,
    /*!< An OTA operation is already enabled */
    OTA_STATUS_UPDATE_INPROGRESS    = 6,
    /*!< An OTA operation update is in progress */
    OTA_STATUS_IMAGE_VERIF_FAILED = 7,
    /*!<  OTA Verification failed */
    OTA_STATUS_CONNECTION_ERROR = 8,
    /*!< OTA connection error */
    OTA_STATUS_SERVER_ERROR = 9,
    /*!< OTA server Error (file not found or else ...) */
    OTA_STATUS_ABORTED        = 10
                                /*!< OTA download has been aborted by the application. */
} tenuOtaUpdateStatus;

/*!
@enum   \
    tenuOtaUpdateStatusType

@brief
    OTA update Status type
*/
typedef enum {

    DL_STATUS        = 1,
    /*!< Download WINC OTA file status
    */
    SW_STATUS        = 2,
    /*!< Switching to the upgrade firmware status
    */
    RB_STATUS        = 3,
    /*!< Roll-back status
    */
    AB_STATUS        = 4,
    /*!< Abort status
    */
    HFD_STATUS       = 5,
    /*!< Host File Download status
    */
} tenuOtaUpdateStatusType;

/*!
@struct	\
 	tstrOtaInitHdr

@brief
 	OTA Image Header 
 */

typedef struct{
	uint32 u32OtaMagicValue;
	/*!< Magic value kept in the OTA image after the 
	sha256 Digest buffer to define the Start of OTA Header */
	uint32 u32OtaPayloadSize;
	/*!<
	The Total OTA image payload size, include the sha256 key size
	*/

}tstrOtaInitHdr;
	

/*!
@struct	\
 	tstrOtaControlSec

@brief
 	Control section structure is used to define the working image and 
	the validity of the roll-back image and its offset, also both firmware versions is kept in that structure.
 */

typedef struct {
	uint32 u32OtaMagicValue;
/*!<
	Magic value used to ensure the structure is valid or not 
*/
	uint32 u32OtaFormatVersion;
/*!<
		NA   NA   NA   Flash version   cs struct version
		00   00   00   00              00 
	Control structure format version, the value will be incremented in case of structure changed or updated
*/
	uint32 u32OtaSequenceNumber;
/*!<
	Sequence number is used while update the control structure to keep track of how many times that section updated 
*/
	uint32 u32OtaLastCheckTime;
/*!<
	Last time OTA check for update
*/
	uint32 u32OtaCurrentworkingImagOffset;
/*!<
	Current working offset in flash 
*/
	uint32 u32OtaCurrentworkingImagFirmwareVer;
/*!<
	current working image version ex 18.0.1
*/
	uint32 u32OtaRollbackImageOffset;
/*!<
	Roll-back image offset in flash 
*/
	uint32 u32OtaRollbackImageValidStatus;
/*!<
	roll-back image valid status 
*/
	uint32 u32OtaRollbackImagFirmwareVer;
/*!<
	Roll-back image version (ex 18.0.3)
*/
	uint32 u32OtaCortusAppWorkingOffset;
/*!<
	cortus app working offset in flash 
*/
	uint32 u32OtaCortusAppWorkingValidSts;
/*!<
	Working Cortus app valid status 
*/
	uint32 u32OtaCortusAppWorkingVer;
/*!<
	Working cortus app version (ex 18.0.3)
*/
	uint32 u32OtaCortusAppRollbackOffset;
/*!<
	cortus app rollback offset in flash 
*/
	uint32 u32OtaCortusAppRollbackValidSts;
/*!<
	roll-back cortus app valid status 
*/
	uint32 u32OtaCortusAppRollbackVer;
/*!<
	Roll-back cortus app version (ex 18.0.3)
*/
	uint32 u32OtaControlSecCrc;
/*!<
	CRC for the control structure to ensure validity 
*/
} tstrOtaControlSec;

/*!
@struct	\
	tstrOtaUpdateStatusResp

@brief
	OTA Update Information

@sa
	tenuWPSTrigger
*/
typedef struct {
	uint8	u8OtaUpdateStatusType;
	/*!<
		Status type tenuOtaUpdateStatusType
	*/
	uint8	u8OtaUpdateStatus;
	/*!<
	OTA_SUCCESS 						
	OTA_ERR_WORKING_IMAGE_LOAD_FAIL		
	OTA_ERR_INVALID_CONTROL_SEC			
	M2M_ERR_OTA_SWITCH_FAIL     		
	M2M_ERR_OTA_START_UPDATE_FAIL     	
	M2M_ERR_OTA_ROLLBACK_FAIL     		
	M2M_ERR_OTA_INVALID_FLASH_SIZE     	
	M2M_ERR_OTA_INVALID_ARG		     
	*/
	uint8 _PAD16_[2];
}tstrOtaUpdateStatusResp;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrOtaUpdateStatusResp)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrOtaUpdateStatusResp)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
	tstrOtaUpdateInfo

@brief
	OTA Update Information

@sa
	tenuWPSTrigger
*/
typedef struct {
	uint32	u8NcfUpgradeVersion;
	/*!< NCF OTA Upgrade Version
	*/
	uint32	u8NcfCurrentVersion;
	/*!< NCF OTA Current firmware version
	*/
	uint32	u8NcdUpgradeVersion;
	/*!< NCD (host) upgraded version (if the u8NcdRequiredUpgrade == true)
	*/
	uint8	u8NcdRequiredUpgrade;
	/*!< NCD Required upgrade to the above version
	*/
	uint8 	u8DownloadUrlOffset;
	/*!< Download URL offset in the received packet
	*/
	uint8 	u8DownloadUrlSize;
	/*!< Download URL size in the received packet
	*/
	uint8	__PAD8__;
	/*!< Padding bytes for forcing 4-byte alignment
	*/
} tstrOtaUpdateInfo;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrOtaUpdateInfo)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrOtaUpdateInfo)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct \
    tstrOtaHostFileGetStatusResp

@brief
    Host File OTA Information
*/
typedef struct {
    uint32  u32OtaFileSize;
    /*!<
    Reports the size of the downloaded file.
    Valid if u8OtaFileGetStatus=OTA_STATUS_SUCCESS.
    */

    uint8   u8OtaFileGetStatus;
    /*!<
    The status of the File Get operation.
    See @ref tenuOtaUpdateStatus.
    */
    uint8   u8CFHandler;
    /*!<
    The file handler stored in the WINC for a valid file.
    Valid if u8OtaFileGetStatus=OTA_STATUS_SUCCESS.
     */
    uint8   __PAD16__[2];
    /*!<
    Padding byte for forcing 4-byte alignment
     */
}tstrOtaHostFileGetStatusResp;

#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrOtaHostFileGetStatusResp)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrOtaHostFileGetStatusResp)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct \
    tstrOtaHostFileReadStatusResp

@brief
    Host File OTA Information
*/
typedef struct {
    uint16  FileBlockSz;
    /*!<
    Reports the size of the block of data read via HIF.
    Valid if u8OtaFileReadStatus=OTA_STATUS_SUCCESS .
    */

    uint8   u8OtaFileReadStatus;
    /*!<
    The status of the File Read operation.
    See @ref tenuOtaUpdateStatus.
    */

    uint8	__PAD8__;
    /*!<
    Padding byte for forcing 4-byte alignment
     */

    uint8   pFileBuf[MAX_FILE_READ_STEP];
    /*!<
    Pointer to the temporary buffer containing the data just read.
    Max size is @ref MAX_FILE_READ_STEP
    */
}tstrOtaHostFileReadStatusResp;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrOtaHostFileReadStatusResp)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrOtaHostFileReadStatusResp)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct \
    tstrOtaHostFileEraseStatusResp

@brief
    Host File OTA Information
*/
typedef struct {
    uint8   u8OtaFileEraseStatus;
    /*!<
    The status of the File Erase operation.
    See @ref tenuOtaUpdateStatus.
    */

    uint8	__PAD24__[3];
    /*!< Padding byte for forcing 4-byte alignment
     */
}tstrOtaHostFileEraseStatusResp;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrOtaHostFileEraseStatusResp)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrOtaHostFileEraseStatusResp)%4)==0, "Structure alignment error");
#endif
#endif

/**@}*/     //OTATYPEDEF

/**@addtogroup WlanEnums
 * @{
 */

/*!
@struct	\
	tstrSystemTime

@brief
	Used for time storage.
*/
typedef struct{
	uint16	u16Year;
	uint8	u8Month;
	uint8	u8Day;
	uint8	u8Hour;
	uint8	u8Minute;
	uint8	u8Second;
	uint8	__PAD8__;
}tstrSystemTime;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrSystemTime)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrSystemTime)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
 	tstrM2MMulticastMac

@brief
 	M2M add/remove multi-cast mac address
 */
 typedef struct {
	uint8 au8macaddress[M2M_MAC_ADDRES_LEN];
	/*!<
		Mac address needed to be added or removed from filter.
	*/
	uint8 u8AddRemove;
	/*!<
		set by 1 to add or 0 to remove from filter.
	*/
	uint8	__PAD8__;
	/*!< Padding bytes for forcing 4-byte alignment
	*/
}tstrM2MMulticastMac;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrM2MMulticastMac)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrM2MMulticastMac)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@struct	\
 	tstrPrng

@brief
 	M2M Request PRNG
 */
 typedef struct {
	 /*!<
		return buffer address
	*/
	uint8 *pu8RngBuff;
	 /*!<
		PRNG size requested
	*/
	uint16 	u16PrngSize;
	/*!<
		PRNG pads
	*/
	uint8 __PAD16__[2];
}tstrPrng;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrPrng)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrPrng)%4)==0, "Structure alignment error");
#endif
#endif

/*!
@enum\
	tenuWlanTxRate

@brief	All possible supported 802.11 WLAN TX rates.
*/
typedef enum {
	TX_RATE_AUTO  = 0xFF, /*!<  Automatic rate selection */
	TX_RATE_LOWEST  = 0xFE, /*!< Force the lowest possible data rate for longest range. */		
	TX_RATE_1	  = 0x00, /* 1 Mbps  */
	TX_RATE_2	  = 0x01, /* 2 Mbps  */
	TX_RATE_5_5   = 0x02, /* 5 Mbps  */
	TX_RATE_11	  = 0x03, /* 11 Mbps */
	TX_RATE_6	  = 0x0B, /* 6 Mbps  */
	TX_RATE_9	  = 0x0F, /* 9 Mbps  */
	TX_RATE_12	  = 0x0A, /* 12 Mbps */
	TX_RATE_18	  = 0x0E, /* 18 Mbps */
	TX_RATE_24	  = 0x09, /* 24 Mbps */
	TX_RATE_36	  = 0x0D, /* 36 Mbps */
	TX_RATE_48	  = 0x08, /* 48 Mbps */
	TX_RATE_54	  = 0x0C, /* 54 Mbps */
	TX_RATE_MCS_0 = 0x80, /* MCS-0: 6.5 Mbps */
	TX_RATE_MCS_1 = 0x81, /* MCS-1: 13 Mbps */
	TX_RATE_MCS_2 = 0x82, /* MCS-2: 19.5 Mbps */
	TX_RATE_MCS_3 = 0x83, /* MCS-3: 26 Mbps */
	TX_RATE_MCS_4 = 0x84, /* MCS-4: 39 Mbps */
	TX_RATE_MCS_5 = 0x85, /* MCS-5: 52 Mbps */
	TX_RATE_MCS_6 = 0x86, /* MCS-6: 58.5 Mbps */
	TX_RATE_MCS_7 = 0x87, /* MCS-7: 65 Mbps */
} tenuWlanTxRate;

/*!
@struct	\
 	tstrConfAutoRate

@brief
 	Auto TX rate selection parameters passed to m2m_wifi_conf_auto_rate.
*/
typedef struct {
	uint16 u16ArMaxRecoveryFailThreshold;
	/*!<
		To stabilize the TX rate and avoid oscillation, the algorithm will not attempt to 
		push the rate up again after a failed attempt to push the rate up.
		An attempt to push the rate up is considered failed if the next rate suffers from 
		very high retransmission. In this case, WINC will not attempt again until a 
		duration of time is elapsed to keep the TX rate stable.
		The min duration is (u16ArMinRecoveryFailThreshold) seconds and doubles 
		on every failed attempt. The doubling continues until the duration is 
		(u16ArMaxRecoveryFailThreshold) max.
		
		Increasing u16ArMaxRecoveryFailThreshold will cause the TX rate to be 
		stable over a long period of time with fewer attempts to increase the data rate. 
		However, increasing it to a very large value will deter the algorithm from 
		attempting to increase the rate if, for instance, the wireless conditions before were better.

		Default is 5 seconds.
	*/
	uint16 u16ArMinRecoveryFailThreshold;
	/*!<
		To stabilize the TX rate and avoid oscillation, the algorithm will not attempt to 
		push the rate up again after a failed attempt to push the rate up.
		An attempt to push the rate up is considered failed if the next rate suffers from 
		very high retransmission. In this case, WINC will not attempt again until a 
		duration of time is elapsed to keep the TX rate stable.
		The min duration is (u16ArMinRecoveryFailThreshold) seconds and doubles 
		on every failed attempt. The doubling continues until the duration is 
		(u16ArMaxRecoveryFailThreshold) max.

		Default is 1 second.
	*/

	tenuWlanTxRate enuWlanTxRate;
	/*!<
		The TX data rate selected as enumerated in tenuWlanTxRate
		Default is TX_RATE_AUTO.
		
		WINC shall override the rate provided through this API if it not supported by the peer WLAN device (STA/AP). 
		For instance, if the TX_RATE_MCS_0 is requested while the connection is to a BG only AP, WINC shall 
		elect the nearest BG data rate to the requested rate. In this example, it will be TX_RATE_9.
	*/
	tenuWlanTxRate enuArInitialRateSel;
	/*!<
		Configures the initial WLAN TX rate used right after association. 
		This is the starting point for auto rate algorithm.
		The algorithm tunes the rate up or down based on the wireless 
		medium condition if enuWlanTxRate is set to TX_RATE_AUTO. 
		If enuWlanTxRate is set to any value other than TX_RATE_AUTO, then 
		u8ArInitialRateSel is ignored.

		By default WINC selects the best initial rate based on the receive
		signal level from the WLAN peer. For applications that favor range 
		right after association, TX_RATE_LOWEST can bs used.
	*/
	uint8 u8ArEnoughTxThreshold; 
	/*!<
		Configures the minimum number of transmitted packets per second for auto 
		rate selection algorithm to start to make rate up or down decisions.
		Default is 10. 
	*/
	uint8 u8ArSuccessTXThreshold;
	/*!<
		Configures the threshold for rate up. Rate goes up if number of 
		WLAN TX retries is less than (1/u8ArSuccessTXThreshold) of the 
		number of packet transmitted within one second. 
		This can be tuned to speed up or slow down the rate at which the algorithm 
		moves the WLAN TX rate up. Default value is 5.
	*/
	uint8 u8ArFailTxThreshold;
	/*!<
		Configures the threshold for rate down. Rate goes down if number of 
		WLAN TX retries is greater than (1/u8ArFailTxThreshold) of the 
		number of packet transmitted within one second. 
		This can be tuned to speed up or slow down the rate at which the algorithm 
		moves the WLAN TX rate down. Default value is 3.
	*/
	uint8 __PAD24__[3];	
	/*!< Pad bytes for forcing 4-byte alignment
	*/
} tstrConfAutoRate;
#ifndef _lint
#ifdef __GNUC__
_Static_assert((sizeof(tstrConfAutoRate)%4)==0, "Structure alignment error");
#else
static_assert((sizeof(tstrConfAutoRate)%4)==0, "Structure alignment error");
#endif
#endif

/**@}*/     //WlanEnums

#endif

/**
 *
 * \file
 *
 * \brief WINC WLAN Application Interface.
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

/**@defgroup m2m_wifi WLAN
    @{
        @defgroup   WLANCallbacks   Callbacks
        @brief
            Provides detail on the available callbacks for the Wlan APIs.

        @defgroup   WlanDefines     Defines
        @brief
            Specifies the macros and defines used by the Wlan APIs.

        @defgroup   WlanEnums       Enumerations and Typedefs
        @brief
            Specifies the enums and Data Structures used by the Wlan APIs.

        @defgroup   WLANAPI         Functions
        @brief
            Here are listed all the functions that implement the Wlan APIs.
@}
 */

#ifndef __M2M_WIFI_H__
#define __M2M_WIFI_H__

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//#include <compiler.h>
#include "common/include/nm_common.h"
#include "driver/include/m2m_types.h"
#include "driver/include/nmdrv.h"

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/**@addtogroup WLANCallbacks
*/
/**@{*/

/*!
@typedef void (*tpfAppWifiCb)(uint8 u8MsgType, void* pvMsg);

@brief	
    Wi-Fi's main callback function handler, for handling the M2M_WIFI events received on the 
    Wi-Fi interface. Such notifications are received in response to Wi-Fi operations such
    as @ref m2m_wifi_request_scan, @ref m2m_wifi_connect_open, @ref m2m_wifi_connect_wep,
    @ref m2m_wifi_connect_psk, @ref m2m_wifi_connect_1x_mschap2, @ref m2m_wifi_connect_1x_tls.
 
    Wi-Fi operations are implemented in an asynchronous mode, and all incoming information/status
				are to be handled through this callback function when the corresponding notification is received.
		
    Applications are expected to assign this wi-fi callback function by 
    calling @ref m2m_wifi_init.
@param [in]	u8MsgType
    Type of notification. Possible types are:
       - @ref M2M_WIFI_RESP_CON_STATE_CHANGED
       - @ref M2M_WIFI_RESP_CONN_INFO
       - @ref M2M_WIFI_REQ_DHCP_CONF
       - @ref M2M_WIFI_REQ_WPS
       - @ref M2M_WIFI_RESP_IP_CONFLICT 
       - @ref M2M_WIFI_RESP_SCAN_DONE
       - @ref M2M_WIFI_RESP_SCAN_RESULT
       - @ref M2M_WIFI_RESP_CURRENT_RSSI
       - @ref M2M_WIFI_RESP_CLIENT_INFO
       - @ref M2M_WIFI_RESP_PROVISION_INFO
       - @ref M2M_WIFI_RESP_DEFAULT_CONNECT
       - @ref M2M_WIFI_RESP_ETHERNET_RX_PACKET (In case Ethernet/Bypass mode is defined)
       - @ref M2M_WIFI_RESP_WIFI_RX_PACKET (In case monitoring mode is used)
				
@param [in]	pvMsg
				A pointer to a buffer containing the notification parameters (if any). It should be
    cast to the correct data type corresponding to the notification type.

@see
	tstrM2mWifiStateChanged
	tstrM2MWPSInfo
	tstrM2mScanDone
	tstrM2mWifiscanResult
    m2m_wifi_init
*/
typedef void (*tpfAppWifiCb) (uint8 u8MsgType, void * pvMsg);

/*!
@typedef void (*tpfAppEthCb)(uint8 u8MsgType, void* pvMsg, void* pvCtrlBuf);

@brief	
    Ethernet (Bypass mode) notification callback function receiving Bypass mode events as 
    defined in the Wi-Fi responses enumeration @ref tenuM2mStaCmd. 

    If bypass mode is enabled, applications must ensure this callback function is registered 
    with the Wi-Fi driver by calling @ref m2m_wifi_init.

@param [in]	u8MsgType
	Type of notification. Possible types are:
        - @ref M2M_WIFI_RESP_ETHERNET_RX_PACKET

@param [in]	pvMsg
    A pointer to a buffer containing the notification parameters (if any). 
	It should be cast to the correct data type corresponding to the notification type.
    
	For example, it could be a pointer to the buffer holding the received frame in case of @ref M2M_WIFI_RESP_ETHERNET_RX_PACKET
	event.
	
@param [in]	pvControlBuf
	A pointer to control buffer describing the accompanied message.
    To be cast to @ref tstrM2mIpCtrlBuf in case of @ref M2M_WIFI_RESP_ETHERNET_RX_PACKET event.

@warning
	Make sure that the application defines ETH_MODE.

@see
	m2m_wifi_init
*/
typedef void (*tpfAppEthCb) (uint8 u8MsgType, void * pvMsg,void * pvCtrlBuf);

/*!
@typedef void (*tpfAppMonCb)(tstrM2MWifiRxPacketInfo* pstrWifiRxPacket, uint8* pu8Payload, uint16 u16PayloadSize);

@brief	
    Wi-Fi monitoring mode callback function. This function delivers all received wi-Fi packets 
    through the Wi-Fi interface. Applications requiring to operate in the monitoring should call the asynchronous 
    function m2m_wifi_enable_monitoring_mode and expect to receive the Wi-Fi packets through this 
    callback function, when the event is received.
	To disable the monitoring mode a call to @ref m2m_wifi_disable_monitoring_mode should be made.

@param [in]	pstrWifiRxPacket
				Pointer to a structure holding the Wi-Fi packet header parameters.

@param [in]	pu8Payload
    Pointer to the buffer holding the Wi-Fi packet payload information required by the application 
    starting from the defined OFFSET by the application (when calling 
    m2m_wifi_enable_monitoring_mode). Could hold a value of NULL, if the application does not need 
    any data from the payload.

@param [in]	u16PayloadSize
				The size of the payload in bytes.
				
@see
    m2m_wifi_enable_monitoring_mode,
    m2m_wifi_init	
	
@warning
	u16PayloadSize should not exceed the buffer size given through m2m_wifi_enable_monitoring_mode.
*/
typedef void (*tpfAppMonCb) (tstrM2MWifiRxPacketInfo *pstrWifiRxPacket, uint8 * pu8Payload, uint16 u16PayloadSize);
/**@}*/     //WLANCallbacks

/**@addtogroup  WlanEnums
 * @{*/
/*!
@enum   \
    tenuWifiState
@brief
    Enumeration for Wi-Fi state
    The following is used to track the state of the wifi (not initialized, initialized or started)

@remarks
    This is useful when putting the WINC in "download mode" to access the flash via SPI. By using
    @ref m2m_wifi_get_state and checking against the desired state, it is possible to validate if
    the Application should proceed with the SPI Flash access or not.
*/
typedef enum {
    WIFI_STATE_DEINIT,
    /*!< Wifi is not initialized */
    WIFI_STATE_INIT,
    /*!< Wifi has been initialized */
    WIFI_STATE_START,
    /*!< Wifi has started */
} tenuWifiState;

typedef enum {
    WIFI_CRED_DONTSAVE,
    /*!< Credentials will not be stored in WINC flash. */
    WIFI_CRED_SAVE_UNENCRYPTED,
    /*!< Credentials will be stored unencrypted in WINC flash. */
    WIFI_CRED_SAVE_ENCRYPTED
    /*!< Credentials will be stored encrypted in WINC flash.
            The encryption is not secure; it is merely intended to prevent sensitive information
            being leaked by an opportunistic read of WINC flash contents.
            The encryption keys involve WINC efuse contents, so WINC efuses should not be written
            while this option is in use. */
} tenuCredStoreOption;

/*!
@struct 	\
	tstrEthInitParam
	
@brief		
	Structure to hold Ethernet interface parameters. 
    Structure is to be defined and have its attributes set, based on the application's functionality
    before a call is made to initialize the wi-fi operations by calling the
    @ref m2m_wifi_init function.
    Part of the wi-fi configuration structure @ref tstrWifiInitParam.
	Applications shouldn't need to define this structure, if the bypass mode is not defined.
	
@see
	tpfAppEthCb
	tpfAppWifiCb
	m2m_wifi_init

@warning
	Make sure that application defines ETH_MODE before using @ref tstrEthInitParam. 
*/
typedef struct {
    tpfAppWifiCb pfAppWifiCb;      /*!< Callback for wifi notifications. */
    tpfAppEthCb  pfAppEthCb;       /*!< Callback for Ethernet interface. */
    uint8 * au8ethRcvBuf;          /*!< Pointer to Receive Buffer of Ethernet Packet */
    uint16	u16ethRcvBufSize;      /*!< Size of Receive Buffer for Ethernet Packet */
	uint8 u8EthernetEnable;        /*!<	Enable Ethernet mode flag */
	uint8 __PAD8__;	               /*!< Padding	*/
} tstrEthInitParam;

/*!
@struct	\
 	tstrM2mIpCtrlBuf
 	
@brief		
 	Structure holding the incoming buffer's data size information, indicating the data size of the 
    buffer and the remaining buffer's data size. The data of the buffer which holds the packet sent 
    to the host when in the bypass mode, is placed in the @ref tstrEthInitParam::au8ethRcvBuf attribute.
    This following information is retrieved in the host when an event 
    @ref M2M_WIFI_RESP_ETHERNET_RX_PACKET is received in the Wi-Fi callback function 
    @ref tpfAppWifiCb. 

	The application is expected to use this structure's information to determine if there is still incoming data to be received from the firmware.
	
 @see
	tpfAppWifiCb     
	 tpfAppEthCb
	 tstrEthInitParam
 
 @warning
	 Make sure that ETHERNET/bypass mode is defined before using @ref tstrM2mIpCtrlBuf

 */
typedef struct{
    uint16	u16DataSize;          /*!< Size of the received data in bytes. */
    uint16	u16RemainingDataSize; /*!< Size of the remaining data bytes to be delivered to host. */
} tstrM2mIpCtrlBuf;

/**
@struct		\
	tstrWifiInitParam

@brief		
	Structure, holding the Wi-fi configuration attributes such as the wi-fi callback , monitoring mode callback and Ethernet parameter initialization structure.
	Such configuration parameters are required to be set before calling the wi-fi initialization function @ref m2m_wifi_init.
	@ref pfAppWifiCb attribute must be set to handle the wi-fi callback operations.
	@ref pfAppMonCb attribute, is optional based on whether the application requires the monitoring mode configuration, and can there not
	be set before the initialization.
	@ref strEthInitParam structure, is another optional configuration based on whether the bypass mode is set.

 @see
	 tpfAppEthCb
	 tpfAppMonCb
	 tstrEthInitParam
*/
typedef struct {
    tpfAppWifiCb pfAppWifiCb;     /*!< Callback for Wi-Fi notifications. */
    tpfAppMonCb  pfAppMonCb;      /*!< Callback for monitoring interface. */
    tstrEthInitParam strEthInitParam ; /*!< Structure to hold Ethernet interface parameters. */
} tstrWifiInitParam;

typedef struct {
    uint8           *pu8Bssid;
    /*!< Pointer to BSSID (6 bytes). Optional (may be NULL).
            If present, this restricts the connection attempt to APs that have a matching BSSID. */
    uint8           *pu8Ssid;
    /*!< Pointer to SSID. Required. */
    uint8           u8SsidLen;
    /*!< Length of SSID in bytes. Permitted values are between 0 and 32. */
    tenuM2mScanCh   enuChannel;
    /*!< Wi-Fi channel to connect on.
            If an appropriate AP cannot be found on this channel then connection fails.
            @ref M2M_WIFI_CH_ALL may be used to allow scanning of all channels. */
}tstrNetworkId;

/* Legacy Wep param structure. */
typedef struct {
    uint8   u8KeyIndx;
    uint8   u8KeySz;
    uint8   au8WepKey[WEP_104_KEY_STRING_SIZE + 1];	// NULL terminated
    uint8   __PAD24__[3];
}tstrM2mWifiWepParams;

/* Legacy 802.1x MsChapv2 param structure. */
typedef struct{
    uint8   au8UserName[21];    // NULL terminated
    uint8   au8Passwd[41];      // NULL terminated
}tstr1xAuthCredentials;

typedef struct {
    uint8   *pu8Psk;
    /*!< Pointer to PSK, represented as an ASCII string (64 characters, representing 32 bytes).
            Must be NULL if Passphrase is provided instead. */
    uint8   *pu8Passphrase;
    /*!< Pointer to Passphrase (Printable ASCII).
            Must be NULL if PSK is provided instead. */
    uint8   u8PassphraseLen;
    /*!< Length of Passphrase. Permitted values are between 8 and 63.
            This field is ignored if pu8Passphrase == NULL. */
}tstrAuthPsk;

typedef struct {
    uint8   *pu8WepKey;
    /*!< Pointer to WEP Key, represented as an ASCII string.
            (10 or 26 characters, representing 5 or 13 bytes.) */
    uint8   u8KeySz;
    /*!< Size of WEP Key string.
            Permitted values are @ref WEP_40_KEY_STRING_SIZE or @ref WEP_104_KEY_STRING_SIZE. */
    uint8   u8KeyIndx;
    /*!< WEP Key Index in the range 1 to 4. */
}tstrAuthWep;

typedef struct {
    uint8   *pu8Domain;
    /*!< Pointer to Domain of authentication server (printable ASCII), including '@' or '\'
            separator character as appropriate. Use NULL if there is no domain information.
            The Domain will be either prepended or appended to the UserName, depending on the
            setting of field bPrependDomain. \n
            Example 1: if [Domain]is "@my_domain" and bPrependDomain is false, then the EAP
            identity response is "[UserName]@my_domain". \n
            Example 2: if [Domain]is "my_domain\" and bPrependDomain is true, then the EAP
            identity response is "my_domain\[UserName]". */
    uint8   *pu8UserName;
    /*!< Pointer to UserName (ASCII).
            This will be sent (encrypted) in the tunneled EAP identity response (if applicable)
            and used during MSCHAPv2 authentication. If bUnencryptedUserName is true then it will
            also be sent (unencrypted) in the initial EAP identity response. */
    uint8   *pu8Password;
    /*!< Pointer to MSCHAPv2 Password (ASCII).
            This will be used during MSCHAPv2 authentication. */
    uint16  u16DomainLen;
    /*!< Length of Domain (in ASCII characters), including '@' or '\' separator character as
            appropriate.
            Permitted values are such that u16DomainLen + u16UserNameLen is between 0 and
            @ref M2M_AUTH_1X_USER_LEN_MAX. */
    uint16  u16UserNameLen;
    /*!< Length of UserName (in ASCII characters).
            Permitted values are such that u16DomainLen + u16UserNameLen is between 0 and
            @ref M2M_AUTH_1X_USER_LEN_MAX. */
    uint16  u16PasswordLen;
    /*!< Length of Password (in ASCII characters).
            Permitted values are between 0 and @ref M2M_AUTH_1X_PASSWORD_LEN_MAX. */
    bool    bUnencryptedUserName;
    /*!< Determines whether UserName or "anonymous" is sent (unencrypted) in the initial EAP
            identity response. Domain is sent in both cases. \n
            true: UserName is sent in the initial EAP identity response (not recommended).
            false: "anonymous" is sent in the initial EAP identity response. This setting is
            recommended for tunneled methods. MSCHAPv2 is always a tunneled method. */
    bool    bPrependDomain;
    /*!< Determines whether Domain is prepended or appended to UserName in EAP identity responses.
            true: Domain is prepended to UserName - [Domain][UserName].
            false: Domain is appended to UserName - [UserName][Domain]. */
}tstrAuth1xMschap2;

typedef struct {
    uint8   *pu8Domain;
    /*!< Pointer to Domain of authentication server (printable ASCII), including '@' or '\'
            separator character as appropriate. Use NULL if there is no domain information.
            The Domain will be either prepended or appended to the UserName, depending on the
            setting of field bPrependDomain. \n
            Example 1: if [Domain]is "@my_domain" and bPrependDomain is false, then the EAP
            identity response is "[UserName]@my_domain". \n
            Example 2: if [Domain]is "my_domain\" and bPrependDomain is true, then the EAP
            identity response is "my_domain\[UserName]". */
    uint8   *pu8UserName;
    /*!< Pointer to UserName (ASCII).
            This will be sent (encrypted) in the tunneled EAP identity response.
            If bUnencryptedUserName is true then it will also be sent (unencrypted) in the initial
            EAP identity response. */
    uint8   *pu8PrivateKey_Mod;
    /*!< Pointer to PrivateKey modulus (raw data).
            This will be used during TLS client authentication. */
    uint8   *pu8PrivateKey_Exp;
    /*!< Pointer to PrivateKey exponent (raw data).
            This will be used during TLS client authentication. */
    uint8   *pu8Certificate;
    /*!< Pointer to TLS client certificate corresponding to PrivateKey.
            This will be used during TLS client authentication. */
    uint16  u16DomainLen;
    /*!< Length of Domain (in ASCII characters), including '@' or '\' separator character as
            appropriate.
            Permitted values are such that u16DomainLen + u16UserNameLen is between 0 and
            @ref M2M_AUTH_1X_USER_LEN_MAX. */
    uint16  u16UserNameLen;
    /*!< Length of UserName (in ASCII characters).
            Permitted values are such that u16DomainLen + u16UserNameLen is between 0 and
            @ref M2M_AUTH_1X_USER_LEN_MAX. */
    uint16  u16PrivateKeyLen;
    /*!< Length of PrivateKey_Mod (in bytes).
            Permitted values are between 0 and @ref M2M_AUTH_1X_PRIVATEKEY_LEN_MAX, typically 128 or 256.
            PrivateKey_Exp must be the same length as modulus, pre-padded with 0s if necessary. */
    uint16  u16CertificateLen;
    /*!< Length of Certificate (in bytes).
            Permitted values are between 0 and @ref M2M_AUTH_1X_CERT_LEN_MAX. */
    bool    bUnencryptedUserName;
    /*!< Determines whether UserName or "anonymous" is sent (unencrypted) in the initial EAP
            identity response. Domain is sent in both cases. \n
            true: UserName is sent in the initial EAP identity response (required for EAP-TLS).
            false: "anonymous" is sent in the initial EAP identity response. This setting is
            recommended for tunneled methods such as EAP-PEAP/TLS. */
    bool    bPrependDomain;
    /*!< Determines whether Domain is prepended or appended to UserName in EAP identity responses.
            true: Domain is prepended to UserName - [Domain][UserName].
            false: Domain is appended to UserName - [UserName][Domain]. */
}tstrAuth1xTls;
/**@}*/     //WlanEnums

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
FUNCTION PROTOTYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/**@addtogroup WLANAPI
 */
/**@{*/

#ifdef __cplusplus
     extern "C" {
#endif

/*!
@fn	\
	NMI_API void  m2m_wifi_download_mode(void);

@brief
    Prepares the WINC board before downloading any data (Firmware, Certificates, etc).

@details
    This function should be called before attempting to download any data to the WINC board.
    Performs the appropriate WINC driver initialization, this includes bus initialization,
    interrupt enabling and it halts the chip to allow for the firmware downloads. Firmware
    can be downloaded through a number of interfaces, UART, I2C and SPI.

@return		
	The function returns @ref M2M_SUCCESS for successful operations  and a negative value otherwise.
*/
NMI_API sint8  m2m_wifi_download_mode(void);

/*!
@fn	\
	NMI_API sint8  m2m_wifi_init(tstrWifiInitParam * pWifiInitParam);

@brief
    Synchronous API to initialize the WINC driver.

@details
    This function initializes the WINC driver by registering the callback function for the M2M_WIFI layer
	(also the call back function for bypass mode/monitoring mode if defined), initializing the host 
	interface layer and the bus interfaces.	Wi-Fi callback registering is essential to allow the 
	handling of the events received, in response to the asynchronous Wi-Fi operations.

    The possible Wi-Fi events that are expected to be received through the callback
    function (provided by the application) to the M2M_WIFI layer are listed below:

     - @ref M2M_WIFI_RESP_CON_STATE_CHANGED
     - @ref M2M_WIFI_RESP_CONN_INFO
     - @ref M2M_WIFI_REQ_DHCP_CONF
     - @ref M2M_WIFI_REQ_WPS
     - @ref M2M_WIFI_RESP_IP_CONFLICT
     - @ref M2M_WIFI_RESP_SCAN_DONE
     - @ref M2M_WIFI_RESP_SCAN_RESULT
     - @ref M2M_WIFI_RESP_CURRENT_RSSI
     - @ref M2M_WIFI_RESP_CLIENT_INFO
     - @ref M2M_WIFI_RESP_PROVISION_INFO
     - @ref M2M_WIFI_RESP_DEFAULT_CONNECT
     - @ref M2M_WIFI_RESP_ETHERNET_RX_PACKET (if bypass mode is enabled)
     - @ref M2M_WIFI_RESP_WIFI_RX_PACKET (if monitoring mode is enabled)

    Any application using the WINC driver must call this function at the start of its main function.

@param [in]	pWifiInitParam
    This is a pointer to a structure of type @ref tstrWifiInitParam which contains pointers to the
	application WIFI layer callback function, monitoring mode callback and @ref tstrEthInitParam 
	structure (which contains initialization settings for bypass mode).
 
@return		
    The function returns @ref M2M_SUCCESS for successful operations  and a negative value otherwise.

@pre 
    Prior to this function call, The application should initialize the BSP using @ref nm_bsp_init.
    Also, application users must provide a call back function responsible for receiving all the
    wi-fi events that are received on the M2M_WIFI layer.
	
@warning
    Failure to successfully complete indicates that the driver could not be initialized and
    a fatal error will prevent the application from proceeding, proper error handling should be
    implemented by the application.
	
@see
	m2m_wifi_deinit
    m2m_wifi_init_hold
    m2m_wifi_init_start
    tstrWifiInitParam
	tenuM2mStaCmd
	tenuM2mStaCmd
*/
NMI_API sint8  m2m_wifi_init(tstrWifiInitParam * pWifiInitParam);

/*!
@fn	\
	NMI_API sint8  m2m_wifi_deinit(void * arg);
	
@brief	
	De-initialize the WINC driver and host interface. 

@details
    Synchronous de-initialization function for the WINC driver.
	De-initializes the host interface and frees any resources used by the M2M_WIFI layer. 
    This function must be called in the application closing phase to ensure that all
    resources have been correctly released.
	No arguments are expected to be passed in. 

@param [in]	arg
        Opaque argument, not used in current implementation. Application should use null.

@return		
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@note
    This function must be called at the de-initialization stage of the application.
    Generally this function should be the last function before switching off the chip
    and it should be followed only by @ref nm_bsp_deinit function call.
    Every function call of @ref m2m_wifi_init should be matched with a call to m2m_wifi_deinit.

@see
	nm_bsp_deinit
    m2m_wifi_init
*/
NMI_API sint8  m2m_wifi_deinit(void * arg);

/*!
@fn	\
    NMI_API sint8 m2m_wifi_init_hold(void);

@brief
    First part of @ref m2m_wifi_init, up to the point of initializing SPI for flash access.

@see
    m2m_wifi_init
    m2m_wifi_init_start
*/
NMI_API sint8 m2m_wifi_init_hold(void);

/*!
@fn	\
    NMI_API sint8 m2m_wifi_init_start(tstrWifiInitParam * pWifiInitParam);

@brief
    Second part of @ref m2m_wifi_init, continuing from where @ref m2m_wifi_init_hold left off.

@param [in] pWifiInitParam
    This is a pointer to a variable of type @ref tstrWifiInitParam which contains pointers to the
    application WIFI layer callback function (see @ref tpfAppWifiCb), monitoring mode callback
    (see @ref tpfAppEthCb) and @ref tstrEthInitParam structure (which contains initialization
    settings for bypass mode).

@see
    m2m_wifi_init
    tstrWifiInitParam
*/
NMI_API sint8 m2m_wifi_init_start(tstrWifiInitParam * pWifiInitParam);

/*!
@fn	\
    NMI_API sint8 m2m_wifi_reinit(tstrWifiInitParam * pWifiInitParam);

@brief
    De-initialize and then initialize wifi. Resets the WINC.

@param [in] pWifiInitParam
    This is a pointer to a variable of type @ref tstrWifiInitParam which contains pointers to the
    application WIFI layer callback function (see @ref tpfAppWifiCb), monitoring mode callback
    (see @ref tpfAppEthCb) and @ref tstrEthInitParam structure (which contains initialization
    settings for bypass mode).

@see
    m2m_wifi_deinit
    m2m_wifi_init
    tstrWifiInitParam
*/
NMI_API sint8 m2m_wifi_reinit(tstrWifiInitParam * pWifiInitParam);

/*!
@fn	\
    NMI_API sint8 m2m_wifi_reinit_hold(void);

@brief
    First part of @ref m2m_wifi_reinit, up to the point of initializing SPI for flash access.

@see
    m2m_wifi_reinit
    m2m_wifi_init_hold
*/
NMI_API sint8 m2m_wifi_reinit_hold(void);

/*!
@fn	\
    NMI_API sint8 m2m_wifi_reinit_start(tstrWifiInitParam * pWifiInitParam);

@brief
    Second part of @ref m2m_wifi_reinit, continuing from where m2m_wifi_reinit_hold left off.

@param [in] pWifiInitParam
                This is a pointer to the @ref tstrWifiInitParam structure which contains pointers to the
    application WIFI layer callback function (see @ref tpfAppWifiCb), monitoring mode callback
    (see @ref tpfAppEthCb) and @ref tstrEthInitParam structure (which contains initialization
    settings for bypass mode).

@see
    m2m_wifi_reinit
    m2m_wifi_init_start
    tstrWifiInitParam
*/
NMI_API sint8 m2m_wifi_reinit_start(tstrWifiInitParam * pWifiInitParam);

/*!
@fn	\
    NMI_API void m2m_wifi_yield(void);

@brief
    Yield from processing more synchronous M2M events.

@details 
    This function causes the synchronous M2M event handler function to yield from processing further
    events and return control to the caller.

@pre
    Prior to receiving  Wi-Fi interrupts, the WINC driver should have been successfully initialized 
	by calling the @ref m2m_wifi_init function.
     
@warning
    Failure to successfully complete this function indicates bus errors and hence a fatal error that will
    prevent the application from proceeding.
*/
NMI_API void m2m_wifi_yield(void);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_handle_events(void * arg);

@brief 	
    Synchronous M2M event handler function.

@details
	This function is responsible for handling interrupts received from the WINC firmware.
	Applications should call this function periodically in-order to receive the events that are to 
	be handled by the callback functions implemented by the application.

	Handle the various events received from the WINC board.
    Whenever an event happens in the WINC board (e.g. Connection, Disconnection, DHCP, etc),
    the WINC will interrupt the host to let it know that a new event has occurred. The host driver
    will attempt to handle these events whenever the application decides to do so by calling
    the m2m_wifi_handle_events function.
    It is mandatory to call this function periodically and independently of any other condition.
    It is ideal to include this function in the main and the most frequent loop of the
		host application.

@pre
    Prior to receiving events, the WINC driver should have been successfully initialized by calling the @ref m2m_wifi_init function.

@warning
    Failure to successfully complete this function indicates bus errors and hence a fatal error that will prevent the application from proceeding.

@return		
    The function returns @ref M2M_SUCCESS for successful interrupt handling and a negative value otherwise.
*/
NMI_API sint8 m2m_wifi_handle_events(void * arg);

/*!
@fn	\
	sint8 m2m_wifi_send_crl(tstrTlsCrlInfo* pCRL);

@brief
	Asynchronous API that notifies the WINC with the Certificate Revocation List.

@param [in]	pCRL
	Pointer to the structure containing certificate revocation list details.

@return
	The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.
*/
sint8 m2m_wifi_send_crl(tstrTlsCrlInfo* pCRL);

/*!
@fn	\
	sint8 m2m_wifi_delete_sc(char *pcSsid, uint8 u8SsidLen);

@brief
	Asynchronous API that deletes connection credentials (PSK, WEP key, 802.1X password) from WINC
	flash. Either deletes all credentials, or for a specific SSID.

@details
	Causes WINC to delete connection credentials. If the parameter is NULL, then WINC will delete
	all credentials from flash. Otherwise WINC will only delete credentials for matching SSID.
    Callback will report the status of the operation (success or not).

@param [in]	pcSsid
	SSID to match on when deleting credentials.
	SSID must not contain '\0'.
	NULL is a valid argument here, in which case all credentials are deleted.

@param [in]	u8SsidLen
	Length of SSID provided in pcSsid. Must be less than @ref M2M_MAX_SSID_LEN.
	This parameter is ignored if pcSsid is NULL.

@pre 
	Prior to deleting credentials, the WINC driver should have been successfully initialized by calling the 
	@ref m2m_wifi_init function.

@warning
    The option to delete for a specific SSID is currently not supported; all credentials are
    deleted regardless of the input parameters.

@return
	The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.
 */
sint8 m2m_wifi_delete_sc(char *pcSsid, uint8 u8SsidLen);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_default_connect(void);

@brief
	Asynchronous API that attempts to reconnect to the last-associated access point.

@details
	Asynchronous Wi-Fi connection function. An application calling this function will cause
	the firmware to correspondingly connect to the last successfully connected AP from the 
    cached connections.\n
 	A failure to connect will result in a response of @ref M2M_WIFI_RESP_DEFAULT_CONNECT 
	indicating the connection error as defined in the structure @ref tstrM2MDefaultConnResp.
	
	Possible errors are: 
	The connection list is empty @ref M2M_DEFAULT_CONN_EMPTY_LIST or a mismatch for the 
	saved AP name @ref M2M_DEFAULT_CONN_SCAN_MISMATCH.

@pre 
    Prior to connecting, the WINC driver should have been successfully initialized by calling the 
	@ref m2m_wifi_init function.

@warning
 This function must be called in station mode only.
	It is important to note that successful completion of a call to m2m_wifi_default_connect()
	  does not guarantee success of the WIFI connection; a negative return value indicates only
	  locally-detected errors.
	
@return		
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.
*/
NMI_API sint8 m2m_wifi_default_connect(void);

/*!
@fn	\
    sint8 m2m_wifi_connect_open(tenuCredStoreOption enuCredStoreOption, tstrNetworkId *pstrNetworkId);

@brief
    Asynchronous API to connect to an access point using open authentication.

@details
    Asynchronous Wi-Fi connection function. An application calling this function will cause the
    firmware to attempt to connect to an access point matching the details in pstrNetworkId, with
    open authentication.
    On successful connection, the connection details may be saved in WINC's flash, according to
    the option selected in enuCredStoreOption.
    Once connection has been attempted (whether successful or otherwise), a response event
    @ref M2M_WIFI_RESP_CON_STATE_CHANGED will be sent to the callback function @ref tpfAppWifiCb
    provided during initialization @ref m2m_wifi_init.

    Possible results indicated by the response event are:
    - @ref M2M_WIFI_DISCONNECTED if the connection attempt failed.
    - @ref M2M_WIFI_CONNECTED if the connection attempt succeeded.

@pre
    Prior to attempting connection, the WINC driver must have been initialized by calling the
    @ref m2m_wifi_init function.

@warning
    This function is handled in station mode only.

@param[in]  enuCredStoreOption
    Option to specify whether connection details (i.e. the contents
    of pstrNetworkId) are stored in WINC's flash and, if so,
                                    whether they are encrypted before storing.

@param[in]  pstrNetworkId
    Structure specifying SSID/BSSID and Wi-Fi channel.

@return
    The function returns @ref M2M_SUCCESS if the connect request has been successfully passed to
    the firmware, and a negative value otherwise.
 */
sint8 m2m_wifi_connect_open(tenuCredStoreOption enuCredStoreOption, tstrNetworkId *pstrNetworkId);

/*!
@fn	\
    sint8 m2m_wifi_connect_wep(tenuCredStoreOption enuCredStoreOption, tstrNetworkId *pstrNetworkId, tstrAuthWep *pstrAuthWep);

@brief
    Asynchronous API to connect to an access point using WEP authentication.
				
@details
    Asynchronous Wi-Fi connection function. An application calling this function will cause the
    firmware to attempt to connect to an access point matching the details in pstrNetworkId, with
    the WEP key provided in pstrAuthWep.
    On successful connection, the connection details may be saved in WINC's flash, according to
    the option selected in enuCredStoreOption.
    Once connection has been attempted (whether successful or otherwise), a response event
    @ref M2M_WIFI_RESP_CON_STATE_CHANGED will be sent to the callback function @ref tpfAppWifiCb
    provided during initialization @ref m2m_wifi_init.
	
    Possible results indicated by the response event are:
    - @ref M2M_WIFI_DISCONNECTED if the connection attempt failed.
    - @ref M2M_WIFI_CONNECTED if the connection attempt succeeded.

@pre
    Prior to attempting connection, the WINC driver must have been initialized by calling the
    @ref m2m_wifi_init function.

@warning
    This function is handled in station mode only.

@param[in]  enuCredStoreOption
    Option to specify whether connection details (i.e. the contents
    of pstrNetworkId and pstrAuthWep) are stored in WINC's flash
                                    and, if so, whether they are encrypted before storing.

@param[in]  pstrNetworkId
    Structure specifying SSID/BSSID and Wi-Fi channel.

@param[in]  pstrAuthWep
    Structure specifying the WEP key.
	
@return
    The function returns @ref M2M_SUCCESS if the connect request has been successfully passed to
    the firmware, and a negative value otherwise.
*/
sint8 m2m_wifi_connect_wep(tenuCredStoreOption enuCredStoreOption, tstrNetworkId *pstrNetworkId, tstrAuthWep *pstrAuthWep);
	
/*!
@fn	\
    sint8 m2m_wifi_connect_psk(tenuCredStoreOption enuCredStoreOption, tstrNetworkId *pstrNetworkId, tstrAuthPsk *pstrAuthPsk);

@brief
    Asynchronous API to connect to an access point using WPA(2) PSK authentication.

@details
    Asynchronous Wi-Fi connection function. An application calling this function will cause the
    firmware to attempt to connect to an access point matching the details in pstrNetworkId, with
    the PSK passphrase provided in pstrAuthPsk.
    On successful connection, the connection details may be saved in WINC's flash, according to
    the option selected in enuCredStoreOption.
    Once connection has been attempted (whether successful or otherwise), a response event
    @ref M2M_WIFI_RESP_CON_STATE_CHANGED will be sent to the callback function @ref tpfAppWifiCb
    provided during initialization @ref m2m_wifi_init.

    Possible results indicated by the response event are:
    - @ref M2M_WIFI_DISCONNECTED if the connection attempt failed.
    - @ref M2M_WIFI_CONNECTED if the connection attempt succeeded.

@pre
    Prior to attempting connection, the WINC driver must have been initialized by calling the
    @ref m2m_wifi_init function.

@warning
    This function is handled in station mode only.

@param[in]  enuCredStoreOption
    Option to specify whether connection details (i.e. the contents
    of pstrNetworkId and pstrAuthPsk) are stored in WINC's flash
                                    and, if so, whether they are encrypted before storing.

@param[in]  pstrNetworkId
    Structure specifying SSID/BSSID and Wi-Fi channel.

@param[in]  pstrAuthPsk
    Structure specifying the Passphrase/PSK.

@return
    The function returns @ref M2M_SUCCESS if the connect request has been successfully passed to
    the firmware, and a negative value otherwise.
 */
sint8 m2m_wifi_connect_psk(tenuCredStoreOption enuCredStoreOption, tstrNetworkId *pstrNetworkId, tstrAuthPsk *pstrAuthPsk);

/*!
@fn	\
    sint8 m2m_wifi_connect_1x_mschap2(tenuCredStoreOption enuCredStoreOption, tstrNetworkId *pstrNetworkId, tstrAuth1xMschap2 *pstrAuth1xMschap2);

@brief
    Asynchronous API to connect to an access point using WPA(2) Enterprise authentication with
    MS-CHAP-V2 credentials.
				
@details
    Asynchronous Wi-Fi connection function. An application calling this function will cause the
    firmware to attempt to connect to an access point matching the details in pstrNetworkId, with
    the Enterprise MS-CHAP-V2 credentials provided in pstrAuth1xMschap2.
    On successful connection, the connection details may be saved in WINC's flash, according to
    the option selected in enuCredStoreOption.
    Once connection has been attempted (whether successful or otherwise), a response event
    @ref M2M_WIFI_RESP_CON_STATE_CHANGED will be sent to the callback function tpfAppWifiCb
    provided during initialization @ref m2m_wifi_init.
	
    Possible results indicated by the response event are:
    - @ref M2M_WIFI_DISCONNECTED if the connection attempt failed.
    - @ref M2M_WIFI_CONNECTED if the connection attempt succeeded.

@pre
    Prior to attempting connection, the WINC driver must have been initialized by calling the
    @ref m2m_wifi_init function.

@warning
    This function is handled in station mode only.

@param[in]  enuCredStoreOption
    Option to specify whether connection details (i.e. the contents
    of pstrNetworkId and pstrAuth1xMschap2) are stored in WINC's
                                    flash and, if so, whether they are encrypted before storing.

@param[in]  pstrNetworkId
    Structure specifying SSID/BSSID and Wi-Fi channel.

@param[in]  pstrAuth1xMschap2
    Structure specifying the MS-CHAP-V2 credentials.
	
@return
    The function returns @ref M2M_SUCCESS if the connect request has been successfully passed to
    the firmware, and a negative value otherwise.
*/
sint8 m2m_wifi_connect_1x_mschap2(tenuCredStoreOption enuCredStoreOption, tstrNetworkId *pstrNetworkId, tstrAuth1xMschap2 *pstrAuth1xMschap2);
	
/*!
@fn	\
    sint8 m2m_wifi_connect_1x_tls(tenuCredStoreOption enuCredStoreOption, tstrNetworkId *pstrNetworkId, tstrAuth1xTls *pstrAuth1xTls);

@brief
    Asynchronous API to connect to an access point using WPA(2) Enterprise authentication with
    MS-CHAP-V2 credentials.

@details
    Asynchronous Wi-Fi connection function. An application calling this function will cause the
    firmware to attempt to connect to an access point matching the details in pstrNetworkId, with
    the Enterprise TLS credentials provided in pstrAuth1xTls.
    On successful connection, the connection details may be saved in WINC's flash, according to
    the option selected in enuCredStoreOption.
    Once connection has been attempted (whether successful or otherwise), a response event
    @ref M2M_WIFI_RESP_CON_STATE_CHANGED will be sent to the callback function @ref tpfAppWifiCb
    provided during initialization @ref m2m_wifi_init.

    Possible results indicated by the response event are:
    - @ref M2M_WIFI_DISCONNECTED if the connection attempt failed.
    - @ref M2M_WIFI_CONNECTED if the connection attempt succeeded.

@pre
    Prior to attempting connection, the WINC driver must have been initialized by calling the
    @ref m2m_wifi_init function.

@warning
    This function is handled in station mode only.

@param[in]  enuCredStoreOption
    Option to specify whether connection details (i.e. the contents
    of pstrNetworkId and pstrAuth1xTls) are stored in WINC's
                                    flash and, if so, whether they are encrypted before storing.

@param[in]  pstrNetworkId
    Structure specifying SSID/BSSID and Wi-Fi channel.

@param[in]  pstrAuth1xTls
    Structure specifying the EAP-TLS credentials.

@return
    The function returns @ref M2M_SUCCESS if the connect request has been successfully passed to
    the firmware, and a negative value otherwise.
 */
sint8 m2m_wifi_connect_1x_tls(tenuCredStoreOption enuCredStoreOption, tstrNetworkId *pstrNetworkId, tstrAuth1xTls *pstrAuth1xTls);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_connect(char *pcSsid, uint8 u8SsidLen, uint8 u8SecType, void *pvAuthInfo, uint16 u16Ch);

@brief
    DEPRECATED in v19.6.1 - Kept only for legacy purposes.\n
    Legacy asynchronous API to request connection to a specified access point.

@details
    Prior to a successful connection, the application must define the SSID of the AP, the security
    type, the authentication information parameters and the channel number to which the connection
    will be established.

    The connection status is known when a response of @ref M2M_WIFI_RESP_CON_STATE_CHANGED is 
    received based on the states defined in @ref tenuM2mConnState, successful connection is defined
    by @ref M2M_WIFI_CONNECTED

    The only difference between this function and @ref m2m_wifi_default_connect, is the set of connection parameters.
    Connection using this function is expected to be made to a specific AP and to a specified channel.

@param [in]	pcSsid
    A buffer holding the SSID corresponding to the requested AP.
    SSID must not contain '\0'.
				
@param [in]	u8SsidLen
    Length of the given SSID (not including the NULL termination).
    A length greater than the maximum defined SSID @ref M2M_MAX_SSID_LEN will result in a negative error 
				@ref M2M_ERR_FAIL.
				
@param [in]	u8SecType
				Wi-Fi security type security for the network. It can be one of the following types:
				-@ref M2M_WIFI_SEC_OPEN
				-@ref M2M_WIFI_SEC_WEP
				-@ref M2M_WIFI_SEC_WPA_PSK
				-@ref M2M_WIFI_SEC_802_1X
				A value outside these possible values will result in a negative return error @ref M2M_ERR_FAIL.

@param [in]	pvAuthInfo
    Authentication parameters required for completing the connection. Its type is based on the
    security type. If the authentication parameters are NULL or are greater than the maximum length
    of the authentication parameters length as defined by @ref M2M_MAX_PSK_LEN a negative error will
    return @ref M2M_ERR_FAIL(-12) indicating connection failure.

@param [in]	u16Ch
    Wi-Fi channel number as defined in @ref tenuM2mScanCh enumeration. Specifying a channel number
    greater than @ref M2M_WIFI_CH_14 results in a negative error @ref M2M_ERR_FAIL(-12), unless
    the value is @ref M2M_WIFI_CH_ALL, since this indicates that the firmware should scan all channels
    to find the SSID requested to connect to.

    Failure to find the connection match will return a negative error
    @ref M2M_DEFAULT_CONN_SCAN_MISMATCH.

@pre
        Prior to a successful connection request, the wi-fi driver must have been successfully initialized
        through the call of the @ref m2m_wifi_init function

@warning
    If there is a '\0' character within the first u8SsidLen characters, then this function will assume
    that the input u8SsidLen was incorrect, set length to strlen(pcSsid) and continue.\n
    This function has been deprecated since v19.6.1 and will no longer be supported afterwards.
    The following should be used instead:
    @ref m2m_wifi_connect_open
    @ref m2m_wifi_connect_wep
    @ref m2m_wifi_connect_psk
    @ref m2m_wifi_connect_1x_mschap2
    @ref m2m_wifi_connect_1x_tls

    Additionally:
    -This function must be called in station mode only.
    -Successful completion of this function does not guarantee success of the WIFI connection, and
     a negative return value indicates only locally-detected errors.

@return	
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
    and a negative value otherwise.	

@see
    tstr1xAuthCredentials
    tstrM2mWifiWepParams
*/
NMI_API sint8 m2m_wifi_connect(char *pcSsid, uint8 u8SsidLen, uint8 u8SecType, void *pvAuthInfo, uint16 u16Ch);

/*!
@fn \
    NMI_API sint8 m2m_wifi_connect_sc(char *pcSsid, uint8 u8SsidLen, uint8 u8SecType, void *pvAuthInfo, uint16 u16Ch, uint8 u8NoSaveCred);

@brief
    DEPRECATED in v19.6.1 - Kept only for legacy purposes.\n
    Legacy asynchronous API to request connection to a specific AP with the option to save credentials in Flash.

@details
	Prior to a successful connection, the application developers must know the SSID of the AP, the security
	type, the authentication information parameters and the channel number to which the connection will
	be established.

    The connection status is known when a response of @ref M2M_WIFI_RESP_CON_STATE_CHANGED is received based
    on the states defined in @ref tenuM2mConnState, successful connection is defined by @ref M2M_WIFI_CONNECTED
    The only difference between this function and @ref m2m_wifi_connect, is the option to save the access point
    info (SSID, password...etc) or not.
    Connection using this function is expected to be made to a specific AP and to a specified channel.

@param [in]	pcSsid
    A buffer holding the SSID corresponding to the requested AP.
    SSID must not contain '\0'.

@param [in]	u8SsidLen
    Length of the given SSID (not including the NULL termination).
    A length greater than the maximum defined SSID @ref M2M_MAX_SSID_LEN will result in a negative error
    @ref M2M_ERR_FAIL.

@param [in]	u8SecType
    Wi-Fi security type security for the network see @ref tenuM2mSecType. It can be one of the following types:
    -@ref M2M_WIFI_SEC_OPEN
    -@ref M2M_WIFI_SEC_WEP
    -@ref M2M_WIFI_SEC_WPA_PSK
    -@ref M2M_WIFI_SEC_802_1X
    A value outside these possible values will result in a negative return error @ref M2M_ERR_FAIL.

@param [in]	pvAuthInfo
    Authentication parameters required for completing the connection. Its type is based on the
    security type. If the authentication parameters are NULL or are greater than the maximum length
    of the authentication parameters length as defined by @ref M2M_MAX_PSK_LEN a negative error will
    return @ref M2M_ERR_FAIL(-12) indicating connection failure.

@param [in]	u16Ch
    Wi-Fi channel number as defined in @ref tenuM2mScanCh enumeration. Specification of a channel
    number greater than @ref M2M_WIFI_CH_14 returns a negative error @ref M2M_ERR_FAIL(-12) unless
    the value is @ref M2M_WIFI_CH_ALL. A channel number of @ref M2M_WIFI_CH_ALL indicates that the
    firmware should scan all channels to find the SSID specified in parameter pcSsid.

    Failure to find the connection match will return a negative error
    @ref M2M_DEFAULT_CONN_SCAN_MISMATCH.

@param [in] u8NoSaveCred
    Option to store the access point SSID and password into the WINC flash memory or not.

@pre
    Prior to a successful connection request, the wi-fi driver must have been successfully initialized through the call of the @ref m2m_wifi_init function.
	
@warning
    If there is a '\0' character within the first u8SsidLen characters, then this function will assume
    that the input u8SsidLen was incorrect, set length to strlen(pcSsid) and continue.\n
    This function has been deprecated since v19.6.1 and will no longer be supported afterwards.
    The following should be used instead:
    @ref m2m_wifi_connect_open
    @ref m2m_wifi_connect_wep
    @ref m2m_wifi_connect_psk
    @ref m2m_wifi_connect_1x_mschap2
    @ref m2m_wifi_connect_1x_tls

    Additionally:
	-This function must be called in station mode only.
	-Successful completion of this function does not guarantee success of the WIFI connection, and
	a negative return value indicates only locally-detected errors.
	
@see
    tenuM2mSecType
    tstr1xAuthCredentials
    tstrM2mWifiWepParams

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC,
    and a negative value otherwise.
*/
NMI_API sint8 m2m_wifi_connect_sc(char *pcSsid, uint8 u8SsidLen, uint8 u8SecType, void *pvAuthInfo, uint16 u16Ch, uint8 u8NoSaveCred);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_disconnect(void);
	
@brief
    Synchronous API to request disconnection from a network.

@details
    Request a Wi-Fi disconnect from the currently connected AP.
    After the Disconnect is complete the driver should receive a response of @ref M2M_WIFI_RESP_CON_STATE_CHANGED based on the states defined
    in @ref tenuM2mConnState, successful disconnection is defined by @ref M2M_WIFI_DISCONNECTED .
	
@return		
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@pre 
	Disconnection request must be made to a successfully connected AP. If the WINC is not in the connected state, a call to this function will hold insignificant.

@warning
	This function must be called in station mode only.
	
@see
    m2m_wifi_connect_open
	m2m_wifi_connect_wep
	m2m_wifi_connect_psk
	m2m_wifi_connect_1x_mschap2
	m2m_wifi_connect_1x_tls
	m2m_wifi_default_connect
*/
NMI_API sint8 m2m_wifi_disconnect(void);
 
/*!
@fn	\
	NMI_API sint8 m2m_wifi_start_provision_mode(tstrM2MAPConfig *pstrAPConfig, char *pcHttpServerDomainName, uint8 bEnableHttpRedirect);
	
@brief
    Asynchronous API for control of Wi-Fi provisioning functionality.

@details	
	Asynchronous Wi-Fi provisioning function, which starts the WINC HTTP PROVISIONING mode.
	The function triggers the WINC to activate the Wi-Fi AP (HOTSPOT) mode with the passed 
	configuration parameters and then starts the HTTP Provision WEB Server. 

	Provisioning status is returned in an event @ref M2M_WIFI_RESP_PROVISION_INFO.

@param [in]	pstrAPConfig
				AP configuration parameters as defined in @ref tstrM2MAPConfig configuration structure.
    If a NULL value is passed in, the call will result in a negative error @ref M2M_ERR_FAIL.
				
@param [in]	pcHttpServerDomainName
				Domain name of the HTTP Provision WEB server which others will use to load the provisioning Home page.
				The domain name can have one of the following 3 forms:
    - 1. "wincprov.com"
    - 2. "http://wincprov.com"
    - 3. "https://wincprov.com"

    Forms 1 and 2 are equivalent, they will both start a plain http server, while form 3
				will start a secure HTTP provisioning Session (HTTP over SSL connection).

@param [in]	bEnableHttpRedirect
				A flag to enable/disable the HTTP redirect feature. If Secure provisioning is enabled (i.e. the server
				domain name uses "https" prefix) this flag is ignored (no meaning for redirect in HTTPS).
				Possible values are:
    - ZERO value, which means DO NOT use HTTP Redirect. In this case, the associated device could open the provisioning
                  page ONLY when the HTTP Provision URL of the WINC HTTP Server is correctly written on the browser.
    - Non-Zero value, means use HTTP Redirect. In this case, all http traffic (http://URL) from the associated
                  device (Phone, PC, etc) will be redirected to the WINC HTTP Provisioning Home page.

@pre	
	- A Wi-Fi notification callback of type @ref tpfAppWifiCb MUST be implemented and registered at startup. Registering the callback
	  is done through passing it to the initialization @ref m2m_wifi_init function.
	- The event @ref M2M_WIFI_RESP_CONN_INFO must be handled in the callback to receive the requested connection info.
	
@warning
    DO NOT use ".local" in the pcHttpServerDomainName.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.
	
@see
	tpfAppWifiCb
	m2m_wifi_init
	M2M_WIFI_RESP_PROVISION_INFO
	m2m_wifi_stop_provision_mode
	tstrM2MAPConfig

\section WIFIExample1 Example
  The example demonstrates a code snippet for how provisioning is triggered and the response event 
  received accordingly. 

@code
	#include "m2m_wifi.h"
	#include "m2m_types.h"

	void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
	{
		switch(u8WiFiEvent)
		{
		case M2M_WIFI_RESP_PROVISION_INFO:
			{
				tstrM2MProvisionInfo	*pstrProvInfo = (tstrM2MProvisionInfo*)pvMsg;
				if(pstrProvInfo->u8Status == M2M_SUCCESS)
				{
                    tstrNetworkId   strNetworkId = {NULL, pstrProvInfo->au8SSID, (uint8)strlen((char*)(pstrProvInfo->au8SSID)), M2M_WIFI_CH_ALL};
                    tstrAuthPsk     strAuthPsk = {NULL, pstrProvInfo->au8Password, (uint8)strlen((char*)(pstrProvInfo->au8Password))};
                    m2m_wifi_connect_psk(WIFI_CRED_SAVE_ENCRYPTED, &strNetworkId, &strAuthPsk);

					printf("PROV SSID : %s\n",pstrProvInfo->au8SSID);
					printf("PROV PSK  : %s\n",pstrProvInfo->au8Password);
				}
				else
				{
					printf("(ERR) Provisioning Failed\n");
				}
			}
			break;

			default:
			break;
		}
	}

	int main()
	{
		tstrWifiInitParam 	param;
		
		param.pfAppWifiCb	= wifi_event_cb;
		if(!m2m_wifi_init(&param))
		{
			tstrM2MAPConfig		apConfig;
			uint8				bEnableRedirect = 1;
			
			strcpy(apConfig.au8SSID, "WINC_SSID");
			apConfig.u8ListenChannel 	= 1;
			apConfig.u8SecType			= M2M_WIFI_SEC_OPEN;
			apConfig.u8SsidHide			= 0;
			
			// IP Address
			apConfig.au8DHCPServerIP[0]	= 192;
			apConfig.au8DHCPServerIP[1]	= 168;
			apConfig.au8DHCPServerIP[2]	= 1;
            apConfig.au8DHCPServerIP[3]	= 1;

			m2m_wifi_start_provision_mode(&apConfig, "atmelwincconf.com", bEnableRedirect);
						
			while(1)
			{
				m2m_wifi_handle_events(NULL);
			}
		}
	}
@endcode
*/
NMI_API sint8 m2m_wifi_start_provision_mode(tstrM2MAPConfig *pstrAPConfig, char *pcHttpServerDomainName, uint8 bEnableHttpRedirect);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_start_provision_mode_ext(tstrM2MAPModeConfig *pstrAPModeConfig, char *pcHttpServerDomainName, uint8 bEnableHttpRedirect);
	
@brief
    Asynchronous API for control of Wi-Fi provisioning functionality with extended options.

@details	
	Asynchronous Wi-Fi provisioning function, which starts the WINC HTTP PROVISIONING mode.
	The function triggers the WINC to activate the Wi-Fi AP (HOTSPOT) mode with the passed 
	configuration parameters and then starts the HTTP Provision WEB Server. 

	Provisioning status is returned in an event @ref M2M_WIFI_RESP_PROVISION_INFO.

@param [in]	pstrAPModeConfig
    AP configuration parameters as defined in @ref tstrM2MAPModeConfig configuration structure.
    A NULL value passed in, will result in a negative error @ref M2M_ERR_FAIL.

@param [in]	pcHttpServerDomainName
    Domain name of the HTTP Provision WEB server which others will use to load the provisioning Home page.
    The domain name can have one of the following 3 forms:
    - 1. "wincprov.com"
    - 2. "http://wincprov.com"
    - 3. "https://wincprov.com"

    The forms 1 and 2 are equivalent, they both will start a plain http server, while form 3
    will start a secure HTTP provisioning Session (HTTP over SSL connection).

@param [in]	bEnableHttpRedirect
    A flag to enable/disable the HTTP redirect feature. If Secure provisioning is enabled (i.e. the server
    domain name uses "https" prefix) this flag is ignored (no meaning for redirect in HTTPS).
    Possible values are:
    - ZERO value, which means DO NOT use HTTP Redirect. In this case, the associated device could open the provisioning
                  page ONLY when the HTTP Provision URL of the WINC HTTP Server is correctly written on the browser.
    - Non-Zero value, means use HTTP Redirect. In this case, all http traffic (http://URL) from the associated
                  device (Phone, PC, etc) will be redirected to the WINC HTTP Provisioning Home page.

@pre	
	- A Wi-Fi notification callback of type @ref tpfAppWifiCb MUST be implemented and registered at startup. Registering the callback
	  is done through passing it to the initialization @ref m2m_wifi_init function.
	- The event @ref M2M_WIFI_RESP_CONN_INFO must be handled in the callback to receive the requested connection info.

@warning
    DO Not use ".local" in the pcHttpServerDomainName.
	
@see
    tpfAppWifiCb
    m2m_wifi_init
    M2M_WIFI_RESP_PROVISION_INFO
    m2m_wifi_stop_provision_mode
    tstrM2MAPModeConfig

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC,
    and a negative value otherwise.

\section WIFIExample1b Example
  The example demonstrates a code snippet for how provisioning is triggered and the response event 
  received accordingly. 

@code
    #include "m2m_wifi.h"
    #include "m2m_types.h"

    void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
    {
        switch(u8WiFiEvent)
        {
        case M2M_WIFI_RESP_PROVISION_INFO:
            {
                tstrM2MProvisionInfo	*pstrProvInfo = (tstrM2MProvisionInfo*)pvMsg;
                if(pstrProvInfo->u8Status == M2M_SUCCESS)
                {
                    tstrNetworkId   strNetworkId = {NULL, pstrProvInfo->au8SSID, (uint8)strlen((char*)(pstrProvInfo->au8SSID)), M2M_WIFI_CH_ALL};
                    tstrAuthPsk     strAuthPsk = {NULL, pstrProvInfo->au8Password, (uint8)strlen((char*)(pstrProvInfo->au8Password))};
                    m2m_wifi_connect_psk(WIFI_CRED_SAVE_ENCRYPTED, &strNetworkId, &strAuthPsk);

                    printf("PROV SSID : %s\n",pstrProvInfo->au8SSID);
                    printf("PROV PSK  : %s\n",pstrProvInfo->au8Password);
                }
                else
                {
                    printf("(ERR) Provisioning Failed\n");
                }
            }
            break;

            default:
            break;
        }
    }

    int main()
    {
        tstrWifiInitParam 	param;
        
        param.pfAppWifiCb	= wifi_event_cb;
        if(!m2m_wifi_init(&param))
        {
            tstrM2MAPModeConfig	apModeConfig;
            uint8				bEnableRedirect = 1;
            
            strcpy(apModeConfig.strApConfig.au8SSID, "WINC_SSID");
            apModeConfig.strApConfig.u8ListenChannel 	= 1;
            apModeConfig.strApConfig.u8SecType			= M2M_WIFI_SEC_OPEN;
            apModeConfig.strApConfig.u8SsidHide			= 0;
            
            // IP Address
            apModeConfig.strApConfig.au8DHCPServerIP[0]	= 192;
            apModeConfig.strApConfig.au8DHCPServerIP[1]	= 168;
            apModeConfig.strApConfig.au8DHCPServerIP[2]	= 1;
            apModeConfig.strApConfig.au8DHCPServerIP[3]	= 1;

            // Default router IP
			m2m_memcpy(apModeConfig.strApConfigExt.au8DefRouterIP, apModeConfig.strApConfig.au8DHCPServerIP, 4);

            // DNS Server IP
			m2m_memcpy(apModeConfig.strApConfigExt.au8DNSServerIP, apModeConfig.strApConfig.au8DHCPServerIP, 4);
			
			// Subnet mask
            apModeConfig.strApConfigExt.au8SubnetMask[0]    = 255;
            apModeConfig.strApConfigExt.au8SubnetMask[1]    = 255;
            apModeConfig.strApConfigExt.au8SubnetMask[2]    = 255;
            apModeConfig.strApConfigExt.au8SubnetMask[3]    = 0;

            m2m_wifi_start_provision_mode_ext(&apModeConfig, "atmelwincconf.com", bEnableRedirect);
                        
            while(1)
            {
                m2m_wifi_handle_events(NULL);
            }
        }
    }
@endcode
*/
NMI_API sint8 m2m_wifi_start_provision_mode_ext(tstrM2MAPModeConfig *pstrAPModeConfig, char *pcHttpServerDomainName, uint8 bEnableHttpRedirect);

/*!
@fn	\
	sint8 m2m_wifi_stop_provision_mode(void);

@brief
    Synchronous API for terminating provisioning mode on the WINC IC.

@details	
    This function will terminate any currently active provisioning mode on the WINC IC, returning the 
	IC to idle.
	
@pre
	An active provisioning session must be active before it is terminated through this function.
	
@return
	The function returns ZERO for success and a negative value otherwise.

@see
    m2m_wifi_start_provision_mode
*/
NMI_API sint8 m2m_wifi_stop_provision_mode(void);

/*!
@fn	\
	sint8 m2m_wifi_get_connection_info(void);

@brief
    Asynchronous API for retrieving the WINC IC's connection status.

@details
	Asynchronous connection status retrieval function, retrieves the status information of the 
	currently connected AP. 
	The result is passed to the Wi-Fi notification callback through the event 
	@ref M2M_WIFI_RESP_CONN_INFO. Connection information is retrieved from 
	the structure @ref tstrM2MConnInfo.
	
	Connection Information retrieved:
	-Connection Security
	-Connection RSSI
	-Remote MAC address
	-Remote IP address

    In case the WINC is operating in station mode, the SSID of the AP is also retrieved.

@pre	
	- A Wi-Fi notification callback of type @ref tpfAppWifiCb MUST be implemented and registered at 
	startup. Registering the callback is done through passing it to the initialization 
	@ref m2m_wifi_init function.
	- The event @ref M2M_WIFI_RESP_CONN_INFO must be handled in the callback to receive the 
	requested connection info.
	
@warning
    - In case the WINC is operating in AP mode, the SSID field will be returned as a NULL string.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.
	
@see
	M2M_WIFI_RESP_CONN_INFO,
	tstrM2MConnInfo
\section WIFIExample2 Example
  The code snippet shows an example of how wi-fi connection information is retrieved .
@code
	#include "m2m_wifi.h"
	#include "m2m_types.h"

	void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
	{
		switch(u8WiFiEvent)
		{
		case M2M_WIFI_RESP_CONN_INFO:
			{
				tstrM2MConnInfo		*pstrConnInfo = (tstrM2MConnInfo*)pvMsg;
				
				printf("CONNECTED AP INFO\n");
				printf("SSID     			: %s\n",pstrConnInfo->acSSID);
				printf("SEC TYPE 			: %d\n",pstrConnInfo->u8SecType);
				printf("Signal Strength		: %d\n", pstrConnInfo->s8RSSI); 
				printf("Local IP Address	: %d.%d.%d.%d\n", 
					pstrConnInfo->au8IPAddr[0] , pstrConnInfo->au8IPAddr[1], pstrConnInfo->au8IPAddr[2], pstrConnInfo->au8IPAddr[3]);
			}
			break;

		case M2M_WIFI_REQ_DHCP_CONF:
			{
				// Get the current AP information.
				m2m_wifi_get_connection_info();
			}
			break;
		default:
			break;
		}
	}

	int main()
	{
		tstrWifiInitParam 	param;
		
		param.pfAppWifiCb	= wifi_event_cb;
		if(!m2m_wifi_init(&param))
		{
			// connect to the default AP
			m2m_wifi_default_connect();
						
			while(1)
			{
				m2m_wifi_handle_events(NULL);
			}
		}
	}
@endcode
*/
NMI_API sint8 m2m_wifi_get_connection_info(void);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_set_mac_address(uint8 au8MacAddress[6]);

@brief
    Synchronous API for assigning a MAC address to the WINC IC.

@details
		This function override the already assigned MAC address of the WINC board with a user provided one. This is for experimental 
		use only and should never be used in the production SW.

@param [in]	au8MacAddress
    MAC Address to be provisioned to the WINC.

@return		
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.
*/
NMI_API sint8 m2m_wifi_set_mac_address(uint8 au8MacAddress[6]);
 
/*!
@fn	\
	NMI_API sint8 m2m_wifi_wps(uint8 u8TriggerType,const char * pcPinNumber);

@brief
    Asynchronous API to engage the WINC IC's Wi-Fi Protected Setup (enrollee) function.

@details
	This function is called for the WINC to enter the WPS (Wi-Fi Protected Setup) mode. The result 
	is passed to the Wi-Fi notification callback with the event @ref M2M_WIFI_REQ_WPS.

@param [in]	u8TriggerType
    WPS Trigger method. This may be:
				- [WPS_PIN_TRIGGER](@ref WPS_PIN_TRIGGER)   Push button method
				- [WPS_PBC_TRIGGER](@ref WPS_PBC_TRIGGER)	Pin method
				
@param [in]	pcPinNumber
				PIN number for WPS PIN method. It is not used if the trigger type is WPS_PBC_TRIGGER. It must follow the rules
				stated by the WPS standard.

@pre	
	- A Wi-Fi notification callback of type (@ref tpfAppWifiCb MUST be implemented and registered at 
    startup. Registering the callback is done through passing it to @ref m2m_wifi_init.
    - The event @ref M2M_WIFI_REQ_WPS must be handled in the callback to receive
	  the WPS status.
    - The WINC device MUST be in IDLE or STA mode. If AP mode is active, the WPS will not be
	  performed. 
    - The @ref m2m_wifi_handle_events MUST be called periodically to receive
	  the responses in the callback.

@warning
    This function is not allowed in AP mode.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC,
    and a negative value otherwise.

@see
	tpfAppWifiCb
	m2m_wifi_init
	M2M_WIFI_REQ_WPS
	tenuWPSTrigger
	tstrM2MWPSInfo

\section WIFIExample3 Example
  The code snippet shows an example of how wi-fi WPS is triggered .
@code
	#include "m2m_wifi.h"
	#include "m2m_types.h"

	void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
	{
		switch(u8WiFiEvent)
		{
		case M2M_WIFI_REQ_WPS:
			{
				tstrM2MWPSInfo	*pstrWPS = (tstrM2MWPSInfo*)pvMsg;
				if(pstrWPS->u8AuthType != 0)
				{
                    // establish Wi-Fi connection
                    tstrNetworkId   strNetworkId = {NULL, pstrWPS->au8SSID, (uint8)strlen((char*)(pstrWPS->au8SSID)), pstrWPS->u8Ch};
                    if(pstrWPS->u8AuthType == M2M_WIFI_SEC_OPEN)
                    {
                        m2m_wifi_connect_open(WIFI_CRED_SAVE_ENCRYPTED, &strNetworkId);
                    }
                    else
                    {
                        tstrAuthPsk     strAuthPsk = {NULL, pstrWPS->au8PSK, (uint8)strlen((char*)(pstrWPS->au8PSK))};
                        m2m_wifi_connect_psk(WIFI_CRED_SAVE_ENCRYPTED, &strNetworkId, &strAuthPsk);
                    }

					printf("WPS SSID           : %s\n",pstrWPS->au8SSID);
					printf("WPS PSK            : %s\n",pstrWPS->au8PSK);
					printf("WPS SSID Auth Type : %s\n",pstrWPS->u8AuthType == M2M_WIFI_SEC_OPEN ? "OPEN" : "WPA/WPA2");
                    printf("WPS Channel        : %d\n",pstrWPS->u8Ch);
				}
				else
				{
					printf("(ERR) WPS Is not enabled OR Timed out\n");
				}
			}
			break;
			
		default:
			break;
		}
	}

	int main()
	{
		tstrWifiInitParam 	param;
		
		param.pfAppWifiCb	= wifi_event_cb;
		if(!m2m_wifi_init(&param))
		{
			// Trigger WPS in Push button mode.
			m2m_wifi_wps(WPS_PBC_TRIGGER, NULL);
			
			while(1)
			{
				m2m_wifi_handle_events(NULL);
			}
		}
	}
@endcode
*/
NMI_API sint8 m2m_wifi_wps(uint8 u8TriggerType,const char  *pcPinNumber);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_wps_disable(void);

@brief	
    Stops the ongoing WPS session.

@pre	
    WINC should be already in WPS mode using @ref m2m_wifi_wps.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see 
	m2m_wifi_wps
*/
NMI_API sint8 m2m_wifi_wps_disable(void);
/**@}*/
 
/**@cond P2P_DOC
 * @addtogroup WLANAPI
 */
/**@{*/
/*!
@fn	\
	NMI_API sint8 m2m_wifi_p2p(uint8 u8Channel);

@warning
    P2P has been deprecated from version 19.5.3!
*/
NMI_API sint8 m2m_wifi_p2p(uint8 u8Channel);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_p2p_disconnect(void);

@warning
    P2P has been deprecated from version 19.5.3!
*/
NMI_API sint8 m2m_wifi_p2p_disconnect(void);
/**@}*/
/**@endcond*/

/**@addtogroup WLANAPI
 */
/**@{*/
/*!
@fn	\
	NMI_API sint8 m2m_wifi_enable_ap(CONST tstrM2MAPConfig* pstrM2MAPConfig);

@brief
    Asynchronous API to enable access point (AKA "hot-spot") mode on the WINC IC.

@details
    The WINC IC supports the ability to operate as an access point with the following limitations:
      - Only 1 station may be associated at any given time.
	  - open system and WEP are the only security suites supported.

@param [in]	pstrM2MAPConfig
				A structure holding the AP configurations.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@pre
	- A Wi-Fi notification callback of type @ref tpfAppWifiCb MUST be implemented and registered at initialization. Registering the callback
    is done through passing it to the @ref m2m_wifi_init.
	- The event @ref M2M_WIFI_REQ_DHCP_CONF must be handled in the callback.
	- The @ref m2m_wifi_handle_events MUST be called to receive the responses in the callback.

@warning
    This function is not allowed in STA mode.

@see
	tpfAppWifiCb
	tenuM2mSecType
	m2m_wifi_init
	M2M_WIFI_REQ_DHCP_CONF
	tstrM2mWifiStateChanged
	tstrM2MAPConfig

\section WIFIExample5 Example
  The code snippet demonstrates how the AP mode is enabled after the driver is initialized in the application's main function and the handling
  of the event @ref M2M_WIFI_REQ_DHCP_CONF, to indicate successful connection.
@code
	#include "m2m_wifi.h"
	#include "m2m_types.h"
	
	void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
	{
		switch(u8WiFiEvent)
		{
		case M2M_WIFI_REQ_DHCP_CONF:
			{
				uint8	*pu8IPAddress = (uint8*)pvMsg;

				printf("Associated STA has IP Address \"%u.%u.%u.%u\"\n",pu8IPAddress[0],pu8IPAddress[1],pu8IPAddress[2],pu8IPAddress[3]);
			}
			break;
			
		default:
			break;
		}
	}
	
	int main()
	{
		tstrWifiInitParam 	param;
		
		param.pfAppWifiCb	= wifi_event_cb;
		if(!m2m_wifi_init(&param))
		{
			tstrM2MAPConfig		apConfig;
			
			strcpy(apConfig.au8SSID, "WINC_SSID");
			apConfig.u8ListenChannel 	= 1;
			apConfig.u8SecType			= M2M_WIFI_SEC_OPEN;
			apConfig.u8SsidHide			= 0;
			
			// IP Address
			apConfig.au8DHCPServerIP[0]	= 192;
			apConfig.au8DHCPServerIP[1]	= 168;
			apConfig.au8DHCPServerIP[2]	= 1;
            apConfig.au8DHCPServerIP[3]	= 1;
			
			// Trigger AP
			m2m_wifi_enable_ap(&apConfig);
			
			while(1)
			{
				m2m_wifi_handle_events(NULL);
			}
		}
	}

@endcode
*/
NMI_API sint8 m2m_wifi_enable_ap(CONST tstrM2MAPConfig* pstrM2MAPConfig);

/*!
@fn	\
    NMI_API sint8 m2m_wifi_enable_ap_ext(CONST tstrM2MAPModeConfig* pstrM2MAPModeConfig);

@brief
    Asynchronous API to enable access point (AKA "hot-spot") mode on the WINC IC with extended options.

@details
    The WINC IC supports the ability to operate as an access point with the following limitations:
    - Only 1 station may be associated at any given time.
	  - open system and WEP are the only security suites supported.

@param [in]	pstrM2MAPModeConfig
    A structure holding the AP configurations.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@pre
    - A Wi-Fi notification callback of type @ref tpfAppWifiCb  MUST be implemented and registered at initialization. Registering the callback
    is done through passing it to the @ref m2m_wifi_init.
    - The event @ref M2M_WIFI_REQ_DHCP_CONF must be handled in the callback.
    - The @ref m2m_wifi_handle_events MUST be called to receive the responses in the callback.

@warning
    This function is not allowed in STA mode.

@see
    tpfAppWifiCb
    tenuM2mSecType
    m2m_wifi_init
    M2M_WIFI_REQ_DHCP_CONF
    tstrM2mWifiStateChanged
    tstrM2MAPModeConfig
    tstrM2MAPConfigExt

\section WIFIExample5b Example
  The code snippet demonstrates how the AP mode is enabled after the driver is initialized in the application's main function and the handling
  of the event @ref M2M_WIFI_REQ_DHCP_CONF, to indicate successful connection.
@code
    #include "m2m_wifi.h"
    #include "m2m_types.h"
    
    void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
    {
        switch(u8WiFiEvent)
        {
        case M2M_WIFI_REQ_DHCP_CONF:
            {
                uint8	*pu8IPAddress = (uint8*)pvMsg;

                printf("Associated STA has IP Address \"%u.%u.%u.%u\"\n",pu8IPAddress[0],pu8IPAddress[1],pu8IPAddress[2],pu8IPAddress[3]);
            }
            break;
            
        default:
            break;
        }
    }
    
    int main()
    {
        tstrWifiInitParam 	param;
        
        param.pfAppWifiCb	= wifi_event_cb;
        if(!m2m_wifi_init(&param))
        {
            tstrM2MAPModeConfig		apModeConfig;
            
            strcpy(apConfig.strApConfig.au8SSID, "WINC_SSID");
            apModeConfig.strApConfig.u8ListenChannel 	= 1;
            apModeConfig.strApConfig.u8SecType			= M2M_WIFI_SEC_OPEN;
            apModeConfig.strApConfig.u8SsidHide			= 0;
            
            // IP Address
            apModeConfig.strApConfig.au8DHCPServerIP[0]	= 192;
            apModeConfig.strApConfig.au8DHCPServerIP[1]	= 168;
            apModeConfig.strApConfig.au8DHCPServerIP[2]	= 1;
            apModeConfig.strApConfig.au8DHCPServerIP[3]	= 1;

            // Default router IP
			m2m_memcpy(apModeConfig.strApConfigExt.au8DefRouterIP, apModeConfig.strApConfig.au8DHCPServerIP, 4);

            // DNS Server IP
			m2m_memcpy(apModeConfig.strApConfigExt.au8DNSServerIP, apModeConfig.strApConfig.au8DHCPServerIP, 4);
			
			// Subnet mask
            apModeConfig.strApConfigExt.au8SubnetMask[0]    = 255;
            apModeConfig.strApConfigExt.au8SubnetMask[1]    = 255;
            apModeConfig.strApConfigExt.au8SubnetMask[2]    = 255;
            apModeConfig.strApConfigExt.au8SubnetMask[3]    = 0;

            // Trigger AP
            m2m_wifi_enable_ap_ext(&apModeConfig);
            
            while(1)
            {
                m2m_wifi_handle_events(NULL);
            }
        }
    }
@endcode
*/
NMI_API sint8 m2m_wifi_enable_ap_ext(CONST tstrM2MAPModeConfig* pstrM2MAPModeConfig);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_disable_ap(void);

@brief
    Synchronous API to disable access point mode on the WINC IC.

@details
    Must be called only when the AP is enabled through the @ref m2m_wifi_enable_ap
    function. Otherwise the call to this function will not be useful.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
         m2m_wifi_enable_ap
*/
NMI_API sint8 m2m_wifi_disable_ap(void);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_set_static_ip(tstrM2MIPConfig * pstrStaticIPConf);

@brief
    Synchronous API to manually assign a (static) IP address to the WINC IC.

@details
	Assign a static IP address to the WINC board.
	This function assigns a static IP address in case the AP doesn't have a DHCP 
	server or in case the application wants to assign a predefined known IP 
    address. The user must keep in mind that assigning a static IP address might
	result in an IP address conflict. In case of an IP address conflict observed 
	by the WINC board the user will get a response of @ref M2M_WIFI_RESP_IP_CONFLICT 
	in the wifi callback. The application is then responsible to either solve the 
	conflict or assign another IP address. 

@pre
    The application must disable auto DHCP using @ref m2m_wifi_enable_dhcp 
	before assigning a static IP address. 

@warning
	Normally this function should not be used. 
	DHCP configuration is requested automatically after successful Wi-Fi connection is established.

@param [in]	pstrStaticIPConf
    Pointer to a structure holding the static IP configuration (IP, Gateway, subnet mask and
    DNS address).

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.
	
@see
	tstrM2MIPConfig
*/
NMI_API sint8 m2m_wifi_set_static_ip(tstrM2MIPConfig * pstrStaticIPConf);
 
/*!
@fn	\
	NMI_API sint8 m2m_wifi_request_dhcp_client(void);
	
@brief
    Legacy (deprecated) Asynchronous API for starting a DHCP client on the WINC IC.
	
@details
    This is a legacy API and is no longer supported. Calls to this API will not result in any 
	changes being made to the state of the WINC IC. 

@warning
    This function has been deprecated. DHCP is used automatically when the WINC IC connects.

@return
    This function always returns @ref M2M_SUCCESS.
*/
NMI_API sint8 m2m_wifi_request_dhcp_client(void);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_request_dhcp_server(uint8* addr);

@brief
    Dhcp requested by the firmware automatically in STA/AP mode).

@warning
    This function is legacy and exists only for compatibility with older applications. 
	DHCP server is started automatically when enabling the AP mode.

@return
    This function always returns @ref M2M_SUCCESS.
*/
NMI_API sint8 m2m_wifi_request_dhcp_server(uint8* addr);

/*!
@fn	\
	NMI_API  sint8 m2m_wifi_enable_dhcp(uint8  u8DhcpEn );
	
@brief
	Enable/Disable the DHCP client after connection.

@details
    Synchronous Wi-Fi DHCP enable function. This function will Enable/Disable the DHCP protocol.

@param [in]	 u8DhcpEn 
    The state of the DHCP client feature after successful association with an access point:
    - 1: Enables DHCP client after connection.
    - 0: Disables DHCP client after connection.
	
@return
	The function SHALL return @ref M2M_SUCCESS  for successful operation and a negative value otherwise.

@warning
    DHCP client is enabled by default.
    This Function should be called before using @ref m2m_wifi_set_static_ip
    
@see
    m2m_wifi_set_static_ip
*/
NMI_API sint8 m2m_wifi_enable_dhcp(uint8  u8DhcpEn );

/*!
@fn	\
    sint8 m2m_wifi_set_scan_options(tstrM2MScanOption* ptstrM2MScanOption);

@brief
    Synchronous API for configuring the behaviour of the WINC IC's network scanning functions.

@details
	This function sets the time configuration parameters for the scan operation.

@param [in]	ptstrM2MScanOption;
	Pointer to the structure holding the Scan Parameters.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	tenuM2mScanCh
	m2m_wifi_request_scan
	tstrM2MScanOption
*/
NMI_API sint8 m2m_wifi_set_scan_options(tstrM2MScanOption* ptstrM2MScanOption);

/*!
@fn	\
    sint8 m2m_wifi_set_scan_region(uint16 ScanRegion);

@brief
    Synchronous API for configuring the regulatory restrictions that may affect the WINC ICs 
	scanning behaviour.

@details
    This function sets a property called the scan region, a parameter that affects the range of 
	channels that the WINC IC may legally scan given a geographic region.

    For 2.4GHz, supported in the current release, the requested scan region can't exceed the
    maximum number of channels (14).

@param [in]	ScanRegion
		ASIA
		NORTH_AMERICA

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	tenuM2mScanCh
	m2m_wifi_request_scan
*/
NMI_API sint8 m2m_wifi_set_scan_region(uint16  ScanRegion);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_request_scan(uint8 ch);

@brief
    Asynchronous API to request the WINC IC to scan for networks.

@details
    Scan statuses are delivered to the application via the Wi-Fi event callback (@ref tpfAppWifiCb) in
    three stages. The first step involves the event @ref M2M_WIFI_RESP_SCAN_DONE which, if successful,
    provides the number of detected networks (access points). The application must then read the list
    of access points via multiple calls to the asynchronous @ref m2m_wifi_req_scan_result API. For
    each call to this function, the application will receive (step three) the event
    @ref M2M_WIFI_RESP_SCAN_RESULT.

@param [in]	ch
    RF Channel ID for SCAN operation. It should be set according to @ref tenuM2mScanCh, with a
    value of @ref M2M_WIFI_CH_ALL to scan all channels.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC,
    and a negative value otherwise.

@pre
    - A Wi-Fi notification callback of type @ref tpfAppWifiCb MUST be implemented and registered at
      initialization. Registration of the callback is done via @ref m2m_wifi_init.
    - The events @ref M2M_WIFI_RESP_SCAN_DONE and @ref M2M_WIFI_RESP_SCAN_RESULT must be handled in
      the (tpfAppWifiCb) callback.
    - The @ref m2m_wifi_handle_events function MUST be called to receive the responses in the
      callback.

@warning
    This API is valid only for STA mode, it may be called regardless of connection state (connected
    or disconnected states).

@see
	M2M_WIFI_RESP_SCAN_DONE
	M2M_WIFI_RESP_SCAN_RESULT
	tpfAppWifiCb
	tstrM2mWifiscanResult
	tenuM2mScanCh
	m2m_wifi_init
	m2m_wifi_handle_events
	m2m_wifi_req_scan_result

\section WIFIExample6 Example
  The code snippet demonstrates an example of how the scan request is called from the application's main function and the handling of
  the events received in response.
@code
	#include "m2m_wifi.h"
	#include "m2m_types.h"
	
	void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
	{
		static uint8	u8ScanResultIdx = 0;
		
		switch(u8WiFiEvent)
		{
		case M2M_WIFI_RESP_SCAN_DONE:
			{
				tstrM2mScanDone	*pstrInfo = (tstrM2mScanDone*)pvMsg;
				
				printf("Num of AP found %d\n",pstrInfo->u8NumofCh);
				if(pstrInfo->s8ScanState == M2M_SUCCESS)
				{
					u8ScanResultIdx = 0;
					if(pstrInfo->u8NumofCh >= 1)
					{
						m2m_wifi_req_scan_result(u8ScanResultIdx);
						u8ScanResultIdx ++;
					}
					else
					{
						printf("No AP Found Rescan\n");
						m2m_wifi_request_scan(M2M_WIFI_CH_ALL);
					}
				}
				else
				{
					printf("(ERR) Scan fail with error <%d>\n",pstrInfo->s8ScanState);
				}
			}
			break;
		
		case M2M_WIFI_RESP_SCAN_RESULT:
			{
				tstrM2mWifiscanResult		*pstrScanResult =(tstrM2mWifiscanResult*)pvMsg;
				uint8						u8NumFoundAPs = m2m_wifi_get_num_ap_found();
				
				printf(">>%02d RI %d SEC %s CH %02d BSSID %02X:%02X:%02X:%02X:%02X:%02X SSID %s\n",
					pstrScanResult->u8index,pstrScanResult->s8rssi,
					pstrScanResult->u8AuthType,
					pstrScanResult->u8ch,
					pstrScanResult->au8BSSID[0], pstrScanResult->au8BSSID[1], pstrScanResult->au8BSSID[2],
					pstrScanResult->au8BSSID[3], pstrScanResult->au8BSSID[4], pstrScanResult->au8BSSID[5],
					pstrScanResult->au8SSID);
				
				if(u8ScanResultIdx < u8NumFoundAPs)
				{
					// Read the next scan result
					m2m_wifi_req_scan_result(index);
					u8ScanResultIdx ++;
				}
			}
			break;
		default:
			break;
		}
	}
	
	int main()
	{
		tstrWifiInitParam 	param;
		
		param.pfAppWifiCb	= wifi_event_cb;
		if(!m2m_wifi_init(&param))
		{
			// Scan all channels
			m2m_wifi_request_scan(M2M_WIFI_CH_ALL);
			
			while(1)
			{
				m2m_wifi_handle_events(NULL);
			}
		}
	}
@endcode
*/
NMI_API sint8 m2m_wifi_request_scan(uint8 ch);

/*!
@fn	\
	NMI_API sint8 m2m_wifi_request_scan_passive(uint8 ch, uint16 scan_time);

@brief
    Similar to @ref m2m_wifi_request_scan but performs passive scanning instead of active scanning.

@param [in]	ch
    RF Channel ID for SCAN operation. It should be set according to @ref tenuM2mScanCh.
    With a value of @ref M2M_WIFI_CH_ALL, means to scan all channels.

@param [in]	scan_time
    The time in ms that passive scan is listening for beacons on each channel per one slot, enter 0 for default setting.

@warning
    This function is not allowed in AP mode. It works only for STA mode (both connected or disconnected states).
				
@pre
	- A Wi-Fi notification callback of type @ref tpfAppWifiCb MUST be implemented and registered at initialization. Registering the callback
	  is done through passing it to the @ref m2m_wifi_init.
	- The events @ref M2M_WIFI_RESP_SCAN_DONE and @ref M2M_WIFI_RESP_SCAN_RESULT.
	  must be handled in the callback.
	- The @ref m2m_wifi_handle_events function MUST be called to receive the responses in the callback.

@see
	M2M_WIFI_RESP_SCAN_DONE
	M2M_WIFI_RESP_SCAN_RESULT
	tpfAppWifiCb
	tstrM2mWifiscanResult
	tenuM2mScanCh
	m2m_wifi_init
    m2m_wifi_request_scan
	m2m_wifi_handle_events
	m2m_wifi_req_scan_result

@return
	The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.
*/
NMI_API sint8 m2m_wifi_request_scan_passive(uint8 ch, uint16 scan_time);

/*!
@fn \
    NMI_API sint8 m2m_wifi_request_scan_ssid_list(uint8 ch,uint8 * u8SsidList);

@brief
    Asynchronous wi-fi scan request on the given channel and the hidden scan list.

@details
	The scan status is delivered in the wi-fi event callback and then the application
	is to read the scan results sequentially. 
	The number of  APs found (N) is returned in event @ref M2M_WIFI_RESP_SCAN_DONE with the number of found
	APs.
	The application could read the list of APs by calling the function @ref m2m_wifi_req_scan_result N times.

@param [in]	ch
    RF Channel ID for SCAN operation. It should be set according to @ref tenuM2mScanCh.
    With a value of @ref M2M_WIFI_CH_ALL, means to scan all channels.

@param [in]	u8SsidList
              u8SsidList is a buffer containing a list of hidden SSIDs to 
			  include during the scan. The first byte in the buffer, u8SsidList[0], 
			  is the number of SSIDs encoded in the string. The number of hidden SSIDs 
    cannot exceed @ref MAX_HIDDEN_SITES. All SSIDs are concatenated in the following
			  bytes and each SSID is prefixed with a one-byte header containing its length.  
			  The total number of bytes in u8SsidList buffer, including length byte, cannot 
			  exceed 133 bytes (MAX_HIDDEN_SITES SSIDs x 32 bytes each, which is max SSID length).
              For instance, encoding the two hidden SSIDs "DEMO_AP" and "TEST" 
			  results in the following buffer content: 

@code
              uint8 u8SsidList[14];
              u8SsidList[0] = 2; // Number of SSIDs is 2
              u8SsidList[1] = 7; // Length of the string "DEMO_AP" without NULL termination
              memcpy(&u8SsidList[2], "DEMO_AP", 7);  // Bytes index 2-9 containing the string DEMO_AP
              u8SsidList[9] = 4; // Length of the string "TEST" without NULL termination
              memcpy(&u8SsidList[10], "TEST", 4);  // Bytes index 10-13 containing the string TEST
@endcode

@note
    It works with STA/AP mode (connected or disconnected).
				
@pre
	- A Wi-Fi notification callback of type @ref tpfAppWifiCb MUST be implemented and registered at initialization. Registering the callback
	  is done through passing it to the @ref m2m_wifi_init.
	- The events @ref M2M_WIFI_RESP_SCAN_DONE and @ref M2M_WIFI_RESP_SCAN_RESULT.
	  must be handled in the callback.
	- The @ref m2m_wifi_handle_events function MUST be called to receive the responses in the callback.

@see
	M2M_WIFI_RESP_SCAN_DONE
	M2M_WIFI_RESP_SCAN_RESULT
	tpfAppWifiCb
	tstrM2mWifiscanResult
	tenuM2mScanCh
	m2m_wifi_init
	m2m_wifi_handle_events
	m2m_wifi_req_scan_result

@return
	The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.

\section WIFIExample6b Example
  The code snippet demonstrates an example of how the scan request is called from the application's main function and the handling of
  the events received in response.
@code
	#include "m2m_wifi.h"
	#include "m2m_types.h"

	static void request_scan_hidden_demo_ap(void);
	
	void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
	{
		static uint8	u8ScanResultIdx = 0;
		
		switch(u8WiFiEvent)
		{
		case M2M_WIFI_RESP_SCAN_DONE:
			{
				tstrM2mScanDone	*pstrInfo = (tstrM2mScanDone*)pvMsg;
				
				printf("Num of AP found %d\n",pstrInfo->u8NumofCh);
				if(pstrInfo->s8ScanState == M2M_SUCCESS)
				{
					u8ScanResultIdx = 0;
					if(pstrInfo->u8NumofCh >= 1)
					{
						m2m_wifi_req_scan_result(u8ScanResultIdx);
						u8ScanResultIdx ++;
					}
					else
					{
						printf("No AP Found Rescan\n");
						request_scan_hidden_demo_ap();
					}
				}
				else
				{
					printf("(ERR) Scan fail with error <%d>\n",pstrInfo->s8ScanState);
				}
			}
			break;
		case M2M_WIFI_RESP_SCAN_RESULT:
			{
				tstrM2mWifiscanResult		*pstrScanResult =(tstrM2mWifiscanResult*)pvMsg;
				uint8						u8NumFoundAPs = m2m_wifi_get_num_ap_found();
				
				printf(">>%02d RI %d SEC %s CH %02d BSSID %02X:%02X:%02X:%02X:%02X:%02X SSID %s\n",
					pstrScanResult->u8index,pstrScanResult->s8rssi,
					pstrScanResult->u8AuthType,
					pstrScanResult->u8ch,
					pstrScanResult->au8BSSID[0], pstrScanResult->au8BSSID[1], pstrScanResult->au8BSSID[2],
					pstrScanResult->au8BSSID[3], pstrScanResult->au8BSSID[4], pstrScanResult->au8BSSID[5],
					pstrScanResult->au8SSID);
				
				if(u8ScanResultIdx < u8NumFoundAPs)
				{
					// Read the next scan result
					m2m_wifi_req_scan_result(index);
					u8ScanResultIdx ++;
				}
			}
			break;
		default:
			break;
		}
	}

	static void request_scan_hidden_demo_ap(void)
	{
		uint8 list[9];
		char ssid[] = "DEMO_AP";
		uint8 len = (uint8)(sizeof(ssid)-1);

		list[0] = 1;
		list[1] = len;
		memcpy(&list[2], ssid, len); // copy 7 bytes
		// Scan all channels
		m2m_wifi_request_scan_ssid_list(M2M_WIFI_CH_ALL, list);
	}
	

	int main()
	{
		tstrWifiInitParam 	param;
		
		param.pfAppWifiCb	= wifi_event_cb;
		if(!m2m_wifi_init(&param))
		{
			request_scan_hidden_demo_ap();
			while(1)
			{
				m2m_wifi_handle_events(NULL);
			}
		}
	}
@endcode
*/
NMI_API sint8 m2m_wifi_request_scan_ssid_list(uint8 ch,uint8 * u8Ssidlist);

/*!
@fn \
    NMI_API uint8 m2m_wifi_get_num_ap_found(void);

@brief
	Synchronous function to retrieve the number of AP's found during the last scan operation.

@details
	The function reads the number of APs from global variable which was updated in the Wi-Fi 
    callback function through the @ref M2M_WIFI_RESP_SCAN_DONE event.
	Function used only in STA mode only. 

@see       
	m2m_wifi_request_scan 
		   M2M_WIFI_RESP_SCAN_DONE
		   M2M_WIFI_RESP_SCAN_RESULT         

@pre
    m2m_wifi_request_scan must be called first to ensure up to date results are available.
		- A Wi-Fi notification callback of type @ref tpfAppWifiCb MUST be implemented and registered at initialization. Registering the callback
		   is done through passing it to the @ref m2m_wifi_init.
		- The event @ref M2M_WIFI_RESP_SCAN_DONE must be handled in the callback to receive the requested scan information. 

@warning   
	This function must be called only in the wi-fi callback function when the events 
	@ref M2M_WIFI_RESP_SCAN_DONE or @ref M2M_WIFI_RESP_SCAN_RESULT are received.
		   Calling this function in any other place will result in undefined/outdated numbers.
		  
@return
	Return the number of AP's found in the last Scan Request.
		  
\section WIFIExample7 Example
  The code snippet demonstrates an example of how the scan request is called from the application's main function and the handling of
  the events received in response.
@code
	#include "m2m_wifi.h"
	#include "m2m_types.h"
	
	void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
	{
		static uint8	u8ScanResultIdx = 0;
		
		switch(u8WiFiEvent)
		{
		case M2M_WIFI_RESP_SCAN_DONE:
			{
				tstrM2mScanDone	*pstrInfo = (tstrM2mScanDone*)pvMsg;
				
				printf("Num of AP found %d\n",pstrInfo->u8NumofCh);
				if(pstrInfo->s8ScanState == M2M_SUCCESS)
				{
					u8ScanResultIdx = 0;
					if(pstrInfo->u8NumofCh >= 1)
					{
						m2m_wifi_req_scan_result(u8ScanResultIdx);
						u8ScanResultIdx ++;
					}
					else
					{
						printf("No AP Found Rescan\n");
						m2m_wifi_request_scan(M2M_WIFI_CH_ALL);
					}
				}
				else
				{
					printf("(ERR) Scan fail with error <%d>\n",pstrInfo->s8ScanState);
				}
			}
			break;
		
		case M2M_WIFI_RESP_SCAN_RESULT:
			{
				tstrM2mWifiscanResult		*pstrScanResult =(tstrM2mWifiscanResult*)pvMsg;
				uint8						u8NumFoundAPs = m2m_wifi_get_num_ap_found();
				
				printf(">>%02d RI %d SEC %s CH %02d BSSID %02X:%02X:%02X:%02X:%02X:%02X SSID %s\n",
					pstrScanResult->u8index,pstrScanResult->s8rssi,
					pstrScanResult->u8AuthType,
					pstrScanResult->u8ch,
					pstrScanResult->au8BSSID[0], pstrScanResult->au8BSSID[1], pstrScanResult->au8BSSID[2],
					pstrScanResult->au8BSSID[3], pstrScanResult->au8BSSID[4], pstrScanResult->au8BSSID[5],
					pstrScanResult->au8SSID);
				
				if(u8ScanResultIdx < u8NumFoundAPs)
				{
					// Read the next scan result
					m2m_wifi_req_scan_result(index);
					u8ScanResultIdx ++;
				}
			}
			break;
		default:
			break;
		}
	}
	
	int main()
	{
		tstrWifiInitParam 	param;
		
		param.pfAppWifiCb	= wifi_event_cb;
		if(!m2m_wifi_init(&param))
		{
			// Scan all channels
			m2m_wifi_request_scan(M2M_WIFI_CH_ALL);
			
			while(1)
			{
				m2m_wifi_handle_events(NULL);
			}
		}
	}
@endcode			 
*/
NMI_API uint8 m2m_wifi_get_num_ap_found(void);

/*!
@fn \
    NMI_API sint8 m2m_wifi_req_scan_result(uint8 index);

@brief
    Synchronous call to read the AP information from the SCAN Result list.

@details
	Synchronous call to read the AP information from the SCAN Result list with the given index.
    This function is expected to be called when the response events @ref M2M_WIFI_RESP_SCAN_RESULT or
    @ref M2M_WIFI_RESP_SCAN_DONE are received in the wi-fi callback function.
	The response information received can be obtained through the casting to the 
    @ref tstrM2mWifiscanResult structure.

@param [in]  index 
    Index for the requested result, the index range start from 0 till number of AP's found.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	tstrM2mWifiscanResult
		   m2m_wifi_get_num_ap_found
		   m2m_wifi_request_scan             
		   
@pre         
	- @ref m2m_wifi_request_scan needs to be called first, then m2m_wifi_get_num_ap_found 
      to get the number of AP's found.
	- A Wi-Fi notification callback of type @ref tpfAppWifiCb MUST be implemented and registered 
      at startup. Registering the callback is done through passing it to the @ref m2m_wifi_init function.
	- The event @ref M2M_WIFI_RESP_SCAN_RESULT must be handled in the callback to receive the 
 	  requested scan information.
			 
@warning
    - Function used in STA mode only. the scan results are updated only if the scan request 
	  is called.
	- Calling this function only without a scan request will lead to 
 	  firmware errors. 
	- Application code should refrain from introducing a large delay  between the scan request and the scan result 
	  request, to prevent errors occurring.

\section WIFIExample8 Example
  The code snippet demonstrates an example of how the scan request is called from the application's main function and the handling of
  the events received in response.
@code
	#include "m2m_wifi.h"
	#include "m2m_types.h"
	
	void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
	{
		static uint8	u8ScanResultIdx = 0;
		
		switch(u8WiFiEvent)
		{
		case M2M_WIFI_RESP_SCAN_DONE:
			{
				tstrM2mScanDone	*pstrInfo = (tstrM2mScanDone*)pvMsg;
				
				printf("Num of AP found %d\n",pstrInfo->u8NumofCh);
				if(pstrInfo->s8ScanState == M2M_SUCCESS)
				{
					u8ScanResultIdx = 0;
					if(pstrInfo->u8NumofCh >= 1)
					{
						m2m_wifi_req_scan_result(u8ScanResultIdx);
						u8ScanResultIdx ++;
					}
					else
					{
						printf("No AP Found Rescan\n");
						m2m_wifi_request_scan(M2M_WIFI_CH_ALL);
					}
				}
				else
				{
					printf("(ERR) Scan fail with error <%d>\n",pstrInfo->s8ScanState);
				}
			}
			break;
		
		case M2M_WIFI_RESP_SCAN_RESULT:
			{
				tstrM2mWifiscanResult		*pstrScanResult =(tstrM2mWifiscanResult*)pvMsg;
				uint8						u8NumFoundAPs = m2m_wifi_get_num_ap_found();
				
				printf(">>%02d RI %d SEC %s CH %02d BSSID %02X:%02X:%02X:%02X:%02X:%02X SSID %s\n",
					pstrScanResult->u8index,pstrScanResult->s8rssi,
					pstrScanResult->u8AuthType,
					pstrScanResult->u8ch,
					pstrScanResult->au8BSSID[0], pstrScanResult->au8BSSID[1], pstrScanResult->au8BSSID[2],
					pstrScanResult->au8BSSID[3], pstrScanResult->au8BSSID[4], pstrScanResult->au8BSSID[5],
					pstrScanResult->au8SSID);
				
				if(u8ScanResultIdx < u8NumFoundAPs)
				{
					// Read the next scan result
					m2m_wifi_req_scan_result(index);
					u8ScanResultIdx ++;
				}
			}
			break;
		default:
			break;
		}
	}
	
	int main()
	{
		tstrWifiInitParam 	param;
		
		param.pfAppWifiCb	= wifi_event_cb;
		if(!m2m_wifi_init(&param))
		{
			// Scan all channels
			m2m_wifi_request_scan(M2M_WIFI_CH_ALL);
			
			while(1)
			{
				m2m_wifi_handle_events(NULL);
			}
		}
	}
@endcode 
*/
NMI_API sint8 m2m_wifi_req_scan_result(uint8 index);

/*!
@fn \
    NMI_API sint8 m2m_wifi_req_curr_rssi(void);

@brief
    Asynchronous API to request the current Receive Signal Strength (RSSI) of the current connection.

@details
    The response received in through the @ref M2M_WIFI_RESP_CURRENT_RSSI event.

@pre
	- A Wi-Fi notification callback of type @ref tpfAppWifiCb MUST be implemented and registered 
	  before initialization. Registering the callback is done through passing it to the 
      @ref m2m_wifi_init through the @ref tstrWifiInitParam initialization structure.
		   - The event @ref M2M_WIFI_RESP_CURRENT_RSSI must be handled in the callback to receive the requested Rssi information.       

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.	

\section WIFIExample9 Example
  The code snippet demonstrates how the RSSI request is called in the application's main function and the handling of the event received in the callback. 
@code
	#include "m2m_wifi.h"
	#include "m2m_types.h"
	
	void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
	{
		static uint8	u8ScanResultIdx = 0;
		
		switch(u8WiFiEvent)
		{
		case M2M_WIFI_RESP_CURRENT_RSSI:
			{
				sint8	*rssi = (sint8*)pvMsg;
				M2M_INFO("ch rssi %d\n",*rssi);
			}
			break;
		default:
			break;
		}
	}
	
	int main()
	{
		tstrWifiInitParam 	param;
		
		param.pfAppWifiCb	= wifi_event_cb;
		if(!m2m_wifi_init(&param))
		{
			// Scan all channels
			m2m_wifi_req_curr_rssi();
			
			while(1)
			{
				m2m_wifi_handle_events(NULL);
			}
		}
	}
@endcode	
*/
NMI_API sint8 m2m_wifi_req_curr_rssi(void);

/*!
@fn \
    NMI_API sint8 m2m_wifi_get_otp_mac_address(uint8 *pu8MacAddr, uint8 * pu8IsValid);

@brief
    Synchronous API to query the MAC address programmed into the WINC ICs OTP memory.

@details
    This function attempts to read the device's MAC address from the One Time Programmable (OTP)
	memory on the IC. The presence (yes or no) of a MAC address in the OTP memory and, in the case
    of it being present, its value is returned via RAM pointed to by the input arguments.

    Request the MAC address stored on the One Time Programmable(OTP) memory of the device.
    The function is blocking until the response is received.

@pre
    Prior call to @ref m2m_wifi_init is required before any WIFI/socket function.

@param [out] pu8MacAddr
    Output MAC address buffer 6 bytes in size. Valid only if *pu8Valid=1.

@param [out] pu8IsValid
		     Output boolean value to indicate the validity of pu8MacAddr in OTP. 
		     Output zero if the OTP memory is not programmed, non-zero otherwise.

@return
    The function returns @ref M2M_SUCCESS for success and a negative value otherwise.

@see         
    m2m_wifi_get_mac_address             
*/
NMI_API sint8 m2m_wifi_get_otp_mac_address(uint8 *pu8MacAddr, uint8 * pu8IsValid);

/*!
@fn \
    NMI_API sint8 m2m_wifi_get_mac_address(uint8 *pu8MacAddr)

@brief
    Synchronous API to retrieve the MAC address currently in use by the device.

@details
	Function to retrieve the current MAC address. 
	The function is blocking until the response is received.

@pre         
    Prior call to @ref m2m_wifi_init is required before any WIFI/socket function.

@param [out] pu8MacAddr
    Pointer to a buffer in memory containing a 6-byte MAC address (provided function returns 
    @ref M2M_SUCCESS).

@return
	The function returns @ref M2M_SUCCESS for successful operation and a negative value otherwise.

@see
	m2m_wifi_get_otp_mac_address             
*/
NMI_API sint8 m2m_wifi_get_mac_address(uint8 *pu8MacAddr);

/*!
@fn \
    NMI_API sint8 m2m_wifi_set_sleep_mode(uint8 PsTyp, uint8 BcastEn);

@brief
    Synchronous API to set the power-save mode of the WINC IC.

@details
    This is one of the two synchronous power-save setting functions that allow the host MCU application
    to tweak the system power consumption. Such tweaking can be done through one of two ways:
    - 1) Changing the power save mode, to one of the allowed power save modes (see @ref tenuPowerSaveModes). This is done by setting the first parameter.
    - 2) Configuring DTIM monitoring: Configuring beacon monitoring parameters by enabling or disabling the reception of broadcast/multicast data.
	this is done by setting the second parameter.

@param [in]	PsTyp
			Desired power saving mode. Supported types are enumerated in @ref tenuPowerSaveModes.

@param [in]	BcastEn
			Broadcast reception enable flag. 
    If it is 1, the WINC will be awake for each DTIM beacon in order to check for and receive broadcast traffic.
    If it is 0, the WINC will ignore broadcast traffic. Through this flag the WINC will not wakeup at the DTIM beacon,
    but instead it will wakeup only based on the configured Listen Interval.

@warning
	The function called once after initialization.

@return
	The function returns @ref M2M_SUCCESS for successful operation and a negative value otherwise.

@see
	tenuPowerSaveModes
		   m2m_wifi_get_sleep_mode
		   m2m_wifi_set_lsn_int
*/
NMI_API sint8 m2m_wifi_set_sleep_mode(uint8 PsTyp, uint8 BcastEn);

/*!
@fn \
    NMI_API sint8 m2m_wifi_request_sleep(uint32 u32SlpReqTime);

@brief
    API to place the WINC IC into sleep mode for a specified period of time.

@details
    Synchronous power-save sleep request function, which requests the WINC device to sleep in the currently configured
    power save mode, as set using @ref m2m_wifi_set_sleep_mode, for a specific time as defined by the passed in parameter.
	This function should be used in the @ref M2M_PS_MANUAL power save mode only.
    A wake up request is automatically performed by the WINC device when any host driver API function, e.g. Wi-Fi or socket operation is called.

@param [in]	u32SlpReqTime
    Request sleep time in ms.\n
	The best recommended sleep duration is left to be determined by the application. 
	Taking into account that if the application sends notifications very rarely, 
	sleeping for a long time can be a power-efficient decision. 
    In contrast, applications that are sensitive for long periods of absence can experience
			performance degradation in the connection if long sleeping times are used.

@return
    The function returns @ref M2M_SUCCESS for successful operation and a negative value otherwise.

@warning 
    The function should be called in @ref M2M_PS_MANUAL power save mode only. As enumerated in @ref tenuPowerSaveModes.
    It's also important to note that during the sleeping time while in the M2M_PS_MANUAL mode, AP beacon monitoring is
    bypassed and the wifi-connection may drop if the sleep period is elongated.

@see
	tenuPowerSaveModes 
	m2m_wifi_set_sleep_mode
*/
NMI_API sint8 m2m_wifi_request_sleep(uint32 u32SlpReqTime);

/*!
@fn \
    NMI_API uint8 m2m_wifi_get_sleep_mode(void);

@brief
	Synchronous power save mode retrieval function.

@return	    
    The current operating power saving mode based on the enumerated sleep modes @ref tenuPowerSaveModes.

@see
	tenuPowerSaveModes 
		    m2m_wifi_set_sleep_mode
*/
NMI_API uint8 m2m_wifi_get_sleep_mode(void);

/*!
@fn \
    NMI_API sint8 m2m_wifi_req_client_ctrl(uint8 cmd);

@brief		
    Asynchronous command sending function to the PS Client.

@details
    Asynchronous command sending function to the PS Client (a WINC board running the ps_firmware)
    if the PS client sends any command, it will be received through the @ref M2M_WIFI_RESP_CLIENT_INFO event.

@param [in]	cmd
			Control command sent from PS Server to PS Client (command values defined by the application)

@pre
    @ref m2m_wifi_req_server_init should be called first.

@warning	
	This mode is not supported in the current release.

@see
	m2m_wifi_req_server_init
			M2M_WIFI_RESP_CLIENT_INFO

@return		
    The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.
*/
NMI_API sint8 m2m_wifi_req_client_ctrl(uint8 cmd);

/*!
@fn \
    NMI_API sint8 m2m_wifi_req_server_init(uint8 ch);

@brief
	Synchronous function to initialize the PS Server.

@details
    The WINC supports non secure communication with another WINC,
	(SERVER/CLIENT) through one byte command (probe request and probe response) without any connection setup.
    The server mode can't be used with any other modes (STA/AP)

@param [in]	ch
			Server listening channel

@see
	m2m_wifi_req_client_ctrl

@warning
	This mode is not supported in the current release.

@return
    The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.
*/
NMI_API sint8 m2m_wifi_req_server_init(uint8 ch);

/*!
@fn \
    NMI_API sint8 m2m_wifi_set_device_name(uint8 *pu8DeviceName, uint8 u8DeviceNameLength);

@brief
    Asynchronous API to set the "Device Name" of the WINC IC.

@details
    Sets the WINC device name. The name string is used as a device name in DHCP
    hostname (option 12).
    If a device is not set through this function a default name is assigned.
	The default name is WINC-XX-YY, where XX and YY are the last 2 octets of the OTP 
	MAC address. If OTP (eFuse) is programmed, then the default name is WINC-00-00.

@warning	
    The function called once after initialization.\n
    Used for DHCP client hostname option (12).\n
	Device name shall contain only characters allowed in valid internet host name as 
	defined in RFC 952 and 1123.

@param [in]	pu8DeviceName
	Buffer holding the device name. Device name is a null terminated C string.

@param [in]	u8DeviceNameLength
	Length of the device name. Should not exceed the maximum device name's 
	length @ref M2M_DEVICE_NAME_MAX (including null character).

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.
*/
NMI_API sint8 m2m_wifi_set_device_name(uint8 *pu8DeviceName, uint8 u8DeviceNameLength);

/*!
@fn \
    NMI_API sint8 m2m_wifi_configure_sntp(uint8 *pu8NTPServerName, uint8 u8NTPServerNameLength, tenuSNTPUseDHCP useDHCP);

@brief
	Configures what NTP server the SNTP client should use.

@details
	Configures what NTP server the SNTP client should use. Only 1 server name can be provided, if the configured server name begins with an asterisk then it will be treated as a server pool.
	The SNTP client can also use the NTP server provided by the DHCP server through option 42.
	By default the NTP server provided by DHCP will be tried first, then the built-in default NTP server (time.nist.gov) will be used.
	Configuring a server name will overwrite the built-in default server until next reboot.

@param [in]	pu8NTPServerName
	Buffer holding the NTP server name. If the first character is an asterisk (*) then it will be treated as a server pool, where the asterisk will
	be replaced with an incrementing value from 0 to 3 each time a server fails (example: *.pool.ntp.org).

@param [in]	u8NTPServerNameLength
    Length of the NTP server name. Should not exceed the maximum NTP server name length of @ref M2M_NTP_MAX_SERVER_NAME_LENGTH.

@param [in]	useDHCP
	Should the NTP server provided by the DHCP server be used.

@return
    The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8 m2m_wifi_configure_sntp(uint8 *pu8NTPServerName, uint8 u8NTPServerNameLength, tenuSNTPUseDHCP useDHCP);

/*!
@fn \
    NMI_API sint8 m2m_wifi_set_lsn_int(tstrM2mLsnInt * pstrM2mLsnInt);

@brief
	API to set Wi-Fi listen interval for power save operation. 

@details
	This is one of the two synchronous power-save setting functions that
	allow the host MCU application to tweak the system power consumption. Such tweaking can be done by modifying the 
    Wi-Fi listen interval. The listen interval is how many beacon periods the station can sleep before it wakes up to receive data buffered in the AP.
	It is represented in units of AP beacon periods(100ms).  

@warning
	The function should be called once after initialization. 

@param [in]	pstrM2mLsnInt
    Structure holding the listen interval configuration.

@pre
	The function @ref m2m_wifi_set_sleep_mode shall be called first, to set the power saving mode required.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC and a negative value otherwise.

@see
	tstrM2mLsnInt
	m2m_wifi_set_sleep_mode
*/
NMI_API sint8 m2m_wifi_set_lsn_int(tstrM2mLsnInt *pstrM2mLsnInt);

/*!
@fn \
    NMI_API sint8 m2m_wifi_enable_monitoring_mode(tstrM2MWifiMonitorModeCtrl *, uint8 *, uint16 , uint16);

@brief
	Asynchronous call to enable Wi-Fi monitoring (promiscuous receive) mode.

@details
	Asynchronous Wi-Fi monitoring mode (Promiscuous mode) enabling function. 
	This function enables the monitoring mode, thus allowing two operations to be 
	performed:
	1) Transmission of manually configured frames, through using the 
		@ref m2m_wifi_send_wlan_pkt function.
    2) Reception of frames based on a defined filtering criteria.

	When the monitoring mode is enabled, reception of all frames that satisfy the 
	filter criteria passed in as a parameter is allowed, on the current wireless channel.
	All packets that meet the filtering criteria are passed to the application layer, to 
	be handled by the assigned monitoring callback function.
	The monitoring callback function must be implemented before starting the monitoring mode, 
	in-order to handle the packets received.
	Registering of the implemented callback function is through the callback pointer 
	@ref tpfAppMonCb in the @ref tstrWifiInitParam structure passed to @ref m2m_wifi_init function at initialization.

@param [in]     pstrMtrCtrl
	Pointer to @ref tstrM2MWifiMonitorModeCtrl structure holding the filtering parameters.

@param [in]     pu8PayloadBuffer
	Pointer to a Buffer allocated by the application. The buffer SHALL hold the Data field of 
	the WIFI RX Packet (Or a part from it). If it is set to NULL, the WIFI data payload will 
	be discarded by the monitoring driver.
	
@param [in]     u16BufferSize
	The total size of the pu8PayloadBuffer in bytes.
	
@param [in]     u16DataOffset
	Starting offset in the DATA FIELD of the received WIFI packet. The application may be interested
	in reading specific information from the received packet. It must assign the offset to the starting
	position of it relative to the DATA payload start.\n
	\e Example, \e if \e the \e SSID \e is \e needed \e to \e be \e read \e from \e a \e PROBE \e REQ 
	\e packet, \e the \e u16Offset \e MUST \e be \e set \e to \e 0.

@warning
    When this mode is enabled, you cannot be connected in any mode (Station or Access Point).

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	tstrM2MWifiMonitorModeCtrl
 			   tstrM2MWifiRxPacketInfo
 			   tstrWifiInitParam
 			   tenuM2mScanCh
 			   m2m_wifi_disable_monitoring_mode  
 			   m2m_wifi_send_wlan_pkt
 			   m2m_wifi_send_ethernet_pkt

\section WIFIExample10 Example
	The example demonstrates the main function where-by the monitoring enable function is called after 
	the initialization of the driver and the packets are handled in the callback function.
			
@code
			#include "m2m_wifi.h"
			#include "m2m_types.h"

			//Declare receive buffer 
			uint8 gmgmt[1600];
	
			//Callback functions
			void wifi_cb(uint8 u8WiFiEvent, void * pvMsg)
			{
				; 
			}

			void wifi_monitoring_cb(tstrM2MWifiRxPacketInfo *pstrWifiRxPacket, uint8 *pu8Payload, uint16 u16PayloadSize)
			{
				if((NULL != pstrWifiRxPacket) && (0 != u16PayloadSize)) {
					if(MANAGEMENT == pstrWifiRxPacket->u8FrameType) {
						M2M_INFO("***# MGMT PACKET #***\n");
					} else if(DATA_BASICTYPE == pstrWifiRxPacket->u8FrameType) {
						M2M_INFO("***# DATA PACKET #***\n");
					} else if(CONTROL == pstrWifiRxPacket->u8FrameType) {
						M2M_INFO("***# CONTROL PACKET #***\n");
					}
				}
			}
			
			int main()
			{
				//Register wifi_monitoring_cb 
				tstrWifiInitParam param;
				param.pfAppWifiCb = wifi_cb;
				param.pfAppMonCb  = wifi_monitoring_cb;
				
				nm_bsp_init();
				
				if(!m2m_wifi_init(&param)) {
					//Enable Monitor Mode with filter to receive all data frames on channel 1
					tstrM2MWifiMonitorModeCtrl	strMonitorCtrl = {0};
					strMonitorCtrl.u8ChannelID		= M2M_WIFI_CH_1;
					strMonitorCtrl.u8FrameType		= DATA_BASICTYPE;
					strMonitorCtrl.u8FrameSubtype	= M2M_WIFI_FRAME_SUB_TYPE_ANY; //Receive any subtype of data frame
					m2m_wifi_enable_monitoring_mode(&strMonitorCtrl, gmgmt, sizeof(gmgmt), 0);
					
					while(1) {
						m2m_wifi_handle_events(NULL);
					}
				}
				return 0;
			}
@endcode
 */
NMI_API sint8 m2m_wifi_enable_monitoring_mode(tstrM2MWifiMonitorModeCtrl *pstrMtrCtrl, uint8 *pu8PayloadBuffer, 
										   uint16 u16BufferSize, uint16 u16DataOffset);

/*!
@fn \
    NMI_API sint8 m2m_wifi_disable_monitoring_mode(void);

@brief
    API to disable Wi-Fi monitoring (promiscuous receive) mode.

@details
	Synchronous function to disable Wi-Fi monitoring mode (Promiscuous mode). Expected to be called, if the enable monitoring mode is set, but if it was called without enabling
	no negative impact will reside.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	m2m_wifi_enable_monitoring_mode               
 */
NMI_API sint8 m2m_wifi_disable_monitoring_mode(void);

/*!
@fn \
    NMI_API sint8 m2m_wifi_send_wlan_pkt(uint8 *, uint16, uint16);
 
@brief
	Synchronous function to transmit a WIFI RAW packet while the implementation of this packet is left to the application developer.

@pre
	Enable Monitoring mode first using @ref m2m_wifi_enable_monitoring_mode

@note
	Packets are user's responsibility.

@warning
    This function available in monitoring mode ONLY.

@param [in]     pu8WlanPacket
                 	     Pointer to a buffer holding the whole WIFI frame.

@param [in]     u16WlanHeaderLength
 			      The size of the WIFI packet header ONLY.

@param [in]     u16WlanPktSize
    The size of the whole packet in bytes.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	m2m_wifi_enable_monitoring_mode
 			   m2m_wifi_disable_monitoring_mode
 */
NMI_API sint8 m2m_wifi_send_wlan_pkt(uint8 *pu8WlanPacket, uint16 u16WlanHeaderLength, uint16 u16WlanPktSize);

/*!
@fn \
    NMI_API sint8 m2m_wifi_send_ethernet_pkt(uint8* pu8Packet,uint16 u16PacketSize);

@brief
	Synchronous function to transmit an Ethernet packet.

@details
	Transmit a packet directly in ETHERNET/bypass mode where the TCP/IP stack is disabled 
	and the implementation of this packet is left to the application developer. 
	The Ethernet packet composition is left to the application developer. 

@note
	Packets are the user's responsibility.

@warning
    This function available in ETHERNET/Bypass mode ONLY. Make sure that application defines ETH_MODE.

@param [in]     pu8Packet
	Pointer to a buffer holding the whole Ethernet frame.

@param [in]     u16PacketSize
    The size of the whole packet in bytes.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	m2m_wifi_enable_mac_mcast
	m2m_wifi_set_receive_buffer
 */
NMI_API sint8 m2m_wifi_send_ethernet_pkt(uint8* pu8Packet,uint16 u16PacketSize);

/*!
@fn \
    NMI_API sint8 m2m_wifi_enable_sntp(uint8);

@brief
    Synchronous function to enable/disable the native Simple Network Time Protocol(SNTP) client in the WINC15x0 firmware.

@details
	The SNTP is enabled by default at start-up.The SNTP client at firmware is used to 
	synchronize the system clock to the UTC time from the well known time servers (e.g. "time-c.nist.gov").
	The SNTP client uses a default update cycle of 1 day.
	
	The UTC is important for checking the expiration date of X509 certificates used while establishing
	TLS (Transport Layer Security) connections.

	It is highly recommended to use it if there is no other means to get the UTC time. If there is a RTC
	on the host MCU, the SNTP could be disabled and the host should set the system time to the firmware 
	using the @ref m2m_wifi_set_system_time function.

@param [in]     bEnable
    Enables or disables the SNTP service
		'0' :disable SNTP
		'1' :enable SNTP  

@return        
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	m2m_wifi_set_system_time
 */
NMI_API sint8 m2m_wifi_enable_sntp(uint8 bEnable);

/*!
@fn \
    NMI_API sint8 m2m_wifi_set_system_time(uint32);

@brief
    Function for setting the system time within the WINC IC.

@details
	Synchronous function for setting the system time in time/date format (@ref uint32).
	The @ref tstrSystemTime structure can be used as a reference to the time values that 
    should be set and pass its value as @ref uint32.

@param [in]     u32UTCSeconds
 	Seconds elapsed since January 1, 1900 (NTP Timestamp). 

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	m2m_wifi_enable_sntp
 	tstrSystemTime

@note
	If there is an RTC on the host MCU, the SNTP may be disabled and the host may set the system 
	time within the firmware using the API @ref m2m_wifi_set_system_time.
 */
NMI_API sint8 m2m_wifi_set_system_time(uint32 u32UTCSeconds);

/*!
@fn \
    NMI_API sint8 m2m_wifi_get_system_time(void);

@brief 
    Asynchronous API to obtain the system time in use by the WINC IC.

@details
	Asynchronous function used to retrieve the system time through the use of the 
	response @ref M2M_WIFI_RESP_GET_SYS_TIME.
	Response time retrieved is parsed into the members defined in the 
	structure @ref tstrSystemTime.

@note
	Get the system time from the SNTP client using the API @ref m2m_wifi_get_system_time.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	m2m_wifi_enable_sntp
 			  tstrSystemTime   
 */
NMI_API sint8 m2m_wifi_get_system_time(void);

/*!
@fn \
    NMI_API sint8 m2m_wifi_set_cust_InfoElement(uint8*);

@brief
    API to add or remove a user-defined Information Element 

@details
	Synchronous function to Add/Remove user-defined Information Element to the WIFIBeacon and Probe Response frames while chip mode is Access Point Mode.\n
	According to the information element layout shown bellow, if it is required to set new data for the information elements, pass in the buffer with the
	information according to the sizes and ordering defined bellow. However, if it's required to delete these IEs, fill the buffer with zeros.

@param [in]     pau8M2mCustInfoElement
	Pointer to Buffer containing the IE to be sent. It is the application developer's 
	responsibility to ensure on the correctness  of the information element's ordering 
	passed in. 

@warning
	Size of All elements combined must not exceed 255 byte.
    Used in Access Point Mode.

@note
	IEs Format will be follow the following layout:

@verbatim 
    --------------- ---------- ---------- ------------------- -------- -------- ----------- -----------------------
              | Byte[0]       | Byte[1]  | Byte[2]  | Byte[3:length1+2] | ..... | Byte[n] | Byte[n+1] | Byte[n+2:lengthx+2]  | 
    |---------------|----------|----------|-------------------|-------- --------|-----------|---------------------|
              | #of all Bytes | IE1 ID   | Length1  | Data1(Hex Coded)  | ..... | IEx ID  | Lengthx   | Datax(Hex Coded)     | 
    --------------- ---------- ---------- ------------------- -------- -------- ----------- -----------------------
@endverbatim

@return
     The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	 and a negative value otherwise.

@see
	m2m_wifi_enable_sntp

 \section WIFIExample11 Example
   The example demonstrates how the information elements are set using this function.
@code
            char elementData[21];
            static char state = 0; // To Add, Append, and Delete
            if(0 == state) {	//Add 3 IEs
                state = 1;
                //Total Number of Bytes
                elementData[0]=12;
                //First IE
                elementData[1]=200; elementData[2]=1; elementData[3]='A';
                //Second IE
                elementData[4]=201; elementData[5]=2; elementData[6]='B'; elementData[7]='C';
                //Third IE
                elementData[8]=202; elementData[9]=3; elementData[10]='D'; elementData[11]=0; elementData[12]='F';
            } else if(1 == state) {	
                //Append 2 IEs to others, Notice that we keep old data in array starting with\n
                //element 13 and total number of bytes increased to 20
                state = 2; 
                //Total Number of Bytes
                elementData[0]=20;
                //Fourth IE
                elementData[13]=203; elementData[14]=1; elementData[15]='G';
                //Fifth IE
                elementData[16]=204; elementData[17]=3; elementData[18]='X'; elementData[19]=5; elementData[20]='Z';
            } else if(2 == state) {	//Delete All IEs
                state = 0; 
                //Total Number of Bytes
                elementData[0]=0;
            }
            m2m_wifi_set_cust_InfoElement(elementData);	
@endcode
 */
NMI_API sint8 m2m_wifi_set_cust_InfoElement(uint8* pau8M2mCustInfoElement);

/*!
@fn \
    NMI_API sint8 m2m_wifi_set_power_profile(uint8 u8PwrMode);

@brief
    Change the power profile mode

@param [in]	u8PwrMode
    Change the WINC15x0 power profile to different mode based on the enumeration
    @ref  tenuM2mPwrMode.

@warning
    May only be called after initialization, before any connection request, and may not be used to change 
	the power mode thereafter. 

@return
    The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.

@see
	tenuM2mPwrMode
			m2m_wifi_init
*/
sint8 m2m_wifi_set_power_profile(uint8 u8PwrMode);

/*!
@fn \
    NMI_API sint8 m2m_wifi_set_tx_power(uint8 u8TxPwrLevel);

@brief	
    Set the TX power tenuM2mTxPwrLevel.

@param [in]	u8TxPwrLevel
    Change the TX power based on the enumeration \ref tenuM2mTxPwrLevel.

@pre
	Must be called after the initialization and before any connection request and can't be changed in runtime.

@return
    The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.

@see
	tenuM2mTxPwrLevel
			m2m_wifi_init
*/
sint8 m2m_wifi_set_tx_power(uint8 u8TxPwrLevel);

/*!
@fn \
    NMI_API sint8 m2m_wifi_set_gain_table_idx(uint8 u8GainTableIdx);

@brief
    Set the Gain table index corresponding to specific WiFi region.

@param [in]	u8GainTableIdx
	change the gain table index based on the WiFi region it is suppose to be used.

@pre
	Must be called after the initialization and before any connection request. 
	The corresponding gain tables must be present in the flash.

@return	
	The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.    
	
@see			
	m2m_wifi_init
*/
sint8 m2m_wifi_set_gain_table_idx(uint8 u8GainTableIdx);

/*!
@fn \
    NMI_API sint8 m2m_wifi_enable_firmware_logs(uint8 u8Enable);

@brief
    Enable or Disable logs in run time.

@details
    Enable or Disable logs in run time (Disable Firmware logs will enhance the firmware start-up time 
    and performance).

@param [in]	u8Enable
    Set 1 to enable the logs, 0 for disable.

@pre
    Must be called after initialization through the following function @ref m2m_wifi_init().

@return
    The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.

@see
	__DISABLE_FIRMWARE_LOGS__ (build option to disable logs from initializations)
			m2m_wifi_init
*/
sint8 m2m_wifi_enable_firmware_logs(uint8 u8Enable);

/*!
@fn \
    NMI_API sint8 m2m_wifi_set_battery_voltage(uint8 u8BattVolt);

@brief
    Set the battery voltage to update the firmware calculations.

@pre
    Must be called after initialization through the following function @ref m2m_wifi_init().

@param [in]	dbBattVolt
    Battery Voltage as double.
	
@return
    The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.

@see
	m2m_wifi_init
*/
sint8 m2m_wifi_set_battery_voltage(uint16 u16BattVoltx100);

/*!
@fn \
    sint8 m2m_wifi_set_gains(tstrM2mWifiGainsParams* pstrM2mGain);

@brief		
    Set the chip PPA gain for 11b/11gn.

@param [in]	pstrM2mGain
    @ref tstrM2mWifiGainsParams contain gain parameters as implemented in rf document.

@pre
    Must be called after initialization through the following function @ref m2m_wifi_init.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	m2m_wifi_init
*/
sint8 m2m_wifi_set_gains(tstrM2mWifiGainsParams* pstrM2mGain);

#ifdef ETH_MODE
/*!
@fn \
    NMI_API sint8 m2m_wifi_enable_mac_mcast(uint8 *, uint8);

@brief
	Synchronous function for filtering received MAC addresses

@details
	Synchronous function for filtering received MAC addresses from certain MAC address groups.
	This function allows the addition/removal of certain MAC addresses, used in the multicast filter.

@param [in]     pu8MulticastMacAddress
	Pointer to MAC address

@param [in]     u8AddRemove
	A flag to add or remove the MAC ADDRESS, based on the following values:
	  -  0 : remove MAC address
	  -  1 : add MAC address    

@note
	Maximum number of MAC addresses that could be added is 8.

@warning
	This function is available in ETHERNET/bypass mode ONLY. 
	Make sure that the application defines ETH_MODE.

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	m2m_wifi_set_receive_buffer
	m2m_wifi_send_ethernet_pkt
 */
NMI_API sint8 m2m_wifi_enable_mac_mcast(uint8* pu8MulticastMacAddress, uint8 u8AddRemove);

/*!
@fn \
    NMI_API sint8 m2m_wifi_set_receive_buffer(void *, uint16);
     
@brief
    Synchronous function for setting or modifying the receiver buffer's length.

@details
	Synchronous function for setting or modifying the receiver buffer's length. 
	In the ETHERNET/bypass mode the application should define a callback of type 
	@ref tpfAppEthCb, through which the application handles the received 
	ethernet frames. It is through this callback function that the user can 
	dynamically modify the length of the currently used receiver buffer.

 @param [in]     pvBuffer
	Pointer to Buffer to receive data.
	NULL pointer causes a negative error @ref M2M_ERR_FAIL.

 @param [in]     u16BufferLen
                 Length of data to be received.  Maximum length of data should not exceed the size defined by TCP/IP
      	     defined as @ref SOCKET_BUFFER_MAX_LENGTH
		     
@warning
	This function is available in the Ethernet/bypass mode ONLY. Make sure that the application defines ETH_MODE.\n 

@return
    The function returns @ref M2M_SUCCESS if the command has been successfully queued to the WINC, 
	and a negative value otherwise.

@see
	m2m_wifi_enable_mac_mcast
	m2m_wifi_send_ethernet_pkt	
 */
NMI_API sint8 m2m_wifi_set_receive_buffer(void* pvBuffer,uint16 u16BufferLen);

#endif /* ETH_MODE */

/*!
@fn \
    sint8 m2m_wifi_prng_get_random_bytes(uint8* pu8PrngBuff,uint16 u16PrngSize);

@brief
	Asynchronous function for retrieving from the firmware a pseudo-random set of bytes as specified in the size passed in as a parameter.
	The registered wifi-cb function retrieves the random bytes through the response @ref M2M_WIFI_RESP_GET_PRNG 

@param [in]      pu8PrngBuff
	Pointer to a buffer to receive data.

@param [in]      u16PrngSize
	Request size in bytes  

@warning
	Size greater than the maximum specified (@ref M2M_BUFFER_MAX_SIZE - sizeof(tstrPrng))
	causes a negative error @ref M2M_ERR_FAIL.

@return      
	The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.
 */
sint8 m2m_wifi_prng_get_random_bytes(uint8 * pu8PrngBuff,uint16 u16PrngSize);

/*!
@fn	\
     NMI_API sint8 m2m_wifi_conf_auto_rate(tstrConfAutoRate * pstrConfAutoRate);

@brief
	Configures WLAN automatic TX rate adaptation algorithm.

@details
	Allow the host MCU app to configure auto TX rate selection algorithm. The application can use this 
	API to tweak the algorithm performance. Moreover, it allows the application to force a specific WLAN 
	PHY rate for transmitted data packets to favor range vs. throughput needs.

@param [in]	pstrConfAutoRate
	The Auto rate configuration parameters as listed in tstrConfAutoRate.

@return
	The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.

@see
	tstrConfAutoRate
*/
NMI_API sint8 m2m_wifi_conf_auto_rate(tstrConfAutoRate * pstrConfAutoRate);

/*!
@fn \
    sint8 m2m_wifi_enable_roaming(uint8 bEnableDhcp);

@brief
    Enable WiFi STA roaming.

@details
	m2m_wifi_enable_roaming enables the firmware to trigger the roaming algorithm/steps on link loss with the current AP.
	If roaming is successful, 
        the @ref M2M_WIFI_RESP_CON_STATE_CHANGED message with state as @ref M2M_WIFI_ROAMED is sent to host.
        Additionally a @ref M2M_WIFI_REQ_DHCP_CONF message with new DHCP lease details is sent to host (only if bEnableDhcp=1).
	If roaming is unsuccessful,
        @ref M2M_WIFI_RESP_CON_STATE_CHANGED message with state as @ref M2M_WIFI_DISCONNECTED is sent to host.
	
@param [in]    bEnableDhcp
    0 : Disables DHCP client execution after roaming to new AP
    1 : Enables DHCP client execution after roaming to new AP

@pre
    Must be called after the initialization.
    The roaming algorithm/procedure is internal to the firmware.

@return
    The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.    
    
@see            
    m2m_wifi_init
*/
sint8 m2m_wifi_enable_roaming(uint8 bEnableDhcp);
/*!
@fn \
    sint8 m2m_wifi_disable_roaming(void);

@brief
    Disable WiFi STA roaming.

@pre
    Must be called after the initialization.

@return    
    The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.    
    
@see            
    m2m_wifi_init
*/
sint8 m2m_wifi_disable_roaming(void);

/*!
@fn \
    NMI_API uint8 m2m_wifi_get_state(void);

@brief
    Get the wifi state.

@return
    The function returns the current wifi state (see @ref tenuWifiState for the possible states).

@note
    Check the WINC state. See @ref tenuWifiState for possible states.\n
    @ref WIFI_STATE_INIT state represents WINC initialized but not started, this is a suitable state
    for safe flash access.

@sa
    m2m_wifi_init
    m2m_wifi_download_mode
*/
NMI_API uint8 m2m_wifi_get_state(void);

/**@}*/     //WLANAPI


/** @addtogroup VERSION Version
@brief
        Describes the APIs for reading the version information of the WINC firmware.
@{
    @defgroup   VERSIONDEF  Defines
    @brief
       Specifies the macros and defines used by the version APIs.

    @defgroup   VERSIONAPI  Functions
    @brief
       Lists the APIs for reading the version information of the WINC firmware.
@}
 */

/*!
@ingroup        VERSIONAPI
@fn             NMI_API m2m_wifi_get_firmware_version(tstrM2mRev* pstrRev);
@brief          Get Firmware version info.
@details        Get the Firmware version info from the active partition, as defined in the structure tstrM2mRev.
@param [out]	pstrRev
                    Pointer to the structure tstrM2mRev that contains the firmware version parameters.
@pre            Must be called after initialization through the following function @ref m2m_wifi_init.
@sa       		m2m_wifi_init
@return		The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.
*/
NMI_API sint8 m2m_wifi_get_firmware_version(tstrM2mRev* pstrRev);

#ifdef __cplusplus
}
#endif
#endif /* __M2M_WIFI_H__ */


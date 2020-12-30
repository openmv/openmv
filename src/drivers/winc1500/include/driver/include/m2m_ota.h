/**
 *
 * \file
 *
 * \brief WINC OTA Upgrade API Interface.
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

/**@defgroup OTAAPI OTA
   @brief
        The WINC supports OTA (Over-The-Air) updates. Using the APIs described in this module,
        it is possible to request an ATWINC15x0 to update its firmware, or safely rollback to
        the previous firmware version.\n There are also APIs to download files and store them in
        the WINC's Flash (supported by ATWINC1510 only), which can be used for Host MCU OTA
        updates or accessing information stored remotely.
    @{
        @defgroup   OTACALLBACKS    Callbacks
        @brief
            Lists the different callbacks that can be used during OTA updates.\n
            Callbacks of type @ref tpfOtaNotifCb and @ref tpfOtaUpdateCb should be passed
            onto @ref m2m_ota_init at system initialization. Other callbacks are provided
            to handle the various steps of Host File Download.

        @defgroup   OTADEFINE       Defines
        @brief
            Specifies the macros and defines used by the OTA APIs.

        @defgroup   OTATYPEDEF      Enumerations and Typedefs
        @brief
            Specifies the enums and Data Structures used by the OTA APIs.

        @defgroup   OTAFUNCTIONS    Functions
        @brief
            Lists the full set of available APIs to manage OTA updates and Host File Downloads.
@}
*/

#ifndef __M2M_OTA_H__
#define __M2M_OTA_H__

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#include "common/include/nm_common.h"
#include "driver/include/m2m_types.h"
#include "driver/include/nmdrv.h"

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/**@addtogroup OTACALLBACKS
 */
/**@{*/
/*!
@typedef void (*tpfOtaNotifCb) (tstrOtaUpdateInfo *pstrOtaUpdateInfo);

@brief 	
		A callback to get notification about a potential OTA update.
 
@param[in] pstrOtaUpdateInfo 
		A structure to provide notification payload.

@sa 
	tstrOtaUpdateInfo

@warning 
		The notification is not supported (Not implemented yet)
*/
typedef void (*tpfOtaNotifCb) (tstrOtaUpdateInfo * pstrOtaUpdateInfo);


/*!
@typedef void (*tpfOtaUpdateCb) (uint8 u8OtaUpdateStatusType ,uint8 u8OtaUpdateStatus);

@brief 
   A callback to get OTA status update, the callback provides the status type and its status.\n
   The OTA callback provides the download status, the switch to the downloaded firmware status,
   roll-back status and Host File Download status.
 
@param[in] u8OtaUpdateStatusType
    Possible values are listed in @ref tenuOtaUpdateStatusType.

@param[in] u8OtaUpdateStatus
    Possible values are listed as enumerated by @ref tenuOtaUpdateStatus.

@note
    Executes other callbacks passed to the OTA module.

@see
	tenuOtaUpdateStatusType
	tenuOtaUpdateStatus
 */
typedef void (*tpfOtaUpdateCb) (uint8 u8OtaUpdateStatusType ,uint8 u8OtaUpdateStatus);

/*!
@typedef void (*tpfFileGetCb) (uint8 u8Status, uint8 u8Handler, uint32 u32Size);

@brief 
   A callback to notify the application of the result of the download (success/fail),
   the generated handler ID and the size of the file which has just finished
   downloading (size expressed in bytes).

@param[in] u8Status
    Status of the operation (see @ref tenuOtaUpdateStatus).

@param[in] u8Handler
    Generated handler ID for the new file.

@param[in] u32Size
    Total size of the downloaded file (in bytes).
 
@warning
    The file handler passed onto this callback will be the valid file handler generated
    by the WINC when the download finished successfully. This handler will be required
    for all operations on the file like read and erase.
 */
typedef void (*tpfFileGetCb) (uint8 u8Status, uint8 u8Handler, uint32 u32Size);

/*!
@typedef void (*tpfFileReadCb) (uint8 u8Status, void *pBuff, uint32 u32Size);

@brief 
   A callback to handle a buffer of data after requesting a Host File read.
   The callback will provide the status of the read operation and if successful,
   a pointer to a valid placeholder containing the data read and the amount of
   data available. Such callback is required when using Host File read via the HIF.

@param[in] u8Status
    Status of the operation (see @ref tenuOtaUpdateStatus).

@param[in] pBuff
    Pointer to a placeholder where the data can be retrieved from.

@param[in] u32Size
    Amount of data available after reading (in bytes).

@warning
    After the callback is executed, pBuff will be freed.
 */
typedef void (*tpfFileReadCb) (uint8 u8Status, void *pBuff, uint32 u32Size);
 
/*!
@typedef void (*tpfFileEraseCb) (uint8 u8Status);

@brief 
   A callback executed when the file erase has been completed.

@param[in] u8Status
    Status of the operation (see @ref tenuOtaUpdateStatus).
 */
typedef void (*tpfFileEraseCb) (uint8 u8Status);
/**@}*/     //OTACALLBACKS
 
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
FUNCTION PROTOTYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/** @addtogroup OTAFUNCTIONS
 */
#ifdef __cplusplus
     extern "C" {
#endif

 /**@{*/
/*!
@fn	\
	NMI_API sint8  m2m_ota_init(tpfOtaUpdateCb  pfOtaUpdateCb,tpfOtaNotifCb  pfOtaNotifCb)

@brief
    Synchronous initialization function for the OTA layer by registering the update callback.\n
    The notification callback is not supported at the current version. Calling this API is a
    MUST for all the OTA API's.

@param [in]	pfOtaUpdateCb
    OTA Update callback function.
				
@param [in]	pfOtaNotifCb
    OTA Notify callback function.

@return		
	The function returns @ref M2M_SUCCESS for successful operations  and a negative value otherwise.
*/
NMI_API sint8  m2m_ota_init(tpfOtaUpdateCb  pfOtaUpdateCb,tpfOtaNotifCb  pfOtaNotifCb);

/*!
@fn	\
	NMI_API sint8  m2m_ota_notif_set_url(uint8 * u8Url);

@brief
    Set the OTA notification server URL, the functions need to be called before any check for update.\n
    This functionality is not supported by WINC firmware.

@param [in]	u8Url
			 Set the OTA notification server URL, the functions need to be called before any check for update.

@pre
    Prior calling of @ref m2m_ota_init is required.

@warning 
    Notification Server is not supported in the current version (function is not implemented).

@see    
			m2m_ota_init			

@return		
	The function returns @ref M2M_SUCCESS for successful operations  and a negative value otherwise.
*/
NMI_API sint8  m2m_ota_notif_set_url(uint8 * u8Url);

/*!
@fn	\
	NMI_API sint8  m2m_ota_notif_check_for_update(void);

@brief
    Synchronous function to check for the OTA update using the Notification Server URL.\n
    Function is not implemented (not supported at the current version).

@warning 
    Function is not implemented (not supported at the current version).
		
@sa   
			m2m_ota_init
			m2m_ota_notif_set_url

@return		
	The function returns @ref M2M_SUCCESS for successful operations  and a negative value otherwise.
*/
NMI_API sint8  m2m_ota_notif_check_for_update(void);

/*!
@fn	\
	NMI_API sint8 m2m_ota_notif_sched(uint32 u32Period);

@brief
    Schedule OTA notification Server check for update request after specific number of days.\n
    Function is not implemented (not supported at the current version).

@param [in]	u32Period
			Period in days

@warning
    Function is not implemented (not supported at the current version).

@sa 
		m2m_ota_init
		m2m_ota_notif_check_for_update
		m2m_ota_notif_set_url

@return		
	The function returns @ref M2M_SUCCESS for successful operations  and a negative value otherwise.
*/
NMI_API sint8 m2m_ota_notif_sched(uint32 u32Period);

/*!
@fn	\
    NMI_API sint8 m2m_ota_start_update(unsigned char * pcDownloadUrl);

@brief
    Request OTA start update using the download URL, the OTA module will download the OTA image, ensure integrity of the image
    and update the validity of the image in the control structure. On completion, a callback of type @ref tpfOtaUpdateCb is called
    (callback previously provided via m2m_ota_init).

@param [in] pcDownloadUrl
    The download firmware URL, according to the application server.

@warning
    Calling this API does not guarantee OTA WINC image update, it depends on the connection with the
    download server and the validity of the image.\n
    Calling this API invalidates any previous valid rollback image. When the OTA succeeds, the current
    image will become the rollback image after @ref m2m_ota_switch_firmware.
				
@pre
    @ref m2m_ota_init is a prerequisite and must have been called before using @ref m2m_ota_start_update().\n
    Switching to the newly downloaded image requires calling @ref m2m_ota_switch_firmware API.

@sa
    @ref m2m_ota_init
    @ref m2m_ota_switch_firmware
    @ref tpfOtaUpdateCb
		
@return		
	The function returns @ref M2M_SUCCESS for successful operations  and a negative value otherwise.

\section OTAExample Example
        This example shows how an OTA image update and switch is carried out.
        It demonstrates use of the following OTA APIs:
            @ref m2m_ota_init
            @ref tpfOtaUpdateCb
            @ref m2m_ota_start_update
            @ref m2m_ota_switch_firmware
            @ref m2m_ota_rollback

@code
static void OtaUpdateCb(uint8 u8OtaUpdateStatusType ,uint8 u8OtaUpdateStatus)
{
	if(u8OtaUpdateStatusType == DL_STATUS) {
		if(u8OtaUpdateStatus == OTA_STATUS_SUCCESS) {
			//switch to the upgraded firmware
			m2m_ota_switch_firmware();
		}
	}
	else if(u8OtaUpdateStatusType == SW_STATUS) {
		if(u8OtaUpdateStatus == OTA_STATUS_SUCCESS) {
			M2M_INFO("Now OTA successfully done");
			//start the host SW upgrade then system reset is required (Reinitialize the driver)
		}
	}
}

void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
{
	case M2M_WIFI_REQ_DHCP_CONF:
	{
		//after successfully connection, start the over air upgrade
		m2m_ota_start_update(OTA_URL);
	}
	break;
	default:
	break;
}

int main (void)
{
    sint8 s8Ret;
	tstrWifiInitParam param;
	tstr1xAuthCredentials gstrCred1x    = AUTH_CREDENTIALS;

	nm_bsp_init();
	
	m2m_memset((uint8*)&param, 0, sizeof(param));
	param.pfAppWifiCb = wifi_event_cb;
	
	//Initialize the WINC Driver
    s8Ret = m2m_wifi_init(&param);
    if (M2M_SUCCESS != s8Ret)
	{
        M2M_ERR("Driver Init Failed <%d>\n",s8Ret);
		while(1);
	}
	//Initialize the OTA module
	m2m_ota_init(OtaUpdateCb,NULL);

	//connect to AP that provide connection to the OTA server
	m2m_wifi_default_connect();

	while(1)
	{
		//Handle the app state machine plus the WINC event handler                                                                     
		while(m2m_wifi_handle_events(NULL) != M2M_SUCCESS) {
			
		}
	}
}
@endcode		
*/
NMI_API sint8 m2m_ota_start_update(unsigned char * pcDownloadUrl);

/*!
@fn	\
    NMI_API sint8 m2m_ota_rollback(void);

@brief
    Request OTA Roll-back to the old (inactive) WINC image, the WINC firmware will check the validity of the Roll-back image
    and activate it if valid. On completion, a callback of type tpfOtaUpdateCb is called (application must previously have
    provided the callback via m2m_ota_init). If the callback indicates successful activation, the newly-activated image
    will start running after next system reset.

@warning
    If rollback requires a host driver update in order to maintain HIF compatibility (HIF
    major value change), then it is recommended to update the host driver prior to calling this
    API.\n
    In the event of system reset with incompatible driver/firmware, compatibility can be
    recovered by calling @ref m2m_ota_rollback or @ref m2m_ota_switch_firmware.

@sa 
	m2m_ota_init
	m2m_ota_start_update	

@return		
	The function returns @ref M2M_SUCCESS for successful operations  and a negative value otherwise.
*/
NMI_API sint8 m2m_ota_rollback(void);

/*!
@fn	\
    NMI_API sint8 m2m_ota_abort(void);

@brief
    Request the WINC to abort an OTA or Host File download in progress.\n
	If no download is in progress, the API will respond with failure.

@return
	The function returns @ref M2M_SUCCESS for successful operation and a negative value otherwise.
*/
NMI_API sint8 m2m_ota_abort(void);

/*!
@fn	\
	NMI_API sint8 m2m_ota_switch_firmware(void);

@brief
    Request switch to the updated WINC image. The WINC firmware will check the validity of the
    inactive image and activate it if it is valid. On completion, a callback of type @ref tpfOtaUpdateCb
    is called (application must previously have provided the callback via @ref m2m_ota_init).
    If the callback indicates successful activation, the newly-activated image will start running
    after next system reset.

@warning
    If switch will necessitate a host driver update in order to maintain HIF compatibility (HIF
    major value change), then it is recommended to update the host driver prior to calling this
    API.\n
    In the event of system reset with incompatible driver/firmware, compatibility can be
    recovered by calling @ref m2m_ota_rollback or @ref m2m_ota_switch_firmware.

@sa 
	m2m_ota_init
	m2m_ota_start_update

@return		
	The function returns @ref M2M_SUCCESS for successful operations  and a negative value otherwise.
*/
NMI_API sint8 m2m_ota_switch_firmware(void);

/*!
@fn	\
    NMI_API sint8 m2m_ota_host_file_get(unsigned char *pcDownloadUrl, tpfFileGetCb pfHFDGetCb);

@brief
    Download a file from a remote location and store it in WINC's Flash.

@param[in]  pcDownloadUrl
                Url pointing to the remote file. HTTP/HTTPS only.

@param[in]  pfHFDGetCb
    Pointer to a callback (see @ref tpfFileGetCb) to be executed when the download finishes.

@return
    The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.

@pre
    Requires @ref m2m_ota_init to be called before a download can start.

@warning
    This functionality is only supported from WINC release 19.6.1 onwards.
    The maximum file size that can be stored in WINC1510 is 508KB, the
    WINC1500 variant is not supported for Host File Download.\n
    Concurrent use of Host File Get and WINC OTA is not possible.\n
    Providing a callback is mandatory.

@sa
    m2m_ota_init
    tpfFileGetCb
*/
NMI_API sint8 m2m_ota_host_file_get(unsigned char *pcDownloadUrl, tpfFileGetCb pfHFDGetCb);

/*!
@fn	\
    NMI_API sint8 m2m_ota_host_file_read_hif(uint8 u8Handler, uint32 u32Offset, uint32 u32Size, tpfFileReadCb pfHFDReadCb);

@brief
    Read a certain amount of bytes from a file previously stored in WINC's Flash using HIF transfer.

@param[in]  u8Handler
    Handler of the file we are trying to read from. Must be valid.

@param[in]  u32Offset
                Offset from start of the file to read from (in bytes).

@param[in]  u32Size
                The amount of data to read (in bytes).

@param[in]  pfHFDReadCb
    Callback (see @ref tpfFileReadCb) to be executed when the read operation completes.

@return
    The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.

@pre
    Requires @ref m2m_ota_init to be called before a read via HIF can be requested.

@warning
    There is a limitation on how much data can be transferred at a time using hif read, which is 128 bytes.
    The limitation described above can potentially reduce the speed of the read due to extra overhead, but
    using the HIF is non-blocking and therefore the Application can continue execution as normal, being
    interrupted only when data is available. Another advantage is that it does not require the WINC to be
    reset or put in download mode, as it is the case for reading the file via SPI (see @ref m2m_ota_host_file_read_spi).\n
    A valid file handler must be provided, this means that it needs to match the handler internally stored
    by the WINC and must not be @ref HFD_INVALID_HANDLER.\n
    Providing a callback is mandatory.

@note
    When calling this API while specifying a size > 128 bytes, the read will be limited to the first 128 bytes
    starting at the read offset. It it recommended that a read for sizes above 128 bytes is performed in
    multiple steps, using the callback to advance the offset and request another read of 128 bytes (or less) each time.

@sa
    m2m_ota_init
    m2m_ota_host_file_get
    tpfFileReadCb
*/
NMI_API sint8 m2m_ota_host_file_read_hif(uint8 u8Handler, uint32 u32Offset, uint32 u32Size, tpfFileReadCb pfHFDReadCb);

/*!
@fn	\
    NMI_API sint8 m2m_ota_host_file_read_spi(uint8 u8Handler, uint8 *pu8Buff, uint32 u32Offset, uint32 u32Size);

@brief
    Read a certain amount of bytes from a file in WINC's Flash using SPI transfer.

@param[in]  u8Handler
    Handler of the file we are trying to read from. Must be valid.

@param[in]  pu8Buff
    Pointer to a buffer to store the data being read. Must not be NULL.

@param[in]  u32Offset
                Offset from start of the file to read from (in bytes).

@param[in]  u32Size
                The amount of data to read (in Bytes).

@return
    The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.

@warning
    Reading of a file via SPI can be much faster than by reading it via the HIF. However, the read will
    be blocking and it will require the WINC to be put into download mode prior to the read, the download
    mode means that the WINC will act as Flash device and not as a Wifi device. So, before using
    m2m_ota_host_file_read_spi, the Application should call @ref m2m_wifi_download_mode before trying to
    read. After the read finishes, the WINC needs to be reset (see @ref m2m_wifi_reinit).\n
    A valid file handler must be provided, this means that it needs to match the handler internally stored
    by the WINC and must not be @ref HFD_INVALID_HANDLER.

@sa
    m2m_ota_init
    m2m_ota_host_file_get

\section Host File Download SPI Read Example
The following is an example of how to perform a read file from the WINC via SPI.
@code
typedef struct {
    uint8 u8Handler;
    uint32 u32Offset;
    uint32 u32Size;
    uint8 au8Buff[200];
}FileDescriptor;

tstrWifiInitParam gstrWifiParam;
static FileDescriptor gstrAppFile;

char *acURL = "http://www.microchip.com/_images/ics/medium-ATWINC1500-MODULE-28.png";

static void ReadFileSPI(void);
static void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg);
static void FileGetCallback(uint8 u8Status, uint8 u8Handler, uint32 u32Size);
static void OtaUpdateCb(uint8 u8OtaUpdateStatusType ,uint8 u8OtaUpdateStatus);

static void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg)
{
    case M2M_WIFI_REQ_DHCP_CONF:
    {
        // After successfully connection, start the File Download
        gstrAppFile.u32Offset = 0;
        s8Ret = m2m_ota_host_file_get(acURL, FileGetCallback);
        if(s8Ret != M2M_SUCCESS)
        {
            M2M_ERR("File Download Failed!\n");
        }
    }
    break;
    default:
    break;
}

static void OtaUpdateCb(uint8 u8OtaUpdateStatusType ,uint8 u8OtaUpdateStatus)
{
    M2M_INFO("%d %d\n",u8OtaUpdateStatusType,u8OtaUpdateStatus);

    if(u8OtaUpdateStatus == OTA_STATUS_SUCCESS)
    {
        if(u8OtaUpdateStatusType == HFD_STATUS)
        {
            // Read the file and process it
            ReadFileSPI();
        }
    }
}

static void FileGetCallback(uint8 u8Status, uint8 u8Handler, uint32 u32Size)
{
    if(OTA_STATUS_SUCCESS == u8Status)
    {
        gstrAppFile.u8Handler = u8Handler;
        gstrAppFile.u32Size   = u32Size;
        // File Get Successful
    }
    else
    {
        M2M_ERR("File Get Failed!\n");
        // File Get Failed
    }
}

static void ReadFileSPI(void)
{
    sint8 s8Ret = M2M_ERR_FAIL;
    
    if(WIFI_STATE_DEINIT != m2m_wifi_get_state())
        m2m_wifi_deinit(NULL);

    s8Ret = m2m_wifi_download_mode();
    if(M2M_SUCCESS != s8Ret) goto EXIT;

    // gstrAppFile.u32Offset can be changed to define a starting point for the read,
    // in which case the size of the requested read should be adjusted to accommodate for this.
    // This call assumes that m2m_ota_host_file_get was called earlier, in this example it is fine
    // since ReadFileSPI is only called from the within OtaUpdateCb
    // This example simply reads the first 200 bytes of the file.
    uint32 u32AmountToRead = 200;
    s8Ret = m2m_ota_host_file_read_spi(gstrAppFile.u8Handler, gstrAppFile.au8Buff, gstrAppFile.u32Offset, u32AmountToRead);

    if(M2M_SUCCESS == s8Ret)
        M2M_INFO("\nFile Read completed, Offset: %lu, Size of Read: %lu.\n", gstrAppFile.u32Offset, u32AmountToRead);

    // *** Do something with the contents of gstrAppFile.au8Buff ***

    s8Ret = m2m_wifi_reinit(&gstrWifiParam);
    if(M2M_SUCCESS != s8Ret) goto EXIT;

    // Initialize the OTA again and reconnect to the previously connected SSID
    m2m_ota_init(OtaUpdateCb, NULL);
    m2m_wifi_default_connect();

EXIT:
    return;
}

void main(void)
{
    nm_bsp_init();

    m2m_memset((uint8*)&gstrWifiParam, 0, sizeof(gstrWifiParam));
    gstrWifiParam.pfAppWifiCb = wifi_event_cb;

    // Initialize the WINC Driver
    sint8 s8Ret = m2m_wifi_init(&gstrWifiParam);
    if (M2M_SUCCESS != s8Ret)
    {
        M2M_ERR("Driver Init Failed <%d>\n",s8Ret);
        while(1);
    }

    // Initialize the OTA module
    m2m_ota_init(OtaUpdateCb, NULL);

    // *** Connect to a wifi network by calling m2m_wifi_connect() ***

    while(1) m2m_wifi_handle_events(NULL);
}
@endcode
*/
NMI_API sint8 m2m_ota_host_file_read_spi(uint8 u8Handler, uint8* pu8Buff, uint32 u32Offset, uint32 u32Size);

/*!
@fn	\
    NMI_API sint8 m2m_ota_host_file_erase(uint8 u8Handler, tpfFileEraseCb pfHFDEraseCb);

@brief
    Erase any traces of file stored in WINC's Flash.

@param[in]  u8Handler
    Handler of the file we are trying to erase. Must be valid.

@param[in]  pfHFDEraseCb
    Pointer to callback (see @ref tpfFileEraseCb) to execute when the file erase is completed by the WINC.

@return
    The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.

@pre
    In order to execute the callback, @ref m2m_ota_init must be called before requesting the erase.

@note
    Providing a callback is optional.
    If the current handler is invalid at this point, it means one of the three:
        1. The file never existed;
        2. The file has already been already deleted;
        3. The request to get the file hasn't fully completed.
    \par
    For 1. and 2. there is no need to signal the WINC to erase the file in Flash.\n
    For 3. the Flash can't be erased while a file download is ongoing.

@warning
    A valid file handler must be provided, this means that it needs to match the handler internally stored
    by the WINC and must not be @ref HFD_INVALID_HANDLER.\n
    The handlers will be destroyed regardless of the call returning success or not.

@sa
    m2m_ota_init
    m2m_ota_host_file_get
*/
NMI_API sint8 m2m_ota_host_file_erase(uint8 u8Handler, tpfFileEraseCb pfHFDEraseCb);

#if 0
NMI_API sint8 m2m_ota_test(void);
#endif
/**@}*/     //OTAFUNCTIONS

/*!
@ingroup        VERSIONAPI
@fn             NMI_API sint8 m2m_ota_get_firmware_version(tstrM2mRev* pstrRev);
@brief          Get the OTA Firmware version.
@details        Get OTA Firmware version info from the inactive partition, as defined in the structure tstrM2mRev.
@param [out]    pstrRev
                    Pointer to the structure tstrM2mRev that contains the firmware version parameters.
@return         The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.
*/
NMI_API sint8 m2m_ota_get_firmware_version(tstrM2mRev* pstrRev);

#ifdef __cplusplus
}
#endif
#endif /* __M2M_OTA_H__ */

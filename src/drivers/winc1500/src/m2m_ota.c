/**
 *
 * \file
 *
 * \brief NMC1500 IoT OTA Interface.
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



/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
#include <stdbool.h>
#include "common/include/nm_common.h"
#include "driver/include/m2m_types.h"
#include "driver/include/m2m_ota.h"
#include "driver/include/m2m_hif.h"
#include "spi_flash/include/spi_flash.h"
#include "driver/include/m2m_wifi.h"
#include "spi_flash/include/flexible_flash.h"
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
DATA TYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
static tpfOtaUpdateCb gpfOtaUpdateCb = NULL;
static tpfOtaNotifCb  gpfOtaNotifCb = NULL;
static tpfFileGetCb   gpfHFDGetCb    = NULL;
static tpfFileReadCb  gpfHFDReadCb   = NULL;
static tpfFileEraseCb gpfHFDEraseCb  = NULL;

typedef struct {
    uint32 u32Offset;
    uint32 u32Size;
}FileBlockDescriptor;

static FileBlockDescriptor FileBlock;

static uint8  gu8CurrFileHandlerID  = HFD_INVALID_HANDLER;

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
FUNCTION PROTOTYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/**
@fn         m2m_ota_cb(uint8 u8OpCode, uint16 u16DataSize, uint32 u32Addr)
@brief      Internal OTA call back function.
@param[in]  u8OpCode
                HIF Opcode type.
@param[in]  u16DataSize
                HIF data length.
@param[in]  u32Addr
                HIF address.
*/
static void m2m_ota_cb(uint8 u8OpCode, uint16 u16DataSize, uint32 u32Addr)
{
	sint8 s8Ret = M2M_SUCCESS;
	if(u8OpCode == M2M_OTA_RESP_NOTIF_UPDATE_INFO)
	{
		tstrOtaUpdateInfo strOtaUpdateInfo;
		m2m_memset((uint8*)&strOtaUpdateInfo,0,sizeof(tstrOtaUpdateInfo));
		s8Ret = hif_receive(u32Addr,(uint8*)&strOtaUpdateInfo,sizeof(tstrOtaUpdateInfo),0);
		if(s8Ret == M2M_SUCCESS)
		{
			if(gpfOtaNotifCb)
				gpfOtaNotifCb(&strOtaUpdateInfo);
		}
	}
	else if (u8OpCode == M2M_OTA_RESP_UPDATE_STATUS)
	{
		tstrOtaUpdateStatusResp strOtaUpdateStatusResp;
		m2m_memset((uint8*)&strOtaUpdateStatusResp,0,sizeof(tstrOtaUpdateStatusResp));
		s8Ret = hif_receive(u32Addr, (uint8*) &strOtaUpdateStatusResp,sizeof(tstrOtaUpdateStatusResp), 0);
		if(s8Ret == M2M_SUCCESS)
		{
			if(gpfOtaUpdateCb)
				gpfOtaUpdateCb(strOtaUpdateStatusResp.u8OtaUpdateStatusType,strOtaUpdateStatusResp.u8OtaUpdateStatus);
		}
	}
    else if (u8OpCode == M2M_OTA_RESP_HOST_FILE_STATUS)
    {
        tstrOtaHostFileGetStatusResp strOtaHostFileGetStatusResp = {0};
        s8Ret = hif_receive(u32Addr, (uint8*)&strOtaHostFileGetStatusResp, sizeof(tstrOtaHostFileGetStatusResp), 1);
        if(M2M_SUCCESS == s8Ret)
        {
            if(strOtaHostFileGetStatusResp.u8OtaFileGetStatus == OTA_STATUS_SUCCESS) {
                gu8CurrFileHandlerID = strOtaHostFileGetStatusResp.u8CFHandler;
            }
        }
    }
    else if (u8OpCode == M2M_OTA_RESP_HOST_FILE_DOWNLOAD)
    {
        tstrOtaHostFileGetStatusResp strOtaHostFileGetStatusResp = {0};
        s8Ret = hif_receive(u32Addr, (uint8*)&strOtaHostFileGetStatusResp, sizeof(tstrOtaHostFileGetStatusResp), 1);
        if(M2M_SUCCESS == s8Ret)
        {
            if(strOtaHostFileGetStatusResp.u8OtaFileGetStatus == OTA_STATUS_SUCCESS) {
                gu8CurrFileHandlerID = strOtaHostFileGetStatusResp.u8CFHandler;
                M2M_INFO("Generated HostFileHandlerID is %u\n", gu8CurrFileHandlerID);
            }

            if(gpfHFDGetCb) {
                gpfHFDGetCb(strOtaHostFileGetStatusResp.u8OtaFileGetStatus, gu8CurrFileHandlerID, strOtaHostFileGetStatusResp.u32OtaFileSize);
                gpfHFDGetCb = NULL;
            }
        }
    }
    else if (u8OpCode == M2M_OTA_RESP_HOST_FILE_READ)
    {
        tstrOtaHostFileReadStatusResp strOtaHostFileReadStatusResp;
        m2m_memset((uint8*)&strOtaHostFileReadStatusResp, 0, sizeof(tstrOtaHostFileReadStatusResp));
        s8Ret = hif_receive(u32Addr, (uint8*)&strOtaHostFileReadStatusResp, sizeof(tstrOtaHostFileReadStatusResp), 1);
        if(M2M_SUCCESS == s8Ret)
            if(gpfHFDReadCb)
                gpfHFDReadCb(strOtaHostFileReadStatusResp.u8OtaFileReadStatus, strOtaHostFileReadStatusResp.pFileBuf, strOtaHostFileReadStatusResp.u16FileBlockSz);
    }
    else if(u8OpCode == M2M_OTA_RESP_HOST_FILE_ERASE)
    {
        tstrOtaHostFileEraseStatusResp strOtaHostFileEraseStatusResp = {0};
        s8Ret = hif_receive(u32Addr, (uint8*)&strOtaHostFileEraseStatusResp, sizeof(tstrOtaHostFileEraseStatusResp), 1);
        if(M2M_SUCCESS == s8Ret)
        {
            if(gpfHFDEraseCb)
            {
                gpfHFDEraseCb(strOtaHostFileEraseStatusResp.u8OtaFileEraseStatus);
                gpfHFDEraseCb = NULL;
            }
        }
    }
	else
	{
		M2M_ERR("Invalid OTA resp %d ?\n",u8OpCode);
	}
}
/*!
@fn         NMI_API sint8  m2m_ota_init(tpfOtaUpdateCb pfOtaUpdateCb, tpfOtaNotifCb pfOtaNotifCb)
@brief      Initialize the OTA layer.
@param[in]  pfOtaUpdateCb
                OTA Update callback function.
@param[in]  pfOtaNotifCb
                OTA Notify callback function.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8  m2m_ota_init(tpfOtaUpdateCb pfOtaUpdateCb, tpfOtaNotifCb pfOtaNotifCb)
{
	sint8 ret = M2M_SUCCESS;

	if(pfOtaUpdateCb){
		gpfOtaUpdateCb = pfOtaUpdateCb;
	}else{
		M2M_ERR("Invalid Ota update cb\n");
	}
	if(pfOtaNotifCb){
		gpfOtaNotifCb = pfOtaNotifCb;
	}else{
		M2M_ERR("Invalid Ota notify cb\n");
	}

	hif_register_cb(M2M_REQ_GROUP_OTA,m2m_ota_cb);
	ret = hif_send(M2M_REQ_GROUP_OTA, M2M_OTA_REQ_HOST_FILE_STATUS, NULL, 0, NULL, 0, 0);

	return ret;
}
/*!
@fn         NMI_API sint8  m2m_ota_notif_set_url(uint8 * u8Url)
@brief      Set the OTA url.
@param[in]  u8Url
                The url server address.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8  m2m_ota_notif_set_url(uint8 * u8Url)
{
	sint8 ret = M2M_SUCCESS;
	uint16 u16UrlSize = m2m_strlen(u8Url) + 1;
	/*Todo: we may change it to data pkt but we need to give it higher priority
			but the priority is not implemented yet in data pkt
	*/
	ret = hif_send(M2M_REQ_GROUP_OTA,M2M_OTA_REQ_NOTIF_SET_URL,u8Url,u16UrlSize,NULL,0,0);
	return ret;

}

/*!
@fn         NMI_API sint8  m2m_ota_notif_check_for_update(void)
@brief      Check for OTA update.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8  m2m_ota_notif_check_for_update(void)
{
	sint8 ret = M2M_SUCCESS;
	ret = hif_send(M2M_REQ_GROUP_OTA,M2M_OTA_REQ_NOTIF_CHECK_FOR_UPDATE,NULL,0,NULL,0,0);
	return ret;
}

/*!
@fn         NMI_API sint8 m2m_ota_notif_sched(uint32 u32Period)
@brief      Schedule OTA update.
@param[in]  u32Period
                Period in days
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ota_notif_sched(uint32 u32Period)
{
	sint8 ret = M2M_SUCCESS;
	ret = hif_send(M2M_REQ_GROUP_OTA,M2M_OTA_REQ_NOTIF_CHECK_FOR_UPDATE,NULL,0,NULL,0,0);
	return ret;
}

/*!
@fn         NMI_API sint8 m2m_ota_start_update(unsigned char * pcDownloadUrl)
@brief      Request OTA start update using the downloaded URL.
@param[in]  pcDownloadUrl
                The download firmware URL, you get it from device info.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ota_start_update(unsigned char * pcDownloadUrl)
{
	sint8 ret = M2M_SUCCESS;
	uint16 u16DurlSize = m2m_strlen(pcDownloadUrl) + 1;
	/*Todo: we may change it to data pkt but we need to give it higher priority
			but the priority is not implemented yet in data pkt
	*/
	ret = hif_send(M2M_REQ_GROUP_OTA,M2M_OTA_REQ_START_FW_UPDATE,pcDownloadUrl,u16DurlSize,NULL,0,0);
	return ret;
}

/*!
@fn         NMI_API sint8 m2m_ota_rollback(void)
@brief      Request OTA Rollback image.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ota_rollback(void)
{
	sint8 ret = M2M_SUCCESS;
	ret = hif_send(M2M_REQ_GROUP_OTA,M2M_OTA_REQ_ROLLBACK_FW,NULL,0,NULL,0,0);
	return ret;
}

/*!
@fn         NMI_API sint8 m2m_ota_abort(void)
@brief      Request OTA Abort.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ota_abort(void)
{
	sint8 ret = M2M_SUCCESS;
	ret = hif_send(M2M_REQ_GROUP_OTA,M2M_OTA_REQ_ABORT,NULL,0,NULL,0,0);
	return ret;
}

/*!
@fn         NMI_API sint8 m2m_ota_switch_firmware(void)
@brief      Switch to the upgraded Firmware.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ota_switch_firmware(void)
{
	sint8 ret = M2M_SUCCESS;
	ret = hif_send(M2M_REQ_GROUP_OTA,M2M_OTA_REQ_SWITCH_FIRMWARE,NULL,0,NULL,0,0);
	return ret;
}

/*!
@fn         NMI_API sint8 m2m_ota_get_firmware_version(tstrM2mRev * pstrRev)
@brief      Get the OTA Firmware version.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ota_get_firmware_version(tstrM2mRev * pstrRev)
{
	sint8 ret = M2M_SUCCESS;
	ret = hif_chip_wake();
	if(ret == M2M_SUCCESS)
	{
    	ret = nm_get_ota_firmware_info(pstrRev);
		hif_chip_sleep();
	}
	return ret;
}
#if 0
#define M2M_OTA_FILE	"../../../m2m_ota.dat"
NMI_API sint8 m2m_ota_test(void)
{
	uint32 page  = 0;
	uint8 buffer[1500];
	uint32 u32Sz = 0;
	sint8 ret = M2M_SUCCESS;
	FILE *fp =NULL;
	fp = fopen(M2M_OTA_FILE,"rb");
	if(fp)
	{
		fseek(fp, 0L, SEEK_END);
		u32Sz = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		while(u32Sz > 0)
		{
			{
				page = (rand()%1400);

				if((page<100)||(page>1400)) page  = 1400;
			}

			if(u32Sz>page)
			{
				u32Sz-=page;
			}
			else
			{
				page = u32Sz;
				u32Sz = 0;
			}
			printf("page %d\n", (int)page);
			fread(buffer,page,1,fp);
			ret = hif_send(M2M_REQ_GROUP_OTA,M2M_OTA_REQ_TEST|M2M_REQ_DATA_PKT,NULL,0,(uint8*)&buffer,page,0);
			if(ret != M2M_SUCCESS)
			{
				M2M_ERR("\n");
			}
			nm_bsp_sleep(1);
		}

	}
	else
	{
		M2M_ERR("nO err\n");
	}
	return ret;
}
#endif

/*!
@fn         NMI_API m2m_ota_host_file_get(unsigned char *pcDownloadUrl, tpfFileGetCb pfHFDGetCb)
@brief      Download a file from a remote location and store it in the WINC's Flash.
@param[in]  pcDownloadUrl
                Url pointing to the remote file. HTTP/HTTPS only.

@param[in]  pfHFDGetCb
                Pointer to a callback to be executed when the download finishes.
@return     Status of the get operation.
@warning    Providing a callback is mandatory.
*/
NMI_API sint8 m2m_ota_host_file_get(unsigned char *pcDownloadUrl, tpfFileGetCb pfHFDGetCb)
{
    sint8 s8Ret = M2M_ERR_FAIL;
    uint16 u16DUrlSize = m2m_strlen(pcDownloadUrl);

    if((NULL == pfHFDGetCb) || (0 == u16DUrlSize))
    {
        M2M_ERR("Invalid parameters.\n");
        goto EXIT;
    }

    if('\0' != pcDownloadUrl[u16DUrlSize])
        pcDownloadUrl[u16DUrlSize] = '\0';
    else
        u16DUrlSize++;

    M2M_INFO("GetHostFile - URL: %s, urlSize: %u\n", pcDownloadUrl, u16DUrlSize);

    s8Ret = hif_send(M2M_REQ_GROUP_OTA, M2M_OTA_REQ_HOST_FILE_DOWNLOAD, pcDownloadUrl, u16DUrlSize, NULL, 0, 0);
    if(s8Ret == M2M_SUCCESS)
    {
        gpfHFDGetCb = pfHFDGetCb;
        gu8CurrFileHandlerID = HFD_INVALID_HANDLER;
    }

EXIT:
    return s8Ret;
}

/*!
@fn         NMI_API m2m_ota_host_file_read_hif(uint8 u8Handler, uint32 u32Offset, uint32 u32Size, tpfFileReadCb pfHFDReadCb)
@brief      Read a certain amount of bytes from a file in WINC's Flash using HIF transfer.
@param[in]  u8Handler
                ID of the file we are trying to read from. Must be valid.

@param[in]  u32Offset
                Offset from start of the file to read from (in bytes).

@param[in]  u32Size
                The amount of data to read (in bytes).

@param[in]  pfHFDReadCb
                Callback to be executed when the read operation completes.
@return     Status of the read operation.
@warning    Providing a callback is mandatory.
*/
NMI_API sint8 m2m_ota_host_file_read_hif(uint8 u8Handler, uint32 u32Offset, uint32 u32Size, tpfFileReadCb pfHFDReadCb)
{
    sint8 s8Ret         = M2M_ERR_INVALID_ARG;
    FileBlock.u32Offset = u32Offset;
    FileBlock.u32Size   = u32Size;

    if((u8Handler != gu8CurrFileHandlerID) || (HFD_INVALID_HANDLER == gu8CurrFileHandlerID) || (NULL == pfHFDReadCb)) goto EXIT; 
    s8Ret = hif_send(M2M_REQ_GROUP_OTA, M2M_OTA_REQ_HOST_FILE_READ, (uint8 *) &FileBlock, sizeof(FileBlockDescriptor), NULL, 0, 0);

    if(M2M_SUCCESS == s8Ret)
        gpfHFDReadCb = pfHFDReadCb;
EXIT:
    return s8Ret;
}

/*!
@fn         NMI_API m2m_ota_host_file_read_spi(uint8 u8Handler, uint8 *pu8Buff, uint32 u32Offset, uint32 u32Size)
@brief      Read a certain amount of bytes from a file in WINC's Flash using SPI transfer.
@param[in]  u8Handler
                ID of the file we are trying to read from. Must be valid.

@param[in]  pu8Buff
                Pointer to a buffer to store the data being read. Must be valid.

@param[in]  u32Offset
                Offset from start of the file to read from (in bytes).

@param[in]  u32Size
                The amount of data to read (in Bytes).
@return     Status of the read operation.
@warning    Before using m2m_ota_host_file_read_spi, the WINC needs to be put in a special
            mode to allow for a safe access to the Flash. This can be done by calling
            @ref m2m_wifi_download_mode or @ref m2m_wifi_reinit_hold before trying to read.
*/
NMI_API sint8 m2m_ota_host_file_read_spi(uint8 u8Handler, uint8 *pu8Buff, uint32 u32Offset, uint32 u32Size)
{
    static uint32 u32FlashHFDStart = 0;
    static uint32 u32FlashHFDSize  = 0;
    sint8 s8Ret = M2M_ERR_INVALID_ARG;
    if((u8Handler == HFD_INVALID_HANDLER) || (NULL == pu8Buff)) goto EXIT;

    if(WIFI_STATE_INIT != m2m_wifi_get_state())
    {
        s8Ret = M2M_ERR_FAIL;
        M2M_ERR("WINC is not in an appropriate state for this operation!\n");
        goto EXIT;
    }

    if((u32FlashHFDStart == 0) || (u32FlashHFDSize == 0))
    {
        s8Ret = spi_flexible_flash_find_section(ENTRY_ID_HOSTFILE, &u32FlashHFDStart, &u32FlashHFDSize);
        if(M2M_SUCCESS != s8Ret) goto EXIT;
    }

    s8Ret = spi_flash_read(pu8Buff, u32FlashHFDStart, 4);

    if((M2M_SUCCESS != s8Ret) || (pu8Buff[0] != u8Handler)) goto EXIT;

    if((u32Offset >= u32FlashHFDSize) ||
            (u32Size   >  u32FlashHFDSize) ||
            ((u32Offset + u32Size) >= u32FlashHFDSize))
    {
        s8Ret = M2M_ERR_FAIL;
        goto EXIT;
    }

    s8Ret = spi_flash_read(pu8Buff, u32FlashHFDStart + FLASH_SECTOR_SZ + u32Offset, u32Size);

    if(M2M_SUCCESS != s8Ret)
        M2M_ERR("Unable to read SPI Flash\n");

EXIT:
    return s8Ret;
}

/*!
@fn         NMI_API m2m_ota_host_file_erase(uint8 u8Handler, tpfFileEraseCb pfHFDEraseCb)
@brief      Erase any traces of an existing file, this means from host driver and WINC firmware.
@param[in]  u8Handler
                ID of the file we are trying to erase. Must be valid.
@param[in]  pfHFDEraseCb
                Pointer to callback to execute when the file erase in the WINC completes.
@return     Status of the erase operation.
@note       Providing a callback is optional.
            If the current handler is invalid at this point, it means one of the three:
                1. The file never existed;
                2. The file has already been already deleted;
                3. The request to get the file hasn't fully completed.
            For 1. and 2. there is no need to signal the WINC to erase the file in Flash.
            For 3. the Flash can't be erased while a file download is ongoing.
*/
NMI_API sint8 m2m_ota_host_file_erase(uint8 u8Handler, tpfFileEraseCb pfHFDEraseCb)
{
    sint8 s8Ret = M2M_ERR_INVALID;
    if((u8Handler != gu8CurrFileHandlerID) || (HFD_INVALID_HANDLER == gu8CurrFileHandlerID)) goto EXIT;

    gu8CurrFileHandlerID = HFD_INVALID_HANDLER;
    gpfHFDReadCb  = NULL;
    gpfHFDEraseCb = pfHFDEraseCb;

    s8Ret = hif_send(M2M_REQ_GROUP_OTA, M2M_OTA_REQ_HOST_FILE_ERASE, NULL, 0, NULL, 0, 0);
EXIT:
    return s8Ret;
}

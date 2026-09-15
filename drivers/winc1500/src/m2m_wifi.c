/*******************************************************************************
  File Name:
    m2m_wifi.c

  Summary:
    This module contains M2M Wi-Fi APIs implementation.

  Description:
    This module contains M2M Wi-Fi APIs implementation.
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

#include "m2m_wifi.h"
#include "m2m_hif.h"
#include "nmasic.h"

/* Require authentication of server. */
#define WIFI_1X_TLS_HS_FLAGS_PEER_AUTH          NBIT1
/* Enable expiry checking of server certificate chain. */
#define WIFI_1X_TLS_HS_FLAGS_PEER_CERTTIMECHECK NBIT2
/* Require local system time to be known (i.e. fail expiry checking if time is not known locally). */
#define WIFI_1X_TLS_HS_FLAGS_REQUIRE_TIME       NBIT3
/* Enable TLS session caching. */
#define WIFI_1X_TLS_HS_FLAGS_SESSION_CACHING    NBIT4
/* Reserved, this bit must be clear. */
#define WIFI_1X_TLS_HS_FLAGS_RSV5               NBIT5
/* Require server authentication to be against a specified root certificate. */
#define WIFI_1X_TLS_HS_FLAGS_SPECIFY_ROOTCERT   NBIT6
/* Reserved, this bit must be clear. */
#define WIFI_1X_TLS_HS_FLAGS_RSV7               NBIT7

#define WIFI_1X_TLS_HS_FLAGS_DEFAULT    (           \
            WIFI_1X_TLS_HS_FLAGS_PEER_AUTH          \
        |   WIFI_1X_TLS_HS_FLAGS_PEER_CERTTIMECHECK \
        |   WIFI_1X_TLS_HS_FLAGS_SESSION_CACHING    \
)

static volatile uint8_t gu8ChNum;
static volatile uint8_t gu8WifiState      = WIFI_STATE_DEINIT;
static tpfAppWifiCb gpfAppWifiCb          = NULL;
static volatile uint8_t gu8scanInProgress = 0;

static uint32_t   gu321xTlsHsFlags      = WIFI_1X_TLS_HS_FLAGS_DEFAULT;
static uint8_t    gau81xRootSha1[20]    = {0};

static tpfAppEthCb  gpfAppEthCb   = NULL;
static uint8_t     *gau8ethRcvBuf = NULL;
static uint16_t     gu16ethRcvBufSize;

/**
@fn         void m2m_wifi_cb(uint8_t u8OpCode, uint16_t u16DataSize, uint32_t u32Addr, uint8_t grp)
@brief      Internal WiFi callback function.
@param[in]  u8OpCode
                HIF Opcode type.
@param[in]  u16DataSize
                HIF data length.
@param[in]  u32Addr
                HIF address.
@param[in]  grp
                HIF group type.
*/
static void m2m_wifi_cb(uint8_t u8OpCode, uint16_t u16DataSize, uint32_t u32Addr)
{
    uint8_t rx_buf[8];
    if (u8OpCode == M2M_WIFI_RESP_CON_STATE_CHANGED)
    {
        tstrM2mWifiStateChanged strState;
        if (hif_receive(u32Addr, (uint8_t*) &strState,sizeof(tstrM2mWifiStateChanged), 0) == M2M_SUCCESS)
        {
            if (gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_RESP_CON_STATE_CHANGED, &strState);
        }
    }
    else if (u8OpCode == M2M_WIFI_RESP_GET_SYS_TIME)
    {
        tstrSystemTime strSysTime;
        if (hif_receive(u32Addr, (uint8_t*) &strSysTime,sizeof(tstrSystemTime), 0) == M2M_SUCCESS)
        {
            if (gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_RESP_GET_SYS_TIME, &strSysTime);
        }
    }
    else if(u8OpCode == M2M_WIFI_RESP_CONN_INFO)
    {
        tstrM2MConnInfo     strConnInfo;
        if(hif_receive(u32Addr, (uint8_t*)&strConnInfo, sizeof(tstrM2MConnInfo), 1) == M2M_SUCCESS)
        {
            if(gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_RESP_CONN_INFO, &strConnInfo);
        }
    }
    else if (u8OpCode == M2M_WIFI_RESP_MEMORY_RECOVER)
    {
    }
    else if (u8OpCode == M2M_WIFI_REQ_DHCP_CONF)
    {
        tstrM2MIPConfig strIpConfig;
        if (hif_receive(u32Addr, (uint8_t*)&strIpConfig, sizeof(tstrM2MIPConfig), 0) == M2M_SUCCESS)
        {
            if (gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_REQ_DHCP_CONF, (uint8_t*)&strIpConfig);
        }
    }
    else if (u8OpCode == M2M_WIFI_REQ_DHCP_FAILURE)
    {
        if (hif_receive(u32Addr, NULL, 0, 1) == M2M_SUCCESS)
        {
            if (gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_REQ_DHCP_FAILURE, NULL);
        }
    }
    else if (u8OpCode == M2M_WIFI_REQ_WPS)
    {
        tstrM2MWPSInfo strWps;
        memset((uint8_t*)&strWps,0,sizeof(tstrM2MWPSInfo));
        if(hif_receive(u32Addr, (uint8_t*)&strWps, sizeof(tstrM2MWPSInfo), 0) == M2M_SUCCESS)
        {
            if (gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_REQ_WPS, &strWps);
        }
    }
    else if (u8OpCode == M2M_WIFI_RESP_IP_CONFLICT)
    {
        uint32_t  u32ConflictedIP;
        if(hif_receive(u32Addr, (uint8_t*)&u32ConflictedIP, sizeof(u32ConflictedIP), 0) == M2M_SUCCESS)
        {
            M2M_INFO("Conflicted IP \" %u.%u.%u.%u \"\r\n",
                BYTE_0(u32ConflictedIP), BYTE_1(u32ConflictedIP), BYTE_2(u32ConflictedIP), BYTE_3(u32ConflictedIP));
            if (gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_RESP_IP_CONFLICT, NULL);
        }
    }
    else if (u8OpCode == M2M_WIFI_RESP_SCAN_DONE)
    {
        tstrM2mScanDone strState;
        gu8scanInProgress = 0;
        if(hif_receive(u32Addr, (uint8_t*)&strState, sizeof(tstrM2mScanDone), 0) == M2M_SUCCESS)
        {
            gu8ChNum = strState.u8NumofCh;
            if (gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_RESP_SCAN_DONE, &strState);
        }
    }
    else if (u8OpCode == M2M_WIFI_RESP_SCAN_RESULT)
    {
        tstrM2mWifiscanResult strScanResult;
        if(hif_receive(u32Addr, (uint8_t*)&strScanResult, sizeof(tstrM2mWifiscanResult), 0) == M2M_SUCCESS)
        {
            if (gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_RESP_SCAN_RESULT, &strScanResult);
        }
    }
    else if (u8OpCode == M2M_WIFI_RESP_CURRENT_RSSI)
    {
        if (hif_receive(u32Addr, rx_buf, 4, 0) == M2M_SUCCESS)
        {
            if (gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_RESP_CURRENT_RSSI, rx_buf);
        }
    }
    else if (u8OpCode == M2M_WIFI_RESP_CLIENT_INFO)
    {
        if (hif_receive(u32Addr, rx_buf, 4, 0) == M2M_SUCCESS)
        {
            if (gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_RESP_CLIENT_INFO, rx_buf);
        }
    }
    else if(u8OpCode == M2M_WIFI_RESP_PROVISION_INFO)
    {
        tstrM2MProvisionInfo    strProvInfo;
        if(hif_receive(u32Addr, (uint8_t*)&strProvInfo, sizeof(tstrM2MProvisionInfo), 1) == M2M_SUCCESS)
        {
            if(gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_RESP_PROVISION_INFO, &strProvInfo);
        }
    }
    else if(u8OpCode == M2M_WIFI_RESP_DEFAULT_CONNECT)
    {
        tstrM2MDefaultConnResp  strResp;
        if(hif_receive(u32Addr, (uint8_t*)&strResp, sizeof(tstrM2MDefaultConnResp), 1) == M2M_SUCCESS)
        {
            if(gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_RESP_DEFAULT_CONNECT, &strResp);
        }
    }
    else if (u8OpCode == M2M_WIFI_REQRSP_DELETE_APID)
    {
        tstrM2MGenericResp strResp;
        if (hif_receive(u32Addr, (uint8_t*)&strResp, sizeof(tstrM2MGenericResp), 0) == M2M_SUCCESS)
        {
            if (gpfAppWifiCb)
                gpfAppWifiCb(M2M_WIFI_REQRSP_DELETE_APID, &strResp);
        }
    }
    else if(u8OpCode == M2M_WIFI_RESP_GET_PRNG)
    {
        tstrPrng strPrng;
        if(hif_receive(u32Addr, (uint8_t*)&strPrng,sizeof(tstrPrng), 0) == M2M_SUCCESS)
        {
            if(hif_receive(u32Addr + sizeof(tstrPrng),strPrng.pu8RngBuff,strPrng.u16PrngSize, 1) == M2M_SUCCESS)
            {
                if(gpfAppWifiCb)
                    gpfAppWifiCb(M2M_WIFI_RESP_GET_PRNG,&strPrng);
            }
        }
    }
    else if(u8OpCode == M2M_WIFI_RESP_ETHERNET_RX_PACKET)
    {
        uint8_t u8SetRxDone;
        tstrM2mIpRsvdPkt strM2mRsvd;

        if(hif_receive(u32Addr, (uint8_t*)&strM2mRsvd, sizeof(tstrM2mIpRsvdPkt), 0) == M2M_SUCCESS)
        {
            tstrM2mIpCtrlBuf  strM2mIpCtrlBuf;
            uint16_t u16Offset = strM2mRsvd.u16PktOffset;

            strM2mIpCtrlBuf.u16RemainingDataSize = strM2mRsvd.u16PktSz;
            if((gpfAppEthCb) && (gau8ethRcvBuf) && (gu16ethRcvBufSize > 0))
            {
                do
                {
                    u8SetRxDone = 1;
                    if(strM2mIpCtrlBuf.u16RemainingDataSize > gu16ethRcvBufSize)
                    {
                        u8SetRxDone = 0;
                        strM2mIpCtrlBuf.u16DataSize = gu16ethRcvBufSize;
                    }
                    else
                    {
                        strM2mIpCtrlBuf.u16DataSize = strM2mIpCtrlBuf.u16RemainingDataSize;
                    }

                    if(hif_receive(u32Addr + u16Offset, gau8ethRcvBuf, strM2mIpCtrlBuf.u16DataSize, u8SetRxDone) == M2M_SUCCESS)
                    {
                        strM2mIpCtrlBuf.u16RemainingDataSize -= strM2mIpCtrlBuf.u16DataSize;
                        u16Offset += strM2mIpCtrlBuf.u16DataSize;
                        gpfAppEthCb(M2M_WIFI_RESP_ETHERNET_RX_PACKET, gau8ethRcvBuf, &(strM2mIpCtrlBuf));
                    }
                    else
                    {
                        break;
                    }
                }
                while (strM2mIpCtrlBuf.u16RemainingDataSize > 0);
            }
        }
    }
    else
    {
        M2M_ERR("REQ Not defined %d\r\n",u8OpCode);
    }
}

int8_t m2m_wifi_download_mode(void)
{
    int8_t ret = M2M_SUCCESS;
    /* Apply device specific initialization. */
    ret = nm_drv_init_download_mode();
    if(ret != M2M_SUCCESS)  goto _EXIT0;

    enable_interrupts();
    gu8WifiState = WIFI_STATE_INIT;

_EXIT0:
    return ret;
}

static int8_t m2m_validate_ap_parameters(const tstrM2MAPModeConfig *pstrM2MAPModeConfig)
{
    int8_t s8Ret = M2M_SUCCESS;
    /* Check for incoming pointer */
    if(pstrM2MAPModeConfig == NULL)
    {
        M2M_ERR("INVALID POINTER\r\n");
        s8Ret = M2M_ERR_FAIL;
        goto ERR1;
    }
    /* Check for SSID */
    if((strlen((const char*)pstrM2MAPModeConfig->strApConfig.au8SSID) <= 0) || (strlen((const char*)pstrM2MAPModeConfig->strApConfig.au8SSID) >= M2M_MAX_SSID_LEN))
    {
        M2M_ERR("INVALID SSID\r\n");
        s8Ret = M2M_ERR_FAIL;
        goto ERR1;
    }
    /* Check for Channel */
    if(pstrM2MAPModeConfig->strApConfig.u8ListenChannel > M2M_WIFI_CH_14 || pstrM2MAPModeConfig->strApConfig.u8ListenChannel < M2M_WIFI_CH_1)
    {
        M2M_ERR("INVALID CH\r\n");
        s8Ret = M2M_ERR_FAIL;
        goto ERR1;
    }
    /* Check for DHCP Server IP address */
    if(!(pstrM2MAPModeConfig->strApConfig.au8DHCPServerIP[0] || pstrM2MAPModeConfig->strApConfig.au8DHCPServerIP[1]))
    {
        if(!(pstrM2MAPModeConfig->strApConfig.au8DHCPServerIP[2]))
        {
            M2M_ERR("INVALID DHCP SERVER IP\r\n");
            s8Ret = M2M_ERR_FAIL;
            goto ERR1;
        }
    }
    /* Check for Security */
    if(pstrM2MAPModeConfig->strApConfig.u8SecType == M2M_WIFI_SEC_OPEN)
    {
        goto ERR1;
    }
    else if(pstrM2MAPModeConfig->strApConfig.u8SecType == M2M_WIFI_SEC_WEP)
    {
        /* As of 19.7.5 the WEP protocol is deprecated */
        s8Ret = M2M_ERR_FAIL;
        goto ERR1;
    }
    else if(pstrM2MAPModeConfig->strApConfig.u8SecType == M2M_WIFI_SEC_WPA_PSK)
    {
        /* Check for WPA Key size */
        if( ((pstrM2MAPModeConfig->strApConfig.u8KeySz + 1) < M2M_MIN_PSK_LEN) || ((pstrM2MAPModeConfig->strApConfig.u8KeySz + 1) > M2M_MAX_PSK_LEN))
        {
            M2M_ERR("INVALID WPA KEY SIZE\r\n");
            s8Ret = M2M_ERR_FAIL;
            goto ERR1;
        }
    }
    else
    {
        M2M_ERR("INVALID AUTHENTICATION MODE\r\n");
        s8Ret = M2M_ERR_FAIL;
        goto ERR1;
    }

ERR1:
    return s8Ret;
}

static int8_t m2m_validate_scan_options(tstrM2MScanOption *ptstrM2MScanOption)
{
    int8_t s8Ret = M2M_SUCCESS;
    /* Check for incoming pointer */
    if(ptstrM2MScanOption == NULL)
    {
        M2M_ERR("INVALID POINTER\r\n");
        s8Ret = M2M_ERR_FAIL;
        goto ERR;
    }
    /* Check for valid No of slots */
    if(ptstrM2MScanOption->u8NumOfSlot == 0)
    {
        M2M_ERR("INVALID No of scan slots! %d\r\n",ptstrM2MScanOption->u8NumOfSlot);
        s8Ret = M2M_ERR_FAIL;
        goto ERR;
    }
    /* Check for valid time of slots */
    if(ptstrM2MScanOption->u8SlotTime < 10 || ptstrM2MScanOption->u8SlotTime > 250)
    {
        M2M_ERR("INVALID scan slot time! %d\r\n",ptstrM2MScanOption->u8SlotTime);
        s8Ret = M2M_ERR_FAIL;
        goto ERR;
    }
    /* Check for valid No of probe requests per slot */
    if((ptstrM2MScanOption->u8ProbesPerSlot == 0) || (ptstrM2MScanOption->u8ProbesPerSlot > M2M_SCAN_DEFAULT_NUM_PROBE))
    {
        M2M_ERR("INVALID No of probe requests per scan slot %d\r\n",ptstrM2MScanOption->u8ProbesPerSlot);
        s8Ret = M2M_ERR_FAIL;
        goto ERR;
    }
    /* Check for valid RSSI threshold */
    if(ptstrM2MScanOption->s8RssiThresh >= 0)
    {
        M2M_ERR("INVALID RSSI threshold %d\r\n",ptstrM2MScanOption->s8RssiThresh);
        s8Ret = M2M_ERR_FAIL;
    }

ERR:
    return s8Ret;
}

int8_t m2m_wifi_send_crl(tstrTlsCrlInfo *pCRL)
{
    int8_t s8Ret = M2M_ERR_FAIL;
    s8Ret = hif_send(M2M_REQ_GROUP_SSL, M2M_SSL_IND_CRL|M2M_REQ_DATA_PKT, NULL, 0, (uint8_t*)pCRL, sizeof(tstrTlsCrlInfo), 0);
    return s8Ret;
}

int8_t m2m_wifi_init_hold(void)
{
    int8_t ret = M2M_ERR_FAIL;

    /* Apply device specific initialization. */
    ret = nm_drv_init_hold();

    if(M2M_SUCCESS == ret)
    {
        gu8WifiState = WIFI_STATE_INIT;
    }
    return ret;
}

int8_t m2m_wifi_init_start(tstrWifiInitParam *pWifiInitParam)
{
    tstrM2mRev strtmp;
    int8_t ret = M2M_SUCCESS;
    uint8_t u8WifiMode = M2M_WIFI_MODE_NORMAL;

    if(pWifiInitParam == NULL) {
        ret = M2M_ERR_FAIL;
        goto _EXIT0;
    }

    gpfAppWifiCb = pWifiInitParam->pfAppWifiCb;

    gpfAppEthCb         = pWifiInitParam->strEthInitParam.pfAppEthCb;
    gau8ethRcvBuf       = pWifiInitParam->strEthInitParam.au8ethRcvBuf;
    gu16ethRcvBufSize   = pWifiInitParam->strEthInitParam.u16ethRcvBufSize;

    if (pWifiInitParam->strEthInitParam.u8EthernetEnable)
        u8WifiMode = M2M_WIFI_MODE_ETHERNET;

    gu8scanInProgress = 0;

    /* Apply device specific initialization. */
    ret = nm_drv_init_start(&u8WifiMode);
    if(ret != M2M_SUCCESS) goto _EXIT0;

    gu8WifiState = WIFI_STATE_START;

    /* Initialize host interface module */
    ret = hif_init(NULL);
    if(ret != M2M_SUCCESS) goto _EXIT1;

    hif_register_cb(M2M_REQ_GROUP_WIFI, m2m_wifi_cb);

    ret = nm_get_firmware_full_info(&strtmp);

    M2M_INFO("\nWINC1500 Firmware Data:\r\n");
    M2M_INFO("Firmware Ver: %u.%u.%u SVN Rev %u\r\n", strtmp.u8FirmwareMajor, strtmp.u8FirmwareMinor, strtmp.u8FirmwarePatch, strtmp.u16FirmwareSvnNum);
    M2M_INFO("Firmware Built at %s Time %s\r\n", strtmp.BuildDate, strtmp.BuildTime);
    M2M_INFO("Firmware Min Driver Ver: %u.%u.%u\r\n", strtmp.u8DriverMajor, strtmp.u8DriverMinor, strtmp.u8DriverPatch);
    M2M_INFO("Driver Ver: %u.%u.%u\r\n", M2M_RELEASE_VERSION_MAJOR_NO, M2M_RELEASE_VERSION_MINOR_NO, M2M_RELEASE_VERSION_PATCH_NO);
    M2M_INFO("Driver Built at %s Time %s\r\n\r\n", __DATE__, __TIME__);

    if(M2M_ERR_FW_VER_MISMATCH == ret)
    {
        M2M_ERR("Mismatch Firmware Version\r\n");
    }

    goto _EXIT0;

_EXIT1:
    gu8WifiState = WIFI_STATE_DEINIT;
    nm_drv_deinit(NULL);
_EXIT0:
    return ret;
}

int8_t m2m_wifi_init(tstrWifiInitParam *pWifiInitParam)
{
    int8_t ret = M2M_SUCCESS;

    ret = m2m_wifi_init_hold();
    if (ret == M2M_SUCCESS)
    {
        ret = m2m_wifi_init_start(pWifiInitParam);
    }
    return ret;
}

int8_t  m2m_wifi_deinit(void *arg)
{
    gu8WifiState = WIFI_STATE_DEINIT;
    hif_deinit(NULL);

    nm_drv_deinit(NULL);

    return M2M_SUCCESS;
}

int8_t m2m_wifi_handle_events(void)
{
    return hif_handle_isr();
}

int8_t m2m_wifi_delete_sc(char *pcSsid, uint8_t u8SsidLen)
{
    tstrM2mWifiApId strApId;
    memset((uint8_t*)&strApId, 0, sizeof(strApId));
    strApId.au8SSID[0] = 0xFF;  // Special value used to cause fw to delete all entries.
    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQRSP_DELETE_APID, (uint8_t*)&strApId, sizeof(tstrM2mWifiApId), NULL, 0, 0);
}

int8_t m2m_wifi_default_connect(void)
{
    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_DEFAULT_CONNECT, NULL, 0, NULL, 0, 0);
}

/*************************************************************************************************/
/*                                WIFI CONNECT INTERNAL FUNCTIONS                                */
/*************************************************************************************************/
static int8_t m2m_wifi_connect_prepare_msg(
    tenuCredStoreOption enuCredStoreOption,
    tenuM2mSecType      enuAuthType,
    uint16_t            u16AuthSize,
    tstrNetworkId       *pstrNetworkId,
    tstrM2mWifiConnHdr  *pstrWifiConn
)
{
    int8_t    ret = M2M_ERR_FAIL;
    uint16_t  u16CredSize = sizeof(tstrM2mConnCredCmn) + u16AuthSize;

    /* Check application params. */
    if (
            (pstrNetworkId == NULL)
        ||  (pstrNetworkId->pu8Ssid == NULL)
        ||  (pstrNetworkId->u8SsidLen >= M2M_MAX_SSID_LEN)
    )
        goto INVALID_ARG;

    if (pstrWifiConn != NULL)
    {
        tstrM2mConnCredHdr  *pstrHdr = &pstrWifiConn->strConnCredHdr;
        tstrM2mConnCredCmn  *pstrCmn = &pstrWifiConn->strConnCredCmn;

        memset((uint8_t*)pstrWifiConn, 0, sizeof(tstrM2mWifiConnHdr));

        pstrHdr->u16CredSize = u16CredSize;
        switch (enuCredStoreOption)
        {
        case WIFI_CRED_SAVE_ENCRYPTED:
            pstrHdr->u8CredStoreFlags |= M2M_CRED_ENCRYPT_FLAG;
        // intentional fall through...
        case WIFI_CRED_SAVE_UNENCRYPTED:
            pstrHdr->u8CredStoreFlags |= M2M_CRED_STORE_FLAG;
        // intentional fall through...
        case WIFI_CRED_DONTSAVE:
            break;
        default:
            goto INVALID_ARG;
        }

        if (pstrNetworkId->enuChannel == M2M_WIFI_CH_ALL)
            pstrHdr->u8Channel = (uint8_t)(pstrNetworkId->enuChannel);
        else if ((pstrNetworkId->enuChannel <= M2M_WIFI_CH_14) && (pstrNetworkId->enuChannel >= M2M_WIFI_CH_1))
            pstrHdr->u8Channel = (uint8_t)(pstrNetworkId->enuChannel) - 1;
        else
            goto INVALID_ARG;

        if ((enuAuthType == M2M_WIFI_SEC_INVALID) || (enuAuthType >= M2M_WIFI_NUM_AUTH_TYPES))
            goto INVALID_ARG;
        pstrCmn->u8AuthType = (uint8_t)enuAuthType;

        pstrCmn->u8SsidLen = pstrNetworkId->u8SsidLen;
        memcpy(pstrCmn->au8Ssid, pstrNetworkId->pu8Ssid, pstrNetworkId->u8SsidLen);
        if (pstrNetworkId->pu8Bssid != NULL)
        {
            pstrCmn->u8Options = M2M_WIFI_CONN_BSSID_FLAG;
            memcpy(pstrCmn->au8Bssid, pstrNetworkId->pu8Bssid, M2M_MAC_ADDRES_LEN);
        }
        /* Everything is ok, set return value. */
        ret = M2M_SUCCESS;
    }
    return ret;
INVALID_ARG:
    return M2M_ERR_INVALID_ARG;
}

/* Convert hexchar to value 0-15 */
static uint8_t hexchar_2_val(uint8_t ch)
{
    ch -= 0x30;
    if (ch <= 9)
        return ch;
    ch |= 0x20;
    ch -= 0x31;
    if (ch <= 5)
        return ch + 10;
    return 0xFF;
}
/* Convert hexstring to bytes */
static int8_t hexstr_2_bytes(uint8_t *pu8Out, uint8_t *pu8In, uint8_t u8SizeOut)
{
    while (u8SizeOut--)
    {
        uint8_t u8Out = hexchar_2_val(*pu8In++);
        if (u8Out > 0xF)
            return M2M_ERR_INVALID_ARG;
        *pu8Out = u8Out * 0x10;
        u8Out = hexchar_2_val(*pu8In++);
        if (u8Out > 0xF)
            return M2M_ERR_INVALID_ARG;
        *pu8Out += u8Out;
        pu8Out++;
    }
    return M2M_SUCCESS;
}

/*************************************************************************************************/
/*                                        WIFI CONNECT APIS                                      */
/*************************************************************************************************/
int8_t m2m_wifi_connect_open(
    tenuCredStoreOption enuCredStoreOption,
    tstrNetworkId       *pstrNetworkId
)
{
    int8_t              ret = M2M_ERR_INVALID_ARG;
    tstrM2mWifiConnHdr  strConnHdr;

    ret = m2m_wifi_connect_prepare_msg(enuCredStoreOption, M2M_WIFI_SEC_OPEN, 0, pstrNetworkId, &strConnHdr);
    if (ret == M2M_SUCCESS)
    {
        ret = hif_send( M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_CONN,
                        (uint8_t*)&strConnHdr, sizeof(strConnHdr),
                        NULL, 0, 0);
    }
    return ret;
}

int8_t m2m_wifi_connect_wep(
    tenuCredStoreOption enuCredStoreOption,
    tstrNetworkId       *pstrNetworkId,
    tstrAuthWep         *pstrAuthWep
)
{
    /* As of 19.7.5 the WEP protocol is deprecated */
    return M2M_ERR_INVALID;
}

int8_t m2m_wifi_connect_psk(
    tenuCredStoreOption enuCredStoreOption,
    tstrNetworkId       *pstrNetworkId,
    tstrAuthPsk         *pstrAuthPsk
)
{
    int8_t  ret = M2M_ERR_INVALID_ARG;

    if (pstrAuthPsk != NULL)
    {
        tstrM2mWifiConnHdr  strConnHdr;

        ret = m2m_wifi_connect_prepare_msg( enuCredStoreOption,
                                            M2M_WIFI_SEC_WPA_PSK,
                                            sizeof(tstrM2mWifiPsk),
                                            pstrNetworkId,
                                            &strConnHdr);

        if (ret == M2M_SUCCESS)
        {
            tstrM2mWifiPsk  *pstrPsk = (tstrM2mWifiPsk*)OSAL_Malloc(sizeof(tstrM2mWifiPsk));

            if (pstrPsk != NULL)
            {
                memset((uint8_t*)pstrPsk, 0, sizeof(tstrM2mWifiPsk));
                if (pstrAuthPsk->pu8Psk != NULL)
                {
                    if (pstrAuthPsk->pu8Passphrase != NULL)
                        ret = M2M_ERR_INVALID_ARG;
                    else
                    {
                        pstrPsk->u8PassphraseLen = M2M_MAX_PSK_LEN-1;
                        /* Use hexstr_2_bytes to verify pu8Psk input. */
                        if (M2M_SUCCESS != hexstr_2_bytes(pstrPsk->au8Passphrase, pstrAuthPsk->pu8Psk, pstrPsk->u8PassphraseLen/2))
                            ret = M2M_ERR_INVALID_ARG;
                        memcpy(pstrPsk->au8Passphrase, pstrAuthPsk->pu8Psk, pstrPsk->u8PassphraseLen);
                    }
                }
                else if (pstrAuthPsk->pu8Passphrase != NULL)
                {
                    if (pstrAuthPsk->u8PassphraseLen > M2M_MAX_PSK_LEN-1)
                        ret = M2M_ERR_INVALID_ARG;
                    else
                    {
                        pstrPsk->u8PassphraseLen = pstrAuthPsk->u8PassphraseLen;
                        memcpy(pstrPsk->au8Passphrase, pstrAuthPsk->pu8Passphrase, pstrPsk->u8PassphraseLen);
                    }
                }
                else
                    ret = M2M_ERR_INVALID_ARG;
                if (ret == M2M_SUCCESS)
                {
                    ret = hif_send( M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_CONN | M2M_REQ_DATA_PKT,
                                    (uint8_t*)&strConnHdr, sizeof(tstrM2mWifiConnHdr),
                                    (uint8_t*)pstrPsk, sizeof(tstrM2mWifiPsk), sizeof(tstrM2mWifiConnHdr));
                }
                OSAL_Free(pstrPsk);
            }
            else
            {
                ret = M2M_ERR_MEM_ALLOC;
            }
        }
    }
    return ret;
}

int8_t m2m_wifi_1x_set_option(tenu1xOption enuOptionName, const void *pOptionValue, size_t OptionLen)
{
    if((pOptionValue == NULL) && (OptionLen > 0))
        return M2M_ERR_INVALID_ARG;
    switch(enuOptionName)
    {
    case WIFI_1X_BYPASS_SERVER_AUTH:
        if(OptionLen != sizeof(int))
            return M2M_ERR_INVALID_ARG;
        switch(*(int*)pOptionValue)
        {
        case 1:
            gu321xTlsHsFlags &= ~WIFI_1X_TLS_HS_FLAGS_PEER_AUTH;
            break;
        case 0:
            gu321xTlsHsFlags |= WIFI_1X_TLS_HS_FLAGS_PEER_AUTH;
            break;
        default:
            return M2M_ERR_INVALID_ARG;
        }
        break;
    case WIFI_1X_TIME_VERIF_MODE:
        if(OptionLen != sizeof(tenuSslCertExpSettings))
            return M2M_ERR_INVALID_ARG;
        switch(*(tenuSslCertExpSettings*)pOptionValue)
        {
        case SSL_CERT_EXP_CHECK_DISABLE:
            gu321xTlsHsFlags &= ~WIFI_1X_TLS_HS_FLAGS_PEER_CERTTIMECHECK;
            gu321xTlsHsFlags &= ~WIFI_1X_TLS_HS_FLAGS_REQUIRE_TIME;
            break;
        case SSL_CERT_EXP_CHECK_ENABLE:
            gu321xTlsHsFlags |= WIFI_1X_TLS_HS_FLAGS_PEER_CERTTIMECHECK;
            gu321xTlsHsFlags |= WIFI_1X_TLS_HS_FLAGS_REQUIRE_TIME;
            break;
        case SSL_CERT_EXP_CHECK_EN_IF_SYS_TIME:
            gu321xTlsHsFlags |= WIFI_1X_TLS_HS_FLAGS_PEER_CERTTIMECHECK;
            gu321xTlsHsFlags &= ~WIFI_1X_TLS_HS_FLAGS_REQUIRE_TIME;
            break;
        default:
            return M2M_ERR_INVALID_ARG;
        }
        break;
    case WIFI_1X_SESSION_CACHING:
        if(OptionLen != sizeof(int))
            return M2M_ERR_INVALID_ARG;
        switch(*(int*)pOptionValue)
        {
        case 1:
            gu321xTlsHsFlags |= WIFI_1X_TLS_HS_FLAGS_SESSION_CACHING;
            break;
        case 0:
            gu321xTlsHsFlags &= ~WIFI_1X_TLS_HS_FLAGS_SESSION_CACHING;
            break;
        default:
            return M2M_ERR_INVALID_ARG;
        }
        break;
    case WIFI_1X_SPECIFIC_ROOTCERT:
        switch(OptionLen)
        {
        case 20:
            gu321xTlsHsFlags |= WIFI_1X_TLS_HS_FLAGS_SPECIFY_ROOTCERT;
            memcpy(gau81xRootSha1, (uint8_t*)pOptionValue, sizeof(gau81xRootSha1));
            break;
        case 0:
            gu321xTlsHsFlags &= ~WIFI_1X_TLS_HS_FLAGS_SPECIFY_ROOTCERT;
            memset(gau81xRootSha1, 0, sizeof(gau81xRootSha1));
            break;
        default:
            return M2M_ERR_INVALID_ARG;
        }
        break;
    default:
        return M2M_ERR_INVALID_ARG;
    }
    return M2M_SUCCESS;
}

int8_t m2m_wifi_1x_get_option(tenu1xOption enuOptionName, void *pOptionValue, size_t *pOptionLen)
{
    if(pOptionValue == NULL)
        return M2M_ERR_INVALID_ARG;
    switch(enuOptionName)
    {
    case WIFI_1X_BYPASS_SERVER_AUTH:
        if(*pOptionLen < sizeof(int))
            return M2M_ERR_INVALID_ARG;
        *pOptionLen = sizeof(int);
        *(int*)pOptionValue = (gu321xTlsHsFlags & WIFI_1X_TLS_HS_FLAGS_PEER_AUTH) ? 0 : 1;
        break;
    case WIFI_1X_TIME_VERIF_MODE:
        if(*pOptionLen < sizeof(tenuSslCertExpSettings))
            return M2M_ERR_INVALID_ARG;
        *pOptionLen = sizeof(tenuSslCertExpSettings);
        if(!(gu321xTlsHsFlags & WIFI_1X_TLS_HS_FLAGS_PEER_CERTTIMECHECK))
            *(tenuSslCertExpSettings*)pOptionValue = SSL_CERT_EXP_CHECK_DISABLE;
        else if(gu321xTlsHsFlags & WIFI_1X_TLS_HS_FLAGS_REQUIRE_TIME)
            *(tenuSslCertExpSettings*)pOptionValue = SSL_CERT_EXP_CHECK_ENABLE;
        else
            *(tenuSslCertExpSettings*)pOptionValue = SSL_CERT_EXP_CHECK_EN_IF_SYS_TIME;
        break;
    case WIFI_1X_SESSION_CACHING:
        if(*pOptionLen < sizeof(int))
            return M2M_ERR_INVALID_ARG;
        *pOptionLen = sizeof(int);
        *(int*)pOptionValue = (gu321xTlsHsFlags & WIFI_1X_TLS_HS_FLAGS_SESSION_CACHING) ? 1 : 0;
        break;
    case WIFI_1X_SPECIFIC_ROOTCERT:
        if(gu321xTlsHsFlags & WIFI_1X_TLS_HS_FLAGS_SPECIFY_ROOTCERT)
        {
            if(*pOptionLen < sizeof(gau81xRootSha1))
                return M2M_ERR_INVALID_ARG;
            *pOptionLen = sizeof(gau81xRootSha1);
            memcpy((uint8_t*)pOptionValue, gau81xRootSha1, sizeof(gau81xRootSha1));
        }
        else
            *pOptionLen = 0;
        break;
    default:
        return M2M_ERR_INVALID_ARG;
    }
    return M2M_SUCCESS;
}

int8_t m2m_wifi_connect_1x_mschap2(
    tenuCredStoreOption enuCredStoreOption,
    tstrNetworkId       *pstrNetworkId,
    tstrAuth1xMschap2   *pstrAuth1xMschap2
)
{
    int8_t ret = M2M_ERR_INVALID_ARG;
    if (pstrAuth1xMschap2 != NULL)
    {
        if (pstrAuth1xMschap2->pu8Domain == NULL)
            pstrAuth1xMschap2->u16DomainLen = 0;
        if (
                (pstrAuth1xMschap2->pu8UserName != NULL)
            &&  (pstrAuth1xMschap2->pu8Password != NULL)
            &&  ((uint32_t)(pstrAuth1xMschap2->u16DomainLen) + pstrAuth1xMschap2->u16UserNameLen <= M2M_AUTH_1X_USER_LEN_MAX)
            &&  (pstrAuth1xMschap2->u16PasswordLen <= M2M_AUTH_1X_PASSWORD_LEN_MAX)
        )
        {
            tstrM2mWifiConnHdr  strConnHdr;
            uint16_t                u16AuthSize =   sizeof(tstrM2mWifi1xHdr) +
                                                pstrAuth1xMschap2->u16DomainLen +
                                                pstrAuth1xMschap2->u16UserNameLen +
                                                pstrAuth1xMschap2->u16PasswordLen;

            ret = m2m_wifi_connect_prepare_msg( enuCredStoreOption,
                                                M2M_WIFI_SEC_802_1X,
                                                u16AuthSize,
                                                pstrNetworkId,
                                                &strConnHdr);

            if (ret == M2M_SUCCESS)
            {
                tstrM2mWifi1xHdr    *pstr1xHdr = (tstrM2mWifi1xHdr*)OSAL_Malloc(u16AuthSize);

                if (pstr1xHdr != NULL)
                {
                    uint8_t *pu8AuthPtr = pstr1xHdr->au81xAuthDetails;
                    memset((uint8_t*)pstr1xHdr, 0, u16AuthSize);

                    pstr1xHdr->u8Flags = M2M_802_1X_MSCHAP2_FLAG;
                    if (pstrAuth1xMschap2->bUnencryptedUserName == true)
                        pstr1xHdr->u8Flags |= M2M_802_1X_UNENCRYPTED_USERNAME_FLAG;
                    if (pstrAuth1xMschap2->bPrependDomain == true)
                        pstr1xHdr->u8Flags |= M2M_802_1X_PREPEND_DOMAIN_FLAG;

                    pstr1xHdr->u8HdrLength = sizeof(tstrM2mWifi1xHdr);
                    pstr1xHdr->u32TlsHsFlags = gu321xTlsHsFlags;
                    memcpy(pstr1xHdr->au8TlsSpecificRootNameSha1, gau81xRootSha1, sizeof(gau81xRootSha1));

                    pstr1xHdr->u8DomainLength = 0;
                    if (pstrAuth1xMschap2->pu8Domain != NULL)
                    {
                        pstr1xHdr->u8DomainLength = (uint8_t)(pstrAuth1xMschap2->u16DomainLen);
                        memcpy(pu8AuthPtr, pstrAuth1xMschap2->pu8Domain, pstr1xHdr->u8DomainLength);
                        pu8AuthPtr += pstr1xHdr->u8DomainLength;
                    }

                    pstr1xHdr->u8UserNameLength = (pstrAuth1xMschap2->u16UserNameLen);
                    memcpy(pu8AuthPtr, pstrAuth1xMschap2->pu8UserName, pstr1xHdr->u8UserNameLength);
                    pu8AuthPtr += pstr1xHdr->u8UserNameLength;

                    pstr1xHdr->u16PrivateKeyOffset = pu8AuthPtr - pstr1xHdr->au81xAuthDetails;
                    pstr1xHdr->u16PrivateKeyLength = pstrAuth1xMschap2->u16PasswordLen;
                    memcpy(pu8AuthPtr, pstrAuth1xMschap2->pu8Password, pstr1xHdr->u16PrivateKeyLength);

                    ret = hif_send( M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_CONN | M2M_REQ_DATA_PKT,
                                    (uint8_t*)&strConnHdr, sizeof(tstrM2mWifiConnHdr),
                                    (uint8_t*)pstr1xHdr, u16AuthSize,
                                    sizeof(tstrM2mWifiConnHdr));
                    OSAL_Free(pstr1xHdr);
                }
                else
                    ret = M2M_ERR_MEM_ALLOC;
            }
        }
    }
    return ret;
}

int8_t m2m_wifi_connect_1x_tls(
tenuCredStoreOption enuCredStoreOption,
tstrNetworkId       *pstrNetworkId,
tstrAuth1xTls       *pstrAuth1xTls
)
{
    int8_t ret = M2M_ERR_INVALID_ARG;
    if (pstrAuth1xTls != NULL)
    {
        if (pstrAuth1xTls->pu8Domain == NULL)
            pstrAuth1xTls->u16DomainLen = 0;
        if (
                (pstrAuth1xTls->pu8UserName != NULL)
            &&  (pstrAuth1xTls->pu8PrivateKey_Mod != NULL)
            &&  (pstrAuth1xTls->pu8PrivateKey_Exp != NULL)
            &&  (pstrAuth1xTls->pu8Certificate != NULL)
            &&  ((uint32_t)(pstrAuth1xTls->u16DomainLen) + pstrAuth1xTls->u16UserNameLen <= M2M_AUTH_1X_USER_LEN_MAX)
            &&  (pstrAuth1xTls->u16PrivateKeyLen <= M2M_AUTH_1X_PRIVATEKEY_LEN_MAX)
            &&  (pstrAuth1xTls->u16CertificateLen <= M2M_AUTH_1X_CERT_LEN_MAX)
        )
        {
            tstrM2mWifiConnHdr  strConnHdr;
            uint16_t            u16AuthSize =   sizeof(tstrM2mWifi1xHdr) +
                                                pstrAuth1xTls->u16DomainLen +
                                                pstrAuth1xTls->u16UserNameLen +
                                                (2 * pstrAuth1xTls->u16PrivateKeyLen) +
                                                pstrAuth1xTls->u16CertificateLen;

            ret = m2m_wifi_connect_prepare_msg( enuCredStoreOption,
                                                M2M_WIFI_SEC_802_1X,
                                                u16AuthSize,
                                                pstrNetworkId,
                                                &strConnHdr);

            if (ret == M2M_SUCCESS)
            {
                uint16_t            u16Payload1Size = u16AuthSize - pstrAuth1xTls->u16CertificateLen;
                tstrM2mWifi1xHdr    *pstr1xHdr = (tstrM2mWifi1xHdr*)OSAL_Malloc(u16Payload1Size);

                if (pstr1xHdr != NULL)
                {
                    tstrM2mWifiAuthInfoHdr strInfoHdr = {0};

                    uint8_t *pu8AuthPtr = pstr1xHdr->au81xAuthDetails;
                    memset((uint8_t*)pstr1xHdr, 0, u16Payload1Size);

                    pstr1xHdr->u8Flags = M2M_802_1X_TLS_FLAG;
                    if (pstrAuth1xTls->bUnencryptedUserName == true)
                        pstr1xHdr->u8Flags |= M2M_802_1X_UNENCRYPTED_USERNAME_FLAG;
                    if (pstrAuth1xTls->bPrependDomain == true)
                        pstr1xHdr->u8Flags |= M2M_802_1X_PREPEND_DOMAIN_FLAG;

                    pstr1xHdr->u8HdrLength = sizeof(tstrM2mWifi1xHdr);
                    pstr1xHdr->u32TlsHsFlags = gu321xTlsHsFlags;
                    memcpy(pstr1xHdr->au8TlsSpecificRootNameSha1, gau81xRootSha1, sizeof(gau81xRootSha1));

                    pstr1xHdr->u8DomainLength = 0;
                    if (pstrAuth1xTls->pu8Domain != NULL)
                    {
                        pstr1xHdr->u8DomainLength = (uint8_t)(pstrAuth1xTls->u16DomainLen);
                        memcpy(pu8AuthPtr, pstrAuth1xTls->pu8Domain, pstr1xHdr->u8DomainLength);
                        pu8AuthPtr += pstr1xHdr->u8DomainLength;
                    }

                    pstr1xHdr->u8UserNameLength = (pstrAuth1xTls->u16UserNameLen);
                    memcpy(pu8AuthPtr, pstrAuth1xTls->pu8UserName, pstr1xHdr->u8UserNameLength);
                    pu8AuthPtr += pstr1xHdr->u8UserNameLength;

                    pstr1xHdr->u16PrivateKeyOffset = pu8AuthPtr - pstr1xHdr->au81xAuthDetails;
                    pstr1xHdr->u16PrivateKeyLength = pstrAuth1xTls->u16PrivateKeyLen;
                    memcpy(pu8AuthPtr, pstrAuth1xTls->pu8PrivateKey_Mod, pstr1xHdr->u16PrivateKeyLength);
                    pu8AuthPtr += pstr1xHdr->u16PrivateKeyLength;
                    memcpy(pu8AuthPtr, pstrAuth1xTls->pu8PrivateKey_Exp, pstr1xHdr->u16PrivateKeyLength);
                    pu8AuthPtr += pstr1xHdr->u16PrivateKeyLength;

                    pstr1xHdr->u16CertificateOffset = pu8AuthPtr - pstr1xHdr->au81xAuthDetails;
                    pstr1xHdr->u16CertificateLength = pstrAuth1xTls->u16CertificateLen;

                    strInfoHdr.u8Type = M2M_802_1X_TLS_CLIENT_CERTIFICATE;
                    strInfoHdr.u16InfoPos = pstr1xHdr->u16CertificateOffset;
                    strInfoHdr.u16InfoLen = pstr1xHdr->u16CertificateLength;
                    ret = hif_send( M2M_REQ_GROUP_WIFI, M2M_WIFI_IND_CONN_PARAM | M2M_REQ_DATA_PKT,
                                    (uint8_t*)&strInfoHdr, sizeof(tstrM2mWifiAuthInfoHdr),
                                    pstrAuth1xTls->pu8Certificate, pstrAuth1xTls->u16CertificateLen,
                                    sizeof(tstrM2mWifiAuthInfoHdr));

                    if (ret == M2M_SUCCESS)
                    {
                        ret = hif_send( M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_CONN | M2M_REQ_DATA_PKT,
                                        (uint8_t*)&strConnHdr, sizeof(tstrM2mWifiConnHdr),
                                        (uint8_t*)pstr1xHdr, u16Payload1Size,
                                        sizeof(tstrM2mWifiConnHdr));
                    }
                    OSAL_Free(pstr1xHdr);
                }
                else
                    ret = M2M_ERR_MEM_ALLOC;
            }
        }
    }
    return ret;
}

int8_t m2m_wifi_connect(char *pcSsid, uint8_t u8SsidLen, uint8_t u8SecType, void *pvAuthInfo, uint16_t u16Ch)
{
    return m2m_wifi_connect_sc(pcSsid, u8SsidLen, u8SecType, pvAuthInfo, u16Ch, 0);
}

int8_t m2m_wifi_connect_sc(char *pcSsid, uint8_t u8SsidLen, uint8_t u8SecType, void *pvAuthInfo, uint16_t u16Ch, uint8_t u8NoSaveCred)
{
    int8_t               s8Ret              = M2M_ERR_INVALID_ARG;
    tstrNetworkId       strNetworkId       = {NULL, (uint8_t*)pcSsid, u8SsidLen, (tenuM2mScanCh)u16Ch};
    tenuCredStoreOption enuCredStoreOption = u8NoSaveCred ? WIFI_CRED_DONTSAVE : WIFI_CRED_SAVE_ENCRYPTED;

    /* This API does not support SSIDs which contain '\0'. If there is a '\0' character within the
     * first u8SsidLen characters, then assume that the input u8SsidLen was incorrect - set length
     * to strlen(pcSsid) and continue. This is to avoid a change from the behaviour of previously
     * released drivers. */
    if (u8SsidLen < M2M_MAX_SSID_LEN)
        while (u8SsidLen--)
            if (strNetworkId.pu8Ssid[u8SsidLen] == 0)
                strNetworkId.u8SsidLen = u8SsidLen;

    switch ((tenuM2mSecType)u8SecType)
    {
    case M2M_WIFI_SEC_OPEN:
        s8Ret = m2m_wifi_connect_open(enuCredStoreOption, &strNetworkId);
        break;
    case M2M_WIFI_SEC_WPA_PSK:
        if (pvAuthInfo != NULL)
        {
            tstrAuthPsk strAuthPsk = {NULL, NULL, 0};
            uint16_t    len        = strlen(pvAuthInfo);

            if (len == M2M_MAX_PSK_LEN-1)
            {
                strAuthPsk.pu8Psk = (uint8_t*)pvAuthInfo;
            }
            else
            {
                strAuthPsk.pu8Passphrase   = (uint8_t*)pvAuthInfo;
                strAuthPsk.u8PassphraseLen = len;
            }
            s8Ret = m2m_wifi_connect_psk(enuCredStoreOption, &strNetworkId, &strAuthPsk);
        }
        break;
    case M2M_WIFI_SEC_WEP:
        if (pvAuthInfo != NULL)
        {
            tstrM2mWifiWepParams    *pstrWepParams = (tstrM2mWifiWepParams*)pvAuthInfo;
            tstrAuthWep             strAuthWep     = {pstrWepParams->au8WepKey, pstrWepParams->u8KeySz-1, pstrWepParams->u8KeyIndx};

            s8Ret = m2m_wifi_connect_wep(enuCredStoreOption, &strNetworkId, &strAuthWep);
        }
        break;
    case M2M_WIFI_SEC_802_1X:
        if (pvAuthInfo != NULL)
        {
            tstr1xAuthCredentials   *pstr1xParams    = (tstr1xAuthCredentials*)pvAuthInfo;
            tstrAuth1xMschap2       strAuth1xMschap2 = {NULL,
                                                        pstr1xParams->au8UserName,
                                                        pstr1xParams->au8Passwd,
                                                        0,
                                                        strlen((const char*)pstr1xParams->au8UserName),
                                                        strlen((const char*)pstr1xParams->au8Passwd),
                                                        false};

            s8Ret = m2m_wifi_connect_1x_mschap2(enuCredStoreOption, &strNetworkId, &strAuth1xMschap2);
        }
        break;
    default:
        break;
    }
    return s8Ret;
}

int8_t m2m_wifi_disconnect(void)
{
    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_DISCONNECT, NULL, 0, NULL, 0, 0);
}

int8_t m2m_wifi_set_mac_address(uint8_t au8MacAddress[6])
{
    tstrM2mSetMacAddress strTmp;
    memcpy((uint8_t*) strTmp.au8Mac, (uint8_t*) au8MacAddress, 6);
    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_MAC_ADDRESS,
                    (uint8_t*) &strTmp, sizeof(tstrM2mSetMacAddress), NULL, 0,0);
}

int8_t m2m_wifi_set_static_ip(tstrM2MIPConfig *pstrStaticIPConf)
{
    pstrStaticIPConf->u32DNS = NM_BSP_B_L_32(pstrStaticIPConf->u32DNS);
    pstrStaticIPConf->u32Gateway = NM_BSP_B_L_32(pstrStaticIPConf->u32Gateway);
    pstrStaticIPConf->u32StaticIP = NM_BSP_B_L_32(
                                        pstrStaticIPConf->u32StaticIP);
    pstrStaticIPConf->u32SubnetMask = NM_BSP_B_L_32(
                                          pstrStaticIPConf->u32SubnetMask);
    return hif_send(M2M_REQ_GROUP_IP, M2M_IP_REQ_STATIC_IP_CONF,
                    (uint8_t*) pstrStaticIPConf, sizeof(tstrM2MIPConfig), NULL, 0,0);
}

int8_t m2m_wifi_enable_dhcp(uint8_t u8DhcpEn )
{

    uint8_t u8Req;
    u8Req = u8DhcpEn ? M2M_IP_REQ_ENABLE_DHCP : M2M_IP_REQ_DISABLE_DHCP;
    return hif_send(M2M_REQ_GROUP_IP, u8Req, NULL, 0, NULL, 0, 0);
}

/*!
@fn         int8_t m2m_wifi_set_lsn_int(tstrM2mLsnInt * pstrM2mLsnInt);
@brief      Set the Wi-Fi listen interval for power save operation. It is represented in units
            of AP Beacon periods.
@param [in] pstrM2mLsnInt
            Structure holding the listen interval configurations.
@return     The function SHALL return 0 for success and a negative value otherwise.
@sa         tstrM2mLsnInt , m2m_wifi_set_sleep_mode
@pre        m2m_wifi_set_sleep_mode shall be called first
@warning    The Function called once after initialization.
*/
int8_t m2m_wifi_set_lsn_int(tstrM2mLsnInt *pstrM2mLsnInt)
{
    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_LSN_INT, (uint8_t*)pstrM2mLsnInt, sizeof(tstrM2mLsnInt), NULL, 0, 0);
}

int8_t m2m_wifi_set_cust_InfoElement(uint8_t *pau8M2mCustInfoElement)
{
    int8_t  ret = M2M_ERR_FAIL;
    if(pau8M2mCustInfoElement != NULL)
    {
        if((pau8M2mCustInfoElement[0] + 1) < M2M_CUST_IE_LEN_MAX)
        {
            ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_CUST_INFO_ELEMENT|M2M_REQ_DATA_PKT, (uint8_t*)pau8M2mCustInfoElement, pau8M2mCustInfoElement[0]+1, NULL, 0, 0);
        }
    }
    return ret;
}

int8_t m2m_wifi_set_scan_options(tstrM2MScanOption *ptstrM2MScanOption)
{
    int8_t  s8Ret = M2M_ERR_FAIL;
    if(m2m_validate_scan_options (ptstrM2MScanOption) == M2M_SUCCESS)
    {
        s8Ret =  hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_SCAN_OPTION, (uint8_t*)ptstrM2MScanOption, sizeof(tstrM2MScanOption),NULL, 0,0);
    }
    return s8Ret;
}

int8_t m2m_wifi_set_stop_scan_on_first(uint8_t u8StopScanOption)
{
    int8_t   s8Ret = M2M_ERR_FAIL;

    tstrM2MStopScanOption StopScanOption = { 0 };

    if(1 >= u8StopScanOption)
    {
        StopScanOption.u8StopOnFirstResult = u8StopScanOption;

        s8Ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_STOP_SCAN_OPTION, (uint8_t *)&StopScanOption, sizeof(tstrM2MStopScanOption), NULL, 0, 0);

        M2M_INFO("Scan will %s stop on first result.\r\n", StopScanOption.u8StopOnFirstResult ? "" : "NOT");
    }
    else
    {
        s8Ret = M2M_ERR_INVALID_ARG;
    }

    return s8Ret;
}

int8_t m2m_wifi_set_scan_region(uint16_t ScanRegion)
{
    int8_t  s8Ret = M2M_ERR_FAIL;
    tstrM2MScanRegion strScanRegion;
    strScanRegion.u16ScanRegion = ScanRegion;
    s8Ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_SCAN_REGION, (uint8_t*)&strScanRegion, sizeof(tstrM2MScanRegion),NULL, 0,0);
    return s8Ret;
}
int8_t m2m_wifi_request_scan(uint8_t ch)
{
    int8_t  s8Ret = M2M_SUCCESS;

    if(!gu8scanInProgress)
    {
        if(((ch >= M2M_WIFI_CH_1) && (ch <= M2M_WIFI_CH_14)) || (ch == M2M_WIFI_CH_ALL))
        {
            tstrM2MScan strtmp;
            strtmp.u8ChNum = ch;
            s8Ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SCAN, (uint8_t*)&strtmp, sizeof(tstrM2MScan),NULL, 0,0);
            if(s8Ret == M2M_SUCCESS)
            {
                gu8scanInProgress = 1;
            }
        }
        else
        {
            s8Ret = M2M_ERR_INVALID_ARG;
        }
    }
    else
    {
        s8Ret = M2M_ERR_SCAN_IN_PROGRESS;
    }
    return s8Ret;
}

int8_t m2m_wifi_request_scan_passive(uint8_t ch, uint16_t scan_time)
{
    int8_t  s8Ret = M2M_SUCCESS;

    if(!gu8scanInProgress)
    {
        if(((ch >= M2M_WIFI_CH_1) && (ch <= M2M_WIFI_CH_14)) || (ch == M2M_WIFI_CH_ALL))
        {
            tstrM2MScan strtmp;
            strtmp.u8ChNum = ch;

            strtmp.u16PassiveScanTime = scan_time;

            s8Ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_PASSIVE_SCAN, (uint8_t*)&strtmp, sizeof(tstrM2MScan),NULL, 0,0);
            if(s8Ret == M2M_SUCCESS)
            {
                gu8scanInProgress = 1;
            }
        }
        else
        {
            s8Ret = M2M_ERR_INVALID_ARG;
        }
    }
    else
    {
        s8Ret = M2M_ERR_SCAN_IN_PROGRESS;
    }
    return s8Ret;
}

int8_t m2m_wifi_request_scan_ssid_list(uint8_t ch,uint8_t *u8Ssidlist)
{
    int8_t  s8Ret = M2M_ERR_INVALID_ARG;

    if(!gu8scanInProgress)
    {
        if((((ch >= M2M_WIFI_CH_1) && (ch <= M2M_WIFI_CH_14)) || (ch == M2M_WIFI_CH_ALL))&&(u8Ssidlist != NULL))
        {
            tstrM2MScan strtmp;
            uint16_t u16Lsize = 0;
            uint8_t u8Apnum = u8Ssidlist[u16Lsize];
            if(u8Apnum <= MAX_HIDDEN_SITES)
            {
                u16Lsize++;
                while(u8Apnum)
                {
                    if(u8Ssidlist[u16Lsize] >= M2M_MAX_SSID_LEN) {
                        goto EXIT;
                    } else {
                        u16Lsize += u8Ssidlist[u16Lsize] + 1;
                        u8Apnum--;
                    }
                }
                strtmp.u8ChNum = ch;
                s8Ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SCAN_SSID_LIST|M2M_REQ_DATA_PKT, (uint8_t*)&strtmp, sizeof(tstrM2MScan),u8Ssidlist, u16Lsize,sizeof(tstrM2MScan));
                if(s8Ret == M2M_SUCCESS)
                {
                    gu8scanInProgress = 1;
                }
            }
        }
    }
    else
    {
        s8Ret = M2M_ERR_SCAN_IN_PROGRESS;
    }
EXIT:
    return s8Ret;
}
int8_t m2m_wifi_wps(uint8_t u8TriggerType,const char  *pcPinNumber)
{
    tstrM2MWPSConnect strtmp;

    /* Stop Scan if it is ongoing.
    */
    gu8scanInProgress = 0;
    strtmp.u8TriggerType = u8TriggerType;
    /*If WPS is using PIN METHOD*/
    if (u8TriggerType == WPS_PIN_TRIGGER)
        memcpy ((uint8_t*)strtmp.acPinNumber,(uint8_t*) pcPinNumber,8);
    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_WPS, (uint8_t*)&strtmp,sizeof(tstrM2MWPSConnect), NULL, 0,0);
}
int8_t m2m_wifi_wps_disable(void)
{
    int8_t ret = M2M_SUCCESS;
    ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_DISABLE_WPS, NULL, 0, NULL, 0, 0);
    return ret;
}

/*!
@fn         int8_t m2m_wifi_req_client_ctrl(uint8_t cmd)
@brief      Send a command to the PS Client (An WINC board running the ps_firmware),
            if the PS client send any commands it will be received in wifi_cb @ref M2M_WIFI_RESP_CLIENT_INFO.
@param[in]  cmd
                Control command sent from PS Server to PS Client (command values defined by the application).
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
@sa         m2m_wifi_req_server_init, M2M_WIFI_RESP_CLIENT_INFO
@pre        m2m_wifi_req_server_init should be called first
*/
int8_t m2m_wifi_req_client_ctrl(uint8_t u8Cmd)
{
    int8_t ret = M2M_SUCCESS;
#ifdef _PS_SERVER_
    tstrM2Mservercmd    strCmd;
    strCmd.u8cmd = u8Cmd;
    ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_CLIENT_CTRL, (uint8_t*)&strCmd, sizeof(tstrM2Mservercmd), NULL, 0, 0);
#else
    M2M_ERR("_PS_SERVER_ is not defined\r\n");
#endif
    return ret;
}

/*!
@fn         int8_t m2m_wifi_req_server_init(uint8_t ch)
@brief      Initialize the PS Server, The WINC support non secure communication with another WINC,
            (SERVER/CLIENT) through one byte command (probe request and probe response) without any connection setup.
@param[in]  ch
                Server listening channel
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
@sa         m2m_wifi_req_client_ctrl
@warning    The server mode can't be used with any other modes (STA/AP).
*/
int8_t m2m_wifi_req_server_init(uint8_t ch)
{
    int8_t ret = M2M_SUCCESS;
#ifdef _PS_SERVER_
    tstrM2mServerInit strServer;
    strServer.u8Channel = ch;
    ret = hif_send(M2M_REQ_GROUP_WIFI,M2M_WIFI_REQ_SERVER_INIT, (uint8_t*)&strServer, sizeof(tstrM2mServerInit), NULL, 0, 0);
#else
    M2M_ERR("_PS_SERVER_ is not defined\r\n");
#endif
    return ret;
}

int8_t m2m_wifi_p2p(uint8_t u8Channel)
{
    int8_t ret = M2M_SUCCESS;
    if((u8Channel == M2M_WIFI_CH_1) || (u8Channel == M2M_WIFI_CH_6) || (u8Channel == M2M_WIFI_CH_11))
    {
        tstrM2MP2PConnect strtmp;
        strtmp.u8ListenChannel = u8Channel;
        ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_ENABLE_P2P, (uint8_t*)&strtmp, sizeof(tstrM2MP2PConnect), NULL, 0,0);
    }
    else
    {
        M2M_ERR("Listen channel should only be M2M_WIFI_CH_1/6/11\r\n");
        ret = M2M_ERR_FAIL;
    }
    return ret;
}

int8_t m2m_wifi_p2p_disconnect(void)
{
    int8_t ret = M2M_SUCCESS;
    ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_DISABLE_P2P, NULL, 0, NULL, 0, 0);
    return ret;
}

int8_t m2m_wifi_enable_ap(const tstrM2MAPConfig* pstrM2MAPConfig)
{
    tstrM2MAPModeConfig strM2MAPModeConfig;

    memcpy((uint8_t*)&strM2MAPModeConfig.strApConfig, (uint8_t*)pstrM2MAPConfig, sizeof(tstrM2MAPConfig));
    memcpy(strM2MAPModeConfig.strApConfigExt.au8DefRouterIP, pstrM2MAPConfig->au8DHCPServerIP, 4);
    memcpy(strM2MAPModeConfig.strApConfigExt.au8DNSServerIP, pstrM2MAPConfig->au8DHCPServerIP, 4);

    strM2MAPModeConfig.strApConfigExt.au8SubnetMask[0] = 0;

    return m2m_wifi_enable_ap_ext(&strM2MAPModeConfig);
}

int8_t m2m_wifi_enable_ap_ext(const tstrM2MAPModeConfig *pstrM2MAPModeConfig)
{
    int8_t ret = M2M_ERR_FAIL;
    if(M2M_SUCCESS == m2m_validate_ap_parameters(pstrM2MAPModeConfig))
    {
        ret = hif_send(M2M_REQ_GROUP_WIFI, (M2M_REQ_DATA_PKT|M2M_WIFI_REQ_ENABLE_AP), NULL, 0, (uint8_t*)pstrM2MAPModeConfig, sizeof(tstrM2MAPModeConfig), 0);
    }
    return ret;
}

int8_t m2m_wifi_set_gains(tstrM2mWifiGainsParams* pstrM2mGain)
{
    int8_t ret = M2M_ERR_FAIL;
    if(pstrM2mGain != NULL)
    {
        ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_GAINS, (uint8_t*)pstrM2mGain, sizeof(tstrM2mWifiGainsParams), NULL, 0, 0);
    }
    return ret;
}

int8_t m2m_wifi_disable_ap(void)
{
    int8_t ret = M2M_SUCCESS;
    ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_DISABLE_AP, NULL, 0, NULL, 0, 0);
    return ret;
}

/*!
@fn          int8_t m2m_wifi_req_curr_rssi(void);
@brief       Request the current RSSI for the current connected AP,
             the response received in wifi_cb M2M_WIFI_RESP_CURRENT_RSSI
@sa          M2M_WIFI_RESP_CURRENT_RSSI
@return      The function shall return M2M_SUCCESS for success and a negative value otherwise.
*/
int8_t m2m_wifi_req_curr_rssi(void)
{
    int8_t ret = M2M_SUCCESS;
    ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_CURRENT_RSSI, NULL, 0, NULL, 0, 0);
    return ret;
}
int8_t m2m_wifi_send_ethernet_pkt(uint8_t *pu8Packet,uint16_t u16PacketSize)
{
    int8_t  s8Ret = -1;
    if((pu8Packet != NULL)&&(u16PacketSize>0))
    {
        tstrM2MWifiTxPacketInfo     strTxPkt;

        strTxPkt.u16PacketSize      = u16PacketSize;
        strTxPkt.u16HeaderLength    = M2M_ETHERNET_HDR_LEN;
        s8Ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SEND_ETHERNET_PACKET | M2M_REQ_DATA_PKT,
                        (uint8_t*)&strTxPkt, sizeof(tstrM2MWifiTxPacketInfo), pu8Packet, u16PacketSize, M2M_ETHERNET_HDR_OFFSET - M2M_HIF_HDR_OFFSET);
    }
    return s8Ret;
}

/*!
@fn          int8_t m2m_wifi_get_otp_mac_address(uint8_t *pu8MacAddr, uint8_t * pu8IsValid);
@brief       Request the MAC address stored on the OTP (one time programmable) memory of the device.
             (the function is Blocking until response received)
@param [out] pu8MacAddr
             Output MAC address buffer of 6 bytes size. Valid only if *pu8Valid=1.
@param [out] pu8IsValid
             A output boolean value to indicate the validity of pu8MacAddr in OTP.
             Output zero if the OTP memory is not programmed, non-zero otherwise.
@return      The function shall return M2M_SUCCESS for success and a negative value otherwise.
@sa          m2m_wifi_get_mac_address
@pre         m2m_wifi_init required to call any WIFI/socket function
*/
int8_t m2m_wifi_get_otp_mac_address(uint8_t *pu8MacAddr, uint8_t *pu8IsValid)
{
    int8_t ret = M2M_SUCCESS;
    ret = hif_chip_wake();
    if(ret == M2M_SUCCESS)
    {
        ret = nmi_get_otp_mac_address(pu8MacAddr, pu8IsValid);
        if(ret == M2M_SUCCESS)
        {
            ret = hif_chip_sleep();
        }
    }
    return ret;
}

/*!
@fn          int8_t m2m_wifi_get_mac_address(uint8_t *pu8MacAddr)
@brief       Request the current MAC address of the device (the working mac address).
             (the function is Blocking until response received)
@param [out] pu8MacAddr
             Output MAC address buffer of 6 bytes size.
@return      The function shall return M2M_SUCCESS for success and a negative value otherwise.
@sa          m2m_wifi_get_otp_mac_address
@pre         m2m_wifi_init required to call any WIFI/socket function
*/
int8_t m2m_wifi_get_mac_address(uint8_t *pu8MacAddr)
{
    int8_t ret = M2M_SUCCESS;
    ret = hif_chip_wake();
    if(ret == M2M_SUCCESS)
    {
        ret = nmi_get_mac_address(pu8MacAddr);
        if(ret == M2M_SUCCESS)
        {
            ret = hif_chip_sleep();
        }
    }
    return ret;
}

/*!
@fn          int8_t m2m_wifi_req_scan_result(uint8_t index);
@brief       Reads the AP information from the Scan Result list with the given index,
             the response received in wifi_cb M2M_WIFI_RESP_SCAN_RESULT,
             the response pointer should be casted with tstrM2mWifiscanResult structure
@param [in]  index
             Index for the requested result, the index range start from 0 till number of AP's found
@sa          tstrM2mWifiscanResult,m2m_wifi_get_num_ap_found,m2m_wifi_request_scan
@return      The function shall return M2M_SUCCESS for success and a negative value otherwise
@pre         m2m_wifi_request_scan need to be called first, then m2m_wifi_get_num_ap_found
             to get the number of AP's found
@warning     Function used only in STA mode only. the scan result updated only if scan request called,
             else it will be cashed in firmware for the host scan request result,
             which mean if large delay occur between the scan request and the scan result request,
             the result will not be up-to-date
*/
int8_t m2m_wifi_req_scan_result(uint8_t index)
{
    int8_t ret = M2M_SUCCESS;
    tstrM2mReqScanResult strReqScanRlt;
    strReqScanRlt.u8Index = index;
    ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SCAN_RESULT, (uint8_t*) &strReqScanRlt, sizeof(tstrM2mReqScanResult), NULL, 0, 0);
    return ret;
}

/*!
@fn          uint8_t m2m_wifi_get_num_ap_found(void);
@brief       Reads the number of AP's found in the last Scan Request,
             The function read the number of AP's from global variable which updated in the
             wifi_cb in M2M_WIFI_RESP_SCAN_DONE.
@sa          m2m_wifi_request_scan
@return      Return the number of AP's found in the last Scan Request.
@pre         m2m_wifi_request_scan need to be called first
@warning     That function need to be called in the wifi_cb in M2M_WIFI_RESP_SCAN_DONE,
             calling that function in any other place will return undefined/undated numbers.
             Function used only in STA mode only.
*/
uint8_t m2m_wifi_get_num_ap_found(void)
{
    return gu8ChNum;
}

/*!
@fn         uint8_t m2m_wifi_get_sleep_mode(void);
@brief      Get the current Power save mode.
@return     The current operating power saving mode.
@sa         tenuPowerSaveModes , m2m_wifi_set_sleep_mode
*/
uint8_t m2m_wifi_get_sleep_mode(void)
{
    return hif_get_sleep_mode();
}

/*!
@fn         int8_t m2m_wifi_set_sleep_mode(uint8_t PsTyp, uint8_t BcastEn);
@brief      Set the power saving mode for the WINC1500.
@param [in] PsTyp
            Desired power saving mode. Supported types are defined in tenuPowerSaveModes.
@param [in] BcastEn
            Broadcast reception enable flag.
            If it is 1, the WINC1500 must be awake each DTIM Beacon for receiving Broadcast traffic.
            If it is 0, the WINC1500 will not wakeup at the DTIM Beacon, but its wakeup depends only
            on the the configured Listen Interval.
@return     The function SHALL return 0 for success and a negative value otherwise.
@sa         tenuPowerSaveModes
@warning    The function called once after initialization.
*/
int8_t m2m_wifi_set_sleep_mode(uint8_t PsTyp, uint8_t BcastEn)
{
    int8_t ret = M2M_SUCCESS;
    tstrM2mPsType strPs;
    strPs.u8PsType = PsTyp;
    strPs.u8BcastEn = BcastEn;
    ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SLEEP, (uint8_t*) &strPs,sizeof(tstrM2mPsType), NULL, 0, 0);
    M2M_INFO("POWER SAVE %d\r\n",PsTyp);
    hif_set_sleep_mode(PsTyp);
    return ret;
}

/*!
@fn         int8_t m2m_wifi_request_sleep(void)
@brief      Request from WINC device to Sleep for specific time in the M2M_PS_MANUAL Power save mode (only).
@param [in] u32SlpReqTime
            Request Sleep in ms
@return     The function SHALL return M2M_SUCCESS for success and a negative value otherwise.
@sa         tenuPowerSaveModes , m2m_wifi_set_sleep_mode
@warning    the Function should be called in M2M_PS_MANUAL power save only
*/
int8_t m2m_wifi_request_sleep(uint32_t u32SlpReqTime)
{
    int8_t ret = M2M_SUCCESS;
    uint8_t psType;
    psType = hif_get_sleep_mode();
    if(psType == M2M_PS_MANUAL)
    {
        tstrM2mSlpReqTime strPs;
        strPs.u32SleepTime = u32SlpReqTime;
        ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_DOZE, (uint8_t*) &strPs,sizeof(tstrM2mSlpReqTime), NULL, 0, 0);
    }
    return ret;
}

/*!
@fn         int8_t m2m_wifi_set_device_name(uint8_t *pu8DeviceName, uint8_t u8DeviceNameLength);
@brief      Sets the WINC device name. The name string is used as a device name in DHCP hostname (option 12).
@param [in] pu8DeviceName
            Buffer holding the device name.
@param [in] u8DeviceNameLength
            Length of the device name.
@return     The function SHALL return M2M_SUCCESS for success and a negative value otherwise.
@warning    The Function called once after initialization.
*/
int8_t m2m_wifi_set_device_name(uint8_t *pu8DeviceName, uint8_t u8DeviceNameLength)
{
    tstrM2MDeviceNameConfig strDeviceName;
    if(u8DeviceNameLength >= M2M_DEVICE_NAME_MAX)
    {
        u8DeviceNameLength = M2M_DEVICE_NAME_MAX;
    }
    //pu8DeviceName[u8DeviceNameLength] = '\0';
    u8DeviceNameLength ++;
    memcpy(strDeviceName.au8DeviceName, pu8DeviceName, u8DeviceNameLength);
    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_DEVICE_NAME,
                    (uint8_t*)&strDeviceName, sizeof(tstrM2MDeviceNameConfig), NULL, 0,0);
}

/*!
@fn         int8_t m2m_wifi_configure_sntp(uint8_t *pu8NTPServerName, uint8_t u8NTPServerNameLength, tenuSNTPUseDHCP useDHCP);
@brief      Configures what NTP server the SNTP client should use.
@param [in] pu8NTPServerName
            Buffer holding the NTP server name. If the first character is an asterisk (*) then it will be treated as a server pool, where the asterisk will
            be replaced with an incrementing value from 0 to 3 each time a server fails (example: *.pool.ntp.org).
@param [in] u8NTPServerNameLength
            Length of the NTP server name. Should not exceed the maximum NTP server name length of @ref M2M_NTP_MAX_SERVER_NAME_LENGTH
@param [in] useDHCP
            Should the NTP server provided by the DHCP server be used.
@return     The function SHALL return M2M_SUCCESS for success and a negative value otherwise.
*/
int8_t m2m_wifi_configure_sntp(uint8_t *pu8NTPServerName, uint8_t u8NTPServerNameLength, tenuSNTPUseDHCP useDHCP)
{
    tstrM2MSNTPConfig strSNTPConfig;
    if(u8NTPServerNameLength > M2M_NTP_MAX_SERVER_NAME_LENGTH)
        return M2M_ERR_FAIL;

    memcpy((uint8_t*)strSNTPConfig.acNTPServer, pu8NTPServerName, u8NTPServerNameLength);
    strSNTPConfig.acNTPServer[u8NTPServerNameLength] = '\0';
    strSNTPConfig.enuUseDHCP                         = useDHCP;

    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_CONFIG_SNTP,
                    (uint8_t*)&strSNTPConfig, sizeof(tstrM2MSNTPConfig), NULL, 0,0);
}

/*!
@fn \
    uint32_t m2m_wifi_get_chipId(void)

@brief
    Get the WINC Chip ID.

@return
    The function SHALL return chipID >0 or 0 for failure.
*/
uint32_t m2m_wifi_get_chipId(void)
{
    return nmi_get_chipid();
}
/*!
@fn     int8_t m2m_wifi_get_firmware_version(tstrM2mRev* pstrRev)

@brief
    Synchronous API to obtain the firmware version currently running on the WINC IC

@param [out]    pstrRev
    pointer holds address of structure "tstrM2mRev" that contains the firmware version parameters

@return
    The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
*/
int8_t m2m_wifi_get_firmware_version(tstrM2mRev *pstrRev)
{
    int8_t ret = M2M_SUCCESS;
    ret = hif_chip_wake();
    if(ret == M2M_SUCCESS)
    {
        ret = nm_get_firmware_full_info(pstrRev);
        hif_chip_sleep();
    }
    return ret;
}

int8_t m2m_wifi_start_provision_mode(tstrM2MAPConfig *pstrM2MAPConfig, char *pcHttpServerDomainName, uint8_t bEnableHttpRedirect)
{
    tstrM2MAPModeConfig strM2MAPModeConfig;

    memcpy((uint8_t*)&strM2MAPModeConfig.strApConfig, (uint8_t*)pstrM2MAPConfig, sizeof(tstrM2MAPConfig));
    memcpy(strM2MAPModeConfig.strApConfigExt.au8DefRouterIP, pstrM2MAPConfig->au8DHCPServerIP, 4);
    memcpy(strM2MAPModeConfig.strApConfigExt.au8DNSServerIP, pstrM2MAPConfig->au8DHCPServerIP, 4);

    strM2MAPModeConfig.strApConfigExt.au8SubnetMask[0] = 0;

    return m2m_wifi_start_provision_mode_ext(&strM2MAPModeConfig, pcHttpServerDomainName, bEnableHttpRedirect);
}

int8_t m2m_wifi_start_provision_mode_ext(tstrM2MAPModeConfig *pstrAPModeConfig, char *pcHttpServerDomainName, uint8_t bEnableHttpRedirect)
{
    int8_t  s8Ret = M2M_ERR_FAIL;

    if(pstrAPModeConfig != NULL)
    {
        tstrM2MProvisionModeConfig  strProvConfig;
        if(M2M_SUCCESS == m2m_validate_ap_parameters(pstrAPModeConfig))
        {
            memcpy((uint8_t*)&strProvConfig.strApConfig, (uint8_t*)&pstrAPModeConfig->strApConfig, sizeof(tstrM2MAPConfig));
            memcpy((uint8_t*)&strProvConfig.strApConfigExt, (uint8_t*)&pstrAPModeConfig->strApConfigExt, sizeof(tstrM2MAPConfigExt));
            if((strlen((const char*)pcHttpServerDomainName) <= 0) || (NULL == pcHttpServerDomainName))
            {
                M2M_ERR("INVALID DOMAIN NAME\r\n");
                goto ERR1;
            }
            memcpy((uint8_t*)strProvConfig.acHttpServerDomainName, (uint8_t*)pcHttpServerDomainName, 64);
            strProvConfig.u8EnableRedirect = bEnableHttpRedirect;

            /* Stop Scan if it is ongoing.
            */
            gu8scanInProgress = 0;
            s8Ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_START_PROVISION_MODE | M2M_REQ_DATA_PKT,
                        (uint8_t*)&strProvConfig, sizeof(tstrM2MProvisionModeConfig), NULL, 0, 0);
        }
        else
        {
            /*goto ERR1;*/
        }
    }
ERR1:
    return s8Ret;
}

int8_t m2m_wifi_stop_provision_mode(void)
{
    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_STOP_PROVISION_MODE, NULL, 0, NULL, 0, 0);
}

int8_t m2m_wifi_get_connection_info(void)
{
    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_GET_CONN_INFO, NULL, 0, NULL, 0, 0);
}

int8_t m2m_wifi_set_system_time(uint32_t u32UTCSeconds)
{
    /*
        The firmware accepts timestamps relative to 1900 like NTP Timestamp.
    */
    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_SYS_TIME, (uint8_t*)&u32UTCSeconds, sizeof(tstrSystemTime), NULL, 0, 0);
}
/*!
 * @fn             int8_t m2m_wifi_get_system_time(void);
 * @see            m2m_wifi_enable_sntp
                    tstrSystemTime
 * @note         get the system time from the sntp client
 *               using the API \ref m2m_wifi_get_system_time.
 * @return        The function returns @ref M2M_SUCCESS for successful operations and a negative value otherwise.
 */
int8_t m2m_wifi_get_system_time(void)
{
    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_GET_SYS_TIME, NULL, 0, NULL, 0, 0);
}

int8_t m2m_wifi_enable_sntp(uint8_t bEnable)
{
    uint8_t u8Req;
    u8Req = bEnable ? M2M_WIFI_REQ_ENABLE_SNTP_CLIENT : M2M_WIFI_REQ_DISABLE_SNTP_CLIENT;
    return hif_send(M2M_REQ_GROUP_WIFI, u8Req, NULL, 0, NULL, 0, 0);
}

/*!
@fn         int8_t m2m_wifi_set_power_profile(uint8_t u8PwrMode);
@brief      Change the power profile mode
@param [in] u8PwrMode
            Change the WINC power profile to different mode
            PWR_LOW1/PWR_LOW2/PWR_HIGH/PWR_AUTO (tenuM2mPwrMode)
@return     The function SHALL return M2M_SUCCESS for success and a negative value otherwise.
@sa         tenuM2mPwrMode
@pre        m2m_wifi_init
@warning    must be called after the initializations and before any connection request and can't be changed in run time,
*/
int8_t m2m_wifi_set_power_profile(uint8_t u8PwrMode)
{
    int8_t ret = M2M_SUCCESS;
    tstrM2mPwrMode strM2mPwrMode;
    strM2mPwrMode.u8PwrMode = u8PwrMode;
    ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_POWER_PROFILE, (uint8_t*)&strM2mPwrMode,sizeof(tstrM2mPwrMode), NULL, 0, 0);
    return ret;
}

/*!
@fn         int8_t m2m_wifi_set_tx_power(uint8_t u8TxPwrLevel);
@brief      set the TX power tenuM2mTxPwrLevel
@param [in] u8TxPwrLevel
            change the TX power tenuM2mTxPwrLevel
@return     The function SHALL return M2M_SUCCESS for success and a negative value otherwise.
@sa         tenuM2mTxPwrLevel
@pre        m2m_wifi_init
@warning
*/
int8_t m2m_wifi_set_tx_power(uint8_t u8TxPwrLevel)
{
    int8_t ret = M2M_SUCCESS;
    tstrM2mTxPwrLevel strM2mTxPwrLevel;
    strM2mTxPwrLevel.u8TxPwrLevel = u8TxPwrLevel;
    ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_TX_POWER, (uint8_t*)&strM2mTxPwrLevel,sizeof(tstrM2mTxPwrLevel), NULL, 0, 0);
    return ret;
}

/*!
@fn         int8_t m2m_wifi_set_gain_table_idx(uint8_t u8GainTableIdx);
@brief      set the gain table index corresponding to a specific WiFi region
@param [in] u8GainTableIdx
            change the gain table index
@return     The function SHALL return M2M_SUCCESS for success and a negative value otherwise.
@pre        The gain tables must be written to the flash through gain builder tool.
            m2m_wifi_init
@warning
*/
int8_t m2m_wifi_set_gain_table_idx(uint8_t u8GainTableIdx)
{
    int8_t ret = M2M_SUCCESS;
    tstrM2mWiFiGainIdx strM2mGainTableIdx;
    strM2mGainTableIdx.u8GainTableIdx = u8GainTableIdx;
    ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_GAIN_TABLE_IDX, (uint8_t*)&strM2mGainTableIdx,sizeof(tstrM2mWiFiGainIdx), NULL, 0, 0);
    return ret;
}

/*!
@fn         int8_t m2m_wifi_enable_firmware_logs(uint8_t u8Enable);
@brief      Enable or Disable logs in run time (Disable Firmware logs will
            enhance the firmware start-up time and performance)
@param [in] u8Enable
            Set 1 to enable the logs 0 for disable
@return     The function SHALL return M2M_SUCCESS for success and a negative value otherwise.
@sa         __DISABLE_FIRMWARE_LOGS__ (build option to disable logs from initializations)
@pre        m2m_wifi_init
@warning
*/
int8_t m2m_wifi_enable_firmware_logs(uint8_t u8Enable)
{
    int8_t ret = M2M_SUCCESS;
    tstrM2mEnableLogs strM2mEnableLogs;
    strM2mEnableLogs.u8Enable = u8Enable;
    ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_ENABLE_LOGS, (uint8_t*)&strM2mEnableLogs,sizeof(tstrM2mEnableLogs), NULL, 0, 0);
    return ret;
}

/*!
@fn         int8_t m2m_wifi_set_battery_voltage(uint16_t u16BattVoltx100);
@brief      Enable or Disable logs in run time (Disable Firmware logs will
            enhance the firmware start-up time and performance)
@param [in] u16BattVoltx100
            battery voltage multiplied by 100
@return     The function SHALL return M2M_SUCCESS for success and a negative value otherwise.
@sa         __DISABLE_FIRMWARE_LOGS__ (build option to disable logs from initializations)
@pre        m2m_wifi_init
@warning
*/
int8_t m2m_wifi_set_battery_voltage(uint16_t u16BattVoltx100)
{
    int8_t ret = M2M_SUCCESS;
    tstrM2mBatteryVoltage strM2mBattVol = {0};
    strM2mBattVol.u16BattVolt = u16BattVoltx100;
    ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_BATTERY_VOLTAGE, (uint8_t*)&strM2mBattVol,sizeof(tstrM2mBatteryVoltage), NULL, 0, 0);
    return ret;
}

/*!
@fn              int8_t m2m_wifi_prng_get_random_bytes(uint8_t * pu8PrngBuff,uint16_t u16PrngSize)
@brief       Get random bytes using the PRNG bytes.
@param [in]    u16PrngSize
             Size of the required random bytes to be generated.
@param [in]    pu8PrngBuff
                Pointer to user allocated buffer.
@return           The function SHALL return M2M_SUCCESS for success and a negative value otherwise.
*/
int8_t m2m_wifi_prng_get_random_bytes(uint8_t * pu8PrngBuff,uint16_t u16PrngSize)
{
    int8_t ret = M2M_ERR_FAIL;
    tstrPrng   strRng = {0};
    if(
            (u16PrngSize <= (M2M_HIF_MAX_PACKET_SIZE - (M2M_HIF_HDR_OFFSET + sizeof(tstrPrng))))
        &&  (pu8PrngBuff != NULL)
    )
    {
        strRng.u16PrngSize = u16PrngSize;
        strRng.pu8RngBuff  = pu8PrngBuff;
        ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_GET_PRNG|M2M_REQ_DATA_PKT,(uint8_t*)&strRng, sizeof(tstrPrng),NULL,0, 0);
    }
    else
    {
        M2M_ERR("PRNG Buffer exceeded maximum size %d or NULL Buffer\r\n",u16PrngSize);
    }
    return ret;
}

/*!
@fn \
     int8_t m2m_wifi_conf_auto_rate(tstrConfAutoRate * pstrConfAutoRate)

@brief
    Allow the host MCU app to configure auto TX rate selection algorithm. The application can use this
    API to tweak the algorithm performance. Moreover, it allows the application to force a specific WLAN
    PHY rate for transmitted data packets to favor range vs. throughput needs.

@param [in] pstrConfAutoRate
    The Auto rate configuration parameters as listed in tstrConfAutoRate.

@warning
    The application will be responsible for keeping track of what values are set and, if required,
    going back to default values. Changes will become permanent until the chip is power-cycled.

@sa
    tstrConfAutoRate

@return
    The function SHALL return 0 for success and a negative value otherwise.
*/

int8_t m2m_wifi_conf_auto_rate(tstrConfAutoRate * pstrConfAutoRate)
{
    int8_t s8ret = M2M_ERR_FAIL;
    s8ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_CONG_AUTO_RATE, (uint8_t*)pstrConfAutoRate,sizeof(tstrConfAutoRate),NULL,0,0);

    return s8ret;
}

/*!
@fn \
     int8_t m2m_wifi_enable_mac_mcast(uint8_t* pu8MulticastMacAddress, uint8_t u8AddRemove)

@brief
    Add MAC filter to receive Multicast packets.

@param [in] pu8MulticastMacAddress
                Pointer to the MAC address.
@param [in] u8AddRemove
                Flag to Add/Remove MAC address.
@return
    The function SHALL return 0 for success and a negative value otherwise.
*/

int8_t m2m_wifi_enable_mac_mcast(uint8_t* pu8MulticastMacAddress, uint8_t u8AddRemove)
{
    int8_t s8ret = M2M_ERR_FAIL;
    tstrM2MMulticastMac  strMulticastMac;

    if(pu8MulticastMacAddress != NULL)
    {
        strMulticastMac.u8AddRemove = u8AddRemove;
        memcpy(strMulticastMac.au8macaddress,pu8MulticastMacAddress,M2M_MAC_ADDRES_LEN);
        M2M_DBG("mac multicast: %x:%x:%x:%x:%x:%x\r\n", strMulticastMac.au8macaddress[0], strMulticastMac.au8macaddress[1], strMulticastMac.au8macaddress[2], strMulticastMac.au8macaddress[3], strMulticastMac.au8macaddress[4], strMulticastMac.au8macaddress[5]);
        s8ret = hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_SET_MAC_MCAST, (uint8_t*)&strMulticastMac,sizeof(tstrM2MMulticastMac),NULL,0,0);
    }

    return s8ret;
}

/*!
@fn \
    int8_t  m2m_wifi_set_receive_buffer(void* pvBuffer,uint16_t u16BufferLen);

@brief
    set the ethernet receive buffer, should be called in the receive call back.

@param [in] pvBuffer
                Pointer to the ethernet receive buffer.
@param [in] u16BufferLen
                Length of the buffer.

@return
    The function SHALL return 0 for success and a negative value otherwise.
*/
int8_t  m2m_wifi_set_receive_buffer(void *pvBuffer,uint16_t u16BufferLen)
{
    int8_t s8ret = M2M_SUCCESS;
    if(pvBuffer != NULL)
    {
        gau8ethRcvBuf = pvBuffer;
        gu16ethRcvBufSize= u16BufferLen;
    }
    else
    {
        s8ret = M2M_ERR_FAIL;
        M2M_ERR("Buffer NULL pointer\r\n");
    }
    return s8ret;
}

int8_t m2m_wifi_reinit(tstrWifiInitParam *pWifiInitParam)
{
    int8_t ret = m2m_wifi_reinit_hold();
    if(M2M_SUCCESS == ret)
        ret = m2m_wifi_reinit_start(pWifiInitParam);

    return ret;
}

int8_t m2m_wifi_reinit_hold(void)
{
    m2m_wifi_deinit(NULL);
    return m2m_wifi_init_hold();
}

int8_t m2m_wifi_reinit_start(tstrWifiInitParam *pWifiInitParam)
{
    return m2m_wifi_init_start(pWifiInitParam);
}

uint8_t m2m_wifi_get_state(void)
{
    return gu8WifiState;
}

int8_t m2m_wifi_enable_roaming(uint8_t bEnableDhcp)
{
    tstrM2mWiFiRoaming  strWiFiRoaming;
    strWiFiRoaming.u8EnableRoaming = 1;
    if(0 == bEnableDhcp || 1 == bEnableDhcp)
    {
        strWiFiRoaming.u8EnableDhcp = bEnableDhcp;
        return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_ROAMING,
            (uint8_t*) &strWiFiRoaming, sizeof(tstrM2mWiFiRoaming), NULL, 0, 0);
    }
    else
    {
        return M2M_ERR_INVALID_ARG;
    }
}

int8_t m2m_wifi_disable_roaming(void)
{
    tstrM2mWiFiRoaming  strWiFiRoaming;
    strWiFiRoaming.u8EnableRoaming = 0;
    return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_ROAMING, (uint8_t*) &strWiFiRoaming, sizeof(tstrM2mWiFiRoaming), NULL, 0,0);
}

int8_t m2m_wifi_enable_XO_during_sleep(uint8_t bXOSleepEnable)
{
    tstrM2mWiFiXOSleepEnable  strM2mWiFiXOSleepEnable;

    if(0 == bXOSleepEnable || 1 == bXOSleepEnable)
    {
        strM2mWiFiXOSleepEnable.u8EnableXODuringSleep = bXOSleepEnable;
        return hif_send(M2M_REQ_GROUP_WIFI, M2M_WIFI_REQ_XO_SLEEP_ENABLE, 
                        (uint8_t *) &strM2mWiFiXOSleepEnable, sizeof(strM2mWiFiXOSleepEnable), NULL, 0, 0);	
    }
    else
    {
        return M2M_ERR_INVALID_ARG;
    }
}

//DOM-IGNORE-END

/**
 *
 * \file
 *
 * \brief This module contains M2M Wi-Fi SSL APIs implementation.
 *
 * Copyright (c) 2017-2021 Microchip Technology Inc. and its subsidiaries.
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

#include "driver/include/m2m_ssl.h"
#include "driver/include/m2m_hif.h"
#include "driver/include/nmasic.h"
#define min(a,b) \
    ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
DATA TYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
static tpfAppSSLCb gpfAppSSLCb       = NULL;
static uint32 gu32HIFAddr            = 0;
static tenuTlsFlashStatus genuStatus = TLS_FLASH_ERR_UNKNOWN;

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
FUNCTION PROTOTYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/*!
@fn         void m2m_ssl_cb(uint8 u8OpCode, uint16 u16DataSize, uint32 u32Addr)
@brief      Internal SSL callback function.
@param [in] u8OpCode
                HIF Opcode type.
@param [in] u16DataSize
                HIF data length.
@param [in] u32Addr
            HIF address.
*/
static void m2m_ssl_cb(uint8 u8OpCode, uint16 u16DataSize, uint32 u32Addr)
{
    sint8 s8tmp = M2M_SUCCESS;
    switch(u8OpCode)
    {
    case M2M_SSL_REQ_ECC:
    {
        tstrEccReqInfo strEccREQ;
        s8tmp = hif_receive(u32Addr, (uint8 *)&strEccREQ, sizeof(tstrEccReqInfo), 0);
        if(s8tmp == M2M_SUCCESS)
        {
            if(gpfAppSSLCb)
            {
                gu32HIFAddr = u32Addr + sizeof(tstrEccReqInfo);
                gpfAppSSLCb(M2M_SSL_REQ_ECC, &strEccREQ);
            }
        }
    }
    break;
    case M2M_SSL_RESP_SET_CS_LIST:
    {
        tstrSslSetActiveCsList strCsList;
        s8tmp = hif_receive(u32Addr, (uint8 *)&strCsList, sizeof(tstrSslSetActiveCsList), 0);
        if(s8tmp == M2M_SUCCESS)
        {
            if(gpfAppSSLCb)
                gpfAppSSLCb(M2M_SSL_RESP_SET_CS_LIST, &strCsList);
        }
    }
    break;
    case M2M_SSL_RESP_WRITE_OWN_CERTS:
    {
        tstrTlsSrvChunkHdr strTlsSrvChunkRsp;
        uint8 bCallApp = 1;

        s8tmp = hif_receive(u32Addr, (uint8 *)&strTlsSrvChunkRsp, sizeof(tstrTlsSrvChunkHdr), 0);
        if(s8tmp == M2M_SUCCESS)
        {
            uint16 offset = strTlsSrvChunkRsp.u16Offset32;
            uint16 chunk_size = strTlsSrvChunkRsp.u16Size32;
            uint16 total_size = strTlsSrvChunkRsp.u16TotalSize32;
            tenuTlsFlashStatus status = (tenuTlsFlashStatus)(strTlsSrvChunkRsp.u16Sig);

            /* If first chunk, reset status. */
            if(offset == 0)
                genuStatus = TLS_FLASH_OK_NO_CHANGE;
            /* Only send status to app when processing last chunk. */
            if(offset + chunk_size != total_size)
                bCallApp = 0;

            switch(status)
            {
            case TLS_FLASH_OK:
                // Good flash write. Update status if no errors yet.
                if(genuStatus == TLS_FLASH_OK_NO_CHANGE)
                    genuStatus = status;
                break;
            case TLS_FLASH_OK_NO_CHANGE:
                // No change, don't update status.
                break;
            case TLS_FLASH_ERR_CORRUPT:
                // Corrupt. Always update status.
                genuStatus = status;
                break;
            case TLS_FLASH_ERR_NO_CHANGE:
                // Failed flash write. Update status if no more serious error.
                if((genuStatus != TLS_FLASH_ERR_CORRUPT) && (genuStatus != TLS_FLASH_ERR_UNKNOWN))
                    genuStatus = status;
                break;
            default:
                // Don't expect any other case. Ensure we don't mask a previous corrupt error.
                if(genuStatus != TLS_FLASH_ERR_CORRUPT)
                    genuStatus = TLS_FLASH_ERR_UNKNOWN;
                break;
            }
        }
        if(bCallApp && gpfAppSSLCb)
            gpfAppSSLCb(M2M_SSL_RESP_WRITE_OWN_CERTS, &genuStatus);
    }
    break;
    }
    if(s8tmp != M2M_SUCCESS)
    {
        M2M_ERR("Error receiving SSL from the HIF\n");
    }
}

/*!
@fn         NMI_API sint8 m2m_ssl_init(tpfAppSSLCb pfAppSSLCb)
@brief      Initializes the SSL layer.
@param [in] pfAppSslCb
                Application SSL callback function.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ssl_init(tpfAppSSLCb pfAppSSLCb)
{
    sint8 s8Ret = M2M_SUCCESS;
    gpfAppSSLCb = pfAppSSLCb;
    gu32HIFAddr = 0;
    genuStatus = TLS_FLASH_ERR_UNKNOWN;
    s8Ret = hif_register_cb(M2M_REQ_GROUP_SSL, m2m_ssl_cb);
    if(s8Ret != M2M_SUCCESS)
    {
        M2M_ERR("hif_register_cb() failed with ret=%d", s8Ret);
    }
    return s8Ret;
}

/*!
@fn         NMI_API sint8 m2m_ssl_handshake_rsp(tstrEccReqInfo* strECCResp, uint8* pu8RspDataBuff, uint16 u16RspDataSz)
@brief      Sends ECC responses to the WINC.
@param[in]  strECCResp
                ECC Response struct.
@param[in]  pu8RspDataBuff
                Pointer of the response data to be sent.
@param[in]  u16RspDataSz
                Response data size.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ssl_handshake_rsp(tstrEccReqInfo *strECCResp, uint8 *pu8RspDataBuff, uint16 u16RspDataSz)
{
    sint8 s8Ret = M2M_SUCCESS;

    s8Ret = hif_send(M2M_REQ_GROUP_SSL, (M2M_SSL_RESP_ECC | M2M_REQ_DATA_PKT), (uint8 *)strECCResp, sizeof(tstrEccReqInfo), pu8RspDataBuff, u16RspDataSz, sizeof(tstrEccReqInfo));

    return s8Ret;
}

/*!
@fn         NMI_API sint8 m2m_ssl_send_certs_to_winc(uint8 *pu8Buffer, uint32 u32BufferSz)
@brief      Sends certificates to the WINC
@param[in]  pu8Buffer
                Pointer to the certificates.
@param[in]  u32BufferSz
                Size of the certificates.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ssl_send_certs_to_winc(uint8 *pu8Buffer, uint32 u32BufferSz)
{
    sint8 s8Ret = M2M_SUCCESS;
#define TXLIMIT  (256 * 6)

    if(u32BufferSz <= TXLIMIT)
    {
        // set chunk header for one chunk
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
        tstrTlsSrvChunkHdr *pchkhdr = (tstrTlsSrvChunkHdr *)pu8Buffer;
#pragma GCC diagnostic pop
        pchkhdr->u16Sig = TLS_CERTS_CHUNKED_SIG_VALUE;
        pchkhdr->u16TotalSize32 = (u32BufferSz + 3) >> 2;
        pchkhdr->u16Offset32 = 0;
        pchkhdr->u16Size32 = (u32BufferSz + 3) >> 2;
        s8Ret = hif_send(M2M_REQ_GROUP_SSL, (M2M_SSL_REQ_WRITE_OWN_CERTS | M2M_REQ_DATA_PKT), NULL, 0, pu8Buffer, u32BufferSz, 0);
        M2M_INFO("Transferred %lu bytes of cert data NON-CHUNKED\n", u32BufferSz);
    }
    else
    {
        // chunk it
        // We are sneaking in a header - tstrTlsSrvChunkHdr
#define CHUNKHDRSZ (sizeof(tstrTlsSrvChunkHdr))
#define CHUNKSZ    (TXLIMIT - 256) // divisible by 4
        uint8 saveblob[CHUNKHDRSZ];
        uint32 ofs = 0;
        uint32 thischunksz = 0;

        // first is special - over writing our header
        m2m_memcpy(saveblob, &pu8Buffer[ofs], CHUNKHDRSZ);
        thischunksz = min(CHUNKSZ, u32BufferSz-ofs); // no need to round up to quad words this time
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
        tstrTlsSrvChunkHdr *pchkhdr = (tstrTlsSrvChunkHdr *)&pu8Buffer[ofs];
#pragma GCC diagnostic pop
        pchkhdr->u16Sig = TLS_CERTS_CHUNKED_SIG_VALUE;
        pchkhdr->u16TotalSize32 = ((u32BufferSz + 3) >> 2);
        pchkhdr->u16Offset32 = ((ofs + 3) >> 2);
        pchkhdr->u16Size32 = ((thischunksz + 3) >> 2);
        s8Ret = hif_send(M2M_REQ_GROUP_SSL, (M2M_SSL_REQ_WRITE_OWN_CERTS | M2M_REQ_DATA_PKT), NULL, 0, &pu8Buffer[ofs], thischunksz, 0);
        M2M_INFO("Transferred %u bytes of cert data CHUNKED to offset %u total %u\n", thischunksz, ofs, u32BufferSz);
        m2m_memcpy(&pu8Buffer[ofs], saveblob, CHUNKHDRSZ);
        ofs += thischunksz;

        while(ofs < u32BufferSz)
        {
            // Subsequent chunks write header before and send a little more
            m2m_memcpy(saveblob, &pu8Buffer[ofs-CHUNKHDRSZ], CHUNKHDRSZ);
            thischunksz = min(CHUNKSZ, u32BufferSz-ofs);
            thischunksz = (thischunksz + 3) & 0xFFFFFFFC; // needs to round up to quad word length
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
            pchkhdr = (tstrTlsSrvChunkHdr *)&pu8Buffer[ofs - CHUNKHDRSZ];
#pragma GCC diagnostic pop
            pchkhdr->u16Sig = TLS_CERTS_CHUNKED_SIG_VALUE;
            pchkhdr->u16TotalSize32 = ((u32BufferSz + 3) >> 2);
            pchkhdr->u16Offset32 = ((ofs + 3) >> 2);
            pchkhdr->u16Size32 = ((thischunksz + 3) >> 2);
            s8Ret = hif_send(M2M_REQ_GROUP_SSL, (M2M_SSL_REQ_WRITE_OWN_CERTS | M2M_REQ_DATA_PKT), NULL, 0, &pu8Buffer[ofs - CHUNKHDRSZ], thischunksz + CHUNKHDRSZ, 0);
            M2M_INFO("Transferred %lu bytes of cert data CHUNKED to offset %u total %u\n", thischunksz, ofs, u32BufferSz);
            m2m_memcpy(&pu8Buffer[ofs - CHUNKHDRSZ], saveblob, CHUNKHDRSZ);
            ofs += thischunksz;
        }
    }

    return s8Ret;
}

/*!
@fn         NMI_API sint8 m2m_ssl_retrieve_next_for_verifying(tenuEcNamedCurve *penuCurve, uint8 *pu8Value, uint16 *pu16ValueSz, uint8 *pu8Sig, uint16 *pu16SigSz, tstrECPoint *pstrKey);
@brief      Retrieve the next set of information from the WINC for ECDSA verification.
@param[out] penuCurve
                The named curve.
@param[out] pu8Value
                Value retrieved for verification. This is the digest of the message, truncated/prepended to the appropriate size.
@param[inout] pu16ValueSz
                in: Size of value buffer provided by caller.
                out: Size of value retrieved (provided for convenience; the value size is in fact determined by the curve).
@param[out] pu8Sig
                Signature retrieved for verification.
@param[inout] pu16SigSz
                in: Size of signature buffer provided by caller.
                out: Size of signature retrieved (provided for convenience; the signature size is in fact determined by the curve).
@param[out] pstrKey
                Public key retrieved for verification.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.

@pre        This function should only be called after the application has been notified that
            verification information is ready via @ref ECC_REQ_SIGN_VERIFY.

@warning    If this function returns @ref M2M_ERR_FAIL, then any remaining verification info from
            the WINC is lost.
*/
NMI_API sint8 m2m_ssl_retrieve_next_for_verifying(tenuEcNamedCurve *penuCurve, uint8 *pu8Value, uint16 *pu16ValueSz, uint8 *pu8Sig, uint16 *pu16SigSz, tstrECPoint *pstrKey)
{
    sint8   s8Ret = M2M_ERR_FAIL;
    uint16  u16HashSz, u16SigSz, u16KeySz;

    if(gu32HIFAddr == 0) return M2M_ERR_FAIL;

    if((NULL == penuCurve) || (NULL == pu8Value) || (NULL == pu16ValueSz) || (NULL == pu8Sig) || (NULL == pu16SigSz) || (NULL == pstrKey))
    {
        s8Ret = M2M_ERR_INVALID_ARG;
        goto __ERR;
    }

    if(hif_receive(gu32HIFAddr, (uint8 *)&u16KeySz, 2, 0) != M2M_SUCCESS) goto __ERR;
    *penuCurve = _htons(u16KeySz);
    gu32HIFAddr += 2;

    if(hif_receive(gu32HIFAddr, (uint8 *)&u16KeySz, 2, 0) != M2M_SUCCESS) goto __ERR;
    u16KeySz = _htons(u16KeySz);
    if(u16KeySz > sizeof(pstrKey->X)) goto __ERR;
    pstrKey->u16Size = u16KeySz;
    gu32HIFAddr += 2;

    if(hif_receive(gu32HIFAddr, (uint8 *)&u16HashSz, 2, 0) != M2M_SUCCESS) goto __ERR;
    u16HashSz = _htons(u16HashSz);
    if(u16HashSz > *pu16ValueSz) goto __ERR;
    *pu16ValueSz = u16HashSz;
    gu32HIFAddr += 2;

    if(hif_receive(gu32HIFAddr, (uint8 *)&u16SigSz, 2, 0) != M2M_SUCCESS) goto __ERR;
    u16SigSz = _htons(u16SigSz);
    if(u16SigSz > *pu16SigSz) goto __ERR;
    *pu16SigSz = u16SigSz;
    gu32HIFAddr += 2;

    if(hif_receive(gu32HIFAddr, pstrKey->X, u16KeySz, 0) != M2M_SUCCESS) goto __ERR;
    gu32HIFAddr += u16KeySz;
    if(hif_receive(gu32HIFAddr, pstrKey->Y, u16KeySz, 0) != M2M_SUCCESS) goto __ERR;
    gu32HIFAddr += u16KeySz;

    if(hif_receive(gu32HIFAddr, pu8Value, u16HashSz, 0) != M2M_SUCCESS) goto __ERR;
    gu32HIFAddr += u16HashSz;

    if(hif_receive(gu32HIFAddr, pu8Sig, u16SigSz, 0) != M2M_SUCCESS) goto __ERR;
    gu32HIFAddr += u16SigSz;

    return M2M_SUCCESS;

__ERR:
    hif_receive(0, NULL, 0, 1);
    return s8Ret;
}

/*!
@fn         NMI_API sint8 m2m_ssl_retrieve_cert(uint16* pu16Curve, uint8* pu8Value, uint8* pu8Sig, tstrECPoint* pstrKey);
@brief      Retrieve the next set of information from the WINC for ECDSA verification.
@param[out] pu16Curve
                The named curve, to be cast to type @ref tenuEcNamedCurve.
@param[out] pu8Value
                Value retrieved for verification. This is the digest of the message, truncated/prepended to the appropriate size.
                The size of the value is equal to the field size of the curve, hence is determined by pu16Curve.
@param[out] pu8Sig
                Signature retrieved for verification.
                The size of the signature is equal to twice the field size of the curve, hence is determined by pu16Curve.
@param[out] pstrKey
                Public key retrieved for verification.
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.

@pre        This function should only be called after the application has been notified that
            verification information is ready via @ref ECC_REQ_SIGN_VERIFY.

@warning    If this function returns @ref M2M_ERR_FAIL, then any remaining verification info from
            the WINC is lost.

@warning    This API has been deprecated and is kept for legacy purposes only. It is recommended
            that @ref m2m_ssl_retrieve_next_for_verifying is used instead.
*/
NMI_API sint8 m2m_ssl_retrieve_cert(uint16 *pu16Curve, uint8 *pu8Value, uint8 *pu8Sig, tstrECPoint *pstrKey)
{
    uint16  u16ValueSz = 32, u16SigSz = 64;

    return m2m_ssl_retrieve_next_for_verifying((tenuEcNamedCurve *)pu16Curve, pu8Value, &u16ValueSz, pu8Sig, &u16SigSz, pstrKey);
}

/*!
@fn         NMI_API sint8 m2m_ssl_retrieve_hash(uint8* pu8Value, uint16 u16ValueSz)
@brief      Retrieve the value from the WINC for ECDSA signing.
@param[out] pu8Value
                Value retrieved for signing. This is the digest of the message, truncated/prepended to the appropriate size.
@param[in]  u16ValueSz
                Size of value to be retrieved. (The application should obtain this information,
                along with the curve, from the associated @ref ECC_REQ_SIGN_GEN notification.)
@return     The function returns @ref M2M_SUCCESS for success and a negative value otherwise.

@pre        This function should only be called after the application has been notified that
            signing information is ready via @ref ECC_REQ_SIGN_GEN.

@warning    If this function returns @ref M2M_ERR_FAIL, then the value for signing is lost.
*/
NMI_API sint8 m2m_ssl_retrieve_hash(uint8 *pu8Value, uint16 u16ValueSz)
{
    sint8   s8Ret = M2M_ERR_FAIL;

    if(gu32HIFAddr == 0) return M2M_ERR_FAIL;

    if(NULL == pu8Value)
    {
        s8Ret = M2M_ERR_INVALID_ARG;
        goto __ERR;
    }

    if(hif_receive(gu32HIFAddr, pu8Value, u16ValueSz, 0) != M2M_SUCCESS) goto __ERR;

    return M2M_SUCCESS;

__ERR:
    hif_receive(0, NULL, 0, 1);
    return s8Ret;
}

/*!
@fn         NMI_API void m2m_ssl_stop_retrieving(void);
@brief      Allow SSL driver to tidy up when the application chooses not to retrieve all available
            information.

@return     None.

@warning    The application must call this function if it has been notified (via
            @ref ECC_REQ_SIGN_GEN or @ref ECC_REQ_SIGN_VERIFY) that information is available for
            retrieving from the WINC, but chooses not to retrieve it all.
            The application must not call this function if it has retrieved all the available
            information, or if a retrieve function returned @ref M2M_ERR_FAIL indicating that any
            remaining information has been lost.

@see        m2m_ssl_retrieve_next_for_verifying\n
            m2m_ssl_retrieve_cert\n
            m2m_ssl_retrieve_hash
*/
NMI_API void m2m_ssl_stop_retrieving(void)
{
    hif_receive(0, NULL, 0, 1);
}

/*!
@fn         NMI_API void m2m_ssl_stop_processing_certs(void);
@brief      Allow SSL driver to tidy up in case application does not read all available certificates.
@return     None.

@warning    This API has been deprecated and is kept for legacy purposes only. It is recommended
            that @ref m2m_ssl_stop_retrieving is used instead.
*/
NMI_API void m2m_ssl_stop_processing_certs(void)
{
    m2m_ssl_stop_retrieving();
}

/*!
@fn         NMI_API void m2m_ssl_ecc_process_done(void);
@brief      Allow SSL driver to tidy up after application has finished processing ECC message.

@return     None.

@warning    The application should call this function after receiving an SSL callback with message
            type @ref M2M_SSL_REQ_ECC, after retrieving any related information, and before
            calling @ref m2m_ssl_handshake_rsp.
*/
NMI_API void m2m_ssl_ecc_process_done(void)
{
    gu32HIFAddr = 0;
}

/*!
@fn         NMI_API sint8 m2m_ssl_set_active_ciphersuites(uint32 u32SslCsBMP)
@brief      Sets the active ciphersuites.
@details    Override the default Active SSL ciphers in the SSL module with a certain combination selected by the caller in the form of
            a bitmap containing the required ciphers to be on.
            There is no need to call this function if the application will not change the default ciphersuites.

@param [in] u32SslCsBMP
                Bitmap containing the desired ciphers to be enabled for the SSL module. The ciphersuites are defined in
                @ref SSLCipherSuiteID.
                The default ciphersuites are all ciphersuites supported by the firmware with the exception of ECC ciphersuites.
                The caller can override the default with any desired combination.
                If u32SslCsBMP does not contain any ciphersuites supported by firmware, then the current active list will not
                change.

@return
            - @ref SOCK_ERR_NO_ERROR
            - @ref SOCK_ERR_INVALID_ARG
*/
NMI_API sint8 m2m_ssl_set_active_ciphersuites(uint32 u32SslCsBMP)
{
    sint8 s8Ret = M2M_SUCCESS;
    tstrSslSetActiveCsList  strCsList;

    strCsList.u32CsBMP = u32SslCsBMP;
    s8Ret = hif_send(M2M_REQ_GROUP_SSL, M2M_SSL_REQ_SET_CS_LIST, (uint8 *)&strCsList, sizeof(tstrSslSetActiveCsList), NULL, 0, 0);

    return s8Ret;
}

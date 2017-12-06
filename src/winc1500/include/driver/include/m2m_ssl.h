/**
 *
 * \file
 *
 * \brief WINC Application Interface Internal Types.
 *
 * Copyright (c) 2017 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/**@defgroup SSLAPI SSL
*/

#ifndef __M2M_SSL_H__
#define __M2M_SSL_H__

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#include "common/include/nm_common.h"
#include "driver/include/m2m_types.h"
#include "driver/include/nmdrv.h"
#include "driver/include/ecc_types.h"
#include "socket/include/socket.h"

/**@defgroup  SSLEnums Enumeration/Typedefs
 * @ingroup SSLAPI
 * @{*/

/*!
@typedef \
	void (*tpfAppSslCb) (uint8 u8MsgType, void * pvMsg);

@brief A callback to get SSL notifications.

@param[in] u8MsgType
@param[in] pvMsg A structure to provide notification payload.
*/
typedef void (*tpfAppSSLCb) (uint8 u8MsgType, void * pvMsg);

/**@}
*/

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
FUNCTION PROTOTYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/** \defgroup SSLFUNCTIONS Functions
*  @ingroup SSLAPI
*/

/**@{*/
/*!
	@fn	\	m2m_ssl_init(tpfAppSslCb pfAppSslCb);
	@brief	Initializes the SSL layer.
	@param [in]	pfAppSslCb
				Application SSL callback function.
	@return		The function SHALL return 0 for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ssl_init(tpfAppSSLCb pfAppSSLCb);

/*!
	@fn	\	 NMI_API sint8 m2m_ssl_handshake_rsp(tstrEccReqInfo* strECCResp, uint8* pu8RspDataBuff, uint16 u16RspDataSz)
	@brief	 Sends ECC responses to the WINC
	@param [in]	strECCResp
				ECC Response struct.
	@param [in]	pu8RspDataBuffe
				Pointer of the response data to be sent.
	@param [in]	u16RspDataSz
				Response data size.
	@return		The function SHALL return 0 for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ssl_handshake_rsp(tstrEccReqInfo* strECCResp, uint8* pu8RspDataBuff, uint16 u16RspDataSz);

/*!
	@fn	\	NMI_API sint8 m2m_ssl_send_certs_to_winc(uint8* pu8Buffer, uint32 u32BufferSz)
	@brief	Sends certificates to the WINC
	@param [in]	pu8Buffer
				Pointer to the certificates.
	@param [in]	u32BufferSz
				Size of the certificates.
	@return		The function SHALL return 0 for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ssl_send_certs_to_winc(uint8* pu8Buffer, uint32 u32BufferSz);

/*!
	@fn	\	NMI_API sint8 m2m_ssl_retrieve_cert(uint16* pu16CurveType, uint8* pu8Hash, uint8* pu8Sig, tstrECPoint* pu8Key)
	@brief	Retrieve the certificate to be verified from the WINC
	@param [in]	pu16CurveType
				Pointer to the certificate curve type.
	@param [in]	pu8Hash
				Pointer to the certificate hash.
	@param [in]	pu8Sig
				Pointer to the certificate signature.
	@param [in]	pu8Key
				Pointer to the certificate Key.
	@return		The function SHALL return 0 for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ssl_retrieve_cert(uint16* pu16CurveType, uint8* pu8Hash, uint8* pu8Sig, tstrECPoint* pu8Key);

/*!
	@fn	\	NMI_API sint8 m2m_ssl_retrieve_hash(uint8* pu8Hash, uint16 u16HashSz)
	@brief	Retrieve the certificate hash
	@param [in]	pu8Hash
				Pointer to the certificate hash.
	@param [in]	u16HashSz
				Hash size.
	@return		The function SHALL return 0 for success and a negative value otherwise.
*/
NMI_API sint8 m2m_ssl_retrieve_hash(uint8* pu8Hash, uint16 u16HashSz);

/*!
	@fn	\	NMI_API void m2m_ssl_stop_processing_certs(void)
	@brief	Allow ssl driver to tidy up in case application does not read all available certificates.
	@warning	This API must only be called if some certificates are left unread.
	@return		None.
*/
NMI_API void m2m_ssl_stop_processing_certs(void);

/*!
	@fn	\	NMI_API void m2m_ssl_ecc_process_done(void)
	@brief	Allow ssl driver to tidy up after application has finished processing ecc message.
	@warning	This API must be called after receiving a SSL callback with type @ref M2M_SSL_REQ_ECC
	@return		None.
*/
NMI_API void m2m_ssl_ecc_process_done(void);

/*!
@fn	\
	NMI_API sint8 m2m_ssl_set_active_ciphersuites(uint32 u32SslCsBMP);
	Override the default Active SSL ciphers in the SSL module with a certain combination selected by the caller in the form of
	a bitmap containing the required ciphers to be on.
	There is no need to call this function if the application will not change the default ciphersuites.

@param [in]	u32SslCsBMP
				Bitmap containing the desired ciphers to be enabled for the SSL module. The ciphersuites are defined in
				@ref SSLCipherSuiteID.
				The default ciphersuites are all ciphersuites supported by the firmware with the exception of ECC ciphersuites.
				The caller can override the default with any desired combination, except for combinations involving both RSA
				and ECC; if any RSA ciphersuite is enabled, then firmware will disable all ECC ciphersuites.
				If u32SslCsBMP does not contain any ciphersuites supported by firmware, then the current active list will not
				be changed.

@return		
	- [SOCK_ERR_NO_ERROR](@ref SOCK_ERR_NO_ERROR)
	- [SOCK_ERR_INVALID_ARG](@ref SOCK_ERR_INVALID_ARG)
*/
sint8 m2m_ssl_set_active_ciphersuites(uint32 u32SslCsBMP);

 /**@}*/
#endif /* __M2M_SSL_H__ */

/**
 *
 * \file
 *
 * \brief X509 Certificate Functions.
 *
 * Copyright (c) 2015 Atmel Corporation. All rights reserved.
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

#ifndef X509_CERT_H_INCLUDED
#define X509_CERT_H_INCLUDED

#include "common.h"
#include "hash.h"

#define X509_SUCCESS                                0
#define X509_FAIL                                   -1

#define X509_SERIAL_NO_MAX_LENGTH                   20

/* X509 ALGORITHMS. */
#define X509_ALG_NULL                               0
#define X509_PUB_RSA                                1
#define X509_SIGN_MD5RSA                            2
#define X509_SIGN_SHA1RSA                           3
#define X509_SIGN_SHA256RSA                         4
#define X509_SIGN_SHA384RSA                         5
#define X509_SIGN_SHA512RSA                         6

#define X509_STATUS_VALID                           0
/*!<
 *      The X.509 certificate is valid.
 */

#define X509_STATUS_EXPIRED                         1
/*!<
 *      The X.509 certificate is expired.
 */

#define X509_STATUS_REVOKED                         2
/*!<
 *      The X.509 certificate is marked as revoked and should not
 *      be trusted.
 */

#define X509_STATUS_DECODE_ERR                      4
/*!<
 *      Error decoding the certificate time.
 */

#define X509_CERT_DUMP                              X509Cert_Dump
#define X509_CERT_IS_VALID                          X509Cert_IsValid

typedef enum {
	PUBKEY_ALG_RSA,
	PUBKEY_ALG_DH,
	PUBKEY_ALG_ECDH
} tenuPubKeyAlg;

typedef enum {
	SIGN_VERIFY,
	SIGN_GEN
} tenuSignMode;

/*!
 * @struct	tstrDHPublicKey
 *
 * @brief	Diffie-Hellman Public Key Definition
 */
typedef struct {
	uint8 *pu8P;
	uint8 *pu8G;
	uint8 *pu8Key;
	uint16 u16PSize;
	uint16 u16GSize;
	uint16 u16KeySize;
} tstrDHPublicKey;

/*!
 * @struct	tstrRSAPublicKey
 *
 * @brief	RSA Public Key Definition
 */
typedef struct {
	uint16 u16NSize;
	uint16 u16ESize;
	uint8 *pu8N;
	uint8 *pu8E;
} tstrRSAPublicKey;

typedef struct {
	tenuPubKeyAlg enuType;
	union {
		tstrRSAPublicKey strRSAKey;
		tstrDHPublicKey strDHKey;
	};
} tstrPublicKey;

typedef struct {
	char acCmnName[65];
	char acOrg[65];
	char acOrgUnit[65];
	uint8 au8NameSHA1[SHA1_DIGEST_SIZE];
} tstrX520Name;

typedef struct {
	uint8 au8Time[20];
} tstrX509Time;

typedef struct {
	uint8 u8Version;
	/*!<
	 *      X509 version.
	 */

	uint8 u8SerialNumberLength;
	/*!<
	 *      X509 certificate serial number Length in bytes.
	 */

	uint8 au8SerialNo[X509_SERIAL_NO_MAX_LENGTH];
	/*!<
	 *      X509 certificate serial number.
	 */
	 
	uint8 u8SignAlg;
	/*!<
	 *      X509 certificate serial number Length in bytes.
	 */

	uint8 u8ValidityStatus;
	tstrX520Name strIssuer;
	tstrX509Time strStartDate;
	tstrX509Time strExpiryDate;
	tstrX520Name strSubject;
	tstrPublicKey strPubKey;
	tstrBuffer strSignature;
	uint8 *pu8TBSCert;
	uint16 u16TBSCertSize;
} tstrX509Cert;

sint8 X509Cert_Decode(uint8 *pu8X509Buffer, uint32 u32CertSize, tstrX509Cert *pstrCert, uint8 bDumpX509);
void X509Cert_Dump(void *pvCert);

#endif /* X509_CERT_H_INCLUDED */

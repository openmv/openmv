/**
 *
 * \file
 *
 * \brief Root Certificate Setup.
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

#include "fb_alloc.h"
#include "root_cert/x509/x509_cert.h"
#include "root_cert/root_setup.h"
#include "programmer/programmer_apis.h"

//#define ENABLE_VERIFICATION

#define WORD_ALIGN(val)                                 (((val) & 0x03) ? ((val) + 4 - ((val) & 0x03)) : (val))

#define ROOT_CERT_FLASH_START_PATTERN_LENGTH            16

#define ROOT_CERT_FLASH_START_PATTERN \
	{ \
		0x01, 0xF1, 0x02, 0xF2, 0x03, 0xF3, 0x04, 0xF4,	\
		0x05, 0xF5, 0x06, 0xF6, 0x07, 0xF7, 0x08, 0xF8 \
	}

/*!< A Pattern is stored at the start of the root certificate flash area
 *      in order to identify if the flash is written before or not.
 */

typedef struct {
	uint8 au8StartPattern[ROOT_CERT_FLASH_START_PATTERN_LENGTH];
	uint32 u32nCerts;
} tstrRootCertFlashHeader;

typedef struct {
	uint8 au8SHA1NameHash[SHA1_DIGEST_SIZE];
	uint16 u16NSize;
	uint16 u16ESize;
	uint8 au8ValidityStartDate[20];
	uint8 au8ValidityExpDate[20];
} tstrRootCertEntryHeader;

#ifdef ENABLE_VERIFICATION
static uint8 gau8Verify[M2M_TLS_FLASH_ROOTCERT_CACHE_SIZE];
#endif

/**
 * \brief Parse and validate certificate.
 *
 * \return M2M_SUCCESS on success, error code otherwise.
 */
static int GetRootCertificate(uint8 *pcRootCertFileData, uint32 u32FileSize, tstrX509Cert *pstrX509, uint8 **ppu8CertificateBuffer)
{
	sint8 ret = -1;

	/* Decode the certificate. */
	if (X509Cert_Decode(pcRootCertFileData, u32FileSize, pstrX509, 1) == X509_SUCCESS) {
		printf(">Root certificate verified OK.\n");
		*ppu8CertificateBuffer = pcRootCertFileData;
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

/**
 * \brief Update the root certificate list.
 *
 * \return M2M_SUCCESS on success, error code otherwise.
 */
static sint8 UpdateRootList(tstrX509Cert *pstrRootCert, uint8 *gau8RootCertMem)
{
	uint32 u32Idx;
	uint8 bIncrement        = 0;
	uint32 u32nStoredCerts  = 0;
	uint8 au8StartPattern[] = ROOT_CERT_FLASH_START_PATTERN;
	tstrRootCertFlashHeader *pstrRootFlashHdr;
	tstrRootCertEntryHeader *pstrEntryHDR;
	uint16 u16BufferOffset;
	uint16 u16WriteSize;
	tstrRSAPublicKey *pstrKey;

	pstrKey                 = &pstrRootCert->strPubKey.strRSAKey;
	pstrRootFlashHdr        = (tstrRootCertFlashHeader *)((void *)gau8RootCertMem);
	u16BufferOffset         = sizeof(tstrRootCertFlashHeader);
	u16WriteSize            = pstrKey->u16ESize + pstrKey->u16NSize + sizeof(tstrRootCertEntryHeader);

	/* Check if the flash has been written before. */
	if (!memcmp(au8StartPattern, pstrRootFlashHdr->au8StartPattern, ROOT_CERT_FLASH_START_PATTERN_LENGTH)) {
		u32nStoredCerts = pstrRootFlashHdr->u32nCerts;
		bIncrement = 1;

		for (u32Idx = 0; u32Idx < u32nStoredCerts; u32Idx++) {
			pstrEntryHDR = (tstrRootCertEntryHeader *)((void *)&gau8RootCertMem[u16BufferOffset]);

			/* Check for match (equivalent NameSHA1). */
			if (!memcmp(pstrRootCert->strSubject.au8NameSHA1, pstrEntryHDR->au8SHA1NameHash, SHA1_DIGEST_SIZE)) {
				/* The current entry will be overwritten. */
				bIncrement = 0;
				break;
			}

			u16BufferOffset += sizeof(tstrRootCertEntryHeader) + WORD_ALIGN(pstrEntryHDR->u16NSize + pstrEntryHDR->u16ESize);
		}
	} else {
		pstrRootFlashHdr->u32nCerts = 0;
		memcpy(pstrRootFlashHdr->au8StartPattern, au8StartPattern, ROOT_CERT_FLASH_START_PATTERN_LENGTH);
		bIncrement = 1;
	}

	if (bIncrement) {
		/* A new certificate is to be inserted into the flash. */
		/* Increment the number of stored Certificates. */
		if (u16BufferOffset + u16WriteSize > M2M_TLS_FLASH_ROOTCERT_CACHE_SIZE) {
			printf("(ERROR) Root Certificate Flash is Full\n");
			return -1;
		}

		pstrRootFlashHdr->u32nCerts++;
	}

	/* Write Root Certificate to flash. The entry is ordered as follows:
		-	SHA1_DIGEST_SIZE	--> NameSHA1 of the Root certificate.
		-	uint16				--> N_SIZE (Byte count for the RSA modulus N).
		-	uint16				--> E_SIZE (Byte count for the RSA public exponent E).
		-	START_DATE			--> Start date of the root certificate(20 bytes).
		-	EXPIRATION_DATE		--> Expiration date of the root certificate(20 bytes).
		-	N_SIZE				--> RSA Modulus N.
		-	E_SIZE				--> RSA Public exponent.
	*/

	pstrEntryHDR = (tstrRootCertEntryHeader *)((void *)&gau8RootCertMem[u16BufferOffset]);

	/* NameSHA1	*/
	memcpy(pstrEntryHDR->au8SHA1NameHash, pstrRootCert->strSubject.au8NameSHA1, SHA1_DIGEST_SIZE);

	/* N_SIZE */
	pstrEntryHDR->u16NSize = pstrKey->u16NSize;

	/* E_SIZE */
	pstrEntryHDR->u16ESize  = pstrKey->u16ESize;

	/* Start Date */
	memcpy(pstrEntryHDR->au8ValidityStartDate, pstrRootCert->strStartDate.au8Time, 20);

	/* Expiration Date */
	memcpy(pstrEntryHDR->au8ValidityExpDate, pstrRootCert->strExpiryDate.au8Time, 20);
	u16BufferOffset += sizeof(tstrRootCertEntryHeader);

	/* N */
	memcpy(&gau8RootCertMem[u16BufferOffset], pstrKey->pu8N, pstrKey->u16NSize);

	/* E */
	memcpy(&gau8RootCertMem[u16BufferOffset + pstrKey->u16NSize], pstrKey->pu8E, pstrKey->u16ESize);

	return 0;
}

/**
 * \brief Write root certificate to flash.
 *
 * \return M2M_SUCCESS on success, error code otherwise.
 */
int WriteRootCertificate(const char *filename, char *pcRootCertFileData, uint32 u32FileSize)
{
	uint8 *pu8CertBuffer;
	tstrX509Cert strX509Root;
#ifdef ENABLE_VERIFICATION
	uint32 u32Idx;
#endif
	int ret = -1;

    uint8 *gau8RootCertMem  = fb_alloc(M2M_TLS_FLASH_ROOTCERT_CACHE_SIZE);

	/* Read Certificate File. */
	if (GetRootCertificate((uint8 *)pcRootCertFileData, u32FileSize, &strX509Root, &pu8CertBuffer) == 0) {
		programmer_read_cert_image(gau8RootCertMem);

		if (UpdateRootList(&strX509Root, gau8RootCertMem) == 0) {
			/* Erase memory. */
			ret = programmer_erase_cert_image();
			if (M2M_SUCCESS != ret) {
				goto END;
			}

			nm_bsp_sleep(50);

			/* Write. */
			printf(">Writing the certificate to SPI flash...\r\n");
			ret = programmer_write_cert_image(gau8RootCertMem);
			if (M2M_SUCCESS != ret) {
				goto END;
			}

			printf("Done\r\n");

#ifdef ENABLE_VERIFICATION
			/* Verification. */
			memset(gau8Verify, 0, M2M_TLS_FLASH_ROOTCERT_CACHE_SIZE);
			programmer_read_cert_image(gau8Verify);

			for (u32Idx = 0; u32Idx < M2M_TLS_FLASH_ROOTCERT_CACHE_SIZE; u32Idx++) {
				if (gau8RootCertMem[u32Idx] != gau8Verify[u32Idx]) {
					printf("ERROR verification failed at %u\n", u32Idx);
					ret = -1;
					break;
				}
			}
#endif
		}
	} else {
		printf("\r\n>>>Invalid root certificate %s\n", filename);
	}

END:
    fb_free();
	return ret;
}

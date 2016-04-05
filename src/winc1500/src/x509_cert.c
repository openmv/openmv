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

#include "root_cert/x509/x509_cert.h"
#include "root_cert/x509/asn1.h"

/* X509 SUPPORTED VERSIONS. */
#define X509_VER_1                                  0
#define X509_VER_2                                  1
#define X509_VER_3                                  2

/* X509 CERTIFICATE PARSING STATES. */

#define CERT_VERSION_PENDING                        0
#define CERT_SERIAL_PENDING                         1
#define CERT_SIGNATURE_PENDING                      2
#define CERT_ISSUER_PENDING                         3
#define CERT_VALIDITY_PENDING                       4
#define CERT_SUBJECT_PENDING                        5
#define CERT_SUBECTKEYINFO_PENDING                  6
#define CERT_EXTENSIONS_PENDING                     7
#define CERT_PARSING_DONE                           8

/* X509 ENCODED FIELD IDs. */

#define X509_VERSION                                0xA0

/*!<
 *      Identifier of the version field  in the X.509 certificate
 *      encoding. It is given in the ASN.1 syntax as
 *      [0] EXPLICIT
 *      Context-Specific class (10) | constructed (1) | 00000
 */

#define X509_SERIAL_NO                              ASN1_INTEGER

/*!<
 *      Identifier for the certificate serial num ber element.
 */

#define X509_SIGNATURE                              ASN1_SEQUENCE

/*!<
 *      Identifier for the Signature algorithm ID element.
 */

#define X509_ISSUER                                 ASN1_SEQUENCE

/*!<
 *      Identifier for the certificate ISSUER element.
 */

#define X509_VALIDITY                               ASN1_SEQUENCE

/*!<
 *      Identifier for the certificate validity interval element.
 */

#define X509_SUBJECT                                ASN1_SEQUENCE

/*!<
 *      Identifier for the certificate SUBJECT information element.
 */

#define X509_SUBJECT_KEY_INFO                       ASN1_SEQUENCE

/*!<
 *      Identifier for the publick key information element.
 */

#define X509_ISSUER_UNIQUE_ID                       0x81

/*!<
 *      Encoded value for the issuerUniqueID tag. It is defined as
 *      [1] IMPLICIT BIT STRING
 *      Context-Specific class (10) | primitive (0) | 00001
 */

#define X509_SUBJECT_UNIQUE_ID                      0x82

/*!<
 *      Encoded value for the subjectUniqueID tag. It is defined as
 *      [2] IMPLICIT BIT STRING
 *      Context-Specific class (10) | primitive (0) | 00010
 */

#define X509_EXTENSIONS_ID                          0xA3

/*!<
 *      Encoded value for the issuerUniqueID tag. It is defined as
 *      [3] EXPLICIT
 *      Context-Specific class (10) | constructed (1) | 00011
 */

/* NAME ATTRIBUTE IDs. */

#define ID_AT                                       85, 0x04,
#define ID_AT_COMMONNAME                            {ID_AT 3 }
#define ID_AT_SERIALNUMBER                          {ID_AT 5 }
#define ID_AT_COUNTRYNAME                           {ID_AT 6 }
#define ID_AT_ORGANIZATIONNAME                      {ID_AT 10}
#define ID_AT_ORGANIZATIONALUNITNAME                {ID_AT 11}

#define X520_COMMON_NAME                            1
#define X520_ORG_UNIT                               2
#define X520_ORG                                    3

/* CERTIFICATE EXTENSION IDs. */

#define ID_CE                                       85, 29,
#define ID_CE_AUTHORITY_KEY_ID                      {ID_CE 35}
#define ID_CE_SUBJECT_KEY_ID                        {ID_CE 14}

/* SIGNATURE ALGORITHM IDs. */

#define PKCS_1                                      0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0d, 0x01, 0x01,
/* 1.2.840.113549.1.1 */

#define RSA_ENCRYPTION                              {PKCS_1  1}  /* 1.2.840.113549.1.1    */
#define MD5_WITH_RSA_ENCRYPTION                     {PKCS_1  4}  /* 1.2.840.113549.1.1.4  */
#define SHA1_WITH_RSA_ENCRYPTION                    {PKCS_1  5}  /* 1.2.840.113549.1.1.5  */
#define SHA256_WITH_RSA_ENCRYPTION                  {PKCS_1 11}  /* 1.2.840.113549.1.1.11 */
#define SHA384_WITH_RSA_ENCRYPTION                  {PKCS_1 12}  /* 1.2.840.113549.1.1.12 */
#define SHA512_WITH_RSA_ENCRYPTION                  {PKCS_1 13}  /* 1.2.840.113549.1.1.13 */

typedef struct {
	uint8 u8Type;
	uint8 *pu8Name;
	uint8 u8Length;
} tstrX520DistinguishedName;

sint8 M2M_GetTimeOfDay(tstrSystemTime *pstrSysTime);
uint8 Cert_DecodeAlgID(tstrAsn1Context *pstrX509Asn1Cxt, uint32 u32Size);
sint8 Cert_DecodeDistinguishedName(tstrAsn1Context *pstrX509Asn1Cxt, uint32 u32Size, tstrX520DistinguishedName *pstrDN);
sint8 Cert_DecodeX520Name(tstrAsn1Context *pstrX509Asn1Cxt, uint32 u32Size, tstrX520Name *pstrName);
sint8 Cert_DecodeRSAPubKey(tstrAsn1Context *pstrX509Asn1Cxt, uint32 u32KeySize, tstrRSAPublicKey *pstrRsaPublicKey);
sint8 Cert_DecodeSubjectPubKey(tstrAsn1Context *pstrX509Asn1Cxt, uint32 u32Size, tstrPublicKey *pstrPubKey);
sint8 Cert_DecodeValidity(tstrAsn1Context *pstrX509Asn1Cxt, uint32 u32Size, tstrX509Time *pstrNotBefore, tstrX509Time *pstrNotAfter);
sint8 Cert_DecodeTBSCertificate(tstrAsn1Context *pstrX509Asn1Cxt, uint16 u16CertSize, tstrX509Cert *pstrCert);
sint8 Cert_DecodeTime(uint8 *pu8Time, tstrSystemTime *pstrTime);
sint8 Cert_CompareTime(tstrSystemTime *pstrT1, tstrSystemTime *pstrT2);
uint8 X509Cert_IsValid(void *pvX509);

sint8 M2M_GetTimeOfDay(tstrSystemTime *pstrSysTime)
{
#ifdef WIN32
	SYSTEMTIME winTime;
	GetSystemTime(&winTime);
	pstrSysTime->u16Year    = (uint16)winTime.wYear;
	pstrSysTime->u8Month    = (uint8)winTime.wMonth;
	pstrSysTime->u8Day      = (uint8)winTime.wDay;
	pstrSysTime->u8Hour     = (uint8)winTime.wHour;
	pstrSysTime->u8Minute   = (uint8)winTime.wMinute;
	pstrSysTime->u8Second   = (uint8)winTime.wSecond;
#else
	pstrSysTime->u16Year    = (uint16)(2015);
	pstrSysTime->u8Month    = (uint8)8;
	pstrSysTime->u8Day      = (uint8)6;
	pstrSysTime->u8Hour     = (uint8)9;
	pstrSysTime->u8Minute   = (uint8)39;
	pstrSysTime->u8Second   = (uint8)10;
#endif

	return M2M_SUCCESS;
}

/**
 * \brief Perform decoding to the data type "AlgorithmIdentifier".
 */
uint8 Cert_DecodeAlgID(tstrAsn1Context *pstrX509Asn1Cxt, uint32 u32Size)
{
	/*
	 *      AlgorithmIdentifier ::= SEQUENCE {
	 *              algorithm       OBJECT IDENTIFIER,
	 *              parameters      NULL
	 *      }
	 */

    const uint8 gau8SecAlgorithms[][16] = {
	    RSA_ENCRYPTION,
	    MD5_WITH_RSA_ENCRYPTION,
	    SHA1_WITH_RSA_ENCRYPTION,
	    SHA256_WITH_RSA_ENCRYPTION,
	    SHA384_WITH_RSA_ENCRYPTION,
	    SHA512_WITH_RSA_ENCRYPTION
    };
    #define X509_NUM_SUPPORTED_SEC_ALGORITHMS (sizeof(gau8SecAlgorithms) / sizeof(gau8SecAlgorithms[0]))

	uint8 u8Alg = X509_ALG_NULL;

	if (pstrX509Asn1Cxt != NULL) {
		tstrAsn1Element strElem;

		/* Initialize a decoding object for this SEQUENCE. */
		ASN1_GetNextElement(pstrX509Asn1Cxt, &strElem);
		if (strElem.u8Tag == ASN1_OBJECT_IDENTIFIER) {
			if (strElem.u32Length <= 16) {
				uint8 u8Idx;
				uint8 au8AlgID[16];
				ASN1_Read(pstrX509Asn1Cxt, strElem.u32Length, au8AlgID);
				for (u8Idx = 0; u8Idx < X509_NUM_SUPPORTED_SEC_ALGORITHMS; u8Idx++) {
					if (!m2m_memcmp(au8AlgID, (uint8 *)gau8SecAlgorithms[u8Idx], strElem.u32Length)) {
						u8Alg = u8Idx + 1;
						break;
					}
				}
			}
		}
	}

	return u8Alg;
}

sint8 Cert_DecodeDistinguishedName(tstrAsn1Context *pstrX509Asn1Cxt, uint32 u32Size, tstrX520DistinguishedName *pstrDN)
{
	sint8 s8Ret = X509_FAIL;

	if ((pstrX509Asn1Cxt != NULL) && (pstrDN != NULL)) {
		tstrAsn1Element strElem;

		ASN1_GetNextElement(pstrX509Asn1Cxt, &strElem);
		if (strElem.u8Tag == ASN1_OBJECT_IDENTIFIER) {
			uint8 au8CmnNameID[4] = ID_AT_COMMONNAME;
			uint8 au8TempID[16];

			pstrDN->u8Type = 0;
			ASN1_Read(pstrX509Asn1Cxt, strElem.u32Length, au8TempID);
			if (!m2m_memcmp(au8CmnNameID, au8TempID, strElem.u32Length)) {
				pstrDN->u8Type = X520_COMMON_NAME;
			}

			if (ASN1_GetNextElement(pstrX509Asn1Cxt, &strElem) != ASN1_FAIL) {
				pstrDN->u8Length = (uint8)strElem.u32Length;
				ASN1_ReadReference(pstrX509Asn1Cxt, strElem.u32Length, &pstrDN->pu8Name);
				s8Ret = X509_SUCCESS;
			}
		}
	}

	return s8Ret;
}

sint8 Cert_DecodeX520Name(tstrAsn1Context *pstrX509Asn1Cxt, uint32 u32Size, tstrX520Name *pstrName)
{
	/*
	 *      Name ::= CHOICE { -- only one possibility for now --
	 *              rdnSequence RDNSequence
	 *      }
	 *
	 *      RDNSequence ::= SEQUENCE OF RelativeDistinguishedName
	 *
	 *      RelativeDistinguishedName ::=
	 *              SET SIZE (1..MAX) OF AttributeTypeAndValue
	 *
	 *      AttributeTypeAndValue ::= SEQUENCE {
	 *              type    AttributeType,
	 *              value   AttributeValue
	 *      }
	 *
	 *      AttributeType ::= OBJECT IDENTIFIER
	 *      AttributeValue ::= ANY -- DEFINED BY AttributeType
	 *
	 *      DirectoryString ::= CHOICE {
	 *              teletexString           TeletexString (SIZE (1..MAX)),
	 *              printableString         PrintableString (SIZE (1..MAX)),
	 *              universalString         UniversalString (SIZE (1..MAX)),
	 *              utf8String              UTF8String (SIZE (1..MAX)),
	 *              bmpString               BMPString (SIZE (1..MAX))
	 *      }
	 */
	sint8 s8Ret = X509_SUCCESS;

	if ((pstrX509Asn1Cxt != NULL) && (pstrName != NULL)) {
		tstrAsn1Element strSetElem, strSeqElem;
		tstrX520DistinguishedName strDN;
		tstrSha1Context strSha1Cxt;
		uint16 u16Offset = 0;

		SHA1_INIT(&strSha1Cxt);

		/* RelativeDistinguishedName. */
		while (u16Offset < u32Size) {
			u16Offset += ASN1_GetNextElement(pstrX509Asn1Cxt, &strSetElem);
			if (strSetElem.u8Tag == ASN1_SET) {
				ASN1_GetNextElement(pstrX509Asn1Cxt, &strSeqElem);
				if (strSeqElem.u8Tag != ASN1_SEQUENCE) {
					s8Ret = X509_FAIL;
					break;
				}

				if (Cert_DecodeDistinguishedName(pstrX509Asn1Cxt, strSeqElem.u32Length, &strDN) != X509_SUCCESS) {
					s8Ret = X509_FAIL;
					break;
				}

				if (strDN.u8Type == X520_COMMON_NAME) {
					m2m_memcpy((uint8 *)pstrName->acCmnName, strDN.pu8Name, strDN.u8Length);
					pstrName->acCmnName[strDN.u8Length] = '\0';
				} else if (strDN.u8Type == X520_ORG_UNIT) {
					if (pstrName->acOrgUnit[0] == 0) {
						m2m_memcpy((uint8 *)pstrName->acOrgUnit, strDN.pu8Name, strDN.u8Length);
						pstrName->acOrgUnit[strDN.u8Length] = '\0';
					}
				} else if (strDN.u8Type == X520_ORG) {
					m2m_memcpy((uint8 *)pstrName->acOrg, strDN.pu8Name, strDN.u8Length);
					pstrName->acOrg[strDN.u8Length] = '\0';
				}

				SHA1_UPDATE(&strSha1Cxt, strDN.pu8Name, strDN.u8Length);
			} else {
				break;
			}
		}
		SHA1_FINISH(&strSha1Cxt, pstrName->au8NameSHA1);
	}

	return s8Ret;
}

sint8 Cert_DecodeRSAPubKey(tstrAsn1Context *pstrX509Asn1Cxt, uint32 u32KeySize, tstrRSAPublicKey *pstrRsaPublicKey)
{
	/*
	 *      RSAPublicKey ::= SEQUENCE {
	 *              modulus         INTEGER, -- n
	 *              publicExponent  INTEGER  -- e
	 *      }
	 */
	sint8 s8Ret = X509_FAIL;
	if ((pstrX509Asn1Cxt != NULL) && (pstrRsaPublicKey != NULL)) {
		tstrAsn1Element strElem;

		ASN1_GetNextElement(pstrX509Asn1Cxt, &strElem);
		if (strElem.u8Tag == ASN1_SEQUENCE) {
			ASN1_GetNextElement(pstrX509Asn1Cxt, &strElem);
			if (strElem.u8Tag == ASN1_INTEGER) {
				ASN1_ReadReference(pstrX509Asn1Cxt, strElem.u32Length, &pstrRsaPublicKey->pu8N);
				while (*pstrRsaPublicKey->pu8N == 0) {
					pstrRsaPublicKey->pu8N++;
					strElem.u32Length--;
				}
				pstrRsaPublicKey->u16NSize = (uint16)strElem.u32Length;

				ASN1_GetNextElement(pstrX509Asn1Cxt, &strElem);
				if (strElem.u8Tag == ASN1_INTEGER) {
					ASN1_ReadReference(pstrX509Asn1Cxt, strElem.u32Length, &pstrRsaPublicKey->pu8E);
					while (*pstrRsaPublicKey->pu8E == 0) {
						pstrRsaPublicKey->pu8E++;
						strElem.u32Length--;
					}
					pstrRsaPublicKey->u16ESize = (uint16)strElem.u32Length;
					s8Ret = X509_SUCCESS;
				}
			}
		}
	}

	return s8Ret;
}

sint8 Cert_DecodeSubjectPubKey(tstrAsn1Context *pstrX509Asn1Cxt, uint32 u32Size, tstrPublicKey *pstrPubKey)
{
	/*
	 *      SubjectPublicKeyInfo ::= SEQUENCE {
	 *              algorithm           AlgorithmIdentifier,
	 *              subjectPublicKey	BIT STRING
	 *      }
	 */
	sint8 s8Ret = X509_SUCCESS;

	if ((pstrX509Asn1Cxt != NULL) && (pstrPubKey != NULL)) {
		tstrAsn1Element strElem;
		uint8 u8PubKeyAlg = 0;
		uint16 u16Offset = 0;

		while (u16Offset < u32Size) {
			u16Offset += ASN1_GetNextElement(pstrX509Asn1Cxt, &strElem);
			if (strElem.u8Tag == ASN1_SEQUENCE) {
				u8PubKeyAlg = Cert_DecodeAlgID(pstrX509Asn1Cxt, strElem.u32Length);
			} else if (strElem.u8Tag == ASN1_BIT_STRING) {
				if (u8PubKeyAlg != X509_PUB_RSA) {
					s8Ret = X509_FAIL;
					break;
				}

				pstrPubKey->enuType = PUBKEY_ALG_RSA;
				ASN1_Read(pstrX509Asn1Cxt, 1, NULL);
				s8Ret = Cert_DecodeRSAPubKey(pstrX509Asn1Cxt, strElem.u32Length - 1, &pstrPubKey->strRSAKey);
				break;
			}
		}
	}

	return s8Ret;
}

sint8 Cert_DecodeValidity(tstrAsn1Context *pstrX509Asn1Cxt, uint32 u32Size, tstrX509Time *pstrNotBefore, tstrX509Time *pstrNotAfter)
{
	/*
	 *      Validity ::= SEQUENCE {
	 *              notBefore       Time,
	 *              notAfter        Time
	 *      }
	 */
	sint8 s8Ret = X509_FAIL;

	if ((pstrX509Asn1Cxt != 0) && (pstrNotAfter != NULL) && (pstrNotBefore != NULL)) {
		tstrAsn1Element strElem;

		if (ASN1_GetNextElement(pstrX509Asn1Cxt, &strElem) != ASN1_INVALID) {
			/* Start time. */
			ASN1_Read(pstrX509Asn1Cxt, strElem.u32Length, pstrNotBefore->au8Time);
			pstrNotBefore->au8Time[strElem.u32Length] = '\0';

			if (ASN1_GetNextElement(pstrX509Asn1Cxt, &strElem)  != ASN1_INVALID) {
				/* End time. */
				ASN1_Read(pstrX509Asn1Cxt, strElem.u32Length, pstrNotAfter->au8Time);
				pstrNotAfter->au8Time[strElem.u32Length] = '\0';
				s8Ret = X509_SUCCESS;
			}
		}
	}

	return s8Ret;
}

sint8 Cert_DecodeTBSCertificate(tstrAsn1Context *pstrX509Asn1Cxt, uint16 u16CertSize, tstrX509Cert *pstrCert)
{
	/*
	 *      TBSCertificate ::= SEQUENCE {
	 *                      version              [0]
	 *      Version DEFAULT v1,
	 *                      serialNumber				CertificateSerialNumber,
	 *                      signature					AlgorithmIdentifier,
	 *                      issuer					    Name,
	 *                      validity					Validity,
	 *                      subject                     Name,
	 *                      subjectPublicKeyInfo		SubjectPublicKeyInfo,
	 *                      issuerUniqueID		 [1]	IMPLICIT UniqueIdentifier OPTIONAL, If present, version MUST be v2 or v3
	 *                      subjectUniqueID      [2]    IMPLICIT UniqueIdentifier OPTIONAL, If present, version MUST be v2 or v3
	 *                      extensions			 [3]    Extensions OPTIONAL If present, version MUST be v3
	 *              }
	 *
	 */
	sint8 s8Ret = X509_SUCCESS;

	if ((pstrX509Asn1Cxt != NULL) && (pstrCert != NULL)) {
		uint16 u16ReadOffset = 0;
		tstrAsn1Element strElement;
		uint8 u8CertState     = CERT_VERSION_PENDING;

		/* Loop on the elements of the certificate until finishing. */
		while (u16ReadOffset < u16CertSize) {
			u16ReadOffset += ASN1_GetNextElement(pstrX509Asn1Cxt, &strElement);
			switch (u8CertState) {
			case CERT_VERSION_PENDING:

				/* Version. */
				if (strElement.u8Tag == X509_VERSION) {
					/* The encoding of version is on the form A0 03 02 01 ver. */
					if (strElement.u32Length < 3) {
						s8Ret = X509_FAIL;
					}

					ASN1_Read(pstrX509Asn1Cxt, 2, NULL);
					ASN1_Read(pstrX509Asn1Cxt, 1, &pstrCert->u8Version);
					u8CertState = CERT_SERIAL_PENDING;
				} else if (strElement.u8Tag == X509_SERIAL_NO) {
					/* The certificate version number is absent from the certificate. */
					/* It will be set to the default value Version 1.0. */
					pstrCert->u8Version = X509_VER_1;

					/* The first field is the serial number. */
					if (strElement.u32Length > X509_SERIAL_NO_MAX_LENGTH) {
						strElement.u32Length = X509_SERIAL_NO_MAX_LENGTH;
					}

					pstrCert->u8SerialNumberLength = (uint8)strElement.u32Length;
					ASN1_Read(pstrX509Asn1Cxt, strElement.u32Length, pstrCert->au8SerialNo);
					u8CertState = CERT_SIGNATURE_PENDING;
				}

				break;

			case CERT_SERIAL_PENDING:

				/* SerialNumber. */
				if (strElement.u8Tag == X509_SERIAL_NO) {
					/* The first field is the serial number. */
					if (strElement.u32Length > X509_SERIAL_NO_MAX_LENGTH) {
						strElement.u32Length = X509_SERIAL_NO_MAX_LENGTH;
					}

					pstrCert->u8SerialNumberLength = (uint8)strElement.u32Length;
					ASN1_Read(pstrX509Asn1Cxt, strElement.u32Length, pstrCert->au8SerialNo);
					u8CertState = CERT_SIGNATURE_PENDING;
				}

				break;

			case CERT_SIGNATURE_PENDING:

				/* Signature. */
				if (strElement.u8Tag == X509_SIGNATURE) {
					pstrCert->u8SignAlg = Cert_DecodeAlgID(pstrX509Asn1Cxt, strElement.u32Length);
					u8CertState = CERT_ISSUER_PENDING;
				}

				break;

			case CERT_ISSUER_PENDING:

				/* Issuer. */
				if (strElement.u8Tag == X509_ISSUER) {
					Cert_DecodeX520Name(pstrX509Asn1Cxt, strElement.u32Length, &pstrCert->strIssuer);
					u8CertState = CERT_VALIDITY_PENDING;
				}

				break;

			case CERT_VALIDITY_PENDING:

				/* Validity. */
				if (strElement.u8Tag == X509_VALIDITY) {
					Cert_DecodeValidity(pstrX509Asn1Cxt, strElement.u32Length,
							&pstrCert->strStartDate, &pstrCert->strExpiryDate);
					u8CertState = CERT_SUBJECT_PENDING;
				}

				break;

			case CERT_SUBJECT_PENDING:

				/* Subject. */
				if (strElement.u8Tag == X509_SUBJECT) {
					Cert_DecodeX520Name(pstrX509Asn1Cxt, strElement.u32Length, &pstrCert->strSubject);
					u8CertState = CERT_SUBECTKEYINFO_PENDING;
				}

				break;

			case CERT_SUBECTKEYINFO_PENDING:

				/* SubjectPublicKeyInfo. */
				if (strElement.u8Tag == X509_SUBJECT_KEY_INFO) {
					Cert_DecodeSubjectPubKey(pstrX509Asn1Cxt, strElement.u32Length, &pstrCert->strPubKey);
					u8CertState = CERT_EXTENSIONS_PENDING;
				}

				break;

			case CERT_EXTENSIONS_PENDING:
				ASN1_Read(pstrX509Asn1Cxt, strElement.u32Length, NULL);
				break;
			}
		}
	}

	return s8Ret;
}

sint8 X509Cert_Decode(uint8 *pu8X509Buffer, uint32 u32CertSize, tstrX509Cert *pstrCert, uint8 bDumpX509)
{
	/*
	 *
	 * Certificate ::= SEQUENCE {
	 *      tbsCertificate          TBSCertificate,
	 *      signatureAlgorithm      AlgorithmIdentifier,
	 *      signature               BIT STRING }
	 *
	 */
	sint8 s8Ret = X509_FAIL;

	if ((pu8X509Buffer != NULL) && (pstrCert != NULL)) {
		tstrAsn1Context strX509ASN1Cxt;
		tstrAsn1Element strElement;
		uint8 u8SignAlgorithm = X509_ALG_NULL;

		/* Initialize the certificate structure. */
		m2m_memset((uint8 *)pstrCert, 0, sizeof(tstrX509Cert));

		/* Initialize the ASN1 Decoding operation. */
		strX509ASN1Cxt.pu8Buff  = pu8X509Buffer;
		strX509ASN1Cxt.pu8Data  = pu8X509Buffer;

		ASN1_GetNextElement(&strX509ASN1Cxt, &strElement);
		if ((strElement.u8Tag == ASN1_SEQUENCE) && (strElement.u32Length < u32CertSize)) {
			/* Copy the TBS certificate. It shall be used later to calculate the certificate HASH. */
			pstrCert->pu8TBSCert = strX509ASN1Cxt.pu8Data;

			/* tbsCertificate. */
			pstrCert->u16TBSCertSize = ASN1_GetNextElement(&strX509ASN1Cxt, &strElement);
			if (strElement.u8Tag == ASN1_SEQUENCE) {
				if (Cert_DecodeTBSCertificate(&strX509ASN1Cxt, (uint16)strElement.u32Length, pstrCert) == 0) {
					/* signatureAlgorithm. */
					ASN1_GetNextElement(&strX509ASN1Cxt, &strElement);
					if (strElement.u8Tag == ASN1_SEQUENCE) {
						/* Check if the signatureAlgorithm ID obtained in this element matches */
						/* the signature field of the TBS Certificate. */
						u8SignAlgorithm = Cert_DecodeAlgID(&strX509ASN1Cxt, strElement.u32Length);
						if (u8SignAlgorithm == pstrCert->u8SignAlg) {
							/* signature. */
							ASN1_GetNextElement(&strX509ASN1Cxt, &strElement);
							if (strElement.u8Tag == ASN1_BIT_STRING) {
								/* Store the obtained signature. */
								pstrCert->strSignature.u16BufferSize = (uint16)strElement.u32Length - 1;

								/* Jump one byte. */
								ASN1_Read(&strX509ASN1Cxt, 1, NULL);

								/* Get the signature value. */
								ASN1_ReadReference(&strX509ASN1Cxt, pstrCert->strSignature.u16BufferSize,
										&pstrCert->strSignature.pu8Data);

								/* Dump the certificate contents. */
								if (bDumpX509) {
									X509_CERT_DUMP(pstrCert);
								}

								s8Ret = X509_SUCCESS;
							}
						}
					}
				}
			}
		}
	}

	return s8Ret;
}

#define X509_UTC_TIME_LENGTH                        0x0D

/*!<
 *      The UTC time for the X.509 encoding takes the format
 *      YYMMDDHHMMSSZ. Each digit is BCD encoded as ASCII digit.
 */

#define X509_GENERALIZED_TIME_LENGTH                0x0F

/*!<
 *      The UTC time for the X.509 encoding takes the format
 *      YYYYMMDDHHMMSSZ. Each digit is BCD encoded as ASCII digit.
 */

#define GET_VAL(BUF, OFFSET)                        (((BUF)[OFFSET] * 10) + ((BUF)[OFFSET + 1]))

sint8 Cert_DecodeTime(uint8 *pu8Time, tstrSystemTime *pstrTime)
{
	sint8 s8Ret = X509_FAIL;

	if (pu8Time != NULL) {
		uint8 u8TimeLength = (uint8)strlen((char *)pu8Time);
		uint8 au8Time[20];
		uint8 u8Idx;

		memcpy(au8Time, pu8Time, 20);

		if ((u8TimeLength >= X509_UTC_TIME_LENGTH) && (u8TimeLength <= X509_GENERALIZED_TIME_LENGTH)) {
			if (au8Time[u8TimeLength - 1] == 'Z') {
				for (u8Idx = 0; u8Idx < (u8TimeLength - 1); u8Idx++) {
					au8Time[u8Idx] -= '0';
				}

				if (u8TimeLength == X509_UTC_TIME_LENGTH) {
					pstrTime->u16Year       = (au8Time[0] * 10) + au8Time[1];
					pstrTime->u16Year       += (pstrTime->u16Year < 50 ? 2000 : 1900);
					u8Idx = 2;
				} else {
					pstrTime->u16Year       = (au8Time[0] * 1000) + (au8Time[1] * 100) + (au8Time[2] * 10) + au8Time[3];
					u8Idx = 4;
				}

				pstrTime->u8Month           = GET_VAL(au8Time, u8Idx);
				u8Idx += 2;

				pstrTime->u8Day             = GET_VAL(au8Time, u8Idx);
				u8Idx += 2;

				pstrTime->u8Hour            = GET_VAL(au8Time, u8Idx);
				u8Idx += 2;

				pstrTime->u8Minute          = GET_VAL(au8Time, u8Idx);
				u8Idx += 2;

				pstrTime->u8Second          = GET_VAL(au8Time, u8Idx);

				s8Ret = X509_SUCCESS;
			}
		}
	}

	return s8Ret;
}

sint8 Cert_CompareTime(tstrSystemTime *pstrT1, tstrSystemTime *pstrT2)
{
	sint8 s8Cmp = 1;

	if ((pstrT1 != NULL) && (pstrT2 != NULL)) {
		/* YEAR. */
		if (pstrT1->u16Year == pstrT2->u16Year) {
			/* MONTH. */
			if (pstrT1->u8Month == pstrT2->u8Month) {
				/* DAY */
				if (pstrT1->u8Day == pstrT2->u8Day) {
					s8Cmp = pstrT1->u8Hour - pstrT2->u8Hour;
				} else {
					s8Cmp = pstrT1->u8Day - pstrT2->u8Day;
				}
			} else {
				s8Cmp = pstrT1->u8Month - pstrT2->u8Month;
			}
		} else {
			s8Cmp = pstrT1->u16Year - pstrT2->u16Year;
		}
	}

	return s8Cmp;
}

uint8 X509Cert_IsValid(void *pvX509)
{
	tstrX509Cert *pstrX509 = (tstrX509Cert *)pvX509;
	uint8 u8Status = X509_STATUS_VALID;

	if (pstrX509 != NULL) {
		tstrSystemTime strExpirationDate = {0};

		if (Cert_DecodeTime(pstrX509->strExpiryDate.au8Time, &strExpirationDate) == X509_SUCCESS) {
			M2M_DBG("Expiration Date\n\t%d-%02d-%02d %02d:%02d:%02d\n",
					strExpirationDate.u16Year, strExpirationDate.u8Month, strExpirationDate.u8Day,
					strExpirationDate.u8Hour, strExpirationDate.u8Minute, strExpirationDate.u8Second);

			if (strExpirationDate.u16Year != 9999) {
#ifdef WIN32
				if (M2M_GetTimeOfDay(&strSystemTime) == M2M_SUCCESS) {
					if (Cert_CompareTime(&strExpirationDate, &strSystemTime) <= 0) {
						u8Status = X509_STATUS_EXPIRED;
					}
				}
#endif
			}
		} else {
			u8Status = X509_STATUS_DECODE_ERR;
		}
	} else {
		u8Status = X509_STATUS_DECODE_ERR;
	}

	return u8Status;
}

void X509Cert_Dump(void *pvCert)
{
	tstrX509Cert *pstrCert = (tstrX509Cert *)pvCert;
	if (pstrCert != NULL) {
#define X509_DUMP_ENABLE
#ifdef X509_DUMP_ENABLE
		uint32 i;
		tstrSystemTime strTime = {0};

		printf("\r\n");
		printf("*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=\r\n");
		printf("                   ROOT CERTIFICATE                     \r\n");
		printf("*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=\r\n");

		printf("VERSION              %u\r\n", pstrCert->u8Version + 1);
		printf("SERIAL NO            ");
		for (i = 0; i < pstrCert->u8SerialNumberLength; i++) {
			printf("%02X ", pstrCert->au8SerialNo[i]);
		}
		printf("\r\n");

		printf("SIGNATURE ALGORITHM  ");
		if (pstrCert->u8SignAlg == X509_SIGN_MD5RSA) {
			printf("md5WithRsaEncryption");
		} else if (pstrCert->u8SignAlg == X509_SIGN_SHA1RSA) {
			printf("shaWithRsaEncryption");
		} else if (pstrCert->u8SignAlg == X509_SIGN_SHA256RSA) {
			printf("sha256WithRsaEncryption");
		} else if (pstrCert->u8SignAlg == X509_SIGN_SHA384RSA) {
			printf("sha384WithRsaEncryption");
		} else if (pstrCert->u8SignAlg == X509_SIGN_SHA512RSA) {
			printf("sha512WithRsaEncryption");
		}

		printf("\r\n");

		printf("ISSUER               ");
		if (strlen(pstrCert->strIssuer.acCmnName) != 0) {
			printf("%s", pstrCert->strIssuer.acCmnName);
		}

		printf("\r\n");

		printf("SUBJECT              ");
		if (strlen(pstrCert->strSubject.acCmnName) != 0) {
			printf("%s", pstrCert->strSubject.acCmnName);
		}

		printf("\r\n");

		printf("VALIDITY\n");
		printf("  Not Before :       ");
		Cert_DecodeTime(pstrCert->strStartDate.au8Time, &strTime);
		printf("%d-%02d-%02d %02d:%02d:%02d\r\n",
				strTime.u16Year, strTime.u8Month, strTime.u8Day,
				strTime.u8Hour, strTime.u8Minute, strTime.u8Second);

		printf("  Not After  :       ");
		Cert_DecodeTime(pstrCert->strExpiryDate.au8Time, &strTime);
		printf("%d-%02d-%02d %02d:%02d:%02d\r\n",
				strTime.u16Year, strTime.u8Month, strTime.u8Day,
				strTime.u8Hour, strTime.u8Minute, strTime.u8Second);

		if (pstrCert->strPubKey.enuType == PUBKEY_ALG_RSA) {
			printf("PUBLIC KEY           ");
			printf("RSA\n\n");
			printf("\tModulus");
			for (i = 0; i < pstrCert->strPubKey.strRSAKey.u16NSize; i++) {
				if (!(i % 16)) {
					printf("\r\n\t\t");
				}

				printf("%02X ", pstrCert->strPubKey.strRSAKey.pu8N[i]);
			}
			printf("\r\n");

			printf("\tExponent");
			for (i = 0; i < pstrCert->strPubKey.strRSAKey.u16ESize; i++) {
				if (!(i % 16)) {
					printf("\r\n\t\t");
				}

				printf("%02X ", pstrCert->strPubKey.strRSAKey.pu8E[i]);
			}
		}

		printf("\r\n");

		printf("SIGNATURE");
		for (i = 0; i < pstrCert->strSignature.u16BufferSize; i++) {
			if (!(i % 16)) {
				printf("\r\n\t");
			}

			printf("%02X ", pstrCert->strSignature.pu8Data[i]);
		}
		printf("\r\n");
		printf("*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=\n");
#else
		M2M_INFO("*=*=* X509 *=*=*\n");
		M2M_INFO("\tSubject <%s>\n", ((strlen(pstrCert->strSubject.acCmnName) != 0) ? pstrCert->strSubject.acCmnName    : " "));
		M2M_INFO("\tIssuer  <%s>\n", ((strlen(pstrCert->strIssuer.acCmnName) != 0) ? pstrCert->strIssuer.acCmnName   : " "));
#endif
	}
}

/**
 *
 * \file
 *
 * \brief ASN1.
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

#include "root_cert/x509/asn1.h"

uint16 ASN1_GetNextElement(tstrAsn1Context *pstrAsn1Ctxt, tstrAsn1Element *pstrElement)
{
	uint16 u16ElemLength = 0;

	if ((pstrElement != NULL) && (pstrAsn1Ctxt != NULL)) {
		uint8 u8NLenBytes;

		do {
			u8NLenBytes = 0;

			/* Get the ASN.1 Element Tag. */
			pstrElement->u8Tag = *pstrAsn1Ctxt->pu8Data++;

			/* Get the ASN.1 element length. */
			pstrElement->u32Length = *pstrAsn1Ctxt->pu8Data++;
			pstrElement->u32Length &= 0xFF;
			if (pstrElement->u32Length & NBIT7) {
				uint8 u8Idx;
				uint8 au8Tmp[4];

				/* Multiple Length octets. */
				u8NLenBytes = (uint8)(pstrElement->u32Length & 0x03);
				pstrElement->u32Length = 0;

				m2m_memcpy(au8Tmp, pstrAsn1Ctxt->pu8Data, u8NLenBytes);
				pstrAsn1Ctxt->pu8Data += u8NLenBytes;

				for (u8Idx = 0; u8Idx < u8NLenBytes; u8Idx++) {
					pstrElement->u32Length += 
							(uint32)(au8Tmp[u8Idx] << ((u8NLenBytes - u8Idx - 1) * 8));
				}
			}

			u16ElemLength += u8NLenBytes + 2 + (uint16)pstrElement->u32Length;
		} while (pstrElement->u8Tag == ASN1_NULL);
	}

	return u16ElemLength;
}

uint16 ASN1_Read(tstrAsn1Context *pstrAsn1Cxt, uint32 u32ReadLength, uint8 *pu8ReadBuffer)
{
	uint16 u16Read = 0;

	if (pstrAsn1Cxt != NULL) {
		uint16 u16RemLen = pstrAsn1Cxt->u16BuffLen - (uint16)(pstrAsn1Cxt->pu8Data - pstrAsn1Cxt->pu8Buff);

		u16Read = (uint16)((u32ReadLength <= u16RemLen) ? u32ReadLength : u16RemLen);

		if (pu8ReadBuffer != NULL) {
			m2m_memcpy(pu8ReadBuffer, pstrAsn1Cxt->pu8Data, u16Read);
		}

		pstrAsn1Cxt->pu8Data += u16Read;
	}

	return u16Read;
}

uint16 ASN1_ReadReference(tstrAsn1Context *pstrAsn1Cxt, uint32 u32ReadLength, uint8 **ppu8ReadBuffer)
{
	uint16 u16Read = 0;

	if (pstrAsn1Cxt != NULL) {
		uint16 u16RemLen = pstrAsn1Cxt->u16BuffLen - (uint16)(pstrAsn1Cxt->pu8Data - pstrAsn1Cxt->pu8Buff);

		u16Read = (uint16)((u32ReadLength <= u16RemLen) ? u32ReadLength : u16RemLen);

		if (ppu8ReadBuffer != NULL) {
			*ppu8ReadBuffer =  pstrAsn1Cxt->pu8Data;
		}

		pstrAsn1Cxt->pu8Data += u16Read;
	}

	return u16Read;
}

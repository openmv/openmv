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

#ifndef ASN1_H_INCLUDED
#define ASN1_H_INCLUDED

#include "common/include/nm_common.h"

/* ASN.1 TAG DEFINITIONS. */
#define ASN1_INVALID                                    0x00
#define ASN1_BOOLEAN                                    0x01
#define ASN1_INTEGER                                    0x02
#define ASN1_BIT_STRING                                 0x03
#define ASN1_OCTET_STRING                               0x04
#define ASN1_NULL                                       0x05
#define ASN1_OBJECT_IDENTIFIER                          0x06
#define ASN1_UTF8_DTRING                                0x0C
#define ASN1_PRINTABLE_STRING                           0x13
#define ASN1_TELETEX_STRING                             0x14
#define ASN1_UTC_TIME                                   0x17
#define ASN1_GENERALIZED_TIME                           0x18
#define ASN1_SEQUENCE                                   0x30
#define ASN1_SET                                        0x31

#define ASN1_SUCCESS                                    0

#define ASN1_FAIL                                       -1

typedef struct {
	uint8 *pu8Buff;
	uint8 *pu8Data;
	uint16 u16BuffLen;
} tstrAsn1Context;

typedef struct {
	uint32 u32Length;
	uint8 u8Tag;
} tstrAsn1Element;

uint16 ASN1_GetNextElement(tstrAsn1Context *pstrAsn1Ctxt, tstrAsn1Element *pstrElement);

uint16 ASN1_Read(tstrAsn1Context *pstrAsn1Cxt, uint32 u32ReadLength, uint8 *pu8ReadBuffer);

uint16 ASN1_ReadReference(tstrAsn1Context *pstrAsn1Cxt, uint32 u32ReadLength, uint8 **ppu8ReadBuffer);

#endif /* ASN1_H_INCLUDED */

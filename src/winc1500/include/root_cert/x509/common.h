/**
 *
 * \file
 *
 * \brief Common Functions.
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

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include "common/include/nm_common.h"
#include <string.h>
#ifdef WIN32
#include <Windows.h>
#endif

typedef struct {
	uint16 u16BufferSize;
	uint8 *pu8Data;
} tstrBuffer;

/* Used for time storage. */
typedef struct {
	uint16 u16Year;
	uint8 u8Month;
	uint8 u8Day;
	uint8 u8Hour;
	uint8 u8Minute;
	uint8 u8Second;
} tstrSystemTime;

#define SIZE_128_BITS                                   (16)
#define SIZE_256_BITS                                   (32)
#define SIZE_512_BITS                                   (64)
#define SIZE_1024_BITS                                  (128)
#define SIZE_1536_BITS                                  (192)
#define SIZE_2048_BITS                                  (256)

#define GETU16(BUF, OFFSET)                              ((((uint16)((BUF)[OFFSET]) << 8)) | (((uint16)(BUF)[OFFSET + 1])))

/*!< Retrieve 2 bytes from the given buffer at the given
 *      offset as 16-bit unsigned integer in the Network byte order.
 */

#define GETU32(BUF,OFFSET)                              ((((uint32)((BUF)[OFFSET]) << 24)) | (((uint32)((BUF)[OFFSET + 1]) << 16)) | \
                                                        (((uint32)((BUF)[OFFSET + 2]) << 8)) | ((uint32)((BUF)[OFFSET + 3])))
/*!< Retrieve 4 bytes from the given buffer at the given 
	offset as 32-bit unsigned integer in the Network byte order.
*/

#define PUTU32(VAL32, BUF, OFFSET) \
	do \
	{  \
		(BUF)[OFFSET     ] = BYTE_3((VAL32)); \
		(BUF)[OFFSET + 1 ] = BYTE_2((VAL32)); \
		(BUF)[OFFSET + 2 ] = BYTE_1((VAL32)); \
		(BUF)[OFFSET + 3 ] = BYTE_0((VAL32)); \
	} while (0)

#define PUTU16(VAL16, BUF, OFFSET) \
	do \
	{  \
		(BUF)[OFFSET     ] = BYTE_1((VAL16)); \
		(BUF)[OFFSET + 1 ] = BYTE_0((VAL16)); \
	} while (0)

#endif /* COMMON_H_INCLUDED */

/**
 *
 * \file
 *
 * \brief Hash Functions.
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

#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

#include "common.h"

#define SHA_BLOCK_SIZE                          64

#define MD5_DIGEST_SIZE                         (16)
#define SHA1_DIGEST_SIZE                        (20)
#define SHA256_DIGEST_SIZE                      32

#define SHA_MAX_DIGEST_SIZE                     SHA256_DIGEST_SIZE

#define SHA_FLAGS_INIT                          0x01

/*!< A flag to tell the hash function to
 *      start hash from the initial state.
 */

#define SHA_FLAGS_UPDATE                        0x02

/*!< Update the current hash with the
 *      given data.
 */

#define SHA_FLAGS_FINISH                        0x04

/*!< Finalize the hashing and calculate
 *      the final result.
 */

#define SHA_FLAGS_FULL_HASH                     (SHA_FLAGS_INIT | SHA_FLAGS_UPDATE | SHA_FLAGS_FINISH)

#define SHA1_INIT(ctxt)                         SHA1_Hash((ctxt), SHA_FLAGS_INIT, NULL, 0, NULL)
#define SHA1_UPDATE(ctxt, data, dataLen)        SHA1_Hash((ctxt), SHA_FLAGS_UPDATE, (data), (dataLen), NULL)
#define SHA1_FINISH(ctxt, digest)               SHA1_Hash((ctxt), SHA_FLAGS_FINISH, NULL, 0, digest)

#define MD5_INIT(ctxt)                          MD5_Hash((ctxt), SHA_FLAGS_INIT, NULL, 0, NULL)
#define MD5_UPDATE(ctxt, data, dataLen)         MD5_Hash((ctxt), SHA_FLAGS_UPDATE, (data), (dataLen), NULL)
#define MD5_FINISH(ctxt, digest)                MD5_Hash((ctxt), SHA_FLAGS_FINISH, NULL, 0, digest)

typedef void (*tpfHashProcessBlock)(uint32 *pu32HashState, const uint8 *pu8MessageBlock);

typedef struct {
	uint32 au32HashState[SHA_MAX_DIGEST_SIZE / 4];
	uint8 au8CurrentBlock[SHA_BLOCK_SIZE];
	uint32 u32TotalLength;
	tpfHashProcessBlock fpHash;
	uint8 u8BlockSize;
	uint8 u8DigestSize;
} tstrHashContext;

typedef tstrHashContext tstrMd5Context;
typedef tstrHashContext tstrSha1Context;
typedef tstrHashContext tstrSha256Context;

typedef void (*tpfHash)
(
	tstrHashContext *pstrHashContext,
	uint8 u8Flags,
	uint8 *pu8Data,
	uint32 u32DataLength,
	uint8 *pu8Digest
);

void MD5_Hash(tstrHashContext *pstrMD5Context, uint8 u8Flags, uint8 *pu8Data, uint32 u32DataLength, uint8 *pu8Digest);
void HMAC_Vector(tpfHash fpHash, uint8 *pu8Key, uint32 u32KeyLength, tstrBuffer *pstrInData, uint8 u8NumInputs, uint8 *pu8Out);
void SHA1_Hash(tstrHashContext *pstrSha1Context, uint8 u8Flags, uint8 *pu8Data, uint32 u32DataLength, uint8 *pu8Digest);
void SHA256_Hash(tstrHashContext *pstrSha256Context, uint8 u8Flags, uint8 *pu8Data, uint32 u32DataLength, uint8 *pu8Digest);

#endif /* HASH_H_INCLUDED */

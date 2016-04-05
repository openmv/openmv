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

#include "root_cert/x509/hash.h"

#define SHA_SHR(x, n)                            ((x & 0xFFFFFFFF) >> n)
#define SHA_SHL(x, n)                            ((x & 0xFFFFFFFF) << n)
#define SHA_ROTR(x, n)                           (SHA_SHR(x, n) | SHA_SHL(x, (32 - n)))
#define SHA_ROTL(x, n)                           (SHA_SHL(x, n) | SHA_SHR(x, (32 - n)))

/* MD5 specific defines. */
#define F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | ~(z)))

#define MD5_INERMEDIATE(A, B, C, D, W, K, S) \
	{ \
		W       += A + K; \
		W       = B + SHA_ROTL(W, S); \
		A       = D; \
		D       = C; \
		C       = B; \
		B       = W; \
	}

/* SHA1 specific defines. */
#define SHA1_CH(x, y, z)                        (((x) & (y)) | ((~(x)) & (z)))
#define SHA1_MAJ(x, y, z)                       (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define SHA_PARITY(x, y, z)                     ((x) ^ (y) ^ (z))
#define SHA1_INERMEDIATE(A, B, C, D, E, W, K) \
	{ \
		W += SHA_ROTL(A, 5) + E  + K; \
		E = D; \
		D = C; \
		C = SHA_ROTL(B, 30); \
		B = A; \
		A = W; \
	}

#define SHA_PAD_BYTE                            0x80

/* SHA256 specific defines. */
#define SHA256_SMALL_SIGMA0(x)                  (SHA_ROTR(x, 7) ^ SHA_ROTR(x, 18) ^  SHA_SHR(x, 3))
#define SHA256_SMALL_SIGMA1(x)                  (SHA_ROTR(x, 17) ^ SHA_ROTR(x, 19) ^ SHA_SHR(x, 10))
#define SHA256_CAP_SIGMA0(x)                    (SHA_ROTR(x, 2) ^ SHA_ROTR(x, 13) ^ SHA_ROTR(x, 22))
#define SHA256_CAP_SIGMA1(x)                    (SHA_ROTR(x, 6) ^ SHA_ROTR(x, 11) ^ SHA_ROTR(x, 25))
#define SHA256_MAJ(x, y, z)                     ((x & y) | (z & (x | y)))
#define SHA256_CH(x, y, z)                      (z ^ (x & (y ^ z)))

static void MD5ProcessBlock(uint32 *pu32Md5State, const uint8 *pu8MessageBlock)
{
	const uint32 T[] =    {
		0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE, 0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
		0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE, 0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
		0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA, 0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
		0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED, 0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
		0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C, 0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
		0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05, 0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
		0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039, 0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
		0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1, 0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
	};
	uint8 S[] = {
		7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
		6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
	};
	uint32 A, B, C, D;
	uint32 W = 0;
	uint32 X[16];
	uint8 u8Idx, u8Count;

	A = pu32Md5State[0];
	B = pu32Md5State[1];
	C = pu32Md5State[2];
	D = pu32Md5State[3];

	memcpy(X, pu8MessageBlock, 64);

	for (u8Idx = 0; u8Idx < 16; u8Idx++) {
		W = F(B, C, D) + X[u8Idx];
		MD5_INERMEDIATE(A, B, C, D, W, T[u8Idx], S[u8Idx]);
	}

	for (u8Idx = 16, u8Count = 1; u8Idx < 32; u8Idx++, u8Count += 5) {
		W = G(B, C, D) + X[u8Count & 0x0F];
		MD5_INERMEDIATE(A, B, C, D, W, T[u8Idx], S[u8Idx]);
	}

	for (u8Idx = 32, u8Count = 5; u8Idx < 48; u8Idx++, u8Count += 3) {
		W = H(B, C, D) + X[u8Count & 0x0F];
		MD5_INERMEDIATE(A, B, C, D, W, T[u8Idx], S[u8Idx]);
	}

	for (u8Idx = 48, u8Count = 0; u8Idx < 64; u8Idx++, u8Count += 7) {
		W = I(B, C, D) + X[u8Count & 0x0F];
		MD5_INERMEDIATE(A, B, C, D, W, T[u8Idx], S[u8Idx]);
	}

	pu32Md5State[0] += A;
	pu32Md5State[1] += B;
	pu32Md5State[2] += C;
	pu32Md5State[3] += D;
}

static void SHA1ProcessBlock(uint32 *pu32Sha1State, const uint8 *pu8MessageBlock)
{
	/* Constants defined in FIPS-180-2, section 4.2.1 */
	const uint32 K[4] = {
		0x5A827999,
		0x6ED9EBA1,
		0x8F1BBCDC,
		0xCA62C1D6
	};
	int t;                   /* Loop counter */
	uint32 W[80];            /* Word sequence */
	uint32 A, B, C, D, E;    /* Word buffers */

	/* Initialize the first 16 words in the array W. */
	for (t = 0; t < 16; t++) {
		W[t] = GETU32(pu8MessageBlock, (t * 4));
	}

	for (t = 16; t < 80; t++) {
		W[t] = SHA_ROTL((W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]), 1);
	}

	A = pu32Sha1State[0];
	B = pu32Sha1State[1];
	C = pu32Sha1State[2];
	D = pu32Sha1State[3];
	E = pu32Sha1State[4];

	for (t = 0; t < 20; t++) {
		W[t] += SHA1_CH(B, C, D);
		SHA1_INERMEDIATE(A, B, C, D, E, W[t], K[0])
	}

	for (t = 20; t < 40; t++) {
		W[t] += SHA_PARITY(B, C, D);
		SHA1_INERMEDIATE(A, B, C, D, E, W[t], K[1])
	}

	for (t = 40; t < 60; t++) {
		W[t] += SHA1_MAJ(B, C, D);
		SHA1_INERMEDIATE(A, B, C, D, E, W[t], K[2])
	}

	for (t = 60; t < 80; t++) {
		W[t] += SHA_PARITY(B, C, D);
		SHA1_INERMEDIATE(A, B, C, D, E, W[t], K[3])
	}

	pu32Sha1State[0] += A;
	pu32Sha1State[1] += B;
	pu32Sha1State[2] += C;
	pu32Sha1State[3] += D;
	pu32Sha1State[4] += E;
}

static void SHA256ProcessBlock(uint32 *pu32Sha256State, const uint8 *pu8Data)
{
	/* Hardcoded when using for loop unrolling. */
	/* Constants defined in SHA-256. */
	const uint32 K[] =    {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	uint32 t;                       /* Loop counter			*/
	uint32 temp1, temp2;            /* Temporary word value	*/
	uint32 W[64];                   /* Word sequence		*/
	uint32 A, B, C, D, E, F, G, H;  /* Word buffers			*/

	A = pu32Sha256State[0];
	B = pu32Sha256State[1];
	C = pu32Sha256State[2];
	D = pu32Sha256State[3];
	E = pu32Sha256State[4];
	F = pu32Sha256State[5];
	G = pu32Sha256State[6];
	H = pu32Sha256State[7];

	/*  Initialize the first 16 words in the array W */
	for (t = 16; (t--) != 0;) {
		W[t] = pu8Data[t * 4] << 24;
		W[t] |= pu8Data[t * 4 + 1] << 16;
		W[t] |= pu8Data[t * 4 + 2] << 8;
		W[t] |= pu8Data[t * 4 + 3];
	}

	for (t = 16; t < 64; t++) {
		W[t] = SHA256_SMALL_SIGMA1(W[t - 2]) + W[t - 7] +
				SHA256_SMALL_SIGMA0(W[t - 15]) + W[t - 16];
	}

	for (t = 0; t < 64; t++) {
		temp1 = H + SHA256_CAP_SIGMA1(E) + SHA256_CH(E, F, G) + K[t] + W[t];
		temp2 = SHA256_CAP_SIGMA0(A) + SHA256_MAJ(A, B, C);

		H = G;
		G = F;
		F = E;
		E = D + temp1;
		D = C;
		C = B;
		B = A;
		A = temp1 + temp2;
	}

	pu32Sha256State[0] += A;
	pu32Sha256State[1] += B;
	pu32Sha256State[2] += C;
	pu32Sha256State[3] += D;
	pu32Sha256State[4] += E;
	pu32Sha256State[5] += F;
	pu32Sha256State[6] += G;
	pu32Sha256State[7] += H;
}

static void Sha_HashUpdate(tstrHashContext *pstrHashCxt, uint8 *pu8Data, uint32 u32DataLength)
{
	if ((pstrHashCxt != NULL) && (pu8Data != NULL) && (u32DataLength != 0)) {
		if (pstrHashCxt->fpHash != NULL) {
			uint32 u32ResidualBytes;
			uint32 u32NBlocks;
			uint32 u32Offset;
			uint32 u32BlockCount;

			/* Get the remaining bytes from the previous update (if the length is not block alligned). */
			u32ResidualBytes = pstrHashCxt->u32TotalLength % pstrHashCxt->u8BlockSize;

			/* Update the total data length. */
			pstrHashCxt->u32TotalLength += u32DataLength;

			if (u32ResidualBytes != 0) {
				if ((u32ResidualBytes + u32DataLength) >= pstrHashCxt->u8BlockSize) {
					u32Offset = pstrHashCxt->u8BlockSize - u32ResidualBytes;
					memcpy(&pstrHashCxt->au8CurrentBlock[u32ResidualBytes], pu8Data, u32Offset);
					pu8Data += u32Offset;
					u32DataLength -= u32Offset;
					pstrHashCxt->fpHash(pstrHashCxt->au32HashState, pstrHashCxt->au8CurrentBlock);
				} else {
					memcpy(&pstrHashCxt->au8CurrentBlock[u32ResidualBytes], pu8Data, u32DataLength);
					u32DataLength = 0;
				}
			}

			/* Get the number of HASH BLOCKs and the residual bytes. */
			u32NBlocks = u32DataLength / pstrHashCxt->u8BlockSize;
			u32ResidualBytes = u32DataLength % pstrHashCxt->u8BlockSize;
			for (u32BlockCount = 0; u32BlockCount < u32NBlocks; u32BlockCount++) {
				pstrHashCxt->fpHash(pstrHashCxt->au32HashState, pu8Data);
				pu8Data += pstrHashCxt->u8BlockSize;
			}

			if (u32ResidualBytes != 0) {
				memcpy(pstrHashCxt->au8CurrentBlock, pu8Data, u32ResidualBytes);
			}
		}
	}
}

static void Sha_HashFinish(tstrHashContext *pstrHashCxt, uint8 *pu8LengthPadding)
{
	if (pstrHashCxt != NULL) {
		if (pstrHashCxt->fpHash != NULL) {
			uint32 u32Offset;
			uint32 u32PaddingLength;

			/* Calculate the offset of the last data byte in the current block. */
			u32Offset = pstrHashCxt->u32TotalLength % pstrHashCxt->u8BlockSize;

			/* Add the padding byte 0x80. */
			pstrHashCxt->au8CurrentBlock[u32Offset++] = SHA_PAD_BYTE;

			/* Calculate the required padding to complete one Hash Block Size. */
			u32PaddingLength = pstrHashCxt->u8BlockSize - u32Offset;
			memset(&pstrHashCxt->au8CurrentBlock[u32Offset], 0, u32PaddingLength);

			/* If the padding count is not enough to hold 64-bit representation */
			/* of the total input message length, one padding block is required. */
			if (u32PaddingLength < 8) {
				pstrHashCxt->fpHash(pstrHashCxt->au32HashState, pstrHashCxt->au8CurrentBlock);
				memset(pstrHashCxt->au8CurrentBlock, 0, pstrHashCxt->u8BlockSize);
			}

			memcpy(&pstrHashCxt->au8CurrentBlock[pstrHashCxt->u8BlockSize - 8], pu8LengthPadding, 8);
			pstrHashCxt->fpHash(pstrHashCxt->au32HashState, pstrHashCxt->au8CurrentBlock);
		}
	}
}

void SHA1_Hash(tstrHashContext *pstrSha1Context, uint8 u8Flags, uint8 *pu8Data, uint32 u32DataLength, uint8 *pu8Digest) {
	if (pstrSha1Context != NULL) {
		/* SHA1 HASH INITIALIZATION. */
		if (u8Flags & SHA_FLAGS_INIT) {
			/* Initialize the Hash Context. */
			memset(pstrSha1Context, 0, sizeof(tstrHashContext));

			/* Define SHA1 Specific parameters. */
			pstrSha1Context->fpHash = SHA1ProcessBlock;
			pstrSha1Context->u8BlockSize = SHA_BLOCK_SIZE;
			pstrSha1Context->u8DigestSize = SHA1_DIGEST_SIZE;

			/* Set the Hash state to the initial value (specified by the standard). */
			pstrSha1Context->au32HashState[0] = 0x67452301;
			pstrSha1Context->au32HashState[1] = 0xEFCDAB89;
			pstrSha1Context->au32HashState[2] = 0x98BADCFE;
			pstrSha1Context->au32HashState[3] = 0x10325476;
			pstrSha1Context->au32HashState[4] = 0xC3D2E1F0;
		}

		/* SHA1 HASH UPDATE. */
		if ((u8Flags & SHA_FLAGS_UPDATE) && (pu8Data != NULL)) {
			Sha_HashUpdate(pstrSha1Context, pu8Data, u32DataLength);
		}

		/* SHA1 HASH FINISH. */
		if (u8Flags & SHA_FLAGS_FINISH) {
			uint8 au8Tmp[8];

			/* Pack the length at the end of the padding block. */
			PUTU32(pstrSha1Context->u32TotalLength >> 29, au8Tmp, 0);
			PUTU32(pstrSha1Context->u32TotalLength << 3, au8Tmp, 4);
			Sha_HashFinish(pstrSha1Context, au8Tmp);

			/* Compute digest. */
			PUTU32(pstrSha1Context->au32HashState[0], pu8Digest, 0);
			PUTU32(pstrSha1Context->au32HashState[1], pu8Digest, 4);
			PUTU32(pstrSha1Context->au32HashState[2], pu8Digest, 8);
			PUTU32(pstrSha1Context->au32HashState[3], pu8Digest, 12);
			PUTU32(pstrSha1Context->au32HashState[4], pu8Digest, 16);
		}
	}
}

void SHA256_Hash(tstrHashContext *pstrSha256Context, uint8 u8Flags, uint8 *pu8Data, uint32 u32DataLength, uint8 *pu8Digest)
{
	if (pstrSha256Context != NULL) {
		/* SHA256 HASH INITIALIZATION. */
		if (u8Flags & SHA_FLAGS_INIT) {
			/* Initialize the Hash Context. */
			memset(pstrSha256Context, 0, sizeof(tstrHashContext));

			/* Define SHA256 specific parameters. */
			pstrSha256Context->fpHash = SHA256ProcessBlock;
			pstrSha256Context->u8BlockSize = SHA_BLOCK_SIZE;
			pstrSha256Context->u8DigestSize = SHA256_DIGEST_SIZE;

			/* Set the Hash state to the initial value (specified by the standard). */
			pstrSha256Context->au32HashState[0] = 0x6A09E667;
			pstrSha256Context->au32HashState[1] = 0xBB67AE85;
			pstrSha256Context->au32HashState[2] = 0x3C6EF372;
			pstrSha256Context->au32HashState[3] = 0xA54FF53A;
			pstrSha256Context->au32HashState[4] = 0x510E527F;
			pstrSha256Context->au32HashState[5] = 0x9B05688C;
			pstrSha256Context->au32HashState[6] = 0x1F83D9AB;
			pstrSha256Context->au32HashState[7] = 0x5BE0CD19;
		}

		/* SHA256 HASH UPDATE. */
		if ((u8Flags & SHA_FLAGS_UPDATE) && (pu8Data != NULL)) {
			Sha_HashUpdate(pstrSha256Context, pu8Data, u32DataLength);
		}

		/* SHA256 HASH FINISH. */
		if ((u8Flags & SHA_FLAGS_FINISH) && (pu8Digest != NULL)) {
			uint8 au8Tmp[8];

			/* Pack the length at the end of the padding block. */
			PUTU32(pstrSha256Context->u32TotalLength >> 29, au8Tmp, 0);
			PUTU32(pstrSha256Context->u32TotalLength << 3, au8Tmp, 4);
			Sha_HashFinish(pstrSha256Context, au8Tmp);

			/* Compute digest. */
			PUTU32(pstrSha256Context->au32HashState[0], pu8Digest, 0);
			PUTU32(pstrSha256Context->au32HashState[1], pu8Digest, 4);
			PUTU32(pstrSha256Context->au32HashState[2], pu8Digest, 8);
			PUTU32(pstrSha256Context->au32HashState[3], pu8Digest, 12);
			PUTU32(pstrSha256Context->au32HashState[4], pu8Digest, 16);
			PUTU32(pstrSha256Context->au32HashState[5], pu8Digest, 20);
			PUTU32(pstrSha256Context->au32HashState[6], pu8Digest, 24);
			PUTU32(pstrSha256Context->au32HashState[7], pu8Digest, 28);
		}
	}
}

void MD5_Hash( tstrHashContext *pstrMD5Context, uint8 u8Flags, uint8 *pu8Data, uint32 u32DataLength, uint8 *pu8Digest)
{
	if (pstrMD5Context != NULL) {
		/* SHA256 HASH INITIALIZATION. */
		if (u8Flags & 1) {
			/* Initialize the Hash Context. */
			memset(pstrMD5Context, 0, sizeof(tstrHashContext));

			/* Define SHA256 specific parameters. */
			pstrMD5Context->fpHash = MD5ProcessBlock;
			pstrMD5Context->u8BlockSize = 64;
			pstrMD5Context->u8DigestSize = 16;

			pstrMD5Context->au32HashState[0] = 0x67452301;
			pstrMD5Context->au32HashState[1] = 0xEFCDAB89;
			pstrMD5Context->au32HashState[2] = 0x98BADCFE;
			pstrMD5Context->au32HashState[3] = 0x10325476;
		}

		/* SHA256 HASH UPDATE. */
		if ((u8Flags & 2) && (pu8Data != NULL)) {
			Sha_HashUpdate(pstrMD5Context, pu8Data, u32DataLength);
		}

		/* SHA256 HASH FINISH. */
		if ((u8Flags & 4) && (pu8Digest != NULL)) {
			uint8 au8Tmp[8] = {0};
			uint8 u8Idx, u8ByteIdx = 0;
			uint32 u32LengthBits = pstrMD5Context->u32TotalLength << 3;

			/* Pack the length at the end of the padding block. */
			au8Tmp[0] = BYTE_0(u32LengthBits);
			au8Tmp[1] = BYTE_1(u32LengthBits);
			au8Tmp[2] = BYTE_2(u32LengthBits);
			au8Tmp[3] = BYTE_3(u32LengthBits);

			Sha_HashFinish(pstrMD5Context, au8Tmp);

			/* Compute digest. */
			for (u8Idx = 0; u8Idx < (MD5_DIGEST_SIZE / 4); u8Idx++) {
				pu8Digest[u8ByteIdx++] = BYTE_0(pstrMD5Context->au32HashState[u8Idx]);
				pu8Digest[u8ByteIdx++] = BYTE_1(pstrMD5Context->au32HashState[u8Idx]);
				pu8Digest[u8ByteIdx++] = BYTE_2(pstrMD5Context->au32HashState[u8Idx]);
				pu8Digest[u8ByteIdx++] = BYTE_3(pstrMD5Context->au32HashState[u8Idx]);
			}
		}
	}
}

/**
 * \brief Perform the HMAC operation on a given vector of data streams.
 */
void HMAC_Vector(tpfHash fpHash, uint8 *pu8Key, uint32 u32KeyLength, tstrBuffer *pstrInData, uint8 u8NumInputs, uint8 *pu8Out)
{
	if ((pu8Key != NULL) && (pstrInData != NULL) && (pu8Out != NULL) && (fpHash != NULL)) {
		tstrHashContext strHashContext;
		uint32 i;
		uint8 au8TmpBuffer[SHA_BLOCK_SIZE];
		uint8 au8TmpHash[SHA_MAX_DIGEST_SIZE];
		uint8 au8TmpKey[SHA_MAX_DIGEST_SIZE];

		/* HMAC INIT. */
		/* Adjust the key size. */
		if (u32KeyLength > SHA_BLOCK_SIZE) {
			fpHash(&strHashContext, SHA_FLAGS_FULL_HASH, pu8Key, u32KeyLength, pu8Key);
			u32KeyLength = strHashContext.u8DigestSize;
			pu8Key = au8TmpKey;
		}

		memset(au8TmpBuffer, 0x36, SHA_BLOCK_SIZE);
		for (i = 0; i < u32KeyLength; i++) {
			au8TmpBuffer[i] ^= pu8Key[i];
		}

		/* Start hashing. */
		fpHash(&strHashContext, SHA_FLAGS_INIT | SHA_FLAGS_UPDATE, au8TmpBuffer, SHA_BLOCK_SIZE, NULL);

		/* HMAC UPDATE. */
		for (i = 0; i < u8NumInputs; i++) {
			fpHash(&strHashContext, SHA_FLAGS_UPDATE, pstrInData[i].pu8Data, pstrInData[i].u16BufferSize, NULL);
		}

		fpHash(&strHashContext, SHA_FLAGS_FINISH, NULL, 0, au8TmpHash);

		/* HMAC Finalize. */
		memset(au8TmpBuffer, 0x5C, SHA_BLOCK_SIZE);
		for (i = 0; i < u32KeyLength; i++) {
			au8TmpBuffer[i] ^= pu8Key[i];
		}

		fpHash(&strHashContext, SHA_FLAGS_INIT | SHA_FLAGS_UPDATE, au8TmpBuffer, SHA_BLOCK_SIZE, NULL);
		fpHash(&strHashContext, SHA_FLAGS_UPDATE | SHA_FLAGS_FINISH, au8TmpHash, strHashContext.u8DigestSize, pu8Out);
	}
}

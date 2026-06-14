/*
 * Copyright (c) 2015-2022, Verisilicon Inc. - All Rights Reserved
 * Copyright (c) 2011-2014, Google Inc. - All Rights Reserved
 *
 *
 ********************************************************************************
 *
 * This software is distributed under the terms of
 * BSD-3-Clause. The following provisions apply :
 *
 ********************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ********************************************************************************
 *
 *  Abstract  : 
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------

    Table of contents

    1. Include headers
    2. External compiler flags
    3. Module defines
    4. Local function prototypes
    5. Functions

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "EncJpegPutBits.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

	EncJpegHeaderPutBits

	Write bits to stream. For example (value=2, number=4) write 0010 to the
	stream. Number of bits must be < 25, otherwise overflow occur.  Four
	bytes is maximum number of bytes to put stream and there should be at
	least 5 byte free space available because of byte buffer.
	stream[1] bits in byte buffer
	stream[0] byte buffer

	Input	stream	Pointer to the stream stucture
		value	Bit pattern
		number	Number of bits

------------------------------------------------------------------------------*/
void EncJpegHeaderPutBits(stream_s * buffer, u32 value, u32 number)
{
    u32 bits;
    u32 byteBuffer = buffer->byteBuffer;
    u8 *stream = buffer->stream;

    if(EncJpegBufferStatus(buffer) != ENCHW_OK)
    {
        return;
    }

    /* Debug: value is too big */
    ASSERT(value < ((u32) 1 << number));
    ASSERT(number < 25);

    TRACE_BIT_STREAM(value, number);

    bits = number + stream[1];

    value <<= (32 - bits);
    byteBuffer = (((u32) stream[0]) << 24) | value;

    while(bits > 7)
    {
        *stream = (u8) (byteBuffer >> 24);
        bits -= 8;
        byteBuffer <<= 8;
        stream++;
        buffer->byteCnt++;
    }

    stream[0] = (u8) (byteBuffer >> 24);
    stream[1] = (u8) bits;
    buffer->stream = stream;
    buffer->bitCnt += number;
    buffer->byteBuffer = byteBuffer;
    buffer->bufferedBits = (u8) bits;

    return;
}

/*------------------------------------------------------------------------------

	EncJpegNextByteAligned

	Function add zero stuffing until next byte aligned if needed. Note that
	stream->stream[1] is bits in byte bufer.

	Input	stream	Pointer to the stream structure.

------------------------------------------------------------------------------*/
void EncJpegNextByteAligned(stream_s * stream)
{
    if(stream->stream[1] > 0)
    {
        EncJpegHeaderPutBits(stream, 0, 8 - stream->stream[1]);
        COMMENT("Stuffing");
    }

    return;
}

/*------------------------------------------------------------------------------

	EncJpegBufferStatus

	Check fullness of stream buffer.

	Input	stream	Pointer to the stream stucture.

	Return	ENCHW_OK	Buffer status is OK.
			ENCHW_NOK	Buffer overflow.

------------------------------------------------------------------------------*/
bool_e EncJpegBufferStatus(stream_s * stream)
{
    if(stream->byteCnt + 5 > stream->size)
    {
        stream->overflow = ENCHW_YES;
        COMMENT("\nStream buffer is full     ");
        return ENCHW_NOK;
    }

    return ENCHW_OK;
}

/*------------------------------------------------------------------------------

	EncJpegSetBuffer

	Set stream buffer.

	Input	buffer	Pointer to the stream_s structure.
		stream	Pointer to stream buffer.
		size	Size of stream buffer.

------------------------------------------------------------------------------*/
bool_e EncJpegSetBuffer(stream_s * buffer, u8 * stream, u32 size)
{
    buffer->stream = stream;
    buffer->size = size;
    buffer->byteCnt = 0;
    buffer->overflow = ENCHW_NO;
    buffer->zeroBytes = 0;
    buffer->byteBuffer = 0;
    buffer->bufferedBits = 0;

    if(EncJpegBufferStatus(buffer) != ENCHW_OK)
    {
        return ENCHW_NOK;
    }
    buffer->stream[0] = 0;
    buffer->stream[1] = 0;

    return ENCHW_OK;
}

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
 *  Abstract : Bit stream handling
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "H264PutBits.h"
#include "enccommon.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/
static bool_e H264BufferStatus(stream_s * stream);

/*------------------------------------------------------------------------------

	H264PutBits

	Write bits to stream. For example (value=2, number=4) write 0010 to the
	stream. Number of bits must be < 25, otherwise overflow occur.  Four
	bytes is maximum number of bytes to put stream and there should be at
	least 5 byte free space available because of byte buffer.
    
    Used only for NAL unit headers!

	Input	stream	Pointer to the stream stucture
		value	Bit pattern
		number	Number of bits

------------------------------------------------------------------------------*/
void H264PutBits(stream_s * buffer, i32 value, i32 number)
{
    i32 bits;
    u32 byteBuffer = buffer->byteBuffer;
    u8 *stream = buffer->stream;

    if(H264BufferStatus(buffer) != ENCHW_OK)
    {
        return;
    }

    TRACE_BIT_STREAM(value, number);

    /* Debug: value is too big */
    ASSERT(value < (1 << number));
    ASSERT(number < 25);

    bits = number + buffer->bufferedBits;
    value <<= (32 - bits);
    byteBuffer = byteBuffer | value;

    while(bits > 7)
    {
        *stream = (u8) (byteBuffer >> 24);

        bits -= 8;
        byteBuffer <<= 8;
        stream++;
        buffer->byteCnt++;
    }

    buffer->byteBuffer = byteBuffer;
    buffer->bufferedBits = (u8) bits;
    buffer->stream = stream;

    return;
}

/*------------------------------------------------------------------------------

	H264PutNalBits

	Write bits to stream. For example (value=2, number=4) write 0010 to the
	stream. Number of bits must be < 25, otherwise overflow occur.  Four
	bytes is maximum number of bytes to put stream and there should be at
	least 5 byte free space available because of byte buffer.

    Used only for NAL unit RBSP data!

	Input	stream	Pointer to the stream stucture
		value	Bit pattern
		number	Number of bits

------------------------------------------------------------------------------*/
void H264PutNalBits(stream_s * buffer, i32 value, i32 number)
{
    i32 bits;
    u8 *stream = buffer->stream;
    u32 byteBuffer = buffer->byteBuffer;
	
    ASSERT(value < (1<<number));
	ASSERT(number < 25);

    TRACE_BIT_STREAM(value, number);

    bits = number + buffer->bufferedBits;
    byteBuffer = byteBuffer | ((u32) value << (32 - bits));

    while(bits > 7)
    {
        i32 zeroBytes = buffer->zeroBytes;
        i32 byteCnt = buffer->byteCnt;

        if(H264BufferStatus(buffer) != ENCHW_OK)
            return;

        *stream = (u8) (byteBuffer >> 24);
        byteCnt++;

        if((zeroBytes == 2) && (*stream < 4))
        {
            *stream++ = 3;
            *stream = (u8) (byteBuffer >> 24);
            byteCnt++;
            zeroBytes = 0;
            buffer->emulCnt++;
        }

        if(*stream == 0)
            zeroBytes++;
        else
            zeroBytes = 0;

        bits -= 8;
        byteBuffer <<= 8;
        stream++;
        buffer->zeroBytes = zeroBytes;
        buffer->byteCnt = byteCnt;
        buffer->stream = stream;
    }

    buffer->bufferedBits = (u8) bits;
    buffer->byteBuffer = byteBuffer;
}

/*------------------------------------------------------------------------------

	EncSetBuffer

	Set stream buffer.

	Input	buffer	Pointer to the stream_s structure.
		stream	Pointer to stream buffer.
		size	Size of stream buffer.

------------------------------------------------------------------------------*/
bool_e H264SetBuffer(stream_s * buffer, u8 * stream, u32 size)
{
    buffer->stream = stream;
    buffer->size = size;
    buffer->byteCnt = 0;
    buffer->overflow = ENCHW_NO;
    buffer->zeroBytes = 0;
    buffer->byteBuffer = 0;
    buffer->bufferedBits = 0;

    if(H264BufferStatus(buffer) != ENCHW_OK)
    {
        return ENCHW_NOK;
    }
    buffer->stream[0] = 0;
    buffer->stream[1] = 0;

    return ENCHW_OK;
}

/*------------------------------------------------------------------------------

	BufferStatus

	Check fullness of stream buffer.

	Input	stream	Pointer to the stream stucture.

	Return	ENCHW_OK	Buffer status is ENCHW_OK.
		ENCHW_NOK	Buffer overflow.

------------------------------------------------------------------------------*/
bool_e H264BufferStatus(stream_s * stream)
{
    if(stream->byteCnt + 5 > stream->size)
    {
        stream->overflow = ENCHW_YES;
        return ENCHW_NOK;
    }

    return ENCHW_OK;
}

/*------------------------------------------------------------------------------

	H264ExpGolombUnsigned

------------------------------------------------------------------------------*/
void H264ExpGolombUnsigned(stream_s * stream, u32 val)
{
    u32 numBits = 0;

    val++;
    while(val >> ++numBits);

    if(numBits > 12)
    {
        u32 tmp;
        tmp = numBits-1;

        if(tmp > 24)
        {
            tmp -= 24;
            H264NalBits(stream,  0, 24);
        }

        H264NalBits(stream,  0, tmp);
        COMMENT("++");

        if(numBits > 24)
        {
            numBits -= 24;
            H264NalBits(stream,  val >> numBits, 24);
            val = val >> numBits;
        }

        H264NalBits(stream,  val, numBits);
    }
    else
    {
        H264NalBits(stream,  val , 2*numBits - 1);
    }

}

/*------------------------------------------------------------------------------

	H264ExpGolombSigned

------------------------------------------------------------------------------*/
void H264ExpGolombSigned(stream_s * stream, i32 val)
{
    u32 tmp;

    if (val > 0)
        tmp = (u32) (2 * val - 1);
    else
        tmp = (u32) (-2 * val);

    H264ExpGolombUnsigned(stream, tmp);
}

/*------------------------------------------------------------------------------

	H264RbspTrailingBits

	Function add rbsp_stop_one_bit and p_alignment_zero_bit until next byte
	aligned if needed. Note that stream->stream[1] is bits in byte bufer.

	Input	stream	Pointer to the stream structure.

------------------------------------------------------------------------------*/
void H264RbspTrailingBits(stream_s * stream)
{
    H264PutNalBits(stream, 1, 1);
    COMMENT("rbsp_stop_one_bit");
    if(stream->bufferedBits > 0)
    {
        H264PutNalBits(stream, 0, 8 - stream->bufferedBits);
        COMMENT("bsp_alignment_zero_bit(s)");
    }

    return;
}

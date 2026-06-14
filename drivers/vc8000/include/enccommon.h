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
 *  Description : Encoder common definitions for control code and system model
 *
 ********************************************************************************
 */

#ifndef __ENC_COMMON_H__
#define __ENC_COMMON_H__

/*------------------------------------------------------------------------------
    1. External compiler flags
------------------------------------------------------------------------------*/

/* Encoder global definitions
 *
 * _ASSERT_USED     # Asserts enabled
 * _DEBUG_PRINT     # Prints debug information on stdout
 * TRACE_STREAM     # Creates stream trace file
 * TEST_DATA        # Creates test data files
 *
 * Can be defined here or using compiler flags */

#define DNF_PARAM_REGS

/*------------------------------------------------------------------------------
    2. Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "ewl.h"

/* Test data generation requires stream trace */
#ifdef TEST_DATA
#ifndef TRACE_STREAM
#define TRACE_STREAM
#endif
#endif

/* Stream tracing requires encdebug.h */
#ifdef TRACE_STREAM
#ifndef H1_HAVE_ENCDEBUG_H
#define H1_HAVE_ENCDEBUG_H
#endif
#endif

#ifdef H1_HAVE_ENCDEBUG_H
#include "encdebug.h"
#else
#define ASSERT(expr)
#define DEBUG_PRINT(args)
#define COMMENT(x)
#define COMMENTMBTYPE(x,y)
#define TRACE_BIT_STREAM(v,n)
#endif

#ifdef H1_HAVE_ENCTRACE_H
#include "enctrace.h"
#endif

/*------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

typedef enum
{
    ENCHW_NOK = -1,
    ENCHW_OK = 0
} bool_e;

typedef enum
{
    ENCHW_NO = 0,
    ENCHW_YES = 1
} true_e;

typedef enum
{
    NONIDR = 1, /* Coded slice of a non-IDR picture */
    IDR = 5,    /* Coded slice of an IDR picture */
    SEI = 6,    /* SEI message */
    SPSET = 7,  /* Sequence parameter set */
    PPSET = 8,  /* Picture parameter set */
    AUD_NUT   = 9,
    ENDOFSEQUENCE = 10, /* End of sequence */
    ENDOFSTREAM = 11,   /* End of stream */
    FILLERDATA = 12, /* Filler data */
    PREFIX = 14,    /* Prefix */
    SSPSET = 15,    /* Subset sequence parameter set */
    MVC = 20        /* Coded slice of a view picture */
} nalUnitType_e;

/* VLC TABLE */
typedef struct
{
    i32 value;  /* Value of bits  */
    i32 number; /* Number of bits */
} table_s;

/* used in stream buffer handling */
typedef struct
{
    u8 *stream; /* Pointer to next byte of stream */
    u32 size;   /* Byte size of stream buffer */
    u32 byteCnt;    /* Byte counter */
    u32 bitCnt; /* Bit counter */
    u32 byteBuffer; /* Byte buffer */
    u32 bufferedBits;   /* Amount of bits in byte buffer, [0-7] */
    u32 zeroBytes;  /* Amount of consecutive zero bytes */
    i32 overflow;    /* This will signal a buffer overflow */
    u32 emulCnt; /* Counter for emulation_3_byte, needed in SEI */
    i32 *table;         /* Video packet or Gob sizes */
    i32 tableSize;      /* Size of above table */
    i32 tableCnt;       /* Table counter of above table */
} stream_s;

/* General tools */
#define ABS(x)          ((x) < (0) ? -(x) : (x))
#ifndef MAX
#define MAX(a, b)       ((a) > (b) ?  (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b)       ((a) < (b) ?  (a) : (b))
#endif
#define SIGN(a)         ((a) < (0) ? (-1) : (1))
#define OUT_OF_RANGE(x,a,b) ((((i32)x)<(a)?(1):(0)) || ((x)>(b)?(1):(0)))
#define CLIP3(v, min, max)  ((v) < (min) ? (min) : ((v) > (max) ? (max) : (v)))

/* VP8 MB output information. Amount of data is 32 bytes/macroblock. */

/* Encoder MB output information defined as a struct.

   NOTE!    By defining this struct we rely that ASIC output endianess
            is configured properly to produce output that is identical
            to the way the compiler constructs this struct. */

typedef struct
{
    u8  mode;           /* refIdx 2b, chromaIntraMode 2b, mbMode 4b */
    u8  nonZeroCnt;
    u16 bitCount;
    i8  mvY[4];
    i16 mvX[4];
    u8  intra4x4[8];    /* 4 bits per block */
    u16 intraSad;
    u16 interSad;
    u8  inputMad_div128;
    u8  yMean;
    u8  cbMean;
    u8  crMean;
    u8  boostQp;       /* H264 macroblock autoboost qp */
    u8  dummy0;        /* Dummy (not used) place holder */
    u16 varRecon[3];   /* Mb variance after reconstruction but before DB-filter */
    u8  dummy1[2];     /* Dummy (not used) place holder */
    u16 varInput[3];   /* Mb variance before encoding loop */
} encOutputMbInfo_s;

typedef struct
{
    u8  mode;           /* refIdx 2b, chromaIntraMode 2b, mbMode 4b */
    u8  nonZeroCnt;
    u16 bitCount;
    i8  mvY[4];
    i16 mvX[4];
    u8  intra4x4[8];    /* 4 bits per block */
    u16 intraSad;
    u16 interSad;
    u8  inputMad_div128;
    u8  yMean;
    u8  cbMean;
    u8  crMean;
    u8  boostQp;       /* H264 macroblock autoboost qp */
    u8  dummy0;        /* Dummy (not used) place holder */
    u16 varRecon[3];   /* Mb variance after reconstruction but before DB-filter */
    u8  dummy1[2];     /* Dummy (not used) place holder */
    u16 varInput[3];   /* Mb variance before encoding loop */
    u8  data[8];
} encOutputMbInfoDebug_s;

/* Masks for encOutputMbInfo.mode */
#define MBOUT_8_MODE_MASK           0x0F
#define MBOUT_8_CHROMA_MODE_MASK    0x30
#define MBOUT_8_REFIDX_MASK         0xC0

/* MB output information. Amount of data/mb in 16-bit words / bytes.
   This is how things used to be before Foxtail release.
   Older models rely on these defines.
   For Foxtail data has increased to 48 bytes/MB, this is defined in model. */
#define MBOUT_16            16
#define MBOUT_8             32

/* 16-bit pointer offset to field. */
#define MBOUT_16_BITCOUNT   1
#define MBOUT_16_MV_B0_X    4
#define MBOUT_16_MV_B1_X    5
#define MBOUT_16_MV_B2_X    6
#define MBOUT_16_MV_B3_X    7
#define MBOUT_16_INTRA_SAD  12
#define MBOUT_16_INTER_SAD  13

/* 8-bit pointer offset to field. */
#define MBOUT_8_MODE        0
#define MBOUT_8_NONZERO_CNT 1
#define MBOUT_8_MV_B0_Y     4
#define MBOUT_8_MV_B1_Y     5
#define MBOUT_8_MV_B2_Y     6
#define MBOUT_8_MV_B3_Y     7
#define MBOUT_8_I4X4        16
#define MBOUT_8_INPUT_MAD   28
#define MBOUT_8_LUMA_MEAN   29
#define MBOUT_8_CB_MEAN     30
#define MBOUT_8_CR_MEAN     31

#define MBOUT_8_BOOS_QP     32
#define MBOUT_8_VAR_REC_LUM 34		/* varRecon[0] */
#define MBOUT_8_VAR_INP_LUM 42		/* varInput[0] */

/* for denoise */
#define USE_TOP_CTRL_DENOISE (1)
#define FIX_POINT_BIT_WIDTH (10)
#define SIG_RECI_BIT_WIDTH  (12)
#define MB_NUM_BIT_WIDTH    (20)

#define SNR_MAX (2<<FIX_POINT_BIT_WIDTH)
#define SNR_MIN (1<<FIX_POINT_BIT_WIDTH)
#define SIGMA_RANGE (20)
#define SIGMA_MAX   (30)
#define FILTER_STRENGTH_H (1<<FIX_POINT_BIT_WIDTH)
#define FILTER_STRENGTH_L (1<<(FIX_POINT_BIT_WIDTH-1))
#define FILTER_STRENGTH_PARAM ((FILTER_STRENGTH_H-FILTER_STRENGTH_L)/SIGMA_RANGE)
#define SIGMA_SMOOTH_NUM  (5)

/*------------------------------------------------------------------------------
 
------------------------------------------------------------------------------*/
typedef void (*EncInputMBLineBufCallBackFunc)(void *pAppData);

typedef struct
{
    u32 inputLineBufEn;     /* enable input image control signals */
    u32 inputLineBufLoopBackEn;    /* input buffer loopback mode enable */
    u32 inputLineBufDepth; /* input loopback buffer size in mb lines */
    u32 inputLineBufHwModeEn; /* hw handshake mode */
    u32 wrCnt;
    EncInputMBLineBufCallBackFunc cbFunc; /* call back function for line buffer interrupt */
} inputLineBuf_s;

#endif

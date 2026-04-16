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
    2. Module defines
    3. Data types
    4. Function prototypes

------------------------------------------------------------------------------*/
#ifndef __ENC_JPEG_H__
#define __ENC_JPEG_H__

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "jpegencapi.h"

#include "basetype.h"
#include "enccommon.h"

#include "EncJpegPutBits.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/

#define MAX_NUMBER_OF_COMPONENTS 3

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

enum
{
    ENC_WHOLE_FRAME,
    ENC_PARTIAL_FRAME,
    ENC_420_MODE,
    ENC_422_MODE
};

typedef enum
{
    ENC_NO_UNITS = 0,
    ENC_DOTS_PER_INCH = 1,
    ENC_DOTS_PER_CM = 2
} EncAppUnitsType;

enum
{
    ENC_SINGLE_MARKER,
    ENC_MULTI_MARKER
};

typedef struct JpegEncQuantTables_t
{
    const u8 *pQlumi;
    const u8 *pQchromi;

} JpegEncQuantTables;

typedef struct JpegEncFrameHeader_t /* SOF0 */
{
    u32 header;
    u32 Lf;
    u32 P;
    u32 Y;
    u32 X;
    u32 Nf;
    u32 Ci[MAX_NUMBER_OF_COMPONENTS];
    u32 Hi[MAX_NUMBER_OF_COMPONENTS];
    u32 Vi[MAX_NUMBER_OF_COMPONENTS];
    u32 Tqi[MAX_NUMBER_OF_COMPONENTS];

} JpegEncFrameHeader;

typedef struct JpegEncCommentHeader_t   /* COM */
{
    u32 comEnable;
    u32 Lc;
    u32 comLen;
    const u8 *pComment;

} JpegEncCommentHeader;

typedef struct JpegEncRestart_t /* DRI */
{
    u32 Lr;
    u32 Ri;

} JpegEncRestart;

typedef struct JpegEncAppn_t    /* APP0 */
{
    u32 Lp;
    u32 ident1;
    u32 ident2;
    u32 ident3;
    u32 version;
    u32 units;
    u32 Xdensity;
    u32 Ydensity;
    /*u32 XThumbnail;*/
    /*u32 YThumbnail;*/
    /*u32 thumbMode;*/
    /*u32 rgb1;*/
    /*u32 rgb2;*/
    u32 thumbEnable;
    /*u32 thumbSize;*/
    /*u32 targetStart;*/
    /*u32 targetEnd;*/
    /*u8 *pStartOfOutput;*/
    /*u8 *pHor;*/
    /*u8 *pVer;*/
    /*u32 appExtLp;*/
    /*u32 appExtId1;*/
    /*u32 appExtId2;*/
    /*u32 appExtId3;*/
    /*u32 extCode;*/
} JpegEncAppn_t;

typedef struct
{
    true_e header;
    JpegEncRestart restart; /* Restart Interval             */
    i32 rstCount;
    JpegEncFrameHeader frame;   /* Frame Header Data            */
    JpegEncCommentHeader com;   /* COM Header Data              */
    JpegEncAppn_t appn; /* APPn Header Data             */
    JpegEncQuantTables qTable;
    i32 markerType;
    i32 codingType; /* Whole or slice */
    i32 codingMode; /* 420 or 422 */
    i32 sliceNum;   /* Number of current input slice */
    i32 sliceRows;  /* Amount of MB rows in a slice */
    /*i32 rotation;*/   /* Rotation 0/-90*+90 */
    i32 width;
    i32 height;
    i32 mbNum;
    i32 mbPerFrame;
    i32 row;
    /*i32 column;*/
    /*i32 lastColumn;*/
    /*i32 dcAbove[6];*/
    /*i32 dcCurrent[6];*/
    /*i32 dc[6];*/  /* Macroblock DC */
    /*i32 rlcCount[6];*/    /* Block RLC count */
    /*const i16 *rlc[6];*/  /* RLC data for each block */
    u8 qTableLuma[64];
    u8 qTableChroma[64];
    u8 *streamStartAddress; /* output start address */
    JpegEncThumb thumbnail; /* thumbnail data */
} jpegData_s;

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

void EncJpegInit(jpegData_s * jpeg);

u32 EncJpegHdr(stream_s * stream, jpegData_s * data);

/*void EncJpegImageEnd(stream_s * stream, jpegData_s * data);*/

/*void EncJpegImageEndReplaceRst(stream_s * stream, jpegData_s * data);*/

#endif

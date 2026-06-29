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
 *  Abstract : H264 Sequence Parameter Set
 *
 ********************************************************************************
 */

#ifndef __H264_SEQUENCE_PARAMETER_SET_h__
#define __H264_SEQUENCE_PARAMETER_SET_h__

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "basetype.h"
#include "enccommon.h"
#include "H264PutBits.h"
#include "H264Slice.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

typedef struct
{
    u32 timeScale;
    u32 numUnitsInTick;
    u32 bitStreamRestrictionFlag;
    u32 videoFullRange;
    u32 sarWidth;
    u32 sarHeight;
    u32 nalHrdParametersPresentFlag;
    u32 vclHrdParametersPresentFlag;
    u32 pictStructPresentFlag;
    u32 initialCpbRemovalDelayLength;
    u32 cpbRemovalDelayLength;
    u32 dpbOutputDelayLength;
    u32 timeOffsetLength;
    u32 bitRate;
    u32 cpbSize;
} vui_t;

typedef struct
{
    true_e byteStream;
    u32 profileIdc;
    true_e constraintSet0;
    true_e constraintSet1;
    true_e constraintSet2;
    true_e constraintSet3;
    u32 levelIdc;
    u32 levelIdx;
    u32 seqParameterSetId;
    i32 log2MaxFrameNumMinus4;
    u32 picOrderCntType;
    u32 numRefFrames;
    true_e gapsInFrameNumValueAllowed;
    i32 picWidthInMbsMinus1;
    i32 picHeightInMapUnitsMinus1;
    true_e frameMbsOnly;
    true_e direct8x8Inference;
    true_e frameCropping;
    true_e vuiParametersPresent;
    vui_t vui;
    u32 frameCropLeftOffset;
    u32 frameCropRightOffset;
    u32 frameCropTopOffset;
    u32 frameCropBottomOffset;
} sps_s;

extern const u32 H264LevelIdc[];
extern const u32 H264MaxCPBS[];
extern const u32 H264MaxFS[];
extern const u32 H264SqrtMaxFS8[];
extern const u32 H264MaxMBPS[];
extern const u32 H264MaxBR[];

#define INVALID_LEVEL 0xFFFF

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/
void H264SeqParameterSetInit(sps_s * sps);
void H264SeqParameterSet(stream_s * stream, sps_s * sps, true_e nalHeader);
void H264SubsetSeqParameterSet(stream_s * stream, sps_s * sps);

void H264EndOfSequence(stream_s * stream, sps_s * sps);
void H264EndOfStream(stream_s * stream, sps_s * sps);

u32 H264GetLevelIndex(u32 levelIdc);

bool_e H264CheckLevel(sps_s * sps, i32 bitRate, i32 frameRateNum,
                      i32 frameRateDenom);

void H264SpsSetVuiTimigInfo(sps_s * sps, u32 timeScale, u32 numUnitsInTick);
void H264SpsSetVuiVideoInfo(sps_s * sps, u32 videoFullRange);
void H264SpsSetVuiAspectRatio(sps_s * sps, u32 sampleAspectRatioWidth,
                              u32 sampleAspectRatioHeight);
void H264SpsSetVuiPictStructPresentFlag(sps_s * sps, u32 flag);
void H264SpsSetVuiHrd(sps_s * sps, u32 present);
void H264SpsSetVuiHrdBitRate(sps_s * sps, u32 bitRate);
void H264SpsSetVuiHrdCpbSize(sps_s * sps, u32 cpbSize);
u32 H264SpsGetVuiHrdBitRate(sps_s * sps);
u32 H264SpsGetVuiHrdCpbSize(sps_s * sps);

#endif

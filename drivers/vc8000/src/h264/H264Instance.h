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
 *  Abstract  :   Encoder instance
 *
 ********************************************************************************
 */

#ifndef __H264_INSTANCE_H__
#define __H264_INSTANCE_H__

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "enccommon.h"
#include "encpreprocess.h"
#include "encasiccontroller.h"

#include "h264encapi.h"     /* Callback type from API is reused */

#include "H264NalUnit.h"
#include "H264SequenceParameterSet.h"
#include "H264PictureParameterSet.h"
#include "H264PictureBuffer.h"
#include "H264Slice.h"
#include "H264RateControl.h"
#include "H264Mad.h"

#ifdef VIDEOSTAB_ENABLED
#include "vidstabcommon.h"
#endif

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/
enum H264EncStatus
{
    H264ENCSTAT_INIT = 0xA1,
    H264ENCSTAT_START_STREAM,
    H264ENCSTAT_START_FRAME,
    H264ENCSTAT_ERROR
};

typedef struct
{
    u32 encStatus;
    u32 mbPerFrame;
    u32 mbPerRow;
    u32 mbPerCol;
    u32 interlaced;
    u32 frameCnt;
    u32 fillerNalSize;
    u32 testId;
    u32 numViews;
    u32 numRefBuffsLum;
    u32 numRefBuffsChr;
    u32 intraSliceMap[3];
    u32 idrHdr;
    stream_s stream;
    preProcess_s preProcess;
    sps_s seqParameterSet;
    pps_s picParameterSet;
    slice_s slice;
    mvc_s mvc;
    svc_s svc; /* info of SVCT */
    h264RateControl_s rateControl;
    madTable_s mad;
    asicData_s asic;
    i32 naluOffset;         /* Start offset for NAL unit size table */
    i32 numNalus;           /* Number of NAL units created */
    H264EncSliceReadyCallBackFunc sliceReadyCbFunc;
    u32 *pOutBuf;           /* User given stream output buffer */
    void *pAppData;         /* User given application specific data */
    const void *inst;
    picBuffer picBuffer;
    EWLHwConfig_t hwCfg;
#ifdef VIDEOSTAB_ENABLED
    HWStabData vsHwData;
    SwStbData vsSwData;
#endif
    i32 gdrEnabled;
    i32 gdrStart;
    i32 gdrDuration;
    i32 gdrCount;
    i32 gdrAverageMBRows;
    i32 gdrMBLeft;
    i32 gdrFirstIntraFrame;
    inputLineBuf_s inputLineBuf;
    i32 rfcBufOverflow;
    /* denoise filter */
    int dnfEnable;
    int dnfNoiseLevelY;
    int dnfNoiseYCRatio;
    int dnfTableLoaded;
    int dnfNoiseLevelLow;
    int dnfNoiseLevels[5];
    int dnfNoiseLevelC;
    u32 dnfNoiseLevelPred;
    int dnfNoiseLevelMax;
    int dnfNoiseMaxPred;
    int dnfNoiseMaxPrev;
    int dnfQpPrev;
    u32 dnfFrameNum;
} h264Instance_s;

#endif

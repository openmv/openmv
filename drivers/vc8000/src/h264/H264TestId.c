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
 *  Abstract : Encoder setup according to a test vector
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "H264TestId.h"

#include <stdio.h>
#include <stdlib.h>

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/
static void H264FrameQuantizationTest(h264Instance_s *inst);
static void H264SliceTest(h264Instance_s *inst);
static void H264StreamBufferLimitTest(h264Instance_s *inst);
static void H264MbQuantizationTest(h264Instance_s *inst);
static void H264FilterTest(h264Instance_s *inst);
static void H264UserDataTest(h264Instance_s *inst);
static void H264Intra16FavorTest(h264Instance_s *inst);
static void H264InterFavorTest(h264Instance_s *inst);
static void H264RgbInputMaskTest(h264Instance_s *inst);
static void H264MadTest(h264Instance_s *inst);
static void H264MvTest(h264Instance_s *inst);
static void H264DMVPenaltyTest(h264Instance_s *inst);
static void H264MaxOverfillMv(h264Instance_s *inst);
static void H264RoiTest(h264Instance_s *inst);
static void H264IntraAreaTest(h264Instance_s *inst);
static void H264CirTest(h264Instance_s *inst);
static void H264IntraSliceMapTest(h264Instance_s *inst);
static void H264SegmentTest(h264Instance_s *inst);
static void H264RefFrameTest(h264Instance_s *inst);
static void H264TemporalLayerTest(h264Instance_s *inst);
static void H264SegmentMapTest(h264Instance_s *inst);
static void H264PenaltyTest(h264Instance_s *inst);
static void H264DownscalingTest(h264Instance_s *inst);
static void H264TransOverflowTest(h264Instance_s *inst);
static void H264RfcBufOverflowTest(h264Instance_s *inst);
static void H264ToggleMvOut(h264Instance_s *inst);

/*------------------------------------------------------------------------------

    TestID defines a test configuration for the encoder. If the encoder control
    software is compiled with INTERNAL_TEST flag the test ID will force the 
    encoder operation according to the test vector. 

    TestID  Description
    0       No action, normal encoder operation
    1       Frame quantization test, adjust qp for every frame, qp = 0..51
    2       Slice test, adjust slice amount for each frame
    4       Stream buffer limit test, limit=500 (4kB) for first frame
    6       Quantization test, min and max QP values.
    7       Filter test, set disableDeblocking and filterOffsets A and B
    8       Segment test, set segment map and segment qps
    9       Reference frame test, all combinations of reference and refresh.
    10      Segment map test
    11      Temporal layer test, reference and refresh as with 3 layers
    12      User data test
    15      Intra16Favor test, set to maximum value
    16      Cropping test, set cropping values for every frame
    19      RGB input mask test, set all values
    20      MAD test, test all MAD QP change values
    21      InterFavor test, set to maximum value
    22      MV test, set cropping offsets so that max MVs are tested
    23      DMV penalty test, set to minimum/maximum values
    24      Max overfill MV
    26      ROI test
    27      Intra area test
    28      CIR test
    29      Intra slice map test
    31      Non-zero penalty test, don't use zero penalties
    34      Downscaling test

------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------

    H264ConfigureTestBeforeFrame

    Function configures the encoder instance before starting frame encoding

------------------------------------------------------------------------------*/
void H264ConfigureTestBeforeFrame(h264Instance_s * inst)
{
    ASSERT(inst);

    switch(inst->testId)
    {
        case 1: H264FrameQuantizationTest(inst);        break;
        case 2: H264SliceTest(inst);                    break;
        case 4: H264StreamBufferLimitTest(inst);        break;
        case 6: H264MbQuantizationTest(inst);           break;
        case 7: H264FilterTest(inst);                   break;
        case 8: H264SegmentTest(inst);                  break;
        case 9: H264RefFrameTest(inst);                 break;
        case 10: H264SegmentMapTest(inst);              break;
        case 11: H264TemporalLayerTest(inst);           break;
        case 12: H264UserDataTest(inst);                break;
        case 16:
            /* do not change vertical offset here in input line buffer mode, because test bench don't know this change */
            if ((!inst->inputLineBuf.inputLineBufEn) ||
                (!inst->inputLineBuf.inputLineBufLoopBackEn))
                H264CroppingTest(inst);
            break;
        case 19: H264RgbInputMaskTest(inst);            break;
        case 20: H264MadTest(inst);                     break;
        case 22: H264MvTest(inst);                      break;
        case 24: H264MaxOverfillMv(inst);               break;
        case 26: H264RoiTest(inst);                     break;
        case 27: H264IntraAreaTest(inst);               break;
        case 28: H264CirTest(inst);                     break;
        case 29: H264IntraSliceMapTest(inst);           break;
        case TID_DOWNSCALING: 
            H264DownscalingTest(inst);
            break;
        case TID_RFC_OVERFLOW: 
            H264RfcBufOverflowTest(inst);
            break;
        case TID_MVOUT_TOGGLE:
            H264ToggleMvOut(inst);
            break;
        default: break;
    }
}

/*------------------------------------------------------------------------------

    H264ConfigureTestPenalties

    Function configures the encoder instance before starting frame encoding

------------------------------------------------------------------------------*/
void H264ConfigureTestPenalties(h264Instance_s * inst)
{
    ASSERT(inst);

    switch(inst->testId)
    {
        case 15: H264Intra16FavorTest(inst);            break;
        case 21: H264InterFavorTest(inst);              break;
        case 23: H264DMVPenaltyTest(inst);              break;
        case TID_NONZERO_PANEALTY: 
            H264PenaltyTest(inst);
            break;
        case TID_TRANS_OVERFLOW:
            H264TransOverflowTest(inst);
            break;
        default: 
            break;
    }
}

/*------------------------------------------------------------------------------
  H264QuantizationTest
------------------------------------------------------------------------------*/
void H264FrameQuantizationTest(h264Instance_s *inst)
{
    i32 vopNum = inst->frameCnt;

    /* Inter frame qp start zero */
    inst->rateControl.qpHdr = MIN(inst->rateControl.qpMax,
                                  MAX(inst->rateControl.qpMin, ((vopNum-1)%52)<< QP_FRACTIONAL_BITS));

    printf("H264FrameQuantTest# qpHdr %d\n", inst->rateControl.qpHdr >> QP_FRACTIONAL_BITS);
}

/*------------------------------------------------------------------------------
  H264SliceTest
------------------------------------------------------------------------------*/
void H264SliceTest(h264Instance_s *inst)
{
    i32 vopNum = inst->frameCnt;

    inst->slice.sliceSize = (inst->mbPerRow*vopNum)%inst->mbPerFrame;

    printf("H264SliceTest# sliceSize %d\n", inst->slice.sliceSize);
}

/*------------------------------------------------------------------------------
  H264StreamBufferLimitTest
------------------------------------------------------------------------------*/
void H264StreamBufferLimitTest(h264Instance_s *inst)
{
    static u32 firstFrame = 1;

    if (!firstFrame)
        return;

    firstFrame = 0;
    inst->asic.regs.outputStrmSize = 4000;

    printf("H264StreamBufferLimitTest# streamBufferLimit %d bytes\n",
            inst->asic.regs.outputStrmSize);
}

/*------------------------------------------------------------------------------
  H264QuantizationTest
  NOTE: ASIC resolution for wordCntTarget and wordError is value/4
------------------------------------------------------------------------------*/
void H264MbQuantizationTest(h264Instance_s *inst)
{
    i32 vopNum = inst->frameCnt;
    h264RateControl_s *rc = &inst->rateControl;

    rc->qpMin = (MIN(51, vopNum/4))<< QP_FRACTIONAL_BITS;
    rc->qpMax = (MAX(0, 51 - vopNum/4))<< QP_FRACTIONAL_BITS;
    rc->qpMax =( MAX(rc->qpMax, rc->qpMin));

    rc->qpLastCoded = rc->qpTarget = rc->qpHdr =
                MIN(rc->qpMax, MAX(rc->qpMin, 26 << QP_FRACTIONAL_BITS));

    printf("H264MbQuantTest# min %d  max %d  QP %d\n",
            rc->qpMin >> QP_FRACTIONAL_BITS, rc->qpMax >> QP_FRACTIONAL_BITS, rc->qpHdr >> QP_FRACTIONAL_BITS);
}

/*------------------------------------------------------------------------------
  H264FilterTest
------------------------------------------------------------------------------*/
void H264FilterTest(h264Instance_s *inst)
{
    i32 vopNum = inst->frameCnt;
    slice_s *slice = &inst->slice; 

    slice->disableDeblocking = (vopNum/2)%3;
    if (vopNum == 0) {
        slice->filterOffsetA = -12;
        slice->filterOffsetB = 12;
    } else if (vopNum < 77) {
        if (vopNum%6 == 0) {
            slice->filterOffsetA += 2;
            slice->filterOffsetB -= 2;
        }
    } else if (vopNum == 78) {
        slice->filterOffsetA = -12;
        slice->filterOffsetB = -12;
    } else if (vopNum < 155) {
        if (vopNum%6 == 0) {
            slice->filterOffsetA += 2;
            slice->filterOffsetB += 2;
        }
    }

    printf("H264FilterTest# disableDeblock = %d, filterOffA = %i filterOffB = %i\n",
             slice->disableDeblocking, slice->filterOffsetA,
             slice->filterOffsetB);

}

/*------------------------------------------------------------------------------
  H264SegmentTest
------------------------------------------------------------------------------*/
void H264SegmentTest(h264Instance_s *inst)
{
    u32 frame = (u32)inst->frameCnt;
    u32 *map = inst->asic.segmentMap.virtualAddress;
    u32 mbPerFrame = (inst->mbPerFrame+7)/8*8;  /* Rounded upwards to 8 */
    u32 i, j;
    u32 mask;
    i32 qpSgm[4];

    inst->asic.regs.segmentEnable = 1;
    inst->asic.regs.segmentMapUpdate = 1;
    if (frame < 2) {
        for (i = 0; i < mbPerFrame/8/4; i++) {
            map[i*4+0] = 0x00000000;
            map[i*4+1] = 0x11111111;
            map[i*4+2] = 0x22222222;
            map[i*4+3] = 0x33333333;
        }
    } else {
        for (i = 0; i < mbPerFrame/8; i++) {
            mask = 0;
            for (j = 0; j < 8; j++)
                mask |= ((j + (frame-2)/2 + j*frame/3)%4) << (28-j*4);
            map[i] = mask;
        }
    }

    for (i = 0; i < 4; i++) {
        qpSgm[i] = (32 + i + frame/2 + frame*i)%52;
        qpSgm[i] = MAX(inst->rateControl.qpMin,
                   MIN(inst->rateControl.qpMax, qpSgm[i]));
    }
    inst->rateControl.qpHdr = qpSgm[0];
    inst->rateControl.mbQpAdjustment[0] = MAX(-8, MIN(7, qpSgm[1] - qpSgm[0]));
    inst->rateControl.mbQpAdjustment[1] = qpSgm[2] - qpSgm[0];
    inst->rateControl.mbQpAdjustment[2] = qpSgm[3] - qpSgm[0];

    printf("H264SegmentTest# enable=%d update=%d map=0x%08x%08x%08x%08x\n",
             inst->asic.regs.segmentEnable, inst->asic.regs.segmentMapUpdate,
             map[0], map[1], map[2], map[3]);
    printf("H264SegmentTest# qp=%d,%d,%d,%d\n",
            qpSgm[0], qpSgm[1], qpSgm[2], qpSgm[3]);
}

/*------------------------------------------------------------------------------
  H264RefFrameTest
------------------------------------------------------------------------------*/
void H264RefFrameTest(h264Instance_s *inst)
{
    i32 pic = inst->frameCnt;
    picBuffer *picBuffer = &inst->picBuffer;

    /* Only adjust for p-frames */
    if (picBuffer->cur_pic->p_frame) {
        picBuffer->cur_pic->ipf = pic & 0x1 ? 1 : 0;
        if (inst->seqParameterSet.numRefFrames > 1)
            picBuffer->cur_pic->grf = pic & 0x2 ? 1 : 0;
        picBuffer->refPicList[0].search = pic & 0x8 ? 1 : 0;
        if (inst->seqParameterSet.numRefFrames > 1)
            picBuffer->refPicList[1].search = pic & 0x10 ? 1 : 0;
    }

    printf("H264RefFrameTest#\n");
}

/*------------------------------------------------------------------------------
  H264TemporalLayerTest
------------------------------------------------------------------------------*/
void H264TemporalLayerTest(h264Instance_s *inst)
{
    i32 pic = inst->frameCnt;
    picBuffer *picBuffer = &inst->picBuffer;

    /* Four temporal layers, base layer (LTR) every 8th frame. */
    if (inst->seqParameterSet.numRefFrames > 1)
        picBuffer->cur_pic->grf = pic & 0x7 ? 0 : 1;
    if (picBuffer->cur_pic->p_frame) {
        /* Odd frames don't update ipf */
        picBuffer->cur_pic->ipf = pic & 0x1 ? 0 : 1;
        /* Frames 1,2,3 & 4,5,6 reference prev */
        picBuffer->refPicList[0].search = pic & 0x3 ? 1 : 0;
        /* Every fourth frame (layers 0&1) reference LTR */
        if (inst->seqParameterSet.numRefFrames > 1)
            picBuffer->refPicList[1].search = pic & 0x3 ? 0 : 1;
    }

    printf("H264TemporalLayer#\n");
}

/*------------------------------------------------------------------------------
  H264SegmentMapTest
------------------------------------------------------------------------------*/
void H264SegmentMapTest(h264Instance_s *inst)
{
    u32 frame = (u32)inst->frameCnt;
    u32 *map = inst->asic.segmentMap.virtualAddress;
    u32 mbPerFrame = (inst->mbPerFrame+7)/8*8;  /* Rounded upwards to 8 */
    u32 i, j;
    u32 mask;
    i32 qpSgm[4];

    inst->asic.regs.segmentEnable = 1;
    inst->asic.regs.segmentMapUpdate = 1;
    if (frame < 2) {
        for (i = 0; i < mbPerFrame/8; i++)
            map[i] = 0x01020120;
    } else {
        for (i = 0; i < mbPerFrame/8; i++) {
            mask = 0;
            for (j = 0; j < 8; j++)
                mask |= ((j + frame)%4) << (28-j*4);
            map[i] = mask;
        }
    }

    if (frame < 2) {
        qpSgm[0] = inst->rateControl.qpMax;
        qpSgm[1] = qpSgm[2] = qpSgm[3] = inst->rateControl.qpMin;
    } else {
        for (i = 0; i < 4; i++) {
            qpSgm[i] = (32 + i*frame)%52;
            qpSgm[i] = MAX(inst->rateControl.qpMin,
                       MIN(inst->rateControl.qpMax, qpSgm[i]));
        }
    }
    inst->rateControl.qpHdr = qpSgm[0];
    inst->rateControl.mbQpAdjustment[0] = MAX(-8, MIN(7, qpSgm[1] - qpSgm[0]));
    inst->rateControl.mbQpAdjustment[1] = qpSgm[2] - qpSgm[0];
    inst->rateControl.mbQpAdjustment[2] = qpSgm[3] - qpSgm[0];

    printf("H264SegmentMapTest# enable=%d update=%d map=0x%08x%08x%08x%08x\n",
             inst->asic.regs.segmentEnable, inst->asic.regs.segmentMapUpdate,
             map[0], map[1], map[2], map[3]);
    printf("H264SegmentMapTest# qp=%d,%d,%d,%d\n",
            qpSgm[0], qpSgm[1], qpSgm[2], qpSgm[3]);
}


/*------------------------------------------------------------------------------
  H264UserDataTest
------------------------------------------------------------------------------*/
void H264UserDataTest(h264Instance_s *inst)
{
    static u8 *userDataBuf = NULL;
    i32 userDataLength = 16 + ((inst->frameCnt*11) % 2000);
    i32 i;

    /* Allocate a buffer for user data, encoder reads data from this buffer
     * and writes it to the stream. TODO: This is never freed. */
    if (!userDataBuf)
        userDataBuf = (u8*)malloc(2048);

    if (!userDataBuf)
        return;

    for(i = 0; i < userDataLength; i++)
    {
        /* Fill user data buffer with ASCII symbols from 48 to 125 */
        userDataBuf[i] = 48 + i % 78;
    }

    /* Enable user data insertion */
    inst->rateControl.sei.userDataEnabled = ENCHW_YES;
    inst->rateControl.sei.pUserData = userDataBuf;
    inst->rateControl.sei.userDataSize = userDataLength;

    printf("H264UserDataTest# userDataSize %d\n", userDataLength);
}

/*------------------------------------------------------------------------------
  H264Intra16FavorTest
------------------------------------------------------------------------------*/
void H264Intra16FavorTest(h264Instance_s *inst)
{
    i32 s;

    /* Force intra16 favor to maximum value */
    for (s = 0; s < 4; s++)
        inst->asic.regs.pen[s][ASIC_PENALTY_I16FAVOR] = 0xFFFF;

    printf("H264Intra16FavorTest# intra16Favor %d\n", 
            inst->asic.regs.pen[0][ASIC_PENALTY_I16FAVOR]);
}

/*------------------------------------------------------------------------------
  H264InterFavorTest
------------------------------------------------------------------------------*/
void H264InterFavorTest(h264Instance_s *inst)
{
    i32 s;

    /* Force combinations of inter favor and skip penalty values */

    for (s = 0; s < 4; s++) {
        if ((inst->frameCnt % 3) == 0) {
            inst->asic.regs.pen[s][ASIC_PENALTY_INTER_FAVOR] = 0x7FFF;
        } else if ((inst->frameCnt % 3) == 1) {
            inst->asic.regs.pen[s][ASIC_PENALTY_SKIP] = 0;
            inst->asic.regs.pen[s][ASIC_PENALTY_INTER_FAVOR] = 0x7FFF;
        } else {
            inst->asic.regs.pen[s][ASIC_PENALTY_SKIP] = 0;
        }
    }

    printf("H264InterFavorTest# interFavor %d skipPenalty %d\n",
            inst->asic.regs.pen[0][ASIC_PENALTY_INTER_FAVOR],
            inst->asic.regs.pen[0][ASIC_PENALTY_SKIP]);
}

/*------------------------------------------------------------------------------
  H264CroppingTest
------------------------------------------------------------------------------*/
void H264CroppingTest(h264Instance_s *inst)
{
    inst->preProcess.horOffsetSrc = inst->frameCnt % 16;
    if (EncPreProcessCheck(&inst->preProcess) == ENCHW_NOK)
        inst->preProcess.horOffsetSrc = 0;
    inst->preProcess.verOffsetSrc = inst->frameCnt / 2;
    if (EncPreProcessCheck(&inst->preProcess) == ENCHW_NOK)
        inst->preProcess.verOffsetSrc = 0;

    printf("H264CroppingTest# horOffsetSrc %d  verOffsetSrc %d\n",
            inst->preProcess.horOffsetSrc, inst->preProcess.verOffsetSrc);
}

/*------------------------------------------------------------------------------
  H264RgbInputMaskTest
------------------------------------------------------------------------------*/
void H264RgbInputMaskTest(h264Instance_s *inst)
{
    u32 frameNum = (u32)inst->frameCnt;
    static u32 rMsb = 0;
    static u32 gMsb = 0;
    static u32 bMsb = 0;
    static u32 lsMask = 0;  /* Lowest possible mask position */
    static u32 msMask = 0;  /* Highest possible mask position */

    /* First frame normal
     * 1..29 step rMaskMsb values
     * 30..58 step gMaskMsb values
     * 59..87 step bMaskMsb values */
    if (frameNum == 0) {
        rMsb = inst->asic.regs.rMaskMsb;
        gMsb = inst->asic.regs.gMaskMsb;
        bMsb = inst->asic.regs.bMaskMsb;
        lsMask = MIN(rMsb, gMsb);
        lsMask = MIN(bMsb, lsMask);
        msMask = MAX(rMsb, gMsb);
        msMask = MAX(bMsb, msMask);
        if (msMask < 16)
            msMask = 15-2;    /* 16bit RGB, 13 mask positions: 3..15  */
        else
            msMask = 31-2;    /* 32bit RGB, 29 mask positions: 3..31 */
    } else if (frameNum <= msMask) {
        inst->asic.regs.rMaskMsb = MAX(frameNum+2, lsMask);
        inst->asic.regs.gMaskMsb = gMsb;
        inst->asic.regs.bMaskMsb = bMsb;
    } else if (frameNum <= msMask*2) {
        inst->asic.regs.rMaskMsb = rMsb;
        inst->asic.regs.gMaskMsb = MAX(frameNum-msMask+2, lsMask);
        if (inst->asic.regs.inputImageFormat == 4)  /* RGB 565 special case */
            inst->asic.regs.gMaskMsb = MAX(frameNum-msMask+2, lsMask+1);
        inst->asic.regs.bMaskMsb = bMsb;
    } else if (frameNum <= msMask*3) {
        inst->asic.regs.rMaskMsb = rMsb;
        inst->asic.regs.gMaskMsb = gMsb;
        inst->asic.regs.bMaskMsb = MAX(frameNum-msMask*2+2, lsMask);
    } else {
        inst->asic.regs.rMaskMsb = rMsb;
        inst->asic.regs.gMaskMsb = gMsb;
        inst->asic.regs.bMaskMsb = bMsb;
    }

    printf("H264RgbInputMaskTest#  %d %d %d\n", inst->asic.regs.rMaskMsb,
            inst->asic.regs.gMaskMsb, inst->asic.regs.bMaskMsb);
}

/*------------------------------------------------------------------------------
  H264MadTest
------------------------------------------------------------------------------*/
void H264MadTest(h264Instance_s *inst)
{
    u32 frameNum = (u32)inst->frameCnt;

    /* All values in range [-8,7] */
    inst->rateControl.mbQpAdjustment[0] = -8 + (frameNum%16);
    inst->rateControl.mbQpAdjustment[1] = -127 + (frameNum%254);
    inst->rateControl.mbQpAdjustment[2] = 127 - (frameNum%254);
    /* Step 256, range [0,63*256] */
    inst->mad.threshold[0] = 256 * ((frameNum+1)%64);
    inst->mad.threshold[1] = 256 * ((frameNum/2)%64);
    inst->mad.threshold[2] = 256 * ((frameNum/3)%64);

    printf("H264MadTest#  Thresholds: %d,%d,%d  QpDeltas: %d,%d,%d\n",
            inst->asic.regs.madThreshold[0],
            inst->asic.regs.madThreshold[1],
            inst->asic.regs.madThreshold[2],
            inst->rateControl.mbQpAdjustment[0],
            inst->rateControl.mbQpAdjustment[1],
            inst->rateControl.mbQpAdjustment[2]);
}

/*------------------------------------------------------------------------------
  H264MvTest
------------------------------------------------------------------------------*/
void H264MvTest(h264Instance_s *inst)
{
    u32 frame = (u32)inst->frameCnt;

    /* Set cropping offsets according to max MV length, decrement by frame
     * x = 32, 160, 32, 159, 32, 158, ..
     * y = 48, 80, 48, 79, 48, 78, .. */
    inst->preProcess.horOffsetSrc = 32 + (frame%2)*128 - (frame%2)*(frame/2);
    if (EncPreProcessCheck(&inst->preProcess) == ENCHW_NOK)
        inst->preProcess.horOffsetSrc = 0;
    inst->preProcess.verOffsetSrc = 48 + (frame%2)*32 - (frame%2)*(frame/2);
    if (EncPreProcessCheck(&inst->preProcess) == ENCHW_NOK)
        inst->preProcess.verOffsetSrc = 0;

    printf("H264MvTest# horOffsetSrc %d  verOffsetSrc %d\n",
            inst->preProcess.horOffsetSrc, inst->preProcess.verOffsetSrc);
}

/*------------------------------------------------------------------------------
  H264DMVPenaltyTest
------------------------------------------------------------------------------*/
void H264DMVPenaltyTest(h264Instance_s *inst)
{
    u32 frame = (u32)inst->frameCnt;
    i32 s;

    /* Set DMV penalty values to maximum and minimum */
    for (s = 0; s < 4; s++) {
        inst->asic.regs.pen[s][ASIC_PENALTY_DMV_4P] = frame%2 ? 127-frame/2 : frame/2;
        inst->asic.regs.pen[s][ASIC_PENALTY_DMV_1P] = frame%2 ? 127-frame/2 : frame/2;
        inst->asic.regs.pen[s][ASIC_PENALTY_DMV_QP] = frame%2 ? 127-frame/2 : frame/2;
    }

    printf("H264DMVPenaltyTest# penalty4p %d  penalty1p %d  penaltyQp %d\n",
            inst->asic.regs.pen[0][ASIC_PENALTY_DMV_4P],
            inst->asic.regs.pen[0][ASIC_PENALTY_DMV_1P],
            inst->asic.regs.pen[0][ASIC_PENALTY_DMV_QP]);
}

/*------------------------------------------------------------------------------
  H264MaxOverfillMv
------------------------------------------------------------------------------*/
void H264MaxOverfillMv(h264Instance_s *inst)
{
    u32 frame = (u32)inst->frameCnt;

    /* Set cropping offsets according to max MV length.
     * In test cases the picture is selected so that this will
     * cause maximum horizontal MV to point into overfilled area. */
    inst->preProcess.horOffsetSrc = 32 + (frame%2)*128;
    if (EncPreProcessCheck(&inst->preProcess) == ENCHW_NOK)
        inst->preProcess.horOffsetSrc = 0;

    inst->preProcess.verOffsetSrc = 176;
    if (EncPreProcessCheck(&inst->preProcess) == ENCHW_NOK)
        inst->preProcess.verOffsetSrc = 0;

    printf("H264MaxOverfillMv# horOffsetSrc %d  verOffsetSrc %d\n",
            inst->preProcess.horOffsetSrc, inst->preProcess.verOffsetSrc);
}

/*------------------------------------------------------------------------------
  H264RoiTest
------------------------------------------------------------------------------*/
void H264RoiTest(h264Instance_s *inst)
{
    regValues_s *regs = &inst->asic.regs;
    u32 frame = (u32)inst->frameCnt;
    u32 mbPerRow = inst->mbPerRow;
    u32 mbPerCol = inst->mbPerCol;
    u32 frames = MIN(mbPerRow, mbPerCol);
    u32 loop = frames*3;

    /* Loop after this many encoded frames */
    frame = frame % loop;

    regs->roi1DeltaQp = (frame % 15) + 1;
    regs->roi2DeltaQp = 15 - (frame % 15);
    regs->roiUpdate = 1;

    /* Set two ROI areas according to frame dimensions. */
    if (frame < frames)
    {
        /* ROI1 in top-left corner, ROI2 in bottom-right corner */
        regs->roi1Left = regs->roi1Top = 0;
        regs->roi1Right = regs->roi1Bottom = frame;
        regs->roi2Left = mbPerRow - 1 - frame;
        regs->roi2Top = mbPerCol - 1 - frame;
        regs->roi2Right = mbPerRow - 1;
        regs->roi2Bottom = mbPerCol - 1;
    }
    else if (frame < frames*2)
    {
        /* ROI1 gets smaller towards top-right corner,
         * ROI2 towards bottom-left corner */
        frame -= frames;
        regs->roi1Left = frame;
        regs->roi1Top = 0;
        regs->roi1Right = mbPerRow - 1;
        regs->roi1Bottom = mbPerCol - 1 - frame;
        regs->roi2Left = 0;
        regs->roi2Top = frame;
        regs->roi2Right = mbPerRow - 1 - frame;
        regs->roi2Bottom = mbPerCol - 1;
    }
    else if (frame < frames*3)
    {
        /* 1x1/2x2 ROIs moving diagonal across frame */
        frame -= frames*2;
        regs->roi1Left = frame - frame%2;
        regs->roi1Right = frame;
        regs->roi1Top = frame - frame%2;
        regs->roi1Bottom = frame;
        regs->roi2Left = frame - frame%2;
        regs->roi2Right = frame;
        regs->roi2Top = mbPerCol - 1 - frame;
        regs->roi2Bottom = mbPerCol - 1 - frame + frame%2;
    }

    printf("H264RoiTest# ROI1:%d x%dy%d-x%dy%d  ROI2:%d x%dy%d-x%dy%d\n",
            regs->roi1DeltaQp, regs->roi1Left, regs->roi1Top,
            regs->roi1Right, regs->roi1Bottom,
            regs->roi2DeltaQp, regs->roi2Left, regs->roi2Top,
            regs->roi2Right, regs->roi2Bottom);
}

/*------------------------------------------------------------------------------
  H264IntraAreaTest
------------------------------------------------------------------------------*/
void H264IntraAreaTest(h264Instance_s *inst)
{
    regValues_s *regs = &inst->asic.regs;
    u32 frame = (u32)inst->frameCnt;
    u32 mbPerRow = inst->mbPerRow;
    u32 mbPerCol = inst->mbPerCol;
    u32 frames = MIN(mbPerRow, mbPerCol);
    u32 loop = frames*3;

    /* Loop after this many encoded frames */
    frame = frame % loop;

    if (frame < frames)
    {
        /* Intra area in top-left corner, gets bigger every frame */
        regs->intraAreaLeft = regs->intraAreaTop = 0;
        regs->intraAreaRight = regs->intraAreaBottom = frame;
    }
    else if (frame < frames*2)
    {
        /* Intra area gets smaller towards top-right corner */
        frame -= frames;
        regs->intraAreaLeft = frame;
        regs->intraAreaTop = 0;
        regs->intraAreaRight = mbPerRow - 1;
        regs->intraAreaBottom = mbPerCol - 1 - frame;
    }
    else if (frame < frames*3)
    {
        /* 1x1/2x2 Intra area moving diagonal across frame */
        frame -= frames*2;
        regs->intraAreaLeft = frame - frame%2;
        regs->intraAreaRight = frame;
        regs->intraAreaTop = frame - frame%2;
        regs->intraAreaBottom = frame;
    }

    printf("H264IntraAreaTest# x%dy%d-x%dy%d\n",
            regs->intraAreaLeft, regs->intraAreaTop,
            regs->intraAreaRight, regs->intraAreaBottom);
}

/*------------------------------------------------------------------------------
  H264CirTest
------------------------------------------------------------------------------*/
void H264CirTest(h264Instance_s *inst)
{
    regValues_s *regs = &inst->asic.regs;
    u32 frame = (u32)inst->frameCnt;
    u32 mbPerRow = inst->mbPerRow;
    u32 mbPerFrame = inst->mbPerFrame;
    u32 loop = inst->mbPerFrame+6;

    /* Loop after this many encoded frames */
    frame = frame % loop;

    switch (frame)
    {
        case 0:
        case 1:
            regs->cirStart = 0;
            regs->cirInterval = 1;
            break;
        case 2:
            regs->cirStart = 0;
            regs->cirInterval = 2;
            break;
        case 3:
            regs->cirStart = 0;
            regs->cirInterval = 3;
            break;
        case 4:
            regs->cirStart = 0;
            regs->cirInterval = mbPerRow;
            break;
        case 5:
            regs->cirStart = 0;
            regs->cirInterval = mbPerRow+1;
            break;
        case 6:
            regs->cirStart = 0;
            regs->cirInterval = mbPerFrame-1;
            break;
        case 7:
            regs->cirStart = mbPerFrame-1;
            regs->cirInterval = 1;
            break;
        default:
            regs->cirStart = frame-7;
            regs->cirInterval = (mbPerFrame-frame)%(mbPerRow*2);
            break;
    }

    printf("H264CirTest# start:%d interval:%d\n",
            regs->cirStart, regs->cirInterval);
}

/*------------------------------------------------------------------------------
  H264IntraSliceMapTest
------------------------------------------------------------------------------*/
void H264IntraSliceMapTest(h264Instance_s *inst)
{
    u32 frame = (u32)inst->frameCnt;
    u32 mbPerCol = inst->mbPerCol;

    frame = frame % (mbPerCol*10);

    if (frame <= 1)
    {
        inst->slice.sliceSize = inst->mbPerRow;
        inst->intraSliceMap[0] = 0x55555555;
        inst->intraSliceMap[1] = 0x55555555;
        inst->intraSliceMap[2] = 0x55555555;
    }
    else if (frame < mbPerCol+1)
    {
        inst->slice.sliceSize = inst->mbPerRow * (frame-1);
        inst->intraSliceMap[0] = 0xAAAAAAAA;
        inst->intraSliceMap[1] = 0xAAAAAAAA;
        inst->intraSliceMap[2] = 0xAAAAAAAA;
    }
    else
    {
        inst->slice.sliceSize = inst->mbPerRow * (frame%mbPerCol);
        inst->intraSliceMap[0] += frame;
        inst->intraSliceMap[1] += frame;
        inst->intraSliceMap[2] += frame;
    }

    printf("H264IntraSliceMapTest# "
           "sliceSize: %d  map1: 0x%x  map2: 0x%x  map3: 0x%x \n",
            inst->slice.sliceSize, inst->intraSliceMap[0],
            inst->intraSliceMap[1], inst->intraSliceMap[2]);
}

/*------------------------------------------------------------------------------
  H264PenaltyTest
------------------------------------------------------------------------------*/
void H264PenaltyTest(h264Instance_s *inst)
{
    i32 s;

    inst->asic.regs.inputReadChunk = 1;

    /* Set non-zero values */
    for (s = 0; s < 4; s++) {
        inst->asic.regs.pen[s][ASIC_PENALTY_SPLIT16x8] = 1;
        inst->asic.regs.pen[s][ASIC_PENALTY_SPLIT8x8] = 2;
        inst->asic.regs.pen[s][ASIC_PENALTY_SPLIT8x4] = 3;
        inst->asic.regs.pen[s][ASIC_PENALTY_SPLIT4x4] = 4;
        inst->asic.regs.pen[s][ASIC_PENALTY_SPLIT_ZERO] = 5;
    }

    printf("H264PenaltyTest# splitPenalty %d %d %d %d %d\n",
            inst->asic.regs.pen[0][ASIC_PENALTY_SPLIT16x8],
            inst->asic.regs.pen[0][ASIC_PENALTY_SPLIT8x8],
            inst->asic.regs.pen[0][ASIC_PENALTY_SPLIT8x4],
            inst->asic.regs.pen[0][ASIC_PENALTY_SPLIT4x4],
            inst->asic.regs.pen[0][ASIC_PENALTY_SPLIT_ZERO]);
}

/*------------------------------------------------------------------------------
  H264Downscaling
------------------------------------------------------------------------------*/
void H264DownscalingTest(h264Instance_s *inst)
{
    u32 frame = (u32)inst->frameCnt;
    u32 xy = MIN(inst->preProcess.lumWidth, inst->preProcess.lumHeight);

    if (!frame) return;

    if (frame <= xy/2) {
        inst->preProcess.scaledWidth = inst->preProcess.lumWidth - (frame/2)*4;
        inst->preProcess.scaledHeight = inst->preProcess.lumHeight - frame*2;
    } else {
        u32 i, x, y;
        i = frame - xy/2;
        x = i%(inst->preProcess.lumWidth/8);
        y = i/(inst->preProcess.lumWidth/8);
        inst->preProcess.scaledWidth = inst->preProcess.lumWidth - x*8;
        inst->preProcess.scaledHeight = inst->preProcess.lumHeight - y*8;
    }

    if (!inst->preProcess.scaledWidth)
        inst->preProcess.scaledWidth = inst->preProcess.lumWidth - 4;

    if (!inst->preProcess.scaledHeight)
        inst->preProcess.scaledHeight = inst->preProcess.lumHeight - 4;

    printf("H264DownscalingTest# %dx%d => %dx%d\n",
             inst->preProcess.lumWidth, inst->preProcess.lumHeight,
             inst->preProcess.scaledWidth, inst->preProcess.scaledHeight);
}

/*------------------------------------------------------------------------------
  H264InputLineBufDepthTest
------------------------------------------------------------------------------*/
void H264InputLineBufDepthTest(h264Instance_s *inst)
{
    u32 maxDepth = inst->mbPerCol;
    /*  limitation: ((line buffer height/16) * (frame width/16)) % 4 == 0 */
    i32 tail = inst->mbPerRow & 3;
    i32 align = 1;
    if (tail == 2) align = 2;
    else if (tail&1) align = 4;

    if ((!inst->inputLineBuf.inputLineBufLoopBackEn) &&
        (!inst->inputLineBuf.inputLineBufHwModeEn)) return;

    if (inst->inputLineBuf.inputLineBufLoopBackEn)
    {
      const u32 maxLineBufSize = 0x100000;
      u32 stride = inst->preProcess.lumWidthSrc * (inst->preProcess.interlacedFrame ? 2 : 1);
      u32 mbLineSize = stride*16*H264EncGetBitsPerPixel(inst->preProcess.inputFormat)/8;
      maxDepth = MIN(maxDepth, maxLineBufSize/mbLineSize/2);
    }
    maxDepth = maxDepth / align;

    /* frame 0 use user config */
    if (inst->frameCnt > 0)
    {
        u32 depth = inst->inputLineBuf.inputLineBufDepth;
        u32 nFrm = inst->frameCnt - 1;
        inst->inputLineBuf.inputLineBufDepth = (nFrm%maxDepth) + 1;
        inst->inputLineBuf.inputLineBufDepth *= align;
    
        printf("inputLineBufTest# depth changed from %d to %d for next frame \n",
          depth, inst->inputLineBuf.inputLineBufDepth);
    }
}

/*------------------------------------------------------------------------------
  H264RfcBufOverflowTest
------------------------------------------------------------------------------*/
void H264RfcBufOverflowTest(h264Instance_s *inst)
{
    u32 lumLimit[16] = {30, 70, 0, 40, 55, 75, 55, 0,  80, 70, 65, 60, 50, 45, 35, 30};
    u32 chrLimit[16] = {70, 25, 0, 30, 45, 65, 0,  45, 70, 60, 55, 50, 40, 35, 25, 20};
    u32 idx = inst->frameCnt & 0xf;

    if (inst->asic.regs.refLumCompress)
        inst->asic.regs.rfcLumBufLimit = inst->mbPerCol*inst->mbPerRow*256*lumLimit[idx]/100;

    if (inst->asic.regs.refChrCompress)
        inst->asic.regs.rfcChrBufLimit = inst->mbPerCol*inst->mbPerRow*128*chrLimit[idx]/100;

    inst->asic.regs.rfcLumBufLimit /= 8;
    inst->asic.regs.rfcChrBufLimit /= 8;

    printf("H264RfcBufOverflowTest# lumLimit = %d%%; chrLimit = %d%%\n", lumLimit[idx], chrLimit[idx]);
}

/*------------------------------------------------------------------------------
  H264TransOverflowTest
------------------------------------------------------------------------------*/
void H264TransOverflowTest(h264Instance_s *inst)
{
    i32 s;

    /* Set large InterFavor register to make more InterMB. 
     * it makes the overflow happens. */

    for (s = 0; s < 4; s++) {
        inst->asic.regs.pen[s][ASIC_PENALTY_INTER_FAVOR] = 32200;
    }

    printf("H264TransOverflowTest# interFavor %d\n",
            inst->asic.regs.pen[0][ASIC_PENALTY_INTER_FAVOR]);
}

/*------------------------------------------------------------------------------
  H264ToggleMvOut
------------------------------------------------------------------------------*/
void H264ToggleMvOut(h264Instance_s *inst)
{
    u32 frame = (u32)inst->frameCnt;
    u32 pattern[4] = {0, 1, 1, 0};

    /* set swreg HEncMvWrite as 0-1-1-0 patter according to frameCnt%4. 
     * so when recon output pattern is 1-0-1-0, full combination will be
     * tested. */
    inst->asic.regs.mvOutEnable = pattern[frame%4];

    printf("H264ToggleMvOut# mvWrite=%d\n", inst->asic.regs.mvOutEnable);
}

#if 0
/*------------------------------------------------------------------------------
  MbPerInterruptTest
------------------------------------------------------------------------------*/
void MbPerInterruptTest(trace_s *trace)
{
    if (trace->testId != 3) {
            return;
    }

    trace->control.mbPerInterrupt = trace->vopNum;
}

/*------------------------------------------------------------------------------
H264FilterTest
------------------------------------------------------------------------------*/
void H264FilterTest(trace_s *trace, slice_s *slice)
{
    i32 vopNum = trace->vopNum;

    if (trace->testId != 7) {
            return;
    }

    slice->disableDeblocking = (vopNum/2)%3;
    if (vopNum == 0) {
            slice->filterOffsetA = -12;
            slice->filterOffsetB = 12;
    } else if (vopNum < 77) {
            if (vopNum%6 == 0) {
                    slice->filterOffsetA += 2;
                    slice->filterOffsetB -= 2;
            }
    } else if (vopNum == 78) {
            slice->filterOffsetA = -12;
            slice->filterOffsetB = -12;
    } else if (vopNum < 155) {
            if (vopNum%6 == 0) {
                    slice->filterOffsetA += 2;
                    slice->filterOffsetB += 2;
            }
    }
}

/*------------------------------------------------------------------------------
H264SliceQuantTest()  Change sliceQP from min->max->min.
------------------------------------------------------------------------------*/
void H264SliceQuantTest(trace_s *trace, slice_s *slice, mb_s *mb,
            rateControl_s *rc)
{
    if (trace->testId != 8) {
            return;
    }

    rc->vopRc   = NO;
    rc->mbRc    = NO;
    rc->picSkip = NO;

    if (mb->qpPrev == rc->qpMin) {
            mb->qpLum = rc->qpMax;
    } else if (mb->qpPrev == rc->qpMax) {
            mb->qpLum = rc->qpMin;
    } else {
            mb->qpLum = rc->qpMax;
    }

    mb->qpCh  = qpCh[MIN(51, MAX(0, mb->qpLum + mb->chromaQpOffset))];
    rc->qp = rc->qpHdr = mb->qpLum;
    slice->sliceQpDelta = mb->qpLum - slice->picInitQpMinus26 - 26;
}

#endif


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
 *  Description :  Encode picture
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "enccommon.h"
#include "ewl.h"
#include "H264CodeFrame.h"

#ifdef INTERNAL_TEST
#include "H264TestId.h"
#endif

/*------------------------------------------------------------------------------
    2. External compiler flags
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/* H.264 intra mode selection favors */
static const u32 h264Intra16Favor[52] = {
    24, 24, 24, 26, 27, 30, 32, 35, 39, 43, 48, 53, 58, 64, 71, 78,
    85, 93, 102, 111, 121, 131, 142, 154, 167, 180, 195, 211, 229,
    248, 271, 296, 326, 361, 404, 457, 523, 607, 714, 852, 1034,
    1272, 1588, 2008, 2568, 3318, 4323, 5672, 7486, 9928, 13216,
    17648
};

static const u32 h264PrevModeFavor[52] = {
    7, 7, 8, 8, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 24, 25, 27, 29, 30, 32, 34, 36, 38, 41, 43, 46,
    49, 51, 55, 58, 61, 65, 69, 73, 78, 82, 87, 93, 98, 104, 110,
    117, 124, 132, 140
};

/* H.264 motion estimation parameters */
static const u32 h264InterFavor[52] = {
    4,   4,   5,   6,   6,   7,   8,   9,  10,  12,  13,  15,  17,  19,
    21,  24,  26,  30,  34,  38,  42,  48,  53,  60,  68,  76,  85,  96,
    107, 121, 136, 152, 171, 192, 215, 242, 272, 305, 342, 384, 431, 484,
    544, 610, 685, 769, 863, 969, 1088, 1221, 1370, 1538
};

/* Favor value for web cam use case, 1.3x larger
static const u32 h264InterFavorWebCam[52] = {
      52,   52,   53,   54,   55,   57,   58,   62,   66,   68,
      71,   78,   80,   87,   89,   93,  101,  109,  117,  124,
     143,  156,  175,  197,  221,  245,  273,  305,  344,  386,
     435,  488,  546,  611,  678,  743,  806,  871,  941, 1001,
    1066, 1127, 1189, 1261, 1326, 1398, 1471, 1534, 1599, 1657,
    1716, 1781
};*/

/* Penalty factor in 1/256 units for skip mode */
static const u32 h264SkipSadPenalty[52] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 240, 224, 208, 192, 176, 160, 144, 128, 112,
    96,  80,  64,  60,  56,  52,  48,  44,  40,  36,
    32,  28,  24,  20,  18,  16,  16,  15,  15,  14,
    14,  13,  13,  12,  12,  11,  11,  10,  10,  9,
    9, 8
};

/* sqrt(2^((qp-12)/3))*8 */
static const u32 h264DiffMvPenalty[52] =
    { 2, 2, 3, 3, 3, 4, 4, 4, 5, 6,
    6, 7, 8, 9, 10, 11, 13, 14, 16, 18,
    20, 23, 26, 29, 32, 36, 40, 45, 51, 57,
    64, 72, 81, 91, 102, 114, 128, 144, 161, 181,
    203, 228, 256, 287, 323, 362, 406, 456, 512, 575,
    645, 724
};

/* 31*sqrt(2^((qp-12)/3))/4 */
static const u32 h264DiffMvPenalty4p[52] =
    { 2, 2, 2, 3, 3, 3, 4, 4, 5, 5,
    6, 7, 8, 9, 10, 11, 12, 14, 16, 17,
    20, 22, 25, 28, 31, 35, 39, 44, 49, 55,
    62, 70, 78, 88, 98, 110, 124, 139, 156, 175,
    197, 221, 248, 278, 312, 351, 394, 442, 496, 557,
    625, 701
};

static const i32 h264DmvPenalty[128] =   /* 4*sqrt(i*4*6) */
    {  0,   19,   27,   33,   39,   43,   48,   51,   55,   58,
      61,   64,   67,   70,   73,   75,   78,   80,   83,   85,
      87,   89,   91,   93,   96,   97,   99,  101,  103,  105,
     107,  109,  110,  112,  114,  115,  117,  119,  120,  122,
     123,  125,  126,  128,  129,  131,  132,  134,  135,  137,
     138,  139,  141,  142,  144,  145,  146,  147,  149,  150,
     151,  153,  154,  155,  156,  157,  159,  160,  161,  162,
     163,  165,  166,  167,  168,  169,  170,  171,  173,  174,
     175,  176,  177,  178,  179,  180,  181,  182,  183,  184,
     185,  186,  187,  188,  189,  190,  192,  192,  193,  194,
     195,  196,  197,  198,  199,  200,  201,  202,  203,  204,
     205,  206,  207,  208,  209,  210,  211,  211,  212,  213,
     214,  215,  216,  217,  218,  219,  219,  220
    };


/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/
static void H264SetNewFrame(h264Instance_s * inst);
static void SetSegmentMap(h264Instance_s *inst);
static i32 ExpGolombSigned(i32 val);
static void EncSwap32(u32 * buf, u32 sizeBytes);
static i32 float2fixpoint8(float data);

/*------------------------------------------------------------------------------

    float2fixpoint8

------------------------------------------------------------------------------*/
static i32 float2fixpoint8(float data)
{
    i32 i = 0;
    i32 result = 0;
    float pow2=2.0;
    /*0.16 format*/
    float base = 0.5;
    for(i= 0; i<8 ;i++)
    {
        result <<= 1;
        if(data >= base)
        {
           result |= 1;
           data -= base;

        }

        pow2 *= 2;
        base = 1.0/pow2;

    }
    return result;

}

/*------------------------------------------------------------------------------

    H264CodeFrame

------------------------------------------------------------------------------*/
h264EncodeFrame_e H264CodeFrame(h264Instance_s * inst)
{
    asicData_s *asic = &inst->asic;
    h264EncodeFrame_e ret;
    i32 status = ASIC_STATUS_ERROR, go_on;
    H264EncSliceReady slice;

    /* Reset callback struct */
    slice.slicesReadyPrev = 0;
    slice.slicesReady = 0;
    slice.sliceSizes = (u32 *) inst->asic.sizeTbl.virtualAddress;
    slice.sliceSizes += inst->naluOffset;
    slice.pOutBuf = inst->pOutBuf;
    slice.pAppData = inst->pAppData;

    H264SetNewFrame(inst);

#ifdef INTERNAL_TEST
    /* Configure the ASIC penalties according to the test vector */
    H264ConfigureTestPenalties(inst);
#endif

    EncAsicFrameStart(inst->asic.ewl, &inst->asic.regs);

    do {
        /* Encode one frame */
        i32 ewl_ret;

        /* Wait for IRQ for every slice or for complete frame */
        if ((inst->slice.sliceSize > 0) && inst->sliceReadyCbFunc)
            ewl_ret = EWLWaitHwRdy(asic->ewl, &slice.slicesReady);
        else
            ewl_ret = EWLWaitHwRdy(asic->ewl, NULL);

        if(ewl_ret != EWL_OK)
        {
            status = ASIC_STATUS_ERROR;

            if(ewl_ret == EWL_ERROR)
            {
                /* IRQ error => Stop and release HW */
                ret = H264ENCODE_SYSTEM_ERROR;
            }
            else    /*if(ewl_ret == EWL_HW_WAIT_TIMEOUT) */
            {
                /* IRQ Timeout => Stop and release HW */
                ret = H264ENCODE_TIMEOUT;
            }

            EncAsicStop(asic->ewl);
            /* Release HW so that it can be used by other codecs */
            EWLReleaseHw(asic->ewl);

            /* Prevent infinite loop in case of encoding timeout */
            go_on = 0;            

        }
        else
        {
            /* Check ASIC status bits and possibly release HW */
            status = EncAsicCheckStatus_V2(asic);

            switch (status)
            {
            case ASIC_STATUS_ERROR:
                ret = H264ENCODE_HW_ERROR;
                break;
            case ASIC_STATUS_HW_TIMEOUT:
                ret = H264ENCODE_TIMEOUT;
                break;
            case ASIC_STATUS_SLICE_READY:
                ret = H264ENCODE_OK;
                break;
            case ASIC_STATUS_BUFF_FULL:
                ret = H264ENCODE_OK;
                inst->stream.overflow = ENCHW_YES;
                break;
            case ASIC_STATUS_HW_RESET:
                ret = H264ENCODE_HW_RESET;
                break;
            case ASIC_STATUS_FUSE:
                ret = H264ENCODE_FUSE_ERROR;
                break;
            case ASIC_STATUS_FRAME_READY:
                {
                    /* Stream header remainder ie. last not full 64-bit address
                     * is counted in HW data. */
                    const u32 hw_offset = inst->stream.byteCnt & (0x07U);

                    inst->stream.byteCnt +=
                        asic->regs.outputStrmSize - hw_offset;
                    inst->stream.stream +=
                        asic->regs.outputStrmSize - hw_offset;

                    ret = H264ENCODE_OK;
                    break;
                }
            case ASIC_STATUS_LINE_BUFFER_DONE:
                ret = H264ENCODE_OK;
                /* SW handshaking: the callback function should wait until enough mb rows has been
                 *   feed into the input buffer and then update the input write pointer to make
                 *   the encoder continue to run. */
                if (!inst->inputLineBuf.inputLineBufHwModeEn)
                {
                    if (inst->inputLineBuf.cbFunc)
                        inst->inputLineBuf.cbFunc(inst->pAppData);
                }
                break;
            case ASIC_STATUS_RFC_BUFF_OVERFLOW:
                /* when current frame is finished, the software force the slice type to I slice
                 *   in next frame, and set a bigger QP */
                inst->rfcBufOverflow = ENCHW_YES;
                ret = H264ENCODE_OK;
                break;
            default:
                /* should never get here */
                ASSERT(0);
                ret = H264ENCODE_HW_ERROR;
            }

            /* double check  ASIC_STATUS_RFC_BUFF_OVERFLOW */
            if (asic->irqStatus & ASIC_STATUS_RFC_BUFF_OVERFLOW)
                inst->rfcBufOverflow = ENCHW_YES;

            go_on = (status == ASIC_STATUS_SLICE_READY) ||
                    (status == ASIC_STATUS_LINE_BUFFER_DONE) ||
                    (status == ASIC_STATUS_RFC_BUFF_OVERFLOW);

            /* Issue callback to application telling how many slices are available. */
            if (go_on && (slice.slicesReadyPrev < slice.slicesReady))
            {
              if (inst->sliceReadyCbFunc)
                  inst->sliceReadyCbFunc(&slice);
              slice.slicesReadyPrev = slice.slicesReady;
            }
        }
    } while (go_on);

    /* Don't update map for next frame unless it is modified. */
    inst->asic.regs.segmentMapUpdate = 0;

    return ret;
}

/*------------------------------------------------------------------------------

    Set encoding parameters at the beginning of a new frame.

------------------------------------------------------------------------------*/
void H264SetNewFrame(h264Instance_s * inst)
{
    asicData_s *asic = &inst->asic;
    regValues_s *regs = &inst->asic.regs;
    i32 qpHdr, qpMin, qpMax;
    i32 qp[4];
    i32 s,lamdaIdx, i, aroiDeltaQ = 0, tmp, qpAddr;
    i32 qpDeltaGain;
    float strength = 1.6f;
    float qpDeltaGainFloat;

    regs->outputStrmSize -= inst->stream.byteCnt;
    regs->outputStrmSize /= 8;  /* 64-bit addresses */
    regs->outputStrmSize &= (~0x07);    /* 8 multiple size */

    /* 64-bit aligned stream base address */
    regs->outputStrmBase += (inst->stream.byteCnt & (~0x07));

    /* bit offset in the last 64-bit word */
    regs->firstFreeBit = (inst->stream.byteCnt & 0x07) * 8;

    /* header remainder is byte aligned, max 7 bytes = 56 bits */
    if(regs->firstFreeBit != 0)
    {
        /* 64-bit aligned stream pointer */
        u8 *pTmp = (u8 *) ((ptr_t) (inst->stream.stream) & (ptr_t) (~0x07));
        u32 val;

        /* Clear remaining bits */
        for (val = 6; val >= regs->firstFreeBit/8; val--)
            pTmp[val] = 0;

        val = pTmp[0] << 24;
        val |= pTmp[1] << 16;
        val |= pTmp[2] << 8;
        val |= pTmp[3];

        regs->strmStartMSB = val;  /* 32 bits to MSB */

        if(regs->firstFreeBit > 32)
        {
            val = pTmp[4] << 24;
            val |= pTmp[5] << 16;
            val |= pTmp[6] << 8;

            regs->strmStartLSB = val;
        }
        else
            regs->strmStartLSB = 0;
    }
    else
    {
        regs->strmStartMSB = regs->strmStartLSB = 0;
    }

    /* MVC frames with same timestamp and field pairs have equal frameNum. */
    if ((inst->numViews > 1) || inst->interlaced)
        regs->frameNum = inst->slice.frameNum/2;
    else
        regs->frameNum = inst->slice.frameNum;

    regs->idrPicId = inst->slice.idrPicId;

    /* Store the final register values in the register structure */
    regs->sliceSizeMbRows = inst->slice.sliceSize / inst->mbPerRow;
    regs->chromaQpIndexOffset = inst->picParameterSet.chromaQpIndexOffset;

    /* Enable slice ready interrupts if defined by config and slices in use */
    regs->sliceReadyInterrupt =
            ENCH1_SLICE_READY_INTERRUPT & (inst->slice.sliceSize > 0);

    regs->picInitQp = (u32) (inst->picParameterSet.picInitQpMinus26 + 26);


    regs->qp = qpHdr = inst->rateControl.qpHdr >> QP_FRACTIONAL_BITS;

    regs->qpfrac = (inst->rateControl.qpHdr - (asic->regs.qp << QP_FRACTIONAL_BITS))<<(16 - QP_FRACTIONAL_BITS);

    regs->qpMin = qpMin = inst->rateControl.qpMin >> QP_FRACTIONAL_BITS;
    regs->qpMax = qpMax = inst->rateControl.qpMax >> QP_FRACTIONAL_BITS;
    regs->cpTarget = NULL;

    regs->filterDisable = inst->slice.disableDeblocking;
    if(inst->slice.disableDeblocking != 1)
    {
        regs->sliceAlphaOffset = inst->slice.filterOffsetA / 2;
        regs->sliceBetaOffset = inst->slice.filterOffsetB / 2;
    }
    else
    {
        regs->sliceAlphaOffset = 0;
        regs->sliceBetaOffset = 0;
    }
    regs->transform8x8Mode = inst->picParameterSet.transform8x8Mode;

    /* CABAC mode 2 uses ppsId=0 (CAVLC) for intra frames and
     * ppsId=1 (CABAC) for inter frames */
    if (inst->picParameterSet.enableCabac == 2)
    {
        regs->ppsId = (inst->slice.sliceType == ISLICE) ? 0 : 1;
        regs->enableCabac = regs->ppsId;
    }
    else
    {
        regs->ppsId = 0;
        regs->enableCabac = inst->picParameterSet.enableCabac;
    }

    if(inst->picParameterSet.enableCabac)
        regs->cabacInitIdc = inst->slice.cabacInitIdc;

    regs->constrainedIntraPrediction =
        (inst->picParameterSet.constIntraPred == ENCHW_YES) ? 1 : 0;

    /* Select frame type based on viewMode and frame number */
    if ((inst->slice.sliceType == ISLICE) && (inst->slice.nalUnitType == IDR))
        regs->frameCodingType = ASIC_INTRA;
    else if ((inst->numViews > 1) && (inst->numRefBuffsLum == 1) &&
             (inst->slice.frameNum % 2) && (inst->slice.frameNum > 1))
        regs->frameCodingType = ASIC_MVC_REF_MOD;
    else if ((inst->numViews > 1) && (inst->slice.frameNum % 2))
        regs->frameCodingType = ASIC_MVC;
    else
        regs->frameCodingType = ASIC_INTER;

    qpDeltaGainFloat = (float)inst->rateControl.rcMSESum / inst->rateControl.mbPerPic;
    //printf("qpDeltaGainFloat=%f\n",qpDeltaGainFloat);
    if(qpDeltaGainFloat<14)
        qpDeltaGainFloat = 14;
    qpDeltaGainFloat = qpDeltaGainFloat*qpDeltaGainFloat*strength/256;
    //printf("qpDeltaGainFloat=%f\n",qpDeltaGainFloat);
    qpDeltaGainFloat = strength;
    qpDeltaGain = (((i32)qpDeltaGainFloat)<<8)+ float2fixpoint8(qpDeltaGainFloat - (float)((i32)qpDeltaGainFloat) );

    if(regs->frameCodingType == ASIC_INTRA)
    {
        regs->qpDeltaMBGain = qpDeltaGain ;
        regs->offsetMBComplexity = 17;
    }
    else
    {
        regs->qpDeltaMBGain = qpDeltaGain ;
        regs->offsetMBComplexity = 15;
    }
    //printf("regs->offsetMBComplexity = 0x%x,regs->qpDeltaMBGain=0x%x\n",regs->offsetMBComplexity,regs->qpDeltaMBGain);

    regs->intraSliceMap1 = inst->intraSliceMap[0];
    regs->intraSliceMap2 = inst->intraSliceMap[1];
    regs->intraSliceMap3 = inst->intraSliceMap[2];

    if ((inst->slice.sliceType == ISLICE) && (inst->slice.nalUnitType == NONIDR)) {
        /* Non-IDR I-frame forced using intra slice map. */
        regs->intraSliceMap1 = regs->intraSliceMap2 =
        regs->intraSliceMap3 = 0xFFFFFFFF;
    }

    /* Disable frame reconstruction for view=1 frames that are not used
     * as reference. */
    if ( (inst->numViews > 1) &&
         (inst->numRefBuffsLum == 1) && (inst->slice.frameNum % 2) )
        regs->recWriteDisable = 1;

    if (inst->slice.quarterPixelMv == 0)
        inst->asic.regs.disableQuarterPixelMv = 1;
    else if (inst->slice.quarterPixelMv == 1)
    {
        /* Adaptive setting. When resolution larger than 1080p = 8160 macroblocks
         * there is not enough time to do 1/4 pixel ME */
        if(inst->mbPerFrame > 8160)
            inst->asic.regs.disableQuarterPixelMv = 1;
        else
            inst->asic.regs.disableQuarterPixelMv = 0;
    }
    else
        inst->asic.regs.disableQuarterPixelMv = 0;

    inst->asic.regs.splitMvMode = 1;

    /* Limits for AROI QP */
    if (inst->preProcess.adaptiveRoi) {
        aroiDeltaQ = inst->preProcess.adaptiveRoi;

        /* When qp close to max limit deltaQ. Note that deltaQ is negative. */
        aroiDeltaQ = MAX(aroiDeltaQ, 2*(qpHdr-51));
        /* When qp close to min limit deltaQ. */
        aroiDeltaQ = MAX(aroiDeltaQ, MIN(0, 10-qpHdr));
    }

    /* Segment map is used when AROI is enabled.
       Update segment map when either AROI is updated or ROIs are updated. */
    if (inst->preProcess.roiMapEnable)
    {
        regValues_s *regs = &inst->asic.regs;
        regs->segmentEnable = 1;
    }
    else if (inst->preProcess.adaptiveRoi &&
        (inst->preProcess.roiUpdate || regs->roiUpdate))
        SetSegmentMap(inst);

    regs->roiUpdate = 0;    /* Clear for next frame */

    if(inst->rateControl.mbRc != 0)
    {
  		  if (inst->preProcess.roiMapEnable)
  	    {
  	        regs->madQpDelta[0] = CLIP3(inst->preProcess.qpOffset[0], -qpHdr, 51-qpHdr);
  	        regs->madQpDelta[0] = CLIP3(regs->madQpDelta[0], -8, 7);
  	        regs->madQpDelta[1] = CLIP3(inst->preProcess.qpOffset[1], -30, 30);
  	        regs->madQpDelta[2] = CLIP3(inst->preProcess.qpOffset[2], -30, 30); /* AROI ID 3 */
  	    }
        else if (inst->preProcess.adaptiveRoi && regs->segmentEnable) {
             regs->madQpDelta[0] = CLIP3(inst->preProcess.qpOffset[0], -qpHdr, 51-qpHdr);
             regs->madQpDelta[0] = CLIP3(regs->madQpDelta[0], -8, 7);
             regs->madQpDelta[1] = CLIP3(inst->preProcess.qpOffset[1], -30, 30);
             regs->madQpDelta[2] = CLIP3(inst->preProcess.qpOffset[2], -30, 30); /* AROI ID 3 */
        }
        else
        {
            //not support MADQP adjustment
            regs->madQpDelta[0] = 0;
            regs->madQpDelta[1] = 0;
            regs->madQpDelta[2] = 0;
        }
        inst->preProcess.qpOffset[0] = regs->madQpDelta[0];
        inst->preProcess.qpOffset[1] = regs->madQpDelta[1];
        inst->preProcess.qpOffset[2] = regs->madQpDelta[2];
    }
    else if (inst->preProcess.roiMapEnable)
    {
        regs->madQpDelta[0] = CLIP3(inst->preProcess.qpOffset[0], -qpHdr, 51-qpHdr);
        regs->madQpDelta[0] = CLIP3(regs->madQpDelta[0], -8, 7);
        regs->madQpDelta[1] = CLIP3(inst->preProcess.qpOffset[1], -qpHdr, 51-qpHdr);
        regs->madQpDelta[2] = CLIP3(inst->preProcess.qpOffset[2], -qpHdr, 51-qpHdr); /* AROI ID 3 */
        inst->preProcess.qpOffset[0] = regs->madQpDelta[0];
        inst->preProcess.qpOffset[1] = regs->madQpDelta[1];
        inst->preProcess.qpOffset[2] = regs->madQpDelta[2];
    }
    else if (inst->preProcess.adaptiveRoi && regs->segmentEnable) {
        regs->madQpDelta[0] = CLIP3(-regs->roi1DeltaQp, -qpHdr, 51-qpHdr);
        regs->madQpDelta[0] = CLIP3(regs->madQpDelta[0], -8, 7);
        regs->madQpDelta[1] = CLIP3(-regs->roi2DeltaQp, -qpHdr, 51-qpHdr);
        regs->madQpDelta[2] = CLIP3(aroiDeltaQ, -qpHdr, 51-qpHdr); /* AROI ID 3 */
    } else {
        /* MAD thresholding and segment map uses the same MB QP delta values. */
        regs->madQpDelta[0] = CLIP3(inst->rateControl.mbQpAdjustment[0],
                                    qpMin - qpHdr, qpMax - qpHdr);
        regs->madQpDelta[1] = CLIP3(inst->rateControl.mbQpAdjustment[1],
                                    qpMin - qpHdr, qpMax - qpHdr);
        regs->madQpDelta[2] = CLIP3(inst->rateControl.mbQpAdjustment[2],
                                    qpMin - qpHdr, qpMax - qpHdr);
    }

    /* QP values for each segment 0-3 */
    qp[0] = qpHdr;
    qp[1] = qpHdr + regs->madQpDelta[0];
    qp[2] = qpHdr + regs->madQpDelta[1];
    qp[3] = qpHdr + regs->madQpDelta[2];

    /* Set penalty values for every segment based on segment QP */
    for (s = 0; s < 4; s++) {
        regs->pen[s][ASIC_PENALTY_I16FAVOR] = h264Intra16Favor[qp[s]];
        regs->pen[s][ASIC_PENALTY_I4_PREV_MODE_FAVOR] = h264PrevModeFavor[qp[s]];
        regs->pen[s][ASIC_PENALTY_INTER_FAVOR] = h264InterFavor[qp[s]];
        regs->pen[s][ASIC_PENALTY_SKIP] = h264SkipSadPenalty[qp[s]];
        {
            i32 tmp = h264DiffMvPenalty4p[qp[s]]/10;
            tmp = (tmp + 4) / 8;
            if( tmp == 0 )  tmp++;
            regs->pen[s][ASIC_PENALTY_DMV_4P] = tmp;

            tmp  = (4*h264DiffMvPenalty[qp[s]])/5;
            tmp = (tmp + 4) / 8;
            if( tmp == 0 )  tmp++;
            regs->pen[s][ASIC_PENALTY_DMV_1P] = tmp;
        }
        regs->pen[s][ASIC_PENALTY_DMV_QP] = h264DiffMvPenalty[qp[s]];
        regs->pen[s][ASIC_PENALTY_SPLIT16x8] = 0;
        regs->pen[s][ASIC_PENALTY_SPLIT8x8] = 0;
        regs->pen[s][ASIC_PENALTY_SPLIT8x4] = 0;
        regs->pen[s][ASIC_PENALTY_SPLIT4x4] = 0;
        regs->pen[s][ASIC_PENALTY_SPLIT_ZERO] = 0;
#if 0
        /* VP8 penalties not used. */
        for (i = ASIC_PENALTY_I16MODE0; i <= ASIC_PENALTY_I4MODE9; i++)
            regs->pen[s][i] = 0;
        for (i = ASIC_PENALTY_DZ_RATE0; i <= ASIC_PENALTY_DZ_SKIP1; i++)
            regs->pen[s][i] = 0;
#endif
        regs->pen[s][ASIC_PENALTY_GOLDEN] = h264DiffMvPenalty[qp[s]]/2;
        if (regs->pen[s][ASIC_PENALTY_GOLDEN] > 255)
            regs->pen[s][ASIC_PENALTY_GOLDEN] = 255;
       // regs->pen[s][ASIC_PENALTY_DMV_COST_CONST] = 0;
        //regs->pen[s][ASIC_PENALTY_COST_INTER] = 0;
    }

    regs->mbRcEnable = 0;
    regs->mbQpDeltaRange = 10;
  	regs->offsetSliceQp = 0;
  	if( qpHdr > 36)
  	{
  		regs->offsetSliceQp = 36 - qpHdr;
  	}
  	if(qpHdr < 15)
  	{
  		regs->offsetSliceQp = 15 - qpHdr;
  	}
    if(inst->rateControl.mbRc != 0)
    {
        /* Set penalty values for every segment based on segment QP */
        regs->mbRcEnable = inst->rateControl.mbRc;
        for (s = -15, lamdaIdx = 17 - regs->offsetSliceQp; s <= +15; s++, lamdaIdx++) {
            qpAddr = (lamdaIdx & 0x1f);
            regs->pen[qpAddr][ASIC_PENALTY_I16FAVOR] = h264Intra16Favor[CLIP3(CLIP3(qpHdr - s + regs->offsetSliceQp,0,51),qpMin, qpMax)];
            regs->pen[qpAddr][ASIC_PENALTY_I4_PREV_MODE_FAVOR] = h264PrevModeFavor[CLIP3(CLIP3(qpHdr - s + regs->offsetSliceQp,0,51),qpMin, qpMax)];
            regs->pen[qpAddr][ASIC_PENALTY_INTER_FAVOR] = h264InterFavor[CLIP3(CLIP3(qpHdr - s + regs->offsetSliceQp,0,51),qpMin, qpMax)];
            regs->pen[qpAddr][ASIC_PENALTY_SKIP] = h264SkipSadPenalty[CLIP3(CLIP3(qpHdr - s + regs->offsetSliceQp,0,51),qpMin, qpMax)];
           #if 0
            {
                i32 tmp = h264DiffMvPenalty4p[CLIP3(CLIP3(qpHdr - s + regs->offsetSliceQp,0,51),qpMin, qpMax)]/10;
                tmp = (tmp + 4) / 8;
                if( tmp == 0 )  tmp++;
                regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_DMV_4P] = tmp;

                tmp  = (4*h264DiffMvPenalty[CLIP3(CLIP3(qpHdr - s + regs->offsetSliceQp,0,51),qpMin, qpMax)])/5;
                tmp = (tmp + 4) / 8;
                if( tmp == 0 )  tmp++;
                regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_DMV_1P] = tmp;
            }
            #endif
            regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_DMV_QP] = h264DiffMvPenalty[CLIP3(CLIP3(qpHdr - s + regs->offsetSliceQp,0,51),qpMin, qpMax)];
            regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_SPLIT16x8] = 0;
            regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_SPLIT8x8] = 0;
            regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_SPLIT8x4] = 0;
            regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_SPLIT4x4] = 0;
            //regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_SPLIT_ZERO] = 0;
#if 0
            /* VP8 penalties not used. */
            for (i = ASIC_PENALTY_I16MODE0; i <= ASIC_PENALTY_I4MODE9; i++)
                regs->pen[(lamdaIdx & 0x1f)][i] = 0;
            for (i = ASIC_PENALTY_DZ_RATE0; i <= ASIC_PENALTY_DZ_SKIP1; i++)
                regs->pen[(lamdaIdx & 0x1f)][i] = 0;
#endif
            regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_GOLDEN] = h264DiffMvPenalty[CLIP3(CLIP3(qpHdr - s + regs->offsetSliceQp,0,51),qpMin, qpMax)]/2;
            if (regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_GOLDEN] > 255)
                regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_GOLDEN] = 255;

            //regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_DMV_COST_CONST] = 0;
            //regs->pen[(lamdaIdx & 0x1f)][ASIC_PENALTY_COST_INTER] = 0;
        }
    }

    /* DMV penalty tables */
    for (i = 0; i < ASIC_PENALTY_TABLE_SIZE; i++) {
        regs->dmvPenalty[i] = (h264DmvPenalty[i] + 1) / 2;
        regs->dmvQpelPenalty[i] = MIN(255,ExpGolombSigned(i));
    }
    regs->zeroMvFavorDiv2 = 10;

    /* HW base address for NAL unit sizes is affected by start offset
     * and SW created NALUs. */
    regs->sizeTblBase = asic->sizeTbl.busAddress +
                            inst->naluOffset*4 + inst->numNalus*4;

    /* HW Base must be 64-bit aligned */
    ASSERT(regs->sizeTblBase%8 == 0);

    if (regs->segmentMapUpdate) {
        /* Swap segment map endianess to match ASIC reading. */
        EncSwap32((u32*)inst->asic.segmentMap.virtualAddress,
                                    (inst->mbPerFrame+15)/16*8);
#ifdef TRACE_SEGMENTS
    EncTraceSegments((u32*)inst->asic.segmentMap.virtualAddress,
        (inst->mbPerFrame+15)/16*8, regs->segmentEnable,
        regs->segmentMapUpdate, 0, regs->madQpDelta);
#endif
    }

    /* When segment map is enabled, MAD thresholding can't be used. */
    if (!regs->segmentEnable) {
        /* MAD threshold range [0, 63*256] register 6-bits range [0,63] */
        regs->madThreshold[0] = inst->mad.threshold[0] / 256;
        regs->madThreshold[1] = inst->mad.threshold[1] / 256;
        regs->madThreshold[2] = inst->mad.threshold[2] / 256;
    }
    else
        regs->madThreshold[0] = regs->madThreshold[1] = regs->madThreshold[2] = 0;

    /* MVC */
    regs->mvcAnchorPicFlag = inst->mvc.anchorPicFlag;
    regs->mvcPriorityId = inst->mvc.priorityId;
    regs->mvcViewId = inst->mvc.viewId;
    regs->mvcTemporalId = inst->mvc.temporalId;
    regs->mvcInterViewFlag = inst->mvc.interViewFlag;

    /* Interlaced */
    if (inst->interlaced) {
        regs->fieldPicFlag = 1;
        regs->bottomFieldFlag = (inst->slice.frameNum%2) == inst->slice.fieldOrder;
        regs->fieldParity = 0;
        /* I-frame bottom field will reference top field */
        if (inst->slice.frameNum == 1) regs->fieldParity = 2-inst->slice.fieldOrder;
    }

    /* current marked as long term */
    regs->markCurrentLongTerm = inst->picBuffer.cur_pic->grf == true;

    /* Pskip coding mode: 0=force coeff to zero (PSKIP), 1=don't force coeff to
     * zero (INTER) */
    if (qpHdr < 45) {
        regs->pskipMode = 1;
    } else {
        regs->pskipMode = 1;
    }

    /* Boost */
    if (inst->rateControl.mbQpAutoBoost) {
        if (inst->slice.frameNum%15 == 0) {
            tmp = qpHdr - 8;
            regs->pskipMode = 1;
        } else {
            tmp = qpHdr;
        }
        regs->boostVar1 = 1000;
        regs->boostVar2 = 10;
        regs->boostQp = MIN(qpMax, MAX(qpMin, tmp));
    } else {
        regs->boostVar1 = 0;
        regs->boostVar2 = 0;
        regs->boostQp = 52;
    }

    /* Intra vs Inter variance limits */
    regs->varLimitDiv32      = 256/32;	/* 0..511 */
    regs->varInterFavorDiv16 = 16/16;	/* 0..255 */
    regs->varMultiplier      = 24;	/* 0..31 */
    regs->varAdd             = 1;	/* 0..15 */

    /* configure the input buffer control related register.*/
    regs->lineBufferEn = inst->inputLineBuf.inputLineBufEn;
    regs->lineBufferHwHandShake = inst->inputLineBuf.inputLineBufHwModeEn;
    if ((regs->lineBufferHwHandShake == 0) && inst->inputLineBuf.inputLineBufDepth)
      regs->lineBufferIrqEnable = 1;
    else
      regs->lineBufferIrqEnable = 0;
    regs->lineBufferLoopBackEn = inst->inputLineBuf.inputLineBufLoopBackEn;
    regs->lineBufferDepth = inst->inputLineBuf.inputLineBufDepth;
    regs->mbWrPtr = inst->inputLineBuf.wrCnt;
    regs->mbRrPtr = 0;  /* MB write pointer. */

#if defined(ASIC_WAVE_TRACE_TRIGGER)
    {
        u32 index;

        if(asic->regs.internalImageLumBaseW ==
           asic->internalImageLuma[0].busAddress)
            index = 0;
        else
            index = 1;

        EWLmemset(asic->internalImageLuma[index].virtualAddress, 0,
                  asic->internalImageLuma[index].size);
    }
#endif

}

/*------------------------------------------------------------------------------

        SetSegmentMap

------------------------------------------------------------------------------*/
void SetSegmentMap(h264Instance_s *inst)
{
    regValues_s *regs = &inst->asic.regs;
    u32 *map = inst->asic.segmentMap.virtualAddress;
    u32 mask, mb, x, y;

    if (inst->preProcess.roiMbCount[2] == 0) {
        /* AROI update with empty map means we can disable segments. */
        regs->segmentEnable = 0;
    } else {
        regs->segmentEnable = 1;
        regs->segmentMapUpdate = 1;

        /* Set AROI (ID=3), ROI1 (ID=1) and ROI2 (ID=2) into segment map. */
        for (y = 0, mb = 0, mask = 0; y < inst->mbPerCol; y++) {
            for (x = 0; x < inst->mbPerRow; x++, mb++) {
                u32 id = 0;
                /* AROI is checked first, then ROI1 and last ROR2 */
                if (inst->preProcess.roiSegmentMap[2][mb])
                    id = 3;
                if ((x >= regs->roi1Left) && (x <= regs->roi1Right) &&
                    (y >= regs->roi1Top)  && (y <= regs->roi1Bottom))
                    id = 1;
                if ((x >= regs->roi2Left) && (x <= regs->roi2Right) &&
                    (y >= regs->roi2Top)  && (y <= regs->roi2Bottom))
                    id = 2;

                mask |= id << (28-4*(mb%8));
                if ((mb%8) == 7) {
                    *map++ = mask;
                    mask = 0;
                }
            }
        }
        *map++ = mask;
    }
    inst->preProcess.roiCoded = 1;  /* AROI is coded into stream */

}

/*------------------------------------------------------------------------------

        ExpGolombSigned

------------------------------------------------------------------------------*/
i32 ExpGolombSigned(i32 val)
{
        i32 tmp = 0;

        if (val > 0) {
                val = 2*val;
        } else {
                val = -2*val + 1;
        }

        while (val >> ++tmp);

        /*
        H264NalBits(stream, val, tmp*2-1);
        */

        return tmp*2-1;
}

/*------------------------------------------------------------------------------
	EncSwap32
------------------------------------------------------------------------------*/
void EncSwap32(u32 * buf, u32 sizeBytes)
{
    u32 i = 0;
    u32 words = sizeBytes / 4;

    ASSERT((sizeBytes % 8) == 0);

    while (i < words)
    {
#if(ENCH1_OUTPUT_SWAP_32 == 1)    /* need this for 64-bit HW */
        u32 val  = buf[i];
        u32 val2 = buf[i+1];

        buf[i]   = val2;
        buf[i+1] = val;
#endif

        i+=2;
    }

}


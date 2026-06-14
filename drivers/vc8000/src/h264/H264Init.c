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
 *  Abstract : Encoder initialization and setup
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "string.h"
#include "H264Init.h"
#include "H264Denoise.h"
#include "enccommon.h"
#include "ewl.h"


/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/
#define H264ENC_MIN_ENC_WIDTH       132     /* 144 - 12 pixels overfill */
#define H264ENC_MAX_ENC_WIDTH       4080
#define H264ENC_MIN_ENC_HEIGHT      96
#define H264ENC_MAX_ENC_HEIGHT      4080
#define H264ENC_MAX_REF_FRAMES      3

#define H264ENC_MAX_MBS_PER_PIC     65025   /* 4080x4080 */

/* Level 51 MB limit is increased to enable max resolution */
#define H264ENC_MAX_LEVEL           51

#define H264ENC_DEFAULT_QP          26

/* Tracing macro */
#ifdef H264ENC_TRACE
#define APITRACE(str) H264EncTrace(str)
#else
#define APITRACE(str)
#endif

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/
static bool_e SetParameter(h264Instance_s *inst,
                           const H264EncConfig *pEncCfg);
static bool_e CheckParameter(const h264Instance_s *inst);

static i32 SetPictureBuffer(h264Instance_s *inst);

/*------------------------------------------------------------------------------

    H264CheckCfg

    Function checks that the configuration is valid.

    Input   pEncCfg Pointer to configuration structure.

    Return  ENCHW_OK      The configuration is valid.
            ENCHW_NOK     Some of the parameters in configuration are not valid.

------------------------------------------------------------------------------*/
bool_e H264CheckCfg(const H264EncConfig *pEncCfg)
{
    u32 height = pEncCfg->height;

    ASSERT(pEncCfg);

    /* Interlaced coding sets height limitations per field */
    if (pEncCfg->viewMode == H264ENC_INTERLACED_FIELD)
        height /= 2;

    if ((pEncCfg->streamType != H264ENC_BYTE_STREAM) &&
            (pEncCfg->streamType != H264ENC_NAL_UNIT_STREAM))
    {
        APITRACE("H264CheckCfg: Invalid stream type");
        return ENCHW_NOK;
    }

    /* Encoded image width limits, multiple of 4 */
    if (pEncCfg->width < H264ENC_MIN_ENC_WIDTH ||
            pEncCfg->width > H264ENC_MAX_ENC_WIDTH || (pEncCfg->width & 0x3) != 0)
    {
        APITRACE("H264CheckCfg: Invalid width");
        return ENCHW_NOK;
    }

    /* Encoded image height limits, multiple of 2 */
    if (height < H264ENC_MIN_ENC_HEIGHT ||
            height > H264ENC_MAX_ENC_HEIGHT || (height & 0x1) != 0)
    {
        APITRACE("H264CheckCfg: Invalid height");
        return ENCHW_NOK;
    }

    /* Scaled image width limits, multiple of 4 (YUYV) and smaller than input */
    if ((pEncCfg->scaledWidth > pEncCfg->width) ||
            (pEncCfg->scaledWidth & 0x3) != 0)
    {
        APITRACE("H264CheckCfg: Invalid scaledWidth");
        return ENCHW_NOK;
    }

    if ((pEncCfg->scaledHeight > height) ||
            (pEncCfg->scaledHeight & 0x1) != 0)
    {
        APITRACE("H264CheckCfg: Invalid scaledHeight");
        return ENCHW_NOK;
    }

    if ((pEncCfg->scaledWidth == pEncCfg->width) &&
            (pEncCfg->scaledHeight == height))
    {
        APITRACE("H264CheckCfg: Invalid scaler output, no downscaling");
        return ENCHW_NOK;
    }

    /* total macroblocks per picture limit */
    if (((height + 15) / 16) *((pEncCfg->width + 15) / 16) >
            H264ENC_MAX_MBS_PER_PIC)
    {
        APITRACE("H264CheckCfg: Invalid max resolution");
        return ENCHW_NOK;
    }

    /* Check frame rate */
    if (pEncCfg->frameRateNum < 1 || pEncCfg->frameRateNum > ((1 << 20) - 1))
    {
        APITRACE("H264CheckCfg: Invalid frameRateNum");
        return ENCHW_NOK;
    }

    if (pEncCfg->frameRateDenom < 1)
    {
        APITRACE("H264CheckCfg: Invalid frameRateDenom");
        return ENCHW_NOK;
    }

    /* special allowal of 1000/1001, 0.99 fps by customer request */
    if (pEncCfg->frameRateDenom > pEncCfg->frameRateNum &&
            !(pEncCfg->frameRateDenom == 1001 && pEncCfg->frameRateNum == 1000))
    {
        APITRACE("H264CheckCfg: Invalid frameRate");
        return ENCHW_NOK;
    }

    /* check level */
    if ((pEncCfg->level > H264ENC_MAX_LEVEL) &&
            (pEncCfg->level != H264ENC_LEVEL_1_b))
    {
        APITRACE("H264CheckCfg: Invalid level");
        return ENCHW_NOK;
    }

    if (pEncCfg->refFrameAmount < 1 ||
            pEncCfg->refFrameAmount > H264ENC_MAX_REF_FRAMES ||
            (pEncCfg->refFrameAmount > 1 &&
             pEncCfg->viewMode != H264ENC_BASE_VIEW_MULTI_BUFFER))
    {
        APITRACE("H264CheckCfg: Invalid refFrameAmount");
        return ENCHW_NOK;
    }

    /* check HW limitations */
    {
        EWLHwConfig_t cfg = EWLReadAsicConfig();
        /* is H.264 encoding supported */
        if (cfg.h264Enabled == EWL_HW_CONFIG_NOT_SUPPORTED)
        {
            APITRACE("H264CheckCfg: Invalid format, h264 not supported by HW");
            return ENCHW_NOK;
        }

        /* max width supported */
        if (cfg.maxEncodedWidth < pEncCfg->width)
        {
            APITRACE("H264CheckCfg: Invalid width, not supported by HW");
            return ENCHW_NOK;
        }
    }

    return ENCHW_OK;
}

/*------------------------------------------------------------------------------

    H264Init

    Function initializes the Encoder and create new encoder instance.

    Input   pEncCfg     Encoder configuration.
            instAddr    Pointer to instance will be stored in this address

    Return  H264ENC_OK
            H264ENC_MEMORY_ERROR
            H264ENC_EWL_ERROR
            H264ENC_EWL_MEMORY_ERROR
            H264ENC_INVALID_ARGUMENT

------------------------------------------------------------------------------*/
H264EncRet H264Init(const H264EncConfig *pEncCfg, h264Instance_s **instAddr)
{
    h264Instance_s *inst = NULL;
    const void *ewl = NULL;
    H264EncRet ret = H264ENC_OK;
    EWLInitParam_t param;

    ASSERT(pEncCfg);
    ASSERT(instAddr);

    *instAddr = NULL;

    param.clientType = EWL_CLIENT_TYPE_H264_ENC;

    /* Init EWL */
    if ((ewl = EWLInit(&param)) == NULL)
        return H264ENC_EWL_ERROR;

    /* Encoder instance */
    inst = (h264Instance_s *) EWLcalloc(1, sizeof(h264Instance_s));

    if (inst == NULL)
    {
        ret = H264ENC_MEMORY_ERROR;
        goto err;
    }
    
    /* read HW configuration */
    inst->hwCfg = EWLReadAsicConfig();

    /* Default values */
    H264SeqParameterSetInit(&inst->seqParameterSet);
    H264PicParameterSetInit(&inst->picParameterSet);
    H264SliceInit(&inst->slice);
    H264EncDnfInit(inst);

    /* Set parameters depending on user config */
    if (SetParameter(inst, pEncCfg) != ENCHW_OK)
    {
        ret = H264ENC_INVALID_ARGUMENT;
        goto err;
    }

    if (SetPictureBuffer(inst) != ENCHW_OK)
    {
        ret = H264ENC_INVALID_ARGUMENT;
        goto err;
    }

    /* Check and init the rest of parameters */
    if (CheckParameter(inst) != ENCHW_OK)
    {
        ret = H264ENC_INVALID_ARGUMENT;
        goto err;
    }

    if (H264InitRc(&inst->rateControl, 1) != ENCHW_OK)
    {
        return H264ENC_INVALID_ARGUMENT;
    }

    if (EncPreProcessAlloc(&inst->preProcess,
                           inst->mbPerRow * inst->mbPerCol) != ENCHW_OK)
        return ENCHW_NOK;

    /* Initialize ASIC */
    inst->asic.ewl = ewl;
    (void) EncAsicControllerInit(&inst->asic);

    /* Allocate internal SW/HW shared memories */
    if (EncAsicMemAlloc_V2(&inst->asic,
                           (u32) inst->preProcess.lumWidth,
                           (u32) inst->preProcess.lumHeight,
                           (u32) inst->preProcess.scaledWidth,
                           (u32) inst->preProcess.scaledHeight,
                           ASIC_H264, inst->numRefBuffsLum,
                           inst->numRefBuffsChr) != ENCHW_OK)
    {

        ret = H264ENC_EWL_MEMORY_ERROR;
        goto err;
    }

    /* Assign allocated HW frame buffers into picture buffer */
    H264PictureBufferSetupH264(&inst->picBuffer, &inst->asic,
                                  inst->numRefBuffsLum, inst->numRefBuffsChr);

    *instAddr = inst;

    /* init VUI */
    {
        const h264VirtualBuffer_s *vb = &inst->rateControl.virtualBuffer;

        H264SpsSetVuiTimigInfo(&inst->seqParameterSet,
                               vb->timeScale, vb->unitsInTic);
    }

    /* Disable 4x4 MV mode for high levels to limit MaxMvsPer2Mb */
    if (inst->seqParameterSet.levelIdc >= 31)
        inst->asic.regs.h264Inter4x4Disabled = 1;
    else
        inst->asic.regs.h264Inter4x4Disabled = 0;

    return ret;

err:
    if (inst != NULL)
        EWLfree(inst);
    if (ewl != NULL)
        (void) EWLRelease(ewl);

    return ret;
}

/*------------------------------------------------------------------------------

    H264Shutdown

    Function frees the encoder instance.

    Input   h264Instance_s *    Pointer to the encoder instance to be freed.
                            After this the pointer is no longer valid.

------------------------------------------------------------------------------*/
void H264Shutdown(h264Instance_s *data)
{
    const void *ewl;

    ASSERT(data);

    ewl = data->asic.ewl;

    EncAsicMemFree_V2(&data->asic);

    EncPreProcessFree(&data->preProcess);

    EWLfree(data);

    (void) EWLRelease(ewl);
}

/*------------------------------------------------------------------------------

    SetParameter

    Set all parameters in instance to valid values depending on user config.

------------------------------------------------------------------------------*/
bool_e SetParameter(h264Instance_s *inst, const H264EncConfig *pEncCfg)
{
    static gopinfo_s SvctGopL1[2] =
    {
        {.layer = 1, .isRef = 0, .orderCmd = 0, .markCmd = 0}, /* L1 */
        {.layer = 0, .isRef = 1, .orderCmd = 0, .markCmd = 0}  /* L0 */
    }; 
    static gopinfo_s SvctGopL2[4] =
    {
        {.layer = 2, .isRef = 0, .orderCmd = 0, .markCmd = 0}, /* L2 */
        {.layer = 1, .isRef = 1, .orderCmd = 0, .markCmd = 0}, /* L1 */
        {.layer = 2, .isRef = 0, .orderCmd = 0, .markCmd = 0}, /* L2 */
        {.layer = 0, .isRef = 1, .orderCmd = 2, .markCmd = 0}  /* L0 */
    }; 
    static gopinfo_s SvctGopL3[8] =
    {
        {.layer = 3, .isRef = 0, .orderCmd = 0, .markCmd = 0}, /* L3 NUM0 */
        {.layer = 2, .isRef = 1, .orderCmd = 0, .markCmd = 0}, /* L2 NUM1 */
        {.layer = 3, .isRef = 0, .orderCmd = 0, .markCmd = 0}, /* L3 NUM1 */
        {.layer = 1, .isRef = 1, .orderCmd = 2, .markCmd = 0}, /* L1 NUM2 */
        {.layer = 3, .isRef = 0, .orderCmd = 0, .markCmd = 0}, /* L3 NUM2 */
        {.layer = 2, .isRef = 1, .orderCmd = 0, .markCmd = 2}, /* L2 NUM3 */
        {.layer = 3, .isRef = 0, .orderCmd = 0, .markCmd = 0}, /* L3 NUM3 */
        {.layer = 0, .isRef = 1, .orderCmd = 4, .markCmd = 0}  /* L0 NUM4 */
    }; 
    static gopinfo_s SvctReorderTest[16] =
    {
        {.layer = 4, .isRef = 1, .orderCmd = 0, .markCmd = 1}, /* L3 */
        {.layer = 3, .isRef = 1, .orderCmd = 2, .markCmd = 1}, /* L2 */
        {.layer = 4, .isRef = 1, .orderCmd = 3, .markCmd = 1}, /* L3 */
        {.layer = 2, .isRef = 1, .orderCmd = 4, .markCmd = 1}, /* L1 */
        {.layer = 4, .isRef = 1, .orderCmd = 5, .markCmd = 1}, /* L3 */
        {.layer = 3, .isRef = 1, .orderCmd = 6, .markCmd = 1}, /* L2 */
        {.layer = 4, .isRef = 1, .orderCmd = 7, .markCmd = 1}, /* L3 */
        {.layer = 1, .isRef = 1, .orderCmd = 8, .markCmd = 1},  /* L0 */
        {.layer = 4, .isRef = 1, .orderCmd = 9, .markCmd = 1}, /* L3 */
        {.layer = 3, .isRef = 1, .orderCmd = 10, .markCmd = 1}, /* L2 */
        {.layer = 4, .isRef = 1, .orderCmd = 11, .markCmd = 1}, /* L3 */
        {.layer = 2, .isRef = 1, .orderCmd = 12, .markCmd = 1}, /* L1 */
        {.layer = 4, .isRef = 1, .orderCmd = 13, .markCmd = 1}, /* L3 */
        {.layer = 2, .isRef = 1, .orderCmd = 14, .markCmd = 1}, /* L2 */
        {.layer = 4, .isRef = 1, .orderCmd = 15, .markCmd = 1}, /* L3 */
        {.layer = 0, .isRef = 1, .orderCmd = 16, .markCmd = 0}  /* L0 */
    }; 
    static gopinfo_s *svctGop[4] = {&SvctGopL1[0], &SvctGopL2[0], &SvctGopL3[0], &SvctReorderTest[0]};

    i32 width, height, tmp, bps;
    EWLHwConfig_t cfg = EWLReadAsicConfig();

    ASSERT(inst);

    /* Internal images, next macroblock boundary */
    width = 16 * ((pEncCfg->width + 15) / 16);
    height = 16 * ((pEncCfg->height + 15) / 16);

    /* SVCT */
    inst->svc.level = pEncCfg->svctLevel;
    inst->svc.enableSvctPrefix = pEncCfg->enableSvctPrefix;
    if (pEncCfg->svctLevel)
    {
        if (inst->hwCfg.svctSupport==0)
          return ENCHW_NOK;

        u32 level = pEncCfg->svctLevel;
        u32 length = 1 << level;
        
        inst->seqParameterSet.gapsInFrameNumValueAllowed = ENCHW_YES;
        inst->svc.gopIndex = -1;
        inst->svc.gopLength = length;
        inst->svc.base_priority_id = inst->svc.next_base_priority_id = 0;
        memcpy(&inst->svc.gop[0], svctGop[level-1], sizeof(gopinfo_s)*length);
    }

    /* stream type */
    if (pEncCfg->streamType == H264ENC_BYTE_STREAM)
    {
        inst->asic.regs.h264StrmMode = ASIC_H264_BYTE_STREAM;
        inst->picParameterSet.byteStream = ENCHW_YES;
        inst->seqParameterSet.byteStream = ENCHW_YES;
        inst->rateControl.sei.byteStream = ENCHW_YES;
        inst->slice.byteStream = ENCHW_YES;
    }
    else
    {
        inst->asic.regs.h264StrmMode = ASIC_H264_NAL_UNIT;
        inst->picParameterSet.byteStream = ENCHW_NO;
        inst->seqParameterSet.byteStream = ENCHW_NO;
        inst->rateControl.sei.byteStream = ENCHW_NO;
        inst->slice.byteStream = ENCHW_NO;
    }

    if (pEncCfg->viewMode == H264ENC_BASE_VIEW_SINGLE_BUFFER)
    {
        inst->numViews = 1;
        inst->numRefBuffsLum = 1;
        inst->numRefBuffsChr = 2;
        inst->seqParameterSet.numRefFrames = 1;
    }
    else if (pEncCfg->viewMode == H264ENC_BASE_VIEW_DOUBLE_BUFFER)
    {
        inst->numViews = 1;
        inst->numRefBuffsLum = 2;
        inst->numRefBuffsChr = 2;
        inst->seqParameterSet.numRefFrames = 1;
    }
    else if (pEncCfg->viewMode == H264ENC_BASE_VIEW_MULTI_BUFFER)
    {
        inst->numViews = 1;
        inst->numRefBuffsLum = pEncCfg->refFrameAmount + 1;
        inst->numRefBuffsChr = pEncCfg->refFrameAmount + 1;
        if (pEncCfg->svctLevel == 3) 
        {
            inst->seqParameterSet.numRefFrames = 4;
        }
        else if (pEncCfg->svctLevel == 4) 
        {
            inst->seqParameterSet.numRefFrames = 16; /*just for HW test */
        }
        else
        {
            inst->seqParameterSet.numRefFrames = pEncCfg->refFrameAmount;
        }
    }
    else if (pEncCfg->viewMode == H264ENC_MVC_STEREO_INTER_VIEW_PRED)
    {
        inst->numViews = 2;
        inst->numRefBuffsLum = 1;
        inst->numRefBuffsChr = 2;
        inst->seqParameterSet.numRefFrames = 1;
    }
    else if (pEncCfg->viewMode == H264ENC_MVC_STEREO_INTER_PRED)
    {
        inst->numViews = 2;
        inst->numRefBuffsLum = 2;
        inst->numRefBuffsChr = 3;
        inst->seqParameterSet.numRefFrames = 1;
    }
    else /*if(pEncCfg->viewMode == H264ENC_INTERLACED_FIELD)*/
    {
        inst->numViews = 1;
        inst->numRefBuffsLum = 2;
        inst->numRefBuffsChr = 3;
        inst->seqParameterSet.frameMbsOnly = ENCHW_NO;
        /* 2 ref frames will be buffered so each field will reference previous
           field with same parity (except for I-frame bottom referencing top) */
        inst->seqParameterSet.numRefFrames = 2;
        inst->interlaced = 1;
        /* Map unit 32-pixels high for fields */
        height = 32 * ((pEncCfg->height + 31) / 32);
    }

    /* Slice */
    inst->slice.sliceSize = 0;

    /* Macroblock */
    inst->mbPerRow = width / 16;
    inst->mbPerCol = height / (16 * (1 + inst->interlaced));
    inst->mbPerFrame = inst->mbPerRow * inst->mbPerCol;

    /* Disable intra and ROI areas by default */
    inst->asic.regs.intraAreaTop = inst->asic.regs.intraAreaBottom = inst->mbPerCol;
    inst->asic.regs.intraAreaLeft = inst->asic.regs.intraAreaRight = inst->mbPerRow;
    inst->asic.regs.roi1Top = inst->asic.regs.roi1Bottom = inst->mbPerCol;
    inst->asic.regs.roi1Left = inst->asic.regs.roi1Right = inst->mbPerRow;
    inst->asic.regs.roi2Top = inst->asic.regs.roi2Bottom = inst->mbPerCol;
    inst->asic.regs.roi2Left = inst->asic.regs.roi2Right = inst->mbPerRow;
    inst->preProcess.roiMapEnable = 0;
    inst->preProcess.qpOffset[0] = 0;
    inst->preProcess.qpOffset[1] = 0;
    inst->preProcess.qpOffset[2] = 0;

    /* Sequence parameter set */
    inst->seqParameterSet.levelIdc = pEncCfg->level;
    inst->seqParameterSet.picWidthInMbsMinus1 = width / 16 - 1;
    inst->seqParameterSet.picHeightInMapUnitsMinus1 = height / (16 * (1 + inst->interlaced)) - 1;

    /* Set cropping parameters if required */
    if (pEncCfg->width % 16 || pEncCfg->height % 16 ||
            (inst->interlaced && pEncCfg->height % 32))
    {
        inst->seqParameterSet.frameCropping = ENCHW_YES;
        inst->seqParameterSet.frameCropRightOffset = (width - pEncCfg->width) / 2 ;
        inst->seqParameterSet.frameCropBottomOffset = (height - pEncCfg->height) / 2;
    }

    /* Level 1b is indicated with levelIdc == 11 (later) and constraintSet3 */
    if (pEncCfg->level == H264ENC_LEVEL_1_b)
    {
        inst->seqParameterSet.constraintSet3 = ENCHW_YES;
    }

    /* Get the index for the table of level maximum values */
    tmp = H264GetLevelIndex(inst->seqParameterSet.levelIdc);
    if (tmp == INVALID_LEVEL)
        return ENCHW_NOK;

    inst->seqParameterSet.levelIdx = tmp;

#if 1   /* enforce maximum frame size in level */
    if (inst->mbPerFrame > H264MaxFS[inst->seqParameterSet.levelIdx])
    {
        return ENCHW_NOK;
    }
#endif

#if 0   /* enforce macroblock rate limit in level */
    {
        u32 mb_rate =
            (pEncCfg->frameRateNum * inst->mbPerFrame) /
            pEncCfg->frameRateDenom;

        if (mb_rate > H264MaxMBPS[inst->seqParameterSet.levelIdx])
        {
            return ENCHW_NOK;
        }
    }
#endif

    /* Picture parameter set */
    inst->picParameterSet.picInitQpMinus26 = (i32) H264ENC_DEFAULT_QP - 26;

    /* CABAC enabled by default */
    inst->picParameterSet.enableCabac = 1;

    /* Rate control setup */

    /* Maximum bitrate for the specified level */
    bps = H264MaxBR[inst->seqParameterSet.levelIdx];

    {
        h264RateControl_s *rc = &inst->rateControl;

        rc->outRateDenom = pEncCfg->frameRateDenom;
        rc->outRateNum = pEncCfg->frameRateNum;
        rc->mbPerPic = (width / 16) * (height / 16);
        rc->mbRows = height / 16;

        {
            h264VirtualBuffer_s *vb = &rc->virtualBuffer;

            vb->bitRate = bps;
            vb->unitsInTic = pEncCfg->frameRateDenom;
            vb->timeScale = pEncCfg->frameRateNum;
            vb->bufferSize = H264MaxCPBS[inst->seqParameterSet.levelIdx];
        }

        rc->hrd = ENCHW_YES;
        rc->picRc = ENCHW_YES;
        rc->mbRc = ENCHW_NO;
        rc->picSkip = ENCHW_NO;

        rc->qpHdr = H264ENC_DEFAULT_QP << QP_FRACTIONAL_BITS;
        rc->qpMin = 10 << QP_FRACTIONAL_BITS;
        rc->qpMax = 51 << QP_FRACTIONAL_BITS;

        rc->frameCoded = ENCHW_YES;
        rc->sliceTypeCur = ISLICE;
        rc->sliceTypePrev = PSLICE;
        rc->gopLen = 150;

        /* Default initial value for intra QP delta */
        rc->intraQpDelta = -3 << QP_FRACTIONAL_BITS;
        rc->fixedIntraQp = 0 << QP_FRACTIONAL_BITS;
        /* default long-term pic rate */
        rc->longTermPicRate = 15;
        rc->mbQpAutoBoost = 0;
    }

    /* no SEI by default */
    inst->rateControl.sei.enabled = ENCHW_NO;

    /* Pre processing */
    inst->preProcess.lumWidth = pEncCfg->width;
    inst->preProcess.lumWidthSrc =
        H264GetAllowedWidth(pEncCfg->width, H264ENC_YUV420_PLANAR);

    inst->preProcess.lumHeight = pEncCfg->height;
    if (inst->interlaced) inst->preProcess.lumHeight = pEncCfg->height / 2;
    inst->preProcess.lumHeightSrc = pEncCfg->height;

    inst->preProcess.horOffsetSrc = 0;
    inst->preProcess.verOffsetSrc = 0;

    inst->preProcess.rotation = ROTATE_0;
    inst->preProcess.inputFormat = H264ENC_YUV420_PLANAR;
    inst->preProcess.videoStab = 0;
    inst->preProcess.scaledWidth    = pEncCfg->scaledWidth;
    inst->preProcess.scaledHeight   = pEncCfg->scaledHeight;

    /* Is HW scaling supported */
    if (cfg.scalingEnabled == EWL_HW_CONFIG_NOT_SUPPORTED)
        inst->preProcess.scaledWidth = inst->preProcess.scaledHeight = 0;

    inst->preProcess.scaledOutput   =
        (inst->preProcess.scaledWidth * inst->preProcess.scaledHeight ? 1 : 0);
    inst->preProcess.adaptiveRoi = 0;
    inst->preProcess.adaptiveRoiColor  = 0;
    inst->preProcess.adaptiveRoiMotion = -5;

    inst->preProcess.colorConversionType = 0;
    EncSetColorConversion(&inst->preProcess, &inst->asic);

    /* Level 1b is indicated with levelIdc == 11 (constraintSet3) */
    if (pEncCfg->level == H264ENC_LEVEL_1_b)
    {
        inst->seqParameterSet.levelIdc = 11;
    }

    /* reference frame compression */
    if ((pEncCfg->refFrameCompress)&&(inst->hwCfg.rfcSupport==0))
        return ENCHW_NOK;
      
    inst->asic.regs.refLumCompress = (pEncCfg->refFrameCompress&1) ? 1 : 0;
    inst->asic.regs.refChrCompress = (pEncCfg->refFrameCompress&2) ? 1 : 0;
    inst->asic.regs.rfcLumDiscreteMode = 1;
    
    if ((pEncCfg->viewMode == H264ENC_BASE_VIEW_DOUBLE_BUFFER) ||
        (pEncCfg->viewMode == H264ENC_BASE_VIEW_MULTI_BUFFER))
        inst->asic.regs.rfcLumDiscreteMode = 0;
    inst->asic.regs.rfcLumBufLimit = inst->mbPerCol*inst->mbPerRow*256*pEncCfg->rfcLumBufLimit/100;
    inst->asic.regs.rfcChrBufLimit = inst->mbPerCol*inst->mbPerRow*128*pEncCfg->rfcChrBufLimit/100;
    inst->asic.regs.rfcLumBufLimit /= 8;
    inst->asic.regs.rfcChrBufLimit /= 8;
    inst->asic.regs.rfcOverflowIRQEn = ENCH1_RFC_OVERFLOW_INTERRUPT&1;

    return ENCHW_OK;
}

/*------------------------------------------------------------------------------

    CheckParameter

------------------------------------------------------------------------------*/
bool_e CheckParameter(const h264Instance_s *inst)
{
    /* Check crop */
    if (EncPreProcessCheck(&inst->preProcess) != ENCHW_OK)
    {
        return ENCHW_NOK;
    }

    return ENCHW_OK;
}

/*------------------------------------------------------------------------------

    SetPictureBuffer

------------------------------------------------------------------------------*/
i32 SetPictureBuffer(h264Instance_s *inst)
{
    picBuffer *picBuffer = &inst->picBuffer;
    i32 width, height;

    width = inst->mbPerRow * 16;
    height = inst->mbPerCol * 16;
    if (H264PictureBufferAlloc(picBuffer, width, height) != ENCHW_OK)
        return ENCHW_NOK;

    return ENCHW_OK;
}

/*------------------------------------------------------------------------------

    Round the width to the next multiple of 8 or 16 depending on YUV type.

------------------------------------------------------------------------------*/
i32 H264GetAllowedWidth(i32 width, H264EncPictureType inputType)
{
    if (inputType == H264ENC_YUV420_PLANAR)
    {
        /* Width must be multiple of 16 to make
         * chrominance row 64-bit aligned */
        return ((width + 15) / 16) * 16;
    }
    else
    {
        /* H264ENC_YUV420_SEMIPLANAR */
        /* H264ENC_YUV422_INTERLEAVED_YUYV */
        /* H264ENC_YUV422_INTERLEAVED_UYVY */
        return ((width + 7) / 8) * 8;
    }
}

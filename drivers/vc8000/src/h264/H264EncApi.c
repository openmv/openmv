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
 *  Abstract : H264 Encoder API
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------
       Version Information
------------------------------------------------------------------------------*/

#define H264ENC_MAJOR_VERSION 5
#define H264ENC_MINOR_VERSION 0

#define H264ENC_BUILD_MAJOR 5
#define H264ENC_BUILD_MINOR 0
#define H264ENC_BUILD_REVISION 0
#define H264ENC_SW_BUILD ((H264ENC_BUILD_MAJOR * 1000000) + \
(H264ENC_BUILD_MINOR * 1000) + H264ENC_BUILD_REVISION)

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include <string.h>
#include "h264encapi.h"
#include "enccommon.h"
#include "H264Instance.h"
#include "H264Init.h"
#include "H264PutBits.h"
#include "H264CodeFrame.h"
#include "H264Sei.h"
#include "H264RateControl.h"
#include "H264Cabac.h"
#include "H264Denoise.h"
#include "encasiccontroller.h"

#ifdef INTERNAL_TEST
#include "H264TestId.h"
#endif

#ifdef VIDEOSTAB_ENABLED
#include "vidstabcommon.h"
#endif


/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

#define EVALUATION_LIMIT 300    max number of frames to encode

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/* Parameter limits */
#define H264ENCSTRMSTART_MIN_BUF        64
#define H264ENCSTRMENCODE_MIN_BUF       4096
#define H264ENC_MAX_PP_INPUT_WIDTH      8176
#define H264ENC_MAX_PP_INPUT_HEIGHT     8176
#define H264ENC_MAX_BITRATE             (50000*1200)    /* Level 4.1 limit */
#define H264ENC_MAX_USER_DATA_SIZE      2048

#define H264ENC_IDR_ID_MODULO           16

#define H264_BUS_ADDRESS_VALID(bus_address)  (((bus_address) != 0) && \
                                              ((bus_address & 0x07) == 0))

/* HW ID check. H264EncInit() will fail if HW doesn't match. */
#define HW_ID_MASK  0xFFFF0000
#define HW_ID       0x48310000
#define HW_ID_NE    0x6e650000


/* Tracing macro */
#ifdef H264ENC_TRACE
#define APITRACE(str) H264EncTrace(str)
#define APITRACEPARAM(str, val) \
  { char tmpstr[255]; sprintf(tmpstr, "  %s: %d", str, (int)val); H264EncTrace(tmpstr); }
#else
#define APITRACE(str)
#define APITRACEPARAM(str, val)
#endif

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/

static void H264AddNaluSize(H264EncOut * pEncOut, u32 naluSizeBytes);

static i32 VSCheckSize(u32 inputWidth, u32 inputHeight, u32 stabilizedWidth,
                       u32 stabilizedHeight);

static void H264PrefixNal(h264Instance_s *pEncInst, bool svcExtFlag);
static void H264SvcSeiNal(h264Instance_s *pEncInst);

/* Supply prefix for svct muti-slice. */
static void H264SupplyPrefixForSvctMutiSlice(H264EncInst inst, const H264EncIn *pEncIn,                 H264EncOut *pEncOut,
                                                             u32 prefix_nalu_length, u8 *prefix_nalu_addr);


/*------------------------------------------------------------------------------

    Function name : H264EncGetApiVersion
    Description   : Return the API version info

    Return type   : H264EncApiVersion
    Argument      : void
------------------------------------------------------------------------------*/
H264EncApiVersion H264EncGetApiVersion(void)
{
    H264EncApiVersion ver;

    ver.major = H264ENC_MAJOR_VERSION;
    ver.minor = H264ENC_MINOR_VERSION;

    APITRACE("H264EncGetApiVersion# OK");
    return ver;
}

/*------------------------------------------------------------------------------
    Function name : H264EncGetBuild
    Description   : Return the SW and HW build information

    Return type   : H264EncBuild
    Argument      : void
------------------------------------------------------------------------------*/
H264EncBuild H264EncGetBuild(void)
{
    H264EncBuild ver;

    ver.swBuild = H264ENC_SW_BUILD;
    ver.hwBuild = EWLReadAsicID();

    APITRACE("H264EncGetBuild# OK");

    return (ver);
}

/*------------------------------------------------------------------------------
    Function name : H264EncInit
    Description   : Initialize an encoder instance and returns it to application

    Return type   : H264EncRet
    Argument      : pEncCfg - initialization parameters
                    instAddr - where to save the created instance
------------------------------------------------------------------------------*/
H264EncRet H264EncInit(const H264EncConfig * pEncCfg, H264EncInst * instAddr)
{
    H264EncRet ret;
    h264Instance_s *pEncInst = NULL;

    APITRACE("H264EncInit#");
    APITRACEPARAM("streamType", pEncCfg->streamType);
    APITRACEPARAM("viewMode", pEncCfg->viewMode);
    APITRACEPARAM("level", pEncCfg->level);
    APITRACEPARAM("refFrameAmount", pEncCfg->refFrameAmount);
    APITRACEPARAM("refFrameCompress", pEncCfg->refFrameCompress);
    APITRACEPARAM("width", pEncCfg->width);
    APITRACEPARAM("height", pEncCfg->height);
    APITRACEPARAM("frameRateNum", pEncCfg->frameRateNum);
    APITRACEPARAM("frameRateDenom", pEncCfg->frameRateDenom);
    APITRACEPARAM("scaledWidth", pEncCfg->scaledWidth);
    APITRACEPARAM("scaledHeight", pEncCfg->scaledHeight);
    APITRACEPARAM("scvtLevel", pEncCfg->svctLevel);

    /* check that right shift on negative numbers is performed signed */
    /*lint -save -e* following check causes multiple lint messages */
#if (((-1) >> 1) != (-1))
#error Right bit-shifting (>>) does not preserve the sign
#endif
    /*lint -restore */

    /* Check for illegal inputs */
    if(pEncCfg == NULL || instAddr == NULL)
    {
        APITRACE("H264EncInit: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    /* Check for correct HW */
    if ((EWLReadAsicID() & HW_ID_MASK) != HW_ID && (EWLReadAsicID() & HW_ID_MASK) != HW_ID_NE)
    {
        APITRACE("H264EncInit: ERROR Invalid HW ID");
        return H264ENC_ERROR;
    }

    /* Check that configuration is valid */
    if(H264CheckCfg(pEncCfg) == ENCHW_NOK)
    {
        APITRACE("H264EncInit: ERROR Invalid configuration");
        return H264ENC_INVALID_ARGUMENT;
    }

    /* Initialize encoder instance and allocate memories */
    ret = H264Init(pEncCfg, &pEncInst);

    if(ret != H264ENC_OK)
    {
        APITRACE("H264EncInit: ERROR Initialization failed");
        return ret;
    }

    /* Status == INIT   Initialization succesful */
    pEncInst->encStatus = H264ENCSTAT_INIT;

    pEncInst->inst = pEncInst;  /* used as checksum */

    *instAddr = (H264EncInst) pEncInst;

    APITRACE("H264EncInit: OK");
    return H264ENC_OK;
}

/*------------------------------------------------------------------------------

    Function name : H264EncRelease
    Description   : Releases encoder instance and all associated resource

    Return type   : H264EncRet
    Argument      : inst - the instance to be released
------------------------------------------------------------------------------*/
H264EncRet H264EncRelease(H264EncInst inst)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;

    APITRACE("H264EncRelease#");

    /* Check for illegal inputs */
    if(pEncInst == NULL)
    {
        APITRACE("H264EncRelease: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("H264EncRelease: ERROR Invalid instance");
        return H264ENC_INSTANCE_ERROR;
    }

#ifdef TRACE_STREAM
    EncCloseStreamTrace();
#endif

    H264Shutdown(pEncInst);

    APITRACE("H264EncRelease: OK");
    return H264ENC_OK;
}

/*------------------------------------------------------------------------------

    Function name : H264EncSetCodingCtrl
    Description   : Sets encoding parameters

    Return type   : H264EncRet
    Argument      : inst - the instance in use
                    pCodeParams - user provided parameters
------------------------------------------------------------------------------*/
H264EncRet H264EncSetCodingCtrl(H264EncInst inst,
                                const H264EncCodingCtrl * pCodeParams)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;
    regValues_s *regs = &pEncInst->asic.regs;

    APITRACE("H264EncSetCodingCtrl#");
    APITRACEPARAM("sliceSize", pCodeParams->sliceSize);
    APITRACEPARAM("seiMessages", pCodeParams->seiMessages);
    APITRACEPARAM("idrHeader", pCodeParams->idrHeader);
    APITRACEPARAM("videoFullRange", pCodeParams->videoFullRange);
    APITRACEPARAM("constrainedIntraPrediction", pCodeParams->constrainedIntraPrediction);
    APITRACEPARAM("disableDeblockingFilter", pCodeParams->disableDeblockingFilter);
    APITRACEPARAM("sampleAspectRatioWidth", pCodeParams->sampleAspectRatioWidth);
    APITRACEPARAM("sampleAspectRatioHeight", pCodeParams->sampleAspectRatioHeight);
    APITRACEPARAM("enableCabac", pCodeParams->enableCabac);
    APITRACEPARAM("cabacInitIdc", pCodeParams->cabacInitIdc);
    APITRACEPARAM("transform8x8Mode", pCodeParams->transform8x8Mode);
    APITRACEPARAM("quarterPixelMv", pCodeParams->quarterPixelMv);
    APITRACEPARAM("cirStart", pCodeParams->cirStart);
    APITRACEPARAM("cirInterval", pCodeParams->cirInterval);
    APITRACEPARAM("intraSliceMap1", pCodeParams->intraSliceMap1);
    APITRACEPARAM("intraSliceMap2", pCodeParams->intraSliceMap2);
    APITRACEPARAM("intraSliceMap3", pCodeParams->intraSliceMap3);
    APITRACEPARAM("intraArea.enable", pCodeParams->intraArea.enable);
    APITRACEPARAM("intraArea.top", pCodeParams->intraArea.top);
    APITRACEPARAM("intraArea.bottom", pCodeParams->intraArea.bottom);
    APITRACEPARAM("intraArea.left", pCodeParams->intraArea.left);
    APITRACEPARAM("intraArea.right", pCodeParams->intraArea.right);
    APITRACEPARAM("roi1Area.enable", pCodeParams->roi1Area.enable);
    APITRACEPARAM("roi1Area.top", pCodeParams->roi1Area.top);
    APITRACEPARAM("roi1Area.bottom", pCodeParams->roi1Area.bottom);
    APITRACEPARAM("roi1Area.left", pCodeParams->roi1Area.left);
    APITRACEPARAM("roi1Area.right", pCodeParams->roi1Area.right);
    APITRACEPARAM("roi2Area.enable", pCodeParams->roi2Area.enable);
    APITRACEPARAM("roi2Area.top", pCodeParams->roi2Area.top);
    APITRACEPARAM("roi2Area.bottom", pCodeParams->roi2Area.bottom);
    APITRACEPARAM("roi2Area.left", pCodeParams->roi2Area.left);
    APITRACEPARAM("roi2Area.right", pCodeParams->roi2Area.right);
    APITRACEPARAM("roi1DeltaQp", pCodeParams->roi1DeltaQp);
    APITRACEPARAM("roi2DeltaQp", pCodeParams->roi2DeltaQp);
    APITRACEPARAM("adaptiveRoi", pCodeParams->adaptiveRoi);
    APITRACEPARAM("adaptiveRoiColor", pCodeParams->adaptiveRoiColor);
    APITRACEPARAM("fieldOrder", pCodeParams->fieldOrder);
    APITRACEPARAM("gdrDuration", pCodeParams->gdrDuration);
    APITRACEPARAM("roiMapEnable", pCodeParams->roiMapEnable);

    /* Check for illegal inputs */
    if((pEncInst == NULL) || (pCodeParams == NULL))
    {
        APITRACE("H264EncSetCodingCtrl: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("H264EncSetCodingCtrl: ERROR Invalid instance");
        return H264ENC_INSTANCE_ERROR;
    }

    /* Check for invalid values */
    if(pCodeParams->sliceSize > pEncInst->mbPerCol)
    {
        APITRACE("H264EncSetCodingCtrl: ERROR Invalid sliceSize");
        return H264ENC_INVALID_ARGUMENT;
    }

    if(pCodeParams->cirStart > pEncInst->mbPerFrame ||
       pCodeParams->cirInterval > pEncInst->mbPerFrame)
    {
        APITRACE("H264EncSetCodingCtrl: ERROR Invalid CIR value");
        return H264ENC_INVALID_ARGUMENT;
    }

    if(pCodeParams->intraArea.enable) {
        if(!(pCodeParams->intraArea.top <= pCodeParams->intraArea.bottom &&
           pCodeParams->intraArea.bottom < pEncInst->mbPerCol &&
           pCodeParams->intraArea.left <= pCodeParams->intraArea.right &&
           pCodeParams->intraArea.right < pEncInst->mbPerRow) || (pCodeParams->gdrDuration > 0))
        {
            APITRACE("H264EncSetCodingCtrl: ERROR Invalid intraArea");
            return H264ENC_INVALID_ARGUMENT;
        }
    }

    if(pCodeParams->roi1Area.enable) {
        if(!(pCodeParams->roi1Area.top <= pCodeParams->roi1Area.bottom &&
           pCodeParams->roi1Area.bottom < pEncInst->mbPerCol &&
           pCodeParams->roi1Area.left <= pCodeParams->roi1Area.right &&
           pCodeParams->roi1Area.right < pEncInst->mbPerRow) || (pCodeParams->gdrDuration > 0))
        {
            APITRACE("H264EncSetCodingCtrl: ERROR Invalid roi1Area");
            return H264ENC_INVALID_ARGUMENT;
        }
    }

    if(pCodeParams->roi2Area.enable) {
        if(!(pCodeParams->roi2Area.top <= pCodeParams->roi2Area.bottom &&
           pCodeParams->roi2Area.bottom < pEncInst->mbPerCol &&
           pCodeParams->roi2Area.left <= pCodeParams->roi2Area.right &&
           pCodeParams->roi2Area.right < pEncInst->mbPerRow))
        {
            APITRACE("H264EncSetCodingCtrl: ERROR Invalid roi2Area");
            return H264ENC_INVALID_ARGUMENT;
        }
    }

    if(pCodeParams->roi1DeltaQp < -15 ||
       pCodeParams->roi1DeltaQp > 0 ||
       pCodeParams->roi2DeltaQp < -15 ||
       pCodeParams->roi2DeltaQp > 0 ||
       pCodeParams->adaptiveRoi < -51 ||
       pCodeParams->adaptiveRoi > 0)
    {
        APITRACE("H264EncSetCodingCtrl: ERROR Invalid ROI delta QP");
        return H264ENC_INVALID_ARGUMENT;
    }

    if(pCodeParams->qpOffset[0] < -8 ||
       pCodeParams->qpOffset[0] > 7 ||
       pCodeParams->qpOffset[1] < -127 ||
       pCodeParams->qpOffset[1] > 127 ||
       pCodeParams->qpOffset[2] < -127 ||
       pCodeParams->qpOffset[2] > 127)
    {
        APITRACE("H264EncSetCodingCtrl: ERROR Invalid ROI delta QP for ROI MAP");
        return H264ENC_INVALID_ARGUMENT;
    }

   /* Check status, only slice size, CIR & ROI allowed to change after start */
    if(pEncInst->encStatus != H264ENCSTAT_INIT)
    {
        goto set_slice_size;
    }

    if(pCodeParams->constrainedIntraPrediction > 1 ||
       pCodeParams->disableDeblockingFilter > 2 ||
       pCodeParams->enableCabac > 2 ||
       pCodeParams->transform8x8Mode > 2 ||
       pCodeParams->quarterPixelMv > 2 ||
       pCodeParams->seiMessages > 1 ||
       pCodeParams->idrHeader > 1 ||
       pCodeParams->videoFullRange > 1)
    {
        APITRACE("H264EncSetCodingCtrl: ERROR Invalid enable/disable");
        return H264ENC_INVALID_ARGUMENT;
    }

    if(pCodeParams->sampleAspectRatioWidth > 65535 ||
       pCodeParams->sampleAspectRatioHeight > 65535)
    {
        APITRACE("H264EncSetCodingCtrl: ERROR Invalid sampleAspectRatio");
        return H264ENC_INVALID_ARGUMENT;
    }

    if(pCodeParams->cabacInitIdc > 2)
    {
        APITRACE("H264EncSetCodingCtrl: ERROR Invalid cabacInitIdc");
        return H264ENC_INVALID_ARGUMENT;
    }

    /* Configure the values */
    if(pCodeParams->constrainedIntraPrediction == 0)
        pEncInst->picParameterSet.constIntraPred = ENCHW_NO;
    else
        pEncInst->picParameterSet.constIntraPred = ENCHW_YES;

    /* filter control header always present */
    pEncInst->picParameterSet.deblockingFilterControlPresent = ENCHW_YES;
    pEncInst->slice.disableDeblocking = pCodeParams->disableDeblockingFilter;

    pEncInst->picParameterSet.enableCabac = pCodeParams->enableCabac;
    pEncInst->slice.cabacInitIdc = pCodeParams->cabacInitIdc;

    /* cavlc -> long-term ref pics disabled */
    /* exception: when svct is enable, it use only 1 reference. */
    if ( pEncInst->svc.level == 0 && pCodeParams->enableCabac == 0 &&
        pEncInst->seqParameterSet.numRefFrames > 1 && !pEncInst->interlaced)
    {
        pEncInst->seqParameterSet.numRefFrames = 1;
        pEncInst->picBuffer.size = 1;
    }

    if(((pCodeParams->gdrDuration > 0) &&
        (pEncInst->seqParameterSet.numRefFrames > 1) &&
        (!pEncInst->interlaced)))
    {
        APITRACE("H264EncSetCodingCtrl: ERROR Invalid gdr parameters");
        return H264ENC_INVALID_ARGUMENT;
    }

    /* SVCT check */
    if (pEncInst->svc.level)
    {
        /* check HW support */
        if (pEncInst->hwCfg.svctSupport==0)
            return ENCHW_NOK;

        if (pCodeParams->gdrDuration)
        {
            APITRACE("H264EncSetCodingCtrl: ERROR Cannot enable GDR  for SVCT");
            return H264ENC_INVALID_ARGUMENT;
        }
        if (pEncInst->interlaced)
        {
            /* TODO: this limitation may be removed later */
            APITRACE("H264EncSetCodingCtrl: ERROR Support interlace  when enable SVCT");
            return H264ENC_INVALID_ARGUMENT;
        }
        if (pEncInst->numViews>1)
        {
            /* TODO: this limitation may be removed later */
            APITRACE("H264EncSetCodingCtrl: ERROR Support MVC  when enable SVCT");
            return H264ENC_INVALID_ARGUMENT;
        }
    }


    /* 8x8 mode: 2=enable, 1=adaptive (720p or bigger frame) */
    if ((pCodeParams->transform8x8Mode == 2) ||
        ((pCodeParams->transform8x8Mode == 1) &&
         (pEncInst->mbPerFrame >= 3600)))
    {
        pEncInst->picParameterSet.transform8x8Mode = ENCHW_YES;
    }

    /* check HW support */
    if ((pCodeParams->noiseReductionEnable)&&(pEncInst->hwCfg.dnfSupport==0))
        return ENCHW_NOK;

    H264EncDnfSetParameters(pEncInst, pCodeParams);

    H264SpsSetVuiAspectRatio(&pEncInst->seqParameterSet,
                             pCodeParams->sampleAspectRatioWidth,
                             pCodeParams->sampleAspectRatioHeight);

    H264SpsSetVuiVideoInfo(&pEncInst->seqParameterSet,
                           pCodeParams->videoFullRange);

    /* SEI messages are written in the beginning of each frame */
    if(pCodeParams->seiMessages)
        pEncInst->rateControl.sei.enabled = ENCHW_YES;
    else
        pEncInst->rateControl.sei.enabled = ENCHW_NO;

  set_slice_size:
    /* Slice size is set in macroblock rows => convert to macroblocks */
    pEncInst->slice.sliceSize = pCodeParams->sliceSize * pEncInst->mbPerRow;
    pEncInst->slice.fieldOrder = pCodeParams->fieldOrder ? 1 : 0;

    pEncInst->slice.quarterPixelMv = pCodeParams->quarterPixelMv;

    /* SPS/PPSheader written to each IDR or in the beginning of sequence */
    if(pCodeParams->idrHeader)
        pEncInst->idrHdr = ENCHW_YES;
    else
        pEncInst->idrHdr = ENCHW_NO;

    /* Set CIR, intra forcing and ROI parameters */
    regs->cirStart = pCodeParams->cirStart;
    regs->cirInterval = pCodeParams->cirInterval;
    pEncInst->intraSliceMap[0] = pCodeParams->intraSliceMap1;
    pEncInst->intraSliceMap[1] = pCodeParams->intraSliceMap2;
    pEncInst->intraSliceMap[2] = pCodeParams->intraSliceMap3;
    if (pCodeParams->intraArea.enable) {
        regs->intraAreaTop = pCodeParams->intraArea.top;
        regs->intraAreaLeft = pCodeParams->intraArea.left;
        regs->intraAreaBottom = pCodeParams->intraArea.bottom;
        regs->intraAreaRight = pCodeParams->intraArea.right;
    } else {
        regs->intraAreaTop = regs->intraAreaLeft = regs->intraAreaBottom =
        regs->intraAreaRight = 255;
    }
    if (pCodeParams->roi1Area.enable) {
        regs->roi1Top = pCodeParams->roi1Area.top;
        regs->roi1Left = pCodeParams->roi1Area.left;
        regs->roi1Bottom = pCodeParams->roi1Area.bottom;
        regs->roi1Right = pCodeParams->roi1Area.right;
    } else {
        regs->roi1Top = regs->roi1Left = regs->roi1Bottom =
        regs->roi1Right = 255;
    }
    if (pCodeParams->roi2Area.enable) {
        regs->roi2Top = pCodeParams->roi2Area.top;
        regs->roi2Left = pCodeParams->roi2Area.left;
        regs->roi2Bottom = pCodeParams->roi2Area.bottom;
        regs->roi2Right = pCodeParams->roi2Area.right;
    } else {
        regs->roi2Top = regs->roi2Left = regs->roi2Bottom =
        regs->roi2Right = 255;
    }
    regs->roi1DeltaQp = -pCodeParams->roi1DeltaQp;
    regs->roi2DeltaQp = -pCodeParams->roi2DeltaQp;
    regs->roiUpdate   = 1;    /* ROI has changed from previous frame. */

    pEncInst->preProcess.roiMapEnable = pCodeParams->roiMapEnable;
    pEncInst->preProcess.adaptiveRoi = pCodeParams->adaptiveRoi;
    pEncInst->preProcess.adaptiveRoiColor =
                                CLIP3(pCodeParams->adaptiveRoiColor, -10, 10);
    if (pCodeParams->roiMapEnable)
    {
        if ((pEncInst->preProcess.adaptiveRoi)||(pCodeParams->roi1Area.enable)
            ||(pCodeParams->roi2Area.enable))
        {
            APITRACE("H264EncSetCodingCtrl: ERROR Invalid ROI feature");
            return H264ENC_INVALID_ARGUMENT;
        }
        /* set the roi1,roi2 region registers as invalid. */
        regs->roi1Top = regs->roi1Left = regs->roi1Bottom = regs->roi1Right = 255;
        regs->roi2Top = regs->roi2Left = regs->roi2Bottom = regs->roi2Right = 255;
        pEncInst->preProcess.qpOffset[0] = pCodeParams->qpOffset[0];
        pEncInst->preProcess.qpOffset[1] = pCodeParams->qpOffset[1];
        pEncInst->preProcess.qpOffset[2] = pCodeParams->qpOffset[2];
    }

    pEncInst->rateControl.sei.insertRecoveryPointMessage = ENCHW_NO;
    pEncInst->rateControl.sei.recoveryFrameCnt = pCodeParams->gdrDuration;
    if (pEncInst->encStatus < H264ENCSTAT_START_FRAME)
    {
        pEncInst->gdrFirstIntraFrame = (1 + pEncInst->interlaced);
    }
    pEncInst->gdrEnabled = (pCodeParams->gdrDuration > 0);
    pEncInst->gdrDuration = pCodeParams->gdrDuration;
    if (pEncInst->gdrEnabled == 1)
    {
        pEncInst->gdrStart = 0;
        pEncInst->gdrCount = 0;
#ifndef SEARCH_AREA_SMALL
        pEncInst->gdrAverageMBRows = (pEncInst->mbPerCol-2) / pEncInst->gdrDuration;
        pEncInst->gdrMBLeft = pEncInst->mbPerCol - 2 - pEncInst->gdrAverageMBRows * pEncInst->gdrDuration;
#else
        pEncInst->gdrAverageMBRows = (pEncInst->mbPerCol - 1) / pEncInst->gdrDuration;
        pEncInst->gdrMBLeft = pEncInst->mbPerCol - 1 - pEncInst->gdrAverageMBRows * pEncInst->gdrDuration;
#endif
        if(pEncInst->gdrAverageMBRows == 0)
        {
            pEncInst->rateControl.sei.recoveryFrameCnt = pEncInst->gdrMBLeft;
            pEncInst->gdrDuration = pEncInst->gdrMBLeft;
        }
    }

    /* Input Line buffer check */
    if (pCodeParams->inputLineBufEn)
    {
        /* check HW support */
        if (pEncInst->hwCfg.instantSupport==0)
            return ENCHW_NOK;

        /* check zero depth */
        if ((pCodeParams->inputLineBufDepth == 0) &&
           (pCodeParams->inputLineBufLoopBackEn || pCodeParams->inputLineBufHwModeEn))
        {
            APITRACE("H264EncSetCodingCtrl: Invalid input buffer depth");
            return ENCHW_NOK;
        }

        /* check depth*mb_width align to 4 */
        if ((pEncInst->mbPerRow * pCodeParams->inputLineBufDepth) & 3)
        {
           APITRACE("H264EncSetCodingCtrl: Invalid input buffer depth");
           return ENCHW_NOK;
        }
    }
    pEncInst->inputLineBuf.inputLineBufEn = pCodeParams->inputLineBufEn;
    pEncInst->inputLineBuf.inputLineBufLoopBackEn = pCodeParams->inputLineBufLoopBackEn;
    pEncInst->inputLineBuf.inputLineBufDepth = pCodeParams->inputLineBufDepth;
    pEncInst->inputLineBuf.inputLineBufHwModeEn = pCodeParams->inputLineBufHwModeEn;

    pEncInst->svc.next_base_priority_id = pCodeParams->nBaseLayerPID;

    APITRACE("H264EncSetCodingCtrl: OK");
    return H264ENC_OK;
}

/*------------------------------------------------------------------------------

    Function name : H264EncGetCodingCtrl
    Description   : Returns current encoding parameters

    Return type   : H264EncRet
    Argument      : inst - the instance in use
                    pCodeParams - palce where parameters are returned
------------------------------------------------------------------------------*/
H264EncRet H264EncGetCodingCtrl(H264EncInst inst,
                                H264EncCodingCtrl * pCodeParams)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;
    regValues_s *regs = &pEncInst->asic.regs;

    APITRACE("H264EncGetCodingCtrl#");

    /* Check for illegal inputs */
    if((pEncInst == NULL) || (pCodeParams == NULL))
    {
        APITRACE("H264EncGetCodingCtrl: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("H264EncGetCodingCtrl: ERROR Invalid instance");
        return H264ENC_INSTANCE_ERROR;
    }

    /* Get values */
    pCodeParams->constrainedIntraPrediction =
        (pEncInst->picParameterSet.constIntraPred == ENCHW_NO) ? 0 : 1;

    /* Slice size from macroblocks to macroblock rows */
    pCodeParams->sliceSize = pEncInst->slice.sliceSize / pEncInst->mbPerRow;
    pCodeParams->fieldOrder = pEncInst->slice.fieldOrder;

    pCodeParams->seiMessages =
        (pEncInst->rateControl.sei.enabled == ENCHW_YES) ? 1 : 0;
    pCodeParams->idrHeader =
        (pEncInst->idrHdr == ENCHW_YES) ? 1 : 0;
    pCodeParams->videoFullRange =
        (pEncInst->seqParameterSet.vui.videoFullRange == ENCHW_YES) ? 1 : 0;
    pCodeParams->sampleAspectRatioWidth =
        pEncInst->seqParameterSet.vui.sarWidth;
    pCodeParams->sampleAspectRatioHeight =
        pEncInst->seqParameterSet.vui.sarHeight;
    pCodeParams->disableDeblockingFilter = pEncInst->slice.disableDeblocking;
    pCodeParams->enableCabac = pEncInst->picParameterSet.enableCabac;
    pCodeParams->cabacInitIdc = pEncInst->slice.cabacInitIdc;
    pCodeParams->transform8x8Mode = pEncInst->picParameterSet.transform8x8Mode;
    pCodeParams->quarterPixelMv = pEncInst->slice.quarterPixelMv;

    pCodeParams->cirStart = regs->cirStart;
    pCodeParams->cirInterval = regs->cirInterval;
    pCodeParams->intraSliceMap1 = pEncInst->intraSliceMap[0];
    pCodeParams->intraSliceMap2 = pEncInst->intraSliceMap[1];
    pCodeParams->intraSliceMap3 = pEncInst->intraSliceMap[2];
    pCodeParams->intraArea.enable = regs->intraAreaTop < pEncInst->mbPerCol ? 1 : 0;
    pCodeParams->intraArea.top    = regs->intraAreaTop;
    pCodeParams->intraArea.left   = regs->intraAreaLeft;
    pCodeParams->intraArea.bottom = regs->intraAreaBottom;
    pCodeParams->intraArea.right  = regs->intraAreaRight;
    pCodeParams->roi1Area.enable = regs->roi1Top < pEncInst->mbPerCol ? 1 : 0;
    pCodeParams->roi1Area.top    = regs->roi1Top;
    pCodeParams->roi1Area.left   = regs->roi1Left;
    pCodeParams->roi1Area.bottom = regs->roi1Bottom;
    pCodeParams->roi1Area.right  = regs->roi1Right;
    pCodeParams->roi2Area.enable = regs->roi2Top < pEncInst->mbPerCol ? 1 : 0;
    pCodeParams->roi2Area.top    = regs->roi2Top;
    pCodeParams->roi2Area.left   = regs->roi2Left;
    pCodeParams->roi2Area.bottom = regs->roi2Bottom;
    pCodeParams->roi2Area.right  = regs->roi2Right;
    pCodeParams->roi1DeltaQp = -regs->roi1DeltaQp;
    pCodeParams->roi2DeltaQp = -regs->roi2DeltaQp;
    pCodeParams->adaptiveRoi       = pEncInst->preProcess.adaptiveRoi;
    pCodeParams->adaptiveRoiColor  = pEncInst->preProcess.adaptiveRoiColor;
    pCodeParams->roiMapEnable = pEncInst->preProcess.roiMapEnable;
    pCodeParams->qpOffset[0] = pEncInst->preProcess.qpOffset[0];
    pCodeParams->qpOffset[1] = pEncInst->preProcess.qpOffset[1];
    pCodeParams->qpOffset[2] = pEncInst->preProcess.qpOffset[2];
    pCodeParams->gdrDuration = pEncInst->gdrEnabled ? pEncInst->gdrDuration : 0;

    H264EncDnfGetParameters(pEncInst, pCodeParams);

    pCodeParams->inputLineBufEn = pEncInst->inputLineBuf.inputLineBufEn;
    pCodeParams->inputLineBufLoopBackEn = pEncInst->inputLineBuf.inputLineBufLoopBackEn;
    pCodeParams->inputLineBufDepth = pEncInst->inputLineBuf.inputLineBufDepth;
    pCodeParams->inputLineBufHwModeEn = pEncInst->inputLineBuf.inputLineBufHwModeEn;

    pCodeParams->nBaseLayerPID = pEncInst->svc.base_priority_id;

    APITRACE("H264EncGetCodingCtrl: OK");
    return H264ENC_OK;
}

/*------------------------------------------------------------------------------

    Function name : H264EncSetRateCtrl
    Description   : Sets rate control parameters

    Return type   : H264EncRet
    Argument      : inst - the instance in use
                    pRateCtrl - user provided parameters
------------------------------------------------------------------------------*/
H264EncRet H264EncSetRateCtrl(H264EncInst inst,
                              const H264EncRateCtrl * pRateCtrl)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;
    h264RateControl_s *rc;
    i32 prevBitrate;

    u32 i, tmp;

    APITRACE("H264EncSetRateCtrl#");
    APITRACEPARAM("pictureRc", pRateCtrl->pictureRc);
    APITRACEPARAM("mbRc", pRateCtrl->mbRc);
    APITRACEPARAM("pictureSkip", pRateCtrl->pictureSkip);
    APITRACEPARAM("qpHdr", pRateCtrl->qpHdr);
    APITRACEPARAM("qpMin", pRateCtrl->qpMin);
    APITRACEPARAM("qpMax", pRateCtrl->qpMax);
    APITRACEPARAM("bitPerSecond", pRateCtrl->bitPerSecond);
    APITRACEPARAM("gopLen", pRateCtrl->gopLen);
    APITRACEPARAM("hrd", pRateCtrl->hrd);
    APITRACEPARAM("hrdCpbSize", pRateCtrl->hrdCpbSize);
    APITRACEPARAM("intraQpDelta", pRateCtrl->intraQpDelta);
    APITRACEPARAM("fixedIntraQp", pRateCtrl->fixedIntraQp);
    APITRACEPARAM("mbQpAdjustment", pRateCtrl->mbQpAdjustment);
    APITRACEPARAM("longTermPicRate", pRateCtrl->longTermPicRate);
    APITRACEPARAM("mbQpAutoBoost", pRateCtrl->mbQpAutoBoost);

    /* Check for illegal inputs */
    if((pEncInst == NULL) || (pRateCtrl == NULL))
    {
        APITRACE("H264EncSetRateCtrl: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("H264EncSetRateCtrl: ERROR Invalid instance");
        return H264ENC_INSTANCE_ERROR;
    }

    rc = &pEncInst->rateControl;

    /* after stream was started with HRD ON,
     * it is not allowed to change RC params */
    if(pEncInst->encStatus == H264ENCSTAT_START_FRAME && rc->hrd == ENCHW_YES)
    {
        APITRACE
            ("H264EncSetRateCtrl: ERROR Stream started with HRD ON. Not allowed to change any parameters");
        return H264ENC_INVALID_STATUS;
    }

    /* Check for invalid input values */
    if(pRateCtrl->pictureRc > 1 ||
       pRateCtrl->mbRc > 1 || pRateCtrl->pictureSkip > 1 || pRateCtrl->hrd > 1)
    {
        APITRACE("H264EncSetRateCtrl: ERROR Invalid enable/disable value");
        return H264ENC_INVALID_ARGUMENT;
    }

    if(pRateCtrl->qpHdr > 51 ||
       pRateCtrl->qpMin > 51 ||
       pRateCtrl->qpMax > 51 ||
       pRateCtrl->qpMax < pRateCtrl->qpMin)
    {
        APITRACE("H264EncSetRateCtrl: ERROR Invalid QP");
        return H264ENC_INVALID_ARGUMENT;
    }

    if((pRateCtrl->qpHdr != -1) &&
      (pRateCtrl->qpHdr < (i32)pRateCtrl->qpMin ||
       pRateCtrl->qpHdr > (i32)pRateCtrl->qpMax))
    {
        APITRACE("H264EncSetRateCtrl: ERROR QP out of range");
        return H264ENC_INVALID_ARGUMENT;
    }
    if((u32)(pRateCtrl->intraQpDelta + 51) > 102)
    {
        APITRACE("H264EncSetRateCtrl: ERROR intraQpDelta out of range");
        return H264ENC_INVALID_ARGUMENT;
    }
    if(pRateCtrl->fixedIntraQp > 51)
    {
        APITRACE("H264EncSetRateCtrl: ERROR fixedIntraQp out of range");
        return H264ENC_INVALID_ARGUMENT;
    }
    if(pRateCtrl->gopLen < 1 || pRateCtrl->gopLen > 300)
    {
        APITRACE("H264EncSetRateCtrl: ERROR Invalid GOP length");
        return H264ENC_INVALID_ARGUMENT;
    }

    /* Bitrate affects only when rate control is enabled */
    if((pRateCtrl->pictureRc ||
        pRateCtrl->pictureSkip || pRateCtrl->hrd) &&
       (pRateCtrl->bitPerSecond < 10000 ||
        pRateCtrl->bitPerSecond > H264ENC_MAX_BITRATE))
    {
        APITRACE("H264EncSetRateCtrl: ERROR Invalid bitPerSecond");
        return H264ENC_INVALID_ARGUMENT;
    }

    {
        u32 cpbSize = pRateCtrl->hrdCpbSize;
        u32 bps = pRateCtrl->bitPerSecond;
        u32 level = pEncInst->seqParameterSet.levelIdx;

        /* Limit maximum bitrate based on resolution and frame rate */
        /* Saturates really high settings */
        /* bits per unpacked frame */
        tmp = 3 * 8 * pEncInst->mbPerFrame * 256 / 2;
        /* bits per second */
        tmp = H264Calculate(tmp, rc->outRateNum, rc->outRateDenom);
        if (bps > (tmp / 2))
            bps = tmp / 2;

        if(cpbSize == 0)
            cpbSize = H264MaxCPBS[level];
        else if(cpbSize == (u32) (-1))
            cpbSize = bps;

        /* Limit minimum CPB size based on average bits per frame */
        tmp = H264Calculate(bps, rc->outRateDenom, rc->outRateNum);
        cpbSize = MAX(cpbSize, tmp);

        /* cpbSize must be rounded so it is exactly the size written in stream */
        i = 0;
        tmp = cpbSize;
        while (4095 < (tmp >> (4 + i++)));

        cpbSize = (tmp >> (4 + i)) << (4 + i);

        /* if HRD is ON we have to obay all its limits */
        if(pRateCtrl->hrd != 0)
        {
            if(cpbSize > H264MaxCPBS[level])
            {
                APITRACE
                    ("H264EncSetRateCtrl: ERROR. HRD is ON. hrdCpbSize higher than maximum allowed for stream level");
                return H264ENC_INVALID_ARGUMENT;
            }

            if(bps > H264MaxBR[level])
            {
                APITRACE
                    ("H264EncSetRateCtrl: ERROR. HRD is ON. bitPerSecond higher than maximum allowed for stream level");
                return H264ENC_INVALID_ARGUMENT;
            }
        }

        rc->virtualBuffer.bufferSize = cpbSize;

        /* Set the parameters to rate control */
        if(pRateCtrl->pictureRc != 0)
            rc->picRc = ENCHW_YES;
        else
            rc->picRc = ENCHW_NO;

        rc->mbRc = pRateCtrl->mbRc;

        if(pRateCtrl->pictureSkip != 0)
            rc->picSkip = ENCHW_YES;
        else
            rc->picSkip = ENCHW_NO;

        if(pRateCtrl->hrd != 0)
        {
            rc->hrd = ENCHW_YES;
            rc->picRc = ENCHW_YES;
            if(rc->mbRc == 0)
              rc->mbRc = 1;
        }
        else
            rc->hrd = ENCHW_NO;

        rc->qpHdr = pRateCtrl->qpHdr << QP_FRACTIONAL_BITS;
        rc->qpMin = pRateCtrl->qpMin << QP_FRACTIONAL_BITS;
        rc->qpMax = pRateCtrl->qpMax << QP_FRACTIONAL_BITS;
        prevBitrate = rc->virtualBuffer.bitRate;
        rc->virtualBuffer.bitRate = bps;
        rc->gopLen = pRateCtrl->gopLen;
    }
    rc->intraQpDelta = pRateCtrl->intraQpDelta << QP_FRACTIONAL_BITS;
    rc->fixedIntraQp = pRateCtrl->fixedIntraQp << QP_FRACTIONAL_BITS;
    rc->mbQpAdjustment[0] = pRateCtrl->mbQpAdjustment*2;
    rc->mbQpAdjustment[1] = pRateCtrl->mbQpAdjustment*3;
    rc->mbQpAdjustment[2] = pRateCtrl->mbQpAdjustment*4;
    rc->longTermPicRate = pRateCtrl->longTermPicRate;
    rc->mbQpAutoBoost = pRateCtrl->mbQpAutoBoost;

    if(rc->mbRc != 0)
    {
        rc->mbQpAutoBoost = 0;
        rc->mbQpAdjustment[0] = 0;
        rc->mbQpAdjustment[1] = 0;
        rc->mbQpAdjustment[2] = 0;
    }

    /* New parameters checked already so ignore return value.
     * Reset RC bit counters when changing bitrate. */
    (void) H264InitRc(rc, (pEncInst->encStatus == H264ENCSTAT_INIT) ||
                (rc->virtualBuffer.bitRate != prevBitrate));

    APITRACE("H264EncSetRateCtrl: OK");
    return H264ENC_OK;
}

/*------------------------------------------------------------------------------

    Function name : H264EncGetRateCtrl
    Description   : Return current rate control parameters

    Return type   : H264EncRet
    Argument      : inst - the instance in use
                    pRateCtrl - place where parameters are returned
------------------------------------------------------------------------------*/
H264EncRet H264EncGetRateCtrl(H264EncInst inst, H264EncRateCtrl * pRateCtrl)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;
    h264RateControl_s *rc;

    APITRACE("H264EncGetRateCtrl#");

    /* Check for illegal inputs */
    if((pEncInst == NULL) || (pRateCtrl == NULL))
    {
        APITRACE("H264EncGetRateCtrl: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("H264EncGetRateCtrl: ERROR Invalid instance");
        return H264ENC_INSTANCE_ERROR;
    }

    /* Get the values */
    rc = &pEncInst->rateControl;

    pRateCtrl->pictureRc = rc->picRc == ENCHW_NO ? 0 : 1;
    pRateCtrl->mbRc = rc->mbRc;
    pRateCtrl->pictureSkip = rc->picSkip == ENCHW_NO ? 0 : 1;
    pRateCtrl->qpHdr = rc->qpHdr >> QP_FRACTIONAL_BITS;
    pRateCtrl->qpMin = rc->qpMin >> QP_FRACTIONAL_BITS;
    pRateCtrl->qpMax = rc->qpMax >> QP_FRACTIONAL_BITS;
    pRateCtrl->bitPerSecond = rc->virtualBuffer.bitRate;
    pRateCtrl->hrd = rc->hrd == ENCHW_NO ? 0 : 1;
    pRateCtrl->gopLen = rc->gopLen;

    pRateCtrl->hrdCpbSize = (u32) rc->virtualBuffer.bufferSize;
    pRateCtrl->intraQpDelta = rc->intraQpDelta >> QP_FRACTIONAL_BITS;
    pRateCtrl->fixedIntraQp = rc->fixedIntraQp >> QP_FRACTIONAL_BITS;
    pRateCtrl->mbQpAdjustment = rc->mbQpAdjustment[0]/2;
    pRateCtrl->longTermPicRate = rc->longTermPicRate;
    pRateCtrl->mbQpAutoBoost = rc->mbQpAutoBoost;

    APITRACE("H264EncGetRateCtrl: OK");
    return H264ENC_OK;
}

/*------------------------------------------------------------------------------
    Function name   : VSCheckSize
    Description     :
    Return type     : i32
    Argument        : u32 inputWidth
    Argument        : u32 inputHeight
    Argument        : u32 stabilizedWidth
    Argument        : u32 stabilizedHeight
------------------------------------------------------------------------------*/
i32 VSCheckSize(u32 inputWidth, u32 inputHeight, u32 stabilizedWidth,
                u32 stabilizedHeight)
{
    /* Input picture minimum dimensions */
    if((inputWidth < 104) || (inputHeight < 104))
        return 1;

    /* Stabilized picture minimum  values */
    if((stabilizedWidth < 96) || (stabilizedHeight < 96))
        return 1;

    /* Stabilized dimensions multiple of 4 */
    if(((stabilizedWidth & 3) != 0) || ((stabilizedHeight & 3) != 0))
        return 1;

    /* Edge >= 4 pixels, not checked because stabilization can be
     * used without cropping for scene detection
    if((inputWidth < (stabilizedWidth + 8)) ||
       (inputHeight < (stabilizedHeight + 8)))
        return 1; */

    return 0;
}

/*------------------------------------------------------------------------------
    Function name   : H264EncSetPreProcessing
    Description     : Sets the preprocessing parameters
    Return type     : H264EncRet
    Argument        : inst - encoder instance in use
    Argument        : pPreProcCfg - user provided parameters
------------------------------------------------------------------------------*/
H264EncRet H264EncSetPreProcessing(H264EncInst inst,
                                   const H264EncPreProcessingCfg * pPreProcCfg)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;
    preProcess_s pp_tmp;

    APITRACE("H264EncSetPreProcessing#");
    APITRACEPARAM("origWidth", pPreProcCfg->origWidth);
    APITRACEPARAM("origHeight", pPreProcCfg->origHeight);
    APITRACEPARAM("xOffset", pPreProcCfg->xOffset);
    APITRACEPARAM("yOffset", pPreProcCfg->yOffset);
    APITRACEPARAM("inputType", pPreProcCfg->inputType);
    APITRACEPARAM("rotation", pPreProcCfg->rotation);
    APITRACEPARAM("videoStabilization", pPreProcCfg->videoStabilization);
    APITRACEPARAM("colorConversion.type", pPreProcCfg->colorConversion.type);
    APITRACEPARAM("colorConversion.coeffA", pPreProcCfg->colorConversion.coeffA);
    APITRACEPARAM("colorConversion.coeffB", pPreProcCfg->colorConversion.coeffB);
    APITRACEPARAM("colorConversion.coeffC", pPreProcCfg->colorConversion.coeffC);
    APITRACEPARAM("colorConversion.coeffE", pPreProcCfg->colorConversion.coeffE);
    APITRACEPARAM("colorConversion.coeffF", pPreProcCfg->colorConversion.coeffF);
    APITRACEPARAM("scaledOutput", pPreProcCfg->scaledOutput);
    APITRACEPARAM("interlacedFrame", pPreProcCfg->interlacedFrame);

    /* Check for illegal inputs */
    if((inst == NULL) || (pPreProcCfg == NULL))
    {
        APITRACE("H264EncSetPreProcessing: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("H264EncSetPreProcessing: ERROR Invalid instance");
        return H264ENC_INSTANCE_ERROR;
    }

    /* check HW limitations */
    {
        EWLHwConfig_t cfg = EWLReadAsicConfig();

        /* is video stabilization supported? */
        if(cfg.vsEnabled == EWL_HW_CONFIG_NOT_SUPPORTED &&
           pPreProcCfg->videoStabilization != 0)
        {
            APITRACE("H264EncSetPreProcessing: ERROR Stabilization not supported");
            return H264ENC_INVALID_ARGUMENT;
        }
        if(cfg.rgbEnabled == EWL_HW_CONFIG_NOT_SUPPORTED &&
           pPreProcCfg->inputType >= H264ENC_RGB565 &&
           pPreProcCfg->inputType <= H264ENC_BGR101010)
        {
            APITRACE("H264EncSetPreProcessing: ERROR RGB input not supported");
            return H264ENC_INVALID_ARGUMENT;
        }
        if(cfg.scalingEnabled == EWL_HW_CONFIG_NOT_SUPPORTED &&
           pPreProcCfg->scaledOutput != 0)
        {
            APITRACE("H264EncSetPreProcessing: WARNING Scaling not supported, disabling output");
        }
    }

    if(pPreProcCfg->origWidth > H264ENC_MAX_PP_INPUT_WIDTH ||
       pPreProcCfg->origHeight > H264ENC_MAX_PP_INPUT_HEIGHT)
    {
        APITRACE("H264EncSetPreProcessing: ERROR Too big input image");
        return H264ENC_INVALID_ARGUMENT;
    }

    if(pPreProcCfg->inputType > H264ENC_P010)
    {
        APITRACE("H264EncSetPreProcessing: ERROR Invalid YUV type");
        return H264ENC_INVALID_ARGUMENT;
    }

    if(pPreProcCfg->rotation > H264ENC_ROTATE_180R)
    {
        APITRACE("H264EncSetPreProcessing: ERROR Invalid rotation");
        return H264ENC_INVALID_ARGUMENT;
    }

    if(pEncInst->interlaced && pPreProcCfg->rotation)
    {
        APITRACE("H264EncSetPreProcessing: ERROR Rotation and interlace");
        return H264ENC_INVALID_ARGUMENT;
    }

    /* Input Line buffer is not supported for stabilization*/
    if(pEncInst->inputLineBuf.inputLineBufEn && pPreProcCfg->videoStabilization)
    {
         APITRACE("H264EncSetPreProcessing: Input Line buffer is not supportted for stabilization feature");
         return H264ENC_INVALID_ARGUMENT;
    }

    /* Input Line buffer is not supported rotation */
    if(pEncInst->inputLineBuf.inputLineBufEn && pPreProcCfg->rotation)
    {
         APITRACE("H264EncSetPreProcessing: ERROR Rotation and Line Buffer Mode");
         return H264ENC_INVALID_ARGUMENT;
    }

    /* Encoded frame resolution as set in Init() */
    pp_tmp = pEncInst->preProcess;

    if(pPreProcCfg->videoStabilization == 0) {
        pp_tmp.horOffsetSrc = pPreProcCfg->xOffset;
        pp_tmp.verOffsetSrc = pPreProcCfg->yOffset;
    } else {
        pp_tmp.horOffsetSrc = pp_tmp.verOffsetSrc = 0;
    }

    pp_tmp.lumWidthSrc  = pPreProcCfg->origWidth;
    pp_tmp.lumHeightSrc = pPreProcCfg->origHeight;
    pp_tmp.rotation     = pPreProcCfg->rotation;
    pp_tmp.inputFormat  = pPreProcCfg->inputType;
    pp_tmp.videoStab    = (pPreProcCfg->videoStabilization != 0) ? 1 : 0;
    pp_tmp.interlacedFrame = (pPreProcCfg->interlacedFrame != 0) ? 1 : 0;

    pp_tmp.scaledOutput = (pPreProcCfg->scaledOutput) ? 1 : 0;
    if (pEncInst->preProcess.scaledWidth*pEncInst->preProcess.scaledHeight == 0)
        pp_tmp.scaledOutput = 0;

    /* Check for invalid values */
    if(EncPreProcessCheck(&pp_tmp) != ENCHW_OK)
    {
        APITRACE("H264EncSetPreProcessing: ERROR Invalid cropping values");
        return H264ENC_INVALID_ARGUMENT;
    }

    /* Set cropping parameters if required */
    if( pEncInst->preProcess.lumWidth%16 || pEncInst->preProcess.lumHeight%16 )
    {
        u32 fillRight = (pEncInst->preProcess.lumWidth+15)/16*16 -
                            pEncInst->preProcess.lumWidth;
        u32 fillBottom = (pEncInst->preProcess.lumHeight+15)/16*16 -
                            pEncInst->preProcess.lumHeight;

        pEncInst->seqParameterSet.frameCropping = ENCHW_YES;
        pEncInst->seqParameterSet.frameCropLeftOffset = 0;
        pEncInst->seqParameterSet.frameCropRightOffset = 0;
        pEncInst->seqParameterSet.frameCropTopOffset = 0;
        pEncInst->seqParameterSet.frameCropBottomOffset = 0;

        if (pPreProcCfg->rotation == 0) {   /* No rotation */
            pEncInst->seqParameterSet.frameCropRightOffset = fillRight/2;
            pEncInst->seqParameterSet.frameCropBottomOffset = fillBottom/2;
        } else if (pPreProcCfg->rotation == 1) {    /* Rotate right */
            pEncInst->seqParameterSet.frameCropLeftOffset = fillRight/2;
            pEncInst->seqParameterSet.frameCropBottomOffset = fillBottom/2;
        } else if (pPreProcCfg->rotation == 2) {    /* Rotate left */
            pEncInst->seqParameterSet.frameCropRightOffset = fillRight/2;
            pEncInst->seqParameterSet.frameCropTopOffset = fillBottom/2;
        } else if (pPreProcCfg->rotation == 3) {    /* Rotate left */
            pEncInst->seqParameterSet.frameCropTopOffset = fillBottom/2;
            pEncInst->seqParameterSet.frameCropLeftOffset = fillRight/2;
        }
    }

    if(pp_tmp.videoStab != 0)
    {
        u32 width = pp_tmp.lumWidth;
        u32 height = pp_tmp.lumHeight;
        u32 heightSrc = pp_tmp.lumHeightSrc;

        if(pp_tmp.rotation == 1 || pp_tmp.rotation == 2)
        {
            u32 tmp;

            tmp = width;
            width = height;
            height = tmp;
        }

        if(VSCheckSize(pp_tmp.lumWidthSrc, heightSrc, width, height)
           != 0)
        {
            APITRACE
                ("H264EncSetPreProcessing: ERROR Invalid size for stabilization");
            return H264ENC_INVALID_ARGUMENT;
        }

#ifdef VIDEOSTAB_ENABLED
        VSAlgInit(&pEncInst->vsSwData, pp_tmp.lumWidthSrc, heightSrc,
                  width, height);

        VSAlgGetResult(&pEncInst->vsSwData, &pp_tmp.horOffsetSrc,
                       &pp_tmp.verOffsetSrc);
#endif
    }

    pp_tmp.colorConversionType = pPreProcCfg->colorConversion.type;
    pp_tmp.colorConversionCoeffA = pPreProcCfg->colorConversion.coeffA;
    pp_tmp.colorConversionCoeffB = pPreProcCfg->colorConversion.coeffB;
    pp_tmp.colorConversionCoeffC = pPreProcCfg->colorConversion.coeffC;
    pp_tmp.colorConversionCoeffE = pPreProcCfg->colorConversion.coeffE;
    pp_tmp.colorConversionCoeffF = pPreProcCfg->colorConversion.coeffF;
    EncSetColorConversion(&pp_tmp, &pEncInst->asic);

    pEncInst->preProcess = pp_tmp;

    APITRACE("H264EncSetPreProcessing: OK");

    return H264ENC_OK;
}

/*------------------------------------------------------------------------------
    Function name   : H264EncGetPreProcessing
    Description     : Returns current preprocessing parameters
    Return type     : H264EncRet
    Argument        : inst - encoder instance
    Argument        : pPreProcCfg - place where the parameters are returned
------------------------------------------------------------------------------*/
H264EncRet H264EncGetPreProcessing(H264EncInst inst,
                                   H264EncPreProcessingCfg * pPreProcCfg)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;
    preProcess_s *pPP;

    APITRACE("H264EncGetPreProcessing#");

    /* Check for illegal inputs */
    if((inst == NULL) || (pPreProcCfg == NULL))
    {
        APITRACE("H264EncGetPreProcessing: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("H264EncGetPreProcessing: ERROR Invalid instance");
        return H264ENC_INSTANCE_ERROR;
    }

    pPP = &pEncInst->preProcess;

    pPreProcCfg->origHeight = pPP->lumHeightSrc;
    pPreProcCfg->origWidth = pPP->lumWidthSrc;
    pPreProcCfg->xOffset = pPP->horOffsetSrc;
    pPreProcCfg->yOffset = pPP->verOffsetSrc;

    pPreProcCfg->rotation = (H264EncPictureRotation) pPP->rotation;
    pPreProcCfg->inputType = (H264EncPictureType) pPP->inputFormat;

    pPreProcCfg->interlacedFrame    = pPP->interlacedFrame;
    pPreProcCfg->videoStabilization = pPP->videoStab;
    pPreProcCfg->scaledOutput       = pPP->scaledOutput;

    pPreProcCfg->colorConversion.type =
                        (H264EncColorConversionType) pPP->colorConversionType;
    pPreProcCfg->colorConversion.coeffA = pPP->colorConversionCoeffA;
    pPreProcCfg->colorConversion.coeffB = pPP->colorConversionCoeffB;
    pPreProcCfg->colorConversion.coeffC = pPP->colorConversionCoeffC;
    pPreProcCfg->colorConversion.coeffE = pPP->colorConversionCoeffE;
    pPreProcCfg->colorConversion.coeffF = pPP->colorConversionCoeffF;

    APITRACE("H264EncGetPreProcessing: OK");
    return H264ENC_OK;
}

/*------------------------------------------------------------------------------
    Function name   : H264EncSetRoiMap
    Description     : Returns current roimap status
    Return type     : H264EncRet
    Argument        : inst - encoder instance
    Argument        : map - place where the roi index map for each mb in raster scan way.
------------------------------------------------------------------------------*/
H264EncRet H264EncSetRoiMap(H264EncInst inst, u8 *roimap)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;
    regValues_s *regs = &pEncInst->asic.regs;
    u32 *map = pEncInst->asic.segmentMap.virtualAddress;
    u32 mask, mb, x, y;
    int idx = 0;

    APITRACE("H264EncSetRoiMap#");

    /* Check for illegal inputs */
    if (inst == NULL)
    {
        APITRACE("H264EncSetRoiMap: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    regs->segmentEnable = 1;
    regs->segmentMapUpdate = 1;
    /* Set AROI (ID=3), ROI1 (ID=1) and ROI2 (ID=2) into segment map. */
    for (y = 0, mb = 0, mask = 0; y < pEncInst->mbPerCol; y++)
    {
        for (x = 0; x < pEncInst->mbPerRow; x++, mb++)
        {
            u32 id = (u32)roimap[idx++];
            mask |= id << (28 - 4 * (mb % 8));
            if ((mb % 8) == 7) {
                *map++ = mask;
                mask = 0;
            }
        }
    }
    *map++ = mask;
    pEncInst->preProcess.roiCoded = 1;  /* AROI is coded into stream */

    APITRACE("H264EncSetRoiMap: OK");
    return H264ENC_OK;
}

/*------------------------------------------------------------------------------
    Function name : H264EncSetSeiUserData
    Description   : Sets user data SEI messages
    Return type   : H264EncRet
    Argument      : inst - the instance in use
                    pUserData - pointer to userData, this is used by the
                                encoder so it must not be released before
                                disabling user data
                    userDataSize - size of userData, minimum size 16,
                                   maximum size H264ENC_MAX_USER_DATA_SIZE
                                   not valid size disables userData sei messages
------------------------------------------------------------------------------*/
H264EncRet H264EncSetSeiUserData(H264EncInst inst, const u8 * pUserData,
                                 u32 userDataSize)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;

    APITRACE("H264EncSetSeiUserData#");
    APITRACEPARAM("userDataSize", userDataSize);

    /* Check for illegal inputs */
    if((pEncInst == NULL) || (userDataSize != 0 && pUserData == NULL))
    {
        APITRACE("H264EncSetSeiUserData: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("H264EncSetSeiUserData: ERROR Invalid instance");
        return H264ENC_INSTANCE_ERROR;
    }

    /* Disable user data */
    if((userDataSize < 16) || (userDataSize > H264ENC_MAX_USER_DATA_SIZE))
    {
        pEncInst->rateControl.sei.userDataEnabled = ENCHW_NO;
        pEncInst->rateControl.sei.pUserData = NULL;
        pEncInst->rateControl.sei.userDataSize = 0;
    }
    else
    {
        pEncInst->rateControl.sei.userDataEnabled = ENCHW_YES;
        pEncInst->rateControl.sei.pUserData = pUserData;
        pEncInst->rateControl.sei.userDataSize = userDataSize;
    }

    return H264ENC_OK;
}

/*------------------------------------------------------------------------------

    Function name : H264EncStrmStart
    Description   : Starts a new stream
    Return type   : H264EncRet
    Argument      : inst - encoder instance
    Argument      : pEncIn - user provided input parameters
                    pEncOut - place where output info is returned
------------------------------------------------------------------------------*/
H264EncRet H264EncStrmStart(H264EncInst inst, const H264EncIn * pEncIn,
                            H264EncOut * pEncOut)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;
    h264RateControl_s *rc;
    u32 tmp;

    APITRACE("H264EncStrmStart#");
    APITRACEPARAM("busLuma", pEncIn->busLuma);
    APITRACEPARAM("busChromaU", pEncIn->busChromaU);
    APITRACEPARAM("busChromaV", pEncIn->busChromaV);
    APITRACEPARAM("pOutBuf", pEncIn->pOutBuf);
    APITRACEPARAM("busOutBuf", pEncIn->busOutBuf);
    APITRACEPARAM("outBufSize", pEncIn->outBufSize);
    APITRACEPARAM("codingType", pEncIn->codingType);
    APITRACEPARAM("timeIncrement", pEncIn->timeIncrement);
    APITRACEPARAM("busLumaStab", pEncIn->busLumaStab);
    APITRACEPARAM("ipf", pEncIn->ipf);
    APITRACEPARAM("ltrf", pEncIn->ltrf);

    /* Check for illegal inputs */
    if((pEncInst == NULL) || (pEncIn == NULL) || (pEncOut == NULL))
    {
        APITRACE("H264EncStrmStart: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("H264EncStrmStart: ERROR Invalid instance");
        return H264ENC_INSTANCE_ERROR;
    }

    pEncOut->streamSize = 0;

    rc = &pEncInst->rateControl;

    /* Check status */
    if((pEncInst->encStatus != H264ENCSTAT_INIT) &&
       (pEncInst->encStatus != H264ENCSTAT_START_FRAME))
    {
        APITRACE("H264EncStrmStart: ERROR Invalid status");
        return H264ENC_INVALID_STATUS;
    }

    /* Check for invalid input values */
    if((pEncIn->pOutBuf == NULL) ||
       (pEncIn->outBufSize < H264ENCSTRMSTART_MIN_BUF))
    {
        APITRACE("H264EncStrmStart: ERROR Invalid input. Stream buffer");
        return H264ENC_INVALID_ARGUMENT;
    }

    /* Set stream buffer, the size has been checked */
    (void) H264SetBuffer(&pEncInst->stream, (u8 *) pEncIn->pOutBuf,
                         (u32) pEncIn->outBufSize);

    /* Set pointer to the beginning of NAL unit size buffer */
    pEncOut->pNaluSizeBuf = (u32 *) pEncInst->asic.sizeTbl.virtualAddress;
    pEncOut->numNalus = 0;

#ifdef TRACE_STREAM
    /* Open stream tracing */
    EncOpenStreamTrace("stream.trc");

    traceStream.frameNum = pEncInst->frameCnt;
    traceStream.id = 0; /* Stream generated by SW */
    traceStream.bitCnt = 0;  /* New frame */
#endif

    /* Set the profile to be used */
    pEncInst->seqParameterSet.profileIdc = 66; /* base profile */

    /* Interlaced => main profile */
    if (pEncInst->interlaced >= 1)
        pEncInst->seqParameterSet.profileIdc = 77;

    /* CABAC => main profile */
    if (pEncInst->picParameterSet.enableCabac >= 1)
        pEncInst->seqParameterSet.profileIdc = 77;

    /* 8x8 transform enabled => high profile */
    if (pEncInst->picParameterSet.transform8x8Mode == ENCHW_YES)
        pEncInst->seqParameterSet.profileIdc = 100;

    if (pEncIn->sendAUD)
    {
        tmp = pEncInst->stream.byteCnt;
        H264AccessUnitDelimiter(&pEncInst->stream, pEncInst->seqParameterSet.byteStream, 2);
        H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt - tmp);
    }

    /* update VUI */
    if(rc->sei.enabled == ENCHW_YES)
    {
        H264SpsSetVuiPictStructPresentFlag(&pEncInst->seqParameterSet, 1);
    }

    if(rc->hrd == ENCHW_YES)
    {
        H264SpsSetVuiHrd(&pEncInst->seqParameterSet, 1);

        H264SpsSetVuiHrdBitRate(&pEncInst->seqParameterSet,
                                rc->virtualBuffer.bitRate);

        H264SpsSetVuiHrdCpbSize(&pEncInst->seqParameterSet,
                                rc->virtualBuffer.bufferSize);
    }

    /* Initialize cabac context tables for HW */
    if (pEncInst->picParameterSet.enableCabac >= 1)
    {
        if (H264CabacInit(pEncInst->asic.cabacCtx.virtualAddress,
                          pEncInst->slice.cabacInitIdc) != 0)
        {
            APITRACE("H264EncStrmStart: ERROR in CABAC Context Init");
            return H264ENC_MEMORY_ERROR;
        }
    }

    /* Use the first frame QP in the PPS */
    pEncInst->picParameterSet.picInitQpMinus26 = (i32) (rc->qpHdr >> QP_FRACTIONAL_BITS) - 26;
    pEncInst->picParameterSet.picParameterSetId = 0;

    if((pEncInst->picParameterSet.enableCabac == 0) ||
       (pEncInst->picParameterSet.enableCabac == 2))
        pEncInst->picParameterSet.entropyCodingMode = ENCHW_NO;
    else
        pEncInst->picParameterSet.entropyCodingMode = ENCHW_YES;

    /* Init SEI */
    H264InitSei(&rc->sei, pEncInst->seqParameterSet.byteStream,
                rc->hrd, rc->outRateNum, rc->outRateDenom);

    if (pEncInst->svc.level)
    {
        tmp = pEncInst->stream.byteCnt;
        H264SvcSeiNal(pEncInst);
        H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-tmp);
    }
    tmp = pEncInst->stream.byteCnt;

    H264SeqParameterSet(&pEncInst->stream, &pEncInst->seqParameterSet, ENCHW_YES);
    H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-tmp);
    tmp = pEncInst->stream.byteCnt;

    /* Subset SPS for MVC */
    if (pEncInst->numViews > 1)
    {
        H264SubsetSeqParameterSet(&pEncInst->stream, &pEncInst->seqParameterSet);
        H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-tmp);
        tmp = pEncInst->stream.byteCnt;
    }

    H264PicParameterSet(&pEncInst->stream, &pEncInst->picParameterSet);
    H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-tmp);
    tmp = pEncInst->stream.byteCnt;

    /* In CABAC mode 2 we need two PPS: one with CAVLC (ppsId=0 for intra) and
     * one with CABAC (ppsId=1 for inter) */
    if (pEncInst->picParameterSet.enableCabac == 2)
    {
        pEncInst->picParameterSet.picParameterSetId = 1;
        pEncInst->picParameterSet.entropyCodingMode = ENCHW_YES;
        H264PicParameterSet(&pEncInst->stream, &pEncInst->picParameterSet);
        H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-tmp);
        tmp = pEncInst->stream.byteCnt;
    }

    if(pEncInst->stream.overflow == ENCHW_YES)
    {
        pEncOut->streamSize = 0;
        pEncOut->numNalus = 0;
        APITRACE("H264EncStrmStart: ERROR Output buffer too small");
        return H264ENC_OUTPUT_BUFFER_OVERFLOW;
    }

    /* Bytes generated */
    pEncOut->streamSize = pEncInst->stream.byteCnt;

    /* Status == START_STREAM   Stream started */
    pEncInst->encStatus = H264ENCSTAT_START_STREAM;

    pEncInst->slice.frameNum = 0;
    pEncInst->rateControl.fillerIdx = (u32) (-1);

    if(rc->hrd == ENCHW_YES)
    {
        /* Update HRD Parameters to RC if needed */
        u32 bitrate = H264SpsGetVuiHrdBitRate(&pEncInst->seqParameterSet);
        u32 cpbsize = H264SpsGetVuiHrdCpbSize(&pEncInst->seqParameterSet);

        if ((rc->virtualBuffer.bitRate != (i32)bitrate) ||
            (rc->virtualBuffer.bufferSize != (i32)cpbsize))
        {
            rc->virtualBuffer.bitRate = bitrate;
            rc->virtualBuffer.bufferSize = cpbsize;
            (void) H264InitRc(rc, 1);
        }
    }

#ifdef VIDEOSTAB_ENABLED
    /* new stream so reset the stabilization */
    VSAlgReset(&pEncInst->vsSwData);
#endif

    APITRACE("H264EncStrmStart: OK");
    return H264ENC_OK;
}

/*------------------------------------------------------------------------------

    Function name : H264EncStrmEncode
    Description   : Encodes a new picture
    Return type   : H264EncRet
    Argument      : inst - encoder instance
    Argument      : pEncIn - user provided input parameters
                    pEncOut - place where output info is returned
------------------------------------------------------------------------------*/
H264EncRet H264EncStrmEncode(H264EncInst inst, const H264EncIn * pEncIn,
                             H264EncOut * pEncOut,
                             H264EncSliceReadyCallBackFunc sliceReadyCbFunc,
                             EncInputMBLineBufCallBackFunc lineBufCbFunc,
                             void * pAppData)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;
    slice_s *pSlice;
    regValues_s *regs;
    h264EncodeFrame_e ret;
    H264EncPictureCodingType ct;
    picBuffer *picBuffer;
    sliceType_e rcSliceType;
    u32 tmp = 0;
    u32 setSpsPps = 0;
    i32 top_pos,bottom_pos;
    i32 prefix_nalu_length = 0;
    u8 *prefix_nalu_addr = NULL;
    APITRACE("H264EncStrmEncode#");
    APITRACEPARAM("busLuma", pEncIn->busLuma);
    APITRACEPARAM("busChromaU", pEncIn->busChromaU);
    APITRACEPARAM("busChromaV", pEncIn->busChromaV);
    APITRACEPARAM("pOutBuf", pEncIn->pOutBuf);
    APITRACEPARAM("busOutBuf", pEncIn->busOutBuf);
    APITRACEPARAM("outBufSize", pEncIn->outBufSize);
    APITRACEPARAM("codingType", pEncIn->codingType);
    APITRACEPARAM("timeIncrement", pEncIn->timeIncrement);
    APITRACEPARAM("busLumaStab", pEncIn->busLumaStab);
    APITRACEPARAM("ipf", pEncIn->ipf);
    APITRACEPARAM("ltrf", pEncIn->ltrf);

    /* Check for illegal inputs */
    if((pEncInst == NULL) || (pEncIn == NULL) || (pEncOut == NULL))
    {
        APITRACE("H264EncStrmEncode: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("H264EncStrmEncode: ERROR Invalid instance");
        return H264ENC_INSTANCE_ERROR;
    }

    /* some shortcuts */
    pSlice = &pEncInst->slice;
    regs = &pEncInst->asic.regs;

    /* Clear the output structure */
    pEncOut->codingType = H264ENC_NOTCODED_FRAME;
    pEncOut->streamSize = 0;
    pEncOut->ipf = pEncOut->ltrf = 0;

    /* Output buffer for down-scaled picture, 0/NULL when disabled. */
    pEncOut->busScaledLuma    = regs->scaledLumBase;
    pEncOut->scaledPicture = (u8*)pEncInst->asic.scaledImage.virtualAddress;

    /* Set pointer to the beginning of NAL unit size buffer */
    pEncOut->pNaluSizeBuf = (u32 *) pEncInst->asic.sizeTbl.virtualAddress;
    pEncOut->numNalus = pEncInst->naluOffset = 0;
    pEncInst->numNalus = 0;

    /* Clear the NAL unit size table */
    if(pEncOut->pNaluSizeBuf != NULL)
        pEncOut->pNaluSizeBuf[0] = 0;

#ifdef EVALUATION_LIMIT
    /* Check for evaluation limit */
    if(pEncInst->frameCnt >= EVALUATION_LIMIT)
    {
        APITRACE("H264EncStrmEncode: OK Evaluation limit exceeded");
        return H264ENC_OK;
    }
#endif

    /* Check status, INIT and ERROR not allowed */
    if((pEncInst->encStatus != H264ENCSTAT_START_STREAM) &&
       (pEncInst->encStatus != H264ENCSTAT_START_FRAME))
    {
        APITRACE("H264EncStrmEncode: ERROR Invalid status");
        return H264ENC_INVALID_STATUS;
    }

    /* Check for invalid input values */
    if((!H264_BUS_ADDRESS_VALID(pEncIn->busOutBuf)) ||
       (pEncIn->pOutBuf == NULL) ||
       (pEncIn->outBufSize < H264ENCSTRMENCODE_MIN_BUF) ||
       (pEncIn->codingType > H264ENC_NONIDR_INTRA_FRAME))
    {
        APITRACE("H264EncStrmEncode: ERROR Invalid input. Output buffer");
        return H264ENC_INVALID_ARGUMENT;
    }

    switch (pEncInst->preProcess.inputFormat)
    {
    case H264ENC_YUV420_PLANAR:
    case H264ENC_P010:
        if(!H264_BUS_ADDRESS_VALID(pEncIn->busChromaV))
        {
            APITRACE("H264EncStrmEncode: ERROR Invalid input busChromaU");
            return H264ENC_INVALID_ARGUMENT;
        }
        /* fall through */
    case H264ENC_YUV420_SEMIPLANAR:
    case H264ENC_YUV420_SEMIPLANAR_VU:
    case H264ENC_SP_101010:
        if(!H264_BUS_ADDRESS_VALID(pEncIn->busChromaU))
        {
            APITRACE("H264EncStrmEncode: ERROR Invalid input busChromaU");
            return H264ENC_INVALID_ARGUMENT;
        }
        /* fall through */
    case H264ENC_YUV422_INTERLEAVED_YUYV:
    case H264ENC_YUV422_INTERLEAVED_UYVY:
    case H264ENC_RGB565:
    case H264ENC_BGR565:
    case H264ENC_RGB555:
    case H264ENC_BGR555:
    case H264ENC_RGB444:
    case H264ENC_BGR444:
    case H264ENC_RGB888:
    case H264ENC_BGR888:
    case H264ENC_RGB101010:
    case H264ENC_BGR101010:
        if(!H264_BUS_ADDRESS_VALID(pEncIn->busLuma))
        {
            APITRACE("H264EncStrmEncode: ERROR Invalid input busLuma");
            return H264ENC_INVALID_ARGUMENT;
        }
        break;
    default:
        APITRACE("H264EncStrmEncode: ERROR Invalid input format");
        return H264ENC_INVALID_ARGUMENT;
    }

    if(pEncInst->preProcess.videoStab)
    {
        if(!H264_BUS_ADDRESS_VALID(pEncIn->busLumaStab))
        {
            APITRACE("H264EncStrmEncode: ERROR Invalid input busLumaStab");
            return H264ENC_INVALID_ARGUMENT;
        }
    }

    /* Set stream buffer, the size has been checked */
    if(H264SetBuffer(&pEncInst->stream, (u8 *) pEncIn->pOutBuf,
                     (i32) pEncIn->outBufSize) == ENCHW_NOK)
    {
        APITRACE("H264EncStrmEncode: ERROR Invalid output buffer");
        return H264ENC_INVALID_ARGUMENT;
    }

    /* Try to reserve the HW resource */
    if(EWLReserveHw(pEncInst->asic.ewl) == EWL_ERROR)
    {
        APITRACE("H264EncStrmEncode: ERROR HW unavailable");
        return H264ENC_HW_RESERVED;
    }
    /* update in/out buffers */
    regs->inputLumBase = pEncIn->busLuma;
    regs->inputCbBase = pEncIn->busChromaU;
    regs->inputCrBase = pEncIn->busChromaV;

    regs->outputStrmBase = pEncIn->busOutBuf;
    regs->outputStrmSize = pEncIn->outBufSize;

    pEncInst->inputLineBuf.wrCnt = pEncIn->lineBufWrCnt;
    pEncInst->inputLineBuf.cbFunc = lineBufCbFunc;
    pEncInst->sliceReadyCbFunc = sliceReadyCbFunc;
    pEncInst->pOutBuf = pEncIn->pOutBuf;
    pEncInst->pAppData = pAppData;
    pEncInst->rfcBufOverflow = ENCHW_NO;

    /* setup stabilization */
    if(pEncInst->preProcess.videoStab)
    {
        regs->vsNextLumaBase = pEncIn->busLumaStab;
    }

    ct = pEncIn->codingType;
    if((pEncInst->gdrEnabled == 1) && (pEncInst->encStatus == H264ENCSTAT_START_FRAME) && (pEncInst->gdrFirstIntraFrame == 0))
    {
        regs->intraAreaTop = regs->intraAreaLeft = regs->intraAreaBottom =
        regs->intraAreaRight = 255;
        regs->roi1Top = regs->roi1Left = regs->roi1Bottom =
        regs->roi1Right = 255;
        regs->roi1DeltaQp = 0;
        if(pEncIn->codingType == H264ENC_INTRA_FRAME)
        {
            pEncInst->gdrStart++ ;
            ct = H264ENC_PREDICTED_FRAME;
        }
        else if(pEncIn->codingType == H264ENC_NONIDR_INTRA_FRAME)
        {
            ct = H264ENC_PREDICTED_FRAME;
        }
        if(pEncInst->gdrStart)
        {
            if(pEncInst->gdrCount == 0)
                pEncInst->rateControl.sei.insertRecoveryPointMessage = ENCHW_YES;
            else
                pEncInst->rateControl.sei.insertRecoveryPointMessage = ENCHW_NO;
            top_pos = (pEncInst->gdrCount / (1 + pEncInst->interlaced)) * pEncInst->gdrAverageMBRows;
            bottom_pos = 0;
            if(pEncInst->gdrMBLeft)
            {
                if((pEncInst->gdrCount / (1 + (i32)pEncInst->interlaced)) < pEncInst->gdrMBLeft)
                {
                    top_pos += (pEncInst->gdrCount / (1 + (i32)pEncInst->interlaced));
                    bottom_pos += 1;
                }
                else
                {
                    top_pos += pEncInst->gdrMBLeft;
                }
            }
            bottom_pos += top_pos + pEncInst->gdrAverageMBRows;
#ifndef SEARCH_AREA_SMALL
            bottom_pos += 1;
#endif
            if(bottom_pos > ((i32)pEncInst->mbPerCol - 1))
            {
                bottom_pos=pEncInst->mbPerCol - 1;
            }

            regs->intraAreaTop = top_pos;
            regs->intraAreaLeft = 0;
            regs->intraAreaBottom = bottom_pos;
            regs->intraAreaRight = pEncInst->mbPerRow - 1;

            //to make video quality in intra area is close to inter area.
            regs->roi1Top = top_pos;
            regs->roi1Left = 0;
            regs->roi1Bottom = bottom_pos;
            regs->roi1Right = pEncInst->mbPerRow - 1;

            regs->roi1DeltaQp = 3;
        }

        regs->roiUpdate   = 1;    /* ROI has changed from previous frame. */

    }

    if (pEncInst->svc.level==0)
    {
        /* Status may affect the frame coding type */
        if(pEncInst->encStatus == H264ENCSTAT_START_STREAM)
        {
            ct = H264ENC_INTRA_FRAME;
        }
#ifdef VIDEOSTAB_ENABLED
        if(pEncInst->vsSwData.sceneChange)
        {
            pEncInst->encStatus = H264ENCSTAT_START_STREAM;
            pEncInst->vsSwData.sceneChange = 0;
            ct = H264ENC_INTRA_FRAME;
        }
#endif
        pSlice->prevFrameNum = pSlice->frameNum;

        /* MVC view frames are always predicted from base view. */
        if ((pEncInst->numViews > 1) && ((pSlice->frameNum % 2) == 1))
            ct = H264ENC_PREDICTED_FRAME;

        /* Interlaced picture second field can't be IDR. */
        if (pEncInst->interlaced && ((pSlice->frameNum % 2) == 1) &&
            (ct == H264ENC_INTRA_FRAME))
            ct = H264ENC_NONIDR_INTRA_FRAME;

        /* Frame coding type defines the NAL unit type */
        switch (ct)
        {
        case H264ENC_INTRA_FRAME:
            /* IDR-slice */
            pSlice->nalUnitType = IDR;
            pSlice->sliceType = rcSliceType = ISLICE;
            pSlice->frameNum = 0;
            pEncInst->preProcess.intra = 1;
            H264MadInit(&pEncInst->mad, pEncInst->mbPerFrame);
            break;
        case H264ENC_NONIDR_INTRA_FRAME:
            /* non-IDR P-slice */
            pSlice->nalUnitType = NONIDR;
            pSlice->sliceType = rcSliceType = ISLICE;
            if (pEncInst->interlaced) rcSliceType = PSLICE; /* No new GOP */
            pEncInst->preProcess.intra = 0;
            break;
        case H264ENC_PREDICTED_FRAME:
        default:
            /* non-IDR P-slice */
            pSlice->nalUnitType = NONIDR;
            pSlice->sliceType = rcSliceType = PSLICE;
            pEncInst->preProcess.intra = 0;
            break;
        }

        /* Interlaced with intra-only, top and bottom fields. */
        if (pEncInst->interlaced && ((pSlice->prevFrameNum % 2) == 1))
            pSlice->frameNum = pSlice->prevFrameNum;

    }
    else
    {
        /* Status may affect the frame coding type */
        if(pEncInst->encStatus == H264ENCSTAT_START_STREAM)
        {
            ct = H264ENC_INTRA_FRAME;
        }
#ifdef VIDEOSTAB_ENABLED
        if(pEncInst->vsSwData.sceneChange)
        {
            pEncInst->encStatus = H264ENCSTAT_START_STREAM;
            pEncInst->vsSwData.sceneChange = 0;
            ct = H264ENC_INTRA_FRAME;
        }
#endif
        pSlice->prevFrameNum = pSlice->frameNum;

        /* MVC view frames are always predicted from base view. */
        if ((pEncInst->numViews > 1) && ((pSlice->frameNum % 2) == 1))
            ct = H264ENC_PREDICTED_FRAME;

        /* Interlaced picture second field can't be IDR. */
        if (pEncInst->interlaced && ((pSlice->frameNum % 2) == 1) &&
            (ct == H264ENC_INTRA_FRAME))
            ct = H264ENC_NONIDR_INTRA_FRAME;

        /* Frame coding type defines the NAL unit type */
        switch (ct)
        {
        case H264ENC_INTRA_FRAME:
            /* IDR-slice */
            pSlice->nalUnitType = IDR;
            pSlice->sliceType = rcSliceType = ISLICE;
            pSlice->frameNum = 0;
            pEncInst->preProcess.intra = 1;
            H264MadInit(&pEncInst->mad, pEncInst->mbPerFrame);
            break;
        case H264ENC_NONIDR_INTRA_FRAME:
            /* non-IDR P-slice */
            pSlice->nalUnitType = NONIDR;
            pSlice->sliceType = rcSliceType = ISLICE;
            if (pEncInst->interlaced) rcSliceType = PSLICE; /* No new GOP */
            pEncInst->preProcess.intra = 0;
            break;
        case H264ENC_PREDICTED_FRAME:
        default:
            /* non-IDR P-slice */
            pSlice->nalUnitType = NONIDR;
            pSlice->sliceType = rcSliceType = PSLICE;
            pEncInst->preProcess.intra = 0;
            break;
        }

        /* Interlaced with intra-only, top and bottom fields. */
        if (pEncInst->interlaced && ((pSlice->prevFrameNum % 2) == 1))
            pSlice->frameNum = pSlice->prevFrameNum;

    }

    /* Rate control, ISLICE begins a new GOP */
    H264BeforePicRc(&pEncInst->rateControl, pEncIn->timeIncrement, rcSliceType);

    /* set registers for denoise */
    H264EncDnfPrepare(pEncInst);

    /* time stamp updated */
    H264UpdateSeiTS(&pEncInst->rateControl.sei, pEncIn->timeIncrement);

    /* Rate control may choose to skip the frame */
    if(pEncInst->rateControl.frameCoded == ENCHW_NO)
    {
        APITRACE("H264EncStrmEncode: OK, frame skipped");
        pSlice->frameNum = pSlice->prevFrameNum;    /* restore frame_num */

#if 0
        /* Write previous reconstructed frame when frame is skipped */
        EncAsicRecycleInternalImage(&pEncInst->asic.regs);
        EncDumpRecon(&pEncInst->asic);
        EncAsicRecycleInternalImage(&pEncInst->asic.regs);
#endif

        EWLReleaseHw(pEncInst->asic.ewl);

        return H264ENC_FRAME_READY;
    }

#ifdef TRACE_STREAM
    traceStream.frameNum = pEncInst->frameCnt;
    traceStream.id = 0; /* Stream generated by SW */
    traceStream.bitCnt = 0;  /* New frame */
#endif

    /* Initialize picture buffer and ref pic list according to frame type */
    picBuffer = &pEncInst->picBuffer;
    picBuffer->cur_pic->show    = 1;
    if (pEncInst->numViews > 1)
        picBuffer->cur_pic->poc     = pEncInst->frameCnt/2;
    else
        picBuffer->cur_pic->poc     = pEncInst->frameCnt;
    picBuffer->cur_pic->i_frame = (ct == H264ENC_INTRA_FRAME);
    picBuffer->cur_pic->frameNum = pSlice->frameNum;
    H264InitializePictureBuffer(picBuffer);

    /* Set picture buffer according to frame coding type */
    if (ct != H264ENC_INTRA_FRAME) {
        picBuffer->cur_pic->p_frame = 1;
        picBuffer->cur_pic->arf = 0;
        picBuffer->cur_pic->ipf = (pEncIn->ipf&H264ENC_REFRESH) ? 1 : 0;
        if (pEncIn->ipf&H264ENC_REFERENCE)
        {
            if (picBuffer->refPicList[0].ipf)
                picBuffer->refPicList[0].search = true;
            else if (picBuffer->refPicList[1].ipf)
                picBuffer->refPicList[1].search = true;
        }
        if (pEncIn->ltrf&H264ENC_REFERENCE)
        {
            if (picBuffer->refPicList[0].grf)
                picBuffer->refPicList[0].search = true;
            else if (picBuffer->refPicList[1].grf)
                picBuffer->refPicList[1].search = true;
        }
    }

    /* LTR marking, not used for interlaced coding. */
    if (!pEncInst->interlaced) {
        picBuffer->cur_pic->grf = (pEncIn->ltrf&H264ENC_REFRESH) ? 1 : 0;

        if (pEncInst->rateControl.longTermPicRate &&
            pEncInst->seqParameterSet.numRefFrames >= 2)
        {
            if ((pEncInst->frameCnt%pEncInst->rateControl.longTermPicRate) == 0)
                picBuffer->cur_pic->grf = 1;
        }
        if (picBuffer->cur_pic->i_frame)
            picBuffer->cur_pic->grf = 1;
    }

#ifdef INTERNAL_TEST
    /* Configure the encoder instance according to the test vector */
    H264ConfigureTestBeforeFrame(pEncInst);
#endif

    /* cannot mark picture both long-term and short-term ref -> if long-term
     * to be refreshed, short-term won't refresh */
    if (picBuffer->cur_pic->grf &&
        pEncInst->seqParameterSet.numRefFrames >= 2)
        picBuffer->cur_pic->ipf = false;

    /* check if GDR and idrHdr enabled, when to set SPS/PPS headers */
    if(pEncInst->idrHdr && pEncInst->gdrEnabled == 1)
    {
        if(pEncInst->gdrCount == 0 && pEncInst->gdrStart == 1)
            setSpsPps = 1;
        else
            setSpsPps = 0;
    }

    if(pEncInst->idrHdr)
    {
        if( (!pEncInst->gdrEnabled && ct == H264ENC_INTRA_FRAME && pEncInst->frameCnt) ||
            (pEncInst->gdrEnabled && setSpsPps) )
        {
            /* Determine the start offset for NALU size table.
             * HW needs a 64-bit aligned address so we leave the first 32-bits unused
             * if SW creates one leading NAL unit. Also the HW bus address needs to be
             * offset in H264CodeFrame. */
            {
                i32 numLeadingNalus = 2;
                sei_s *sei = &pEncInst->rateControl.sei;

                if(sei->enabled == ENCHW_YES || sei->userDataEnabled == ENCHW_YES ||
                   sei->insertRecoveryPointMessage == ENCHW_YES)
                    numLeadingNalus++;

                if((pEncInst->numViews > 1) && ((pSlice->frameNum % 2) == 0))
                    numLeadingNalus++;

                if(pEncInst->picParameterSet.enableCabac == 2)
                    numLeadingNalus++;

                if( pEncInst->svc.level || pEncInst->svc.enableSvctPrefix )   ////prefix
                    numLeadingNalus++;

                if(pEncInst->frameCnt && pEncIn->sendAUD)  ////aud
                    numLeadingNalus++;

                if(numLeadingNalus % 2)
                {
                    pEncOut->pNaluSizeBuf++;
                    pEncInst->naluOffset++;
                }

            }

            if(pEncInst->frameCnt && pEncIn->sendAUD)
            {
                tmp = pEncInst->stream.byteCnt;
                H264AccessUnitDelimiter(&pEncInst->stream, pEncInst->seqParameterSet.byteStream, 2);
                H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt - tmp);
                tmp = pEncInst->stream.byteCnt;
                pEncInst->numNalus++;
            }

            tmp = pEncInst->stream.byteCnt;
            H264SeqParameterSet(&pEncInst->stream, &pEncInst->seqParameterSet, ENCHW_YES);
            H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-tmp);
            tmp = pEncInst->stream.byteCnt;
            pEncInst->numNalus++;

            /* Subset SPS for MVC */
            if (pEncInst->numViews > 1)
            {
                H264SubsetSeqParameterSet(&pEncInst->stream, &pEncInst->seqParameterSet);
                H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-tmp);
                tmp = pEncInst->stream.byteCnt;
                pEncInst->numNalus++;
            }

            H264PicParameterSet(&pEncInst->stream, &pEncInst->picParameterSet);
            H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-tmp);
            tmp = pEncInst->stream.byteCnt;
            pEncInst->numNalus++;

            /* In CABAC mode 2 we need two PPS: one with CAVLC (ppsId=0 for intra) and
             * one with CABAC (ppsId=1 for inter) */
            if (pEncInst->picParameterSet.enableCabac == 2)
            {
                pEncInst->picParameterSet.picParameterSetId = 1;
                pEncInst->picParameterSet.entropyCodingMode = ENCHW_YES;
                H264PicParameterSet(&pEncInst->stream, &pEncInst->picParameterSet);
                H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-tmp);
                tmp = pEncInst->stream.byteCnt;
                pEncInst->numNalus++;
            }
            pEncOut->streamSize = pEncInst->stream.byteCnt;
        }
        else if(ct == H264ENC_INTRA_FRAME && !pEncInst->frameCnt )//the first I frame
        {
            i32 numLeadingNalus = 0;
            sei_s *sei = &pEncInst->rateControl.sei;

            if(sei->enabled == ENCHW_YES || sei->userDataEnabled == ENCHW_YES ||
               sei->insertRecoveryPointMessage == ENCHW_YES)
                numLeadingNalus++;

            if((pEncInst->numViews > 1) && ((pSlice->frameNum % 2) == 0))
                numLeadingNalus++;

            if( pEncInst->svc.level || pEncInst->svc.enableSvctPrefix )  //prefix
                numLeadingNalus++;

            //aud is added in H264EncStrmStrat
            /*if(pEncInst->frameCnt && pEncInst->svc.level && pEncIn->sendAUD)
                numLeadingNalus++; */

            if(numLeadingNalus % 2)
            {
                pEncOut->pNaluSizeBuf++;
                pEncInst->naluOffset++;
            }
        }
        else if(ct != H264ENC_INTRA_FRAME )
        {
            i32 numLeadingNalus = 0;
            sei_s *sei = &pEncInst->rateControl.sei;

            if(sei->enabled == ENCHW_YES || sei->userDataEnabled == ENCHW_YES ||
               sei->insertRecoveryPointMessage == ENCHW_YES)
                numLeadingNalus++;

            if((pEncInst->numViews > 1) && ((pSlice->frameNum % 2) == 0))
                numLeadingNalus++;

            if( pEncInst->svc.level || pEncInst->svc.enableSvctPrefix )   //prefix
                numLeadingNalus++;

            if(pEncInst->frameCnt && pEncIn->sendAUD) //aud
                numLeadingNalus++;

            if(numLeadingNalus % 2)
            {
                pEncOut->pNaluSizeBuf++;
                pEncInst->naluOffset++;
            }
        }
    }
    /* Determine the start offset for NALU size table.
     * HW needs a 64-bit aligned address so we leave the first 32-bits unused
     * if SW creates one leading NAL unit. Also the HW bus address needs to be
     * offset in H264CodeFrame. */
    if(pEncInst->idrHdr == ENCHW_NO)
    {
        i32 numLeadingNalus = 0;
        sei_s *sei = &pEncInst->rateControl.sei;

        if(sei->enabled == ENCHW_YES || sei->userDataEnabled == ENCHW_YES || sei->insertRecoveryPointMessage == ENCHW_YES)
            numLeadingNalus++;

        if ((pEncInst->numViews > 1) && ((pSlice->frameNum % 2) == 0))
            numLeadingNalus++;

        if( pEncInst->svc.level || pEncInst->svc.enableSvctPrefix )   ////prefix
            numLeadingNalus++;

        if(pEncInst->frameCnt && pEncIn->sendAUD)  ////aud
            numLeadingNalus++;

        //pEncInst->numNalus = numLeadingNalus; //Not pre add.
        if(numLeadingNalus % 2)
        {
            pEncOut->pNaluSizeBuf++;
            pEncInst->naluOffset++;
        }
    }

    /* update any cropping/rotation/filling */
    pEncInst->preProcess.bottomField = (pSlice->frameNum%2) == pSlice->fieldOrder;
    EncPreProcess(&pEncInst->asic, &pEncInst->preProcess);

    if (!((pEncInst->idrHdr && !pEncInst->gdrEnabled && ct == H264ENC_INTRA_FRAME && pEncInst->frameCnt) ||
       (pEncInst->idrHdr && pEncInst->gdrEnabled && setSpsPps) ))
    {
        if(pEncInst->frameCnt && pEncIn->sendAUD)
        {
            tmp = pEncInst->stream.byteCnt;
            H264AccessUnitDelimiter(&pEncInst->stream, pEncInst->seqParameterSet.byteStream, 2);
            H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt - tmp);
            tmp = pEncInst->stream.byteCnt;
            pEncInst->numNalus++;
        }
    }

    /* SEI message */
    {
        sei_s *sei = &pEncInst->rateControl.sei;

        if(sei->enabled == ENCHW_YES || sei->userDataEnabled == ENCHW_YES || sei->insertRecoveryPointMessage == ENCHW_YES)
        {

            H264NalUnitHdr(&pEncInst->stream, 0, SEI, sei->byteStream);

            if(sei->enabled == ENCHW_YES)
            {
                if(pSlice->nalUnitType == IDR)
                {
                    H264BufferingSei(&pEncInst->stream, sei);
                }

                H264PicTimingSei(&pEncInst->stream, sei);
            }

            if(sei->userDataEnabled == ENCHW_YES)
            {
                H264UserDataUnregSei(&pEncInst->stream, sei);
            }

            if (sei->insertRecoveryPointMessage == ENCHW_YES)
            {
                H264RecoveryPointSei(&pEncInst->stream, sei);
            }

            H264RbspTrailingBits(&pEncInst->stream);

            sei->nalUnitSize = pEncInst->stream.byteCnt;

            H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-tmp);
            //if(pEncInst->idrHdr == ENCHW_YES)
                pEncInst->numNalus++;
        }
    }

    /* For MVC stream insert prefix NALU before base view pictures */
    if ((pEncInst->numViews > 1) && ((pSlice->frameNum % 2) == 0))
    {
        i32 byteCnt = pEncInst->stream.byteCnt;
        H264PrefixNal(pEncInst, false);
        H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-byteCnt);
        //if(pEncInst->idrHdr == ENCHW_YES)
            pEncInst->numNalus++;
    }
    else
    {
        pEncInst->mvc.anchorPicFlag = (pSlice->frameNum == 1);
        pEncInst->mvc.viewId = 1;
        pEncInst->mvc.interViewFlag = 0;
    }

    if (pEncInst->svc.level)
    {
        i32 byteCnt = pEncInst->stream.byteCnt;

        if (ct==H264ENC_INTRA_FRAME)
        {
            picBuffer->cur_pic->ipf = true;
            picBuffer->cur_pic->grf = false;
            picBuffer->cur_pic->arf = false;
            picBuffer->cur_pic->orderCmd = 0;
            pEncInst->svc.gopIndex = -1;
            /* Get the reference frame buffers from picture buffer */
            H264PictureBufferSetRef(picBuffer, &pEncInst->asic,
                    pEncInst->interlaced ? 2 : pEncInst->numViews);
        }
        else
        {
            int index;;
            gopinfo_s *ctrl;
            index = (pEncInst->svc.gopIndex+1)&(pEncInst->svc.gopLength-1);
            pEncInst->svc.gopIndex = index;
            ctrl = &pEncInst->svc.gop[index];
            if ((ctrl->isRef))
            {
                picBuffer->cur_pic->ipf = true;
                picBuffer->cur_pic->grf = false;
                picBuffer->cur_pic->arf = false;
            }
            else
            {
                picBuffer->cur_pic->ipf = false;
                picBuffer->cur_pic->grf = false;
                picBuffer->cur_pic->arf = false;
            }
            if (ct==H264ENC_INTRA_FRAME)
            {
                picBuffer->cur_pic->orderCmd = 0;
            }
            else
            {
                picBuffer->cur_pic->orderCmd = ctrl->orderCmd;
            }
            /* Get the reference frame buffers from picture buffer */
            H264PictureBufferSetRefForSvct(picBuffer, &pEncInst->asic,
                    pSlice->frameNumBits);
        }

        /* for SVCT, insert prefix NALU before each slice */
        //pEncOut->pNaluSizeBuf++;
        //pEncInst->naluOffset++;

        H264PrefixNal(pEncInst, true);
        H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-byteCnt);
        pEncInst->numNalus++;
        prefix_nalu_length = pEncInst->stream.byteCnt-byteCnt;//save prefix length.
        prefix_nalu_addr = pEncInst->stream.stream - (pEncInst->stream.byteCnt-byteCnt);//save prefix addr.
        //if(pEncInst->idrHdr == ENCHW_YES)
            //pEncInst->numNalus++;
    }
    else
    {
        picBuffer->cur_pic->orderCmd = 0;
        pEncInst->svc.gopIndex = -1;
        /* Get the reference frame buffers from picture buffer */
        H264PictureBufferSetRef(picBuffer, &pEncInst->asic,
                pEncInst->interlaced ? 2 : pEncInst->numViews);

        if( pEncInst->svc.enableSvctPrefix )
        {
            if (ct==H264ENC_INTRA_FRAME)
            {
                pEncInst->svc.gopIndex = -1;
            }
            else
            {
                pEncInst->svc.gopIndex = -2;
            }
            APITRACE("svc.enableSVC");

            i32 byteCnt = pEncInst->stream.byteCnt;
            /* for SVCT, insert prefix NALU before each slice */
            //pEncOut->pNaluSizeBuf++;
            //pEncInst->naluOffset++;
            pEncInst->numNalus++;
            H264PrefixNal(pEncInst, true);
            H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt-byteCnt);
            prefix_nalu_length = pEncInst->stream.byteCnt-byteCnt;//save prefix length.
            prefix_nalu_addr = pEncInst->stream.stream - (pEncInst->stream.byteCnt-byteCnt);//save prefix addr.
            //if(pEncInst->idrHdr == ENCHW_YES)
                //pEncInst->numNalus++;
        }
    }

    /* Code one frame */
    ret = H264CodeFrame(pEncInst);

#ifdef TRACE_RECON
    EncDumpRecon(&pEncInst->asic);
#endif

    if(ret != H264ENCODE_OK)
    {
        /* Error has occured and the frame is invalid */
        H264EncRet to_user;

        switch (ret)
        {
        case H264ENCODE_TIMEOUT:
            APITRACE("H264EncStrmEncode: ERROR HW/IRQ timeout");
            to_user = H264ENC_HW_TIMEOUT;
            break;
        case H264ENCODE_HW_RESET:
            APITRACE("H264EncStrmEncode: ERROR HW reset detected");
            to_user = H264ENC_HW_RESET;
            break;
        case H264ENCODE_HW_ERROR:
            APITRACE("H264EncStrmEncode: ERROR HW bus access error");
            to_user = H264ENC_HW_BUS_ERROR;
            break;
        case H264ENCODE_FUSE_ERROR:
            APITRACE("H264EncStrmEncode: ERROR Fuse error");
            to_user = H264ENC_FUSE_ERROR;
            break;
        case H264ENCODE_SYSTEM_ERROR:
        default:
            /* System error has occured, encoding can't continue */
            pEncInst->encStatus = H264ENCSTAT_ERROR;
            APITRACE("H264EncStrmEncode: ERROR Fatal system error");
            to_user = H264ENC_SYSTEM_ERROR;
        }

        return to_user;
    }

#ifdef VIDEOSTAB_ENABLED
    /* Finalize video stabilization */
    if(pEncInst->preProcess.videoStab)
    {
        u32 no_motion;

        VSReadStabData(pEncInst->asic.regs.regMirror, &pEncInst->vsHwData);

        no_motion = VSAlgStabilize(&pEncInst->vsSwData, &pEncInst->vsHwData);
        if(no_motion)
        {
            VSAlgReset(&pEncInst->vsSwData);
        }

        /* update offset after stabilization */
        VSAlgGetResult(&pEncInst->vsSwData, &pEncInst->preProcess.horOffsetSrc,
                       &pEncInst->preProcess.verOffsetSrc);
    }
#endif

    H264EncDnfUpdate(pEncInst);

    /* Update NALU table with the amount of slices created by the HW */
    {
        i32 numSlices;

        if (pEncInst->slice.sliceSize)
            numSlices = (pEncInst->mbPerFrame + pEncInst->slice.sliceSize - 1) /
                        pEncInst->slice.sliceSize;
        else
            numSlices = 1;

        pEncOut->numNalus += numSlices;
        pEncOut->pNaluSizeBuf[pEncOut->numNalus] = 0;
    }

    if (pEncInst->svc.level || pEncInst->svc.enableSvctPrefix)
    {
        pEncOut->streamSize = pEncInst->stream.byteCnt;
        H264SupplyPrefixForSvctMutiSlice(inst, pEncIn, pEncOut, prefix_nalu_length, prefix_nalu_addr);
    }

    /* Filler data if needed */
    if(0)
    {
        u32 s = H264FillerRc(&pEncInst->rateControl, pEncInst->frameCnt);

        if(s != 0)
        {
            s = H264FillerNALU(&pEncInst->stream,
                               (i32) s, pEncInst->seqParameterSet.byteStream);
        }
        pEncInst->fillerNalSize = s;
        H264AddNaluSize(pEncOut, s);
    }

    pEncOut->motionVectors = (i8*)pEncInst->asic.mvOutput.virtualAddress;

    /* After stream buffer overflow discard the coded frame */
    if(pEncInst->stream.overflow == ENCHW_YES)
    {
        if ((pEncInst->numRefBuffsLum == 1) || (pEncInst->numViews == 2))
        {
            /* Only one reference frame buffer in use, so we can't use it
             * as reference => we must encode next frame as intra */
            pEncInst->encStatus = H264ENCSTAT_START_STREAM;
        }
        pEncOut->numNalus = 0;
        pEncOut->pNaluSizeBuf[0] = 0;
        APITRACE("H264EncStrmEncode: ERROR Output buffer too small");
        return H264ENC_OUTPUT_BUFFER_OVERFLOW;
    }

    /* Rate control action after vop */
    {
        i32 stat;

        pEncInst->rateControl.rcMSESum = regs->rcMSESum;

        stat = H264AfterPicRc(&pEncInst->rateControl, regs->rlcCount,
                              pEncInst->stream.byteCnt, regs->qpSum);
        /*H264MadThreshold(&pEncInst->mad, regs->madCount);*/

        /* After HRD overflow discard the coded frame and go back old time,
         * just like not coded frame. But if only one reference frame
         * buffer is in use we can't discard the frame unless the next frame
         * is coded as intra. */
        if((stat == H264RC_OVERFLOW) &&
           (pEncInst->numRefBuffsLum > 1) &&
           (pEncInst->numViews == 1))
        {
            pSlice->frameNum = pSlice->prevFrameNum;    /* revert frame_num */
            pEncOut->numNalus = 0;
            pEncOut->pNaluSizeBuf[0] = 0;
            APITRACE("H264EncStrmEncode: OK, Frame discarded (HRD overflow)");
            return H264ENC_FRAME_READY;
        }
    }

    /* Use the reconstructed frame as the reference for the next frame */
    EncAsicRecycleInternalImage(&pEncInst->asic, pEncInst->interlaced ? 2 : pEncInst->numViews,
                (pSlice->frameNum % 2), (pSlice->frameNum <= 1),
                pEncInst->numRefBuffsLum, pEncInst->numRefBuffsChr);

    /* Store the stream size and frame coding type in output structure */
    pEncOut->streamSize = pEncInst->stream.byteCnt;
    if(pSlice->nalUnitType == IDR) {
        pEncOut->codingType = H264ENC_INTRA_FRAME;
        pSlice->idrPicId += 1;
        if(pSlice->idrPicId == H264ENC_IDR_ID_MODULO)
            pSlice->idrPicId = 0;
        pEncOut->ipf = pEncOut->ltrf = 0;
    } else if (pSlice->sliceType == PSLICE) {
        pEncOut->codingType = H264ENC_PREDICTED_FRAME;
        if (picBuffer->refPicList[0].ipf)
        {
            pEncOut->ipf = picBuffer->refPicList[0].search ? H264ENC_REFERENCE : 0;
            pEncOut->ltrf = picBuffer->refPicList[1].search ? H264ENC_REFERENCE : 0;
        }
        else
        {
            pEncOut->ipf = picBuffer->refPicList[1].search ? H264ENC_REFERENCE : 0;
            pEncOut->ltrf = picBuffer->refPicList[0].search ? H264ENC_REFERENCE : 0;
        }
    } else {
        pEncOut->codingType = H264ENC_NONIDR_INTRA_FRAME;
    }

    /* Mark which reference frame was refreshed */
    pEncOut->ipf |= picBuffer->cur_pic->ipf ? H264ENC_REFRESH : 0;
    pEncOut->ltrf |= picBuffer->cur_pic->grf ? H264ENC_REFRESH : 0;

    pEncOut->mse_mul256 = regs->mse_mul256;

    if (pEncInst->svc.level)
    {
        H264UpdatePictureBufferForSvct(picBuffer, pSlice->frameNumBits, pEncInst->svc.gop[pEncInst->svc.gopIndex].markCmd);
    }
    else
    {
        H264UpdatePictureBuffer(picBuffer);
    }

    /* Frame was encoded so increment frame number */
    pEncInst->frameCnt++;
    /* When frame not used for reference, POC shouldn't increment. */
    if (!regs->recWriteDisable || pEncInst->numViews > 1) pSlice->frameNum++;
    pSlice->frameNum %= (1U << pSlice->frameNumBits);

    pEncInst->encStatus = H264ENCSTAT_START_FRAME;

    /* Force to Intra Frame because last compressed ref is overflow */
    if (pEncInst->rfcBufOverflow == ENCHW_YES)
        pEncInst->encStatus = H264ENCSTAT_START_STREAM;

    if(pEncInst->gdrEnabled == 1)
    {
        if(pEncInst->gdrFirstIntraFrame != 0)
        {
            pEncInst->gdrFirstIntraFrame--;
        }
        if(pEncInst->gdrStart)
            pEncInst->gdrCount++;

        if(pEncInst->gdrCount == (pEncInst->gdrDuration * (1 + (i32)pEncInst->interlaced)))
        {
            pEncInst->gdrStart--;
            pEncInst->gdrCount = 0;
        }
    }
    APITRACE("H264EncStrmEncode: OK");
    return H264ENC_FRAME_READY;
}

/*------------------------------------------------------------------------------

    Function name : H264EncStrmEnd
    Description   : Ends a stream
    Return type   : H264EncRet
    Argument      : inst - encoder instance
    Argument      : pEncIn - user provided input parameters
                    pEncOut - place where output info is returned
------------------------------------------------------------------------------*/
H264EncRet H264EncStrmEnd(H264EncInst inst, const H264EncIn * pEncIn,
                          H264EncOut * pEncOut)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;

    APITRACE("H264EncStrmEnd#");
    APITRACEPARAM("busLuma", pEncIn->busLuma);
    APITRACEPARAM("busChromaU", pEncIn->busChromaU);
    APITRACEPARAM("busChromaV", pEncIn->busChromaV);
    APITRACEPARAM("pOutBuf", pEncIn->pOutBuf);
    APITRACEPARAM("busOutBuf", pEncIn->busOutBuf);
    APITRACEPARAM("outBufSize", pEncIn->outBufSize);
    APITRACEPARAM("codingType", pEncIn->codingType);
    APITRACEPARAM("timeIncrement", pEncIn->timeIncrement);
    APITRACEPARAM("busLumaStab", pEncIn->busLumaStab);
    APITRACEPARAM("ipf", pEncIn->ipf);
    APITRACEPARAM("ltrf", pEncIn->ltrf);

    /* Check for illegal inputs */
    if((pEncInst == NULL) || (pEncIn == NULL) || (pEncOut == NULL))
    {
        APITRACE("H264EncStrmEnd: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    /* Check for existing instance */
    if(pEncInst->inst != pEncInst)
    {
        APITRACE("H264EncStrmEnd: ERROR Invalid instance");
        return H264ENC_INSTANCE_ERROR;
    }

    /* Check status, this also makes sure that the instance is valid */
    if((pEncInst->encStatus != H264ENCSTAT_START_FRAME) &&
       (pEncInst->encStatus != H264ENCSTAT_START_STREAM))
    {
        APITRACE("H264EncStrmEnd: ERROR Invalid status");
        return H264ENC_INVALID_STATUS;
    }

    pEncOut->streamSize = 0;

    /* Set pointer to the beginning of NAL unit size buffer */
    pEncOut->pNaluSizeBuf = (u32 *) pEncInst->asic.sizeTbl.virtualAddress;
    pEncOut->numNalus = 0;

    /* Clear the NAL unit size table */
    if(pEncOut->pNaluSizeBuf != NULL)
        pEncOut->pNaluSizeBuf[0] = 0;

    /* Check for invalid input values */
    if(pEncIn->pOutBuf == NULL ||
       (pEncIn->outBufSize < H264ENCSTRMSTART_MIN_BUF))
    {
        APITRACE("H264EncStrmEnd: ERROR Invalid input. Stream buffer");
        return H264ENC_INVALID_ARGUMENT;
    }

    /* Set stream buffer and check the size */
    if(H264SetBuffer(&pEncInst->stream, (u8 *) pEncIn->pOutBuf,
                     (u32) pEncIn->outBufSize) != ENCHW_OK)
    {
        APITRACE("H264EncStrmEnd: ERROR Output buffer too small");
        return H264ENC_INVALID_ARGUMENT;
    }

    /* Write end-of-stream code */
    H264EndOfSequence(&pEncInst->stream, &pEncInst->seqParameterSet);

    /* Bytes generated */
    pEncOut->streamSize = pEncInst->stream.byteCnt;
    H264AddNaluSize(pEncOut, pEncInst->stream.byteCnt);

    /* Status == INIT   Stream ended, next stream can be started */
    pEncInst->encStatus = H264ENCSTAT_INIT;

    APITRACE("H264EncStrmEnd: OK");
    return H264ENC_OK;
}

/*------------------------------------------------------------------------------

    Function name : H264AddNaluSize
    Description   : Adds the size of a NAL unit into NAL size output buffer.

    Return type   : void
    Argument      : pEncOut - encoder output structure
    Argument      : naluSizeBytes - size of the NALU in bytes
------------------------------------------------------------------------------*/
void H264AddNaluSize(H264EncOut * pEncOut, u32 naluSizeBytes)
{
    if(pEncOut->pNaluSizeBuf != NULL)
    {
        pEncOut->pNaluSizeBuf[pEncOut->numNalus++] = naluSizeBytes;
        pEncOut->pNaluSizeBuf[pEncOut->numNalus] = 0;
    }
}

/*------------------------------------------------------------------------------

        H264PrefixSvcNal

------------------------------------------------------------------------------*/
void H264PrefixNal(h264Instance_s *pEncInst, bool svcExtFlag)
{
    i32 nalRefIdc = 1;

    if (svcExtFlag)
    {
        svc_s *svc = &pEncInst->svc;
        u32 layer = (svc->gopIndex==-1)?(0):(svc->gop[svc->gopIndex].layer);
        if ( svc->level > 0 && layer == svc->level )
        {
            nalRefIdc = 0;
        }
    }

    H264NalUnitHdr(&pEncInst->stream,    nalRefIdc, PREFIX,
                    pEncInst->seqParameterSet.byteStream);

    H264PutBits(&pEncInst->stream, svcExtFlag, 1);
    COMMENT("svc_extension_flag");

    if (svcExtFlag)
    {
        H264NalUnitHdrSvcExtension(&pEncInst->stream, &pEncInst->svc);
    }
    else
    {
        pEncInst->mvc.anchorPicFlag = (pEncInst->slice.nalUnitType == IDR);
        pEncInst->mvc.priorityId = 0;
        pEncInst->mvc.viewId = 0;
        pEncInst->mvc.temporalId = 0;
        pEncInst->mvc.interViewFlag = 1;

        H264NalUnitHdrMvcExtension(&pEncInst->stream, &pEncInst->mvc);
    }
}

/*------------------------------------------------------------------------------

        H264PrefixSvcNal

------------------------------------------------------------------------------*/
void H264SvcSeiNal(h264Instance_s *pEncInst)
{
    h264RateControl_s *rc = &pEncInst->rateControl;
    H264NalUnitHdr(&pEncInst->stream, 0, SEI,
                    pEncInst->seqParameterSet.byteStream);

    H264ScalabilityInfoSei(&pEncInst->stream, pEncInst->svc.level, rc->outRateNum*256/rc->outRateDenom);
    H264RbspTrailingBits(&pEncInst->stream);
}
/*------------------------------------------------------------------------------
    Function name : H264EncGetBitsPerPixel
    Description   : Returns the amount of bits per pixel for given format.
    Return type   : u32
------------------------------------------------------------------------------*/
u32 H264EncGetBitsPerPixel(H264EncPictureType type)
{
    switch (type)
    {
        case H264ENC_YUV420_PLANAR:
        case H264ENC_YUV420_SEMIPLANAR:
        case H264ENC_YUV420_SEMIPLANAR_VU:
            return 12;
        case H264ENC_YUV422_INTERLEAVED_YUYV:
        case H264ENC_YUV422_INTERLEAVED_UYVY:
        case H264ENC_RGB565:
        case H264ENC_BGR565:
        case H264ENC_RGB555:
        case H264ENC_BGR555:
        case H264ENC_RGB444:
        case H264ENC_BGR444:
            return 16;
        case H264ENC_RGB888:
        case H264ENC_BGR888:
        case H264ENC_RGB101010:
        case H264ENC_BGR101010:
            return 32;
        case H264ENC_P010:
        case H264ENC_SP_101010:
            return 24;
        default:
            return 0;
    }
}

/*------------------------------------------------------------------------------
    Function name : H264EncSetTestId
    Description   : Sets the encoder configuration according to a test vector
    Return type   : H264EncRet
    Argument      : inst - encoder instance
    Argument      : testId - test vector ID
------------------------------------------------------------------------------*/
H264EncRet H264EncSetTestId(H264EncInst inst, u32 testId)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;
    (void) pEncInst;
    (void) testId;

    APITRACE("H264EncSetTestId#");

#ifdef INTERNAL_TEST
    pEncInst->testId = testId;

    APITRACE("H264EncSetTestId# OK");
    return H264ENC_OK;
#else
    /* Software compiled without testing support, return error always */
    APITRACE("H264EncSetTestId# ERROR, testing disabled at compile time");
    return H264ENC_ERROR;
#endif
}

/*------------------------------------------------------------------------------
    Function name : H264EncGetMbInfo
    Description   : Set the motionVectors field of H264EncOut structure to
                    point macroblock mbNum
    Return type   : H264EncRet
    Argument      : inst - encoder instance
    Argument      : mbNum - macroblock number
------------------------------------------------------------------------------*/
H264EncRet H264EncGetMbInfo(H264EncInst inst, H264EncOut * pEncOut, u32 mbNum)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;

    APITRACE("H264EncSetTestId#");

    /* Check for illegal inputs */
    if (!pEncInst || !pEncOut) {
        APITRACE("H264EncSetTestId: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    if (mbNum >= pEncInst->mbPerFrame) {
        APITRACE("H264EncSetTestId: ERROR Invalid argument");
        return H264ENC_INVALID_ARGUMENT;
    }

    pEncOut->motionVectors = (i8 *)EncAsicGetMvOutput(&pEncInst->asic, mbNum);

    return H264ENC_OK;
}

/*------------------------------------------------------------------------------
    Function name : H264EncGetInputBufferConsumed
    Description   : Get how many MB lines has been encoded by encoder.
    Return type   : int
    Argument      : inst - encoder instance
------------------------------------------------------------------------------*/
u32 H264EncGetEncodedMbLines(H264EncInst inst)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;
    int lines;

    APITRACE("H264EncGetEncodedMbLines#");

    /* Check for illegal inputs */
    if (!pEncInst) {
        APITRACE("H264EncGetEncodedMbLines: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    if (!pEncInst->inputLineBuf.inputLineBufEn) {
        APITRACE("H264EncGetEncodedMbLines: ERROR Invalid mode for input control");
        return H264ENC_INVALID_ARGUMENT;
    }
    lines = EncAsicGetRegisterValue(pEncInst->asic.ewl, pEncInst->asic.regs.regMirror, HEncMbRdPtr);
    return lines;
}

/*------------------------------------------------------------------------------
    Function name : H264EncSetInputMbLines
    Description   : Set the input buffer lines available of current picture.
    Return type   : H264EncRet
    Argument      : inst - encoder instance
    Argument      : mbNum - macroblock number
------------------------------------------------------------------------------*/
H264EncRet H264EncSetInputMbLines(H264EncInst inst, u32 lines)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;

    APITRACE("H264EncSetInputMBLines#");

    /* Check for illegal inputs */
    if (!pEncInst) {
        APITRACE("H264EncSetInputMbLines: ERROR Null argument");
        return H264ENC_NULL_ARGUMENT;
    }

    if (!pEncInst->inputLineBuf.inputLineBufEn) {
        APITRACE("H264EncSetInputMbLines: ERROR Invalid mode for input control");
        return H264ENC_INVALID_ARGUMENT;
    }
    EncAsicWriteRegisterValue(pEncInst->asic.ewl, pEncInst->asic.regs.regMirror, HEncMbWrPtr, lines);
    return H264ENC_OK;
}

/*------------------------------------------------------------------------------
    Function name   : H264SupplyPrefixForSvctMutiSlice
    Description     : Supply prefix for svct muti-slice.
    Return type     : void
    Argument        : inst - encoder instance
    Argument        : *pEncIn - user provided input parameters
    Argument        : *pEncOut - place where output info is returned
    Argument        : prefix_length - the length of prefix nalu
    Argument        : prefix_nalu_addr - prefix nalu addr
------------------------------------------------------------------------------*/
void H264SupplyPrefixForSvctMutiSlice(H264EncInst inst, const H264EncIn *pEncIn, H264EncOut *pEncOut,
                                                      u32 prefix_length, u8 *prefix_nalu_addr)
{
    h264Instance_s *pEncInst = (h264Instance_s *) inst;
    if (!pEncInst) {
        APITRACE("H264SupplyPrefixForSvctMutiSlice: ERROR Null argument");
        return;
    }
    if(!pEncInst->slice.sliceSize)
    {
        APITRACE("H264SupplyPrefixForSvctMutiSlice: Prifix is not added for singal slice.");
        return;
    }
    ASSERT((prefix_length != 0) && (prefix_nalu_addr != NULL) && (pEncOut->streamSize != 0));

    u32 num_prefix_to_supply = (pEncInst->mbPerFrame + pEncInst->slice.sliceSize - 1) / pEncInst->slice.sliceSize - 1;//how many prefixs lack.
    if(pEncOut->streamSize + prefix_length * num_prefix_to_supply > pEncIn->outBufSize)
    {
        APITRACE("H264SupplyPrefixForSvctMutiSlice: The output buffer overflows.");
        pEncInst->stream.overflow = ENCHW_YES;
        return;
    }

    u32 i = 0, j = 0, copy_times = 0, data_left = 0;
    u32 num_nalus = pEncOut->numNalus;
    u8 *data_addr_tail = (u8 *)pEncIn->pOutBuf + pEncOut->streamSize;
    u8 *copy_src = data_addr_tail;
    u8 *copy_dst = data_addr_tail + prefix_length * num_prefix_to_supply;//spare n prefix_length
    u8 *prefix_data = prefix_nalu_addr;

    /* 1.insert prefix data and move slice data. */
    for(j = 0; j < num_prefix_to_supply; j++)
    {
        copy_times = pEncOut->pNaluSizeBuf[num_nalus - 1 - j] / prefix_length;
        data_left = pEncOut->pNaluSizeBuf[num_nalus - 1 - j] % prefix_length;
        copy_src -= prefix_length;
        copy_dst -= prefix_length;
        ASSERT(copy_dst >= (u8 *)pEncIn->pOutBuf);

        for(i = 0; i < copy_times; i++)
        {
            memcpy(copy_dst, copy_src, prefix_length);
            if(i == copy_times-1)
            {
                copy_src -= data_left;
                copy_dst -= data_left;
            }
            else
            {
                copy_src -= prefix_length;
                copy_dst -= prefix_length;
            }
        }
        if(data_left)
            memcpy(copy_dst, copy_src, data_left);
        copy_dst -= prefix_length;
        memcpy(copy_dst, prefix_data, prefix_length);
    }

    /* 2.update H264EncOut pEncInst state. */
    for(j = 0; j < num_prefix_to_supply; j++)
    {
        ASSERT(&pEncOut->pNaluSizeBuf[num_nalus - 1 + num_prefix_to_supply -j*2]);
        pEncOut->pNaluSizeBuf[num_nalus-1+num_prefix_to_supply-j*2 ] = pEncOut->pNaluSizeBuf[num_nalus-1-j];
        pEncOut->pNaluSizeBuf[num_nalus - 1 + num_prefix_to_supply - j*2 - 1] = prefix_length;
        pEncOut->streamSize += prefix_length;
        pEncOut->numNalus++;
    }
    pEncOut->pNaluSizeBuf[pEncOut->numNalus] = 0;

    pEncInst->stream.stream += prefix_length * num_prefix_to_supply;
    pEncInst->stream.byteCnt += prefix_length * num_prefix_to_supply;
}

/*------------------------------------------------------------------------------
    Function name   : H264AccessUnitDelimiter
    Description     : Add AUD
    Return type     : void
    Argument        : u32 byte_stream
    Argument        : u32 primary_pic_type - primary_pic_type
------------------------------------------------------------------------------*/
void H264AccessUnitDelimiter(stream_s *b, u32 byte_stream, u32 primary_pic_type)
{
    H264NalUnitHdr(b, 0, AUD_NUT, byte_stream);
    H264PutBits(b, primary_pic_type, 3);
    COMMENT("primary_pic_type");
    H264RbspTrailingBits(b);
}

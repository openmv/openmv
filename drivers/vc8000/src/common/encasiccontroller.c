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
 *  Description : ASIC low level controller
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------
    Include headers
------------------------------------------------------------------------------*/

#include "enccommon.h"
#include "encasiccontroller.h"
#include "encpreprocess.h"
#include "ewl.h"
#include "encswhwregisters.h"

/*------------------------------------------------------------------------------
    External compiler flags
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

#ifdef ASIC_WAVE_TRACE_TRIGGER
extern i32 trigger_point;    /* picture which will be traced */
#endif

/* Mask fields */
#define mask_2b         (u32)0x00000003
#define mask_3b         (u32)0x00000007
#define mask_4b         (u32)0x0000000F
#define mask_5b         (u32)0x0000001F
#define mask_6b         (u32)0x0000003F
#define mask_7b         (u32)0x0000007F
#define mask_8b         (u32)0x000000FF
#define mask_11b        (u32)0x000007FF
#define mask_14b        (u32)0x00003FFF
#define mask_16b        (u32)0x0000FFFF

#define HSWREG(n)       ((n)*4)

/* JPEG QUANT table order */
static const u32 qpReorderTable[64] =
    { 0,  8, 16, 24,  1,  9, 17, 25, 32, 40, 48, 56, 33, 41, 49, 57,
      2, 10, 18, 26,  3, 11, 19, 27, 34, 42, 50, 58, 35, 43, 51, 59,
      4, 12, 20, 28,  5, 13, 21, 29, 36, 44, 52, 60, 37, 45, 53, 61,
      6, 14, 22, 30,  7, 15, 23, 31, 38, 46, 54, 62, 39, 47, 55, 63
};


/* Global variables for test data trace file configuration.
 * These are set in testbench and used in system model test data generation. */
char * H1EncTraceFileConfig = NULL;
int H1EncTraceFirstFrame    = 0;

/*------------------------------------------------------------------------------
    Local function prototypes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Initialize empty structure with default values.
------------------------------------------------------------------------------*/
i32 EncAsicControllerInit(asicData_s * asic)
{
    ASSERT(asic != NULL);

    /* Initialize default values from defined configuration */
    asic->regs.irqDisable = ENCH1_IRQ_DISABLE;
    asic->regs.inputReadChunk = ENCH1_INPUT_READ_CHUNK;

    asic->regs.asicCfgReg =
        ((ENCH1_AXI_WRITE_ID & (255)) << 24) |
        ((ENCH1_AXI_READ_ID & (255)) << 16) |
        ((ENCH1_OUTPUT_SWAP_16 & (1)) << 15) |
        ((ENCH1_BURST_LENGTH & (63)) << 8) |
        ((ENCH1_BURST_SCMD_DISABLE & (1)) << 7) |
        ((ENCH1_BURST_INCR_TYPE_ENABLED & (1)) << 6) |
        ((ENCH1_BURST_DATA_DISCARD_ENABLED & (1)) << 5) |
        ((ENCH1_ASIC_CLOCK_GATING_ENABLED & (1)) << 4) |
        ((ENCH1_OUTPUT_SWAP_32 & (1)) << 3) |
        ((ENCH1_OUTPUT_SWAP_8 & (1)) << 1);

    asic->regs.recWriteBuffer = ENCH1_REC_WRITE_BUFFER & 1;

    /* Initialize default values */
    asic->regs.roundingCtrl = 0;
    asic->regs.cpDistanceMbs = 0;
    asic->regs.reconImageId = 0;

    /* User must set these */
    asic->regs.inputLumBase = 0;
    asic->regs.inputCbBase = 0;
    asic->regs.inputCrBase = 0;

    asic->internalImageLuma[0].virtualAddress = NULL;
    asic->internalImageChroma[0].virtualAddress = NULL;
    asic->internalImageLuma[1].virtualAddress = NULL;
    asic->internalImageChroma[1].virtualAddress = NULL;
    asic->internalImageLuma[2].virtualAddress = NULL;
    asic->internalImageChroma[2].virtualAddress = NULL;
    asic->internalImageChroma[3].virtualAddress = NULL;
    asic->scaledImage.virtualAddress = NULL;
    asic->sizeTbl.virtualAddress = NULL;
    asic->cabacCtx.virtualAddress = NULL;
    asic->mvOutput.virtualAddress = NULL;
    asic->probCount.virtualAddress = NULL;
    asic->segmentMap.virtualAddress = NULL;

#ifdef ASIC_WAVE_TRACE_TRIGGER
    asic->regs.vop_count = 0;
#endif

    /* get ASIC ID value */
    asic->regs.asicHwId = EncAsicGetId(asic->ewl);

/* we do NOT reset hardware at this point because */
/* of the multi-instance support                  */

    return ENCHW_OK;
}


/*------------------------------------------------------------------------------

    EncAsicSetQuantTable

    Set new jpeg quantization table to be used by ASIC

------------------------------------------------------------------------------*/
void EncAsicSetQuantTable(asicData_s * asic,
                          const u8 * lumTable, const u8 * chTable)
{
    i32 i;

    ASSERT(lumTable);
    ASSERT(chTable);

    for(i = 0; i < 64; i++)
    {
        asic->regs.quantTable[i] = lumTable[qpReorderTable[i]];
    }
    for(i = 0; i < 64; i++)
    {
        asic->regs.quantTable[64 + i] = chTable[qpReorderTable[i]];
    }
}

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
u32 EncAsicGetId(const void *ewl)
{
    return EWLReadReg(ewl, 0x0);
}

/*------------------------------------------------------------------------------
    When the frame is successfully encoded the internal image is recycled
    so that during the next frame the current internal image is read and
    the new reconstructed image is written. Note that this is done for
    the NEXT frame.
------------------------------------------------------------------------------*/
#define NEXTID(currentId, maxId) ((currentId == maxId) ? 0 : currentId+1)
#define PREVID(currentId, maxId) ((currentId == 0) ? maxId : currentId-1)

void EncAsicRecycleInternalImage(asicData_s *asic, u32 numViews, u32 viewId,
        u32 anchor, u32 numRefBuffsLum, u32 numRefBuffsChr)
{
    u32 id;
    ptr_t tmp;

    if (numViews == 1)
    {
        /* H.264 base view stream, just swap buffers */
        tmp = asic->regs.internalImageLumBaseW;
        asic->regs.internalImageLumBaseW = asic->regs.internalImageLumBaseR[0];
        asic->regs.internalImageLumBaseR[0] = tmp;

        tmp = asic->regs.internalImageChrBaseW;
        asic->regs.internalImageChrBaseW = asic->regs.internalImageChrBaseR[0];
        asic->regs.internalImageChrBaseR[0] = tmp;
    }
    else
    {
        /* MVC stereo stream, the buffer amount tells if the second view
         * is inter-view or inter coded. This affects how the buffers
         * should be swapped. */

        if ((viewId == 0) && (anchor || (numRefBuffsLum == 1)))
        {
            /* Next frame is viewId=1 with inter-view prediction */
            asic->regs.internalImageLumBaseR[0] = asic->internalImageLuma[0].busAddress;
            asic->regs.internalImageLumBaseW = asic->internalImageLuma[1].busAddress;
        }
        else if (viewId == 0)
        {
            /* Next frame is viewId=1 with inter prediction */
            asic->regs.internalImageLumBaseR[0] = asic->internalImageLuma[1].busAddress;
            asic->regs.internalImageLumBaseW = asic->internalImageLuma[1].busAddress;
        }
        else
        {
            /* Next frame is viewId=0 with inter prediction */
            asic->regs.internalImageLumBaseR[0] = asic->internalImageLuma[0].busAddress;
            asic->regs.internalImageLumBaseW = asic->internalImageLuma[0].busAddress;
        }

        if (numRefBuffsChr == 2)
        {
            if (viewId == 0)
            {
                /* For inter-view prediction chroma is swapped after base view only */
                tmp = asic->regs.internalImageChrBaseW;
                asic->regs.internalImageChrBaseW = asic->regs.internalImageChrBaseR[0];
                asic->regs.internalImageChrBaseR[0] = tmp;
            }
        }
        else
        {
            /* For viewId==1 the anchor frame uses inter-view prediction,
             * all other frames use inter-prediction. */
            if ((viewId == 0) && anchor)
                id = asic->regs.reconImageId;
            else
                id = PREVID(asic->regs.reconImageId, 2);

            asic->regs.internalImageChrBaseR[0] = asic->internalImageChroma[id].busAddress;
            asic->regs.reconImageId = id = NEXTID(asic->regs.reconImageId, 2);
            asic->regs.internalImageChrBaseW = asic->internalImageChroma[id].busAddress;
        }
    }
}

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void CheckRegisterValues(regValues_s * val)
{
    ASSERT(val->irqDisable <= 1);
    ASSERT(val->rlcLimitSpace / 2 < (1 << 20));
    ASSERT(val->mbsInCol <= 511);
    ASSERT(val->mbsInRow <= 511);
    ASSERT(val->filterDisable <= 2);
    ASSERT(val->recWriteBuffer <= 1);
    ASSERT(val->recWriteDisable <= 1);
    ASSERT(val->madThreshold[0] <= 63);
    ASSERT(val->madThreshold[1] <= 63);
    ASSERT(val->madThreshold[2] <= 63);
    ASSERT(val->madQpDelta[0] >= -8 && val->madQpDelta[0] <= 7);
    ASSERT(val->madQpDelta[1] >= -127 && val->madQpDelta[1] <= 127);
    ASSERT(val->madQpDelta[2] >= -127 && val->madQpDelta[2] <= 127);
    ASSERT(val->qp <= 63);
    ASSERT(val->constrainedIntraPrediction <= 1);
    ASSERT(val->roundingCtrl <= 1);
    ASSERT(val->frameCodingType <= 3);
    ASSERT(val->codingType <= 3);
    ASSERT(val->outputStrmSize <= 0x1FFFFFF);
    ASSERT(val->pixelsOnRow >= 16 && val->pixelsOnRow <= 8192); /* max input for cropping */
    ASSERT(val->xFill <= 3);
    ASSERT(val->yFill <= 14 && ((val->yFill & 0x01) == 0));
    ASSERT(val->inputLumaBaseOffset <= 7);
    ASSERT(val->inputChromaBaseOffset <= 7);
    ASSERT(val->sliceAlphaOffset >= -6 && val->sliceAlphaOffset <= 6);
    ASSERT(val->sliceBetaOffset >= -6 && val->sliceBetaOffset <= 6);
    ASSERT(val->chromaQpIndexOffset >= -12 && val->chromaQpIndexOffset <= 12);
    ASSERT(val->sliceSizeMbRows <= 127);
    ASSERT(val->inputImageFormat <= ASIC_INPUT_P010);
    ASSERT(val->inputImageRotation <= 3);
    ASSERT(val->cpDistanceMbs <= 8191);
    if(val->codingType == ASIC_H264) {
        ASSERT(val->roi1DeltaQp >= 0 && val->roi1DeltaQp <= 15);
        ASSERT(val->roi2DeltaQp >= 0 && val->roi2DeltaQp <= 15);
    }
    ASSERT(val->cirStart <= 65535);
    ASSERT(val->cirInterval <= 65535);
    ASSERT(val->intraAreaTop <= 255);
    ASSERT(val->intraAreaLeft <= 255);
    ASSERT(val->intraAreaBottom <= 255);
    ASSERT(val->intraAreaRight <= 255);
    ASSERT(val->roi1Top <= 255);
    ASSERT(val->roi1Left <= 255);
    ASSERT(val->roi1Bottom <= 255);
    ASSERT(val->roi1Right <= 255);
    ASSERT(val->roi2Top <= 255);
    ASSERT(val->roi2Left <= 255);
    ASSERT(val->roi2Bottom <= 255);
    ASSERT(val->roi2Right <= 255);
    ASSERT(val->boostQp <= 52);
    ASSERT(val->boostVar1 <= 8191);
    ASSERT(val->boostVar2 <= 8191);
    ASSERT(val->varLimitDiv32 <= 15);
    ASSERT(val->varInterFavorDiv16 <= 15);
    ASSERT(val->varMultiplier <= 31);
    ASSERT(val->varAdd <= 7);
    ASSERT(val->pskipMode <= 1);

    (void) val;
}

/*------------------------------------------------------------------------------
    Function name   : EncAsicFrameStart
    Description     : 
    Return type     : void 
    Argument        : const void *ewl
    Argument        : regValues_s * val
------------------------------------------------------------------------------*/
void EncAsicFrameStart(const void *ewl, regValues_s * val)
{
    i32 i;
    u32 asicCfgReg = val->asicCfgReg;

    CheckRegisterValues(val);

    EWLmemset(val->regMirror, 0, sizeof(val->regMirror));

#ifdef INTERNAL_TEST
    /* With internal testing use random values for burst mode.
       Random value taken from squared error register of previous frame. */
    asicCfgReg = (asicCfgReg & ~0xE0) | (EWLReadReg(ewl, 0x3b0) & 0xE0);

    /* Also test both values of recon output buffer. */
    val->recWriteBuffer = val->frameNum & 1;
#endif

    /* must burst for RFC */
    if (val->refLumCompress || val->refChrCompress)
        asicCfgReg &= (~0x80);

    /* encoder interrupt */
    EncAsicSetRegisterValue(val->regMirror, HEncIRQDisable, val->irqDisable);
    /* clear all interrupt */
    if (EWLReadReg(ewl, BASE_HWFuse2) & HWCFGIrqClearSupport)
    {
        EncAsicSetRegisterValue(val->regMirror,     HEncIRQRfcBufOverflow, 1);
        EncAsicSetRegisterValue(val->regMirror,     HEncIRQLowLatency, 1);
        EncAsicSetRegisterValue(val->regMirror,     HEncIRQFuse, 1);
        EncAsicSetRegisterValue(val->regMirror,     HEncIRQSliceReady, 1);
        EncAsicSetRegisterValue(val->regMirror,     HEncIRQTimeout, 1);
        EncAsicSetRegisterValue(val->regMirror,     HEncIRQBuffer, 1);
        EncAsicSetRegisterValue(val->regMirror,     HEncIRQReset, 1);
        EncAsicSetRegisterValue(val->regMirror,     HEncIRQBusError, 1);
        EncAsicSetRegisterValue(val->regMirror,     HEncIRQFrameReady, 1);
        EncAsicSetRegisterValue(val->regMirror,     HEncIRQ, 1);
    }

    /* system configuration */
    if ( (val->inputImageFormat < ASIC_INPUT_RGB565) || 
         (val->inputImageFormat == ASIC_INPUT_SP_101010) ||
         (val->inputImageFormat == ASIC_INPUT_P010) )     /* YUV input */
        val->regMirror[2] = asicCfgReg |
            ((ENCH1_INPUT_SWAP_16_YUV & (1)) << 14) |
            ((ENCH1_INPUT_SWAP_32_YUV & (1)) << 2) |
            (ENCH1_INPUT_SWAP_8_YUV & (1));
    else if (val->inputImageFormat < ASIC_INPUT_RGB888) /* 16-bit RGB input */
        val->regMirror[2] = asicCfgReg |
            ((ENCH1_INPUT_SWAP_16_RGB16 & (1)) << 14) |
            ((ENCH1_INPUT_SWAP_32_RGB16 & (1)) << 2) |
            (ENCH1_INPUT_SWAP_8_RGB16 & (1));
    else    /* 32-bit RGB input */
        val->regMirror[2] = asicCfgReg |
            ((ENCH1_INPUT_SWAP_16_RGB32 & (1)) << 14) |
            ((ENCH1_INPUT_SWAP_32_RGB32 & (1)) << 2) |
            (ENCH1_INPUT_SWAP_8_RGB32 & (1));

    EncAsicSetRegisterValue(val->regMirror, HEncScaleOutputSwap8,  ENCH1_SCALE_OUTPUT_SWAP_8);
    EncAsicSetRegisterValue(val->regMirror, HEncScaleOutputSwap16, ENCH1_SCALE_OUTPUT_SWAP_16);
    EncAsicSetRegisterValue(val->regMirror, HEncScaleOutputSwap32, ENCH1_SCALE_OUTPUT_SWAP_32);
    EncAsicSetRegisterValue(val->regMirror, HEncMvOutputSwap8,  ENCH1_MV_OUTPUT_SWAP_8);
    EncAsicSetRegisterValue(val->regMirror, HEncMvOutputSwap16, ENCH1_MV_OUTPUT_SWAP_16);
    EncAsicSetRegisterValue(val->regMirror, HEncMvOutputSwap32, ENCH1_MV_OUTPUT_SWAP_32);
    EncAsicSetRegisterValue(val->regMirror, HEncInputReadChunk, val->inputReadChunk);
    EncAsicSetRegisterValue(val->regMirror, HEncTestIrq, val->traceMbTiming);
    EncAsicSetRegisterValue(val->regMirror, HEncAXIDualCh, ENCH1_AXI_2CH_DISABLE);

    /* set axi read ID for DEC400 8 */
    EncAsicSetRegisterValue(val->regMirror, HEncAXIReadIDInC0, ENCH1_AXI_READ_ID_C0);
    EncAsicSetRegisterValue(val->regMirror, HEncAXIReadIDInC1, ENCH1_AXI_READ_ID_C1);
    EncAsicSetRegisterValue(val->regMirror, HEncAXIReadIDInC2, ENCH1_AXI_READ_ID_C2);
    EncAsicSetRegisterValue(val->regMirror, HEncAXIReadIDEnable, ENCH1_AXI_READ_ID_EN);

    /* output stream buffer */
    SET_ADDR_REG(val->regMirror, HEncBaseStream, val->outputStrmBase);

    /* Video encoding output buffers and reference picture buffers */
    if(val->codingType != ASIC_JPEG)
    {
        SET_ADDR_REG(val->regMirror, HEncBaseControl, val->sizeTblBase);
        EncAsicSetRegisterValue(val->regMirror, HEncNalSizeWrite, val->sizeTblBase != 0);
        EncAsicSetRegisterValue(val->regMirror, HEncMvWrite, val->mvOutEnable);

        SET_ADDR_REG(val->regMirror, HEncBaseRefLum, val->internalImageLumBaseR[0]);
        SET_ADDR_REG(val->regMirror, HEncBaseRefChr, val->internalImageChrBaseR[0]);
        SET_ADDR_REG(val->regMirror, HEncBaseRecLum, val->internalImageLumBaseW);
        SET_ADDR_REG(val->regMirror, HEncBaseRecChr, val->internalImageChrBaseW);
    }

    /* Input picture buffers */
    SET_ADDR_REG(val->regMirror, HEncBaseInLum, val->inputLumBase);
    SET_ADDR_REG(val->regMirror, HEncBaseInCb, val->inputCbBase);
    SET_ADDR_REG(val->regMirror, HEncBaseInCr, val->inputCrBase);

    /* Common control register */
    if (val->lineBufferEn)
        EncAsicSetRegisterValue(val->regMirror, HEncIntTimeout, 0); 
    else
        EncAsicSetRegisterValue(val->regMirror, HEncIntTimeout, ENCH1_TIMEOUT_INTERRUPT&1);
    EncAsicSetRegisterValue(val->regMirror, HEncIntSliceReady, val->sliceReadyInterrupt);
    EncAsicSetRegisterValue(val->regMirror, HEncRecWriteBuffer, val->recWriteBuffer);
    EncAsicSetRegisterValue(val->regMirror, HEncRecWriteDisable, val->recWriteDisable);
    EncAsicSetRegisterValue(val->regMirror, HEncWidth, val->mbsInRow);
    EncAsicSetRegisterValue(val->regMirror, HEncHeight, val->mbsInCol);
    EncAsicSetRegisterValue(val->regMirror, HEncPictureType, val->frameCodingType);
    EncAsicSetRegisterValue(val->regMirror, HEncEncodingMode, val->codingType);
    EncAsicSetRegisterValue(val->regMirror, HEncRefReorderFlag, val->reorderFlag);
    EncAsicSetRegisterValue(val->regMirror, HEncRefReorderDiffPicNum, val->reorderDiff);

    /* PreP control */
    EncAsicSetRegisterValue(val->regMirror, HEncChrOffset, val->inputChromaBaseOffset);
    EncAsicSetRegisterValue(val->regMirror, HEncLumOffset, val->inputLumaBaseOffset);
    EncAsicSetRegisterValue(val->regMirror, HEncRowLength, val->pixelsOnRow);
    EncAsicSetRegisterValue(val->regMirror, HEncXFill, val->xFill);
    EncAsicSetRegisterValue(val->regMirror, HEncYFill, val->yFill);
    EncAsicSetRegisterValue(val->regMirror, HEncInputFormat, val->inputImageFormat);
    EncAsicSetRegisterValue(val->regMirror, HEncInputRot, val->inputImageRotation);

    /* Common controls */
    EncAsicSetRegisterValue(val->regMirror, HEncCabacEnable, val->enableCabac);
    EncAsicSetRegisterValue(val->regMirror, HEncIPIntra16Favor, val->pen[0][ASIC_PENALTY_I16FAVOR]);
    EncAsicSetRegisterValue(val->regMirror, HEncInterFavor, val->pen[0][ASIC_PENALTY_INTER_FAVOR] & mask_16b);
    EncAsicSetRegisterValue(val->regMirror, HEncDisableQPMV, val->disableQuarterPixelMv);
    EncAsicSetRegisterValue(val->regMirror, HEncDeblocking, val->filterDisable);
    EncAsicSetRegisterValue(val->regMirror, HEncSkipPenalty, val->pen[0][ASIC_PENALTY_SKIP]);
    EncAsicSetRegisterValue(val->regMirror, HEncChromaSwap, val->chromaSwap);
    EncAsicSetRegisterValue(val->regMirror, HEncSplitMv, val->splitMvMode);
    EncAsicSetRegisterValue(val->regMirror, HEncSplitPenalty16x8, val->pen[0][ASIC_PENALTY_SPLIT16x8]);
    EncAsicSetRegisterValue(val->regMirror, HEncSplitPenalty8x8, val->pen[0][ASIC_PENALTY_SPLIT8x8]);
    EncAsicSetRegisterValue(val->regMirror, HEncSplitPenalty8x4, val->pen[0][ASIC_PENALTY_SPLIT8x4]);
    EncAsicSetRegisterValue(val->regMirror, HEncSplitPenalty4x4, val->pen[0][ASIC_PENALTY_SPLIT4x4]);
    EncAsicSetRegisterValue(val->regMirror, HEncSplitZeroPenalty, val->pen[0][ASIC_PENALTY_SPLIT_ZERO]);
    EncAsicSetRegisterValue(val->regMirror, HEncZeroMvFavor, val->zeroMvFavorDiv2);

    /* H.264 specific control */
    if (val->codingType == ASIC_H264)
    {
        EncAsicSetRegisterValue(val->regMirror, HEncPicInitQp, val->picInitQp);
        EncAsicSetRegisterValue(val->regMirror, HEncSliceAlpha, val->sliceAlphaOffset & mask_4b);
        EncAsicSetRegisterValue(val->regMirror, HEncSliceBeta, val->sliceBetaOffset & mask_4b);
        EncAsicSetRegisterValue(val->regMirror, HEncChromaQp, val->chromaQpIndexOffset & mask_5b);
        EncAsicSetRegisterValue(val->regMirror, HEncIdrPicId, val->idrPicId);
        EncAsicSetRegisterValue(val->regMirror, HEncConstrIP, val->constrainedIntraPrediction);

        EncAsicSetRegisterValue(val->regMirror, HEncPPSID, val->ppsId);
        EncAsicSetRegisterValue(val->regMirror, HEncIPPrevModeFavor, val->pen[0][ASIC_PENALTY_I4_PREV_MODE_FAVOR]);

        EncAsicSetRegisterValue(val->regMirror, HEncSliceSize, val->sliceSizeMbRows);
        EncAsicSetRegisterValue(val->regMirror, HEncTransform8x8, val->transform8x8Mode);
        EncAsicSetRegisterValue(val->regMirror, HEncCabacInitIdc, val->cabacInitIdc);
        EncAsicSetRegisterValue(val->regMirror, HEncInter4Restrict, val->h264Inter4x4Disabled);
        EncAsicSetRegisterValue(val->regMirror, HEncStreamMode, val->h264StrmMode);
        EncAsicSetRegisterValue(val->regMirror, HEncFrameNum, val->frameNum);
    }

    /* JPEG specific control */
    if (val->codingType == ASIC_JPEG)
    {
        EncAsicSetRegisterValue(val->regMirror, HEncJpegMode, val->jpegMode);
        EncAsicSetRegisterValue(val->regMirror, HEncJpegSlice, val->jpegSliceEnable);
        EncAsicSetRegisterValue(val->regMirror, HEncJpegRSTInt, val->jpegRestartInterval);
        EncAsicSetRegisterValue(val->regMirror, HEncJpegRST, val->jpegRestartMarker);
    }

    /* stream buffer limits */
    EncAsicSetRegisterValue(val->regMirror, HEncStrmHdrRem1, val->strmStartMSB);
    EncAsicSetRegisterValue(val->regMirror, HEncStrmHdrRem2, val->strmStartLSB);
    EncAsicSetRegisterValue(val->regMirror, HEncStrmBufLimit, val->outputStrmSize-8);

    /* video encoding rate control regs 0x6C - 0x90,
     * different register definitions for VP8 and H.264 */
    if(val->codingType != ASIC_JPEG)
    {
        EncAsicSetRegisterValue(val->regMirror, HEncMadQpDelta, val->madQpDelta[0] & mask_4b);
        EncAsicSetRegisterValue(val->regMirror, HEncMadThreshold, val->madThreshold[0]);
        EncAsicSetRegisterValue(val->regMirror, HEncMadQpDelta2, val->madQpDelta[1] & mask_8b);
        EncAsicSetRegisterValue(val->regMirror, HEncMadThreshold2, val->madThreshold[1]);
        EncAsicSetRegisterValue(val->regMirror, HEncMadQpDelta3, val->madQpDelta[2] & mask_8b);
        EncAsicSetRegisterValue(val->regMirror, HEncMadThreshold3, val->madThreshold[2]);

        if (val->codingType == ASIC_VP8)
        {
            SET_ADDR_REG(val->regMirror, HEncBaseRefLum2, val->internalImageLumBaseR[1]);
            SET_ADDR_REG(val->regMirror, HEncBaseRefChr2, val->internalImageChrBaseR[1]);

            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y1QuantDc, val->qpY1QuantDc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y1QuantAc, val->qpY1QuantAc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y2QuantDc, val->qpY2QuantDc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y2QuantAc, val->qpY2QuantAc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8ChQuantDc, val->qpChQuantDc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8ChQuantAc, val->qpChQuantAc[0]);

            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y1ZbinDc, val->qpY1ZbinDc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y1ZbinAc, val->qpY1ZbinAc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y2ZbinDc, val->qpY2ZbinDc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y2ZbinAc, val->qpY2ZbinAc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8ChZbinDc, val->qpChZbinDc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8ChZbinAc, val->qpChZbinAc[0]);

            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y1RoundDc, val->qpY1RoundDc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y1RoundAc, val->qpY1RoundAc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y2RoundDc, val->qpY2RoundDc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y2RoundAc, val->qpY2RoundAc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8ChRoundDc, val->qpChRoundDc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8ChRoundAc, val->qpChRoundAc[0]);

            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y1DequantDc, val->qpY1DequantDc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y1DequantAc, val->qpY1DequantAc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y2DequantDc, val->qpY2DequantDc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Y2DequantAc, val->qpY2DequantAc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8ChDequantDc, val->qpChDequantDc[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8ChDequantAc, val->qpChDequantAc[0]);

            EncAsicSetRegisterValue(val->regMirror, HEncVp8MvRefIdx, val->mvRefIdx[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8MvRefIdx2, val->mvRefIdx[1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Ref2Enable, val->ref2Enable);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8DeadzoneEnable, val->deadzoneEnable);

            EncAsicSetRegisterValue(val->regMirror, HEncVp8BoolEncValue, val->boolEncValue);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8BoolEncValueBits, val->boolEncValueBits);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8BoolEncRange, val->boolEncRange);

            EncAsicSetRegisterValue(val->regMirror, HEncVp8FilterLevel, val->filterLevel[0]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8GoldenPenalty, val->pen[0][ASIC_PENALTY_GOLDEN]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8FilterSharpness, val->filterSharpness);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8DctPartitionCount, val->dctPartitions);
        } else {    /* H.264 */
            EncAsicSetRegisterValue(val->regMirror, HEncQp, val->qp);
            EncAsicSetRegisterValue(val->regMirror, HEncMaxQp, val->qpMax);
            EncAsicSetRegisterValue(val->regMirror, HEncMinQp, val->qpMin);
            EncAsicSetRegisterValue(val->regMirror, HEncCPDist, 0);
            /* long term ref stuff */
            EncAsicSetRegisterValue(val->regMirror, HEncH264LongTermPenalty, val->pen[0][ASIC_PENALTY_GOLDEN]);
            EncAsicSetRegisterValue(val->regMirror, HEncH264Ref2Enable, val->ref2Enable);
            EncAsicSetRegisterValue(val->regMirror, HEncH264MvRefIdx,  val->mvRefIdx[0]);

            EncAsicSetRegisterValue(val->regMirror, HEncH264MarkCurrentLongTerm, val->markCurrentLongTerm);
            SET_ADDR_REG(val->regMirror, HEncH264BaseRefLum2,   val->internalImageLumBaseR[1]);
            SET_ADDR_REG(val->regMirror, HEncH264BaseRefChr2,   val->internalImageChrBaseR[1]);
        }

        /* Segment map registers for VP8 and H264. */
        EncAsicSetRegisterValue(val->regMirror, HEncVp8SegmentEnable, val->segmentEnable);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8SegmentMapUpdate, val->segmentMapUpdate);
        SET_ADDR_REG(val->regMirror, HEncBaseVp8SegmentMap,  val->segmentMapBase);

        /* Reference Frame Compression */
        EncAsicSetRegisterValue(val->regMirror, HEncRefLumCompress, val->refLumCompress);
        EncAsicSetRegisterValue(val->regMirror, HEncRefChrCompress, val->refChrCompress);
        if (val->refLumCompress || val->refChrCompress)
            EncAsicSetRegisterValue(val->regMirror, HEncRfcIntEn, val->rfcOverflowIRQEn);
        if (val->refLumCompress)
        {
            EncAsicSetRegisterValue(val->regMirror, HEncRfcLumBufLimit, val->rfcLumBufLimit);
            EncAsicSetRegisterValue(val->regMirror, HEncRfcLumDiscreteMode,  val->rfcLumDiscreteMode);
            SET_ADDR_REG(val->regMirror, HEncBaseRefLumTbl,  val->internalImageLumTableBaseR[0]);
            SET_ADDR_REG(val->regMirror, HEncBaseRecLumTbl,  val->internalImageLumTableBaseW);
            SET_ADDR_REG(val->regMirror, HEncBaseRefLumTbl2, val->internalImageLumTableBaseR[1]);
        }        
        if (val->refChrCompress)
        {
            EncAsicSetRegisterValue(val->regMirror, HEncRfcChrBufLimit, val->rfcChrBufLimit);
            SET_ADDR_REG(val->regMirror, HEncBaseRefChrTbl,  val->internalImageChrTableBaseR[0]);        
            SET_ADDR_REG(val->regMirror, HEncBaseRecChrTbl,  val->internalImageChrTableBaseW);        
            SET_ADDR_REG(val->regMirror, HEncBaseRefChrTbl2, val->internalImageChrTableBaseR[1]);
        }
    }

    /* Stream start offset */
    EncAsicSetRegisterValue(val->regMirror, HEncStartOffset, val->firstFreeBit);
    EncAsicSetRegisterValue(val->regMirror, HEncIpolFilterMode, val->ipolFilterMode);

    /* Stabilization */
    if(val->codingType != ASIC_JPEG)
    {
        SET_ADDR_REG(val->regMirror, HEncBaseNextLum, val->vsNextLumaBase);
        EncAsicSetRegisterValue(val->regMirror, HEncStabMode, val->vsMode);
    }

    EncAsicSetRegisterValue(val->regMirror, HEncDMVPenalty4p, val->pen[0][ASIC_PENALTY_DMV_4P]);
    EncAsicSetRegisterValue(val->regMirror, HEncDMVPenalty1p, val->pen[0][ASIC_PENALTY_DMV_1P]);
    EncAsicSetRegisterValue(val->regMirror, HEncDMVPenaltyQp, val->pen[0][ASIC_PENALTY_DMV_QP]);

    SET_ADDR_REG(val->regMirror, HEncBaseCabacCtx, val->cabacCtxBase);
    SET_ADDR_REG(val->regMirror, HEncBaseMvWrite, val->mvOutputBase);

    EncAsicSetRegisterValue(val->regMirror, HEncRGBCoeffA,
                            val->colorConversionCoeffA & mask_16b);
    EncAsicSetRegisterValue(val->regMirror, HEncRGBCoeffB,
                            val->colorConversionCoeffB & mask_16b);
    EncAsicSetRegisterValue(val->regMirror, HEncRGBCoeffC,
                            val->colorConversionCoeffC & mask_16b);
    EncAsicSetRegisterValue(val->regMirror, HEncRGBCoeffE,
                            val->colorConversionCoeffE & mask_16b);
    EncAsicSetRegisterValue(val->regMirror, HEncRGBCoeffF,
                            val->colorConversionCoeffF & mask_16b);

    EncAsicSetRegisterValue(val->regMirror, HEncRMaskMSB, val->rMaskMsb & mask_5b);
    EncAsicSetRegisterValue(val->regMirror, HEncGMaskMSB, val->gMaskMsb & mask_5b);
    EncAsicSetRegisterValue(val->regMirror, HEncBMaskMSB, val->bMaskMsb & mask_5b);

    EncAsicSetRegisterValue(val->regMirror, HEncCirStart, val->cirStart);
    EncAsicSetRegisterValue(val->regMirror, HEncCirInterval, val->cirInterval);

    EncAsicSetRegisterValue(val->regMirror, HEncIntraAreaLeft, val->intraAreaLeft);
    EncAsicSetRegisterValue(val->regMirror, HEncIntraAreaRight, val->intraAreaRight);
    EncAsicSetRegisterValue(val->regMirror, HEncIntraAreaTop, val->intraAreaTop);
    EncAsicSetRegisterValue(val->regMirror, HEncIntraAreaBottom, val->intraAreaBottom);
    EncAsicSetRegisterValue(val->regMirror, HEncRoi1Left, val->roi1Left);
    EncAsicSetRegisterValue(val->regMirror, HEncRoi1Right, val->roi1Right);
    EncAsicSetRegisterValue(val->regMirror, HEncRoi1Top, val->roi1Top);
    EncAsicSetRegisterValue(val->regMirror, HEncRoi1Bottom, val->roi1Bottom);

    EncAsicSetRegisterValue(val->regMirror, HEncRoi2Left, val->roi2Left);
    EncAsicSetRegisterValue(val->regMirror, HEncRoi2Right, val->roi2Right);
    EncAsicSetRegisterValue(val->regMirror, HEncRoi2Top, val->roi2Top);
    EncAsicSetRegisterValue(val->regMirror, HEncRoi2Bottom, val->roi2Bottom);

    /* H.264 specific */
    if (val->codingType == ASIC_H264)
    {
        /* Limit ROI delta QPs so that ROI area QP is always >= 0.
         * ASIC doesn't check this. */
        EncAsicSetRegisterValue(val->regMirror, HEncRoi1DeltaQp, MIN((i32)val->qp, val->roi1DeltaQp));
        EncAsicSetRegisterValue(val->regMirror, HEncRoi2DeltaQp, MIN((i32)val->qp, val->roi2DeltaQp));

        EncAsicSetRegisterValue(val->regMirror, HEncIntraSliceMap1, val->intraSliceMap1);
        EncAsicSetRegisterValue(val->regMirror, HEncIntraSliceMap2, val->intraSliceMap2);
        EncAsicSetRegisterValue(val->regMirror, HEncIntraSliceMap3, val->intraSliceMap3);

        /* MVC */
        EncAsicSetRegisterValue(val->regMirror, HEncMvcAnchorPicFlag, val->mvcAnchorPicFlag);
        EncAsicSetRegisterValue(val->regMirror, HEncMvcPriorityId, val->mvcPriorityId);
        EncAsicSetRegisterValue(val->regMirror, HEncMvcViewId, val->mvcViewId);
        EncAsicSetRegisterValue(val->regMirror, HEncMvcTemporalId, val->mvcTemporalId);
        EncAsicSetRegisterValue(val->regMirror, HEncMvcInterViewFlag, val->mvcInterViewFlag);

        EncAsicSetRegisterValue(val->regMirror, HEncFieldPicFlag, val->fieldPicFlag);
        EncAsicSetRegisterValue(val->regMirror, HEncBottomFieldFlag, val->bottomFieldFlag);
        EncAsicSetRegisterValue(val->regMirror, HEncFieldParity, val->fieldParity);
        /* Boost */
        EncAsicSetRegisterValue(val->regMirror, HEncBoostQp, val->boostQp);
        EncAsicSetRegisterValue(val->regMirror, HEncBoostVar1, val->boostVar1);
        EncAsicSetRegisterValue(val->regMirror, HEncBoostVar2, val->boostVar2);

        /* Variane intar vs. inter favor */
        EncAsicSetRegisterValue(val->regMirror, HEncVarLimit, val->varLimitDiv32);
        EncAsicSetRegisterValue(val->regMirror, HEncVarInterFavor, val->varInterFavorDiv16);
        EncAsicSetRegisterValue(val->regMirror, HEncVarMultiplier, val->varMultiplier);
        EncAsicSetRegisterValue(val->regMirror, HEncVarAdd, val->varAdd);

        /* Pskip coding mode */
        EncAsicSetRegisterValue(val->regMirror, HEncPskipMode, val->pskipMode);

        /* Denoise */
        EncAsicSetRegisterValue(val->regMirror, HEncDnfEnable, val->dnfEnable);
        EncAsicSetRegisterValue(val->regMirror, HEncDnfNoiseMax, val->dnfNoiseLevelMax);
        EncAsicSetRegisterValue(val->regMirror, HEncDnfNoiseLevelInvY, val->dnfNoiseLevelInvertY);
        EncAsicSetRegisterValue(val->regMirror, HEncDnfNoiseLevelInvC, val->dnfNoiseLevelInvertC);
        EncAsicSetRegisterValue(val->regMirror, HEncDnfStregth, val->dnfStrength);
        #ifdef DNF_PARAM_REGS
        for(i=0; i<16; i++)
        {
            EncAsicSetRegisterValue(val->regMirror, HEncDnfParaS1_0+i, val->dnfParamS1[i]);
        }
        for(i=0; i<16; i++)
        {
            EncAsicSetRegisterValue(val->regMirror, HEncDnfParaS2_0+i, val->dnfParamS2[i]);
        }
        for(i=0; i<64; i++)
        {
            EncAsicSetRegisterValue(val->regMirror, HEncDnfParaS3_0+i, val->dnfParamS3[i]);
        }
        for(i=0; i<64; i++)
        {
            EncAsicSetRegisterValue(val->regMirror, HEncDnfParaS4_0+i, val->dnfParamS4[i]);
        }
        #else
        EncAsicSetRegisterValue(val->regMirror, HEncDnfBaseTable, val->dnfTable);
        #endif
    }

    EncAsicSetRegisterValue(val->regMirror, HEncMbWrPtr, val->mbWrPtr);
    EncAsicSetRegisterValue(val->regMirror, HEncMbRdPtr, val->mbRrPtr);
    EncAsicSetRegisterValue(val->regMirror, HEncLowLatencyHwSyncMbRows, val->lineBufferDepth);
    EncAsicSetRegisterValue(val->regMirror, HEncLowLatencyEn, val->lineBufferEn);    
    EncAsicSetRegisterValue(val->regMirror, HEncLowLatencyHwIntfEn, val->lineBufferHwHandShake);
    EncAsicSetRegisterValue(val->regMirror, HEncLowLatencyIrqEn, val->lineBufferIrqEnable);
    EncAsicSetRegisterValue(val->regMirror, HEncInputBufferLoopBackEn, val->lineBufferLoopBackEn);

    /* VP8 specific */
    if (val->codingType == ASIC_VP8)
    {
        SET_ADDR_REG(val->regMirror, HEncBasePartition1, val->partitionBase[0]);
        SET_ADDR_REG(val->regMirror, HEncBasePartition2, val->partitionBase[1]);
        SET_ADDR_REG(val->regMirror, HEncBasePartition3, val->partitionBase[2]);
        SET_ADDR_REG(val->regMirror, HEncBasePartition4, val->partitionBase[3]);
        SET_ADDR_REG(val->regMirror, HEncBaseVp8ProbCount, val->probCountBase);
        

        EncAsicSetRegisterValue(val->regMirror, HEncVp8Mode0Penalty, val->pen[0][ASIC_PENALTY_I16MODE0]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8Mode1Penalty, val->pen[0][ASIC_PENALTY_I16MODE1]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8Mode2Penalty, val->pen[0][ASIC_PENALTY_I16MODE2]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8Mode3Penalty, val->pen[0][ASIC_PENALTY_I16MODE3]);

        EncAsicSetRegisterValue(val->regMirror, HEncVp8Bmode0Penalty, val->pen[0][ASIC_PENALTY_I4MODE0]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8Bmode1Penalty, val->pen[0][ASIC_PENALTY_I4MODE1]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8Bmode2Penalty, val->pen[0][ASIC_PENALTY_I4MODE2]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8Bmode3Penalty, val->pen[0][ASIC_PENALTY_I4MODE3]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8Bmode4Penalty, val->pen[0][ASIC_PENALTY_I4MODE4]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8Bmode5Penalty, val->pen[0][ASIC_PENALTY_I4MODE5]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8Bmode6Penalty, val->pen[0][ASIC_PENALTY_I4MODE6]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8Bmode7Penalty, val->pen[0][ASIC_PENALTY_I4MODE7]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8Bmode8Penalty, val->pen[0][ASIC_PENALTY_I4MODE8]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8Bmode9Penalty, val->pen[0][ASIC_PENALTY_I4MODE9]);


        for (i = 0; i < 3; i++) {
            i32 off = (HEncVp8Seg2Y1QuantDc - HEncVp8Seg1Y1QuantDc) * i;
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y1QuantDc+off, val->qpY1QuantDc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y1QuantAc+off, val->qpY1QuantAc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y2QuantDc+off, val->qpY2QuantDc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y2QuantAc+off, val->qpY2QuantAc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1ChQuantDc+off, val->qpChQuantDc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1ChQuantAc+off, val->qpChQuantAc[i+1]);

            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y1ZbinDc+off, val->qpY1ZbinDc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y1ZbinAc+off, val->qpY1ZbinAc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y2ZbinDc+off, val->qpY2ZbinDc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y2ZbinAc+off, val->qpY2ZbinAc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1ChZbinDc+off, val->qpChZbinDc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1ChZbinAc+off, val->qpChZbinAc[i+1]);

            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y1RoundDc+off, val->qpY1RoundDc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y1RoundAc+off, val->qpY1RoundAc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y2RoundDc+off, val->qpY2RoundDc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y2RoundAc+off, val->qpY2RoundAc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1ChRoundDc+off, val->qpChRoundDc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1ChRoundAc+off, val->qpChRoundAc[i+1]);

            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y1DequantDc+off, val->qpY1DequantDc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y1DequantAc+off, val->qpY1DequantAc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y2DequantDc+off, val->qpY2DequantDc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1Y2DequantAc+off, val->qpY2DequantAc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1ChDequantDc+off, val->qpChDequantDc[i+1]);
            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1ChDequantAc+off, val->qpChDequantAc[i+1]);

            EncAsicSetRegisterValue(val->regMirror, HEncVp8Seg1FilterLevel+off, val->filterLevel[i+1]);
        }

        EncAsicSetRegisterValue(val->regMirror, HEncVp8LfRefDelta0, val->lfRefDelta[0] & mask_7b);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8LfRefDelta1, val->lfRefDelta[1] & mask_7b);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8LfRefDelta2, val->lfRefDelta[2] & mask_7b);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8LfRefDelta3, val->lfRefDelta[3] & mask_7b);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8LfModeDelta0, val->lfModeDelta[0] & mask_7b);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8LfModeDelta1, val->lfModeDelta[1] & mask_7b);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8LfModeDelta2, val->lfModeDelta[2] & mask_7b);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8LfModeDelta3, val->lfModeDelta[3] & mask_7b);

        EncAsicSetRegisterValue(val->regMirror, HEncVp8CostInter, val->pen[0][ASIC_PENALTY_COST_INTER]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8DmvCostConst, val->pen[0][ASIC_PENALTY_DMV_COST_CONST]);

        EncAsicSetRegisterValue(val->regMirror, HEncVp8DzRateM0, val->pen[0][ASIC_PENALTY_DZ_RATE0]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8DzRateM1, val->pen[0][ASIC_PENALTY_DZ_RATE1]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8DzRateM2, val->pen[0][ASIC_PENALTY_DZ_RATE2]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8DzRateM3, val->pen[0][ASIC_PENALTY_DZ_RATE3]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8DzSkipRate0, val->pen[0][ASIC_PENALTY_DZ_SKIP0]);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8DzSkipRate1, val->pen[0][ASIC_PENALTY_DZ_SKIP1]);

    }

    /* Mode decision penalty values for segments 1-3 */
    if(val->codingType != ASIC_JPEG)
    {
        for (i = 0; i < 3; i++) {
            i32 off = (HEncSeg2I16Mode0Penalty - HEncSeg1I16Mode0Penalty) * i;
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I16Mode0Penalty +off, val->pen[1+i][ASIC_PENALTY_I16MODE0]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I16Mode1Penalty +off, val->pen[1+i][ASIC_PENALTY_I16MODE1]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I16Mode2Penalty +off, val->pen[1+i][ASIC_PENALTY_I16MODE2]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I16Mode3Penalty +off, val->pen[1+i][ASIC_PENALTY_I16MODE3]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode0Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE0]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode1Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE1]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode2Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE2]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode3Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE3]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode4Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE4]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode5Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE5]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode6Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE6]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode7Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE7]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode8Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE8]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode9Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE9]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I16Favor        +off, val->pen[1+i][ASIC_PENALTY_I16FAVOR]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1CostInter       +off, val->pen[1+i][ASIC_PENALTY_COST_INTER]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4PrevModeFavor +off, val->pen[1+i][ASIC_PENALTY_I4_PREV_MODE_FAVOR]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1InterFavor      +off, val->pen[1+i][ASIC_PENALTY_INTER_FAVOR] & mask_16b);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1SkipPenalty     +off, val->pen[1+i][ASIC_PENALTY_SKIP]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1GoldenPenalty   +off, val->pen[1+i][ASIC_PENALTY_GOLDEN]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1SplitPenalty8x4 +off, val->pen[1+i][ASIC_PENALTY_SPLIT8x4]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1SplitPenalty8x8 +off, val->pen[1+i][ASIC_PENALTY_SPLIT8x8]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1SplitPenalty16x8+off, val->pen[1+i][ASIC_PENALTY_SPLIT16x8]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1SplitPenalty4x4 +off, val->pen[1+i][ASIC_PENALTY_SPLIT4x4]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1SplitZeroPenalty+off, val->pen[1+i][ASIC_PENALTY_SPLIT_ZERO]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DMVPenaltyQp    +off, val->pen[1+i][ASIC_PENALTY_DMV_QP]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DzRateM0        +off, val->pen[1+i][ASIC_PENALTY_DZ_RATE0]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DzRateM1        +off, val->pen[1+i][ASIC_PENALTY_DZ_RATE1]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DzRateM2        +off, val->pen[1+i][ASIC_PENALTY_DZ_RATE2]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DzRateM3        +off, val->pen[1+i][ASIC_PENALTY_DZ_RATE3]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DzSkipRate0     +off, val->pen[1+i][ASIC_PENALTY_DZ_SKIP0]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DzSkipRate1     +off, val->pen[1+i][ASIC_PENALTY_DZ_SKIP1]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DmvCostConst    +off, val->pen[1+i][ASIC_PENALTY_DMV_COST_CONST]);
        }

        EncAsicSetRegisterValue(val->regMirror, HEncVp8AvgVar, val->avgVar);
        EncAsicSetRegisterValue(val->regMirror, HEncVp8InvAvgVar, val->invAvgVar);
    }

    if(val->codingType != ASIC_JPEG)
    {
        EncAsicSetRegisterValue(val->regMirror, HEncMBRCEnable, val->mbRcEnable);
        //printf("val->qpfrac =0x%x\n",val->qpfrac);
        EncAsicSetRegisterValue(val->regMirror, HEncMBRCQpFrac, val->qpfrac);
        EncAsicSetRegisterValue(val->regMirror, HEncMBRcSliceQpOffset, val->offsetSliceQp & mask_6b);
        
        EncAsicSetRegisterValue(val->regMirror, HEncMBRCQpDeltaRange, val->mbQpDeltaRange);

        EncAsicSetRegisterValue(val->regMirror, HEncMBComplexityOffset, val->offsetMBComplexity);

        EncAsicSetRegisterValue(val->regMirror, HEncMBQpDeltaGain, val->qpDeltaMBGain);

        if(val->mbRcEnable)
        {
        for (i = 0; i < 3; i++) {
            i32 off = (HEncSeg2I16Mode0Penalty - HEncSeg1I16Mode0Penalty) * i;
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I16Mode0Penalty +off, val->pen[1+i][ASIC_PENALTY_I16MODE0]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I16Mode1Penalty +off, val->pen[1+i][ASIC_PENALTY_I16MODE1]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I16Mode2Penalty +off, val->pen[1+i][ASIC_PENALTY_I16MODE2]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I16Mode3Penalty +off, val->pen[1+i][ASIC_PENALTY_I16MODE3]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode0Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE0]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode1Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE1]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode2Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE2]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode3Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE3]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode4Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE4]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode5Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE5]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode6Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE6]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode7Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE7]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode8Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE8]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4Mode9Penalty  +off, val->pen[1+i][ASIC_PENALTY_I4MODE9]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I16Favor        +off, val->pen[1+i][ASIC_PENALTY_I16FAVOR]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1CostInter       +off, val->pen[1+i][ASIC_PENALTY_COST_INTER]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1I4PrevModeFavor +off, val->pen[1+i][ASIC_PENALTY_I4_PREV_MODE_FAVOR]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1InterFavor      +off, (val->pen[1+i][ASIC_PENALTY_INTER_FAVOR]) & mask_16b);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1SkipPenalty     +off, CLIP3(val->pen[1+i][ASIC_PENALTY_SKIP]/*+25*/,1,255));
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1GoldenPenalty   +off, val->pen[1+i][ASIC_PENALTY_GOLDEN]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1SplitPenalty8x4 +off, val->pen[1+i][ASIC_PENALTY_SPLIT8x4]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1SplitPenalty8x8 +off, val->pen[1+i][ASIC_PENALTY_SPLIT8x8]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1SplitPenalty16x8+off, val->pen[1+i][ASIC_PENALTY_SPLIT16x8]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1SplitPenalty4x4 +off, val->pen[1+i][ASIC_PENALTY_SPLIT4x4]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1SplitZeroPenalty+off, val->pen[1+i][ASIC_PENALTY_SPLIT_ZERO]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DMVPenaltyQp    +off, val->pen[1+i][ASIC_PENALTY_DMV_QP]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DzRateM0        +off, val->pen[1+i][ASIC_PENALTY_DZ_RATE0]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DzRateM1        +off, val->pen[1+i][ASIC_PENALTY_DZ_RATE1]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DzRateM2        +off, val->pen[1+i][ASIC_PENALTY_DZ_RATE2]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DzRateM3        +off, val->pen[1+i][ASIC_PENALTY_DZ_RATE3]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DzSkipRate0     +off, val->pen[1+i][ASIC_PENALTY_DZ_SKIP0]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DzSkipRate1     +off, val->pen[1+i][ASIC_PENALTY_DZ_SKIP1]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg1DmvCostConst    +off, val->pen[1+i][ASIC_PENALTY_DMV_COST_CONST]);
        }
        
        for (i = 3; i < 31; i++) {
            i32 off = (HEncSeg5I4PrevModeFavor - HEncSeg4I4PrevModeFavor) * (i-3);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg4I16Favor        +off, val->pen[1+i][ASIC_PENALTY_I16FAVOR]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg4I4PrevModeFavor +off, val->pen[1+i][ASIC_PENALTY_I4_PREV_MODE_FAVOR]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg4InterFavor      +off, (val->pen[1+i][ASIC_PENALTY_INTER_FAVOR]) & mask_16b);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg4SkipPenalty     +off, CLIP3(val->pen[1+i][ASIC_PENALTY_SKIP]/*+25*/,1,255));
            EncAsicSetRegisterValue(val->regMirror, HEncSeg4GoldenPenalty   +off, val->pen[1+i][ASIC_PENALTY_GOLDEN]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg4SplitPenalty8x4 +off, val->pen[1+i][ASIC_PENALTY_SPLIT8x4]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg4SplitPenalty8x8 +off, val->pen[1+i][ASIC_PENALTY_SPLIT8x8]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg4SplitPenalty16x8+off, val->pen[1+i][ASIC_PENALTY_SPLIT16x8]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg4SplitPenalty4x4 +off, val->pen[1+i][ASIC_PENALTY_SPLIT4x4]);
            EncAsicSetRegisterValue(val->regMirror, HEncSeg4DMVPenaltyQp    +off, val->pen[1+i][ASIC_PENALTY_DMV_QP]);        
          }
        }

    }
      
    /* Scaled output picture */
    SET_ADDR_REG(val->regMirror, HEncBaseScaledOutLum, val->scaledLumBase);
    
    EncAsicSetRegisterValue(val->regMirror, HEncScaleMode, val->scaledWidth > 0 ? 2 : 0);
    EncAsicSetRegisterValue(val->regMirror, HEncScaledOutWidth, val->scaledWidth);
    EncAsicSetRegisterValue(val->regMirror, HEncScaledOutHeight, val->scaledHeight);
    EncAsicSetRegisterValue(val->regMirror, HEncScaledOutWidthRatio, val->scaledWidthRatio);
    EncAsicSetRegisterValue(val->regMirror, HEncScaledOutHeightRatio, val->scaledHeightRatio);


#ifdef ASIC_WAVE_TRACE_TRIGGER
    if(val->vop_count++ == trigger_point)
    {
        /* logic analyzer triggered by writing to the ID reg */
        EWLWriteReg(ewl, 0x00, ~0);
    }
#endif

    /* Write regMirror to registers, 0x4 to 0xF8 */
    {
        for(i = 1; i <= 62; i++)
            EWLWriteReg(ewl, HSWREG(i), val->regMirror[i]);
    }

    /* Write JPEG quantization tables to regs if needed (JPEG) */
    if(val->codingType == ASIC_JPEG)
    {
        for(i = 0; i < 128; i += 4)
        {
            /* swreg[64] to swreg[95] */
            EWLWriteReg(ewl, HSWREG(64 + (i/4)),
                        (val->quantTable[i    ] << 24) |
                        (val->quantTable[i + 1] << 16) |
                        (val->quantTable[i + 2] << 8) |
                        (val->quantTable[i + 3]));
        }
        /* swreg[231]=0x39C to swreg[233]=0x3A4, scaler regs for dragonfly */
        if (val->asicHwId > ASIC_ID_CLOUDBERRY) {
            for(i = 231; i <= 233; i++)
                EWLWriteReg(ewl, HSWREG(i), val->regMirror[i]);
        }
        /* write MSB registers */
        EWLWriteReg(ewl, BASE_HEncBaseStream_MSB, val->regMirror[BASE_HEncBaseStream_MSB/4]);
        EWLWriteReg(ewl, BASE_HEncBaseInLum_MSB, val->regMirror[BASE_HEncBaseInLum_MSB/4]);
        EWLWriteReg(ewl, BASE_HEncBaseInCb_MSB, val->regMirror[BASE_HEncBaseInCb_MSB/4]);
        EWLWriteReg(ewl, BASE_HEncBaseInCr_MSB, val->regMirror[BASE_HEncBaseInCr_MSB/4]);

        /* write input line buffer register */
        EWLWriteReg(ewl, BASE_HEncInstantInput, val->regMirror[BASE_HEncInstantInput/4]);
    } 
    else 
    {
        i32 i = 0;

        /* swreg[64]=0x100 to swreg[95]=0x17C, VP8 regs */
        for(i = 64; i <= 95; i++)
            EWLWriteReg(ewl, HSWREG(i), val->regMirror[i]);

        /* Write DMV penalty tables to regs */
        for(i = 0; i < 128; i += 4)
        {
            /* swreg[96]=0x180 to swreg[127]=0x1FC */
            EWLWriteReg(ewl, HSWREG(96 + (i/4)),
                        (val->dmvPenalty[i    ] << 24) |
                        (val->dmvPenalty[i + 1] << 16) |
                        (val->dmvPenalty[i + 2] << 8) |
                        (val->dmvPenalty[i + 3]));
        }
        for(i = 0; i < 128; i += 4)
        {
            /* swreg[128]=0x200 to swreg[159]=0x27C */
            EWLWriteReg(ewl, HSWREG(128 + (i/4)),
                        (val->dmvQpelPenalty[i    ] << 24) |
                        (val->dmvQpelPenalty[i + 1] << 16) |
                        (val->dmvQpelPenalty[i + 2] << 8) |
                        (val->dmvQpelPenalty[i + 3]));
        }

        /* swreg[160]=0x280 to swreg[163]=0x28C, VP8 regs */
        for(i = 160; i <= 163; i++)
            EWLWriteReg(ewl, HSWREG(i), val->regMirror[i]);

        /* Write deadzone tables to regs when needed. */
        if ((val->asicHwId > ASIC_ID_BLUEBERRY) && val->deadzoneEnable) {
            for(i = 0; i < 192; i += 4)
            {
                /* swreg[164]=0x290 to swreg[211]=0x34C */
                EWLWriteReg(ewl, HSWREG(164 + (i/4)),
                            (val->dzCoeffRate[i    ] << 24) |
                            (val->dzCoeffRate[i + 1] << 16) |
                            (val->dzCoeffRate[i + 2] << 8) |
                            (val->dzCoeffRate[i + 3]));
            }
            for(i = 0; i < 64; i += 4)
            {
                /* swreg[212]=0x350 to swreg[227]=0x38C */
                EWLWriteReg(ewl, HSWREG(212 + (i/4)),
                            (val->dzEobRate[i    ] << 24) |
                            (val->dzEobRate[i + 1] << 16) |
                            (val->dzEobRate[i + 2] << 8) |
                            (val->dzEobRate[i + 3]));
            }
            EWLWriteReg(ewl, HSWREG(228), val->regMirror[228]);
            EWLWriteReg(ewl, HSWREG(229), val->regMirror[229]);
            EWLWriteReg(ewl, HSWREG(230), val->regMirror[230]);
        }

        /* swreg[231]=0x39C to swreg[238]=0x3B8, VP8 regs for dragonfly */
        if (val->asicHwId > ASIC_ID_CLOUDBERRY) {
            for(i = 231; i <= 238; i++)
                EWLWriteReg(ewl, HSWREG(i), val->regMirror[i]);
        }

        /* swreg[239]=0x3BC to swreg[293]=0x490, VP8 regs for evergreen */
        if (val->asicHwId > ASIC_ID_DRAGONFLY) {
            /* Write VP8 bicubic Ipol coeffs straight to regs */
            if (val->codingType == ASIC_VP8 && val->ipolFilterMode == 0) {
                EWLWriteReg(ewl, HSWREG(239), (0<<29) | ( 0<<24) | (128<<16) | ( 0<<8) | ( 0<<3) | 0);
                EWLWriteReg(ewl, HSWREG(240), (0<<29) | ( 6<<24) | (123<<16) | (12<<8) | ( 1<<3) | 0);
                EWLWriteReg(ewl, HSWREG(241), (2<<29) | (11<<24) | (108<<16) | (36<<8) | ( 8<<3) | 1);
                EWLWriteReg(ewl, HSWREG(242), (0<<29) | ( 9<<24) | ( 93<<16) | (50<<8) | ( 6<<3) | 0);
                EWLWriteReg(ewl, HSWREG(243), (3<<29) | (16<<24) | ( 77<<16) | (77<<8) | (16<<3) | 3);
            } else if (val->codingType == ASIC_VP8 && val->ipolFilterMode == 1) {
                /* Write VP8 bilinear Ipol coeffs straight to regs */
                EWLWriteReg(ewl, HSWREG(239), (0<<29) | ( 0<<24) | (128<<16) | ( 0<<8) | ( 0<<3) | 0);
                EWLWriteReg(ewl, HSWREG(240), (0<<29) | ( 0<<24) | (112<<16) | (16<<8) | ( 0<<3) | 0);
                EWLWriteReg(ewl, HSWREG(241), (0<<29) | ( 0<<24) | ( 96<<16) | (32<<8) | ( 0<<3) | 0);
                EWLWriteReg(ewl, HSWREG(242), (0<<29) | ( 0<<24) | ( 80<<16) | (48<<8) | ( 0<<3) | 0);
                EWLWriteReg(ewl, HSWREG(243), (0<<29) | ( 0<<24) | ( 64<<16) | (64<<8) | ( 0<<3) | 0);
            } else {
                EWLWriteReg(ewl, HSWREG(239), 0);
                EWLWriteReg(ewl, HSWREG(240), 0);
                EWLWriteReg(ewl, HSWREG(241), 0);
                EWLWriteReg(ewl, HSWREG(242), 0);
                EWLWriteReg(ewl, HSWREG(243), 0);
            }
            for(i = 244; i < BASE_HWFuse2/4; i++)
                EWLWriteReg(ewl, HSWREG(i), val->regMirror[i]);
            
        }
        
        if (val->asicHwId >= ASIC_ID_V7BASE) {
            /* Skip the HWFuse2 register */
            for(i = (BASE_HWFuse2+4)/4; i < ASIC_SWREG_AMOUNT; i++) {
                EWLWriteReg(ewl, HSWREG(i), val->regMirror[i]);
            }
        }
    }

#ifdef TRACE_REGS
    EncTraceRegs(ewl, 0, 0);
#endif

    /* Register with enable bit is written last */
    val->regMirror[14] |= ASIC_STATUS_ENABLE;

    EWLEnableHW(ewl, HSWREG(14), val->regMirror[14]);
}

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void EncAsicFrameContinue(const void *ewl, regValues_s * val)
{
    /* clear status bits, clear IRQ => HW restart */
    u32 status = val->regMirror[1];

    if (!(EWLReadReg(ewl, BASE_HWFuse2)&HWCFGIrqClearSupport))
    {
      status &= (~ASIC_STATUS_ALL);
      status &= ~ASIC_IRQ_LINE;
    }

    val->regMirror[1] = status;

    /*CheckRegisterValues(val); */

    /* Write only registers which may be updated mid frame */
    EWLWriteReg(ewl, HSWREG(24), (val->rlcLimitSpace / 2));

    val->regMirror[5] = val->rlcBase;
    EWLWriteReg(ewl, HSWREG(5), val->regMirror[5]);

#ifdef TRACE_REGS
    EncTraceRegs(ewl, 0, EncAsicGetRegisterValue(ewl, val->regMirror, HEncMbCount));
#endif

    /* Register with status bits is written last */
    EWLEnableHW(ewl, HSWREG(1), val->regMirror[1]);

}

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void EncAsicGetRegisters(const void *ewl, regValues_s * val)
{

    /* HW output stream size, bits to bytes */
    val->outputStrmSize =
            EncAsicGetRegisterValue(ewl, val->regMirror, HEncStrmBufLimit) / 8;

    /* Calculate frame PSNR based on the squared error. */
    val->squaredError =
            EncAsicGetRegisterValue(ewl, val->regMirror, HEncSquaredError);

    if (val->squaredError) {
        u32 pels;

        /* Error is calculated over 13x13 pixels on every macroblock. */
        pels = EncAsicGetRegisterValue(ewl, val->regMirror, HEncMbCount);
        pels *= 13*13;

        if (pels)
            val->mse_mul256 = val->squaredError < 0xFFFFFF ?
                              256*val->squaredError/pels :
                              256*(val->squaredError/pels);
        else
            val->mse_mul256 = 0;
    }

    /* QP sum div2 */
   
    val->qpSum = EncAsicGetRegisterValue(ewl, val->regMirror, HEncQpSum);
    
    val->rcMSESum = EncAsicGetRegisterValue(ewl, val->regMirror, HEncMBComplexityAverage);
    /* MAD MB count*/
    val->madCount[0] = EncAsicGetRegisterValue(ewl, val->regMirror, HEncMadCount);
    val->madCount[1] = EncAsicGetRegisterValue(ewl, val->regMirror, HEncMadCount2);
    val->madCount[2] = EncAsicGetRegisterValue(ewl, val->regMirror, HEncMadCount3);

    val->avgVar = EncAsicGetRegisterValue(ewl, val->regMirror, HEncVp8AvgVar);

    /* Non-zero coefficient count*/
    val->rlcCount = EncAsicGetRegisterValue(ewl, val->regMirror, HEncRlcSum) * 4;

    /* Denoise */
    val->dnfNoiseLevelPred = EncAsicGetRegisterValue(ewl, val->regMirror, HEncDnfNoisePred);
    val->dnfNoiseMaxPred = EncAsicGetRegisterValue(ewl, val->regMirror, HEncDnfThresholdPred);
    val->dnfNoiseMbNum = EncAsicGetRegisterValue(ewl, val->regMirror, HEncDnfMbNum);

    /* get stabilization results if needed */
    if(val->vsMode != 0)
    {
        i32 i;

        for(i = 40; i <= 50; i++)
        {
            val->regMirror[i] = EWLReadReg(ewl, HSWREG(i));
        }
    }

#ifdef TRACE_REGS
    EncTraceRegs(ewl, 1, EncAsicGetRegisterValue(ewl, val->regMirror, HEncMbCount));
#endif

}

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void EncAsicStop(const void *ewl)
{
    EWLDisableHW(ewl, HSWREG(14), 0);
}

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
u32 EncAsicGetStatus(const void *ewl)
{
    return EWLReadReg(ewl, HSWREG(1));
}

/*------------------------------------------------------------------------------
    EncAsicGetMvOutput
        Return encOutputMbInfo_s pointer to beginning of macroblock mbNum
------------------------------------------------------------------------------*/
u32 * EncAsicGetMvOutput(asicData_s *asic, u32 mbNum)
{
    u32 *mvOutput = asic->mvOutput.virtualAddress;
    u32 traceMbTiming = asic->regs.traceMbTiming;

    if (traceMbTiming) {    /* encOutputMbInfoDebug_s */
        if (asic->regs.asicHwId <= ASIC_ID_EVERGREEN_PLUS) {
            mvOutput += mbNum * 40/4;
        } else {
            mvOutput += mbNum * 56/4;
        }
    } else {        /* encOutputMbInfo_s */
        if (asic->regs.asicHwId <= ASIC_ID_EVERGREEN_PLUS) {
            mvOutput += mbNum * 32/4;
        } else {
            mvOutput += mbNum * 48/4;
        }
    }

    return mvOutput;
}

/*------------------------------------------------------------------------------
    EncAsicClearStatusBit: clear IRQ status bit
        when support write-one-clear method, apply new way.
------------------------------------------------------------------------------*/
void EncAsicClearStatusBit(const void *ewl, u32 statusBit)
{
    u32 fuse2 = EWLReadReg(ewl, BASE_HWFuse2);

    if (fuse2&HWCFGIrqClearSupport)
    {
      EWLWriteReg(ewl, HSWREG(1), statusBit);
    }
    else
    {
      u32 status = EWLReadReg(ewl, HSWREG(1));
      status &= (~statusBit);
      EWLWriteReg(ewl, HSWREG(1), status);
    }

    return;
}


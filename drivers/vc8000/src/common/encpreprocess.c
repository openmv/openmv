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
 *  Description : Preprocessor setup
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "encpreprocess.h"
#include "enccommon.h"

/* Minimum number of frames for how often the ROI can change.
   Each ROI change has to signal new segment map which uses a lot of bits,
   but the more frames there is between updates the more motion can occur. */
#ifndef H1ENC_AROI_MIN_UPDATE_FRAMES
#define H1ENC_AROI_MIN_UPDATE_FRAMES    10
#endif

/* Enable white balance fixing. */
#ifndef H1ENC_AROI_WHITE_BALANCE_FIX
#define H1ENC_AROI_WHITE_BALANCE_FIX    1
#endif

/* Fixed threshold for skin MAD, MBs > threshold are selected. */
#ifndef H1ENC_AROI_SKIN_MAD_THRESHOLD
#define H1ENC_AROI_SKIN_MAD_THRESHOLD   2
#endif

/* Filter box dimensions. BoxWidth*2+1 x BoxHeight*2+1 macroblock filter
   is applied on skin map to find the skin-colored face shapes. */
#ifndef H1ENC_AROI_FILTER_BOX_HEIGHT
#define H1ENC_AROI_FILTER_BOX_HEIGHT    mbPerCol/5/2
#endif

#ifndef H1ENC_AROI_FILTER_BOX_WIDTH
#define H1ENC_AROI_FILTER_BOX_WIDTH     mbPerCol/5*3/4/2
#endif

/* Threshold factor for macroblock score, all MBs >= threshold are selected. */
#ifndef H1ENC_AROI_SCORE_THRESHOLD
#define H1ENC_AROI_SCORE_THRESHOLD      2/3*4/3
#endif

/* Threshold for updating ROI segment in percentage of MBs. */
#ifndef H1ENC_AROI_UPDATE_THRESHOLD
#define H1ENC_AROI_UPDATE_THRESHOLD     70
#endif

/* Define this if you want to write aroi.yuv and aroi.trc
   DEBUG_AROI == 1 for frame level trace, 2 for MB level trace
#define DEBUG_AROI 1 */

#ifdef DEBUG_AROI
#include <stdio.h>

void TraceAroiYuv(asicData_s * asic, preProcess_s * preProcess);
void TraceAroi(asicData_s * asic, preProcess_s * preProcess,
                i32 stage, i32 value);
#endif

void EncAdaptiveROI(asicData_s * asic, preProcess_s * preProcess);

/*------------------------------------------------------------------------------
	EncPreProcessAlloc
------------------------------------------------------------------------------*/
i32 EncPreProcessAlloc(preProcess_s * preProcess, i32 mbPerPicture)
{
    i32 status = ENCHW_OK;
    i32 i;

    for (i = 0; i < 3; i++) {
        preProcess->roiSegmentMap[i] = (u8 *)EWLcalloc(mbPerPicture, sizeof(u8));
        if (preProcess->roiSegmentMap[i] == NULL) status = ENCHW_NOK;
    }

    for (i = 0; i < 2; i++) {
        preProcess->skinMap[i] = (u8 *)EWLcalloc(mbPerPicture, sizeof(u8));
        if (preProcess->skinMap[i] == NULL) status = ENCHW_NOK;
    }

    preProcess->mvMap = (i32 *)EWLcalloc(mbPerPicture, sizeof(i32));
    if (preProcess->mvMap == NULL) status = ENCHW_NOK;

    preProcess->scoreMap = (u8 *)EWLcalloc(mbPerPicture, sizeof(u8));
    if (preProcess->scoreMap == NULL) status = ENCHW_NOK;

    if (status != ENCHW_OK) {
        EncPreProcessFree(preProcess);
        return ENCHW_NOK;
    }

    return ENCHW_OK;
}

/*------------------------------------------------------------------------------
	EncPreProcessFree
------------------------------------------------------------------------------*/
void EncPreProcessFree(preProcess_s * preProcess)
{
    i32 i;

    for (i = 0; i < 3; i++) {
        if (preProcess->roiSegmentMap[i]) EWLfree(preProcess->roiSegmentMap[i]);
        preProcess->roiSegmentMap[i] = NULL;
    }

    for (i = 0; i < 2; i++) {
        if (preProcess->skinMap[i]) EWLfree(preProcess->skinMap[i]);
        preProcess->skinMap[i] = NULL;
    }

    if (preProcess->mvMap) EWLfree(preProcess->mvMap);
    preProcess->mvMap = NULL;

    if (preProcess->scoreMap) EWLfree(preProcess->scoreMap);
    preProcess->scoreMap = NULL;
}

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

/* Input format mapping API values to SW/HW register values. */
static const u32 inputFormatMapping[17] = {
    ASIC_INPUT_YUV420PLANAR,
    ASIC_INPUT_YUV420SEMIPLANAR,
    ASIC_INPUT_YUV420SEMIPLANAR,
    ASIC_INPUT_YUYV422INTERLEAVED,
    ASIC_INPUT_UYVY422INTERLEAVED,
    ASIC_INPUT_RGB565,
    ASIC_INPUT_RGB565,
    ASIC_INPUT_RGB555,
    ASIC_INPUT_RGB555,
    ASIC_INPUT_RGB444,
    ASIC_INPUT_RGB444,
    ASIC_INPUT_RGB888,
    ASIC_INPUT_RGB888,
    ASIC_INPUT_RGB101010,
    ASIC_INPUT_RGB101010,
    ASIC_INPUT_SP_101010,
    ASIC_INPUT_P010
};

static const u32 rgbMaskBits[15][3] = {
    {  0,  0,  0 },
    {  0,  0,  0 },
    {  0,  0,  0 },
    {  0,  0,  0 },
    {  0,  0,  0 },
    { 15, 10,  4 }, /* RGB565 */
    {  4, 10, 15 }, /* BGR565 */
    { 14,  9,  4 }, /* RGB565 */
    {  4,  9, 14 }, /* BGR565 */
    { 11,  7,  3 }, /* RGB444 */
    {  3,  7, 11 }, /* BGR444 */
    { 23, 15,  7 }, /* RGB888 */
    {  7, 15, 23 }, /* BGR888 */
    { 29, 19,  9 }, /* RGB101010 */
    {  9, 19, 29 }  /* BGR101010 */
};

/*------------------------------------------------------------------------------

	EncPreProcessCheck

	Check image size: Cropped frame _must_ fit inside of source image

	Input	preProcess Pointer to preProcess_s structure.

	Return	ENCHW_OK	No errors.
		ENCHW_NOK	Error condition.

------------------------------------------------------------------------------*/
i32 EncPreProcessCheck(const preProcess_s * preProcess)
{
    i32 status = ENCHW_OK;
    u32 tmp;
    u32 width, height;

#if 0   /* These limits apply for input stride but camstab needs the
           actual pixels without padding. */
    u32 w_mask;

    /* Check width limits: lum&ch strides must be full 64-bit addresses */
    if(preProcess->inputFormat == 0)        /* YUV 420 planar */
        w_mask = 0x0F;                      /* 16 multiple */
    else if(preProcess->inputFormat <= 1)   /* YUV 420 semiplanar */
        w_mask = 0x07;                      /* 8 multiple  */
    else if(preProcess->inputFormat <= 9)   /* YUYV 422 or 16-bit RGB */
        w_mask = 0x03;                      /* 4 multiple  */
    else                                    /* 32-bit RGB */
        w_mask = 0x01;                      /* 2 multiple  */

    if(preProcess->lumWidthSrc & w_mask)
    {
        status = ENCHW_NOK;
    }
#endif

    if(preProcess->lumHeightSrc & 0x01)
    {
        status = ENCHW_NOK;
    }

    if(preProcess->lumWidthSrc > MAX_INPUT_IMAGE_WIDTH)
    {
        status = ENCHW_NOK;
    }

    width = preProcess->lumWidth;
    height = preProcess->lumHeight;
    if(preProcess->rotation == 1 || preProcess->rotation == 2)
    {
        u32 tmp;

        tmp = width;
        width = height;
        height = tmp;
    }

    /* Bottom right corner */
    tmp = preProcess->horOffsetSrc + width;
    if(tmp > preProcess->lumWidthSrc)
    {
        status = ENCHW_NOK;
    }

    tmp = preProcess->verOffsetSrc + height;
    if(tmp > preProcess->lumHeightSrc)
    {
        status = ENCHW_NOK;
    }

    return status;
}

/*------------------------------------------------------------------------------

	EncPreProcess

	Preform cropping

	Input	asic	Pointer to asicData_s structure
		preProcess Pointer to preProcess_s structure.

------------------------------------------------------------------------------*/
void EncPreProcess(asicData_s * asic, preProcess_s * preProcess)
{
    u32 tmp;
    u32 width, height;
    regValues_s *regs;
    u32 stride;
    u32 horOffsetSrc = preProcess->horOffsetSrc;

    ASSERT(asic != NULL && preProcess != NULL);

    regs = &asic->regs;

    stride = (preProcess->lumWidthSrc + 15) & (~15); /* 16 pixel multiple stride */

    /* When input is interlaced frame containing two fields we read the frame
       with twice the width and crop the left(top) or right(bottom) field. */
    if (preProcess->interlacedFrame) {
        if (preProcess->bottomField) horOffsetSrc += stride;
        stride *= 2;
    }

    /* cropping */
    if(preProcess->inputFormat <= 2)    /* YUV 420 planar/semiplanar */
    {
        /* Input image position after crop and stabilization */
        tmp = preProcess->verOffsetSrc;
        tmp *= stride;
        tmp += horOffsetSrc;
        regs->inputLumBase += (tmp & (~7));
        regs->inputLumaBaseOffset = tmp & 7;

        if(preProcess->videoStab)
            regs->vsNextLumaBase += (tmp & (~7));

        /* Chroma */
        if(preProcess->inputFormat == 0)
        {
            tmp = preProcess->verOffsetSrc / 2;
            tmp *= stride / 2;
            tmp += horOffsetSrc / 2;

            regs->inputCbBase += (tmp & (~7));
            regs->inputCrBase += (tmp & (~7));
            regs->inputChromaBaseOffset = tmp & 7;
        }
        else
        {
            tmp = preProcess->verOffsetSrc / 2;
            tmp *= stride / 2;
            tmp += horOffsetSrc / 2;
            tmp *= 2;

            regs->inputCbBase += (tmp & (~7));
            regs->inputChromaBaseOffset = tmp & 7;
        }
    }
    else if(preProcess->inputFormat <= 10)    /* YUV 422 / RGB 16bpp */
    {
        /* Input image position after crop and stabilization */
        tmp = preProcess->verOffsetSrc;
        tmp *= stride;
        tmp += horOffsetSrc;
        tmp *= 2;

        regs->inputLumBase += (tmp & (~7));
        regs->inputLumaBaseOffset = tmp & 7;
        regs->inputChromaBaseOffset = (regs->inputLumaBaseOffset / 4) * 4;

        if(preProcess->videoStab)
            regs->vsNextLumaBase += (tmp & (~7));
    }
    else if(preProcess->inputFormat <= 14)    /* RGB 32bpp */
    {
        /* Input image position after crop and stabilization */
        tmp = preProcess->verOffsetSrc;
        tmp *= stride;
        tmp += horOffsetSrc;
        tmp *= 4;

        regs->inputLumBase += (tmp & (~7));
        /* Note: HW does the cropping AFTER RGB to YUYV conversion
         * so the offset is calculated using 16bpp */
        regs->inputLumaBaseOffset = (tmp & 7)/2;

        if(preProcess->videoStab)
            regs->vsNextLumaBase += (tmp & (~7));
    } 
    else  /* P010 planar/semi planar */
    {
        /* Input image position after crop and stabilization */
        tmp = preProcess->verOffsetSrc;
        tmp *= stride;
        tmp += horOffsetSrc;
        tmp *= 2;

        regs->inputLumBase += (tmp & (~7));
        regs->inputLumaBaseOffset = tmp & 7;
        regs->inputChromaBaseOffset = (regs->inputLumaBaseOffset / 4) * 4;

        if(preProcess->videoStab)
            regs->vsNextLumaBase += (tmp & (~7));

         /* Chroma */
        tmp = preProcess->verOffsetSrc / 2;
        tmp *= stride / 2;
        tmp += horOffsetSrc / 2;
        tmp *= 2*2;

        regs->inputCbBase += (tmp & (~7));
        regs->inputCrBase += (tmp & (~7));
        regs->inputChromaBaseOffset = tmp & 7;
    }

    /* YUV subsampling, map API values into HW reg values */
    regs->inputImageFormat = inputFormatMapping[preProcess->inputFormat];

    if (preProcess->inputFormat == 2)
        regs->chromaSwap = 1;

    regs->inputImageRotation = preProcess->rotation;

    /* source image setup, size and fill */
    width = preProcess->lumWidth;
    height = preProcess->lumHeight;

    /* Scaling ratio for down-scaling, fixed point 1.16, calculate from
        rotated dimensions. */
    if (preProcess->scaledWidth*preProcess->scaledHeight &&
        preProcess->scaledOutput) {
        u32 width16 = (width+15)/16*16;
        u32 height16 = (height+15)/16*16;

        regs->scaledWidth = preProcess->scaledWidth;
        regs->scaledHeight = preProcess->scaledHeight;
        if(preProcess->rotation == 1 || preProcess->rotation == 2) {
            /* Scaling factors calculated from overfilled dimensions. */
            regs->scaledWidthRatio =
                            (u32)(preProcess->scaledWidth << 16) / width16 + 1;
            regs->scaledHeightRatio =
                            (u32)(preProcess->scaledHeight << 16) / height16 +1;
        } else {
            /* Scaling factors calculated from image pixels, round upwards. */
            regs->scaledWidthRatio =
                            (u32)(preProcess->scaledWidth << 16) / width + 1;
            regs->scaledHeightRatio =
                            (u32)(preProcess->scaledHeight << 16) / height + 1;
            /* Adjust horizontal scaling factor using overfilled dimensions
               to avoid problems in HW implementation with pixel leftover. */
            regs->scaledWidthRatio =
                ((regs->scaledWidthRatio * width16) & 0xFFF0000) / width16 + 1;
        }
        regs->scaledHeightRatio = MIN(65536, regs->scaledHeightRatio);
        regs->scaledWidthRatio = MIN(65536, regs->scaledWidthRatio);
    } else {
        regs->scaledWidth = regs->scaledHeight = 0;
        regs->scaledWidthRatio = regs->scaledHeightRatio = 0;
    }

    /* For rotated image, swap dimensions back to normal. */
    if(preProcess->rotation == 1 || preProcess->rotation == 2)
    {
        u32 tmp;

        tmp = width;
        width = height;
        height = tmp;
    }

    /* Set mandatory input parameters in asic structure */
    regs->mbsInRow = (width + 15) / 16;
    regs->mbsInCol = (height + 15) / 16;
    regs->pixelsOnRow = stride;

    /* Set the overfill values */
    if(width & 0x0F)
        regs->xFill = (16 - (width & 0x0F)) / 4;
    else
        regs->xFill = 0;

    if(height & 0x0F)
        regs->yFill = 16 - (height & 0x0F);
    else
        regs->yFill = 0;

    /* video stabilization */
    if(regs->codingType != ASIC_JPEG && preProcess->videoStab != 0)
        regs->vsMode = 2;
    else
        regs->vsMode = 0;

    /* Adaptive Region of Interest */
    if (!preProcess->roiMapEnable)
    {
        /* only enable the adaptive process if Map is not enable. */
        if (preProcess->adaptiveRoi)
            EncAdaptiveROI(asic, preProcess);
    }

#ifdef TRACE_PREPROCESS
    EncTracePreProcess(preProcess);
#endif

    return;
}

/*------------------------------------------------------------------------------

	EncSetColorConversion

	Set color conversion coefficients and RGB input mask

	Input	asic	Pointer to asicData_s structure
		preProcess Pointer to preProcess_s structure.

------------------------------------------------------------------------------*/
void EncSetColorConversion(preProcess_s * preProcess, asicData_s * asic)
{
    regValues_s *regs;

    ASSERT(asic != NULL && preProcess != NULL);

    regs = &asic->regs;

    switch (preProcess->colorConversionType)
    {
        case 0:         /* BT.601 */
        default:
            /* Y  = 0.2989 R + 0.5866 G + 0.1145 B
             * Cb = 0.5647 (B - Y) + 128
             * Cr = 0.7132 (R - Y) + 128
             */
            preProcess->colorConversionType = 0;
            regs->colorConversionCoeffA = preProcess->colorConversionCoeffA = 19589;
            regs->colorConversionCoeffB = preProcess->colorConversionCoeffB = 38443;
            regs->colorConversionCoeffC = preProcess->colorConversionCoeffC = 7504;
            regs->colorConversionCoeffE = preProcess->colorConversionCoeffE = 37008;
            regs->colorConversionCoeffF = preProcess->colorConversionCoeffF = 46740;
            break;

        case 1:         /* BT.709 */
            /* Y  = 0.2126 R + 0.7152 G + 0.0722 B
             * Cb = 0.5389 (B - Y) + 128
             * Cr = 0.6350 (R - Y) + 128
             */
            regs->colorConversionCoeffA = preProcess->colorConversionCoeffA = 13933;
            regs->colorConversionCoeffB = preProcess->colorConversionCoeffB = 46871;
            regs->colorConversionCoeffC = preProcess->colorConversionCoeffC = 4732;
            regs->colorConversionCoeffE = preProcess->colorConversionCoeffE = 35317;
            regs->colorConversionCoeffF = preProcess->colorConversionCoeffF = 41615;
            break;

        case 2:         /* User defined */
            /* Limitations for coefficients: A+B+C <= 65536 */
            regs->colorConversionCoeffA = preProcess->colorConversionCoeffA;
            regs->colorConversionCoeffB = preProcess->colorConversionCoeffB;
            regs->colorConversionCoeffC = preProcess->colorConversionCoeffC;
            regs->colorConversionCoeffE = preProcess->colorConversionCoeffE;
            regs->colorConversionCoeffF = preProcess->colorConversionCoeffF;
    }

    /* Setup masks to separate R, G and B from RGB */
    regs->rMaskMsb = rgbMaskBits[preProcess->inputFormat][0];
    regs->gMaskMsb = rgbMaskBits[preProcess->inputFormat][1];
    regs->bMaskMsb = rgbMaskBits[preProcess->inputFormat][2];
}

/*------------------------------------------------------------------------------
    SkinMb  Determine whether MB=i matches skin color
------------------------------------------------------------------------------*/
static u8 SkinMb(encOutputMbInfo_s * mbInfo, preProcess_s * preProcess)
{
    i32 y, cb, cr;
    i32 sensitivity = preProcess->adaptiveRoiColor;

    y = mbInfo->yMean;
    cb = mbInfo->cbMean + preProcess->cbComp;
    cr = mbInfo->crMean + preProcess->crComp;

    /* Luma limits for skin */
    if (y < 40 || y > 220) return 0;

    /* Filter out mbs with low MAD */
    if (mbInfo->inputMad_div128 <= H1ENC_AROI_SKIN_MAD_THRESHOLD)
        return 0;

    /* Skin tone region defined with five lines. */
    /* Sensitivity == color temperature -10 == 2000K (red), +10 == 6000K (blue) */
    if ((cr + cb >= 249) &&
        (cb >= 100-y/10-sensitivity) &&
        (cr + cb*3/4 <= 250) &&
        (cb <= 128+sensitivity/2) &&
        (cr - cb >= 2 + y/10 - y*sensitivity/100)) {
        return 1;
    }

    return 0;
}

/*------------------------------------------------------------------------------
    GetWbCompensation   Determine white balance compensation based on color of
                        high luminocity.
------------------------------------------------------------------------------*/
static void GetWbCompensation(preProcess_s * preProcess, asicData_s *asic)
{
    i32 mbPerRow = preProcess->lumWidth/16;
    i32 mbPerCol = preProcess->lumHeight/16;
    i32 mbPerFrame = mbPerRow * mbPerCol;
    i32 mb, i;
    i32 yhist[256] = {0};
    i32 ymax=0, ymax_cbavg=0, ymax_cravg=0, ymax_cnt=0;
    encOutputMbInfo_s *mbInfo;


    /* Create luminance histogram and find luminance maximum. */
    for (mb = 0; mb < mbPerFrame; mb++) {
        mbInfo = (encOutputMbInfo_s *)EncAsicGetMvOutput(asic, mb);
        yhist[mbInfo->yMean]++;
    }
    /* Find the highest luminance values, resolution affects how many values
       are needed. */
    for (i = 255; i > 150; i--) {
        ymax_cnt += yhist[i];
        if (ymax_cnt > mbPerFrame/512) break;
    }
    ymax = MIN(i, 235);

    /* Find the average chrominance of the highest luminance macroblocks. */
    ymax_cnt=0;
    for (mb = 0; mb < mbPerFrame; mb++) {
        mbInfo = (encOutputMbInfo_s *)EncAsicGetMvOutput(asic, mb);
        if (mbInfo->yMean >= ymax) {
            ymax_cnt++;
            ymax_cbavg += mbInfo->cbMean;
            ymax_cravg += mbInfo->crMean;
        }
    }
    if (!ymax_cnt) ymax_cnt++;
    ymax_cbavg /= ymax_cnt;
    ymax_cravg /= ymax_cnt;
    ymax = MIN(ymax, 235);

    /* Calculate the white balance compensation. */
    preProcess->cbComp = (127-ymax_cbavg)*20/(255-ymax);
    preProcess->crComp = (127-ymax_cravg)*20/(255-ymax);

    /* Don't try to compensate if low lightness. */
    if (ymax <= 150 || ymax_cnt <= mbPerFrame/512)
        preProcess->cbComp = preProcess->crComp = 0;
}

/*------------------------------------------------------------------------------
    FilterMap  Perform filtering on skin map
------------------------------------------------------------------------------*/
static void FilterMap(u8 *skinmapf, u8 *skinmap, i32 mbPerRow, i32 mbPerCol,
              i32 boxw, i32 boxh)
{
    i32 mbPerFrame = mbPerRow*mbPerCol;
    i32 mb, x, y, j, k;
    i32 cnt;
    i32 weight;

    /* Rectancular filter with given dimensions: 2*boxw+1, 2*boxh+1 */
    weight = mbPerCol*2/3;
    for (mb = mbPerRow; mb < mbPerRow*boxh; mb++)
        skinmapf[mb] = 0;
    for (y = boxh; y < mbPerCol-boxh; y++) {
        for (x = boxw; x < mbPerRow-boxw; x++) {
            cnt = 0;
            for (j = y-boxh; j <= y+boxh; j++) {
                for (k = x-boxw; k <= x+boxw; k++) {
                    cnt += skinmap[j*mbPerRow+k];
                }
            }
            /* Weight filter based on location (MB row) */
            if (y < mbPerCol/3)
                cnt *= 2*weight+y;
            else
                cnt *= 3*weight-y;

            skinmapf[y*mbPerRow+x] = MIN(255, cnt/mbPerCol);
        }
    }
    for (mb = mbPerFrame-mbPerRow*boxh; mb < mbPerFrame-mbPerRow; mb++)
        skinmapf[mb] = 0;
}

/*------------------------------------------------------------------------------
    ScoreMb  Score for MB=i based on skin, mv and previous roi maps
------------------------------------------------------------------------------*/
static u8 ScoreMb(i32 i, preProcess_s * preProcess)
{
    u8 *skinmapf = preProcess->skinMap[1];  /* Skin map after filter */
    i32 *mvMap = preProcess->mvMap;
    i32 motionScore = 0;

    if (preProcess->adaptiveRoiMotion < 0) {
        i32 mvThreshold = -preProcess->mvFrames*preProcess->adaptiveRoiMotion;

        /* Negative motion sensitivity rules out MBs below threshold. */
        if ((preProcess->mvFrames > 2) && (mvMap[i] < mvThreshold)) return 0;
    } else if (preProcess->adaptiveRoiMotion > 0) {
        i32 mvThreshold = preProcess->mvFrames*preProcess->adaptiveRoiMotion;

        /* Positive motion sensitivity includes MBs above threshold. */
        if ((preProcess->mvFrames) && (mvMap[i] > mvThreshold))
            motionScore = mvMap[i]/preProcess->mvFrames;
    }

    return MIN(255, skinmapf[i] + motionScore);
}

/*------------------------------------------------------------------------------

	EncAdaptiveROI

	Set color conversion coefficients and RGB input mask

	Input	asic	Pointer to asicData_s structure
		preProcess Pointer to preProcess_s structure.

------------------------------------------------------------------------------*/
void EncAdaptiveROI(asicData_s * asic, preProcess_s * preProcess)
{
    i32 mbPerRow = preProcess->lumWidth/16;
    i32 mbPerCol = preProcess->lumHeight/16;
    i32 mbPerFrame = mbPerRow * mbPerCol;
    i32 skinCount = 0;
    i32 mapCount = 0;
    i32 prevSkinCount = 0;
    i32 threshold, mb;
    i32 boxh, boxw;
    i32 reuseMap = 0, clearMap = 0;
    u8 *skinmap = preProcess->skinMap[0];   /* Skin detected mbs */
    u8 *skinmapf = preProcess->skinMap[1];  /* Skin map after filter */
    u8 *roimap;
    encOutputMbInfo_s *mbInfo;

    /* Macroblock filter box dimensions. */
    preProcess->boxh = boxh = MAX(1, H1ENC_AROI_FILTER_BOX_HEIGHT);
    preProcess->boxw = boxw = MAX(1, H1ENC_AROI_FILTER_BOX_WIDTH);

    ASSERT(asic);

    if (preProcess->roiCoded) {
        /* The map from previous frame was coded into stream => save it */
        EWLmemcpy(preProcess->roiSegmentMap[1],
                  preProcess->roiSegmentMap[0], mbPerFrame);
        preProcess->roiMbCount[1] = preProcess->roiMbCount[0];
        preProcess->roiCoded = 0;
    }

    if (preProcess->intra) {
        /* Intra frame resets segment map. AROI not used for intra frames. */
        if (preProcess->roiMbCount[1])
            EWLmemset(preProcess->roiSegmentMap[1], 0, mbPerFrame);
        if (preProcess->roiMbCount[2])
            EWLmemset(preProcess->roiSegmentMap[2], 0, mbPerFrame);
        if (preProcess->mvFrames)
            EWLmemset(preProcess->mvMap, 0, mbPerFrame*sizeof(i32));
        preProcess->roiUpdate = 0;
        preProcess->roiMbCount[1] = 0;
        preProcess->roiMbCount[2] = 0;
        preProcess->prevMapCount = 999;
        preProcess->mvFrames = 0;
#ifdef DEBUG_AROI
        TraceAroi(asic, preProcess, 9, preProcess->roiMbCount[2]);
        TraceAroiYuv(asic, preProcess);
#endif
        return;
    }

    if (preProcess->adaptiveRoiMotion) {
        preProcess->mvFrames++;
        /* Cumulative sum for macroblock motion. */
        for (mb = 0; mb < mbPerFrame; mb++) {
            mbInfo = (encOutputMbInfo_s *)EncAsicGetMvOutput(asic, mb);
            preProcess->mvMap[mb] += ABS(mbInfo->mvX[0]) + ABS(mbInfo->mvY[0]);
        }
    }

#ifdef DEBUG_AROI
    if (preProcess->adaptiveRoiMotion)
        TraceAroi(asic, preProcess, 1, 0);
#endif

    /* Force using map atleast a defined number of frames after update. */
    if (preProcess->prevMapCount < H1ENC_AROI_MIN_UPDATE_FRAMES) {
        preProcess->prevMapCount++;
        preProcess->roiUpdate = 0;
    } else {

        /* Try to detect if the frame white balance is invalid and determine
           chroma compensation to fix the white balance. */
        if (H1ENC_AROI_WHITE_BALANCE_FIX)
            GetWbCompensation(preProcess, asic);

        /* Find macroblocks with skin color and motion vectors for non-skin. */
        for (mb = 0; mb < mbPerFrame; mb++) {
            mbInfo = (encOutputMbInfo_s *)EncAsicGetMvOutput(asic, mb);
            skinmap[mb] = SkinMb(mbInfo, preProcess);
            if (skinmap[mb]) skinCount++;
        }

#ifdef DEBUG_AROI
        TraceAroi(asic, preProcess, 0, skinCount);
#endif

        /* Filter based on found skin macroblocks. */
        FilterMap(skinmapf, skinmap, mbPerRow, mbPerCol, boxw, boxh);

        /* Threshold for macroblock scores based on filter area, adjusted
           by the percentage of skin MBs / frame and motion sensitivity. */
        threshold = (boxh*2+1)*(boxw*2+1)*H1ENC_AROI_SCORE_THRESHOLD;
        if (skinCount)
            threshold = MAX(1, threshold - threshold*mbPerFrame/skinCount/200);
        if (preProcess->adaptiveRoiMotion > 0)
            threshold += preProcess->adaptiveRoiMotion;

        /* Score thresholding to create ROI segment map */
        roimap = preProcess->roiSegmentMap[0];
        for (mb = mbPerRow; mb < mbPerFrame-mbPerRow; mb++) {
            preProcess->scoreMap[mb] = ScoreMb(mb, preProcess);
            roimap[mb] = (preProcess->scoreMap[mb] >= threshold);
        }

        /* Remove one macroblock holes in segment map */
        for (mb = mbPerRow; mb < mbPerFrame-mbPerRow; mb++) {
            roimap[mb] = (roimap[mb-1] + roimap[mb] + roimap[mb+1]) > 1;
        }

        /* Compare previous and new map to find out if it is worth to update */
        skinCount = mapCount = 0;
        for (mb = mbPerRow; mb < mbPerFrame-mbPerRow; mb++) {
            if (preProcess->roiSegmentMap[0][mb] && preProcess->roiSegmentMap[1][mb])
                mapCount++; /* Overlapping maps */
            if (preProcess->roiSegmentMap[0][mb])
                skinCount++;
        }
        preProcess->roiMbCount[0] = skinCount;
        prevSkinCount = preProcess->roiMbCount[1];

        /* Threshold for changing map, adjust the threshold from 80% => 95%
           as more frames get coded with the same map. */
        threshold = MIN(H1ENC_AROI_UPDATE_THRESHOLD+preProcess->prevMapCount, 95);

        /* Threshold for not finding a small area to focus on */
        if (skinCount > mbPerFrame/3) {
            /* New skin map is too big, we can either use the previous map or
               no map at all. */
            if (mapCount >= prevSkinCount*threshold/100) reuseMap = 1;
            else clearMap = 1;
        } else if ((mapCount >= skinCount*threshold/100) &&
                   (mapCount >= prevSkinCount*threshold/100)) {
            reuseMap = 1;
        }

        if (clearMap) {
            /* Clear segment map. */
            if (preProcess->roiMbCount[0])
                EWLmemset(preProcess->roiSegmentMap[0], 0, mbPerFrame);
            if (preProcess->roiMbCount[2])
                EWLmemset(preProcess->roiSegmentMap[2], 0, mbPerFrame);
            if (preProcess->mvFrames)
                EWLmemset(preProcess->mvMap, 0, mbPerFrame*sizeof(i32));
            preProcess->roiMbCount[0] = 0;
            preProcess->roiMbCount[2] = 0;
            preProcess->mvFrames = 0;
            preProcess->prevMapCount = 999;
            preProcess->roiUpdate = 1;
        } else if (reuseMap) {
            /* Reuse previous map for this frame. */
            EWLmemcpy(preProcess->roiSegmentMap[0],
                      preProcess->roiSegmentMap[1], mbPerFrame);
            preProcess->roiMbCount[0] = preProcess->roiMbCount[1];
            preProcess->prevMapCount++;
            preProcess->roiUpdate = 0;
        } else {
            /* New segment map for ROI. */
            preProcess->prevMapCount = 1;

            /* Reset motion map for each new ROI */
            if (preProcess->mvFrames)
                EWLmemset(preProcess->mvMap, 0, mbPerFrame*sizeof(i32));
            preProcess->mvFrames = 0;

            /* Expand the new segment map with the size of filter box. */
            EWLmemset(preProcess->roiSegmentMap[2], 0, mbPerFrame);
            for (mb = boxh*mbPerRow+boxw; mb < mbPerFrame-mbPerRow*boxh-boxw; mb++) {
                i32 i,j;

                if (preProcess->roiSegmentMap[0][mb]) {
                    for (i = 0; i <= boxw; i++) {
                        for (j = 0; j <= boxh; j++) {
                            preProcess->roiSegmentMap[2][mb+j*mbPerRow+i] = 1;
                            preProcess->roiSegmentMap[2][mb+j*mbPerRow-i] = 1;
                            preProcess->roiSegmentMap[2][mb-j*mbPerRow+i] = 1;
                            preProcess->roiSegmentMap[2][mb-j*mbPerRow-i] = 1;
                        }
                    }
                }
            }
            skinCount = 0;
            for (mb = 0; mb < mbPerFrame; mb++) {
                if (preProcess->roiSegmentMap[2][mb]) skinCount++;
            }
            /* Segments will be updated based on ROI MB counts */
            preProcess->roiMbCount[2] = skinCount;
            preProcess->roiUpdate = 1;
        }

    }

#ifdef DEBUG_AROI
    TraceAroi(asic, preProcess, 9, preProcess->roiMbCount[2]);
    TraceAroiYuv(asic, preProcess);
#endif
}

#ifdef DEBUG_AROI
/*------------------------------------------------------------------------------
    TraceAroi       Write aroi.trc file with adaptive ROI information
------------------------------------------------------------------------------*/
void TraceAroi(asicData_s * asic, preProcess_s * preProcess, i32 stage, i32 value)
{
    i32 mbPerRow = preProcess->lumWidth/16;
    i32 mbPerCol = preProcess->lumHeight/16;
    i32 mbPerFrame = mbPerRow * mbPerCol;
    i32 mb;
    static i32 frameNum = 0;
    static FILE *ftrc;

    if (!ftrc) ftrc = fopen("aroi.trc", "w");

    /* Print out the skin scores. */
    if (stage == 0) {
        fprintf(ftrc, "Frame=%d skin scores, chroma compensation %d, %d\n",
                frameNum, preProcess->cbComp, preProcess->crComp);
        if (DEBUG_AROI > 1) for (mb = 0; mb < mbPerFrame; mb++) {
            fprintf(ftrc, "%2d ", preProcess->skinMap[0][mb]);
            if (mb%mbPerRow == mbPerRow-1) fprintf(ftrc, "\n");
        }
        fprintf(ftrc, "Skin count: %d\n\n", value);
    }

    /* Print out the motion map. */
    if (stage == 1) {
        fprintf(ftrc, "Frame=%d motion map\n", frameNum);
        if (DEBUG_AROI > 1) for (mb = 0; mb < mbPerFrame; mb++) {
            fprintf(ftrc, "%4d ", preProcess->mvMap[mb]);
            if (mb%mbPerRow == mbPerRow-1) fprintf(ftrc, "\n");
        }
    }

    /* Print out the final roi, its motion and skin count. */
    if (stage == 9) {
        fprintf(ftrc, "Frame=%d final roi map, box filter %dx%d\n",
                frameNum, preProcess->boxw*2+1, preProcess->boxh*2+1);
        fprintf(ftrc, "Frame=%d Map update %d\n", frameNum, preProcess->roiUpdate);
        fprintf(ftrc, "Frame=%d MapCount=%d\n",
                frameNum, preProcess->prevMapCount);
        if (DEBUG_AROI > 1) for (mb = 0; mb < mbPerFrame; mb++) {
            fprintf(ftrc, "%2d ", preProcess->roiSegmentMap[2][mb]);
            if (mb%mbPerRow == mbPerRow-1) fprintf(ftrc, "\n");
        }
        fprintf(ftrc, "Frame=%d MapSkinCount=%d (%d%%)\n",
                frameNum, value, value*100/mbPerFrame);
        fprintf(ftrc, "\n\n");
        frameNum++;
    }
}

/*------------------------------------------------------------------------------
    TraceAroiYuv    Write aroi.yuv file with ROI edges visible
------------------------------------------------------------------------------*/
void TraceAroiYuv(asicData_s * asic, preProcess_s * preProcess)
{
    i32 mbPerRow = preProcess->lumWidth/16;
    i32 mbPerCol = preProcess->lumHeight/16;
    i32 x, y, i;
    i32 b=0;
    u8 *roimap = preProcess->roiSegmentMap[0];  /* Current frame */
    u8 *roimape = preProcess->roiSegmentMap[2]; /* Current frame expanded */
    u8 *skinmap = preProcess->skinMap[0];   /* Skin mbs, for each frame */
    u8 *skinmapf = preProcess->skinMap[1];  /* Skin map after filter */
    u32 skincbhist[256] = {0};
    u32 skincrhist[256] = {0};
    encOutputMbInfo_s *mbInfo;
    i32 lum, cb, cr, threshold;
    i32 skincnt=0, skincbavg=0, skincravg=0;
    i32 cbcnt,crcnt,cbmed,crmed;
    static FILE *file[10] = {0};

    /* Full resolution image with visible AROI edges. */
    if (!file[0]) file[0] = fopen("aroi.yuv", "w");
    /* One pixel per MB files for AROI stages. */
    if (!file[1]) file[1] = fopen("mb_cb.yuv", "w");
    if (!file[2]) file[2] = fopen("mb_cr.yuv", "w");
    if (!file[3]) file[3] = fopen("mb_lum.yuv", "w");
    if (!file[4]) file[4] = fopen("mb_roi1.yuv", "w");
    if (!file[5]) file[5] = fopen("mb_roi2.yuv", "w");
    if (!file[6]) file[6] = fopen("mb_roi3.yuv", "w");
    if (!file[7]) file[7] = fopen("mb_roi4.yuv", "w");
    if (!file[8]) file[8] = fopen("mb_roi5.yuv", "w");
    if (!file[9]) file[9] = fopen("mb_roi6.yuv", "w");

    /* Chroma component */
    if (file[1]) {
        for (y = 0; y < mbPerCol; y++) {
            for (x = 0; x < mbPerRow; x++) {
                i = (y)*mbPerRow + x;
                mbInfo = (encOutputMbInfo_s *)EncAsicGetMvOutput(asic, i);
                cb = mbInfo->cbMean;
                if (skinmap[i]) { skincnt++; skincbavg += cb; skincbhist[cb]++; }
                fwrite(&cb, 1, 1, file[1]);
            }
        }
        cr=128;
        for (y = 0; y < mbPerCol*mbPerRow/2; y++) fwrite(&cr, 1, 1, file[1]);
    }

    /* Chroma component */
    if (file[2]) {
        for (y = 0; y < mbPerCol; y++) {
            for (x = 0; x < mbPerRow; x++) {
                i = (y)*mbPerRow + x;
                mbInfo = (encOutputMbInfo_s *)EncAsicGetMvOutput(asic, i);
                cr = mbInfo->crMean;
                if (skinmap[i]) { skincravg += cr; skincrhist[cr]++; }
                fwrite(&cr, 1, 1, file[2]);
            }
        }
        cr=128;
        for (y = 0; y < mbPerCol*mbPerRow/2; y++) fwrite(&cr, 1, 1, file[2]);
    }

    /* Luma component */
    if (file[3]) {
        for (y = 0; y < mbPerCol; y++) {
            for (x = 0; x < mbPerRow; x++) {
                i = (y)*mbPerRow + x;
                mbInfo = (encOutputMbInfo_s *)EncAsicGetMvOutput(asic, i);
                lum = mbInfo->yMean;
                fwrite(&lum, 1, 1, file[3]);
            }
        }
        cr=128;
        for (y = 0; y < mbPerCol*mbPerRow/2; y++) fwrite(&cr, 1, 1, file[3]);
    }

    /* Average chroma value on detected skin mbs */
    if (skincnt) {
        skincbavg /= skincnt;
        skincravg /= skincnt;
    }

    /* Median chroma value on detected skin mbs */
    crcnt=cbcnt=crmed=cbmed=0;
    for (i = 0; i < 256; i++) {
        cbcnt += skincbhist[i];
        crcnt += skincrhist[i];
        if (cbcnt > skincnt/2 && cbmed == 0)
            cbmed = i;
        if (crcnt > skincnt/2 && crmed == 0)
            crmed = i;
    }

    /* Detected skin mbs */
    if (file[4]) {
        for (y = 0; y < mbPerCol; y++) {
            for (x = 0; x < mbPerRow; x++) {
                i = (y)*mbPerRow + x;
                lum = MIN(255, skinmap[i]*250);
                fwrite(&lum, 1, 1, file[4]);
            }
        }
        cr=128;
        for (y = 0; y < mbPerCol*mbPerRow/2; y++) fwrite(&cr, 1, 1, file[4]);
    }

    /* Filtered skin mbs without thresholding */
    FilterMap(skinmapf, skinmap, mbPerRow, mbPerCol,
                        preProcess->boxw, preProcess->boxh);
    if (file[5]) {
        for (y = 0; y < mbPerCol; y++) {
            for (x = 0; x < mbPerRow; x++) {
                i = (y)*mbPerRow + x;
                lum = MIN(255, skinmapf[i]*2);
                fwrite(&lum, 1, 1, file[5]);
            }
        }
        cr=128;
        for (y = 0; y < mbPerCol*mbPerRow/2; y++) fwrite(&cr, 1, 1, file[5]);
    }

    /* Filtered skin mbs with threshold */
    threshold = (preProcess->boxh*2+1)*(preProcess->boxw*2+1)*2/3*4/3;
    if (file[6]) {
        for (y = 0; y < mbPerCol; y++) {
            for (x = 0; x < mbPerRow; x++) {
                i = (y)*mbPerRow + x;
                lum = MIN(255, skinmapf[i]*2);
                if (skinmapf[i] < threshold) lum=0;
                fwrite(&lum, 1, 1, file[6]);
            }
        }
        cr=128;
        for (y = 0; y < mbPerCol*mbPerRow/2; y++) fwrite(&cr, 1, 1, file[6]);
    }

    /* Macroblock scores */
    if (file[7]) {
        for (y = 0; y < mbPerCol; y++) {
            for (x = 0; x < mbPerRow; x++) {
                i = (y)*mbPerRow + x;
                lum = MIN(255, preProcess->scoreMap[i]*2);
                fwrite(&lum, 1, 1, file[7]);
            }
        }
        cr=128;
        for (y = 0; y < mbPerCol*mbPerRow/2; y++) fwrite(&cr, 1, 1, file[7]);
    }

    /* Score thresholding */
    if (file[8]) {
        for (y = 0; y < mbPerCol; y++) {
            for (x = 0; x < mbPerRow; x++) {
                i = (y)*mbPerRow + x;
                lum = MIN(255, roimap[i]*255);
                fwrite(&lum, 1, 1, file[8]);
            }
        }
        cr=128;
        for (y = 0; y < mbPerCol*mbPerRow/2; y++) fwrite(&cr, 1, 1, file[8]);
    }

    /* Cumulative MV sum */
    if (file[9]) {
        for (y = 0; y < mbPerCol; y++) {
            for (x = 0; x < mbPerRow; x++) {
                i = (y)*mbPerRow + x;
                lum = MIN(255, preProcess->mvMap[i]*2);
                fwrite(&lum, 1, 1, file[9]);
            }
        }
        cr=128;
        for (y = 0; y < mbPerCol*mbPerRow/2; y++) fwrite(&cr, 1, 1, file[9]);
    }

    /* Reconstructed frame luma with visible ROI edges */
    if (file[0]) {
        for (y = 0; y < mbPerCol*16; y++) {
            for (x = 0; x < mbPerRow*16; x++) {
                i32 edge = 0;
                i = (y/16)*mbPerRow + x/16;

                /* Detect edges of ROI macroblocks */
                if (x > 15 && x < (mbPerRow-1)*16 &&
                    y > 15 && y < (mbPerCol-1)*16) {
                    if ((x%16 == 0)  && roimape[i] && (roimape[i-1] == 0)) edge = 1;
                    if ((x%16 == 15) && roimape[i] && (roimape[i+1] == 0)) edge = 1;
                    if ((y%16 == 0)  && roimape[i] && (roimape[i-mbPerRow] == 0)) edge = 1;
                    if ((y%16 == 15) && roimape[i] && (roimape[i+mbPerRow] == 0)) edge = 1;
                }

                if (edge)
                    fwrite(&b, 1, 1, file[0]);
                else
                    /* Previous reconstructed frame luma */
                    if (asic->regs.internalImageLumBaseR[0]) {
                        fwrite((u8*)(asic->regs.internalImageLumBaseR[0])+
                                    y*preProcess->lumWidth+x, 1, 1, file[0]);
                     } else {
                        mbInfo = (encOutputMbInfo_s *)EncAsicGetMvOutput(asic, i);
                        fwrite(&mbInfo->yMean, 1, 1, file[0]);
                    }
            }
        }
        /* TBD use actual chroma, not mean */
        for (y = 0; y < mbPerCol*8; y++) {
            for (x = 0; x < mbPerRow*8; x++) {
                i = (y/8)*mbPerRow + x/8;
                mbInfo = (encOutputMbInfo_s *)EncAsicGetMvOutput(asic, i);
                cb = mbInfo->cbMean + preProcess->cbComp;
                fwrite(&cb, 1, 1, file[0]);
            }
        }
        for (y = 0; y < mbPerCol*8; y++) {
            for (x = 0; x < mbPerRow*8; x++) {
                i = (y/8)*mbPerRow + x/8;
                mbInfo = (encOutputMbInfo_s *)EncAsicGetMvOutput(asic, i);
                cr = mbInfo->crMean + preProcess->crComp;
                fwrite(&cr, 1, 1, file[0]);
            }
        }
    }

}
#endif

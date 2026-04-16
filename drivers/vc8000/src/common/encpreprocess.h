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
#ifndef __ENC_PRE_PROCESS_H__
#define __ENC_PRE_PROCESS_H__

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "basetype.h"
#include "encasiccontroller.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

#define    ROTATE_0     0U
#define    ROTATE_90R   1U  /* Rotate 90 degrees clockwise */
#define    ROTATE_90L   2U  /* Rotate 90 degrees counter-clockwise */

/* maximum input picture width set by the available bits in ASIC regs */
#define    MAX_INPUT_IMAGE_WIDTH   (8192)

typedef struct
{
    u32 lumWidthSrc;    /* Source input image width */
    u32 lumHeightSrc;   /* Source input image height */
    u32 lumWidth;   /* Encoded image width */
    u32 lumHeight;  /* Encoded image height */
    u32 scaledWidth;   /* Scaled output image width */
    u32 scaledHeight;  /* Scaled output image height */
    u32 horOffsetSrc;   /* Encoded frame offset, reference is ... */
    u32 verOffsetSrc;   /* ...top  left corner of source image */
    u32 inputFormat;
    u32 rotation;
    u32 videoStab;
    u32 scaledOutput;
    u32 colorConversionType;    /* 0 = bt601, 1 = bt709, 2 = user defined */
    u32 colorConversionCoeffA;
    u32 colorConversionCoeffB;
    u32 colorConversionCoeffC;
    u32 colorConversionCoeffE;
    u32 colorConversionCoeffF;
    i32 roiMapEnable;
    i32 adaptiveRoi;
    i32 qpOffset[3];
    i32 adaptiveRoiColor;   /* Color temperature -10..10 = 2000K..5000K */
    i32 adaptiveRoiMotion;  /* Motion sensitivity -10..10 */
    u32 prevMapCount;       /* How many frames previous map is used in. */
    u32 intra;
    u32 mvFrames;           /* How many frames mvMap is counted for. */
    u32 cbComp;             /* Chroma compensation for white balance fix. */
    u32 crComp;
    u32 boxw;               /* Filter box dimensions. */
    u32 boxh;
    u8 *skinMap[2];         /* Skin MBs before/after filtering */
    u8 *roiSegmentMap[3];   /* MB maps, 0=curr, 1=prev, 2=curr expanded */
    u32 roiMbCount[3];      /* Amount of MBs in each of above maps */
    i32 *mvMap;             /* Motion map based on MB MVs */
    u8 *scoreMap;           /* Skin + motion score for each MB */
    u32 roiUpdate;          /* AROI calculation has updated roiSegmentMap[2] */
    u32 roiCoded;           /* The new ROI map has been coded to stream */
    u32 interlacedFrame;    /* Enable interlaced frame input */
    u32 bottomField;        /* Current picture is interlaced bottom field */
} preProcess_s;

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/
i32 EncPreProcessAlloc(preProcess_s * preProcess, i32 mbPerPicture);
void EncPreProcessFree(preProcess_s * preProcess);
i32 EncPreProcessCheck(const preProcess_s * preProcess);
void EncPreProcess(asicData_s * asic, preProcess_s * preProcess);
void EncSetColorConversion(preProcess_s * preProcess, asicData_s * asic);

#endif

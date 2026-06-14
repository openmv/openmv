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
 *  Abstract : 
 *
 ********************************************************************************
 */

#ifndef H264PICTURE_BUFFER_H
#define H264PICTURE_BUFFER_H

/*------------------------------------------------------------------------------
	Include headers
------------------------------------------------------------------------------*/
#include "basetype.h"
#include "encasiccontroller.h"

/*------------------------------------------------------------------------------
	External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
	Module defines
------------------------------------------------------------------------------*/
#define BUFFER_SIZE	4   /* to support svct */

typedef struct {
	i32 lumWidth;		/* Width of *lum */
	i32 lumHeight;		/* Height of *lum */
	i32 chWidth;		/* Width of *cb and *cr */
	i32 chHeight;		/* Height of *cb and *cr */
	ptr_t lum;
	ptr_t cb;
} picture;

typedef struct refPic {
	picture picture;	/* Image data */
	i32 poc;		/* Picture order count */
	i32 frameNum;

	bool i_frame;		/* I frame (key frame), only intra mb */
	bool p_frame;		/* P frame, intra and inter mb */
	bool show;		/* Frame is for display (showFrame flag) */
	bool ipf;		/* Frame is immediately previous frame */
	bool arf;		/* Frame is altref frame */
	bool grf;		/* Frame is golden frame */
	bool search;		/* Frame is used for motion estimation */
	struct refPic *refPic;	/* Back reference pointer to itself */
	i32  orderCmd;
} refPic;

typedef struct {
	i32 size;		/* Amount of allocated reference pictures */
	picture input;		/* Input picture */
	refPic refPic[BUFFER_SIZE + 1];	/* Reference picture store */
	refPic refPicList[BUFFER_SIZE];	/* Reference picture list */
	refPic *cur_pic;	/* Pointer to picture under reconstruction */
	refPic *last_pic;	/* Last picture */
} picBuffer;

/*------------------------------------------------------------------------------
	Function prototypes
------------------------------------------------------------------------------*/
i32 H264PictureBufferAlloc(picBuffer *picBuffer, i32 width, i32 height);
void H264PictureBufferFree(picBuffer *picBuffer);
void H264InitializePictureBuffer(picBuffer *picBuffer);
void H264UpdatePictureBuffer(picBuffer *picBuffer);
void H264PictureBufferSetRef(picBuffer *picBuffer, asicData_s *asic, u32 numViews);
i32 H264PictureBufferSetupH264(picBuffer *picBuffer, asicData_s *asic, u32 numRefBuffsLum, u32 numRefBuffsChr);
void H264UpdatePictureBufferForSvct(picBuffer *picBuffer, u32 frameNumBits, i32 gopIndex);
void H264PictureBufferSetRefForSvct(picBuffer *picBuffer, asicData_s *asic, u32 frameNumBits);

#endif

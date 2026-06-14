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
 *  Abstract  :   NAL unit handling
 *
 ********************************************************************************
 */

#ifndef __H264_NAL_UNIT_H__
#define __H264_NAL_UNIT_H__

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "basetype.h"
#include "H264PutBits.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

typedef struct
{
    u32 anchorPicFlag;
    u32 priorityId;
    u32 viewId;
    u32 temporalId;
    u32 interViewFlag;
} mvc_s;

#define MAX_GOP_SIZE 16

typedef struct
{
    int layer;
    int isRef;
    int orderCmd;
    int markCmd;
} gopinfo_s;

typedef struct 
{
    u32 level;
    gopinfo_s gop[MAX_GOP_SIZE];
    i32 gopIndex;  /* -1: IDR; 0~GOP_SIZE-1: index in gop */
    i32 gopLength;
    u32 next_base_priority_id;
    u32 base_priority_id;
    u32 enableSvctPrefix;
} svc_s;

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/
void H264NalUnitHdr(stream_s * stream, i32 nalRefIdc, nalUnitType_e
                    nalUnitType, true_e byteStream);
void H264NalUnitHdrMvcExtension(stream_s * stream, mvc_s * mvc);
void H264NalUnitHdrSvcExtension(stream_s * stream, svc_s * svc);
void H264NalUnitTrailinBits(stream_s * stream, true_e byteStream);
u32 H264FillerNALU(stream_s * sp, i32 cnt, true_e byteStream);

#endif

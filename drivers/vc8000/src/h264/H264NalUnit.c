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

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "enccommon.h"
#include "H264NalUnit.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

	H264NalUnit

------------------------------------------------------------------------------*/
void H264NalUnitHdr(stream_s * stream, i32 nalRefIdc, nalUnitType_e nalUnitType,
                    true_e byteStream)
{
    if(byteStream == ENCHW_YES)
    {
        H264PutBits(stream, 0, 8);
        COMMENT("BYTE STREAM: leadin_zero_8bits");

        H264PutBits(stream, 0, 8);
        COMMENT("BYTE STREAM: Start_code_prefix");

        H264PutBits(stream, 0, 8);
        COMMENT("BYTE STREAM: Start_code_prefix");

        H264PutBits(stream, 1, 8);
        COMMENT("BYTE STREAM: Start_code_prefix");
    }

    H264PutBits(stream, 0, 1);
    COMMENT("forbidden_zero_bit");

    H264PutBits(stream, nalRefIdc, 2);
    COMMENT("nal_ref_idc");

    H264PutBits(stream, (i32) nalUnitType, 5);
    COMMENT("nal_unit_type");

    stream->zeroBytes = 0; /* we start new counter for zero bytes */

}

/*------------------------------------------------------------------------------

	H264NalUnitHdrMvcExtension

------------------------------------------------------------------------------*/
void H264NalUnitHdrMvcExtension(stream_s * stream, mvc_s * mvc)
{

    H264PutBits(stream, !mvc->anchorPicFlag, 1);
    COMMENT("non_idr_flag");
    H264PutBits(stream, mvc->priorityId, 6);
    COMMENT("priority_id");
    H264PutBits(stream, mvc->viewId, 10);
    COMMENT("view_id");
    H264PutBits(stream, mvc->temporalId, 3);
    COMMENT("temporal_id");
    H264PutBits(stream, mvc->anchorPicFlag, 1);
    COMMENT("anchor_pic_flag");
    H264PutBits(stream, mvc->interViewFlag, 1);
    COMMENT("inter_view_flag");
    H264PutBits(stream, 1, 1);
    COMMENT("reserved_one_bit");

    stream->zeroBytes = 0; /* we start new counter for zero bytes */

}

/*------------------------------------------------------------------------------

	H264NalUnitHdrSvcExtension

------------------------------------------------------------------------------*/
void H264NalUnitHdrSvcExtension(stream_s * stream, svc_s * svc)
{
    u32 layer = (svc->gopIndex==-1)?(0):(svc->gop[svc->gopIndex].layer);
    H264PutBits(stream, (svc->gopIndex == -1), 1);
    COMMENT("idr_flag");
    H264PutBits(stream, layer, 6);
    COMMENT("priority_id");
    H264PutBits(stream, 1, 1);
    COMMENT("no_inter_layer_pred_flag");
    H264PutBits(stream, 0, 3);
    COMMENT("dependency_id");
    H264PutBits(stream, 0, 4);
    COMMENT("quality_id");
    H264PutBits(stream, layer, 3);
    COMMENT("temporal_id");
    H264PutBits(stream, 0, 1);
    COMMENT("use_ref_base_pic_flag");
    H264PutBits(stream, 1, 1);
    COMMENT("discardable_flag");
    H264PutBits(stream, 1, 1);
    COMMENT("output_flag");
    H264PutBits(stream, 3, 2);
    COMMENT("reserved_three_2bits");

    stream->zeroBytes = 0; /* we start new counter for zero bytes */

    /* prefix_nal_unit_svc() */
    if (layer != svc->level)
    {
        H264PutBits(stream, 0x20, 8);
        COMMENT("prefix_nal_unit_svc()");
    }

}

/*------------------------------------------------------------------------------

	H264NalUnitTrailinBits

------------------------------------------------------------------------------*/
void H264NalUnitTrailinBits(stream_s * stream, true_e byteStream)
{
    H264RbspTrailingBits(stream);

    if(byteStream == ENCHW_YES)
    {
#if 0   /* system model has removed this */
        H264PutBits(stream, 0, 8);
        COMMENT("BYTE STREAM: trailing_zero_8bits");
#endif
    }

    return;
}

u32 H264FillerNALU(stream_s * sp, i32 cnt, true_e byteStream)
{
    i32 i = cnt;
    u32 nal_size;

    nal_size = sp->byteCnt;

    ASSERT(sp != NULL);

    H264NalUnitHdr(sp, 0, FILLERDATA, byteStream);

    for(; i > 0; i--)
    {
        H264NalBits(sp, 0xFF, 8);
        COMMENT("filler ff_byte");
    }
    H264RbspTrailingBits(sp);

    nal_size = sp->byteCnt - nal_size;

    return nal_size;
}

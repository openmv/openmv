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
 *  Abstract  : Picture Parameter Set handling
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "H264PictureParameterSet.h"
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

	H264PicParameterInit

------------------------------------------------------------------------------*/
void H264PicParameterSetInit(pps_s * pps)
{
    pps->byteStream = ENCHW_YES;
    pps->picParameterSetId = 0;
    pps->seqParameterSetId = 0;
    pps->entropyCodingMode = ENCHW_NO;
    pps->picOrderPresent = ENCHW_NO;
    pps->numSliceGroupsMinus1 = 0;
    pps->numRefIdxL0ActiveMinus1 = 0;
    pps->numRefIdxL1ActiveMinus1 = 0;
    pps->weightedPred = ENCHW_NO;
    pps->weightedBipredIdc = 0;
    pps->picInitQpMinus26 = 0;
    pps->picInitQsMinus26 = 0;
    pps->chromaQpIndexOffset = 2;
    pps->deblockingFilterControlPresent = ENCHW_YES;
    pps->constIntraPred = ENCHW_NO;
    pps->redundantPicCntPresent = ENCHW_NO;
    pps->transform8x8Mode = ENCHW_NO;

    return;
}

/*------------------------------------------------------------------------------

	H264PicParameterSet

------------------------------------------------------------------------------*/
void H264PicParameterSet(stream_s * stream, pps_s * pps)
{
    /* Nal unit sytax */
    H264NalUnitHdr(stream, 1, PPSET, pps->byteStream);

    H264ExpGolombUnsigned(stream, pps->picParameterSetId);
    COMMENT("pic_parameter_set_id");

    H264ExpGolombUnsigned(stream, pps->seqParameterSetId);
    COMMENT("seq_parameter_set_id");

    H264NalBits(stream, (i32) pps->entropyCodingMode, 1);
    COMMENT("entropy_coding_mode_flag");

    H264NalBits(stream, (i32) pps->picOrderPresent, 1);
    COMMENT("pic_order_present_flag");

    H264ExpGolombUnsigned(stream, pps->numSliceGroupsMinus1);
    COMMENT("num_slice_groups_minus1");

    /* if( num_slice_groups_minus1 > 0 ) etc... not implementet yet */

    H264ExpGolombUnsigned(stream, pps->numRefIdxL0ActiveMinus1);
    COMMENT("num_ref_idx_l0_active_minus1");

    H264ExpGolombUnsigned(stream, pps->numRefIdxL1ActiveMinus1);
    COMMENT("num_ref_idx_l1_active_minus1");

    H264NalBits(stream, (i32) pps->weightedPred, 1);
    COMMENT("weighted_pred_flag");

    H264NalBits(stream, pps->weightedBipredIdc, 2);
    COMMENT("weighted_bipred_idc");

    H264ExpGolombSigned(stream, pps->picInitQpMinus26);
    COMMENT("pic_init_qp_minus26");

    H264ExpGolombSigned(stream, pps->picInitQsMinus26);
    COMMENT("pic_init_qs_minus26");

    H264ExpGolombSigned(stream, pps->chromaQpIndexOffset);
    COMMENT("chroma_qp_index_offset");

    H264NalBits(stream, (i32) pps->deblockingFilterControlPresent, 1);
    COMMENT("deblocking_filter_control_present_flag");

    H264NalBits(stream, (i32) pps->constIntraPred, 1);
    COMMENT("constrained_intra_pred_flag");

    H264NalBits(stream, (i32) pps->redundantPicCntPresent, 1);
    COMMENT("redundant_pic_cnt_present_flag");

    if (pps->transform8x8Mode == ENCHW_YES)
    {
        H264NalBits(stream, pps->transform8x8Mode, 1);
        COMMENT("transform_8x8_mode_flag");

        H264NalBits(stream, 0, 1);
        COMMENT("pic_scaling_matrix_present_flag");

        H264ExpGolombSigned(stream, pps->chromaQpIndexOffset);
        COMMENT("second_chroma_qp_index_offset");
    }


    H264NalUnitTrailinBits(stream, pps->byteStream);

    return;
}

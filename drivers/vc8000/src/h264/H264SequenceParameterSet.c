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
 *  Abstract : H264 Sequence Parameter Set
 *
 ********************************************************************************
 */

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "H264SequenceParameterSet.h"
#include "H264NalUnit.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

#define MAX_LEVEL_INDEX           (sizeof(H264LevelIdc)/sizeof(*H264LevelIdc))

/* max MV length is 512 horizontal and 128 vertical in quarter pixel resolution */
#define LOG2_MAX_MV_LENGTH_HOR        9
#define LOG2_MAX_MV_LENGTH_VER        7
#define EXTENDED_SAR            255

/* Level 1b indicated by value 99 */
const u32 H264LevelIdc[16] =
    { 10, 99, 11, 12, 13, 20, 21, 22, 30, 31, 32, 40, 41, 42, 50, 51 };

const u32 H264MaxCPBS[16] =
    { 210000, 420000, 600000, 1200000, 2400000, 2400000, 4800000, 4800000, 12000000,
    16800000, 24000000, 30000000, 75000000, 75000000, 162000000, 288000000
};

/* Level 51 MaxFS in standard is 36864, increased to enable max resolution */
const u32 H264MaxFS[16] = { 99, 99, 396, 396, 396, 396, 792, 1620, 1620,
    3600, 5120, 8192, 8192, 8704, 22080, 65025
};

/* sqrt(8*maxFrameSize) is maximum width and height of specific level */
const u32 H264SqrtMaxFS8[16] =
    { 28, 28, 56, 56, 56, 56, 79, 113, 113, 169, 202, 256, 256, 263, 420, 543 };

const u32 H264MaxMBPS[16] =
    { 1485, 1485, 3000, 6000, 11880, 11880, 19800, 20250, 40500,
    108000, 216000, 245760, 245760, 522240, 589824, 983040
};

const u32 H264MaxBR[16] =
    { 76800, 153600, 230400, 460800, 921600, 2400000, 4800000, 4800000, 12000000,
    16800000, 24000000, 24000000, 60000000, 60000000, 162000000, 288000000
};

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/

static void SeqParameterSetMvcExtension(stream_s * stream, sps_s * sps);
static void WriteVui(stream_s * strm, vui_t * vui, i32 numRefFrames);
static i32 GetAspectRatioIdc(i32 sarWidth, i32 sarHeight);
static void UpdateVuiPresence(sps_s * sps);

/*------------------------------------------------------------------------------

	H264SpsInit

------------------------------------------------------------------------------*/
void H264SeqParameterSetInit(sps_s * sps)
{
    sps->byteStream = ENCHW_YES;
    sps->profileIdc = 66;   /* 66 = baseline, 77 = main, 100 = high, 118 = MVC */
    sps->constraintSet0 = ENCHW_YES;
    sps->constraintSet1 = ENCHW_YES;
    sps->constraintSet2 = ENCHW_YES;
    sps->constraintSet3 = ENCHW_NO;

    sps->levelIdc = 30;
    sps->seqParameterSetId = 0;
    sps->log2MaxFrameNumMinus4 = (16 - 4);
    sps->picOrderCntType = 2;

    sps->numRefFrames = 1;
    sps->gapsInFrameNumValueAllowed = ENCHW_NO;
    sps->picWidthInMbsMinus1 = 176 / 16 - 1;
    sps->picHeightInMapUnitsMinus1 = 144 / 16 - 1;
    sps->frameMbsOnly = ENCHW_YES;
    sps->direct8x8Inference = ENCHW_YES;
    sps->frameCropping = ENCHW_NO;
    sps->vuiParametersPresent = ENCHW_YES;
    sps->vui.bitStreamRestrictionFlag = 1;

    sps->vui.videoFullRange = 0;
    sps->vui.sarWidth = 0;
    sps->vui.sarHeight = 0;

    sps->frameCropLeftOffset = 0;
    sps->frameCropRightOffset = 0;
    sps->frameCropTopOffset = 0;
    sps->frameCropBottomOffset = 0;

    return;
}

/*------------------------------------------------------------------------------

	H264SeqParameterSet

------------------------------------------------------------------------------*/
void H264SeqParameterSet(stream_s * stream, sps_s * sps, true_e nalHeader)
{
    /* Nal unit syntax */
    if (nalHeader == ENCHW_YES)
        H264NalUnitHdr(stream, 1, SPSET, sps->byteStream);

    H264NalBits(stream, sps->profileIdc, 8);
    COMMENT("profile_idc");

    /* constraint_set0 streams obey all baseline profile constraints. */
    if (sps->profileIdc > 66)
        sps->constraintSet0 = ENCHW_NO;
    /* constraint_set1 streams obey all main profile constraints. */
    if (sps->profileIdc > 77)
        sps->constraintSet1 = ENCHW_NO;
    /* constraint_set2 streams obey all extended profile constraints. */
    if (sps->profileIdc > 88)
        sps->constraintSet2 = ENCHW_NO;

    H264NalBits(stream, (i32) sps->constraintSet0, 1);
    COMMENT("constraint_set0_flag");

    H264NalBits(stream, (i32) sps->constraintSet1, 1);
    COMMENT("constraint_set1_flag");

    H264NalBits(stream, (i32) sps->constraintSet2, 1);
    COMMENT("constraint_set2_flag");

    H264NalBits(stream, (i32) sps->constraintSet3, 1);
    COMMENT("constraint_set3_flag");

    H264NalBits(stream, 0, 4);
    COMMENT("reserved_zero_4bits");

    H264NalBits(stream, sps->levelIdc, 8);
    COMMENT("level_idc");

    H264ExpGolombUnsigned(stream, sps->seqParameterSetId);
    COMMENT("seq_parameter_set_id");

    if (sps->profileIdc >= 100)
    {
        H264ExpGolombUnsigned(stream, 1);
        COMMENT("chroma_format_idc");

        H264ExpGolombUnsigned(stream, 0);
        COMMENT("bit_depth_luma_minus8");

        H264ExpGolombUnsigned(stream, 0);
        COMMENT("bit_depth_chroma_minus8");

        H264NalBits(stream, 0, 1);
        COMMENT("qpprime_y_zero_transform_bypass_flag");

        H264NalBits(stream, 0, 1);
        COMMENT("seq_scaling_matrix_present_flag");
    }

    H264ExpGolombUnsigned(stream, sps->log2MaxFrameNumMinus4);
    COMMENT("log2_max_frame_num_minus4");

    H264ExpGolombUnsigned(stream, sps->picOrderCntType);
    COMMENT("pic_order_cnt_type");

    H264ExpGolombUnsigned(stream, sps->numRefFrames);
    COMMENT("num_ref_frames");

    H264NalBits(stream, (i32) sps->gapsInFrameNumValueAllowed, 1);
    COMMENT("gaps_in_frame_num_value_allowed_flag");

    H264ExpGolombUnsigned(stream, sps->picWidthInMbsMinus1);
    COMMENT("pic_width_in_mbs_minus1");

    H264ExpGolombUnsigned(stream, sps->picHeightInMapUnitsMinus1);
    COMMENT("pic_height_in_map_units_minus1");

    H264NalBits(stream, (i32) sps->frameMbsOnly, 1);
    COMMENT("frame_mbs_only_flag");

    if (!sps->frameMbsOnly)
    {
        H264NalBits(stream, 0, 1);
        COMMENT("mb_adaptive_frame_field_flag");
    }

    H264NalBits(stream, (i32) sps->direct8x8Inference, 1);
    COMMENT("direct_8x8_inference_flag");

    H264NalBits(stream, (i32) sps->frameCropping, 1);
    COMMENT("frame_cropping_flag");

    /* Frame cropping parameters */
    if (sps->frameCropping)
    {
        H264ExpGolombUnsigned(stream, sps->frameCropLeftOffset);
        COMMENT("frame_crop_left_offset");
        H264ExpGolombUnsigned(stream, sps->frameCropRightOffset);
        COMMENT("frame_crop_right_offset");
        H264ExpGolombUnsigned(stream, sps->frameCropTopOffset);
        COMMENT("frame_crop_top_offset");
        H264ExpGolombUnsigned(stream, sps->frameCropBottomOffset);
        COMMENT("frame_crop_bottom_offset");
    }

    UpdateVuiPresence(sps);

#if 0
    /* Currently JMVC can't decode this */
    sps->vuiParametersPresent = ENCHW_NO;
#endif

    H264NalBits(stream, (i32) sps->vuiParametersPresent, 1);
    COMMENT("vui_parameters_present_flag");

    if(sps->vuiParametersPresent == ENCHW_YES)
        WriteVui(stream, &sps->vui, sps->numRefFrames);

    if (nalHeader == ENCHW_YES)
        H264NalUnitTrailinBits(stream, sps->byteStream);
}

void UpdateVuiPresence(sps_s * sps)
{

    if(sps->vui.nalHrdParametersPresentFlag == 0 &&
       sps->vui.timeScale == 0 &&
       sps->vui.pictStructPresentFlag == 0 &&
       sps->vui.sarWidth == 0 && sps->vui.videoFullRange == 0 &&
       sps->vui.bitStreamRestrictionFlag == 0)
    {
        sps->vuiParametersPresent = ENCHW_NO;
    }
}

/*------------------------------------------------------------------------------

	H264SubsetSeqParameterSet

------------------------------------------------------------------------------*/
void H264SubsetSeqParameterSet(stream_s * stream, sps_s * sps)
{
    /* Nal unit syntax */
    H264NalUnitHdr(stream, 1, SSPSET, sps->byteStream);

    sps->profileIdc = 128;  /* Stereo High profile */

    H264SeqParameterSet(stream, sps, ENCHW_NO);

    H264NalBits(stream, 1, 1);
    COMMENT("bit_equal_to_one");

    SeqParameterSetMvcExtension(stream, sps);

    H264NalBits(stream, 0, 1);
    COMMENT("mvc_vui_parameters_present_flag");

    H264NalBits(stream, 0, 1);
    COMMENT("additional_extension2_flag");

    H264NalUnitTrailinBits(stream, sps->byteStream);
}

/*------------------------------------------------------------------------------

	H264SubsetSeqParameterSet

------------------------------------------------------------------------------*/
static void SeqParameterSetMvcExtension(stream_s * stream, sps_s * sps)
{
    H264ExpGolombUnsigned(stream, 1);
    COMMENT("num_views_minus_1");

    H264ExpGolombUnsigned(stream, 0);
    COMMENT("view_id[0]");
    H264ExpGolombUnsigned(stream, 1);
    COMMENT("view_id[1]");

    H264ExpGolombUnsigned(stream, 1);
    COMMENT("num_anchor_refs_l0[1]");
    H264ExpGolombUnsigned(stream, 0);
    COMMENT("anchor_ref_l0[1]");
    H264ExpGolombUnsigned(stream, 0);
    COMMENT("num_anchor_refs_l1[1]");

    H264ExpGolombUnsigned(stream, 1);
    COMMENT("num_non_anchor_refs_l0[1]");
    H264ExpGolombUnsigned(stream, 0);
    COMMENT("non_anchor_ref_l0[1]");
    H264ExpGolombUnsigned(stream, 0);
    COMMENT("num_non_anchor_refs_l1[1]");

    H264ExpGolombUnsigned(stream, 0);
    COMMENT("num_level_values_signalled_minus1");
    H264NalBits(stream, sps->levelIdc, 8);
    COMMENT("level_idc");

    H264ExpGolombUnsigned(stream, 0);
    COMMENT("num_applicable_ops_minus1");
    H264NalBits(stream, 0, 3);
    COMMENT("applicable_op_temporal_id");
    H264ExpGolombUnsigned(stream, 0);
    COMMENT("applicable_op_num_target_view_minus1");
    H264ExpGolombUnsigned(stream, 1);
    COMMENT("applicable_op_target_view_id");
    H264ExpGolombUnsigned(stream, 1);
    COMMENT("applicable_op_num_views_minus1");


}

/*------------------------------------------------------------------------------

    Function: WriteVui

        Functional description:
          Write VUI params into the stream

        Inputs:
          vui_t *vui            pointer to VUI params structure
          u32 numRefFrames      number of reference frames, used as
                                max_dec_frame_buffering

        Outputs:
          stream_s *            pointer to stream data

------------------------------------------------------------------------------*/
static void WriteVui(stream_s * strm, vui_t * vui, i32 numRefFrames)
{

/* Variables */

    i32 sarIdc;

/* Code */

    ASSERT(strm);
    ASSERT(vui);

    sarIdc = GetAspectRatioIdc(vui->sarWidth, vui->sarHeight);

    if(sarIdc == 0) /* unspecified sample aspect ratio -> not present */
    {
        H264NalBits(strm, 0, 1);
        COMMENT("aspect_ratio_info_present_flag");
    }
    else
    {
        H264NalBits(strm, 1, 1);
        COMMENT("aspect_ratio_info_present_flag");
        H264NalBits(strm, sarIdc, 8);
        COMMENT("aspect_ratio_idc");
        if(sarIdc == EXTENDED_SAR)
        {
            H264NalBits(strm, vui->sarWidth, 16);
            COMMENT("sar_width");
            H264NalBits(strm, vui->sarHeight, 16);
            COMMENT("sar_height");
        }
    }

    H264NalBits(strm, 0, 1);
    COMMENT("overscan_info_present_flag");

    if(vui->videoFullRange != 0)
    {
        H264NalBits(strm, 1, 1);
        COMMENT("video_signal_type_present_flag");
        H264NalBits(strm, 5, 3);
        COMMENT("unspecified video_format");
        H264NalBits(strm, 1, 1);
        COMMENT("video_full_range_flag");
        H264NalBits(strm, 0, 1);
        COMMENT("colour_description_present_flag");
    }
    else
    {
        H264NalBits(strm, 0, 1);
        COMMENT("video_signal_type_present_flag");
    }

    H264NalBits(strm, 0, 1);
    COMMENT("chroma_loc_info_present_flag");

    if(vui->timeScale != 0)
    {
        H264NalBits(strm, 1, 1);
        COMMENT("timing_info_present_flag");
        H264NalBits(strm, vui->numUnitsInTick >> 16, 16);
        COMMENT("num_units_in_tick msb");
        H264NalBits(strm, vui->numUnitsInTick & 0xFFFF, 16);
        COMMENT("num_units_in_tick lsb");
        H264NalBits(strm, vui->timeScale >> 16, 16);
        COMMENT("time_scale msb");
        H264NalBits(strm, vui->timeScale & 0xFFFF, 16);
        COMMENT("time_scale lsb");
        H264NalBits(strm, 1, 1);
        COMMENT("fixed_frame_rate_flag");
    }
    else
    {
        H264NalBits(strm, 0, 1);
        COMMENT("timing_info_present_flag");
    }

    H264NalBits(strm, (i32) vui->nalHrdParametersPresentFlag, 1);
    COMMENT("nal_hrd_parameters_present_flag");

    if(vui->nalHrdParametersPresentFlag == ENCHW_YES)
    {
        H264ExpGolombUnsigned(strm, 0);
        COMMENT("cpb_cnt_minus1");
        
        {
            u32 bit_rate_scale = 1;
            u32 cpb_size_scale = 1;
            u32 tmp, i = 0;

            tmp = vui->cpbSize;
            while (4095 < (tmp >> (4 + i++)));
            cpb_size_scale = i;

            i = 0;
            tmp = vui->bitRate;
            while (4095 < (tmp >> (6 + i++)));
            bit_rate_scale = i;
            
            H264NalBits(strm, bit_rate_scale, 4);
            COMMENT("bit_rate_scale");

            H264NalBits(strm, cpb_size_scale, 4);
            COMMENT("cpb_size_scale");

            tmp = vui->bitRate >> (6 + bit_rate_scale);
            H264ExpGolombUnsigned(strm, tmp - 1);
            vui->bitRate = tmp << (6 + bit_rate_scale);
            COMMENT("bit_rate_value_minus1");

            tmp = vui->cpbSize >> (4 + cpb_size_scale);
            H264ExpGolombUnsigned(strm, tmp - 1);
            vui->cpbSize = tmp << (4 + cpb_size_scale);
            COMMENT("cpb_size_value_minus1");
        }

        H264NalBits(strm, 0, 1);
        COMMENT("cbr_flag");

        H264NalBits(strm, vui->initialCpbRemovalDelayLength - 1, 5);
        COMMENT("initial_cpb_removal_delay_length_minus1");
        H264NalBits(strm, vui->cpbRemovalDelayLength - 1, 5);
        COMMENT("cpb_removal_delay_length_minus1");
        H264NalBits(strm, vui->dpbOutputDelayLength - 1, 5);
        COMMENT("dpb_output_delay_length_minus1");
        H264NalBits(strm, vui->timeOffsetLength, 5);
        COMMENT("time_offset_length");
    }

    H264NalBits(strm, 0, 1);
    COMMENT("vcl_hrd_parameters_present_flag");

    if(vui->nalHrdParametersPresentFlag == ENCHW_YES)
    {
        H264NalBits(strm, 0, 1);
        COMMENT("low_delay_hrd_flag");
    }

    H264NalBits(strm, (i32) vui->pictStructPresentFlag, 1);
    COMMENT("pic_struct_present_flag");

    H264NalBits(strm, (i32) vui->bitStreamRestrictionFlag, 1);
    COMMENT("bit_stream_restriction_flag");

    if(vui->bitStreamRestrictionFlag == ENCHW_YES)
    {
        H264NalBits(strm, 1, 1);
        COMMENT("motion_vectors_over_pic_boundaries");

        H264ExpGolombUnsigned(strm, 0);
        COMMENT("max_bytes_per_pic_denom");

        H264ExpGolombUnsigned(strm, 0);
        COMMENT("max_bits_per_mb_denom");

        H264ExpGolombUnsigned(strm, LOG2_MAX_MV_LENGTH_HOR);
        COMMENT("log2_mv_length_horizontal");

        H264ExpGolombUnsigned(strm, LOG2_MAX_MV_LENGTH_VER);
        COMMENT("log2_mv_length_vertical");

        H264ExpGolombUnsigned(strm, 0);
        COMMENT("num_reorder_frames");

        H264ExpGolombUnsigned(strm, numRefFrames);
        COMMENT("max_dec_frame_buffering");
    }

}

/*------------------------------------------------------------------------------

    Function: GetAspectRatioIdc

        Functional description:

        Inputs:
          u32 sarWidth      sample aspect ratio width
          u32 sarHeight     sample aspect ratio height

        Outputs:

        Returns:
          u32   acpectRatioIdc

------------------------------------------------------------------------------*/
static i32 GetAspectRatioIdc(i32 sarWidth, i32 sarHeight)
{

    i32 aspectRatioIdc;

    if(sarWidth == 0 || sarHeight == 0) /* unspecified */
        aspectRatioIdc = 0;
    else if(sarWidth == sarHeight)  /* square, 1:1 */
        aspectRatioIdc = 1;
    else if(sarHeight == 11)
    {
        if(sarWidth == 12)  /* 12:11 */
            aspectRatioIdc = 2;
        else if(sarWidth == 10) /* 10:11 */
            aspectRatioIdc = 3;
        else if(sarWidth == 16) /* 16:11 */
            aspectRatioIdc = 4;
        else if(sarWidth == 24) /* 24:11 */
            aspectRatioIdc = 6;
        else if(sarWidth == 20) /* 20:11 */
            aspectRatioIdc = 7;
        else if(sarWidth == 32) /* 32:11 */
            aspectRatioIdc = 8;
        else if(sarWidth == 18) /* 18:11 */
            aspectRatioIdc = 10;
        else if(sarWidth == 15) /* 15:11 */
            aspectRatioIdc = 11;
        else    /* Extended_SAR */
            aspectRatioIdc = EXTENDED_SAR;
    }
    else if(sarHeight == 33)
    {
        if(sarWidth == 40)  /* 40:33 */
            aspectRatioIdc = 5;
        else if(sarWidth == 80) /* 80:33 */
            aspectRatioIdc = 9;
        else if(sarWidth == 64) /* 64:33 */
            aspectRatioIdc = 12;
        else    /* Extended_SAR */
            aspectRatioIdc = EXTENDED_SAR;
    }
    else if(sarWidth == 160 && sarHeight == 99) /* 160:99 */
        aspectRatioIdc = 13;
    else    /* Extended_SAR */
        aspectRatioIdc = EXTENDED_SAR;

    return (aspectRatioIdc);

}

/*------------------------------------------------------------------------------

    Function: H264CheckLevel

        Functional description:
          Check whether levelIdc can accommodate the stream based on bit and
          frame rates set by the application

        Inputs:
          seqParamSet_t *       pointer to SPS data structure
          u32 bitRate           bit rate in bits per second
          u32 frameRateNum      numerator of the frame rate
          u32 frameRateDenom    denominator of the frame rate

        Outputs:
          seqParamSet_t *       pointer to SPS data structure

        Returns:
          ENCHW_OK for success
          ENCHW_NOK for invalid params

------------------------------------------------------------------------------*/
bool_e H264CheckLevel(sps_s * sps, i32 bitRate, i32 frameRateNum,
                      i32 frameRateDenom)
{

/* Variables */

    i32 tmp, i;

/* Code */

    ASSERT(sps);

    if(bitRate <= 0 || frameRateNum <= 0 || frameRateDenom <= 0)
        return (ENCHW_NOK);

    i = sps->levelIdx;

    tmp = (sps->picWidthInMbsMinus1 + 1) * (sps->picHeightInMapUnitsMinus1 + 1);

    if((u32) tmp > H264MaxFS[i] ||
       (u32) sps->picWidthInMbsMinus1 >= H264SqrtMaxFS8[i] ||
       (u32) sps->picHeightInMapUnitsMinus1 >= H264SqrtMaxFS8[i])
        return (ENCHW_NOK);

    tmp = frameRateNum * tmp / frameRateDenom;

    if(H264MaxMBPS[sps->levelIdx] < (u32) tmp)
        return (ENCHW_NOK);

    return (ENCHW_OK);

}

/*------------------------------------------------------------------------------

    Function: H264GetLevelIndex

        Functional description:
            function determines index to level tables
            level argument. If invalid level -> return INVALID_LEVEL

        Inputs:
            u32 levelIdc

        Outputs:

        Returns:
            u32 index
            INVALID_LEVEL

------------------------------------------------------------------------------*/

u32 H264GetLevelIndex(u32 levelIdc)
{

    u32 i;

    i = 0;
    while(H264LevelIdc[i] != levelIdc)
    {
        i++;
        if(i >= MAX_LEVEL_INDEX)
            return INVALID_LEVEL;
    }

    return (i);

}

/*------------------------------------------------------------------------------

    Function: H264SpsSetVui

        Functional description:
          Set VUI parameters in the SPS structure

        Inputs:
          u32 timeScale
          u32 numUnitsInTick
          bool zeroReorderFrames

        Outputs:
          seqParamSet_t *       pointer to SPS structure

------------------------------------------------------------------------------*/
void H264SpsSetVuiTimigInfo(sps_s * sps, u32 timeScale, u32 numUnitsInTick)
{
    if(timeScale)
        sps->vuiParametersPresent = ENCHW_YES;

    sps->vui.timeScale = 2 * timeScale; /* used as timing_info_present_flag */
    sps->vui.numUnitsInTick = numUnitsInTick;
}

void H264SpsSetVuiVideoInfo(sps_s * sps, u32 videoFullRange)
{
    if(videoFullRange)
        sps->vuiParametersPresent = ENCHW_YES;

    sps->vui.videoFullRange = videoFullRange;   /* used as video_signal_type_present_flag */
}

void H264SpsSetVuiAspectRatio(sps_s * sps, u32 sampleAspectRatioWidth,
                              u32 sampleAspectRatioHeight)
{
    ASSERT(sampleAspectRatioWidth < (1 << 16));
    ASSERT(sampleAspectRatioHeight < (1 << 16));

    if(sampleAspectRatioWidth)
        sps->vuiParametersPresent = ENCHW_YES;

    sps->vui.sarWidth = sampleAspectRatioWidth; /* used as aspect_ratio_info_present_flag */
    sps->vui.sarHeight = sampleAspectRatioHeight;
}

/*------------------------------------------------------------------------------

    Function: H264SpsSetVuiHrd

        Functional description:
          Set VUI HRD parameters in the SPS structure

        Inputs:
          seqParamSet_t *       pointer to SPS structure

        Outputs:
          seqParamSet_t *       pointer to SPS structure

------------------------------------------------------------------------------*/

void H264SpsSetVuiHrd(sps_s * sps, u32 present)
{
    ASSERT(sps);

    sps->vui.nalHrdParametersPresentFlag = present;

    if(present)
        sps->vuiParametersPresent = ENCHW_YES;
    else
    {
        return;
    }

    ASSERT(sps->vui.timeScale && sps->vui.numUnitsInTick);  /* set these first */

    sps->vui.initialCpbRemovalDelayLength = 24;
    sps->vui.cpbRemovalDelayLength = 24;
    sps->vui.dpbOutputDelayLength = 24;

    {
        u32 n = 1;

        while(sps->vui.numUnitsInTick > (1U << n))
        {
            n++;
        }
        sps->vui.timeOffsetLength = n;

    }
}

/*------------------------------------------------------------------------------

    Function: H264SpsSetVuiHrdBitRate

        Functional description:
          Set VUI HRD bit rate in the SPS structure

        Inputs:
          seqParamSet_t *       pointer to SPS structure
          u32 bitRate

        Outputs:
          seqParamSet_t *       pointer to SPS structure

------------------------------------------------------------------------------*/

void H264SpsSetVuiHrdBitRate(sps_s * sps, u32 bitRate)
{
    ASSERT(sps);

    sps->vui.bitRate = bitRate;
}

void H264SpsSetVuiHrdCpbSize(sps_s * sps, u32 cpbSize)
{
    ASSERT(sps);

    sps->vui.cpbSize = cpbSize;
}

u32 H264SpsGetVuiHrdBitRate(sps_s * sps)
{
    ASSERT(sps);

    return sps->vui.bitRate;
}

u32 H264SpsGetVuiHrdCpbSize(sps_s * sps)
{
    ASSERT(sps);

    return sps->vui.cpbSize;
}

/*------------------------------------------------------------------------------
    Function name   : H264EndOfSequence
    Description     : 
    Return type     : void 
    Argument        : stream_s *stream
    Argument        : sps_s *sps
------------------------------------------------------------------------------*/
void H264EndOfSequence(stream_s * stream, sps_s * sps)
{
    H264NalUnitHdr(stream, 0, ENDOFSEQUENCE, sps->byteStream);
}

/*------------------------------------------------------------------------------
    Function name   : H264EndOfStream
    Description     : 
    Return type     : void 
    Argument        : stream_s *stream
    Argument        : sps_s *sps
------------------------------------------------------------------------------*/
void H264EndOfStream(stream_s * stream, sps_s * sps)
{
    H264NalUnitHdr(stream, 0, ENDOFSTREAM, sps->byteStream);
}

/*------------------------------------------------------------------------------
    Function name   : H264SpsSetVuiPictStructPresentFlag
    Description     : Signal presence of pic_struct in picture timing SEI
    Return type     : void 
    Argument        : sps_s * sps
    Argument        : u32 flag
------------------------------------------------------------------------------*/
void H264SpsSetVuiPictStructPresentFlag(sps_s * sps, u32 flag)
{
    sps->vui.pictStructPresentFlag = flag;
}

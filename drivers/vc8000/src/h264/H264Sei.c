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
 * Description : H.264 SEI Messages.
 *
 ********************************************************************************
 */

#include "H264Sei.h"
#include "H264PutBits.h"
#include "H264NalUnit.h"
#include "enccommon.h"

#define SEI_BUFFERING_PERIOD        0
#define SEI_PIC_TIMING              1
#define SEI_FILLER_PAYLOAD          3
#define SEI_USER_DATA_UNREGISTERED  5
#define SEI_RECOVERY_POINT_PAYLOAD  6
#define SEI_SCALABILITY_INFO        24
/*------------------------------------------------------------------------------
  H264InitSei()
------------------------------------------------------------------------------*/
void H264InitSei(sei_s * sei, true_e byteStream, u32 hrd, u32 timeScale,
                 u32 nuit)
{
    ASSERT(sei != NULL);

    sei->byteStream = byteStream;
    sei->hrd = hrd;
    sei->seqId = 0x0;
    sei->psp = (u32) ENCHW_YES;
    sei->cts = (u32) ENCHW_YES;
    /* sei->icrd = 0; */
    sei->icrdLen = 24;
    /* sei->icrdo = 0; */
    sei->icrdoLen = 24;
    /* sei->crd = 0; */
    sei->crdLen = 24;
    /* sei->dod = 0; */
    sei->dodLen = 24;
    sei->ps = 0;
    sei->cntType = 1;
    sei->cdf = 0;
    sei->nframes = 0;
    sei->toffs = 0;

    {
        u32 n = 1;

        while(nuit > (1U << n))
            n++;
        sei->toffsLen = n;
    }

    sei->ts.timeScale = timeScale;
    sei->ts.nuit = nuit;
    sei->ts.time = 0;
    sei->ts.sec = 0;
    sei->ts.min = 0;
    sei->ts.hr = 0;
    sei->ts.fts = (u32) ENCHW_YES;
    sei->ts.secf = (u32) ENCHW_NO;
    sei->ts.minf = (u32) ENCHW_NO;
    sei->ts.hrf = (u32) ENCHW_NO;
    
    sei->userDataEnabled = (u32) ENCHW_NO;
    sei->userDataSize = 0;
    sei->pUserData = NULL;
}

/*------------------------------------------------------------------------------
  H264UpdateSeiTS()  Calculate new time stamp.
------------------------------------------------------------------------------*/
void H264UpdateSeiTS(sei_s * sei, u32 timeInc)
{
    timeStamp_s *ts = &sei->ts;

    ASSERT(sei != NULL);
    timeInc += ts->time;

    while(timeInc >= ts->timeScale)
    {
        timeInc -= ts->timeScale;
        ts->sec++;
        if(ts->sec == 60)
        {
            ts->sec = 0;
            ts->min++;
            if(ts->min == 60)
            {
                ts->min = 0;
                ts->hr++;
                if(ts->hr == 32)
                {
                    ts->hr = 0;
                }
            }
        }
    }

    ts->time = timeInc;

    sei->nframes = ts->time / ts->nuit;
    sei->toffs = ts->time - sei->nframes * ts->nuit;

    ts->hrf = (ts->hr != 0);
    ts->minf = ts->hrf || (ts->min != 0);
    ts->secf = ts->minf || (ts->sec != 0);

#ifdef TRACE_PIC_TIMING
    DEBUG_PRINT(("Picture Timing: %02i:%02i:%02i  %6i ticks\n", ts->hr, ts->min,
                 ts->sec, (sei->nframes * ts->nuit + sei->toffs)));
#endif
}

/*------------------------------------------------------------------------------
  H264FillerSei()  Filler payload SEI message. Requested filler payload size
  could be huge. Use of temporary stream buffer is not needed, because size is
  know.
------------------------------------------------------------------------------*/
void H264FillerSei(stream_s * sp, sei_s * sei, i32 cnt)
{
    i32 i = cnt;

    ASSERT(sp != NULL && sei != NULL);

    H264NalUnitHdr(sp, 0, SEI, sei->byteStream);

    H264NalBits(sp, SEI_FILLER_PAYLOAD, 8);
    COMMENT("last_payload_type_byte");

    while(cnt >= 255)
    {
        H264NalBits(sp, 0xFF, 0x8);
        COMMENT("ff_byte");
        cnt -= 255;
    }
    H264NalBits(sp, cnt, 8);
    COMMENT("last_payload_size_byte");

    for(; i > 0; i--)
    {
        H264NalBits(sp, 0xFF, 8);
        COMMENT("filler ff_byte");
    }
    H264RbspTrailingBits(sp);
}

/*------------------------------------------------------------------------------
  H264BufferingSei()  Buffering period SEI message.
------------------------------------------------------------------------------*/
void H264BufferingSei(stream_s * sp, sei_s * sei)
{
    u8 *pPayloadSizePos;
    u32 startByteCnt;

    ASSERT(sei != NULL);

    if(sei->hrd == ENCHW_NO)
    {
        return;
    }

    H264NalBits(sp, SEI_BUFFERING_PERIOD, 8);
    COMMENT("last_payload_type_byte");

    pPayloadSizePos = sp->stream;

    H264NalBits(sp, 0xFF, 8);   /* this will be updated after we know exact payload size */
    COMMENT("last_payload_size_byte");

    startByteCnt = sp->byteCnt;
    sp->emulCnt = 0;    /* count emul_3_byte for this payload */

    H264ExpGolombUnsigned(sp, sei->seqId);
    COMMENT("seq_parameter_set_id");

    H264NalBits(sp, sei->icrd, sei->icrdLen);
    COMMENT("initial_cpb_removal_delay");

    H264NalBits(sp, sei->icrdo, sei->icrdoLen);
    COMMENT("initial_cpb_removal_delay_offset");

    if(sp->bufferedBits)
    {
        H264RbspTrailingBits(sp);
    }

    {
        u32 payload_size;

        payload_size = sp->byteCnt - startByteCnt - sp->emulCnt;
        *pPayloadSizePos = payload_size;
    }

    /* reset cpb_removal_delay */
    sei->crd = 0;
}

/*------------------------------------------------------------------------------
  TimeStamp()
------------------------------------------------------------------------------*/
static void TimeStamp(stream_s * sp, timeStamp_s * ts)
{
    if(ts->fts)
    {
        H264NalBits(sp, ts->sec, 6);
        COMMENT("seconds_value");
        H264NalBits(sp, ts->min, 6);
        COMMENT("minutes_value");
        H264NalBits(sp, ts->hr, 5);
        COMMENT("hours_value");
    }
    else
    {
        H264NalBits(sp, ts->secf, 1);
        COMMENT("seconds_flag");
        if(ts->secf)
        {
            H264NalBits(sp, ts->sec, 6);
            COMMENT("seconds_value");
            H264NalBits(sp, ts->minf, 1);
            COMMENT("minutes_flag");
            if(ts->minf)
            {
                H264NalBits(sp, ts->min, 6);
                COMMENT("minutes_value");
                H264NalBits(sp, ts->hrf, 1);
                COMMENT("hours_flag");
                if(ts->hrf)
                {
                    H264NalBits(sp, ts->hr, 5);
                    COMMENT("hours_value");
                }
            }
        }
    }
}

/*------------------------------------------------------------------------------
  H264PicTimingSei()  Picture timing SEI message.
------------------------------------------------------------------------------*/
void H264PicTimingSei(stream_s * sp, sei_s * sei)
{
    u8 *pPayloadSizePos;
    u32 startByteCnt;

    ASSERT(sei != NULL);

    H264NalBits(sp, SEI_PIC_TIMING, 8);
    COMMENT("last_payload_type_byte");

    pPayloadSizePos = sp->stream;

    H264NalBits(sp, 0xFF, 8);   /* this will be updated after we know exact payload size */
    COMMENT("last_payload_size_byte");

    startByteCnt = sp->byteCnt;
    sp->emulCnt = 0;    /* count emul_3_byte for this payload */

    if(sei->hrd)
    {
        H264NalBits(sp, sei->crd, sei->crdLen);
        COMMENT("cpb_removal_delay");
        H264NalBits(sp, sei->dod, sei->dodLen);
        COMMENT("dpb_output_delay");
    }

    if(sei->psp)
    {
        H264NalBits(sp, sei->ps, 4);
        COMMENT("pic_struct");
        H264NalBits(sp, sei->cts, 1);
        COMMENT("clock_timestamp_flag");
        if(sei->cts)
        {
            H264NalBits(sp, 0, 2);
            COMMENT("ct_type");
            H264NalBits(sp, 0, 1);
            COMMENT("nuit_field_based_flag");
            H264NalBits(sp, sei->cntType, 5);
            COMMENT("counting_type");
            H264NalBits(sp, sei->ts.fts, 1);
            COMMENT("full_timestamp_flag");
            H264NalBits(sp, 0, 1);
            COMMENT("discontinuity_flag");
            H264NalBits(sp, sei->cdf, 1);
            COMMENT("cnt_dropped_flag");
            H264NalBits(sp, sei->nframes, 8);
            COMMENT("n_frames");
            TimeStamp(sp, &sei->ts);
            if(sei->toffsLen > 0)
            {
                H264NalBits(sp, sei->toffs, sei->toffsLen);
                COMMENT("time_offset");
            }
        }
    }

    if(sp->bufferedBits)
    {
        H264RbspTrailingBits(sp);
    }

    {
        u32 payload_size;

        payload_size = sp->byteCnt - startByteCnt - sp->emulCnt;
        *pPayloadSizePos = payload_size;
    }
}

/*------------------------------------------------------------------------------
  H264UserDataUnregSei()  User data unregistered SEI message.
------------------------------------------------------------------------------*/
void H264UserDataUnregSei(stream_s * sp, sei_s * sei)
{
    u32 i, cnt;
    const u8 * pUserData;
    ASSERT(sei != NULL);
    ASSERT(sei->pUserData != NULL);
    ASSERT(sei->userDataSize >= 16);

    pUserData = sei->pUserData;
    cnt = sei->userDataSize;
    if(sei->userDataEnabled == ENCHW_NO)
    {
        return;
    }

    H264NalBits(sp, SEI_USER_DATA_UNREGISTERED, 8);
    COMMENT("last_payload_type_byte");

    while(cnt >= 255)
    {
        H264NalBits(sp, 0xFF, 0x8);
        COMMENT("ff_byte");
        cnt -= 255;
    }

    H264NalBits(sp, cnt, 8);
    COMMENT("last_payload_size_byte");

    /* Write uuid */
    for (i = 0; i < 16; i++)
    {
    	H264NalBits(sp, pUserData[i], 8);
        COMMENT("uuid_iso_iec_11578_byte");
    }

    /* Write payload */
    for (i = 16; i < sei->userDataSize; i++)
    {
        H264NalBits(sp, pUserData[i], 8);
        COMMENT("user_data_payload_byte");
    }
}
/*------------------------------------------------------------------------------
  HevcRecoveryPointSei()  recovery point SEI message.
------------------------------------------------------------------------------*/
void H264RecoveryPointSei(stream_s * sp, sei_s * sei)
{
    u8 *pPayloadSizePos;
    u32 startByteCnt;

    ASSERT(sei != NULL);

    H264NalBits(sp, SEI_RECOVERY_POINT_PAYLOAD, 8);
    COMMENT("last_payload_type_byte");

    pPayloadSizePos = sp->stream;

    H264NalBits(sp, 0xFF, 8);   /* this will be updated after we know exact payload size */
    COMMENT("last_payload_size_byte");

    startByteCnt = sp->byteCnt;
    sp->emulCnt = 0;    /* count emul_3_byte for this payload */

    H264ExpGolombUnsigned(sp, sei->recoveryFrameCnt-1);
    COMMENT("recovery_frame_cnt");

    H264NalBits(sp, 1, 1);
    COMMENT("exact_match_flag");

    H264NalBits(sp, 0, 1);
    COMMENT("broken_link_flag");

    H264NalBits(sp, 0, 2);
    COMMENT("changing_slice_group_idc");

    if(sp->bufferedBits)
    {
        H264RbspTrailingBits(sp);
    }

    {
        u32 payload_size;

        payload_size = sp->byteCnt - startByteCnt - sp->emulCnt;
        *pPayloadSizePos = payload_size;
    }

}


/*------------------------------------------------------------------------------
  H264ScalabilityInfoSei()  Scalability information SEI message.
------------------------------------------------------------------------------*/
void H264ScalabilityInfoSei(stream_s * sp, i32 svctLevel, i32 frameRate)
{
    u8 *pPayloadSizePos;
    u32 startByteCnt;
    i32 i;
    const bool priority_id_setting_flag=true;
    const bool sub_pic_layer_flag=false;
    const bool iroi_division_info_present_flag=false;
    const bool bitrate_info_present_flag=false;
    const bool frm_rate_info_present_flag=true;
    const bool frm_size_info_present_flag=false;
    const bool layer_dependency_info_present_flag=true;
    bool sub_region_layer_flag;
    bool profile_level_info_present_flag;
    bool parameter_sets_info_present_flag;
    
    H264NalBits(sp, SEI_SCALABILITY_INFO, 8);
    COMMENT("last_payload_type_byte");

    pPayloadSizePos = sp->stream;

    H264NalBits(sp, 0xFF, 8);   /* this will be updated after we know exact payload size */
    COMMENT("last_payload_size_byte");

    startByteCnt = sp->byteCnt;
    sp->emulCnt = 0;    /* count emul_3_byte for this payload */

    H264NalBits(sp, 0, 1);
    COMMENT("temporal_id_nesting_flag");
    H264NalBits(sp, 0, 1);
    COMMENT("priority_layer_info_present_flag");
    H264NalBits(sp, priority_id_setting_flag , 1);
    COMMENT("priority_id_setting_flag");
    H264ExpGolombUnsigned(sp, svctLevel);
    COMMENT("num_layers_minus1");

    for(i=0; i<svctLevel+1; i++)
    {
        H264ExpGolombUnsigned(sp, i);
        COMMENT("layer_id[i]");
        H264NalBits(sp, i, 6); /* 0/1 */
        COMMENT("priority_id[i]");
        H264NalBits(sp, 1, 1);
        COMMENT("discardable_flag[i]");
        H264NalBits(sp, 0, 3);
        COMMENT("dependency_id[i]");
        H264NalBits(sp, 0, 4);
        COMMENT("quality_id[i]");
        H264NalBits(sp, i, 3);
        COMMENT("temporal_id[i]");
        H264NalBits(sp, sub_pic_layer_flag, 1);
        COMMENT("sub_pic_layer_flag[i]");
        sub_region_layer_flag = 0; /*(i==0)?true:false; */
        H264NalBits(sp, sub_region_layer_flag, 1);
        COMMENT("sub_region_layer_flag[i]");
        H264NalBits(sp, iroi_division_info_present_flag, 1);
        COMMENT("iroi_division_info_present_flag[i]");
        profile_level_info_present_flag = 0; /* (i==0)?true:false; */
        H264NalBits(sp, profile_level_info_present_flag, 1);
        COMMENT("profile_level_info_present_flag[i]");
        H264NalBits(sp, bitrate_info_present_flag, 1);  
        COMMENT("bitrate_info_present_flag[i]");
        H264NalBits(sp, frm_rate_info_present_flag, 1); 
        COMMENT("frm_rate_info_present_flag[i]");
        H264NalBits(sp, frm_size_info_present_flag, 1); 
        COMMENT("frm_size_info_present_flag[i]");
        H264NalBits(sp, layer_dependency_info_present_flag, 1); /* true */
        COMMENT("layer_dependency_info_present_flag[i]");
        parameter_sets_info_present_flag = (i==0)?true:false;
        H264NalBits(sp, parameter_sets_info_present_flag, 1);
        COMMENT("parameter_sets_info_present_flag[i]");
        H264NalBits(sp, 0, 1);
        COMMENT("bitstream_restriction_info_present_flag[i]");
        H264NalBits(sp, 1, 1);
        COMMENT("exact_inter_layer_pred_flag[i]");
        if ( sub_pic_layer_flag || iroi_division_info_present_flag )
        {
            H264NalBits(sp, 0, 1);
            COMMENT("exact_sample_value_match_flag[i]");
        }
        H264NalBits(sp, 0, 1);
        COMMENT("layer_conversion_flag[i]");
        H264NalBits(sp, 1, 1); /* output */
        COMMENT("layer_output_flag[i]");
        if (profile_level_info_present_flag)
        {
            H264NalBits(sp, 0, 8);
            H264NalBits(sp, 0, 8);
            H264NalBits(sp, 0, 8);
            COMMENT("layer_profile_level_idc[i]");
        }
        if (bitrate_info_present_flag) {
#if 0  /*       */
            avg_bitrate[ i ] 5 u(16)
            max_bitrate_layer[ i ] 5 u(16)
            max_bitrate_layer_representation[ i ] 5 u(16)
            max_bitrate_calc_window[ i ] 5 u(16)
#endif /*       */
        }
        if(frm_rate_info_present_flag) 
        {
            u32 frame_rate;
            H264NalBits(sp, 0, 2); /* constant frame rate */
            COMMENT("constant_frm_rate_idc[i]");
            frame_rate = frameRate>>(svctLevel-i);
            H264NalBits(sp, frame_rate, 16); /* constant frame rate */
            COMMENT("avg_frm_rate[i]");
        }
        if( frm_size_info_present_flag || iroi_division_info_present_flag ) 
        {
            /* frm_width_in_mbs_minus1[ i ] 5 ue(v) */
            /* frm_height_in_mbs_minus1[ i ] 5 ue(v) */
        }
#if 0
        if( sub_region_layer_flag[ i ] ) {
            base_region_layer_id[ i ] 5 ue(v)
            dynamic_rect_flag[ i ] 5 u(1)
            if( !dynamic_rect_flag[ i ] ) {
                horizontal_offset[ i ] 5 u(16)
                vertical_offset[ i ] 5 u(16)
                region_width[ i ] 5 u(16)
                region_height[ i ] 5 u(16)
            }
        }
        if( sub_pic_layer_flag )
        {
            roi_id[ i ] 5 ue(v)
        }
        if ( iroi_division_info_present_flag) 
        {
            iroi_grid_flag[ i ] 5 u(1)
            if ( iroi_grid_flag[ i ] ) {
              grid_width_in_mbs_minus1[ i ] 5 ue(v)
              grid_height_in_mbs_minus1[ i ] 5 ue(v)
            } else {
              num_rois_minus1[ i ] 5 ue(v)
                for (j = 0; j <= num_rois_minus1[ i ]; j++ ) {
                    first_mb_in_roi[ i ][ j ] 5 ue(v)
                    roi_width_in_mbs_minus1[ i ][ j ] 5 ue(v)
                    roi_height_in_mbs_minus1[ i ][ j ] 5 ue(v)
                }
            }
        }
#endif        
        if( layer_dependency_info_present_flag) 
        {
            u32 num_directly_dependent_layers;
            u32 j;
            num_directly_dependent_layers = (i==0)?0:1;
            H264ExpGolombUnsigned(sp, num_directly_dependent_layers);
            COMMENT("num_directly_dependent_layers[i]");
            for( j = 0; j < num_directly_dependent_layers; j++ )
            {
                H264ExpGolombUnsigned(sp, 0);
                COMMENT("directly_dependent_layer_id_delta_minus1[i]");
            }
        } 
        else
        {
            H264ExpGolombUnsigned(sp, 0);
            COMMENT("layer_dependency_info_src_layer_id_delta[i]");
        }
        if( parameter_sets_info_present_flag) 
        {
            u32 j;
            u32 num_seq_parameter_set_minus1 = 0;
            u32 num_subset_seq_parameter_set_minus1 = 0;
            u32 num_pic_parameter_set_minus1 = 0;
            H264ExpGolombUnsigned(sp, num_seq_parameter_set_minus1);
            COMMENT("num_seq_parameter_set_minus1[i]");
            for( j = 0; j <= num_seq_parameter_set_minus1; j++ )
            {
                H264ExpGolombUnsigned(sp, 0);
                COMMENT("seq_parameter_set_id_delta[i]");
            }
            H264ExpGolombUnsigned(sp, num_subset_seq_parameter_set_minus1);
            COMMENT("num_subset_seq_parameter_set_minus1[i]");
            for( j = 0; j <= num_subset_seq_parameter_set_minus1; j++ )
            {
                H264ExpGolombUnsigned(sp, 0);
                COMMENT("subset_seq_parameter_set_id_delta[i]");
            }
            H264ExpGolombUnsigned(sp, num_pic_parameter_set_minus1);
            COMMENT("num_pic_parameter_set_minus1[i]");
            for( j = 0; j <= num_pic_parameter_set_minus1; j++ )
            {
                H264ExpGolombUnsigned(sp, 0);
                COMMENT("pic_parameter_set_id_delta[i]");
            }
        } 
        else
        {
            H264ExpGolombUnsigned(sp, 0);
            COMMENT("parameter_sets_info_src_layer_id_delta[i]");
        }
#if 0
        if( bitstream_restriction_info_present_flag[ i ] ) {
            motion_vectors_over_pic_boundaries_flag[ i ] 5 u(1)
            max_bytes_per_pic_denom[ i ] 5 ue(v)
            max_bits_per_mb_denom[ i ] 5 ue(v)
            log2_max_mv_length_horizontal[ i ] 5 ue(v)
            log2_max_mv_length_vertical[ i ] 5 ue(v)
            num_reorder_frames[ i ] 5 ue(v)
            max_dec_frame_buffering[ i ] 5 ue(v)
        }
        if( layer_conversion_flag[ i ] ) {
            conversion_type_idc[ i ] 5 ue(v)
            for( j=0; j < 2; j++ ) {
                rewriting_info_flag[ i ][ j ] 5 u(1)
                if( rewriting_info_flag[ i ][ j ] ) {
                    rewriting_profile_level_idc[ i ][ j ] 5 u(24)
                    rewriting_avg_bitrate[ i ][ j ] 5 u(16)
                    rewriting_max_bitrate[ i ][ j ] 5 u(16)
                }
            }
        }
#endif
    }

#if 0    
    if( priority_layer_info_present_flag ) {
        pr_num_dId_minus1 5 ue(v)
        for( i = 0; i <= pr_num_dId_minus1; i++ ) {
            pr_dependency_id[ i ] 5 u(3)
            pr_num_minus1[ i ] 5 ue(v)
            for( j = 0; j <= pr_num_minus1[ i ]; j++ ) {
                pr_id[ i ][ j ] 5 ue(v)
                pr_profile_level_idc[ i ][ j ] 5 u(24)
                pr_avg_bitrate[ i ][ j ] 5 u(16)
                pr_max_bitrate[ i ][ j ] 5 u(16)
            }
        }
    }
#endif
    
    if( priority_id_setting_flag ) {
        int PriorityIdSettingUriIdx = 0;
        char priority_id_setting_uri[20] = "http://svc.com";
        do {
            H264NalBits(sp, priority_id_setting_uri[ PriorityIdSettingUriIdx ], 8);
            COMMENT("priority_id_setting_uri[i]");
        } while( priority_id_setting_uri[ PriorityIdSettingUriIdx++ ] != 0 );
    }

    if(sp->bufferedBits)
    {
        H264RbspTrailingBits(sp);
    }

    {
        u32 payload_size;

        payload_size = sp->byteCnt - startByteCnt - sp->emulCnt;
        *pPayloadSizePos = payload_size;
    }
}


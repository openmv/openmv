/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     dma_op.c
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     29-Sep-2023
 * @brief    DMA Driver to generate the microcode
 * @bug      None
 * @Note     None
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <dma_op.h>
#include <stdbool.h>
#include <string.h>

/**
  \fn          bool dma_generate_opcode(dma_config_info_t *dma_cfg,
                                        uint8_t            channel_num)
  \brief       Prepare the DMA opcode for the channel
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \return      bool false if the buffer is not enough, true otherwise
*/
bool dma_generate_opcode(dma_config_info_t *dma_cfg, uint8_t channel_num)
{
    dma_thread_info_t  *thread_info   = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info  = &thread_info->channel_info;
    dma_desc_info_t    *desc          = &channel_info->desc_info;
    dma_ccr_t           dma_ccr;
    dma_loop_t          lp_args;
    dma_opcode_buf      op_buf;
    uint32_t            total_bytes, req_burst, rem_blen;
    uint32_t            burst, rem_bytes;
    uint16_t            lp_start_lc1, lp_start_lc0;
    uint16_t            lc0, lc1;
    DMA_XFER            xfer_type;
    bool                ret;

    op_buf.buf      = &thread_info->dma_mcode[0];
    op_buf.buf_size = DMA_MICROCODE_SIZE;
    op_buf.off      = 0;


    dma_ccr = dma_get_channel_ctrl_info(dma_cfg, channel_num);

    ret = dma_construct_move(dma_ccr.value, DMA_REG_CCR, &op_buf);
    if(!ret)
        return ret;

    ret = dma_construct_move(desc->src_addr, DMA_REG_SAR, &op_buf);
    if(!ret)
        return ret;

    ret = dma_construct_move(desc->dst_addr0, DMA_REG_DAR, &op_buf);
    if(!ret)
        return ret;

    // For double buffering.
    uint32_t op_buf_start = op_buf.off;

    burst       = (1 << desc->dst_bsize) * desc->dst_blen;
    total_bytes = desc->total_len;
    req_burst   = total_bytes / burst;
    rem_bytes   = total_bytes - (req_burst * burst);
    rem_blen    = rem_bytes / (1 << desc->dst_bsize);

    while(req_burst)
    {
        if(req_burst >= (DMA_MAX_LP_CNT * DMA_MAX_LP_CNT))
        {
            lc0 = DMA_MAX_LP_CNT;
            lc1 = DMA_MAX_LP_CNT;
            req_burst = req_burst - (DMA_MAX_LP_CNT * DMA_MAX_LP_CNT);
        }
        else if(req_burst >= DMA_MAX_LP_CNT)
        {
            lc0 = DMA_MAX_LP_CNT;
            lc1 = (uint16_t)(req_burst / lc0);
            req_burst = req_burst - (lc0 * lc1) ;
        }
        else
        {
            lc0 = (uint16_t)req_burst;
            lc1 = 0;
            req_burst = 0;
        }

        lp_start_lc1 = 0;
        if(lc1)
        {
            ret = dma_construct_loop(DMA_LC_1, (uint8_t)lc1, &op_buf);
            if(!ret)
                return ret;
            lp_start_lc1 = op_buf.off;
        }

        if(lc0 == 0)
            return ret;

        ret = dma_construct_loop(DMA_LC_0, (uint8_t)lc0, &op_buf);
        if(!ret)
            return ret;

        lp_start_lc0 = op_buf.off;

        if(desc->dst_blen == 1)
            xfer_type = DMA_XFER_SINGLE;
        else
            xfer_type = DMA_XFER_BURST;

        if(desc->direction != DMA_TRANSFER_MEM_TO_MEM)
        {
            if(!(channel_info->flags & DMA_CHANNEL_FLAG_CRC_MODE))
            {
                ret = dma_construct_flushperiph(desc->periph_num, &op_buf);
                if (!ret)
                    return ret;

                ret = dma_construct_wfp(xfer_type, desc->periph_num, &op_buf);
                if (!ret)
                    return ret;
            }

            if(desc->direction ==  DMA_TRANSFER_MEM_TO_DEV)
            {
                ret = dma_construct_load(xfer_type, &op_buf);
                if(!ret)
                    return ret;

                if(channel_info->flags & DMA_CHANNEL_FLAG_CRC_MODE)
                {
                    ret = dma_construct_store(xfer_type, &op_buf);
                    if (!ret)
                        return ret;
                }
                else
                {
                    ret = dma_construct_storeperiph(xfer_type,
                                                    desc->periph_num,
                                                    &op_buf);
                    if(!ret)
                        return ret;
                }

                /* If I2S mono mode is enabled for this channel, write zeros */
                if(channel_info->flags & DMA_CHANNEL_FLAG_I2S_MONO_MODE)
                {
                    ret = dma_construct_store_zeros(&op_buf);
                    if(!ret)
                        return ret;
                }
            }
            else /* ARM_DMA_DEV_TO_MEM */
            {
                ret = dma_construct_loadperiph(xfer_type,
                                               desc->periph_num,
                                               &op_buf);
                if(!ret)
                    return ret;

                ret = dma_construct_store(xfer_type, &op_buf);
                if(!ret)
                    return ret;

                /* If I2S mono mode is enabled, read right channel and discard it */
                if(channel_info->flags & DMA_CHANNEL_FLAG_I2S_MONO_MODE)
                {
                    ret = dma_construct_loadperiph(xfer_type,
                                                   desc->periph_num,
                                                   &op_buf);
                    if(!ret)
                        return ret;
                    ret = dma_construct_store(xfer_type, &op_buf);
                    if(!ret)
                        return ret;
                    ret = dma_construct_addneg(DMA_REG_DAR,
                                               (int16_t)(1 << desc->dst_bsize),
                                               &op_buf);
                    if(!ret)
                        return ret;
                }
            }
        }
        else /* ARM_DMA_MEM_TO_MEM */
        {
            ret = dma_construct_load(DMA_XFER_FORCE, &op_buf);
            if(!ret)
                return ret;
            ret = dma_construct_store(DMA_XFER_FORCE, &op_buf);
            if(!ret)
                return ret;
        }

        if((op_buf.off - lp_start_lc0) > DMA_MAX_BACKWARD_JUMP)
            return false;
        lp_args.jump = (uint8_t)(op_buf.off - lp_start_lc0);
        lp_args.lc = DMA_LC_0;
        lp_args.nf = 1;
        lp_args.xfer_type = DMA_XFER_FORCE;
        ret = dma_construct_loopend(&lp_args, &op_buf);
        if(!ret)
            return ret;

        if(lc1)
        {
            if((op_buf.off - lp_start_lc1) > DMA_MAX_BACKWARD_JUMP)
                return false;
            lp_args.jump = (uint8_t)(op_buf.off - lp_start_lc1);
            lp_args.lc = DMA_LC_1;
            lp_args.nf = 1;
            lp_args.xfer_type = DMA_XFER_FORCE;
            ret = dma_construct_loopend(&lp_args, &op_buf);
            if(!ret)
                return ret;
        }
    }

    if(rem_blen)
    {

        dma_ccr.value_b.dst_burst_len = rem_blen - 1;
        dma_ccr.value_b.src_burst_len = rem_blen - 1;

        ret = dma_construct_move(dma_ccr.value, DMA_REG_CCR, &op_buf);
        if(!ret)
            return ret;

        if(desc->direction != DMA_TRANSFER_MEM_TO_MEM)
        {
            if(!(channel_info->flags & DMA_CHANNEL_FLAG_CRC_MODE))
            {
                ret = dma_construct_flushperiph(desc->periph_num, &op_buf);
                if(!ret)
                    return ret;

                ret = dma_construct_wfp(DMA_XFER_BURST,
                                        desc->periph_num,
                                        &op_buf);
                if(!ret)
                    return ret;
            }

            if(desc->direction ==  DMA_TRANSFER_MEM_TO_DEV)
            {
                ret = dma_construct_load(DMA_XFER_BURST, &op_buf);
                if(!ret)
                    return ret;

                if(channel_info->flags & DMA_CHANNEL_FLAG_CRC_MODE) {
                    ret = dma_construct_store(DMA_XFER_BURST, &op_buf);
                    if(!ret)
                        return ret;
                }
                else
                {
                    ret = dma_construct_storeperiph(DMA_XFER_BURST,
                                                    desc->periph_num,
                                                    &op_buf);
                    if(!ret)
                        return ret;
                }

                /* If I2S mono mode is enabled for this channel, write zeros */
                if(channel_info->flags & DMA_CHANNEL_FLAG_I2S_MONO_MODE)
                {
                    ret = dma_construct_store_zeros(&op_buf);
                    if(!ret)
                        return ret;
                }
            }
            else /* ARM_DMA_DEV_TO_MEM */
            {
                ret = dma_construct_loadperiph(DMA_XFER_BURST,
                                               desc->periph_num,
                                               &op_buf);
                if(!ret)
                    return ret;

                ret = dma_construct_store(DMA_XFER_BURST, &op_buf);
                if(!ret)
                    return ret;

                /* If I2S mono mode is enabled, discard right channel data */
                if(channel_info->flags & DMA_CHANNEL_FLAG_I2S_MONO_MODE)
                {
                    ret = dma_construct_loadperiph(DMA_XFER_BURST,
                                                   desc->periph_num,
                                                   &op_buf);
                    if(!ret)
                        return ret;
                    ret = dma_construct_store(DMA_XFER_BURST, &op_buf);
                    if(!ret)
                        return ret;
                    ret = dma_construct_addneg(DMA_REG_DAR,
                                               (int16_t)(1 << desc->dst_bsize),
                                               &op_buf);
                    if(!ret)
                        return ret;
                }
            }
        }
        else /* ARM_DMA_MEM_TO_MEM */
        {
            ret = dma_construct_load(DMA_XFER_FORCE, &op_buf);
            if(!ret)
                return ret;
            ret = dma_construct_store(DMA_XFER_FORCE, &op_buf);
            if(!ret)
                return ret;
        }
    }

    ret = dma_construct_wmb(&op_buf);
    if(!ret)
        return ret;

    if(desc->dst_addr1 == 0) {
        ret = dma_construct_send_event(channel_info->event_index, &op_buf);
        if(!ret)
            return ret;
        ret = dma_construct_end(&op_buf);
    } else {
        // The size of the transfer loops microcode.
        uint32_t op_buf_size = op_buf.off - op_buf_start;

        // Move ADDR1 to DAR
        if (!dma_construct_move(desc->dst_addr1, DMA_REG_DAR, &op_buf)) {
            return false;
        }

        // Notify that DAR was switched.
        if (!dma_construct_send_event(channel_info->event_index, &op_buf)) {
            return false;
        }

        // Make sure there's enough space in the buffer.
        if ((op_buf.off + op_buf_size) > DMA_MICROCODE_SIZE) {
            return false;
        }

        // Copy the transfer microcode
        memcpy(&op_buf.buf[op_buf.off], &op_buf.buf[op_buf_start], op_buf_size);
        op_buf.off += op_buf_size;

        // Move ADDR0 to DAR
        if (!dma_construct_move(desc->dst_addr0, DMA_REG_DAR, &op_buf)) {
            return false;
        }

        // Notify that DAR was switched.
        if (!dma_construct_send_event(channel_info->event_index, &op_buf)) {
            return false;
        }

        // Loop back to the start which will set ADDR0.
        dma_loop_t loop_config = { .nf = 0, .jump = op_buf.off - op_buf_start };
        ret = dma_construct_loopend(&loop_config, &op_buf);
    }

    if(!ret)
        return ret;

    return true;
}

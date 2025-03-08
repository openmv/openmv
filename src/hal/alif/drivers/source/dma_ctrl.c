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
 * @file     dma_ctrl.c
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     28-Sep-2023
 * @brief    DMA control driver
 * @bug      None
 * @Note     None
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <dma_ctrl.h>

/**
  \fn          int8_t dma_allocate_channel(dma_config_info_t *dma_cfg)
  \brief       Allocate a channel if available
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \return      int8_t channel number or -1 if not available
*/
int8_t dma_allocate_channel(dma_config_info_t *dma_cfg)
{
    dma_thread_info_t  *channel_thread = &dma_cfg->channel_thread[0];
    dma_channel_info_t *channel_info;
    uint8_t             channel_num;

    for(channel_num = 0; channel_num < DMA_MAX_CHANNELS; channel_num++)
    {
        if(channel_thread[channel_num].in_use == false)
        {
            channel_thread[channel_num].in_use = true;
            channel_thread[channel_num].user_mcode = (void *)0;

            channel_info  = &channel_thread[channel_num].channel_info;
            channel_info->flags = 0;

            return (int8_t)channel_num;
        }
    }

    return -1;
}

/**
  \fn          int8_t dma_allocate_event(dma_config_info_t *dma_cfg,
                                         uint8_t            channel_num)
  \brief       Allocate a event if available
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \return      int8_t Event index number or -1 if not available
*/
int8_t dma_allocate_event(dma_config_info_t *dma_cfg, uint8_t channel_num)
{
    dma_thread_info_t  *thread_info   = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info  = &thread_info->channel_info;
    uint8_t             event_index;

    for(event_index = 0; event_index < DMA_MAX_EVENTS; event_index++)
    {
        if(dma_cfg->event_map[event_index] == 0xFF)
        {
            dma_cfg->event_map[event_index] = channel_num;
            channel_info->event_index       = event_index;
            return (int8_t)event_index;
        }
    }

    return -1;
}

/**
  \fn          void dma_copy_desc_info(dma_config_info_t *dma_cfg,
                                       uint8_t            channel_num,
                                       dma_desc_info_t   *desc_info)
  \brief       Update the channel descriptor Structure
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \param[in]   desc_info  Descriptor Information
  \return      None
*/
void dma_copy_desc_info(dma_config_info_t *dma_cfg,
                        uint8_t            channel_num,
                        dma_desc_info_t   *desc_info)
{
    dma_thread_info_t  *thread_info       = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info      = &thread_info->channel_info;
    dma_desc_info_t    *channel_desc_info = &channel_info->desc_info;

    channel_desc_info->direction   = desc_info->direction;
    channel_desc_info->total_len   = desc_info->total_len;
    channel_desc_info->src_addr    = desc_info->src_addr;
    channel_desc_info->dst_addr0   = desc_info->dst_addr0;
    channel_desc_info->src_bsize   = desc_info->src_bsize;
    channel_desc_info->dst_bsize   = desc_info->dst_bsize;
    channel_desc_info->src_blen    = desc_info->src_blen;
    channel_desc_info->dst_blen    = desc_info->dst_blen;
    channel_desc_info->periph_num  = desc_info->periph_num;

    if(channel_desc_info->direction == DMA_TRANSFER_MEM_TO_DEV)
    {
        channel_desc_info->dst_cache_ctrl = 0x0;
    }
    else if(channel_desc_info->direction == DMA_TRANSFER_DEV_TO_MEM)
    {
        channel_desc_info->src_cache_ctrl = 0x0;
    }
}

/**
  \fn          dma_ccr_t dma_get_channel_ctrl_info(dma_config_info_t *dma_cfg,
                                                   uint8_t            channel_num)
  \brief       Return the Channel Control Info from Descriptor info
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \return      dma_ccr_t channel Control Info (CCR)
*/
dma_ccr_t dma_get_channel_ctrl_info(dma_config_info_t *dma_cfg,
                                    uint8_t            channel_num)
{
    dma_thread_info_t  *thread_info       = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info      = &thread_info->channel_info;
    dma_desc_info_t    *channel_desc_info = &channel_info->desc_info;
    dma_ccr_t           dma_ccr;

    dma_ccr.value                    = 0;

    dma_ccr.value_b.dst_burst_len    = channel_desc_info->dst_blen - 1;
    dma_ccr.value_b.src_burst_len    = channel_desc_info->src_blen - 1;

    dma_ccr.value_b.dst_burst_size   = channel_desc_info->dst_bsize;
    dma_ccr.value_b.src_burst_size   = channel_desc_info->src_bsize;

    dma_ccr.value_b.dst_cache_ctrl   = channel_desc_info->dst_cache_ctrl;
    dma_ccr.value_b.src_cache_ctrl   = channel_desc_info->src_cache_ctrl;

    dma_ccr.value_b.dst_prot_ctrl    = channel_desc_info->dst_prot_ctrl;
    dma_ccr.value_b.src_prot_ctrl    = channel_desc_info->src_prot_ctrl;

    dma_ccr.value_b.endian_swap_size = channel_desc_info->endian_swap_size;

    if(channel_desc_info->direction == DMA_TRANSFER_MEM_TO_DEV)
    {
        dma_ccr.value_b.dst_inc = DMA_BURST_FIXED;
        dma_ccr.value_b.src_inc = DMA_BURST_INCREMENTING;
    }
    else if(channel_desc_info->direction == DMA_TRANSFER_DEV_TO_MEM)
    {
        dma_ccr.value_b.dst_inc = DMA_BURST_INCREMENTING;
        dma_ccr.value_b.src_inc = DMA_BURST_FIXED;
    }
    else
    {
        dma_ccr.value_b.dst_inc = DMA_BURST_INCREMENTING;
        dma_ccr.value_b.src_inc = DMA_BURST_INCREMENTING;
    }

    return dma_ccr;
}

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
 * @file     dma_op.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     29-Sep-2023
 * @brief    DMA Header file for opcode generation
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef DMA_OP_H_
#define DMA_OP_H_

#include <dma_ctrl.h>

#ifdef  __cplusplus
}
#endif

typedef enum _DMA_CHANNEL_FLAG {
    DMA_CHANNEL_FLAG_USE_USER_MCODE      = (1 << 0),         /*!< Use user provided mcode for channel */
    DMA_CHANNEL_FLAG_I2S_MONO_MODE       = (1 << 1),         /*!< DMA channel in I2S mono mode */
    DMA_CHANNEL_FLAG_CRC_MODE            = (1 << 2),         /*!< CRC: Skip peripheral flush and wait */
} DMA_CHANNEL_FLAG;


/**
  \fn          void dma_assign_user_opcode(dma_config_info_t *dma_cfg,
                                           uint8_t            channel_num,
                                           void*              opcode_buf)
  \brief       Set the user opcode
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \param[in]   opcode_buf  Buffer pointing to the opcode
  \return      None
*/
static inline void dma_assign_user_opcode(dma_config_info_t *dma_cfg,
                                          uint8_t            channel_num,
                                          void*              opcode_buf)
{
    dma_thread_info_t  *thread_info    = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info   = &thread_info->channel_info;

    channel_info->flags     |= DMA_CHANNEL_FLAG_USE_USER_MCODE;
    thread_info->user_mcode  = opcode_buf;
}

/**
  \fn          void dma_set_i2s_mono_mode(dma_config_info_t *dma_cfg,
                                          uint8_t            channel_num)
  \brief       Set I2S mono operation
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \return      None
*/
static inline void dma_set_i2s_mono_mode(dma_config_info_t *dma_cfg,
                                          uint8_t           channel_num)
{
    dma_thread_info_t  *thread_info    = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info   = &thread_info->channel_info;

    channel_info->flags     |= DMA_CHANNEL_FLAG_I2S_MONO_MODE;
}

/**
  \fn          void dma_set_crc_mode(dma_config_info_t *dma_cfg,
                                     uint8_t            channel_num)
  \brief       Set CRC operation
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \return      None
*/
static inline void dma_set_crc_mode(dma_config_info_t *dma_cfg,
                                    uint8_t            channel_num)
{
    dma_thread_info_t  *thread_info    = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info   = &thread_info->channel_info;

    channel_info->flags     |= DMA_CHANNEL_FLAG_CRC_MODE;
}

/**
  \fn          void dma_set_swap_size(dma_config_info_t *dma_cfg,
                                      uint8_t            channel_num,
                                      uint8_t            swap_size)
  \brief       Set Swap Size
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \param[in]   swap_size  Endian Swap Size
  \return      None
*/
static inline void dma_set_swap_size(dma_config_info_t *dma_cfg,
                                     uint8_t            channel_num,
                                     uint8_t            swap_size)
{
    dma_thread_info_t  *thread_info    = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info   = &thread_info->channel_info;

    channel_info->desc_info.endian_swap_size = swap_size;
}

/**
  \fn          uint8_t* dma_get_opcode_buf(dma_config_info_t *dma_cfg,
                                           uint8_t            channel_num)
  \brief       Get the opcode buffer address of the channel
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \return      uint8_t* Pointer to the buffer
*/
static inline uint8_t* dma_get_opcode_buf(dma_config_info_t *dma_cfg,
                                          uint8_t            channel_num)
{
    dma_thread_info_t  *thread_info    = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info   = &thread_info->channel_info;

    if(channel_info->flags & DMA_CHANNEL_FLAG_USE_USER_MCODE)
        return (uint8_t *)thread_info->user_mcode;
    else
        return &thread_info->dma_mcode[0];
}

/**
  \fn          bool dma_generate_opcode(dma_config_info_t *dma_cfg,
                                        uint8_t            channel_num)
  \brief       Prepare the DMA opcode for the channel
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \return      bool false if the buffer is not enough, true otherwise
*/
bool dma_generate_opcode(dma_config_info_t *dma_cfg, uint8_t channel_num);

#ifdef  __cplusplus
}
#endif

#endif /* DMA_OP_H_ */

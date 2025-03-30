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
 * @file     dma_info.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     11-08-2023
 * @brief    DMA Header file for channel and thread info
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef DMA_CTRL_H_
#define DMA_CTRL_H_

#include <dma_config.h>
#include <dma_opcode.h>
#include <dma.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef  __cplusplus
}
#endif

/**
\brief DMA Transfer Direction
*/
typedef enum _DMA_TRANSFER {
    DMA_TRANSFER_MEM_TO_MEM,
    DMA_TRANSFER_MEM_TO_DEV,
    DMA_TRANSFER_DEV_TO_MEM,
    DMA_TRANSFER_NONE,
} DMA_TRANSFER;

typedef struct _dma_desc_info_t {
    DMA_TRANSFER      direction;                   /*!< Direction of data transfer      */
    DMA_SECURE_STATE  sec_state;                   /*!< Secure state of channel         */
    uint32_t          total_len;                   /*!< Number of bytes                 */
    uint32_t          src_addr;                    /*!< Source address                  */
    uint32_t          dst_addr0;                   /*!< Destination address             */
    uint32_t          dst_addr1;                   /*!< Destination address             */
    uint8_t           src_bsize;                   /*!< Src Burst Size                  */
    uint8_t           dst_bsize;                   /*!< Dest Burst Size                 */
    uint8_t           src_blen;                    /*!< Src Burst length                */
    uint8_t           dst_blen;                    /*!< Dest Burst length               */
    uint8_t           periph_num;                  /*!< Peripheral request number       */
    uint8_t           dst_cache_ctrl;              /*!< Dest Cache Control              */
    uint8_t           src_cache_ctrl;              /*!< Src Cache Control               */
    uint8_t           dst_prot_ctrl;               /*!< Dest Protection Control         */
    uint8_t           src_prot_ctrl;               /*!< Src Protection Control          */
    uint8_t           endian_swap_size;            /*!< Endian Swap Size                */
} dma_desc_info_t;

typedef struct _dma_channel_info_t {
    uint32_t          flags;                       /*!< Channel flags                   */
    bool              last_req;                    /*!< If this is last request         */
    uint8_t           event_index;                 /*!< Event/IRQ index                 */
    dma_desc_info_t   desc_info;                   /*!< DMA descriptor                  */
} dma_channel_info_t;

typedef struct _dma_thread_info_t {
    dma_channel_info_t  channel_info;             /*!< Channel information              */
    uint8_t             dma_mcode[DMA_MICROCODE_SIZE];/*!< DMA microcode buffer         */
    void                *user_mcode;              /*!< User provided mcode address      */
    bool                in_use;                   /*!< Status of DMA thread being used  */
} dma_thread_info_t;

typedef struct _dma_config_info_t {
    dma_thread_info_t  channel_thread[DMA_MAX_CHANNELS];/*!< Channel thread info        */
    uint8_t            event_map[DMA_MAX_EVENTS];       /*!< Events utilization mapping */
} dma_config_info_t;


/**
  \fn          uint8_t dma_get_event_index(dma_config_info_t *dma_cfg,
                                           uint8_t            channel_num)
  \brief       Get the event index of the channel
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \return      uint8_t Event index
*/
static inline uint8_t dma_get_event_index(dma_config_info_t *dma_cfg,
                                           uint8_t           channel_num)
{
    dma_thread_info_t  *thread_info    = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info   = &thread_info->channel_info;

    return channel_info->event_index;
}

/**
  \fn          uint8_t dma_get_channel_flags(dma_config_info_t *dma_cfg,
                                             uint8_t            channel_num)
  \brief       Get the flags set for the channel
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \return      uint8_t Return the current flags
*/
static inline uint32_t dma_get_channel_flags(dma_config_info_t *dma_cfg,
                                             uint8_t            channel_num)
{
    dma_thread_info_t  *thread_info    = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info   = &thread_info->channel_info;

    return channel_info->flags;
}

/**
  \fn          dma_desc_info_t* dma_get_desc_info(dma_config_info_t *dma_cfg,
                                                  uint8_t            channel_num)
  \brief       Get the descriptor info of the channel
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \return      dma_desc_info_t* Pointer to the descriptor
*/
static inline dma_desc_info_t* dma_get_desc_info(dma_config_info_t *dma_cfg,
                                                 uint8_t            channel_num)
{
    dma_thread_info_t  *thread_info    = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info   = &thread_info->channel_info;

    return &channel_info->desc_info;
}

/**
  \fn          void dma_reset_all_channels(dma_config_info_t *dma_cfg)
  \brief       Reset all the channels
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \return      void
*/
static inline void dma_reset_all_channels(dma_config_info_t *dma_cfg)
{
    uint8_t channel_num = 0;

    for (channel_num = 0; channel_num < DMA_MAX_CHANNELS; channel_num++)
    {
        dma_cfg->channel_thread[channel_num].in_use = false;
    }
}

/**
  \fn          int8_t dma_allocate_channel(dma_config_info_t *dma_cfg)
  \brief       Allocate a channel if available
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \return      int8_t channel number or -1 if not available
*/
int8_t dma_allocate_channel(dma_config_info_t *dma_cfg);

/**
  \fn          void dma_release_channel(dma_config_info_t *dma_cfg,
                                        uint8_t            channel_num)
  \brief       Release the channel
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \return      void
*/
static inline void dma_release_channel(dma_config_info_t *dma_cfg,
                                       uint8_t            channel_num)
{
    dma_thread_info_t  *thread_info    = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info   = &thread_info->channel_info;

    thread_info->in_use = false;
    thread_info->user_mcode = (void *)0;

    channel_info->flags = 0;

}

/**
  \fn          void dma_reset_all_events(dma_config_info_t *dma_cfg)
  \brief       Reset all the events
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \return      void
*/
static inline void dma_reset_all_events(dma_config_info_t *dma_cfg)
{
    uint8_t event_index = 0;

    for (event_index = 0; event_index < DMA_MAX_EVENTS; event_index++)
    {
        dma_cfg->event_map[event_index] = 0xFF;
    }
}

/**
  \fn          int8_t dma_allocate_event(dma_config_info_t *dma_cfg,
                                         uint8_t            channel_num)
  \brief       Allocate a event if available
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel Number
  \return      int8_t Event index number or -1 if not available
*/
int8_t dma_allocate_event(dma_config_info_t *dma_cfg, uint8_t channel_num);

/**
  \fn          int8_t dma_release_event(dma_config_info_t *dma_cfg,
                                        int8_t             event_index)
  \brief       Release the event
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   event_index  Event Index
  \return      void
*/
static inline void dma_release_event(dma_config_info_t *dma_cfg,
                                     uint8_t            event_index)

{
    dma_cfg->event_map[event_index] = 0xFF;
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
                        dma_desc_info_t   *desc_info);

/**
  \fn          void dma_set_secure_state(dma_config_info_t *dma_cfg,
                                         uint8_t            channel_num,
                                         DMA_SECURE_STATE   sec_state)
  \brief       Set the Secure state of the channel desc
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel number
  \param[in]   sec_state  Secure State
  \return      void
*/
static inline void dma_set_secure_state(dma_config_info_t *dma_cfg,
                                        uint8_t            channel_num,
                                        DMA_SECURE_STATE   sec_state)

{
    dma_thread_info_t  *thread_info       = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info      = &thread_info->channel_info;
    dma_desc_info_t    *channel_desc_info = &channel_info->desc_info;

    channel_desc_info->sec_state = sec_state;
}

/**
  \fn          DMA_SECURE_STATE dma_get_secure_state(dma_config_info_t *dma_cfg,
                                                     uint8_t            channel_num,
                                                     DMA_SECURE_STATE   sec_state)
  \brief       Get the Secure state of the channel desc
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel number
  \return      DMA_SECURE_STATE Secure State
*/
static inline DMA_SECURE_STATE dma_get_secure_state(dma_config_info_t *dma_cfg,
                                                    uint8_t            channel_num)

{
    dma_thread_info_t  *thread_info       = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info      = &thread_info->channel_info;
    dma_desc_info_t    *channel_desc_info = &channel_info->desc_info;

    return channel_desc_info->sec_state;
}

/**
  \fn          void dma_set_cache_ctrl(dma_config_info_t *dma_cfg,
                                       uint8_t            channel_num,
                                       uint8_t            src_cache_ctrl,
                                       uint8_t            dst_cache_ctrl)
  \brief       Set the Cache control state of the channel desc
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel number
  \param[in]   src_cache_ctrl  Source Cache Control bits
  \param[in]   dst_cache_ctrl  Destination Cache Control bits
  \return      void
*/
static inline void dma_set_cache_ctrl(dma_config_info_t *dma_cfg,
                                      uint8_t            channel_num,
                                      uint8_t            src_cache_ctrl,
                                      uint8_t            dst_cache_ctrl)

{
    dma_thread_info_t  *thread_info       = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info      = &thread_info->channel_info;
    dma_desc_info_t    *channel_desc_info = &channel_info->desc_info;

    channel_desc_info->dst_cache_ctrl = dst_cache_ctrl;
    channel_desc_info->src_cache_ctrl = src_cache_ctrl;
}

/**
  \fn          void dma_set_prot_ctrl(dma_config_info_t *dma_cfg,
                                      uint8_t            channel_num,
                                      uint8_t            src_prot_ctrl,
                                      uint8_t            dst_prot_ctrl)
  \brief       Set the protection(secure) control state of the channel desc
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel number
  \param[in]   src_prot_ctrl  Source protection Control bits
  \param[in]   dst_prot_ctrl  Destination protection Control bits
  \return      void
*/
static inline void dma_set_prot_ctrl(dma_config_info_t *dma_cfg,
                                     uint8_t            channel_num,
                                     uint8_t            src_prot_ctrl,
                                     uint8_t            dst_prot_ctrl)

{
    dma_thread_info_t  *thread_info       = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info      = &thread_info->channel_info;
    dma_desc_info_t    *channel_desc_info = &channel_info->desc_info;

    channel_desc_info->dst_prot_ctrl = dst_prot_ctrl;
    channel_desc_info->src_prot_ctrl = src_prot_ctrl;
}

/**
  \fn          void dma_set_endian_swap_size(dma_config_info_t *dma_cfg,
                                             uint8_t            channel_num,
                                             DMA_SWAP           endian_swap)
  \brief       Set the Endian Swap Size of the channel desc
  \param[in]   dma_cfg  Pointer to DMA Configuration resources
  \param[in]   channel_num  Channel number
  \param[in]   endian_swap  Endian Swap Size
  \return      void
*/
static inline void dma_set_endian_swap_size(dma_config_info_t *dma_cfg,
                                            uint8_t            channel_num,
                                            DMA_SWAP           endian_swap)

{
    dma_thread_info_t  *thread_info       = &dma_cfg->channel_thread[channel_num];
    dma_channel_info_t *channel_info      = &thread_info->channel_info;
    dma_desc_info_t    *channel_desc_info = &channel_info->desc_info;

    channel_desc_info->endian_swap_size = endian_swap;
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
                                    uint8_t            channel_num);

#ifdef  __cplusplus
}
#endif

#endif /* DMA_CTRL_H_ */

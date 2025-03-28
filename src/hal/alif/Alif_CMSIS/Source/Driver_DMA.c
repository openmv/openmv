/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     Driver_DMA.c
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     04-Nov-2020
 * @brief    CMSIS-Driver for DMA.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "Driver_DMA_Private.h"
#include <evtrtr.h>
#include <dma_op.h>
#include <string.h>

#define ARM_DMA_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 1) /*!< DMA Driver Version */

static const ARM_DRIVER_VERSION DriverVersion = {
        ARM_DMA_API_VERSION,
        ARM_DMA_DRV_VERSION
};

#if ((RTE_DMA1) || (RTE_DMA2))
#define RTE_DMALOCAL 1
#endif

#if !((RTE_DMA0) || (RTE_DMALOCAL))
#error "DMA is not enabled in the RTE_Device.h"
#endif

#if !defined(RTE_Drivers_DMA)
#error "DMA not defined in RTE_Components.h!"
#endif

#if RTE_GPIO3
#define GPIO3_DMA_GLITCH_FILTER ((RTE_GPIO3_PIN0_DMA_GLITCH_FILTER_ENABLE << 0)|\
                                 (RTE_GPIO3_PIN1_DMA_GLITCH_FILTER_ENABLE << 1)|\
                                 (RTE_GPIO3_PIN2_DMA_GLITCH_FILTER_ENABLE << 2)|\
                                 (RTE_GPIO3_PIN3_DMA_GLITCH_FILTER_ENABLE << 3)|\
                                 (RTE_GPIO3_PIN4_DMA_GLITCH_FILTER_ENABLE << 4)|\
                                 (RTE_GPIO3_PIN5_DMA_GLITCH_FILTER_ENABLE << 5)|\
                                 (RTE_GPIO3_PIN6_DMA_GLITCH_FILTER_ENABLE << 6)|\
                                 (RTE_GPIO3_PIN7_DMA_GLITCH_FILTER_ENABLE << 7))
#else
#define GPIO3_DMA_GLITCH_FILTER 0
#endif
#if RTE_GPIO4
#define GPIO4_DMA_GLITCH_FILTER ((RTE_GPIO4_PIN0_DMA_GLITCH_FILTER_ENABLE << 8)|\
                                 (RTE_GPIO4_PIN1_DMA_GLITCH_FILTER_ENABLE << 9)|\
                                 (RTE_GPIO4_PIN2_DMA_GLITCH_FILTER_ENABLE << 10)|\
                                 (RTE_GPIO4_PIN3_DMA_GLITCH_FILTER_ENABLE << 11)|\
                                 (RTE_GPIO4_PIN4_DMA_GLITCH_FILTER_ENABLE << 12)|\
                                 (RTE_GPIO4_PIN5_DMA_GLITCH_FILTER_ENABLE << 13)|\
                                 (RTE_GPIO4_PIN6_DMA_GLITCH_FILTER_ENABLE << 14)|\
                                 (RTE_GPIO4_PIN7_DMA_GLITCH_FILTER_ENABLE << 15))
#else
#define GPIO4_DMA_GLITCH_FILTER 0
#endif
#if RTE_GPIO7
#define GPIO7_DMA_GLITCH_FILTER ((RTE_GPIO7_PIN0_DMA_GLITCH_FILTER_ENABLE << 16)|\
                                 (RTE_GPIO7_PIN1_DMA_GLITCH_FILTER_ENABLE << 17)|\
                                 (RTE_GPIO7_PIN2_DMA_GLITCH_FILTER_ENABLE << 18)|\
                                 (RTE_GPIO7_PIN3_DMA_GLITCH_FILTER_ENABLE << 19)|\
                                 (RTE_GPIO7_PIN4_DMA_GLITCH_FILTER_ENABLE << 20)|\
                                 (RTE_GPIO7_PIN5_DMA_GLITCH_FILTER_ENABLE << 21)|\
                                 (RTE_GPIO7_PIN6_DMA_GLITCH_FILTER_ENABLE << 22)|\
                                 (RTE_GPIO7_PIN7_DMA_GLITCH_FILTER_ENABLE << 23))
#else
#define GPIO7_DMA_GLITCH_FILTER 0
#endif
#if RTE_GPIO8
#define GPIO8_DMA_GLITCH_FILTER ((RTE_GPIO8_PIN0_DMA_GLITCH_FILTER_ENABLE << 24)|\
                                 (RTE_GPIO8_PIN1_DMA_GLITCH_FILTER_ENABLE << 25)|\
                                 (RTE_GPIO8_PIN2_DMA_GLITCH_FILTER_ENABLE << 26)|\
                                 (RTE_GPIO8_PIN3_DMA_GLITCH_FILTER_ENABLE << 27)|\
                                 (RTE_GPIO8_PIN4_DMA_GLITCH_FILTER_ENABLE << 28)|\
                                 (RTE_GPIO8_PIN5_DMA_GLITCH_FILTER_ENABLE << 29)|\
                                 (RTE_GPIO8_PIN6_DMA_GLITCH_FILTER_ENABLE << 30)|\
                                 (RTE_GPIO8_PIN7_DMA_GLITCH_FILTER_ENABLE << 31))
#else
#define GPIO8_DMA_GLITCH_FILTER 0
#endif

#define DMA0_GLITCH_FILTER     (GPIO3_DMA_GLITCH_FILTER | \
                                GPIO4_DMA_GLITCH_FILTER | \
                                GPIO7_DMA_GLITCH_FILTER | \
                                GPIO8_DMA_GLITCH_FILTER)

/* ----------  Local DMA Driver Access Struct Alias & RTE alias  ---------- */
#if defined(M55_HP)
#define Driver_DMALOCAL                    Driver_DMA1

#define RTE_DMALOCAL_APB_INTERFACE         RTE_DMA1_APB_INTERFACE
#define RTE_DMALOCAL_ABORT_IRQ_PRI         RTE_DMA1_ABORT_IRQ_PRI
#define RTE_DMALOCAL_BOOT_IRQ_NS_STATE     RTE_DMA1_BOOT_IRQ_NS_STATE
#define RTE_DMALOCAL_BOOT_PERIPH_NS_STATE  RTE_DMA1_BOOT_PERIPH_NS_STATE

#if RTE_GPIO9
#define DMALOCAL_GLITCH_FILTER ((RTE_GPIO9_PIN0_DMA_GLITCH_FILTER_ENABLE << 0)|\
                                (RTE_GPIO9_PIN1_DMA_GLITCH_FILTER_ENABLE << 1)|\
                                (RTE_GPIO9_PIN2_DMA_GLITCH_FILTER_ENABLE << 2)|\
                                (RTE_GPIO9_PIN3_DMA_GLITCH_FILTER_ENABLE << 3)|\
                                (RTE_GPIO9_PIN4_DMA_GLITCH_FILTER_ENABLE << 4)|\
                                (RTE_GPIO9_PIN5_DMA_GLITCH_FILTER_ENABLE << 5)|\
                                (RTE_GPIO9_PIN6_DMA_GLITCH_FILTER_ENABLE << 6)|\
                                (RTE_GPIO9_PIN7_DMA_GLITCH_FILTER_ENABLE << 7))
#else
#define DMALOCAL_GLITCH_FILTER 0
#endif

#elif defined(M55_HE)
#define Driver_DMALOCAL                    Driver_DMA2

#define RTE_DMALOCAL_APB_INTERFACE         RTE_DMA2_APB_INTERFACE
#define RTE_DMALOCAL_ABORT_IRQ_PRI         RTE_DMA2_ABORT_IRQ_PRI
#define RTE_DMALOCAL_BOOT_IRQ_NS_STATE     RTE_DMA2_BOOT_IRQ_NS_STATE
#define RTE_DMALOCAL_BOOT_PERIPH_NS_STATE  RTE_DMA2_BOOT_PERIPH_NS_STATE

#if RTE_LPGPIO
#define DMALOCAL_GLITCH_FILTER ((RTE_LPGPIO_PIN0_DMA_GLITCH_FILTER_ENABLE << 0)|\
                                (RTE_LPGPIO_PIN1_DMA_GLITCH_FILTER_ENABLE << 1)|\
                                (RTE_LPGPIO_PIN2_DMA_GLITCH_FILTER_ENABLE << 2)|\
                                (RTE_LPGPIO_PIN3_DMA_GLITCH_FILTER_ENABLE << 3)|\
                                (RTE_LPGPIO_PIN4_DMA_GLITCH_FILTER_ENABLE << 4)|\
                                (RTE_LPGPIO_PIN5_DMA_GLITCH_FILTER_ENABLE << 5)|\
                                (RTE_LPGPIO_PIN6_DMA_GLITCH_FILTER_ENABLE << 6)|\
                                (RTE_LPGPIO_PIN7_DMA_GLITCH_FILTER_ENABLE << 7))
#else
#define DMALOCAL_GLITCH_FILTER 0
#endif
#endif

/* Driver Capabilities */
static const ARM_DMA_CAPABILITIES DriverCapabilities = {
    1,   /* supports memory to memory operation */
    1,   /* supports memory to peripheral operation */
    1,   /* supports peripheral to memory operation */
    0,   /* supports Scatter Gather */
    1,   /* supports Secure/Non-Secure mode operation */
    0    /* reserved (must be zero) */
};

#if (RTE_DMA0)
static DMA_RESOURCES DMA0 = {
    .regs       = NULL,
    .state      = {0},
    .irq_start  = DMA0_IRQ0_IRQn,
    .abort_irq_priority = RTE_DMA0_ABORT_IRQ_PRI,
    .instance   = DMA_INSTANCE_0,
};
#endif

#if (RTE_DMALOCAL)
static DMA_RESOURCES DMALOCAL = {
    .regs       = NULL,
    .state      = {0},
    .irq_start  = DMALOCAL_IRQ0_IRQn,
    .abort_irq_priority = RTE_DMALOCAL_ABORT_IRQ_PRI,
    .instance   = DMA_INSTANCE_LOCAL,
};
#endif

/**
  \fn          ARM_DRIVER_VERSION DMA_GetVersion(void)
  \brief       Get DMA driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION DMA_GetVersion(void)
{
    return DriverVersion;
}

/**
  \fn          ARM_DMA_CAPABILITIES DMA_GetCapabilities(void)
  \brief       Get DMA driver capabilities
  \return      \ref ARM_DMA_CAPABILITIES
*/
static ARM_DMA_CAPABILITIES DMA_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
  \fn          DMA_SECURE_STATE DMA_GetSecureState(uint8_t value)
  \brief       Get the Secure State from RTE
  \param[in]   value   Input value from RTE configuration
  \return      DMA_SECURE_STATE  \ref DMA_SECURE_STATE
*/
static DMA_SECURE_STATE DMA_GetSecureState(uint8_t value)
{
    DMA_SECURE_STATE  sec_state;

    switch(value)
    {
    case 0:
        sec_state = DMA_STATE_SECURE;
        break;
    default:
        sec_state = DMA_STATE_NON_SECURE;
        break;
    }

    return sec_state;
}

/**
  \fn          void DMA_InitDescDefaults(uint8_t channel_num, DMA_RESOURCES *DMA)
  \brief       Set the descriptor defaults
  \param[in]   channel_num  DMA channel
  \param[in]   DMA  Pointer to DMA resources
  \return      None
*/
static void DMA_InitDescDefaults(uint8_t channel_num, DMA_RESOURCES *DMA)
{
    DMA_SECURE_STATE  sec_state;
    dma_config_info_t *dma_cfg = &DMA->cfg;

    sec_state = DMA_GetSecureState(dma_manager_is_nonsecure(DMA->regs));
    dma_set_secure_state(dma_cfg, channel_num, sec_state);

    dma_set_cache_ctrl(dma_cfg, channel_num,
                       DMA_SRC_CACHE_CTRL,
                       DMA_DEST_CACHE_CTRL);

    dma_set_prot_ctrl(dma_cfg, channel_num,
                      DMA_SRC_PROT_CTRL,
                      DMA_DEST_PROT_CTRL);

    dma_set_endian_swap_size(dma_cfg, channel_num, DMA_SWAP_NONE);
}

/**
  \fn          int32_t DMA_GetEndianSwapSize(ARM_DMA_ESS_Type swap_size,
                                             uint8_t *ess)
  \brief       Get the endian swap size value
  \param[in]   swap_size  User provided swap size
  \param[out]  Endian Swap size value
  \return      /ref execution_status
*/
static int32_t DMA_GetEndianSwapSize(ARM_DMA_ESS_Type swap_size, uint8_t *ess)
{
    uint8_t val = 0;

    switch(swap_size)
    {
    case ESS_SWAP_NONE:
        val = 0;
        break;
    case ESS_SWAP_16BIT:
        val = 1;
        break;
    case ESS_SWAP_32BIT:
        val = 2;
        break;
    case ESS_SWAP_64BIT:
        val = 3;
        break;
    default:
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    *ess = val;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DMA_CopyDesc(uint8_t         channel_num,
                                    ARM_DMA_PARAMS *params,
                                    DMA_RESOURCES  *DMA)
  \brief       Copy the descriptor information
  \param[in]   channel_num  DMA channel
  \param[in]   params  Descriptor information
  \param[in]   DMA  Pointer to DMA resources
  \return      \ref execution_status
*/
static int32_t DMA_CopyDesc(uint8_t         channel_num,
                            ARM_DMA_PARAMS *params,
                            DMA_RESOURCES  *DMA)
{
    dma_config_info_t *dma_cfg = &DMA->cfg;
    dma_desc_info_t    dma_desc;

    if(((1 << params->burst_size) > DMA_MAX_BURST_SIZE) ||
       (params->burst_len > DMA_MAX_BURST_LEN) ||
       (!params->burst_len) ||
       (!params->cb_event) ||
       (!params->num_bytes))
        return ARM_DRIVER_ERROR_PARAMETER;

    if(params->dir == ARM_DMA_MEM_TO_MEM)
        dma_desc.direction = DMA_TRANSFER_MEM_TO_MEM;
    else if(params->dir == ARM_DMA_MEM_TO_DEV)
        dma_desc.direction = DMA_TRANSFER_MEM_TO_DEV;
    else if(params->dir == ARM_DMA_DEV_TO_MEM)
        dma_desc.direction = DMA_TRANSFER_DEV_TO_MEM;
    else
        return ARM_DRIVER_ERROR_PARAMETER;

    if((dma_desc.direction == DMA_TRANSFER_DEV_TO_MEM) ||
       (dma_desc.direction == DMA_TRANSFER_MEM_TO_DEV))
    {
        if(params->peri_reqno >= DMA_MAX_PERIPH_REQ)
            return ARM_DRIVER_ERROR_PARAMETER;
    }

    dma_desc.periph_num  = (uint8_t)params->peri_reqno;
    dma_desc.dst_addr    = LocalToGlobal(params->dst_addr);
    dma_desc.src_addr    = LocalToGlobal(params->src_addr);
    dma_desc.dst_blen    = params->burst_len;
    dma_desc.src_blen    = params->burst_len;
    dma_desc.total_len   = params->num_bytes;

    dma_desc.dst_bsize   = params->burst_size;

    if(params->dir == ARM_DMA_MEM_TO_MEM)
    {
        while((dma_desc.dst_addr |
               dma_desc.src_addr |
               dma_desc.total_len) &
               ((1 << dma_desc.dst_bsize) - 1))
        {
            dma_desc.dst_bsize = dma_desc.dst_bsize - 1;
        }
    }
    else
    {
        if((dma_desc.dst_addr |
            dma_desc.src_addr |
            dma_desc.total_len) &
           ((1 << dma_desc.dst_bsize) - 1))
        {
            return ARM_DMA_ERROR_UNALIGNED;
        }
    }

    dma_desc.src_bsize   = dma_desc.dst_bsize;

    dma_copy_desc_info(dma_cfg, channel_num, &dma_desc);

    return ARM_DRIVER_OK;
}

/**
  \fn          void DMA_InvalidateDCache(dma_desc_info_t *desc_info)
  \brief       Invalidate the Dcache based on direction
  \param[in]   desc_info  DMA descriptor info
  \return      None
*/
__STATIC_INLINE void DMA_InvalidateDCache(dma_desc_info_t *desc_info)
{
    if((desc_info->direction == DMA_TRANSFER_MEM_TO_MEM) ||
       (desc_info->direction == DMA_TRANSFER_DEV_TO_MEM))
    {
        RTSS_InvalidateDCache_by_Addr(GlobalToLocal(desc_info->dst_addr),
                                      (int32_t)desc_info->total_len);
    }
}

/**
  \fn          int32_t DMA_CleanDCache(dma_desc_info_t *desc_info)
  \brief       Clean the Dcache based on direction
  \param[in]   desc_info  DMA descriptor info
  \return      None
*/
__STATIC_INLINE void DMA_CleanDCache(dma_desc_info_t *desc_info)
{
    if((desc_info->direction == DMA_TRANSFER_MEM_TO_MEM) ||
       (desc_info->direction == DMA_TRANSFER_MEM_TO_DEV))
    {
        RTSS_CleanDCache_by_Addr(GlobalToLocal(desc_info->src_addr),
                                 (int32_t)desc_info->total_len);
    }
}

/**
  \fn          int32_t DMA_DeAllocate(DMA_Handle_Type *handle,
                                      DMA_RESOURCES   *DMA)
  \brief       DeAllocate DMA channel
  \param[in]   handle  Pointer to DMA channel number
  \param[in]   DMA  Pointer to DMA resources
  \return      \ref execution_status
*/
__STATIC_INLINE int32_t DMA_DeAllocate(DMA_Handle_Type *handle,
                                       DMA_RESOURCES   *DMA)
{
    dma_config_info_t *dma_cfg = &DMA->cfg;
    uint8_t            event_index;
    uint8_t            channel_num;

    if(!DMA->state.powered)
        return ARM_DRIVER_ERROR;

    if(!handle)
        return ARM_DRIVER_ERROR_PARAMETER;

    if((*handle > DMA_MAX_CHANNELS) || (*handle < 0))
        return ARM_DMA_ERROR_HANDLE;

    __disable_irq();

    channel_num = (uint8_t)*handle;

    /* If the Channel is busy then return error so that app can call Stop */
    if(dma_get_channel_status(DMA->regs, channel_num) != DMA_THREAD_STATUS_STOPPED)
    {
        __enable_irq();
        return ARM_DRIVER_ERROR_BUSY;
    }

    event_index = dma_get_event_index(dma_cfg, channel_num);

    NVIC_DisableIRQ((IRQn_Type)(DMA->irq_start + event_index));

    DMA->cb_event[event_index] = (void *)0;

    dma_release_event(dma_cfg, event_index);
    dma_release_channel(dma_cfg, channel_num);

    *handle = -1;
    __enable_irq();

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DMA_GetStatus(DMA_Handle_Type *handle,
                                     uint32_t        *count,
                                     DMA_RESOURCES   *DMA)
  \brief       Status of a DMA channel
  \param[in]   handle  Pointer to DMA channel number
  \param[in]   count  Pointer to number of bytes
  \param[in]   DMA  Pointer to DMA resources
  \return      \ref execution_status
*/
static int32_t DMA_GetStatus(DMA_Handle_Type *handle,
                             uint32_t        *count,
                             DMA_RESOURCES   *DMA)
{
    dma_config_info_t  *dma_cfg        = &DMA->cfg;
    dma_desc_info_t    *desc;
    uint32_t            curr_addr;
    uint8_t             channel_num;
    DMA_THREAD_STATUS   thread_status;

    if(!DMA->state.powered)
        return ARM_DRIVER_ERROR;

    if(!handle || !count)
        return ARM_DRIVER_ERROR_PARAMETER;

    if((*handle > DMA_MAX_CHANNELS) || (*handle < 0))
        return ARM_DMA_ERROR_HANDLE;

    __disable_irq();

    channel_num = (uint8_t)*handle;

    desc = dma_get_desc_info(dma_cfg, channel_num);

    if((desc->direction == DMA_TRANSFER_MEM_TO_DEV) ||
       (desc->direction == DMA_TRANSFER_MEM_TO_MEM))
    {
        curr_addr = dma_get_channel_src_addr(DMA->regs, channel_num);
        *count = curr_addr - desc->src_addr;
    }
    else if((desc->direction == DMA_TRANSFER_DEV_TO_MEM) ||
            (desc->direction == DMA_TRANSFER_MEM_TO_MEM))
    {
        curr_addr = dma_get_channel_dest_addr(DMA->regs, channel_num);
        *count = curr_addr - desc->dst_addr;
    }

    thread_status = dma_get_channel_status(DMA->regs, channel_num);
    if((thread_status == DMA_THREAD_STATUS_FAULTING_COMPLETING) ||
       (thread_status == DMA_THREAD_STATUS_FAULTING))
    {
        __enable_irq();
        return ARM_DMA_ERROR_FAULT;
    }

    __enable_irq();
    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DMA_Stop(DMA_Handle_Type *handle, DMA_RESOURCES *DMA)
  \brief       Stop a DMA channel
  \param[in]   handle  Pointer to DMA channel number
  \param[in]   DMA  Pointer to DMA resources
  \return      \ref execution_status
*/
static int32_t DMA_Stop(DMA_Handle_Type *handle, DMA_RESOURCES *DMA)
{
    dma_config_info_t  *dma_cfg = &DMA->cfg;
    dma_dbginst0_t      dma_dbginst0;
    dma_desc_info_t    *desc_info;
    uint8_t             kill_opcode_buf =  {0};
    uint8_t             channel_num;
    uint8_t             event_index;
    DMA_THREAD_STATUS   thread_status;
    dma_opcode_buf      kill_opcode =
    {
        .buf = &kill_opcode_buf,
        .buf_size = DMA_OP_1BYTE_LEN,
        .off = 0
    };

    if(!DMA->state.powered)
        return ARM_DRIVER_ERROR;

    if(!handle)
        return ARM_DRIVER_ERROR_PARAMETER;

    if((*handle > DMA_MAX_CHANNELS) || (*handle < 0))
        return ARM_DMA_ERROR_HANDLE;

    __disable_irq();

    channel_num = (uint8_t)*handle;
    event_index = dma_get_event_index(dma_cfg, channel_num);

    thread_status = dma_get_channel_status(DMA->regs, channel_num);
    if(thread_status == DMA_THREAD_STATUS_STOPPED)
    {
        __enable_irq();
        return ARM_DRIVER_OK;
    }

    if(dma_debug_is_busy(DMA->regs))
    {
        __enable_irq();
        return ARM_DRIVER_ERROR_BUSY;
    }

    dma_construct_kill(&kill_opcode);

    dma_dbginst0.dbginst0               = 0;
    dma_dbginst0.dbginst0_b.ins_byte0   = kill_opcode_buf;
    dma_dbginst0.dbginst0_b.chn_num     = channel_num;
    dma_dbginst0.dbginst0_b.dbg_thrd    = true;

    dma_execute(DMA->regs, dma_dbginst0.dbginst0, 0);

    /* Wait for the Channel to be in the STOP state */
    while(1)
    {
        thread_status = dma_get_channel_status(DMA->regs, channel_num);
        if(thread_status == DMA_THREAD_STATUS_STOPPED)
        {
            break;
        }
    }

    dma_disable_interrupt(DMA->regs, event_index);
    dma_clear_interrupt(DMA->regs, event_index);

    NVIC_DisableIRQ((IRQn_Type)(DMA->irq_start + event_index));

    /* Invalidate the data from cache */
    desc_info = dma_get_desc_info(dma_cfg, channel_num);
    DMA_InvalidateDCache(desc_info);

    __enable_irq();

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DMA_Start(DMA_Handle_Type *handle,
                                 ARM_DMA_PARAMS  *params,
                                 DMA_RESOURCES   *DMA)
  \brief       Start a DMA channel
  \param[in]   handle  Pointer to DMA channel number
  \param[in]   params  Pointer to DMA desc parameters
  \param[in]   DMA  Pointer to DMA resources
  \return      \ref execution_status
*/
static int32_t DMA_Start(DMA_Handle_Type *handle,
                         ARM_DMA_PARAMS  *params,
                         DMA_RESOURCES   *DMA)
{
    dma_config_info_t  *dma_cfg = &DMA->cfg;
    dma_dbginst0_t      dma_dbginst0;
    dma_dbginst1_t      dma_dbginst1;
    dma_desc_info_t     desc_info;
    dma_desc_info_t    *channel_desc_info;
    uint8_t             go_opcode_buf[DMA_OP_6BYTE_LEN] =  {0};
    uint8_t            *opcode_buf;
    uint8_t             channel_num;
    uint8_t             event_index;
    int32_t             ret = 0;
    dma_opcode_buf go_opcode =
    {
        .buf = go_opcode_buf,
        .buf_size = DMA_OP_6BYTE_LEN,
        .off = 0
    };

    if(!DMA->state.powered)
        return ARM_DRIVER_ERROR;

    if(!handle || !params)
        return ARM_DRIVER_ERROR_PARAMETER;

    if((*handle > DMA_MAX_CHANNELS) || (*handle < 0))
        return ARM_DMA_ERROR_HANDLE;

    __disable_irq();

    channel_num = (uint8_t)*handle;
    event_index = dma_get_event_index(dma_cfg, channel_num);

    if(dma_debug_is_busy(DMA->regs))
    {
        __enable_irq();
        return ARM_DRIVER_ERROR_BUSY;
    }

    if(dma_get_channel_status(DMA->regs, channel_num) != DMA_THREAD_STATUS_STOPPED)
    {
        __enable_irq();
        return ARM_DMA_ERROR_BUSY;
    }

    memset((void*)&desc_info, 0, sizeof(desc_info));

    /* Check for user provided microcode */
    if(dma_get_channel_flags(dma_cfg, channel_num)
       & DMA_CHANNEL_FLAG_USE_USER_MCODE)
    {
        opcode_buf = dma_get_opcode_buf(dma_cfg, channel_num);

        if(params->dir == ARM_DMA_MEM_TO_MEM)
            desc_info.direction = DMA_TRANSFER_MEM_TO_MEM;
        else if(params->dir == ARM_DMA_MEM_TO_DEV)
            desc_info.direction = DMA_TRANSFER_MEM_TO_DEV;
        else if(params->dir == ARM_DMA_DEV_TO_MEM)
            desc_info.direction = DMA_TRANSFER_DEV_TO_MEM;
        else
        {
            __enable_irq();
            return ARM_DRIVER_ERROR_PARAMETER;
        }

        if((desc_info.direction == DMA_TRANSFER_DEV_TO_MEM) ||
           (desc_info.direction == DMA_TRANSFER_MEM_TO_DEV))
        {
            if(params->peri_reqno >= DMA_MAX_PERIPH_REQ)
            {
                __enable_irq();
                return ARM_DRIVER_ERROR_PARAMETER;
            }
        }

        desc_info.src_addr    = LocalToGlobal(params->src_addr);
        desc_info.dst_addr    = LocalToGlobal(params->dst_addr);
        desc_info.total_len   = params->num_bytes;
        desc_info.periph_num  = (uint8_t)params->peri_reqno;

        dma_copy_desc_info(dma_cfg, channel_num, &desc_info);
    }
    else
    {
        ret = DMA_CopyDesc(channel_num, params, DMA);
        if(ret < 0)
        {
            __enable_irq();
            return ret;
        }

        ret = dma_generate_opcode(dma_cfg, channel_num);
        if(!ret)
        {
            __enable_irq();
            return ARM_DMA_ERROR_BUFFER;
        }

        /* Flush the Cache now */
        opcode_buf = dma_get_opcode_buf(dma_cfg, channel_num);
        RTSS_CleanDCache_by_Addr(opcode_buf, DMA_MICROCODE_SIZE);
    }

    /* Assign the callback against the allocated event_index */
    DMA->cb_event[event_index] = params->cb_event;

    channel_desc_info = dma_get_desc_info(dma_cfg, channel_num);
    /* Src: Clean the data from the cache */
    DMA_CleanDCache(channel_desc_info);

    /* Dst: Invalidate the data from cache */
    DMA_InvalidateDCache(channel_desc_info);

    dma_construct_go(channel_desc_info->sec_state,
                     channel_num,
                     LocalToGlobal(opcode_buf),
                     &go_opcode);

    dma_enable_interrupt(DMA->regs, event_index);

    /* Disable it first */
    NVIC_DisableIRQ((IRQn_Type)(DMA->irq_start + event_index));
    /* Clear Any Pending IRQ */
    NVIC_ClearPendingIRQ((IRQn_Type)(DMA->irq_start + event_index));
    /* Set the priority of this particular IRQ */
    NVIC_SetPriority((IRQn_Type)(DMA->irq_start + event_index),
                     params->irq_priority);
    /* Enable the IRQ */
    NVIC_EnableIRQ((IRQn_Type)(DMA->irq_start + event_index));


    dma_dbginst0.dbginst0             = 0;
    dma_dbginst0.dbginst0_b.ins_byte0 = go_opcode_buf[0];
    dma_dbginst0.dbginst0_b.ins_byte1 = go_opcode_buf[1];
    dma_dbginst0.dbginst0_b.chn_num   = channel_num;
    dma_dbginst0.dbginst0_b.dbg_thrd  = DMA_THREAD_MANAGER;

    dma_dbginst1.dbginst1_b.ins_byte2 = go_opcode_buf[2];
    dma_dbginst1.dbginst1_b.ins_byte3 = go_opcode_buf[3];
    dma_dbginst1.dbginst1_b.ins_byte4 = go_opcode_buf[4];
    dma_dbginst1.dbginst1_b.ins_byte5 = go_opcode_buf[5];

    dma_execute(DMA->regs, dma_dbginst0.dbginst0, dma_dbginst1.dbginst1);

    __enable_irq();

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DMA_Control(DMA_Handle_Type *handle,
                                   uint32_t         control,
                                   uint32_t         arg,
                                   DMA_RESOURCES   *DMA)
  \brief       Control DMA Interface.
  \param[in]   handle  Pointer to DMA channel number
  \param[in]   control Operation
  \param[in]   arg     Argument (optional)
  \param[in]   DMA     Pointer to DMA resources
  \return      \ref execution_status and driver specific \ref dma exec status
*/
static int32_t DMA_Control (DMA_Handle_Type *handle,
                            uint32_t         control,
                            uint32_t         arg,
                            DMA_RESOURCES   *DMA)
{
    dma_config_info_t  *dma_cfg = &DMA->cfg;
    uint8_t             channel_num;
    int32_t             ret = ARM_DRIVER_OK;
    uint8_t             ess = 0;

    /* Verify whether the driver is initialized */
    if(!DMA->state.initialized)
        return ARM_DRIVER_ERROR;

    if(!handle)
        return ARM_DRIVER_ERROR_PARAMETER;

    if((*handle > DMA_MAX_CHANNELS) || (*handle < 0))
        return ARM_DMA_ERROR_HANDLE;

    channel_num = (uint8_t)*handle;

    /* Handle Control Codes */
    switch(control & ARM_DMA_CONTROL_Msk)
    {
    case ARM_DMA_USER_PROVIDED_MCODE:
        if(!arg)
            return ARM_DRIVER_ERROR_PARAMETER;
        dma_assign_user_opcode(dma_cfg, channel_num, (void*)arg);
        break;
    case ARM_DMA_I2S_MONO_MODE:
        dma_set_i2s_mono_mode(dma_cfg, channel_num);
        break;
    case ARM_DMA_CRC_MODE:
        dma_set_crc_mode(dma_cfg, channel_num);
        break;
    case ARM_DMA_ENDIAN_SWAP_SIZE:
        ret = DMA_GetEndianSwapSize(arg, &ess);
        if(ret)
            return ret;

        dma_set_swap_size(dma_cfg, channel_num, ess);
        break;
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DMA_Allocate(DMA_Handle_Type *handle, DMA_RESOURCES *DMA)
  \brief       Allocate Channel for transfer operation
  \param[in]   handle  Pointer to DMA channel number
  \param[in]   DMA  Pointer to DMA resources
  \return      \ref execution_status
*/
static int32_t DMA_Allocate(DMA_Handle_Type *handle, DMA_RESOURCES *DMA)
{
    dma_config_info_t *dma_cfg  = &DMA->cfg;
    int8_t             event    = 0;
    uint8_t            channel_num;

    if(!DMA->state.powered)
        return ARM_DRIVER_ERROR;

    if(!handle)
        return ARM_DRIVER_ERROR_PARAMETER;

    __disable_irq();

    *handle = dma_allocate_channel(dma_cfg);
    if(*handle < 0)
    {
        __enable_irq();
        return ARM_DMA_ERROR_HANDLE;
    }

    channel_num = (uint8_t)*handle;

    event = dma_allocate_event(dma_cfg, channel_num);
    if(event < 0)
    {
        dma_release_channel(dma_cfg, channel_num);
        __enable_irq();
        return ARM_DMA_ERROR_EVENT;
    }

    /* Set the Channel Descriptor Defaults */
    DMA_InitDescDefaults(channel_num, DMA);

    __enable_irq();

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DMA_Initialize(DMA_RESOURCES *DMA)
  \brief       Initialize DMA Interface.
  \param[in]   DMA       Pointer to DMA resources
  \return      \ref execution_status
*/
static int32_t DMA_Initialize(DMA_RESOURCES *DMA)
{
    dma_config_info_t *dma_cfg = &DMA->cfg;
    uint8_t            count;

    if(DMA->state.initialized)
        return ARM_DRIVER_OK;

    __disable_irq();

    for(count = 0; count < DMA_MAX_EVENTS; count++)
    {
        DMA->cb_event[count] = (void *)0;
    }

    dma_reset_all_events(dma_cfg);
    dma_reset_all_channels(dma_cfg);

    DMA->state.initialized = 1;

    __enable_irq();

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DMA_Uninitialize(DMA_RESOURCES *DMA)
  \brief       De-initialize DMA Interface.
  \param[in]   DMA       Pointer to DMA resources
  \return      \ref execution_status
*/
static int32_t DMA_Uninitialize(DMA_RESOURCES *DMA)
{
    /*
     * Note: All the consumers who used DMA has to call Uninitialize
     * for the DMA to release all its resources.
     *
     */

    __disable_irq();

    if(!DMA->state.powered)
    {
        DMA->state.initialized = 0;
    }

    __enable_irq();

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DMA_PowerControl(ARM_POWER_STATE state, DMA_RESOURCES *DMA)
  \brief       Control DMA Interface Power.
  \param[in]   state  Power state
  \param[in]   DMA    Pointer to DMA resources
  \return      \ref execution_status
*/
static int32_t DMA_PowerControl(ARM_POWER_STATE state, DMA_RESOURCES *DMA)
{
    if(!DMA->state.initialized)
    {
        return ARM_DRIVER_ERROR;
    }

    switch(state)
    {
    case ARM_POWER_OFF:
        __disable_irq();

        if(!DMA->state.powered)
        {
            __enable_irq();
            return ARM_DRIVER_OK;
        }

        /* Decrement the consumer count */
        DMA->consumer_cnt--;
        if(DMA->consumer_cnt)
        {
            __enable_irq();
            return ARM_DRIVER_OK;
        }

        NVIC_DisableIRQ((IRQn_Type)(DMA->irq_start + DMA_IRQ_ABORT_OFFSET));

        switch(DMA->instance)
        {
        case DMA_INSTANCE_0:
            evtrtr0_disable_dma_req();
            dma0_disable_periph_clk();

            DMA->state.powered = 0;

            break;
        case DMA_INSTANCE_LOCAL:
            evtrtrlocal_disable_dma_req();
            dmalocal_disable_periph_clk();

            DMA->state.powered = 0;

            break;
        default:
            break;
        }

        __enable_irq();

        break;
    case ARM_POWER_FULL:
        __disable_irq();

        /* Increment the consumer count */
        DMA->consumer_cnt++;

        if(DMA->state.powered)
        {
            __enable_irq();
            return ARM_DRIVER_OK;
        }

        switch(DMA->instance)
        {
        case DMA_INSTANCE_0:
            dma0_set_glitch_filter(DMA0_GLITCH_FILTER);

#if RTE_LPPDM_SELECT_DMA0
            lppdm_select_dma0();
#endif

#if RTE_LPI2S_SELECT_DMA0
            lpi2s_select_dma0();
#endif
#if RTE_LPSPI_SELECT_DMA0
            lpspi_select_dma0(RTE_LPSPI_SELECT_DMA0_GROUP);
#endif
#if RTE_LPUART_SELECT_DMA0
            lpuart_select_dma0();
#endif

            dma0_enable_periph_clk();
            evtrtr0_enable_dma_req();

            if(DMA->ns_iface)
                dma0_set_boot_manager_nonsecure();
            else
                dma0_set_boot_manager_secure();

            dma0_set_boot_irq_ns_mask(RTE_DMA0_BOOT_IRQ_NS_STATE);
            dma0_set_boot_periph_ns_mask(RTE_DMA0_BOOT_PERIPH_NS_STATE);

            dma0_reset();

            DMA->state.powered = 1;

            break;
        case DMA_INSTANCE_LOCAL:
            dmalocal_set_glitch_filter(DMALOCAL_GLITCH_FILTER);
            dmalocal_enable_periph_clk();
            evtrtrlocal_enable_dma_req();

            if(DMA->ns_iface)
                dmalocal_set_boot_manager_nonsecure();
            else
                dmalocal_set_boot_manager_secure();

            dmalocal_set_boot_irq_ns_mask(RTE_DMALOCAL_BOOT_IRQ_NS_STATE);
            dmalocal_set_boot_periph_ns_mask(RTE_DMALOCAL_BOOT_PERIPH_NS_STATE);

            dmalocal_reset();

            DMA->state.powered = 1;

            break;
        default:
            break;
        }

        /* Clear Any Pending IRQ */
        NVIC_ClearPendingIRQ((IRQn_Type)(DMA->irq_start + DMA_IRQ_ABORT_OFFSET));
        /* Set the priority of this particular IRQ */
        NVIC_SetPriority((IRQn_Type)(DMA->irq_start + DMA_IRQ_ABORT_OFFSET),
                         DMA->abort_irq_priority);
        /* Enable the Abort IRQ */
        NVIC_EnableIRQ((IRQn_Type)(DMA->irq_start + DMA_IRQ_ABORT_OFFSET));

        __enable_irq();

        break;

    case ARM_POWER_LOW:
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;

    }
    return ARM_DRIVER_OK;
}

/**
  \fn          void DMA_IRQHandler(void)
  \brief       common DMA IRQ handler
  \param[in]   event_idx  Event index
  \param[in]   DMA  DMA resource
  \return      None
*/
static void DMA_IRQHandler(uint8_t event_idx, DMA_RESOURCES *DMA)
{
    dma_config_info_t   *dma_cfg     = &DMA->cfg;
    dma_desc_info_t     *desc_info;
    uint8_t              channel_num = dma_cfg->event_map[event_idx];

    dma_clear_interrupt(DMA->regs, event_idx);

    desc_info = dma_get_desc_info(dma_cfg, channel_num);

    /* Invalidate the data from cache */
    DMA_InvalidateDCache(desc_info);

    if(DMA->cb_event[event_idx])
        DMA->cb_event[event_idx](ARM_DMA_EVENT_COMPLETE,
                                 (int8_t)desc_info->periph_num);
}

/**
  \fn          void DMA_AbortIRQHandler(void)
  \brief       Abort DMA handler
  \param[in]   DMA  DMA resource
  \return      None
*/
static void DMA_AbortIRQHandler(DMA_RESOURCES *DMA)
{
    dma_config_info_t   *dma_cfg      = &DMA->cfg;
    dma_desc_info_t     *desc_info;
    DMA_Handle_Type      handle       = 0;
    uint8_t              channel_num;
    uint8_t              event_idx;

    /* Get Manager Fault Status */
    if(dma_manager_is_faulting(DMA->regs))
    {
        /*
         * It requires software reset to the DMA controller to come out this
         * state and this can be achieved if ARM_POWER_FULL is called (make sure
         * all the consumers did call ARM_POWER_OFF before this)
         */
        for(channel_num = 0; channel_num < DMA_MAX_CHANNELS; channel_num++)
        {
            desc_info = dma_get_desc_info(dma_cfg, channel_num);
            event_idx = dma_get_event_index(dma_cfg, channel_num);

            /* Invalidate the data from cache */
            DMA_InvalidateDCache(desc_info);

            if(DMA->cb_event[event_idx])
                DMA->cb_event[event_idx](ARM_DMA_EVENT_ABORT,
                                         (int8_t)desc_info->periph_num);
        }
    }

    for(channel_num = 0; channel_num < DMA_MAX_CHANNELS; channel_num++)
    {
        if(dma_get_channel_fault_status(DMA->regs, channel_num))
        {
            handle = channel_num;
            DMA_Stop (&handle, DMA);

            desc_info = dma_get_desc_info(dma_cfg, channel_num);
            event_idx = dma_get_event_index(dma_cfg, channel_num);

            /* Invalidate the data from cache */
            DMA_InvalidateDCache(desc_info);

            if(DMA->cb_event[event_idx])
                DMA->cb_event[event_idx](ARM_DMA_EVENT_ABORT,
                                         (int8_t)desc_info->periph_num);
        }
    }
}

#if (RTE_DMA0)

/**
  \fn          int32_t DMA0_Initialize(void)
  \brief       Initialize DMA Interface.
  \return      \ref execution_status
*/
static int32_t DMA0_Initialize(void)
{
    DMA_RESOURCES *DMA = &DMA0;

    /* set the apb interface for accessing registers */
    DMA->ns_iface = DMA_GetSecureState(RTE_DMA0_APB_INTERFACE);
    if(DMA->ns_iface)
        DMA->regs = (DMA_Type*)DMA0_NS_BASE;
    else
        DMA->regs = (DMA_Type*)DMA0_SEC_BASE;

    return DMA_Initialize(DMA);
}

/**
  \fn          int32_t DMA0_Uninitialize(void)
  \brief       Un-Initialize DMA Interface.
  \return      \ref execution_status
*/
static int32_t DMA0_Uninitialize(void)
{
    return DMA_Uninitialize(&DMA0);
}

/**
  \fn          int32_t DMA0_PowerControl(ARM_POWER_STATE state)
  \brief       Control DMA0 Interface Power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
static int32_t DMA0_PowerControl(ARM_POWER_STATE state)
{
    return DMA_PowerControl(state, &DMA0);
}

/**
  \fn          int32_t DMA0_Allocate(DMA_Handle_Type *handle)
  \brief       Allocate Channel for transfer operation
  \param[in]   handle  Pointer to DMA handle
  \return      \ref execution_status
*/
static int32_t DMA0_Allocate(DMA_Handle_Type *handle)
{
    return DMA_Allocate(handle, &DMA0);
}

/**
  \fn          int32_t DMA0_Control(DMA_Handle_Type *handle,
                                    uint32_t         control,
                                    uint32_t         arg)
  \brief       Control DMA Interface.
  \param[in]   handle  Pointer to DMA handle
  \param[in]   control Operation
  \param[in]   arg     Argument 1 of operation
  \return      \ref execution_status and driver specific \ref dma exec status
*/
static int32_t DMA0_Control(DMA_Handle_Type *handle,
                            uint32_t         control,
                            uint32_t         arg)
{
    return DMA_Control(handle, control, arg, &DMA0);
}

/**
  \fn          int32_t DMA0_Start(int32_t *handle, ARM_DMA_PARAMS *params)
  \brief       Start DMA
  \param[in]   handle  Pointer to DMA handle
  \param[in]   params  Pointer to DMA desc parameters
  \return      \ref execution_status
*/
static int32_t DMA0_Start(DMA_Handle_Type *handle, ARM_DMA_PARAMS *params)
{
    return DMA_Start(handle, params, &DMA0);
}

/**
  \fn          int32_t DMA_Stop(int32_t *handle)
  \brief       Stop DMA
  \param[in]   handle  Pointer to DMA handle
  \return      \ref execution_status
*/
static int32_t DMA0_Stop(DMA_Handle_Type *handle)
{
    return DMA_Stop(handle, &DMA0);
}

/**
  \fn          DMA0_GetStatus(int32_t *handle, uint32_t *count)
  \brief       Status of a DMA handle
  \param[in]   handle  Pointer to DMA handle
  \param[in]   count  Pointer to pass transferred count
  \return      \ref execution_status
*/
static int32_t DMA0_GetStatus(DMA_Handle_Type *handle, uint32_t *count)
{
    return DMA_GetStatus(handle, count, &DMA0);
}

/**
  \fn          DMA0_DeAllocate(int32_t *handle)
  \brief       De-Allocate a DMA handle
  \param[in]   handle  Pointer to DMA handle
  \return      \ref execution_status
*/
static int32_t DMA0_DeAllocate(DMA_Handle_Type *handle)
{
    return DMA_DeAllocate(handle, &DMA0);
}

/**
  \fn          void  DMA0_IRQ0Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ0
*/
void DMA0_IRQ0Handler(void)
{
    DMA_IRQHandler(0, &DMA0);
}

/**
  \fn          void  DMA0_IRQ1Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ1
*/
void DMA0_IRQ1Handler(void)
{
    DMA_IRQHandler(1, &DMA0);
}

/**
  \fn          void  DMA0_IRQ2Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ2
*/
void DMA0_IRQ2Handler(void)
{
    DMA_IRQHandler(2, &DMA0);
}

/**
  \fn          void  DMA0_IRQ3Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ3
*/
void DMA0_IRQ3Handler(void)
{
    DMA_IRQHandler(3, &DMA0);
}

/**
  \fn          void  DMA0_IRQ4Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ4
*/
void DMA0_IRQ4Handler(void)
{
    DMA_IRQHandler(4, &DMA0);
}

/**
  \fn          void  DMA0_IRQ5Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ5
*/
void DMA0_IRQ5Handler(void)
{
    DMA_IRQHandler(5, &DMA0);
}

/**
  \fn          void  DMA0_IRQ6Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ6
*/
void DMA0_IRQ6Handler(void)
{
    DMA_IRQHandler(6, &DMA0);
}

/**
  \fn          void  DMA0_IRQ7Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ7
*/
void DMA0_IRQ7Handler(void)
{
    DMA_IRQHandler(7, &DMA0);
}

/**
  \fn          void  DMA0_IRQ8Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ8
*/
void DMA0_IRQ8Handler(void)
{
    DMA_IRQHandler(8, &DMA0);
}

/**
  \fn          void  DMA0_IRQ9Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ9
*/
void DMA0_IRQ9Handler(void)
{
    DMA_IRQHandler(9, &DMA0);
}

/**
  \fn          void  DMA0_IRQ10Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ10
*/
void DMA0_IRQ10Handler(void)
{
    DMA_IRQHandler(10, &DMA0);
}

/**
  \fn          void  DMA0_IRQ11Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ11
*/
void DMA0_IRQ11Handler(void)
{
    DMA_IRQHandler(11, &DMA0);
}

/**
  \fn          void  DMA0_IRQ12Handler (void)
  \brief       Run the IRQ Handler for DMA0-IRQ12
*/
void DMA0_IRQ12Handler(void)
{
    DMA_IRQHandler(12, &DMA0);
}

/**
  \fn          void  DMA0_IRQ13Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ13
*/
void DMA0_IRQ13Handler(void)
{
    DMA_IRQHandler(13, &DMA0);
}

/**
  \fn          void  DMA0_IRQ14Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ14
*/
void DMA0_IRQ14Handler(void)
{
    DMA_IRQHandler(14, &DMA0);
}

/**
  \fn          void  DMA0_IRQ15Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ15
*/
void DMA0_IRQ15Handler(void)
{
    DMA_IRQHandler(15, &DMA0);
}

/**
  \fn          void  DMA0_IRQ16Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ16
*/
void DMA0_IRQ16Handler(void)
{
    DMA_IRQHandler(16, &DMA0);
}

/**
  \fn          void  DMA0_IRQ17Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ17
*/
void DMA0_IRQ17Handler(void)
{
    DMA_IRQHandler(17, &DMA0);
}

/**
  \fn          void  DMA0_IRQ18Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ18
*/
void DMA0_IRQ18Handler(void)
{
    DMA_IRQHandler(18, &DMA0);
}

/**
  \fn          void  DMA0_IRQ19Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ19
*/
void DMA0_IRQ19Handler(void)
{
    DMA_IRQHandler(19, &DMA0);
}

/**
  \fn          void  DMA0_IRQ20Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ20
*/
void DMA0_IRQ20Handler(void)
{
    DMA_IRQHandler(20, &DMA0);
}

/**
  \fn          void  DMA0_IRQ21Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ21
*/
void DMA0_IRQ21Handler(void)
{
    DMA_IRQHandler(21, &DMA0);
}

/**
  \fn          void  DMA0_IRQ22Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ22
*/
void DMA0_IRQ22Handler(void)
{
    DMA_IRQHandler(22, &DMA0);
}

/**
  \fn          void  DMA0_IRQ23Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ23
*/
void DMA0_IRQ23Handler(void)
{
    DMA_IRQHandler(23, &DMA0);
}

/**
  \fn          void  DMA0_IRQ24Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ24
*/
void DMA0_IRQ24Handler(void)
{
    DMA_IRQHandler(24, &DMA0);
}

/**
  \fn          void  DMA0_IRQ25Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ25
*/
void DMA0_IRQ25Handler(void)
{
    DMA_IRQHandler(25, &DMA0);
}

/**
  \fn          void  DMA0_IRQ26Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ26
*/
void DMA0_IRQ26Handler(void)
{
    DMA_IRQHandler(26, &DMA0);
}

/**
  \fn          void  DMA0_IRQ27Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ27
*/
void DMA0_IRQ27Handler(void)
{
    DMA_IRQHandler(27, &DMA0);
}

/**
  \fn          void  DMA0_IRQ28Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ28
*/
void DMA0_IRQ28Handler(void)
{
    DMA_IRQHandler(28, &DMA0);
}

/**
  \fn          void  DMA0_IRQ29Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ29
*/
void DMA0_IRQ29Handler(void)
{
    DMA_IRQHandler(29, &DMA0);
}

/**
  \fn          void  DMA0_IRQ30Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ30
*/
void DMA0_IRQ30Handler(void)
{
    DMA_IRQHandler(30, &DMA0);
}

/**
  \fn          void  DMA0_IRQ31Handler(void)
  \brief       Run the IRQ Handler for DMA0-IRQ31
*/
void DMA0_IRQ31Handler(void)
{
    DMA_IRQHandler(31, &DMA0);
}

/**
  \fn          void  DMA0_IRQ_ABORT_Handler(void)
  \brief       Run the IRQ Handler for DMA0 Abort
*/
void DMA0_IRQ_ABORT_Handler(void)
{
    DMA_AbortIRQHandler(&DMA0);
}
/**
\brief Access structure of the DMA0 Driver.
*/
extern \
ARM_DRIVER_DMA Driver_DMA0;
ARM_DRIVER_DMA Driver_DMA0 = {
    DMA_GetVersion,
    DMA_GetCapabilities,
    DMA0_Initialize,
    DMA0_Uninitialize,
    DMA0_PowerControl,
    DMA0_Allocate,
    DMA0_Control,
    DMA0_Start,
    DMA0_Stop,
    DMA0_GetStatus,
    DMA0_DeAllocate
};
#endif

#if (RTE_DMALOCAL)

/**
  \fn          int32_t DMALOCAL_Initialize(void)
  \brief       Initialize Local DMA Interface.
  \return      \ref execution_status
*/
static int32_t DMALOCAL_Initialize(void)
{
    DMA_RESOURCES *DMA = &DMALOCAL;

    /* set the apb interface for accessing registers */
    DMA->ns_iface = DMA_GetSecureState(RTE_DMALOCAL_APB_INTERFACE);
    if(DMA->ns_iface)
        DMA->regs = (DMA_Type*)DMALOCAL_NS_BASE;
    else
        DMA->regs = (DMA_Type*)DMALOCAL_SEC_BASE;

    return DMA_Initialize(DMA);
}

/**
  \fn          int32_t DMALOCAL_Uninitialize(void)
  \brief       Un-Initialize Local DMA Interface.
  \return      \ref execution_status
*/
static int32_t DMALOCAL_Uninitialize(void)
{
    return DMA_Uninitialize(&DMALOCAL);
}


/**
  \fn          int32_t DMALOCAL_PowerControl(ARM_POWER_STATE state)
  \brief       Control Local DMA Interface Power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
static int32_t DMALOCAL_PowerControl(ARM_POWER_STATE state)
{
    return DMA_PowerControl(state, &DMALOCAL);
}

/**
  \fn          int32_t DMALOCAL_Allocate(DMA_Handle_Type *handle)
  \brief       Allocate Channel for transfer operation
  \param[in]   handle  Pointer to DMA Handle
  \return      \ref execution_status
*/
static int32_t DMALOCAL_Allocate(DMA_Handle_Type *handle)
{
    return DMA_Allocate(handle, &DMALOCAL);
}

/**
  \fn          int32_t DMALOCAL_Control (DMA_Handle_Type *handle,
                                         uint32_t         control,
                                         uint32_t         arg)
  \brief       Control Local DMA Interface.
  \param[in]   handle  Pointer to DMA Handle
  \param[in]   control Operation
  \param[in]   arg     Argument 1 of operation
  \return      \ref execution_status and driver specific \ref dma exec status
*/
static int32_t DMALOCAL_Control(DMA_Handle_Type *handle,
                                uint32_t         control,
                                uint32_t         arg)
{
    return DMA_Control(handle, control, arg, &DMALOCAL);
}

/**
  \fn          int32_t DMALOCAL_Start(int32_t *handle, ARM_DMA_PARAMS *params)
  \brief       Start a DMA channel
  \param[in]   handle  Pointer to DMA Handle
  \param[in]   params  Pointer to DMA desc parameters
  \return      \ref execution_status
*/
static int32_t DMALOCAL_Start(DMA_Handle_Type *handle, ARM_DMA_PARAMS *params)
{
    return DMA_Start(handle, params, &DMALOCAL);
}

/**
  \fn          int32_t DMA_Stop(int32_t *handle)
  \brief       Stop a DMA channel
  \param[in]   handle  Pointer to DMA Handle
  \return      \ref execution_status
*/
static int32_t DMALOCAL_Stop(DMA_Handle_Type *handle)
{
    return DMA_Stop(handle, &DMALOCAL);
}

/**
  \fn          DMALOCAL_GetStatus(int32_t *handle, uint32_t *count)
  \brief       Status of a DMA channel
  \param[in]   handle  Pointer to DMA Handle
  \param[in]   count  Pointer to pass transferred count
  \return      \ref execution_status
*/
static int32_t DMALOCAL_GetStatus(DMA_Handle_Type *handle, uint32_t *count)
{
    return DMA_GetStatus(handle, count, &DMALOCAL);
}

/**
  \fn          DMALOCAL_DeAllocate(int32_t *handle)
  \brief       De-Allocate a DMA channel
  \param[in]   handle  Pointer to Handle
  \return      \ref execution_status
*/
static int32_t DMALOCAL_DeAllocate(DMA_Handle_Type *handle)
{
    return DMA_DeAllocate(handle, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ0Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ0
*/
void DMALOCAL_IRQ0Handler(void)
{
    DMA_IRQHandler(0, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ1Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ1
*/
void DMALOCAL_IRQ1Handler(void)
{
    DMA_IRQHandler(1, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ2Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ2
*/
void DMALOCAL_IRQ2Handler(void)
{
    DMA_IRQHandler(2, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ3Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ3
*/
void DMALOCAL_IRQ3Handler(void)
{
    DMA_IRQHandler(3, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ4Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ4
*/
void DMALOCAL_IRQ4Handler(void)
{
    DMA_IRQHandler(4, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ5Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ5
*/
void DMALOCAL_IRQ5Handler(void)
{
    DMA_IRQHandler(5, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ6Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ6
*/
void DMALOCAL_IRQ6Handler(void)
{
    DMA_IRQHandler(6, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ7Handler (void)
  \brief       Run the IRQ Handler for Local DMA-IRQ7
*/
void DMALOCAL_IRQ7Handler(void)
{
    DMA_IRQHandler(7, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ8Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ8
*/
void DMALOCAL_IRQ8Handler(void)
{
    DMA_IRQHandler(8, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ9Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ9
*/
void DMALOCAL_IRQ9Handler(void)
{
    DMA_IRQHandler(9, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ10Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ10
*/
void DMALOCAL_IRQ10Handler(void)
{
    DMA_IRQHandler(10, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ11Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ11
*/
void DMALOCAL_IRQ11Handler(void)
{
    DMA_IRQHandler(11, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ12Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ12
*/
void DMALOCAL_IRQ12Handler(void)
{
    DMA_IRQHandler(12, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ13Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ13
*/
void DMALOCAL_IRQ13Handler(void)
{
    DMA_IRQHandler(13, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ14Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ14
*/
void DMALOCAL_IRQ14Handler(void)
{
    DMA_IRQHandler(14, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ15Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ15
*/
void DMALOCAL_IRQ15Handler(void)
{
    DMA_IRQHandler(15, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ16Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ16
*/
void DMALOCAL_IRQ16Handler(void)
{
    DMA_IRQHandler(16, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ17Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ17
*/
void DMALOCAL_IRQ17Handler(void)
{
    DMA_IRQHandler(17, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ18Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ18
*/
void DMALOCAL_IRQ18Handler(void)
{
    DMA_IRQHandler(18, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ19Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ19
*/
void DMALOCAL_IRQ19Handler(void)
{
    DMA_IRQHandler(19, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ20Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ20
*/
void DMALOCAL_IRQ20Handler(void)
{
    DMA_IRQHandler(20, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ21Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ21
*/
void DMALOCAL_IRQ21Handler(void)
{
    DMA_IRQHandler (21, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ22Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ22
*/
void DMALOCAL_IRQ22Handler(void)
{
    DMA_IRQHandler(22, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ23Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ23
*/
void DMALOCAL_IRQ23Handler(void)
{
    DMA_IRQHandler(23, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ24Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ24
*/
void DMALOCAL_IRQ24Handler(void)
{
    DMA_IRQHandler(24, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ25Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ25
*/
void DMALOCAL_IRQ25Handler(void)
{
    DMA_IRQHandler(25, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ26Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ26
*/
void DMALOCAL_IRQ26Handler(void)
{
    DMA_IRQHandler(26, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ27Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ27
*/
void DMALOCAL_IRQ27Handler(void)
{
    DMA_IRQHandler(27, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ28Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ28
*/
void DMALOCAL_IRQ28Handler(void)
{
    DMA_IRQHandler(28, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ29Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ29
*/
void DMALOCAL_IRQ29Handler(void)
{
    DMA_IRQHandler(29, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ30Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ30
*/
void DMALOCAL_IRQ30Handler(void)
{
    DMA_IRQHandler(30, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ31Handler(void)
  \brief       Run the IRQ Handler for Local DMA-IRQ31
*/
void DMALOCAL_IRQ31Handler(void)
{
    DMA_IRQHandler(31, &DMALOCAL);
}

/**
  \fn          void  DMALOCAL_IRQ_ABORT_Handler(void)
  \brief       Run the IRQ Handler for Local DMA Abort
*/
void DMALOCAL_IRQ_ABORT_Handler(void)
{
    DMA_AbortIRQHandler(&DMALOCAL);
}
/**
\brief Access structure of the LOCAL DMA Driver.
*/
extern \
ARM_DRIVER_DMA Driver_DMALOCAL;
ARM_DRIVER_DMA Driver_DMALOCAL = {
    DMA_GetVersion,
    DMA_GetCapabilities,
    DMALOCAL_Initialize,
    DMALOCAL_Uninitialize,
    DMALOCAL_PowerControl,
    DMALOCAL_Allocate,
    DMALOCAL_Control,
    DMALOCAL_Start,
    DMALOCAL_Stop,
    DMALOCAL_GetStatus,
    DMALOCAL_DeAllocate
};
#endif

/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/* System Includes */
#include <string.h>
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

/* Project Includes */
#include "Driver_I3C_Private.h"
#include "i3c.h"
#include "sys_ctrl_i3c.h"

#if !(RTE_I3C)
#error "I3C is not enabled in the RTE_Device.h"
#endif

#if (defined(RTE_Drivers_I3C) && !RTE_I3C)
#error "I3C not configured in RTE_Device.h!"
#endif

#define ARM_I3C_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(7, 3) /* driver version */

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion =
{
    ARM_I3C_API_VERSION,
    ARM_I3C_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_I3C_CAPABILITIES DriverCapabilities =
{
    1,          /* Supports legacy i2c device                                         */
    1,          /* Supports Slave I2C/I3C Adaptive mode support during boot up phase  */
    1,          /* Supports In-Band Interrupts                                        */
    0,          /* Doesn't support In-Band Interrupt with Payload                     */
    1,          /* Supports Secondary Master Configuration                            */
    1,          /* Supports HDR DDR0 mode                                             */
    0,          /* Doesn't support HDR Ternary Symbol Pure-Bus  mode                  */
    0,          /* Doesn't support HDR Ternary Symbol Legacy-Inclusive-Bus mode       */
    0           /* Reserved (must be zero)                                            */
};


#if I3C_DMA_ENABLE
/**
  \fn          int32_t I3C_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Initialize DMA for I3C
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
__STATIC_INLINE int32_t I3C_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Initializes DMA interface */
    status = dma_drv->Initialize();
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I3C_DMA_PowerControl(ARM_POWER_STATE state,
                                            DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       PowerControl DMA for I3C
  \param[in]   state  Power state
  \param[in]   dma_periph     Pointer to DMA resources
  \return      \ref execution_status
*/
__STATIC_INLINE int32_t I3C_DMA_PowerControl(ARM_POWER_STATE state,
                                             DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Initializes DMA interface */
    status = dma_drv->PowerControl(state);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I3C_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Allocate a channel for I3C
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
__STATIC_INLINE int32_t I3C_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Allocate handle for peripheral */
    status = dma_drv->Allocate(&dma_periph->dma_handle);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Enable the channel in the Event Router */
    if(dma_periph->evtrtr_cfg.instance == 0)
    {
        evtrtr0_enable_dma_channel(dma_periph->evtrtr_cfg.channel,
        						   dma_periph->evtrtr_cfg.group,
                                   DMA_ACK_COMPLETION_PERIPHERAL);
    }
    else
    {
        evtrtrlocal_enable_dma_channel(dma_periph->evtrtr_cfg.channel,
                                       DMA_ACK_COMPLETION_PERIPHERAL);
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I3C_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       De-allocate channel of I3C
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
__STATIC_INLINE int32_t I3C_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* De-Allocate handle  */
    status = dma_drv->DeAllocate(&dma_periph->dma_handle);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Disable the channel in the Event Router */
    if(dma_periph->evtrtr_cfg.instance == 0)
    {
        evtrtr0_disable_dma_channel(dma_periph->evtrtr_cfg.channel);
        evtrtr0_disable_dma_handshake(dma_periph->evtrtr_cfg.channel,
                                      dma_periph->evtrtr_cfg.group);
    }
    else
    {
        evtrtrlocal_disable_dma_channel(dma_periph->evtrtr_cfg.channel);
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I3C_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
                                     ARM_DMA_PARAMS *dma_params)
  \brief       Start I3C DMA transfer
  \param[in]   dma_periph     Pointer to DMA resources
  \param[in]   dma_params     Pointer to DMA parameters
  \return      \ref           execution_status
*/
__STATIC_INLINE int32_t I3C_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
                                      ARM_DMA_PARAMS *dma_params)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Start transfer */
    status = dma_drv->Start(&dma_periph->dma_handle, dma_params);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I3C_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Stop I3C DMA transfer
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
__STATIC_INLINE int32_t I3C_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Stop transfer */
    status = dma_drv->Stop(&dma_periph->dma_handle);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I3C_DMA_Start_TX(I3C_RESOURCES *i3c,
                                        const void    *src_addr,
                                        uint32_t       len)
  \brief       Start sending data to I3C TX FIFO using DMA
  \param[in]   i3c      : Pointer to i3c resources structure
  \param[in]   src_addr : Pointer to source address
  \param[in]   len      : number of bytes
  \return      execution_status
*/
static int32_t I3C_DMA_Start_TX(I3C_RESOURCES *i3c,
                                const void    *src_addr,
                                uint32_t       len)
{
    int32_t        status;
    ARM_DMA_PARAMS dma_params;

    /* Start the DMA engine for sending the data to I3C */
    dma_params.peri_reqno    = (int8_t)i3c->dma_cfg->dma_tx.dma_periph_req;
    dma_params.dir           = ARM_DMA_MEM_TO_DEV;
    dma_params.cb_event      = i3c->dma_cb;
    dma_params.src_addr      = src_addr;
    dma_params.dst_addr      = i3c_get_dma_tx_addr(i3c->regs);

    dma_params.num_bytes     = len;
    /* i3c TX/RX FIFO is 4-byte(word) aligned,
     *  if length is not 4-byte aligned(multiple of 4),
     *   make it aligned by adding extra length.
     */
    if(len % 4)
    {
        dma_params.num_bytes += (4 - (len % 4));
    }

    dma_params.irq_priority  = i3c->dma_irq_priority;

    /* i3c TX/RX FIFO is 4-byte(word) aligned */
    dma_params.burst_size = BS_BYTE_4;
    dma_params.burst_len  = i3c_get_tx_empty_buf_thld(i3c->regs);
    if( dma_params.burst_len > 16)
    {
        dma_params.burst_len = 16;
    }

    /* Start DMA transfer */
    status = I3C_DMA_Start(&i3c->dma_cfg->dma_tx, &dma_params);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I3C_DMA_Start_RX(I3C_RESOURCES *i3c,
                                        void          *dst_addr
                                        uint32_t       len)
  \brief       Start receiving data from I3C RX FIFO using DMA
  \param[in]   i3c      : Pointer to i3c resources structure
  \param[in]   dst_addr : Pointer to destination address
  \param[in]   len      : number of bytes
  \return      execution_status
*/
static int32_t I3C_DMA_Start_RX(I3C_RESOURCES *i3c,
                                void          *dst_addr,
                                uint32_t       len)
{
    ARM_DMA_PARAMS dma_params;
    int32_t        status;

    /* Start the DMA engine for sending the data to i3c */
    dma_params.peri_reqno    = (int8_t)i3c->dma_cfg->dma_rx.dma_periph_req;
    dma_params.dir           = ARM_DMA_DEV_TO_MEM;
    dma_params.cb_event      = i3c->dma_cb;
    dma_params.src_addr      = i3c_get_dma_rx_addr(i3c->regs);
    dma_params.dst_addr      = dst_addr;

    dma_params.num_bytes     = len;
    /* i3c TX/RX FIFO is 4-byte(word) aligned,
     *  if length is not 4-byte aligned(multiple of 4),
     *   make it aligned by adding extra length.
     */
    if(len % 4)
    {
        dma_params.num_bytes += (4 - (len % 4));
    }

    dma_params.irq_priority  = i3c->dma_irq_priority;

    /* i3c TX/RX FIFO is 4-byte(word) aligned */
    dma_params.burst_size = BS_BYTE_4;
    dma_params.burst_len  = i3c_get_rx_buf_thld(i3c->regs);
    if( dma_params.burst_len > 16)
    {
        dma_params.burst_len = 16;
    }

    /* Start DMA transfer */
    status = I3C_DMA_Start(&i3c->dma_cfg->dma_rx, &dma_params);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}
#endif /* I3C_DMA_ENABLE */


/**
  \fn           int I3cMasterGetAddrPos(I3C_RESOURCES *i3c, uint8_t addr)
  \brief        Get already assigned slave address position from DAT(Device Address Table).
                For i3c, dynamic address and for i2c, static address is used for communication.
  \param[in]    i3c     : Pointer to i3c resources structure
  \param[in]    addr    : Assigned Slave Address;
                           Dynamic Address for i3c, Static Address for i2c slave device
  \return       Assigned slave address position from DAT(Device Address Table) index OR
                ARM_DRIVER_ERROR in case slave is not already assigned.
*/
static int I3cMasterGetAddrPos(I3C_RESOURCES *i3c, uint8_t addr)
{
    uint32_t pos;

    for (pos = 0; pos < i3c->slave_dat.maxdevs; pos++)
    {
        if (addr == (i3c->slave_dat.addrs[pos] &
                    (~I3C_TARGET_SLAVE_TYPE_I2C)))
            return pos;
    }

  return ARM_DRIVER_ERROR;
}

/**
  \fn           int I3cMasterGetFreePos(I3C_RESOURCES *i3c)
  \brief        Get free position from DAT(Device Address Table)
  \param[in]    i3c     : Pointer to i3c resources structure
  \return       Free position from DAT OR
                ARM_DRIVER_ERROR in case DAT is Full.
                Maximum 8 Slave Devices are supported
                (\ref register DEVICE_ADDR_TABLE_POINTER)
*/
static int I3cMasterGetFreePos(I3C_RESOURCES *i3c)
{
    uint32_t i;

    if (!(i3c->slave_dat.freepos & GENMASK(i3c->slave_dat.maxdevs - 1, 0)))
        return ARM_DRIVER_ERROR;

    for (i = 0; i < i3c->slave_dat.maxdevs; i++)
    {
        if (i3c->slave_dat.freepos & (1 << i))
            return i;
    }

    return ARM_DRIVER_ERROR;
}

/**
  \fn           int I3Cx_GetSlaveDynamicAddr(I3C_RESOURCES *i3c,
                                             uint8_t* addr_list,
                                             uint8_t count)
  \brief        Fetches the slave's dynamic address from DAT
  \             given the static address
  \param[in]    i3c           : Pointer to i3c resources structure
  \param[in]    static_addr   : Slave's static address
  \param[in]    dynamic_addr  : Slave's dynamic address
  \return       \ref execution_status
*/
static int I3Cx_GetSlaveDynamicAddr(I3C_RESOURCES *i3c,
                                    uint8_t static_addr,
                                    uint8_t *dynamic_addr)
{
    if(!dynamic_addr)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    *dynamic_addr = i3c_get_slv_dyn_addr(i3c->regs, static_addr);

    /* Returns error if the requested address not found */
    if(!(*dynamic_addr))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int I3Cx_GetSlvsInfo(I3C_RESOURCES *i3c,
                                     ARM_I3C_DEV_CHAR* data,
                                     const uint8_t value)
  \             Note: It is valid only for secondary masters, not for
  \                   current bus master
  \brief        Fetches the slaves info from DCT
  \param[in]    i3c       : Pointer to i3c resources structure
  \param[in]    data      : Device characteristics data
  \param[in]    count     : Slaves count
  \return       \ref execution_status
*/
static int I3Cx_GetSlvsInfo(I3C_RESOURCES *i3c,
                            void* data,
                            const uint8_t value)
{
    uint8_t iter;

    if(!data)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if(i3c->state.is_master)
    {
        /* Performs the following if it's a master */
        /* Considering value as the slave address.
         * Returns error if the slave address is present in table */
        if(I3cMasterGetAddrPos(i3c, value) < 0)
        {
            return ARM_DRIVER_ERROR_PARAMETER;
        }

        ARM_I3C_DEV_PRIME_INFO *prime_info = (ARM_I3C_DEV_PRIME_INFO*)data;

        i3c_dev_prime_info_t p_info;

        /* Gets slave characteristics */
        i3c_master_get_dct(i3c->regs, &p_info, value);

        prime_info->dev_char.static_addr   = p_info.dev_char.static_addr;
        prime_info->dev_char.bcr           = p_info.dev_char.bcr;
        prime_info->dev_char.dcr           = p_info.dev_char.dcr;
        prime_info->dev_char.dynamic_addr  = p_info.dev_char.dynamic_addr;

        prime_info->pid.dcr                = p_info.pid.dcr;
        prime_info->pid.inst_id            = p_info.pid.inst_id;
        prime_info->pid.part_id            = p_info.pid.part_id;
        prime_info->pid.pid_sel            = p_info.pid.pid_sel;
        prime_info->pid.mipi_mfg_id        = p_info.pid.mipi_mfg_id;

    }
    else
    {
        /* Performs the following if it's a master */

        /* Considering value as the count of slaves.
         * Returns error if the slave count is more than
         * max supported slaves */
        if(value > i3c->slave_dat.maxdevs)
        {
            return ARM_DRIVER_ERROR_PARAMETER;
        }

        ARM_I3C_DEV_CHAR *char_info = (ARM_I3C_DEV_CHAR*)data;

        /* Create device characteristics array of specified length */
        i3c_dev_char_t dev_chr[I3C_MAX_DEVS];

        /* Secondary master fetches slaves' characteristics */
        i3c_sec_master_get_dct(i3c->regs, dev_chr, value);

        /* Store the Device characteristics info */
        for(iter = 0U; iter < value; iter++)
        {
            char_info->static_addr  = dev_chr[iter].static_addr;
            char_info->bcr          = dev_chr[iter].bcr;
            char_info->dcr          = dev_chr[iter].dcr;
            char_info->dynamic_addr = dev_chr[iter].dynamic_addr;

            char_info++;
        }
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int I3Cx_GetSlaveList(I3C_RESOURCES *i3c,
                                      uint8_t* addr_list,
                                      uint8_t count)
  \brief        Fetches the slaves list from address table
  \param[in]    i3c       : Pointer to i3c resources structure
  \param[in]    addr_list : Slave address list
  \param[in]    count     : Slaves count
  \return       \ref execution_status
*/
static int I3Cx_GetSlaveList(I3C_RESOURCES *i3c,
                             uint8_t* addr_list,
                             uint8_t* count)
{
    uint8_t pos = 0U;

    if((!addr_list) || (!count))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    (*count) = 0U;
    /* Fetches all valid slave addresses*/
    for (pos = 0; pos < i3c->slave_dat.maxdevs; pos++)
    {
        if (!(i3c->slave_dat.freepos & (1 << pos)))
        {
            *addr_list = (i3c->slave_dat.addrs[pos] &
                         (~I3C_TARGET_SLAVE_TYPE_I2C));
            addr_list++;
            (*count)++;
        }
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           static void I3C_DetachSlaves(I3C_RESOURCES *i3c,
                                             uint8_t slv_cnt)
  \brief        Detach already attached i3c devices from the i3c bus.
  \param[in]    i3c      : Pointer to i3c resources structure
  \param[in]    slv_cnt  : Slaves count
  \param[in]    ccc_cmd  : CCC command
  \return       None
*/
static void I3C_DetachSlaves(I3C_RESOURCES *i3c,
                             uint8_t slv_cnt,
                             uint32_t ccc_cmd)
{
    int8_t pos = 0;

    if((ccc_cmd == I3C_CCC_ENTDAA)   ||
       (ccc_cmd == I3C_CCC_SETDASA))
    {
        /* Perform the following when ENTDAA or SETDASA failed */
        for(pos = i3c->slave_dat.maxdevs; ((pos > 0) && (slv_cnt)); pos--)
        {
            if(i3c->slave_dat.last_asgd_addr_pos & (1U << (pos - 1U)))
            {
                /* free the index */
                i3c->slave_dat.freepos          |= (BIT(pos - 1U));
                i3c->slave_dat.addrs[(pos - 1U)] = 0U;
                /* clear the DAT index pos */
                i3c_remove_slv_from_dat(i3c->regs, (pos - 1U));

                slv_cnt--;
            }
        }
    }
    else if(ccc_cmd == I3C_CCC_RSTDAA(false))
    {
        /* Perform the following when Direct RSTDAA is successful */
        pos = I3cMasterGetAddrPos(i3c,
              (i3c->slave_dat.addrs[i3c->xfer.xfer_cmd.addr_index] &
              (~I3C_TARGET_SLAVE_TYPE_I2C)));
        if(pos >= 0)
        {
            i3c->slave_dat.freepos   |= (BIT(pos));
            i3c->slave_dat.addrs[pos] = 0U;
            i3c_remove_slv_from_dat(i3c->regs, pos);
        }

    }
    else if(ccc_cmd == I3C_CCC_RSTDAA(true))
    {
        /* Perform the following when broadcast RSTDAA is successful */
        for(pos = 0; pos < ((int8_t)i3c->slave_dat.maxdevs); pos++)
        {
            /* Detaches only I3C slaves */
            if((!(i3c->slave_dat.freepos & (1U << pos))) &&
              (!(i3c->slave_dat.addrs[pos] & I3C_TARGET_SLAVE_TYPE_I2C)))
            {
                i3c_remove_slv_from_dat(i3c->regs, pos);
                i3c->slave_dat.freepos   |= (BIT(pos));
                i3c->slave_dat.addrs[pos] = 0U;
            }
        }
    }
    else if(ccc_cmd == I3C_CCC_SETNEWDA)
    {
        /* Perform the following when Direct SETNEWDA is successful */
        pos = I3cMasterGetAddrPos(i3c,
              (i3c->slave_dat.addrs[i3c->xfer.xfer_cmd.addr_index] &
              (~I3C_TARGET_SLAVE_TYPE_I2C)));
        if(pos >= 0)
        {
            /* Update the slave address with new one */
            i3c->slave_dat.addrs[pos] = (*((uint8_t*)i3c->xfer.tx_buf) >> 1U);
            i3c_update_slv_addr_in_dat(i3c->regs,
                                       pos,
                                       (*((uint8_t*)i3c->xfer.tx_buf) >> 1U));
        }
    }
}

/**
  \fn           int I3C_AddDynamicAddrParity(uint8_t *dyn_addr)
  \brief        Generates and adds the parity to dynamic address
  \param[in]    dyn_addr : Dynamic address
  \return       None
*/
static void I3C_AddDynamicAddrParity(uint8_t *dyn_addr)
{
    uint8_t bit_iter    = 0U;
    uint8_t xor_value   = 0U;

    /* XOR the 1st 7 bits of dynamic address*/
    xor_value = ((*dyn_addr) & (1U << 0U));

    for(bit_iter = 1U; bit_iter < 7U; bit_iter++)
    {
        xor_value ^= (((*dyn_addr) >> bit_iter) & 1U);
    }

    /* Assign the negated XOR value to 7th
     * bit of dynamic address */
    *dyn_addr |= ((~xor_value) << 7U);
}

/**
  \fn           ARM_DRIVER_VERSION I3C_GetVersion(void)
  \brief        Get i3c driver version
  \return       i3c driver version
*/
static ARM_DRIVER_VERSION I3C_GetVersion(void)
{
    return DriverVersion;
}

/**
  \fn           ARM_I3C_CAPABILITIES I3C_GetCapabilities(void)
  \brief        Get i3c driver capabilities
  \return       i3c driver capabilities
*/
static ARM_I3C_CAPABILITIES I3C_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
  \fn           ARM_I3C_STATUS I3Cx_GetStatus(I3C_RESOURCES *i3c)
  \brief        Get i3c driver status
  \return       i3c driver status
*/
static ARM_I3C_STATUS I3Cx_GetStatus(I3C_RESOURCES *i3c)
{
    /* Returns Current I3C driver status */
    i3c->status.mode         = i3c->state.is_master;
    i3c->status.ibi_slv_addr = i3c->xfer.addr;

    /* Fetch the last error found */
    switch(i3c->xfer.error)
    {
        case I3C_COMM_ERROR_CRC:
            i3c->status.last_error_code = ARM_I3C_LEC_CRC_ERROR;
            break;
        case I3C_COMM_ERROR_PARITY:
            i3c->status.last_error_code = ARM_I3C_LEC_PARITY_ERROR;
            break;
        case I3C_COMM_ERROR_FRAME:
            i3c->status.last_error_code = ARM_I3C_LEC_FRAME_ERROR;
            break;
        case I3C_COMM_ERROR_IBA_NACK:
            i3c->status.last_error_code = ARM_I3C_LEC_IBA_NACK_ERROR;
            break;
        case I3C_COMM_ERROR_ADDR_NACK:
            i3c->status.last_error_code = ARM_I3C_LEC_ADDR_NACK_ERROR;
            break;
        case I3C_COMM_ERROR_BUF_UNDR_OVR_FLW:
            i3c->status.last_error_code = ARM_I3C_LEC_BUF_UNDER_OVERFLOW_ERROR;
            break;
        case I3C_COMM_ERROR_XFER_ABORT:
            i3c->status.last_error_code = ARM_I3C_LEC_TRANSFER_ABORT_ERROR;
            break;
        case I3C_COMM_ERROR_I2C_SLV_W_NACK:
            i3c->status.last_error_code = ARM_I3C_LEC_I2C_SLAVE_NACK_ERROR;
            break;
        case I3C_COMM_ERROR_PEC_OR_EARLY_TERM:
            if(i3c->state.is_master)
            {
                i3c->status.last_error_code = ARM_I3C_LEC_PEC_BYTE_ERROR;
            }
            else
            {
                i3c->status.last_error_code = ARM_I3C_LEC_EARLY_TERMINATION_ERROR;
            }
            break;
        default:
            i3c->status.last_error_code = ARM_I3C_LEC_NO_ERROR;
            break;
    }

    return i3c->status;
}

/**
  \fn           ARM_I3C_DEVICE_INFO I3Cx_GetDeviceInfo(I3C_RESOURCES *i3c)
  \brief        Get i3c device information
  \return       i3c driver info
*/
static ARM_I3C_DEVICE_INFO I3Cx_GetDeviceInfo(I3C_RESOURCES *i3c)
{
    ARM_I3C_DEVICE_INFO dev_info;
    i3c_slave_pid_t     slv_pid;

    /* Fetch the device information from low level and return */
    dev_info.prime_info.dev_char.static_addr  = i3c_get_static_addr(i3c->regs);
    dev_info.prime_info.dev_char.dynamic_addr = i3c_get_dynamic_addr(i3c->regs);
    dev_info.prime_info.dev_char.dcr          = i3c_get_dcr(i3c->regs);
    dev_info.prime_info.dev_char.bcr          = i3c_get_bcr(i3c->regs);

    if(i3c->state.is_master)
    {
        /* Fetch master specific information */
        dev_info.max_read_len           = I3C_MAX_DATA_BUF_SIZE;
        dev_info.max_write_len          = I3C_MAX_DATA_BUF_SIZE;
        dev_info.max_read_speed         = 0U;
        dev_info.max_write_speed        = 0U;
        dev_info.max_read_turnaround    = 0U;
        memset(&dev_info.prime_info.pid, 0, sizeof(ARM_I3C_SLV_PID));
    }
    else
    {
        /* Fetch slave specific information */
        dev_info.max_read_len           = i3c_slave_get_max_read_len(i3c->regs);
        dev_info.max_write_len          = i3c_slave_get_max_write_len(i3c->regs);
        dev_info.max_read_speed         = i3c_slave_get_max_read_speed(i3c->regs);
        dev_info.max_write_speed        = i3c_slave_get_max_write_speed(i3c->regs);
        dev_info.max_read_turnaround    = i3c_slave_get_max_read_turn(i3c->regs);

        slv_pid = i3c_slave_get_pid(i3c->regs);

        dev_info.prime_info.pid.dcr         = slv_pid.dcr;
        dev_info.prime_info.pid.inst_id     = slv_pid.inst_id;
        dev_info.prime_info.pid.part_id     = slv_pid.part_id;
        dev_info.prime_info.pid.pid_sel     = slv_pid.pid_sel;
        dev_info.prime_info.pid.mipi_mfg_id = slv_pid.mipi_mfg_id;
    }

    return dev_info;
}

/**
  \fn           int I3Cx_MasterSendCommand(I3C_RESOURCES *i3c, ARM_I3C_CMD *ccc)
  \brief        Send an I3C command to the slave
  \param[in]    i3c      : Pointer to i3c resources structure
  \param[in]    ccc      : Pointer to i3c command structure
  \return       \ref execution_status
*/
static int I3Cx_MasterSendCommand(I3C_RESOURCES *i3c, ARM_I3C_CMD *ccc)
{
    int32_t index;

    if (i3c->state.powered == 0U)
        return ARM_DRIVER_ERROR;

    if (i3c->status.busy)
        return ARM_DRIVER_ERROR_BUSY;

    if (!ccc)
        return ARM_DRIVER_ERROR_PARAMETER;

    if(ccc->len > I3C_MAX_DATA_BUF_SIZE)
        return ARM_DRIVER_ERROR_PARAMETER;

    index = I3cMasterGetAddrPos(i3c, ccc->addr);
    if (index < 0)
    {
        /* Returns error if it is a Direct CCC
         * and no such slave present.*/
        if(ccc->cmd_id & I3C_CCC_DIRECT)
        {
            return ARM_DRIVER_ERROR;
        }
    }

    i3c->status.busy = 1;
    i3c->xfer.error  = 0U;

    if (ccc->rw) /* command read */
    {
        i3c->xfer.tx_buf              = NULL;
        i3c->xfer.tx_len              = 0U;
        i3c->xfer.rx_buf              = ccc->data;
        i3c->xfer.rx_len              = ccc->len;
        i3c->xfer.xfer_cmd.cmd_type   = I3C_XFER_CCC_GET;
        i3c->xfer.xfer_cmd.cmd_id     = ccc->cmd_id;
        i3c->xfer.xfer_cmd.addr_index = index;
        i3c->xfer.xfer_cmd.addr_depth = 1U;
        i3c->xfer.xfer_cmd.def_byte   = ccc->def_byte;
        i3c->xfer.xfer_cmd.data_len   = ccc->len;

#if RTE_I3C_BLOCKING_MODE_ENABLE
        if(i3c->blocking_mode)
        {
            /* Invoke xfer cmd blocking api */
            i3c_send_xfer_cmd_blocking(i3c->regs, &i3c->xfer);
            i3c->status.busy = 0U;

            if(!(i3c->xfer.status & I3C_XFER_STATUS_DONE))
            {
                /* Resume the device if error occurs */
                i3c_resume(i3c->regs);
                return ARM_DRIVER_ERROR;
            }
        }
        else
#endif
        {
            /* Invoke xfer cmd api */
            i3c_send_xfer_cmd(i3c->regs, &(i3c->xfer));
        }
    }    /* command read  */
    else /* command write */
    {
        if(ccc->cmd_id == I3C_CCC_ENTHDR(0U))
        {
            /* Set speed to HDR-DDR speed */
            i3c->xfer.xfer_cmd.speed      = I3C_SPEED_HDR_DDR;
            i3c->status.busy              = 0;
        }
        else if((ccc->cmd_id > I3C_CCC_ENTHDR(0U)) &&
                (ccc->cmd_id <= I3C_CCC_ENTHDR(7U)))
        {
            /* Return unsupported if other HDR-DDR speeds requested */
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }
        else
        {
            i3c->xfer.rx_buf              = NULL;
            i3c->xfer.rx_len              = 0U;
            i3c->xfer.tx_buf              = ccc->data;
            i3c->xfer.tx_len              = ccc->len;
            i3c->xfer.xfer_cmd.cmd_type   = I3C_XFER_CCC_SET;
            i3c->xfer.xfer_cmd.cmd_id     = ccc->cmd_id;
            i3c->xfer.xfer_cmd.addr_index = index;
            i3c->xfer.xfer_cmd.addr_depth = 1U;
            i3c->xfer.xfer_cmd.def_byte   = ccc->def_byte;
            i3c->xfer.xfer_cmd.data_len   = ccc->len;

#if RTE_I3C_BLOCKING_MODE_ENABLE
            if(i3c->blocking_mode)
            {
                /* Invoke xfer cmd blocking api */
                i3c_send_xfer_cmd_blocking(i3c->regs, &i3c->xfer);
                i3c->status.busy = 0U;
                if(i3c->xfer.status != I3C_XFER_STATUS_DONE)
                {
                    /* Resume the device if error occurs */
                    i3c_resume(i3c->regs);
                    return ARM_DRIVER_ERROR;
                }
                else
                {
                    /* If RSTDAA or SETNEWDA command is successful
                     * then invoke detach slave fuction to remove slaves' info */
                    if((i3c->xfer.xfer_cmd.cmd_id == I3C_CCC_RSTDAA(true))   ||
                       (i3c->xfer.xfer_cmd.cmd_id == I3C_CCC_RSTDAA(false))  ||
                       (i3c->xfer.xfer_cmd.cmd_id == I3C_CCC_SETNEWDA))
                    {
                        I3C_DetachSlaves(i3c,
                                         i3c->xfer.tx_len,
                                         i3c->xfer.xfer_cmd.cmd_id);
                    }
                }
            }
            else
#endif
            {
                /* Invoke xfer cmd api */
                i3c_send_xfer_cmd(i3c->regs, &i3c->xfer);
            }
        }
    }

    return ARM_DRIVER_OK;
}


/*
 * Note for I3C DMA:
 * For proper Master and Slave communication,
 *  There should be fix protocol between Master and Slave,
 *  in which both should be knowing well in advanced that
 *  how much data is going to transmit/receive from both the sides.
 *  (currently different-different transfer data length from
 *   Master and Slave is not supported.)
 */

/**
  \fn           int I3Cx_MasterTransmit(I3C_RESOURCES *i3c,  uint8_t  addr,
                                        const uint8_t *data, uint16_t len)
  \brief        Write data to the slave
  \param[in]    i3c      : Pointer to i3c resources structure
  \param[in]    addr     : Assigned Slave Address;
                            Dynamic Address for i3c, Static Address for i2c slave device
  \param[in]    data     : Pointer to buffer with data which needs to be transmit to slave
  \param[in]    len      : Number of bytes needs to be transmit
  \return       \ref execution_status
*/
static int I3Cx_MasterTransmit(I3C_RESOURCES *i3c,  uint8_t  addr,
                               const uint8_t *data, uint16_t len)
{
    int32_t index;

#if I3C_DMA_ENABLE
    int32_t ret;
#endif

    if (i3c->state.powered == 0U)
        return ARM_DRIVER_ERROR;

    /* Checking for Master initialization */
    if ((!i3c->state.enabled) ||
        (!i3c->state.is_master))
        return ARM_DRIVER_ERROR;

    if (!data || !len)
        return ARM_DRIVER_ERROR_PARAMETER;

    if(len > I3C_MAX_DATA_BUF_SIZE)
        return ARM_DRIVER_ERROR_PARAMETER;

    if (i3c->status.busy)
        return ARM_DRIVER_ERROR_BUSY;

    index = I3cMasterGetAddrPos(i3c, addr);
    if (index < 0)
        return ARM_DRIVER_ERROR_PARAMETER;

    i3c->status.busy              = 1;
    i3c->xfer.error               = 0U;
    i3c->xfer.rx_buf              = NULL;
    i3c->xfer.rx_len              = 0U;
    i3c->xfer.xfer_cmd.addr_index = index;
    i3c->xfer.xfer_cmd.data_len   = len;

#if (!I3C_DMA_ENABLE) /* update only if DMA disable */
    i3c->xfer.tx_buf              = data;
    i3c->xfer.tx_len              = len;
#else
    i3c->xfer.tx_buf              = NULL;
    i3c->xfer.tx_len              = 0U;
#endif

#if RTE_I3C_BLOCKING_MODE_ENABLE
    if(i3c->blocking_mode)
    {
        /* Invoke master send blocking api */
        i3c_master_tx_blocking(i3c->regs, &i3c->xfer);
        i3c->status.busy         = 0U;
        /* Sets the speed to SDR Maximum */
        i3c->xfer.xfer_cmd.speed = I3C_SPEED_SDR0;

        if(!(i3c->xfer.status & I3C_XFER_STATUS_MST_TX_DONE))
        {
            /* Resume the device if error occurs */
            i3c_resume(i3c->regs);
            return ARM_DRIVER_ERROR;
        }
    }
    else
#endif
    {
        /* Invoke master send api */
        i3c_master_tx(i3c->regs, &i3c->xfer);
    }

#if I3C_DMA_ENABLE
    ret = I3C_DMA_Start_TX(i3c, data, len);
    if(ret)
    {
        return ARM_DRIVER_ERROR;
    }
#endif

    return ARM_DRIVER_OK;
}

/**
  \fn           int I3Cx_MasterReceive(I3C_RESOURCES *i3c,  uint8_t  addr,
                                             uint8_t *data, uint16_t len)
  \brief        Read data from the slave
  \param[in]    i3c      : Pointer to i3c resources structure
  \param[in]    addr     : Assigned Slave Address;
                           Dynamic Address for i3c, Static Address for i2c slave device
  \param[in]    data     : Pointer to buffer for data to receive from slave
  \param[in]    len      : Number of bytes needs to be receive
  \return       \ref execution_status
*/
static int I3Cx_MasterReceive(I3C_RESOURCES *i3c,  uint8_t  addr,
                                    uint8_t *data, uint16_t len)
{
    int32_t index;

#if I3C_DMA_ENABLE
    int32_t ret;
#endif

    if (i3c->state.powered == 0U)
        return ARM_DRIVER_ERROR;

    /* Checking for Master initialization */
    if ((!i3c->state.enabled) ||
        (!i3c->state.is_master))
        return ARM_DRIVER_ERROR;

    if (!data || !len)
        return ARM_DRIVER_ERROR_PARAMETER;

    if(len > I3C_MAX_DATA_BUF_SIZE)
        return ARM_DRIVER_ERROR_PARAMETER;

    if (i3c->status.busy)
        return ARM_DRIVER_ERROR_BUSY;

    index = I3cMasterGetAddrPos(i3c, addr);
    if (index < 0)
        return ARM_DRIVER_ERROR_PARAMETER;

    i3c->status.busy              = 1;
    i3c->xfer.error               = 0U;
    i3c->xfer.tx_buf              = NULL;
    i3c->xfer.tx_len              = 0U;
    i3c->xfer.xfer_cmd.addr_index = index;
    i3c->xfer.xfer_cmd.data_len   = len;
    i3c->xfer.rx_len              = len;

#if (!I3C_DMA_ENABLE) /* update only if DMA disable */
    i3c->xfer.rx_buf              = data;
#else
    i3c->xfer.rx_buf              = NULL;
#endif

#if RTE_I3C_BLOCKING_MODE_ENABLE
    if(i3c->blocking_mode)
    {
        /* Invoke master receive blocking api */
        i3c_master_rx_blocking(i3c->regs, &i3c->xfer);
        i3c->status.busy         = 0U;
        /* Sets the speed to SDR Maximum */
        i3c->xfer.xfer_cmd.speed = I3C_SPEED_SDR0;

        if(!(i3c->xfer.status & I3C_XFER_STATUS_MST_RX_DONE))
        {
            /* Resume the device if error occurs */
            i3c_resume(i3c->regs);
            return ARM_DRIVER_ERROR;
        }
    }
    else
#endif
    {
        /* Invoke master receive api */
        i3c_master_rx(i3c->regs, &i3c->xfer);
    }

#if I3C_DMA_ENABLE
    ret = I3C_DMA_Start_RX(i3c, data, len);
    if(ret)
    {
        return ARM_DRIVER_ERROR;
    }
#endif

    return ARM_DRIVER_OK;
}

/**
  \fn           int I3Cx_SlaveTransmit(I3C_RESOURCES *i3c,
                                       const uint8_t *data,
                                       uint16_t       len)
  \brief        Write data to the master
  \param[in]    i3c      : Pointer to i3c resources structure
  \param[in]    data     : Pointer to buffer with data
                            which needs to be transmit to master
  \param[in]    len      : Number of bytes needs to be transmit
  \return       \ref execution_status
*/
static int I3Cx_SlaveTransmit(I3C_RESOURCES *i3c,
                              const uint8_t *data,
                              uint16_t       len)
{
#if I3C_DMA_ENABLE
    int32_t ret;
#endif

    /* Checking for power done initialization */
    if (i3c->state.powered == 0U)
        return ARM_DRIVER_ERROR;

    /* Checking for slave initialization */
    if ((!i3c->state.enabled) ||
        (i3c->state.is_master))
        return ARM_DRIVER_ERROR;

    /* Parameter check */
    if (!data || !len)
        return ARM_DRIVER_ERROR_PARAMETER;

    if((len > i3c_slave_get_max_write_len(i3c->regs)) ||
       (len > I3C_MAX_DATA_BUF_SIZE))
    {
        /* Return error if required legth is greater than
         * Master Set Max Write Length or I3C_MAX_DATA_BUF_SIZE */
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (i3c->status.busy)
        return ARM_DRIVER_ERROR_BUSY;

    i3c->status.busy            = 1;
    i3c->xfer.error             = 0U;
    i3c->xfer.rx_buf            = NULL;
    i3c->xfer.rx_len            = 0U;
    i3c->xfer.xfer_cmd.data_len = len;
    i3c->xfer.tx_len            = len;

#if (!I3C_DMA_ENABLE) /* update only if DMA disable */
    i3c->xfer.tx_buf            = data;
#else
    i3c->xfer.tx_buf            = NULL;
#endif

#if RTE_I3C_BLOCKING_MODE_ENABLE
    if(i3c->blocking_mode)
    {
        /* Invoke slave transmit blocking api */
        i3c_slave_tx_blocking(i3c->regs, &i3c->xfer);
        i3c->status.busy = 0U;

        if(!(i3c->xfer.status & I3C_XFER_STATUS_SLV_TX_DONE))
        {
            /* Resume the device if error occurs */
            i3c_resume(i3c->regs);
            return ARM_DRIVER_ERROR;
        }
    }
    else
#endif
    {
        /* Invoke slave transmit api */
        i3c_slave_tx(i3c->regs, &i3c->xfer);
    }

#if I3C_DMA_ENABLE
    ret = I3C_DMA_Start_TX(i3c, data, len);
    if(ret)
    {
        return ARM_DRIVER_ERROR;
    }
#endif

    return ARM_DRIVER_OK;
}

/**
  \fn           int I3Cx_SlaveReceive(I3C_RESOURCES *i3c,
                                      uint8_t       *data,
                                      uint32_t       len)
  \brief        Read data from the master
  \param[in]    i3c      : Pointer to i3c resources structure
  \param[in]    data     : Pointer to buffer for data
                            to receive from master
  \param[in]    len      : Number of bytes needs to be receive
  \return       \ref execution_status
*/
static int I3Cx_SlaveReceive(I3C_RESOURCES *i3c,
                             uint8_t       *data,
                             uint32_t       len)
{
#if I3C_DMA_ENABLE
    int32_t ret;
#endif

    /* Checking for power done initialization */
    if (i3c->state.powered == 0U)
        return ARM_DRIVER_ERROR;

    /* Checking for slave initialization */
    if ((!i3c->state.enabled) ||
        (i3c->state.is_master))
        return ARM_DRIVER_ERROR;

    /* Parameter check */
    if (!data || !len)
        return ARM_DRIVER_ERROR_PARAMETER;

    if((len > i3c_slave_get_max_read_len(i3c->regs)) ||
       (len > I3C_MAX_DATA_BUF_SIZE))
    {
        /* Return error if required legth is greater than
         * Master Set Max Read Length or I3C_MAX_DATA_BUF_SIZE */
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (i3c->status.busy)
        return ARM_DRIVER_ERROR_BUSY;

    i3c->status.busy = 1;
    i3c->xfer.error  = 0U;
    i3c->xfer.tx_buf = NULL;
    i3c->xfer.tx_len = 0;
    i3c->xfer.rx_len = len;

#if (!I3C_DMA_ENABLE) /* update only if DMA disable */
    /* Buffer initialization for TX/RX */
    i3c->xfer.rx_buf = data;
#else
    i3c->xfer.rx_buf = NULL;
#endif

#if RTE_I3C_BLOCKING_MODE_ENABLE
    if(i3c->blocking_mode)
    {
        /* Invoke slave receive blocking api */
        i3c_slave_rx_blocking(i3c->regs, &i3c->xfer);
        i3c->status.busy = 0U;

        if(!(i3c->xfer.status & I3C_XFER_STATUS_SLV_RX_DONE))
        {
            /* Resume the device if error occurs */
            i3c_resume(i3c->regs);
            return ARM_DRIVER_ERROR;
        }
    }
    else
#endif
    {
        /* Invoke slave receive api */
        i3c_slave_rx(i3c->regs, &i3c->xfer);
    }

#if I3C_DMA_ENABLE
    ret = I3C_DMA_Start_RX(i3c, data, len);
    if(ret)
    {
        return ARM_DRIVER_ERROR;
    }
#endif

    return ARM_DRIVER_OK;
}

/**
  \fn           int I3Cx_MasterAssignDA(I3C_RESOURCES *i3c,
                                        ARM_I3C_CMD *addr_cmd)
  \brief        Assign dynamic address to the i3c slave using
                ENTDAA and SETDASA
                Note: Only required for i3c slave devices;
                      i2c slave device uses static address
                      for communication \ref I3Cx_AttachSlvDev.
  \param[in]    i3c       : Pointer to i3c resources structure
  \param[in]    addr_cmd  : Address CCC
  \return       \ref execution_status
*/
static int I3Cx_MasterAssignDA(I3C_RESOURCES *i3c,
                               ARM_I3C_CMD *addr_cmd)
{
    int32_t pos         = 0U;
    uint8_t init_pos    = 0xFFU;
    uint8_t iter        = 0U;
    uint8_t dyn_addr    = 0U;

    if (i3c->state.powered == 0U)
        return ARM_DRIVER_ERROR;

    if (i3c->status.busy)
        return ARM_DRIVER_ERROR_BUSY;

    /* Perform below if it is ENTDAA CCC */
    if(addr_cmd->cmd_id == I3C_CCC_ENTDAA)
    {
        if(!addr_cmd->len)
        {
            return ARM_DRIVER_ERROR_PARAMETER;
        }

        else if(addr_cmd->len > i3c->slave_dat.maxdevs)
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }

        for(iter = 0U; iter < addr_cmd->len; iter++)
        {
            pos = I3cMasterGetFreePos(i3c);

            /* the dat is full */
            if (pos < 0)
            {
                if(init_pos == 0xFFU)
                    return ARM_DRIVER_ERROR;

                else
                    break;
            }
            else
            {
                /* reserve the index */
                i3c->slave_dat.freepos            &= ~(BIT(pos));
                i3c->slave_dat.last_asgd_addr_pos |=  (BIT(pos));

                /* we start assigning addresses from 0x09 */
                addr_cmd->addr             = (pos + I3C_NEXT_SLAVE_ADDR_OFFSET);
                while(true)
                {
                    /* Checks if new address is not a self address and
                     * not already assigned to some slave. If true then,
                     * assign it otherwise increment it */
                    if((I3cMasterGetAddrPos(i3c,  addr_cmd->addr) < 0) &&
                       (i3c_get_dynamic_addr(i3c->regs) !=  addr_cmd->addr))
                    {
                        break;
                    }
                    else
                    {
                        addr_cmd->addr++;
                    }
                }
                i3c->slave_dat.addrs[pos]  = addr_cmd->addr;
                i3c->slave_dat.addrs[pos] &= (~I3C_TARGET_SLAVE_TYPE_I2C);

                I3C_AddDynamicAddrParity(&addr_cmd->addr);

                /* We have space in the dat,
                 * program the dat in index pos */
                i3c_add_slv_to_dat(i3c->regs, pos,
                                   addr_cmd->addr,
                                   0);
            }

            /* Stores first found free address position in
             * init position*/
            if(init_pos == 0xFFU)
            {
                init_pos = pos;
            }
        }

        i3c->xfer.xfer_cmd.cmd_id     = addr_cmd->cmd_id;
        i3c->xfer.xfer_cmd.addr_index = init_pos;
        i3c->xfer.xfer_cmd.addr_depth = addr_cmd->len;
    }
    else
    {
        /*Returns error if slave static address is invalid */
        if (!addr_cmd->addr)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* Find the first unused index in freepos, note that this also
         * corresponds to the first unused location in the DAT
         */
        pos = I3cMasterGetFreePos(i3c);

        /* the dat is full */
        if (pos < 0)
        {
            return ARM_DRIVER_ERROR;
        }

        /* reserve the index */
        i3c->slave_dat.freepos            &= ~(BIT(pos));
        i3c->slave_dat.last_asgd_addr_pos |=  (BIT(pos));

        /* we start assigning addresses from 0x09 */
        dyn_addr                   = pos + I3C_NEXT_SLAVE_ADDR_OFFSET;
        while(true)
        {
            /* Checks if new address is not a self address and
             * not already assigned to some slave. If true then,
             * assign it otherwise increment it */
            if((I3cMasterGetAddrPos(i3c,  dyn_addr) < 0) &&
               (i3c_get_dynamic_addr(i3c->regs) !=  dyn_addr))
            {
                break;
            }
            else
            {
                dyn_addr++;
            }
        }
        i3c->slave_dat.addrs[pos]  = dyn_addr;
        i3c->slave_dat.addrs[pos]  &= (~I3C_TARGET_SLAVE_TYPE_I2C);

        /* We have space in the dat,
         * program the dat in index pos */
        i3c_add_slv_to_dat(i3c->regs, pos,
                           dyn_addr,
                           addr_cmd->addr);

        i3c->xfer.xfer_cmd.cmd_id     = addr_cmd->cmd_id;
        i3c->xfer.xfer_cmd.addr_index = pos;
        i3c->xfer.xfer_cmd.addr_depth = 1U;
    }

    i3c->status.busy              = 1U;
    i3c->xfer.error               = 0U;

    i3c->xfer.rx_len              = 0U;
    i3c->xfer.xfer_cmd.cmd_type   = I3C_XFER_TYPE_ADDR_ASSIGN;
    i3c->xfer.xfer_cmd.def_byte   = addr_cmd->def_byte;
    i3c->xfer.xfer_cmd.data_len   = 0U;

#if RTE_I3C_BLOCKING_MODE_ENABLE
    if(i3c->blocking_mode)
    {
        i3c->xfer.rx_len = 0U;
        i3c_send_xfer_cmd_blocking(i3c->regs, &i3c->xfer);
        i3c->status.busy = 0U;
        if(i3c->xfer.status != I3C_XFER_STATUS_DONE)
        {
            /* Error during address assignment,
             * so remove slave from Device address table */
            I3C_DetachSlaves(i3c, i3c->xfer.rx_len, i3c->xfer.xfer_cmd.cmd_id);
            i3c->slave_dat.last_asgd_addr_pos = 0U;

            i3c_resume(i3c->regs);
            return ARM_DRIVER_ERROR;
        }
    }
    else
#endif
    {
        i3c_send_xfer_cmd(i3c->regs, &i3c->xfer);
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int I3Cx_AttachSlvDev(I3C_RESOURCES *i3c,
                                      const ARM_I3C_DEVICE_TYPE dev_type,
                                      const uint8_t addr)
  \brief        Attach legacy i2c device to the i3c bus.
  \param[in]    i3c      : Pointer to i3c resources structure
  \param[in]    dev_type : i2c/i3c device
  \param[in]    addr     : Static/Dynamic address of slave device
  \return       \ref execution_status
*/
static int I3Cx_AttachSlvDev(I3C_RESOURCES *i3c,
                             const ARM_I3C_DEVICE_TYPE dev_type,
                             const uint8_t addr)
{
    int32_t  pos;

    if (i3c->state.powered == 0U)
        return ARM_DRIVER_ERROR;

    if (!addr)
        return ARM_DRIVER_ERROR_PARAMETER;

    /* Return error if the slave is already attached to
     * Device address table */
    if(I3cMasterGetAddrPos(i3c, addr) >= 0)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Return error if the slave address is same as
     * self address */
    if(addr == i3c_get_dynamic_addr(i3c->regs))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Find the first unused index in freepos, note that this also
     * corresponds to the first unused location in the DAT(Device Address Table) */
    pos = I3cMasterGetFreePos(i3c);

    /* DAT(Device Address Table) is full? */
    if (pos < 0)
    {
        /* error: DAT is full */
        return ARM_DRIVER_ERROR;
    }

    /* reserve the index */
    i3c->slave_dat.freepos   &= ~(BIT(pos));

    if(dev_type == ARM_I3C_DEVICE_TYPE_I2C)
    {
        /* ok, we have space in the DAT, store the static address and
         * mark as i2c legacy device is present.
         */
        i3c->slave_dat.addrs[pos] = (addr | I3C_TARGET_SLAVE_TYPE_I2C);

        /* Program the DAT(device address table) in index pos. */
        i3c_add_slv_to_dat(i3c->regs, pos, 0, addr);
    }
    else
    {
        /* ok, we have space in the DAT, store the i3c address */
        i3c->slave_dat.addrs[pos] = addr;

        /* Program the DAT(device address table) in index pos.
         * Store it as a dynamic address */
        i3c_add_slv_to_dat(i3c->regs, pos, addr, 0);
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int I3Cx_Detachdev(I3C_RESOURCES *i3c, uint8_t addr)
  \brief        Detach already attached i2c/i3c device from the i3c bus.
  \param[in]    i3c      : Pointer to i3c resources structure
  \param[in]    addr     : Static  address of already attached i2c device
                                        OR
                           Dynamic address of already attached i3c device
  \return       \ref execution_status
*/
static int I3Cx_Detachdev(I3C_RESOURCES *i3c, uint8_t addr)
{
    int32_t  pos;

    if (!addr)
        return ARM_DRIVER_ERROR_PARAMETER;

    if (i3c->state.powered == 0U)
        return ARM_DRIVER_ERROR;

    /* Get already attached i2c device address index in
     * DAT (device address table). */
    pos = I3cMasterGetAddrPos(i3c, addr);

    /* i2c i3c is not attached to DAT? */
    if (pos < 0)
    {
        /* err: i2c slave device is not attached to DAT,
         * first attach i2c device \ref I3Cx_AttachI2Cdev */
        return ARM_DRIVER_ERROR;
    }

    /* free the index */
    i3c->slave_dat.freepos   |= (BIT(pos));
    i3c->slave_dat.addrs[pos] = 0;

    /* clear the DAT index pos */
    i3c_remove_slv_from_dat(i3c->regs, pos);

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t I3Cx_Control(I3C_RESOURCES *i3c,
                                     uint32_t       control,
                                     uint32_t       arg)
  \brief        Control i3c master and slave.
  \param[in]    i3c      : Pointer to i3c resources structure
  \param[in]    control  : Operation
  \param[in]    arg      : Argument of operation
  \return       \ref execution_status
*/
static int32_t I3Cx_Control(I3C_RESOURCES *i3c,
                            uint32_t       control,
                            uint32_t       arg)
{
    I3C_I2C_SPEED_MODE i2c_speed_mode = 0;
    uint8_t slv_addr                  = 0U;
    int32_t pos                       = 0;
    uint8_t retry_cnt                 = 0U;
    ARM_I3C_SLV_PID    *hal_pid;
    i3c_slave_pid_t    slv_pid;

    if (i3c->state.powered == 0U)
        return ARM_DRIVER_ERROR;

    bool blocking_mode = false;
#if RTE_I3C_BLOCKING_MODE_ENABLE
    if(i3c->blocking_mode)
        blocking_mode = true;

#endif
    switch(control)
    {
        case I3C_MASTER_INIT:

            if(!i3c->state.enabled)
            {
                /* Sets the dynamic address */
                i3c_master_set_dynamic_addr(i3c->regs);
            }

            /* Initialise master */
            i3c_master_init(i3c->regs);

            if(!blocking_mode)
            {
                /* Enables interrupts */
                i3c_master_enable_interrupts(i3c->regs);
            }

            /* set state as master enabled. */
            i3c->state.enabled    = 1U;
            i3c->state.is_master  = 1U;
            break;

        case I3C_MASTER_SET_BUS_MODE:

            switch(arg)
            {
                case I3C_BUS_MODE_MIXED_FAST_I2C_FMP_SPEED_1_MBPS:
                case I3C_BUS_MODE_MIXED_FAST_I2C_FM_SPEED_400_KBPS:
                case I3C_BUS_MODE_MIXED_SLOW_I2C_SS_SPEED_100_KBPS:
                case I3C_BUS_MODE_MIXED_LIMITED:

                    if(arg == I3C_BUS_MODE_MIXED_FAST_I2C_FMP_SPEED_1_MBPS)
                        i2c_speed_mode = I3C_I2C_SPEED_MODE_FMP_1_MBPS;

                    if(arg == I3C_BUS_MODE_MIXED_FAST_I2C_FM_SPEED_400_KBPS)
                        i2c_speed_mode = I3C_I2C_SPEED_MODE_FM_400_KBPS;

                    if(arg == I3C_BUS_MODE_MIXED_SLOW_I2C_SS_SPEED_100_KBPS)
                        i2c_speed_mode = I3C_I2C_SPEED_MODE_SS_100_KBPS;

                    if(arg == I3C_BUS_MODE_MIXED_LIMITED)
                        i2c_speed_mode = I3C_I2C_SPEED_MODE_LIMITED;

                    if (!(i3c->core_clk))
                        return ARM_DRIVER_ERROR;

                    /* i2c clock configuration for selected Speed mode. */
                    i2c_clk_cfg(i3c->regs, i3c->core_clk, i2c_speed_mode);

                    /* fall through */
                case I3C_BUS_SLOW_MODE:

                    if (!(i3c->core_clk))
                        return ARM_DRIVER_ERROR;

                    /* i3c clock configuration */
                    i3c_slow_bus_clk_cfg(i3c->regs, i3c->core_clk);
                    break;

                case I3C_BUS_NORMAL_MODE:

                    if (!(i3c->core_clk))
                        return ARM_DRIVER_ERROR;

                    /* i3c clock configuration */
                    i3c_normal_bus_clk_cfg(i3c->regs, i3c->core_clk);
                    break;

                default:
                    return ARM_DRIVER_ERROR_UNSUPPORTED;
            }
            break;

        case I3C_SLAVE_SET_ADDR:

            /* Initialize and Enable i3c Slave */
            slv_addr = arg;

            i3c_slave_setup_adaptive_mode(i3c->regs, i3c->adaptive_mode);

            i3c_slave_init(i3c->regs, slv_addr, i3c->core_clk);

            if(!blocking_mode)
            {
                i3c_slave_enable_interrupts(i3c->regs);
            }
            /* set state as slave enabled. */
            i3c->state.enabled    = 1U;
            i3c->state.is_master  = 0U;
            break;

        case I3C_MASTER_SET_SLAVE_NACK_RETRY_COUNT:

            pos = I3cMasterGetAddrPos(i3c, (uint8_t)arg);

            if(pos < 0)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }

            retry_cnt = ((arg & I3C_SLAVE_NACK_RETRY_COUNT_Msk) >>
                          I3C_SLAVE_NACK_RETRY_COUNT_Pos);
            if(retry_cnt > I3C_SLAVE_NACK_RETRY_COUNT_MAX)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }

            /* Invoke low level api to set slv nack retry cnt */
            i3c_set_slave_nack_retry_cnt(i3c->regs, pos, retry_cnt);
            break;

        case I3C_SET_SDA_TX_HOLD_TIME:

            if(arg > I3C_SDA_TX_HOLD_TIME_MAX)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }
            /* Sets sda tx hold time */
            i3c_set_sda_tx_hold_time(i3c->regs, arg);
            break;

        case I3C_MASTER_ABORT_MESSAGE_TRANSFER:

            i3c_abort_msg_transfer(i3c->regs);
            break;

        case I3C_MASTER_SETUP_HOT_JOIN_ACCEPTANCE:
            /* Sets up HJ acceptability at master side */
            i3c_master_setup_hot_join_ctrl(i3c->regs, arg);
            break;

        case I3C_MASTER_SETUP_MR_ACCEPTANCE:
            /* Sets Master Request acceptability at master side */
            i3c_master_setup_mst_req_ctrl(i3c->regs, arg);
            break;

        case I3C_MASTER_SETUP_SIR_ACCEPTANCE:
            /* Sets Slave Interrupt Request acceptability at master side */
            i3c_master_setup_slv_intr_req_ctrl(i3c->regs, arg);
            break;

        case I3C_SLAVE_REQUEST_IBI_BUS_MASTERSHIP:

            if(i3c_slave_req_bus_mastership(i3c->regs) != 0)
            {
                return ARM_DRIVER_ERROR;
            }
            break;

        case I3C_SLAVE_SET_IBI_SIR:
            /* Sets Slave interrupt request to master */
            if(i3c_slave_tx_slv_intr_req(i3c->regs) != 0)
            {
                return ARM_DRIVER_ERROR;
            }
            break;

        case I3C_SET_DEVICE_CHARACTERISTICS:

            /* Sets device's DCR */
            i3c_set_dcr(i3c->regs, arg);
            break;

        case I3C_SLAVE_SET_PID:

            if(!arg)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }

            /* Copy the PID and invoke set pid fun
             * to store it */
            hal_pid             = (ARM_I3C_SLV_PID*)arg;

            slv_pid.dcr         = hal_pid->dcr;
            slv_pid.inst_id     = hal_pid->inst_id;
            slv_pid.part_id     = hal_pid->part_id;
            slv_pid.pid_sel     = hal_pid->pid_sel;
            slv_pid.mipi_mfg_id = hal_pid->mipi_mfg_id;

            i3c_slave_set_pid(i3c->regs, slv_pid);
            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t I3Cx_Initialize(I3C_RESOURCES         *i3c,
                                        ARM_I3C_SignalEvent_t  cb_event)
  \brief        Initialize the i3c device.
  \param[in]    i3c      : Pointer to i3c resources structure
  \param[in]    cb_event : Pointer to I3C Event
                            \ref ARM_I3C_SignalEvent_t
  \return       \ref execution_status
*/
static int32_t I3Cx_Initialize(I3C_RESOURCES         *i3c,
                               ARM_I3C_SignalEvent_t  cb_event)
{
    if (i3c->state.initialized == 1)
        return ARM_DRIVER_OK;

    bool blocking_mode = false;
#if RTE_I3C_BLOCKING_MODE_ENABLE
    if(i3c->blocking_mode)
    {
        blocking_mode = true;
    }
#endif

    /* If callback function is null in non-blocking mode,
     * then sends Error parameter */
    if((!blocking_mode) && (!cb_event))
        return ARM_DRIVER_ERROR_PARAMETER;

    /* set the user callback event. */
    i3c->cb_event = cb_event;

#if I3C_DMA_ENABLE
    i3c->dma_cfg->dma_rx.dma_handle = -1;
    i3c->dma_cfg->dma_tx.dma_handle = -1;

    /* Initialize DMA for I3C-Tx */
    if(I3C_DMA_Initialize(&i3c->dma_cfg->dma_tx) != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;

    /* Initialize DMA for I3C-Rx */
    if(I3C_DMA_Initialize(&i3c->dma_cfg->dma_rx) != ARM_DRIVER_OK)
        return ARM_DRIVER_ERROR;
#endif

    i3c->core_clk = GetSystemAPBClock();
    /* set the state as initialized. */
    i3c->state.initialized = 1;
    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t I3Cx_Uninitialize(I3C_RESOURCES  *i3c)
  \brief        Uninitialize the i3c device
  \param[in]    i3c      : Pointer to i3c resources structure
  \return       \ref execution_status
*/
static int32_t I3Cx_Uninitialize(I3C_RESOURCES  *i3c)
{
#if I3C_DMA_ENABLE
    i3c->dma_cfg->dma_rx.dma_handle = -1;
    i3c->dma_cfg->dma_tx.dma_handle = -1;
#endif

    i3c->state.initialized = 0;
    return ARM_DRIVER_OK;
}

/**
  \fn           int32_t I3Cx_PowerControl(I3C_RESOURCES   *i3c,
                                          ARM_POWER_STATE  state)
  \brief        i3c power control
  \param[in]    i3c      : Pointer to i3c resources structure
  \param[in]    state    : Power state
  \return       \ref execution_status
*/
static int32_t I3Cx_PowerControl(I3C_RESOURCES   *i3c,
                                 ARM_POWER_STATE  state)
{
    bool blocking_mode = false;
#if RTE_I3C_BLOCKING_MODE_ENABLE
    if(i3c->blocking_mode)
    {
        blocking_mode = true;
    }
#endif

    switch (state)
    {
        case ARM_POWER_OFF:

            /* Perform below steps if not blocking mode */
            if(!blocking_mode)
            {
                /* Disable i3c IRQ */
                NVIC_DisableIRQ(i3c->irq);

                /* Clear Any Pending i3c IRQ */
                NVIC_ClearPendingIRQ(i3c->irq);
            }

#if I3C_DMA_ENABLE
            /* Disable i3c DMA */
            i3c_dma_disable(i3c->regs);

            /* Deallocate DMA channel for Tx */
            if(I3C_DMA_DeAllocate(&i3c->dma_cfg->dma_tx))
                return ARM_DRIVER_ERROR;

            /* Deallocate DMA channel for Rx */
            if(I3C_DMA_DeAllocate(&i3c->dma_cfg->dma_rx))
                return ARM_DRIVER_ERROR;
#endif /* I3C_DMA_ENABLE */

            /* i3c EXPMST0 control configuration:
             *  Disable i3c clock. */
            disable_i3c_clock();

            /* Reset the power state. */
            i3c->state.enabled   = 0U;
            i3c->state.is_master = 0U;
            i3c->state.powered   = 0;
            break;

        case ARM_POWER_FULL:

            if (i3c->state.initialized == 0U)
                return ARM_DRIVER_ERROR;

            if (i3c->state.powered)
                break;

            /* i3c EXPMST0 control configuration:
             *  Enable i3c clock. */
            enable_i3c_clock();

#if I3C_DMA_ENABLE
            /* if DMA2 is selected? */
            if(i3c->dma_cfg->dma_tx.evtrtr_cfg.instance == 2)
            {
                select_i3c_dma2();
            }
            /* else: default DMA0 is selected. */

            /* Enable i3c DMA */
            i3c_dma_enable(i3c->regs);
#endif /* I3C_DMA_ENABLE */

            /* Perform below steps if not blocking mode */
            if(!blocking_mode)
            {
                /* Enable i3c IRQ */
                NVIC_ClearPendingIRQ(i3c->irq);
                NVIC_SetPriority(i3c->irq, i3c->irq_priority);
                NVIC_EnableIRQ(i3c->irq);
            }

            i3c->slave_dat.datp    = i3c_get_dat_addr(i3c->regs);
            i3c->slave_dat.maxdevs = i3c_get_dat_depth(i3c->regs);
            i3c->slave_dat.freepos = GENMASK(i3c->slave_dat.maxdevs - 1, 0);

            /* Set the state as powered */
            i3c->state.enabled   = 0U;
            i3c->state.is_master = 0U;
            i3c->state.powered = 1;
            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

#if I3C_DMA_ENABLE
    /* Power Control DMA for I3C-Tx */
    if(I3C_DMA_PowerControl(state, &i3c->dma_cfg->dma_tx) != ARM_DRIVER_OK)
    {
        i3c->state.powered = 0;
        return ARM_DRIVER_ERROR;
    }

    /* Power Control DMA for I3C-Rx */
    if(I3C_DMA_PowerControl(state, &i3c->dma_cfg->dma_rx) != ARM_DRIVER_OK)
    {
        i3c->state.powered = 0;
        return ARM_DRIVER_ERROR;
    }

    if(state == ARM_POWER_FULL)
    {
        /* Try to allocate a DMA channel for TX */
        if(I3C_DMA_Allocate(&i3c->dma_cfg->dma_tx))
        {
            i3c->state.powered = 0;
            return ARM_DRIVER_ERROR;
        }

        /* Try to allocate a DMA channel for RX */
        if(I3C_DMA_Allocate(&i3c->dma_cfg->dma_rx))
        {
            i3c->state.powered = 0;
            return ARM_DRIVER_ERROR;
        }
    }
#endif /* I3C_DMA_ENABLE */

    return ARM_DRIVER_OK;
}


#if I3C_DMA_ENABLE
/**
  \fn          static void  I3Cx_DMACallback(uint32_t event, int8_t peri_num,
                                            I3C_RESOURCES *i3c)
  \brief       Callback function from DMA for I3C
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \param[in]   I3C       Pointer to I3C resources
  \return      none
*/
static void I3Cx_DMACallback(uint32_t event, int8_t peri_num,
                            I3C_RESOURCES *i3c)
{
    if(!i3c->cb_event)
        return;

    /* Transfer Completed */
    if(event & ARM_DMA_EVENT_COMPLETE)
    {
        switch(peri_num)
        {
            case I3C_DMA_TX_PERIPH_REQ:
                /* For DMA TX,
                 *  Success/Error decision will be taken by
                 *   Interrupt Handler based on status of Response-Queue.
                 *   (as this callback will be always called
                 *    irrespective of slave gives ACK/NACK.)
                 */
                break;

            case I3C_DMA_RX_PERIPH_REQ:
                /* For DMA RX,
                 *  Success decision will be taken here(DMA RX Callback).
                 *  Error decision will be taken by Interrupt Handler
                 *   based on status of Response-Queue.
                 */

                /* clear transfer status. */
                i3c->xfer.status = I3C_XFER_STATUS_NONE;

                /* clear busy flag. */
                i3c->status.busy = 0;

                /* Mark event as success and call the user callback */
                i3c->cb_event(ARM_I3C_EVENT_TRANSFER_DONE);
                break;

            default:
                break;
        }
    }

    /* Abort Occurred */
    if(event & ARM_DMA_EVENT_ABORT)
    {

    }
}
#endif /* RTE_I3C_DMA_ENABLE */


/* I3C Driver Instance */
#if (RTE_I3C)

#if RTE_I3C_DMA_ENABLE
static void I3C_DMACallback(uint32_t event, int8_t peri_num);
static I3C_DMA_HW_CONFIG I3Cx_DMA_HW_CONFIG =
{
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I3C_DMA),
        .dma_periph_req = I3C_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
            .instance = I3C_DMA,
            .group    = I3C_DMA_GROUP,
            .channel  = I3C_DMA_RX_PERIPH_REQ,
            .enable_handshake = I3C_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I3C_DMA),
        .dma_periph_req = I3C_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
            .instance = I3C_DMA,
            .group    = I3C_DMA_GROUP,
            .channel  = I3C_DMA_TX_PERIPH_REQ,
            .enable_handshake = I3C_DMA_HANDSHAKE_ENABLE,
        },

    },
};
#endif /* RTE_I3C_DMA_ENABLE */

/* I3C Device Resources */
static I3C_RESOURCES i3c =
{
    .regs              = (I3C_Type *)I3C_BASE,
    .cb_event          = (void*)0,
    .xfer              = {
                          .xfer_cmd = {0},
                          .tx_len   = 0,
                          .rx_len   = 0,
                          .tx_buf   = (void*)0,
                          .rx_buf   = (void*)0,
                          .status   = I3C_XFER_STATUS_NONE,
                          .error    = 0,
                          .addr     = 0
                         },
    .status            = {0},
    .state             = {0},
#if RTE_I3C_BLOCKING_MODE_ENABLE
    .blocking_mode     = true,
#endif
    .adaptive_mode     = RTE_I3C_SLAVE_ADAPTIVE_MODE_ENABLE,
    .irq               = (IRQn_Type) I3C_IRQ_IRQn,
    .irq_priority      = RTE_I3C_IRQ_PRI,

#if RTE_I3C_DMA_ENABLE
    .dma_cb            = I3C_DMACallback,
    .dma_cfg           = &I3Cx_DMA_HW_CONFIG,
    .dma_irq_priority  = RTE_I3C_DMA_IRQ_PRI,
#endif

};

#if RTE_I3C_DMA_ENABLE
/**
  \fn          static void  I3C_DMACallback (uint32_t event, int8_t peri_num)
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \brief       Callback function from DMA for I3C
*/
static void I3C_DMACallback(uint32_t event, int8_t peri_num)
{
    I3Cx_DMACallback(event, peri_num, &i3c);
}
#endif /* RTE_I3C_DMA_ENABLE */

void I3C_IRQHandler(void)
{
    i3c_xfer_t *xfer = &(i3c.xfer);
    uint32_t event   = 0;

    /* Invokes master ISR if the mode is master, else invokes slave ISR */
    if(i3c.state.is_master)
    {
        i3c_master_irq_handler(i3c.regs, xfer);
    }
    else
    {
        i3c_slave_irq_handler(i3c.regs, xfer);
    }

    /* check status: Transfer Error? */
    if(xfer->status & I3C_XFER_STATUS_ERROR)
    {
#if RTE_I3C_DMA_ENABLE
        /* Stop DMA TX transfer */
        if(xfer->status & I3C_XFER_STATUS_ERROR_TX)
        {
            I3C_DMA_Stop(&i3c.dma_cfg->dma_tx);
        }

        /* Stop DMA RX transfer */
        if(xfer->status & I3C_XFER_STATUS_ERROR_RX)
        {
            I3C_DMA_Stop(&i3c.dma_cfg->dma_rx);
        }
#endif /* RTE_I3C_DMA_ENABLE */

        /* mark event as Transfer Error. */
        event = ARM_I3C_EVENT_TRANSFER_ERROR;

        if(xfer->status & I3C_XFER_STATUS_ERROR_ADDR_ASSIGN)
        {
            /* Error during address assignment,
             * so remove slave from Device address table */
            I3C_DetachSlaves(&i3c, i3c.xfer.tx_len, i3c.xfer.xfer_cmd.cmd_id);
            i3c.slave_dat.last_asgd_addr_pos = 0U;
        }

        else if(xfer->status & I3C_XFER_STATUS_ERROR_XFER_ABORT)
        {
            /* mark event as Transfer Error. */
            event = ARM_I3C_EVENT_MESSAGE_TRANSFER_ABORT;
        }
        /* error: Resume i3c controller and
         *        clear error status. */
        i3c_resume(i3c.regs);
        i3c_clear_xfer_error(i3c.regs);

    } /* if I3C_XFER_STATUS_ERROR */

    /* check status: Transfer Success? */
    else if(xfer->status & I3C_XFER_STATUS_DONE)
    {
        /* mark event as Transfer done. */
        event = ARM_I3C_EVENT_TRANSFER_DONE;

        if(xfer->status & I3C_XFER_STATUS_SLV_HOT_JOIN_REQ)
        {
            /* mark event as Hot Join request */
            event = ARM_I3C_EVENT_IBI_HOT_JOIN_REQ;
        }
        else if(xfer->status & I3C_XFER_STATUS_IBI_SLV_INTR_REQ)
        {
            /* mark event as SIR request */
            event = ARM_I3C_EVENT_IBI_SLV_INTR_REQ;
        }
        else if(xfer->status & I3C_XFER_STATUS_DEFSLV_LIST)
        {
            i3c.status.defslv_cnt = xfer->rx_len;
            /* mark event as slave list received */
            event = ARM_I3C_EVENT_SLAVE_LIST;
        }
        else if(xfer->status & I3C_XFER_STATUS_IBI_MASTERSHIP_REQ)
        {
            if(I3cMasterGetAddrPos(&i3c, xfer->addr) != ARM_DRIVER_ERROR)
            {
                /* mark event as Mastership request */
                event = ARM_I3C_EVENT_IBI_MASTERSHIP_REQ;
            }
            else
            {
                /* clear transfer status. */
                xfer->status            = I3C_XFER_STATUS_NONE;

                /* clear busy flag. */
                i3c.status.busy         = 0;

                /* Sets the speed to SDR Maximum */
                i3c.xfer.xfer_cmd.speed = I3C_SPEED_SDR0;

                return;
            }
        }
        else if(xfer->status & I3C_XFER_STATUS_BUSOWNER_UPDATED)
        {
            if(i3c_is_master(i3c.regs))
            {
                i3c.state.is_master = 1U;
            }
            else
            {
                i3c.state.is_master = 0U;
                memset(&i3c.slave_dat, 0, sizeof(I3C_SLAVE_DAT_TYPE));
            }

            /* Flushes all buffers and resumes */
            i3c_flush_all_buffers(i3c.regs);
            i3c_resume(i3c.regs);

            /* mark event as Bus owner updated */
            event = ARM_I3C_EVENT_BUSOWNER_UPDATED;
        }
        else if(xfer->status & I3C_XFER_STATUS_ADDR_ASSIGN_DONE)
        {
            i3c.slave_dat.last_asgd_addr_pos = 0U;
        }
        else if(xfer->status & I3C_XFER_STATUS_CCC_SET_DONE)
        {
            if((i3c.xfer.xfer_cmd.cmd_id == I3C_CCC_RSTDAA(true))    ||
               (i3c.xfer.xfer_cmd.cmd_id == I3C_CCC_RSTDAA(false))   ||
               (i3c.xfer.xfer_cmd.cmd_id == I3C_CCC_SETNEWDA))
            {
                I3C_DetachSlaves(&i3c,
                                 i3c.xfer.tx_len,
                                 i3c.xfer.xfer_cmd.cmd_id);
            }
        }
        else if(xfer->status & I3C_XFER_STATUS_SLV_CCC_UPDATED)
        {
            /* mark event as Slave CCC updated */
            event = ARM_I3C_EVENT_SLAVE_CCC_UPDATED;
        }
        /* check status: Slave dynamic address assign? (only for Slave mode) */
        else if(xfer->status & I3C_XFER_STATUS_SLV_DYN_ADDR_ASSGN)
        {
            /* mark event as Slave dynamic address assignment done. */
            event = ARM_I3C_EVENT_SLV_DYN_ADDR_ASSGN;
        }

#if RTE_I3C_DMA_ENABLE
        /* DMA TX,
         *  Success/Error decision will be taken by
         *   Interrupt Handler based on status of Response-Queue.
         *
         * DMA RX,
         *   Success decision will be taken in DMA_RX callback.
         *   Error decision will be taken by Interrupt Handler
         *    based on status of Response-Queue.
         */
        if( (xfer->status & I3C_XFER_STATUS_MST_RX_DONE) ||
            (xfer->status & I3C_XFER_STATUS_SLV_RX_DONE) )
        {
            /* for DMA RX Success, mark event as 0. */
            event = 0;

            /* clear transfer status. */
            xfer->status = I3C_XFER_STATUS_NONE;
        }
#endif /* RTE_I3C_DMA_ENABLE */
    }/* else if I3C_XFER_STATUS_DONE*/

    if(event)
    {
        /* clear transfer status. */
        xfer->status = I3C_XFER_STATUS_NONE;

        /* clear busy flag. */
        i3c.status.busy = 0;

        /* call the user callback */
        if(i3c.cb_event)
            i3c.cb_event(event);
    }

    /* Sets the speed to SDR Maximum */
    i3c.xfer.xfer_cmd.speed      = I3C_SPEED_SDR0;
}

/* wrapper functions for I3C */
static ARM_I3C_STATUS I3C_GetStatus(void)
{
    return I3Cx_GetStatus(&i3c);
}

static ARM_I3C_DEVICE_INFO I3C_GetDeviceInfo(void)
{
    return I3Cx_GetDeviceInfo(&i3c);
}

static int32_t I3C_Initialize(ARM_I3C_SignalEvent_t cb_event)
{
    return (I3Cx_Initialize(&i3c, cb_event));
}

static int32_t I3C_Uninitialize(void)
{
    return (I3Cx_Uninitialize(&i3c));
}

static int32_t I3C_PowerControl(ARM_POWER_STATE state)
{
    return (I3Cx_PowerControl(&i3c, state));
}

static int32_t I3C_MasterTransmit(uint8_t addr, const uint8_t *data, uint16_t len)
{
    return (I3Cx_MasterTransmit(&i3c, addr, data, len));
}

static int32_t I3C_MasterReceive(uint8_t addr, uint8_t *data, uint16_t len)
{
    return (I3Cx_MasterReceive(&i3c, addr, data, len));
}

static int32_t I3C_SlaveTransmit(const uint8_t *data, uint16_t len)
{
    return (I3Cx_SlaveTransmit(&i3c, data, len));
}

static int32_t I3C_SlaveReceive(uint8_t *data, uint16_t len)
{
    return (I3Cx_SlaveReceive(&i3c, data, len));
}

static int32_t I3C_MasterSendCommand(ARM_I3C_CMD *ccc)
{
    return (I3Cx_MasterSendCommand(&i3c, ccc));
}

static int32_t I3C_Control(uint32_t control, uint32_t arg)
{
    return (I3Cx_Control(&i3c, control, arg));
}

static int32_t I3C_MasterAssignDA(ARM_I3C_CMD *addr_cmd)
{
    return (I3Cx_MasterAssignDA(&i3c, addr_cmd));
}

static int32_t I3C_AttachSlvdev(const ARM_I3C_DEVICE_TYPE dev_type,
                                const uint8_t addr)
{
    return (I3Cx_AttachSlvDev(&i3c, dev_type, addr));
}

static int32_t I3C_Detachdev(uint8_t addr)
{
    return (I3Cx_Detachdev(&i3c, addr));
}

static int32_t I3C_GetSlaveList(uint8_t* addr_list,
                            uint8_t* count)
{
    return I3Cx_GetSlaveList(&i3c, addr_list, count);
}

static int32_t I3C_GetSlaveDynamicAddr(const uint8_t static_addr,
                                       uint8_t *dynamic_addr)
{
    return I3Cx_GetSlaveDynamicAddr(&i3c, static_addr, dynamic_addr);
}

static int32_t I3C_GetSlvsInfo(void* data, const uint8_t value)
{
    return I3Cx_GetSlvsInfo(&i3c, data, value);
}

/* I3C Driver Control Block */
extern ARM_DRIVER_I3C Driver_I3C;
ARM_DRIVER_I3C Driver_I3C =
{
    I3C_GetVersion,
    I3C_GetCapabilities,
    I3C_GetStatus,
    I3C_GetDeviceInfo,
    I3C_Initialize,
    I3C_Uninitialize,
    I3C_PowerControl,
    I3C_MasterTransmit,
    I3C_MasterReceive,
    I3C_SlaveTransmit,
    I3C_SlaveReceive,
    I3C_Control,
    I3C_MasterSendCommand,
    I3C_MasterAssignDA,
    I3C_AttachSlvdev,
    I3C_Detachdev,
    I3C_GetSlaveList,
    I3C_GetSlaveDynamicAddr,
    I3C_GetSlvsInfo
};
#endif /* RTE_I3C */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/

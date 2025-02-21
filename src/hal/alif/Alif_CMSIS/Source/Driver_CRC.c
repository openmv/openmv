/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/* Project Includes */
#include "crc.h"
#include "Driver_CRC_Private.h"

#if !(RTE_CRC0 || RTE_CRC1)
#error "CRC is not enabled in the RTE_device.h"
#endif

#if (defined(RTE_Drivers_CRC0) && !RTE_CRC0)
#error "CRC0 not configured in RTE_Device.h!"
#endif

#if (defined(RTE_Drivers_CRC1) && !RTE_CRC1)
#error "CRC1 not configured in RTE_Device.h!"
#endif

#define ARM_CRC_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)  /*  Driver version */

/*Driver version*/
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_CRC_API_VERSION,
    ARM_CRC_DRV_VERSION
};

/*Driver Capabilities   */
static const ARM_CRC_CAPABILITIES DriverCapabilities = {
    1,  /* Supports CRC_8_CCITT */
    1,  /* Supports CRC_16 */
    1,  /* Supports CRC_16_CCITT */
    1,  /* Supports CRC_32 */
    1,  /* Supports CRC_32C */
    0   /* Reserved ( must be ZERO) */
};

/**
 @fn           ARM_DRIVER_VERSION CRC_GetVersion(void)
 @brief        get CRC version
 @param        none
 @return       driver version
 */
static ARM_DRIVER_VERSION CRC_GetVersion(void)
{
    return DriverVersion;
}

/**
 @fn           ARM_CRC_CAPABILITIES CRC_GetCapabilities(void)
 @brief        get CRC Capabilites
 @param        none
 @return       driver Capabilites
 */
static ARM_CRC_CAPABILITIES CRC_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
 @fn           Control_Bit(uint32_t control, uint32_t arg, CRC_RESOURCES *CRC)
 @brief        To enable or disable the Reflect, Invert, Bit, Byte, Custom polynomial bit of CRC
 @param[in]    control : To check CRC Reflect, Invert, Bit, Byte, Custom polynomial bits of CRC
                         are enabled.
 @param[in]    arg     : To enable or disable the Reflect, Invert, Bit, Byte,
                         Custom polynomial bits of CRC
 @param[in]    CRC     : Pointer to CRC resources
 @return       none
 */
__STATIC_INLINE void Control_Bit (uint32_t control, uint32_t arg, CRC_RESOURCES *CRC)
{
    /* To select the CRC byte swap */
    if (control & ARM_CRC_ENABLE_BYTE_SWAP )
    {
        if(arg)
            crc_enable_byte_swap(CRC->regs);
        else
            crc_disable_byte_swap(CRC->regs);
    }

    /*To select the CRC bit swap */
    if (control & ARM_CRC_ENABLE_BIT_SWAP)
    {
        if(arg)
            crc_enable_bit_swap(CRC->regs);
        else
            crc_disable_bit_swap(CRC->regs);
    }

    /*To select the CRC custom polynomial */
    if(control & ARM_CRC_ENABLE_CUSTOM_POLY)
    {
        if(arg)
            crc_enable_custom_poly(CRC->regs);
        else
            crc_disable_custom_poly(CRC->regs);
    }

    /*To select the CRC Invert */
    if(control & ARM_CRC_ENABLE_INVERT_OUTPUT)
    {
        if(arg)
            crc_enable_invert(CRC->regs);
        else
            crc_disable_invert(CRC->regs);
    }

    /*To select the CRC reflect */
    if(control & ARM_CRC_ENABLE_REFLECT_OUTPUT)
    {
        if(arg)
            crc_enable_reflect(CRC->regs);
        else
            crc_disable_reflect(CRC->regs);
    }
}

#if CRC_DMA_ENABLE
/**
  \fn          int32_t CRC_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Initialize DMA for CRC
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
__STATIC_INLINE int32_t CRC_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Initializes DMA interface */
    status = dma_drv->Initialize();
    if(status) {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t CRC_DMA_PowerControl(ARM_POWER_STATE state,
                                            DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       PowerControl DMA for CRC
  \param[in]   state  Power state
  \param[in]   dma_periph     Pointer to DMA resources
  \return      \ref execution_status
*/
__STATIC_INLINE int32_t CRC_DMA_PowerControl(ARM_POWER_STATE state,
                                             DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Initializes DMA interface */
    status = dma_drv->PowerControl(state);
    if(status) {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t CRC_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Allocate a channel for I2S
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
__STATIC_INLINE int32_t CRC_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Allocate handle for peripheral */
    status = dma_drv->Allocate(&dma_periph->dma_handle);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Disable DMA Handshaking for CRC */
    status = dma_drv->Control(&dma_periph->dma_handle, ARM_DMA_CRC_MODE, NULL);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t CRC_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       De-allocate channel of CRC
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
__STATIC_INLINE int32_t CRC_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* De-Allocate handle  */
    status = dma_drv->DeAllocate(&dma_periph->dma_handle);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t CRC_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
                                     ARM_DMA_PARAMS *dma_params)
  \brief       Start CRC DMA transfer
  \param[in]   dma_periph     Pointer to DMA resources
  \param[in]   dma_params     Pointer to DMA parameters
  \return      \ref           execution_status
*/
__STATIC_INLINE int32_t CRC_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
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
  \fn          int32_t CRC_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Stop CRC DMA transfer
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
__STATIC_INLINE int32_t CRC_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
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
  \fn          static void  CRC_DMACallback(uint32_t event, int8_t peri_num, CRC_RESOURCES *CRC)
  \brief       Callback function from DMA for CRC
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \param[in]   crc       Pointer to crc resources
*/
static void CRC_DMACallback(uint32_t event, int8_t peri_num, CRC_RESOURCES *CRC)
{
    uint8_t   algo_size;

    (void)peri_num;
    CRC->dma_event = event;

    /* Deallocate the DMA channel */
    CRC_DMA_DeAllocate(&CRC->dma_cfg);

    /* Transfer Completed */
    if(event & ARM_DMA_EVENT_COMPLETE)
    {
        /* data_out pointer to store the CRC output */
        *CRC->transfer.data_out = crc_read_output_value(CRC->regs);

        /* To check whether the algorithm size is 8 bit or 16 or 32 bit */
        algo_size = (uint8_t)crc_get_algorithm_size(CRC->regs);
        if(algo_size == CRC_32_BIT_SIZE)
        {
            /* Calculated the 32bit CRC of the unaligned part - if any */
            crc_calculate_32bit_unaligned_sw(CRC->regs, &CRC->transfer);
        }

        if(CRC->cb_event)
            CRC->cb_event(ARM_CRC_COMPUTE_EVENT_DONE);
    }

    /* Abort Occurred */
    if(event & ARM_DMA_EVENT_ABORT)
    {
        /*
        * There is no event for indicating error in CRC driver.
        * Let the application get timeout and restart the CRC.
        *
        */
    }

    /* Clear busy flag */
    CRC->busy = 0;
}

/**
  \fn          static int32_t  CRC_DMA_Copy(const void *data_in, uint32_t data_len,
                                            uint8_t algo_size, CRC_RESOURCES *CRC)
  \brief       CRC DMA Copy function
  \param[in]   data_in   Input Data to the CRC register
  \param[in]   data_len  Data length
  \param[in]   algo_size Algorithm size
  \param[in]   CRC       Pointer to crc resources
*/
static int32_t CRC_DMA_Copy(const void *data_in, uint32_t data_len,
                            uint8_t algo_size, CRC_RESOURCES *CRC)
{
    ARM_DMA_PARAMS params;
    int32_t        ret;

    /* Deallocate the DMA channel */
    if(CRC_DMA_Allocate(&CRC->dma_cfg))
        return ARM_DRIVER_ERROR;

    params.peri_reqno   = (int8_t)-1;
    params.dir          = ARM_DMA_MEM_TO_DEV;
    params.cb_event     = CRC->dma_cb;
    params.src_addr     = data_in;
    params.burst_len    = 1;
    params.num_bytes    = data_len;
    params.irq_priority = CRC->dma_irq_priority;

    CRC->dma_event      = 0U;

    switch(algo_size)
    {
    /* For 8 bit CRC */
    case CRC_8_BIT_SIZE:
    case CRC_16_BIT_SIZE:
        params.dst_addr     = crc_get_8bit_datain_addr(CRC->regs);
        params.burst_size   = BS_BYTE_1;
        break;
    case CRC_32_BIT_SIZE:
        params.dst_addr     = crc_get_32bit_datain_addr(CRC->regs);
        params.burst_size   = BS_BYTE_4;
        break;
    }

    ret = CRC_DMA_Start(&CRC->dma_cfg, &params);

    return ret;
}
#endif /* CRC_DMA_ENABLE */

/**
@fn          int32_t CRC_Initialize (CRC_RESOURCES *CRC, ARM_CRC_SignalEvent_t cb_event)
@brief       Initialize the CRC interface
@param[in]   CRC      : Pointer to CRC resources
@param[in]   cb_event : Pointer to /ref ARM_CRC_Signal_Event_t cb_event
 @return     ARM_DRIVER_ERROR_PARAMETER : if CRC device is invalid
             ARM_DRIVER_OK              : if CRC successfully initialized or already initialized
 */
static int32_t CRC_Initialize(CRC_RESOURCES *CRC, ARM_CRC_SignalEvent_t cb_event)
{
    int ret = ARM_DRIVER_OK;

    if(CRC->state.initialized == 1)
    {
        return ARM_DRIVER_OK;
    }

    /* User call back Event */
    CRC->cb_event = cb_event;

#if CRC_DMA_ENABLE
    if(CRC->dma_enable)
    {
        CRC->dma_cfg.dma_handle = -1;
        CRC->dma_event          = 0U;

        /* Initialize DMA for CRC */
        if(CRC_DMA_Initialize(&CRC->dma_cfg) != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;
    }
#endif
    /* Setting the state */
    CRC->state.initialized = 1;

    return ret;
}

/**
@fn          int32_t CRC_Uninitialize (CRC_RESOURCES *CRC)
@brief       Clear the CRC configuration
@param[in]   CRC : Pointer to CRC resources
 @return     ARM_DRIVER_ERROR_PARAMETER : if CRC device is invalid
             ARM_DRIVER_OK              : if CRC successfully initialized or already initialized
 */
static int32_t CRC_Uninitialize(CRC_RESOURCES *CRC)
{
    int ret = ARM_DRIVER_OK;

    if(CRC->state.initialized == 0)
        return ARM_DRIVER_OK;

    if(CRC->state.powered == 1)
        return ARM_DRIVER_ERROR;

    /* set call back to NULL */
    CRC->cb_event = NULL;

    /* Clear the CRC configuration */
    crc_clear_config(CRC->regs);
#if CRC_DMA_ENABLE
    if(CRC->dma_enable)
    {
        CRC->dma_cfg.dma_handle = -1;
    }
#endif
    /* Reset the state */
    CRC->state.initialized = 0;

    return ret;
}

/**
 @fn           int32_t CRC_PowerControl (ARM_POWER_STATE state,
                                          CRC_RESOURCES *CRC)
 @brief        CMSIS-DRIVER CRC power control
 @param[in]    state : Power state
 @param[in]    CRC   : Pointer to CRC resources
 @return       ARM_DRIVER_ERROR_PARAMETER  : if CRC device is invalid
               ARM_DRIVER_OK               : if CRC successfully uninitialized or already not initialized
 */
static int32_t CRC_PowerControl(ARM_POWER_STATE status,
                                CRC_RESOURCES *CRC)
{
    if(CRC->state.initialized == 0)
        return ARM_DRIVER_ERROR;

    switch(status)
    {
    case ARM_POWER_OFF:

    /* Clear the CRC configuration */
    crc_clear_config(CRC->regs);

    /* Reset the power state */
    CRC->state.powered = 0;

    break;

    case ARM_POWER_FULL:

    if(CRC->state.initialized == 0)
    {
        /* error:Driver is not initialized */
        return ARM_DRIVER_ERROR;
    }

    if(CRC->state.powered == 1)
    {
        return ARM_DRIVER_OK;
    }

    /* Clear the CRC configuration */
    crc_clear_config(CRC->regs);

    /* Set the power state enabled */
    CRC->state.powered = 1;

    break;

    case ARM_POWER_LOW:
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

#if CRC_DMA_ENABLE
    if(CRC->dma_enable)
    {
        CRC_DMA_Stop(&CRC->dma_cfg);

        /* Power control for DMA */
        if(CRC_DMA_PowerControl(status, &CRC->dma_cfg) != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;
    }
#endif

    return ARM_DRIVER_OK;
}

/**
 @fn           int32_t CRC_Control (uint32_t control,
                                    uint32_t arg,
                                    CRC_RESOURCES *CRC)
 @brief        CMSIS-Driver CRC control.
               Control CRC Interface.
 @param[in]    control : Operation \ref Driver_CRC.h : CRC control codes
 @param[in]    arg     : Argument of operation (optional)
 @param[in]    CRC     : Pointer to CRC resources
 @return       ARM_DRIVER_ERROR_PARAMETER  : if CRC device is invalid
               ARM_DRIVER_OK               : if CRC successfully uninitialized or already not initialized
 */
static int32_t CRC_Control (uint32_t control,
                            uint32_t arg,
                            CRC_RESOURCES *CRC)
{
    int32_t ret = ARM_DRIVER_OK;

    if(CRC->state.initialized == 0)
        return ARM_DRIVER_ERROR;

    if(CRC->state.powered == 0)
        return ARM_DRIVER_ERROR;

    if(control & ARM_CRC_CONTROL_MASK)
    {
        /* To enable or disable the Reflect, Invert, Bit, Byte, Custom polynomial bits of CRC*/
        Control_Bit(control, arg, CRC);
    }

    else
    {
        switch (control)
        {
        case ARM_CRC_ALGORITHM_SEL:

            /* clear 8,16 and 32 bit algorithm */
            crc_clear_algo(CRC->regs);

            /* clear the 8, 16, 32 bit algorithm size */
            crc_clear_algo_size(CRC->regs);

            switch(arg)
            {
            case ARM_CRC_ALGORITHM_SEL_8_BIT_CCITT:

                /* To enable 8 bit CRC algorithm and size */
                crc_enable_8bit(CRC->regs);

            break;

            case ARM_CRC_ALGORITHM_SEL_16_BIT:

                /* To enable 16 bit CRC algorithm and size */
                crc_enable_16bit(CRC->regs);

            break;

            case ARM_CRC_ALGORITHM_SEL_16_BIT_CCITT:

                /* To enable 16 bit CCITT CRC algorithm and size */
                crc_enable_16bit_ccitt(CRC->regs);

            break;

            case ARM_CRC_ALGORITHM_SEL_32_BIT:

                /* To enable 32 bit CRC algorithm and size */
                crc_enable_32bit(CRC->regs);

            break;

            case ARM_CRC_ALGORITHM_SEL_32_BIT_CUSTOM_POLY:

                /* To enable 32 bit poly custom CRC algorithm and size */
                crc_enable_32bit_custom_poly(CRC->regs);

            break;
            default:
            ret = ARM_DRIVER_ERROR_UNSUPPORTED;
            }
       break;
       default:
       ret = ARM_DRIVER_ERROR_UNSUPPORTED;
       }
    }

    return ret;
}

/**
@fn         int32_t CRC_Seed (uint32_t value, CRC_RESOURCES *CRC)
@brief      CMSIS-DRIVER CRC Seed value
            Enable the Init bit [0th bit] of the control register to load the seed value
@param[in]  seed_value : Seed value depending on whether the data is 8 bit or 16 or 32 bit
@param[in]  CRC        : pointer to CRC resources
@return     \ref execution_status
*/
static int32_t CRC_Seed (uint32_t seed_value, CRC_RESOURCES *CRC)
{
    int32_t ret = ARM_DRIVER_OK;

    if(CRC->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Adding 8 bit or 16 bit or 32 bit seed value to the Seed register of CRC */
    crc_set_seed(CRC->regs, seed_value);

    /* Write the Init value in control register to load the Seed value in Seed register */
    crc_enable(CRC->regs);

    return ret;
}

/**
@fn         int32_t CRC_PolyCustom (uint32_t value, CRC_RESOURCES *CRC)
@brief      To add the polynomial value to polycustom register
            Enable the Init bit [0th bit] of the conrol register to load the
            polynomial value
@param[in]  polynomial : Polynomial data for 8 bit or 16 or 32 bit
@param[in]  CRC        : pointer to CRC resources
@return     \ref execution_status
*/
static int32_t CRC_PolyCustom (uint32_t value, CRC_RESOURCES *CRC)
{
    int32_t ret = ARM_DRIVER_OK;

    if(CRC->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Adding Polynomial value to the poly_custom register of CRC */
    crc_set_custom_poly(CRC->regs, value);

    /* Write the Init value in control register to load the polynomial value in Poly_custom register */
    crc_enable(CRC->regs);

    return ret;
}

/**
@fn         int32_t CRC_Compute (const void *data_in, uint32_t len, uint32_t *data_out, CRC_RESOURCES *CRC)
@brief      1.To calculate the CRC result for 8 bit 16 bit and 32 bit CRC algorithm.
            2.For 8 bit and 16 bit CRC algorithm our hardware can able to calculate the CRC
              result for both aligned and unaligned CRC input data by loading the CRC inputs
              in DATA_IN_8 bit register.
            3. For 32 bit CRC our hardware will support for aligned data to calculate the CRC Result.
            4. For unaligned data CRC_calculate_Unaligned function will calculate the CRC result for
               unaligned CRC input
            5. In CRC_calculate_Unaligned function load the aligned CRC result from the hardware ,
               unaligned CRC input,length of unaligned input data and the polynomial for the 32 bit CRC
@param[in]  data_in : it is a pointer which holds the address of user input
            len     : Length of the input data
            data_out : To get the CRC output
@param[in]  CRC  : pointer to CRC resources
@return     \ref execution_status
*/
static int32_t CRC_Compute (const void *data_in, uint32_t len, uint32_t *data_out, CRC_RESOURCES *CRC)
{
    int32_t   ret = ARM_DRIVER_OK;
    uint8_t   algo_size;
    uint32_t  control_val;

    if(CRC->state.powered == 0)
    {
         /* error:Driver is not initialized */
         return ARM_DRIVER_ERROR;
    }

    if(CRC->busy == 1)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    if(data_in == NULL || data_out == NULL || len == 0)
    {
        /* error: pointer is not valid */
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Initialize the transfer params */
    CRC->transfer.aligned_len   = 0U;
    CRC->transfer.unaligned_len = 0U;
    CRC->transfer.data_in       = data_in;
    CRC->transfer.data_out      = data_out;
    CRC->transfer.len           = len;

    /* To check whether the algorithm size is 8 bit or 16 or 32 bit */
    algo_size = (uint8_t)crc_get_algorithm_size(CRC->regs);

    /* Set the busy flag */
    CRC->busy = 1;

    switch(algo_size)
    {
    /* For 8 bit CRC */
    case CRC_8_BIT_SIZE:
#if CRC_DMA_ENABLE
        if(CRC->dma_enable && (CRC->transfer.len > CRC_DMA_MIN_TRANSFER_LEN))
        {
            ret = CRC_DMA_Copy(CRC->transfer.data_in,
                               CRC->transfer.len,
                               algo_size,
                               CRC);
        }
        else
#endif
            crc_calculate_8bit(CRC->regs,
                               CRC->transfer.data_in,
                               CRC->transfer.len,
                               CRC->transfer.data_out);

        break;

    /* For 16 bit CRC */
    case CRC_16_BIT_SIZE:
#if CRC_DMA_ENABLE
        if(CRC->dma_enable && (CRC->transfer.len > CRC_DMA_MIN_TRANSFER_LEN))
        {
            ret = CRC_DMA_Copy(CRC->transfer.data_in,
                               CRC->transfer.len,
                               algo_size,
                               CRC);
        }
        else
#endif
            crc_calculate_16bit(CRC->regs,
                                CRC->transfer.data_in,
                                CRC->transfer.len,
                                CRC->transfer.data_out);

        break;

    /* For 32 bit CRC*/
    case CRC_32_BIT_SIZE:

        CRC->transfer.aligned_len   = len - (len % 4);
        CRC->transfer.unaligned_len = (len % 4);

        control_val = crc_get_control_val(CRC->regs);

        /* Unaligned data is not supported, if Bit swap is disabled */
        if((CRC->transfer.unaligned_len > 0) & !(control_val & CRC_BIT_SWAP))
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }

#if CRC_DMA_ENABLE
        if(CRC->dma_enable && (CRC->transfer.aligned_len > CRC_DMA_MIN_TRANSFER_LEN))
        {
            ret = CRC_DMA_Copy(CRC->transfer.data_in,
                               CRC->transfer.aligned_len,
                               algo_size,
                               CRC);
            if(ret != ARM_DRIVER_OK)
            {
                break;
            }
        }
        else
#endif
        {
            crc_calculate_32bit(CRC->regs,
                                CRC->transfer.data_in,
                                CRC->transfer.aligned_len,
                                CRC->transfer.data_out);

            crc_calculate_32bit_unaligned_sw(CRC->regs, &CRC->transfer);
        }

        break;
    }

#if CRC_DMA_ENABLE
    if(CRC->dma_enable && (CRC->transfer.aligned_len > CRC_DMA_MIN_TRANSFER_LEN))
    {
        /* Wait till we get the DMA callback event */
        if(ret == ARM_DRIVER_OK && !CRC->cb_event)
        {
            while(CRC->dma_event == 0)
            {
                __WFE();
            }

            /* clear busy flag */
            CRC->busy = 0;

            if(CRC->dma_event != ARM_DMA_EVENT_COMPLETE)
                ret = ARM_DRIVER_ERROR;

            CRC->dma_event = 0U;

            /* call user callback */
            if(CRC->cb_event)
                CRC->cb_event(ARM_CRC_COMPUTE_EVENT_DONE);
        }
    }
    else
    {
        /* If the DMA is not used for this transaction, clear busy flag */
        CRC->busy = 0;

        /* call user callback */
        if(CRC->cb_event)
            CRC->cb_event(ARM_CRC_COMPUTE_EVENT_DONE);
    }
#else
    /* If the DMA is not enabled, clear busy flag */
    CRC->busy = 0;

    /* call user callback */
    if(CRC->cb_event)
        CRC->cb_event(ARM_CRC_COMPUTE_EVENT_DONE);
#endif

    return ret;
}

/* CRC0 Driver instance */
#if (RTE_CRC0)

#if RTE_CRC0_DMA_ENABLE
static void CRC0_DMACallback(uint32_t event, int8_t peri_num);
#endif

static CRC_RESOURCES CRC0_RES = {
    .cb_event       = NULL,
    .regs           = (CRC_Type*) CRC0_BASE,
    .state          = {0},
    .busy           = 0,
#if RTE_CRC0_DMA_ENABLE
    .dma_enable         = RTE_CRC0_DMA_ENABLE,
    .dma_irq_priority   = RTE_CRC0_DMA_IRQ_PRI,
    .dma_cb             = CRC0_DMACallback,
    .dma_cfg            =
    {
            .dma_drv        = &ARM_Driver_DMA_(RTE_CRC0_SELECT_DMA),
    }
#endif
};

#if RTE_CRC0_DMA_ENABLE
/**
  \fn          static void  CRC0_DMACallback(uint32_t event, int8_t peri_num)
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \brief       Callback function from DMA for CRC0
*/
static void CRC0_DMACallback(uint32_t event, int8_t peri_num)
{
    CRC_DMACallback(event, peri_num, &CRC0_RES);
}
#endif

/* Function Name: CRC0_Initialize */
static int32_t CRC0_Initialize(ARM_CRC_SignalEvent_t cb_event)
{
    return (CRC_Initialize(&CRC0_RES, cb_event));
}

/* Function Name: CRC0_Uninitialize */
static int32_t CRC0_Uninitialize(void)
{
    return (CRC_Uninitialize(&CRC0_RES));
}

/* Function Name: CRC0_PowerControl */
static int32_t CRC0_PowerControl(ARM_POWER_STATE status)
{
    return (CRC_PowerControl(status, &CRC0_RES));
}

/* Function Name: CRC0_Control */
static int32_t CRC0_Control(uint32_t control, uint32_t arg)
{
    return (CRC_Control(control, arg, &CRC0_RES));
}

/* Function Name: CRC0_Seed */
static int32_t CRC0_Seed(uint32_t value)
{
    return (CRC_Seed(value, &CRC0_RES));
}

/* Function Name: CRC0_PolyCustom */
static int32_t CRC0_PolyCustom(uint32_t value)
{
    return (CRC_PolyCustom(value, &CRC0_RES));
}

/* Function Name: CRC0_Compute */
static int32_t CRC0_Compute(const void *data_in, uint32_t len, uint32_t *data_out)
{
    return (CRC_Compute(data_in, len, data_out, &CRC0_RES));
}

extern ARM_DRIVER_CRC Driver_CRC0;
ARM_DRIVER_CRC Driver_CRC0 = {
    CRC_GetVersion,
    CRC_GetCapabilities,
    CRC0_Initialize,
    CRC0_Uninitialize,
    CRC0_PowerControl,
    CRC0_Control,
    CRC0_Seed,
    CRC0_PolyCustom,
    CRC0_Compute,
};

#endif /* RTE_CRC0 */

/* CRC1 driver instance */
#if (RTE_CRC1)

#if RTE_CRC1_DMA_ENABLE
static void CRC1_DMACallback(uint32_t event, int8_t peri_num);
#endif

static CRC_RESOURCES CRC1_RES = {
    .cb_event       = NULL,
    .regs           = (CRC_Type*) CRC1_BASE,
    .state          = {0},
    .busy           = 0,
#if RTE_CRC1_DMA_ENABLE
    .dma_enable         = RTE_CRC1_DMA_ENABLE,
    .dma_irq_priority   = RTE_CRC1_DMA_IRQ_PRI,
    .dma_cb             = CRC1_DMACallback,
    .dma_cfg            =
    {
            .dma_drv        = &ARM_Driver_DMA_(RTE_CRC1_SELECT_DMA),
    }
#endif
};

#if RTE_CRC1_DMA_ENABLE
/**
  \fn          static void  CRC1_DMACallback(uint32_t event, int8_t peri_num)
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \brief       Callback function from DMA for CRC1
*/
static void CRC1_DMACallback(uint32_t event, int8_t peri_num)
{
    CRC_DMACallback(event, peri_num, &CRC1_RES);
}
#endif
/* Function Name: CRC1_Initialize */
static int32_t CRC1_Initialize(ARM_CRC_SignalEvent_t cb_event)
{
    return (CRC_Initialize(&CRC1_RES, cb_event));
}

/* Function Name: CRC1_Uninitialize */
static int32_t CRC1_Uninitialize(void)
{
    return (CRC_Uninitialize(&CRC1_RES));
}

/* Function Name: CRC1_PowerControl */
static int32_t CRC1_PowerControl(ARM_POWER_STATE status)
{
    return (CRC_PowerControl(status, &CRC1_RES));
}

/* Function Name: CRC1_Control */
static int32_t CRC1_Control(uint32_t control, uint32_t arg)
{
    return (CRC_Control(control, arg, &CRC1_RES));
}

/* Function Name: CRC1_Seed */
static int32_t CRC1_Seed(uint32_t value)
{
    return (CRC_Seed(value, &CRC1_RES));
}

/* Function Name: CRC1_PolyCustom */
static int32_t CRC1_PolyCustom(uint32_t value)
{
    return (CRC_PolyCustom(value, &CRC1_RES));
}

/* Function Name: CRC1_Compute */
static int32_t CRC1_Compute(const void *data_in, uint32_t len, uint32_t *data_out)
{
    return (CRC_Compute(data_in, len, data_out, &CRC1_RES));
}

extern ARM_DRIVER_CRC Driver_CRC1;
ARM_DRIVER_CRC Driver_CRC1 = {
    CRC_GetVersion,
    CRC_GetCapabilities,
    CRC1_Initialize,
    CRC1_Uninitialize,
    CRC1_PowerControl,
    CRC1_Control,
    CRC1_Seed,
    CRC1_PolyCustom,
    CRC1_Compute
};

#endif /* RTE_CRC1 */

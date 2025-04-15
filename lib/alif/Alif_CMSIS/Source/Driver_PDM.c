/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     Driver_PDM.h
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     12-Jan-2023
 * @brief    CMSIS-Driver for PDM.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

/* Project Includes */
#include "Driver_PDM_Private.h"
#include "sys_ctrl_pdm.h"

#define ARM_PDM_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 1)  /*  Driver version */

/*Driver version*/
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_PDM_API_VERSION,
    ARM_PDM_DRV_VERSION
};

/*Driver Capabilities   */
static const ARM_PDM_CAPABILITIES DriverCapabilities = {
    1,  /* supports Mono mode           */
    1,  /* supports synchronous Receive */
    0   /* Reserved ( must be ZERO)     */
};

/**
 @fn           ARM_DRIVER_VERSION PDM_GetVersion(void)
 @brief        get PDM version
 @param        none
 @return       driver version
 */
static ARM_DRIVER_VERSION PDM_GetVersion(void)
{
    return DriverVersion;
}

/**
 @fn           ARM_PDM_CAPABILITIES PDM_GetCapabilities(void)
 @brief        get PDM Capabilites
 @param        none
 @return       driver Capabilites
 */
static ARM_PDM_CAPABILITIES PDM_GetCapabilities(void)
{
    return DriverCapabilities;
}

#if PDM_DMA_ENABLE
/**
  \fn          int32_t PDM_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Initialize DMA for PDM
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
static inline int32_t PDM_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
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
  \fn          int32_t PDM_DMA_PowerControl(ARM_POWER_STATE state,
                                            DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       PowerControl DMA for PDM
  \param[in]   state  Power state
  \param[in]   dma_periph     Pointer to DMA resources
  \return      \ref execution_status
*/
static inline int32_t PDM_DMA_PowerControl(ARM_POWER_STATE state,
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
  \fn          int32_t PDM_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Allocate a channel for PDM
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
static inline int32_t PDM_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
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
  \fn          int32_t PDM_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       De-allocate channel of PDM
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
static inline int32_t PDM_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
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
  \fn          int32_t PDM_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
                                     ARM_DMA_PARAMS *dma_params)
  \brief       Start PDM DMA transfer
  \param[in]   dma_periph     Pointer to DMA resources
  \param[in]   dma_params     Pointer to DMA parameters
  \return      \ref           execution_status
*/
static inline int32_t PDM_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
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
  \fn          int32_t PDM_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Stop PDM DMA transfer
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
static inline int32_t PDM_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
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
  \fn          int32_t PDM_DMA_GetStatus(DMA_PERIPHERAL_CONFIG *dma_periph, uint32_t *count)
  \brief       Status of PDM DMA transfer
  \param[in]   dma_periph   Pointer to DMA resources
  \param[in]   count        Current transfer count
  \return      \ref         execution_status
*/
static inline int32_t PDM_DMA_GetStatus(DMA_PERIPHERAL_CONFIG *dma_periph, uint32_t *count)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Stop transfer */
    status = dma_drv->GetStatus(&dma_periph->dma_handle, count);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}
#endif

/**
 @fn          void PDM_ERROR_IRQ_handler(PDM_RESOURCES *PDM)
 @brief       IRQ handler for the error interrupt
 @param[in]   PDM : Pointer to PDM resources
 @return      none
 */
void PDM_ERROR_IRQ_handler(PDM_RESOURCES *PDM)
{
    pdm_error_detect_irq_handler(PDM->regs);

    /* call user callback */
    PDM->cb_event(ARM_PDM_EVENT_ERROR);
}

/**
 @fn          void PDM_AUDIO_DETECT_IRQ_handler(PDM_RESOURCES *PDM)
 @brief       IRQ handler for the audio detect interrupt
 @param[in]   PDM : Pointer to PDM resources
 @return      none
 */
void PDM_AUDIO_DETECT_IRQ_handler(PDM_RESOURCES *PDM)
{
    pdm_transfer_t *transfer = &PDM->transfer;

    pdm_audio_detect_irq_handler(PDM->regs, transfer);

    if(transfer->status == PDM_AUDIO_STATUS_DETECTION)
    {
        transfer->status  = PDM_CAPTURE_STATUS_NONE;

        /* call user callback */
        PDM->cb_event(ARM_PDM_EVENT_AUDIO_DETECTION);
    }
}

/**
 @fn          void PDM_WARNING_IRQ_handler(PDM_RESOURCES *PDM)
 @brief       IRQ handler for the PDM warning interrupt
 @param[in]   PDM : Pointer to PDM resources
 @return      none
 */
void PDM_WARNING_IRQ_handler(PDM_RESOURCES *PDM)
{
    pdm_transfer_t *transfer = &(PDM->transfer);

    pdm_warning_irq_handler(PDM->regs, transfer);

    if(transfer->status ==  PDM_CAPTURE_STATUS_COMPLETE)
    {
        transfer->status  = PDM_CAPTURE_STATUS_NONE;

        /* call user callback */
        PDM->cb_event(ARM_PDM_EVENT_CAPTURE_COMPLETE);
    }
}

#if PDM_DMA_ENABLE
/**
 * @fn         void PDMx_DMACallback(uint32_t event, int8_t peri_num, PDM_RESOURCES *PDM)
 * @brief      DMA Callback function for PDM.
 * @note       none.
 * @param[in]  PDM : Pointer to pdm resources structure.
 * @param[in]  event : Event from DMA
 * @param[in]  peri_num : peripheral request number
 * @retval     none
 */
static void PDMx_DMACallback(uint32_t event, int8_t peri_num, PDM_RESOURCES *PDM)
{
    ARG_UNUSED(peri_num);

    if(!PDM->cb_event)
        return;

    if(event & ARM_DMA_EVENT_COMPLETE)
    {
        /* Disable the PDM error irq */
        pdm_disable_error_irq(PDM->regs);

        PDM->cb_event(ARM_PDM_EVENT_CAPTURE_COMPLETE);
    }

    /* Abort Occurred */
    if(event & ARM_DMA_EVENT_ABORT)
    {
        /*
        * There is no event for indicating error in PDM driver.
        * Let the application get timeout and restart the PDM.
        *
        */
    }
}
#endif

/**
 * @fn      ARM_PDM_STATUS PDMx_GetStatus(PDM_RESOURCES *PDM)
 * @brief   CMSIS-Driver PDM get status
 * @note    none.
 * @param   PDM    : Pointer to PDM resources structure
 * @retval  ARM_PDM_STATUS
 */
static ARM_PDM_STATUS PDMx_GetStatus(PDM_RESOURCES *PDM)
{
    return PDM->status;
}

/**
@fn          int32_t PDMx_Initialize(ARM_PDM_SignalEvent_t cb_event, PDM_RESOURCES *PDM)
@brief       Initialize the PDM interface
@param[in]   PDM : Pointer to PDM resources
 @return     ARM_DRIVER_ERROR_PARAMETER : if PDM device is invalid
             ARM_DRIVER_OK              : if PDM successfully initialized or already initialized
 */
static int32_t PDMx_Initialize(ARM_PDM_SignalEvent_t cb_event, PDM_RESOURCES *PDM)
{
    if(!cb_event)
        return ARM_DRIVER_ERROR_PARAMETER;

    if(PDM->state.initialized == 1)
    {
        return ARM_DRIVER_OK;
    }

    /* User call back Event */
    PDM->cb_event = cb_event;

#if PDM_DMA_ENABLE
    if(PDM->dma_enable)
    {
        PDM->dma_cfg->dma_rx.dma_handle = -1;

        /* Initialize DMA for PDM-Rx */
        if(PDM_DMA_Initialize(&PDM->dma_cfg->dma_rx)!= ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
    }
#endif

    /* Setting the state */
    PDM->state.initialized = 1;

    return ARM_DRIVER_OK;
}

/**
@fn          int32_t PDMx_Uninitialize(PDM_RESOURCES *PDM)
@brief       UnInitialize the PDM interface
@param[in]   PDM : Pointer to PDM resources
 @return     ARM_DRIVER_ERROR_PARAMETER : if PDM device is invalid
             ARM_DRIVER_OK              : if PDM successfully initialized or already initialized
 */
static int32_t PDMx_Uninitialize(PDM_RESOURCES *PDM)
{
    if(PDM->state.initialized == 0)
        return ARM_DRIVER_OK;

    /* set call back to NULL */
    PDM->cb_event = NULL;

#if PDM_DMA_ENABLE
    if(PDM->dma_enable)
    {
        PDM->dma_cfg->dma_rx.dma_handle = -1;
    }
#endif

    /* Reset the state */
    PDM->state.initialized = 0U;

    return ARM_DRIVER_OK;
}

/**
 @fn          int32_t PDMx_Channel_Config(PDM_CH_CONFIG *cnfg, PDM_RESOURCES *PDM)
 @brief       PDM channel configurations
 @param[in]   PDM : Pointer to PDM resources
 @param[in]   cngf : Pointer to PDM_CH_CONFIG
 @return      ARM_DRIVER_OK : if function return successfully
 */
static int32_t PDMx_Channel_Config(PDM_CH_CONFIG *cnfg, PDM_RESOURCES *PDM)
{
    if(PDM->state.initialized == 0)
        return ARM_DRIVER_ERROR;

    if(PDM->state.powered == 0)
        return ARM_DRIVER_ERROR;

    /* Store the fir coefficient values */
    pdm_set_fir_coeff(PDM->regs, cnfg->ch_num, cnfg->ch_fir_coef);

    /* Store the iir coefficient values */
    pdm_set_ch_iir_coef(PDM->regs, cnfg->ch_num, cnfg->ch_iir_coef);

    return ARM_DRIVER_OK;
}


/**
 @fn           int32_t PDMx_PowerControl (ARM_POWER_STATE state,
                                          PDM_RESOURCES *PDM)
 @brief        CMSIS-DRIVER PDM power control
 @param[in]    state : Power state
 @param[in]    PDM   : Pointer to PDM resources
 @return       ARM_DRIVER_ERROR_PARAMETER  : if PDM device is invalid
               ARM_DRIVER_OK               : if PDM successfully uninitialized or already not initialized
 */
static int32_t PDMx_PowerControl(ARM_POWER_STATE state,
                                PDM_RESOURCES *PDM)
{
    if(PDM->state.initialized == 0)
        return ARM_DRIVER_ERROR;

    switch(state)
    {
    case ARM_POWER_OFF:

        /* Clear the fifo clear bit */
        pdm_disable_fifo_clear(PDM->regs);

        if(PDM->instance == PDM_INSTANCE_LPPDM)
        {
            /* Clear Any Pending IRQ */
            NVIC_ClearPendingIRQ(PDM->warning_irq);

            /* Disable the NIVC */
            NVIC_DisableIRQ(PDM->warning_irq);

            /* Disable LPPDM clock */
            disable_lppdm_periph_clk();
        }
        else
        {
            /* Clear Any Pending IRQ */
            NVIC_ClearPendingIRQ(PDM->warning_irq);
            NVIC_ClearPendingIRQ(PDM->error_irq);
            NVIC_ClearPendingIRQ(PDM->audio_detect_irq);

            /* Disable the NIVC */
            NVIC_DisableIRQ(PDM->warning_irq);
            NVIC_DisableIRQ(PDM->error_irq);
            NVIC_DisableIRQ(PDM->audio_detect_irq);

            /* Disable PDM clock */
            disable_pdm_periph_clk();
        }

#if PDM_DMA_ENABLE
       if(PDM->dma_enable)
       {
            /* Disable the DMA handshake */
            pdm_dma_handshake(PDM->regs, false);

            /* DeAllocate DMA for PDM-Rx */
            if(PDM_DMA_DeAllocate(&PDM->dma_cfg->dma_rx) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }
            /* Power Control DMA for PDM-Rx */
            if(PDM_DMA_PowerControl(state, &PDM->dma_cfg->dma_rx) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }
      }
#endif

        /* Reset the power status of PDM */
        PDM->state.powered = 0;

    break;

    case ARM_POWER_FULL:

        if(PDM->state.initialized == 0)
            return ARM_DRIVER_ERROR;

        if(PDM->state.powered == 1)
            return ARM_DRIVER_OK;

        if(PDM->instance == PDM_INSTANCE_LPPDM)
        {
            /* Clear Any Pending IRQ */
            NVIC_ClearPendingIRQ(PDM->warning_irq);

            /* Set priority */
            NVIC_SetPriority (PDM->warning_irq, PDM->warning_irq_priority);

            /* Enable the NIVC */
            NVIC_EnableIRQ(PDM->warning_irq);

            /* Enable LPPDM clock */
            enable_lppdm_periph_clk();
        }
        else
        {
            /* Clear Any Pending IRQ */
            NVIC_ClearPendingIRQ(PDM->warning_irq);
            NVIC_ClearPendingIRQ(PDM->error_irq);
            NVIC_ClearPendingIRQ(PDM->audio_detect_irq);

            /* Set priority */
            NVIC_SetPriority (PDM->warning_irq, PDM->warning_irq_priority);
            NVIC_SetPriority(PDM->error_irq, PDM->error_irq_priority);
            NVIC_SetPriority(PDM->audio_detect_irq, PDM->audio_irq_priority);

            /* Enable the NIVC */
            NVIC_EnableIRQ(PDM->warning_irq);
            NVIC_EnableIRQ(PDM->error_irq);
            NVIC_EnableIRQ(PDM->audio_detect_irq);

            /* Enable PDM clock */
            enable_pdm_periph_clk();
        }

        /* Set the FIFO clear bit */
        pdm_enable_fifo_clear(PDM->regs);

        /* Set the fifo watermark value */
        pdm_set_fifo_watermark(PDM->regs, PDM->fifo_watermark);

#if PDM_DMA_ENABLE
        if(PDM->dma_enable)
        {
            /* Power Control DMA for PDM-Rx */
            if(PDM_DMA_PowerControl(state, &PDM->dma_cfg->dma_rx) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }
            /* Allocate DMA for PDM-Rx */
            if(PDM_DMA_Allocate(&PDM->dma_cfg->dma_rx) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }

            /* Enable the DMA handshake */
            pdm_dma_handshake(PDM->regs, true);
      }
#endif

        /* Set the power state enabled */
        PDM->state.powered = 1;

    break;

    case ARM_POWER_LOW:

    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/**
 @fn           int32_t PDMx_Control (uint32_t control,
                                    uint32_t arg1, uint32_t arg2,
                                    PDM_RESOURCES *PDM)
 @brief        CMSIS-Driver PDM control.
               Control PDM Interface.
 @param[in]    control : Operation \ref Driver_PDM.h : PDM control codes
 @param[in]    arg1     : Argument of operation (optional)
  @param[in]   arg2     : Argument of operation (optional)
 @param[in]    PDM     : Pointer to PDM resources
 @return       ARM_DRIVER_ERROR_PARAMETER  : if PDM device is invalid
               ARM_DRIVER_OK               : if PDM successfully uninitialized or already not initialized
 */
static int32_t PDMx_Control (uint32_t control,
                             uint32_t arg1,
                             uint32_t arg2,
                             PDM_RESOURCES *PDM)
{
    /* Verify whether the driver is initialized and powered*/
    if(PDM->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    switch (control)
    {
    case ARM_PDM_SELECT_RESOLUTION:

        if(arg1 != ARM_PDM_16BIT_RESOLUTION)
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }

        break;

    case ARM_PDM_CHANNEL_PHASE:

        if(arg1 > PDM_MAX_CHANNEL || arg2 > PDM_MAX_PHASE_CTRL)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* Store the channel phase control values */
        pdm_set_ch_phase(PDM->regs, arg1, arg2 );

        break;

    case ARM_PDM_CHANNEL_GAIN:

        if(arg1 > PDM_MAX_CHANNEL || arg2 > PDM_MAX_GAIN_CTRL)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* Store the Gain value */
        pdm_set_ch_gain(PDM->regs, arg1, arg2 );

        break;

    case ARM_PDM_CHANNEL_PEAK_DETECT_TH:

        if(arg1 > PDM_MAX_CHANNEL)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* Store the Peak Detector Threshold */
        pdm_set_peak_detect_th(PDM->regs, arg1, arg2 );

        break;

    case ARM_PDM_CHANNEL_PEAK_DETECT_ITV:

        if(arg1 > PDM_MAX_CHANNEL)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* Store the Peak Detector Interval */
        pdm_set_peak_detect_itv(PDM->regs, arg1, arg2 );

        break;

    case ARM_PDM_SELECT_CHANNEL:

        if(arg2 != NULL)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* Clear PDM channels */
        pdm_clear_channel(PDM->regs);

        /* Enable PDM multi channel */
        pdm_enable_multi_ch(PDM->regs, arg1);

        break;

    case ARM_PDM_MODE:

        if(arg2 != NULL)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* Clear the PDM modes */
        pdm_clear_modes(PDM->regs);

        /* Select the PDM modes */
        pdm_enable_modes(PDM->regs, arg1);

        break;

    case  ARM_PDM_BYPASS_IIR_FILTER:

        if(arg2 != NULL)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* To select the Bypass IIR filter */
        pdm_bypass_iir(PDM->regs, arg1);

        break;

    case ARM_PDM_BYPASS_FIR_FILTER:

        if(arg2 != NULL)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* To select the Bypass FIR filter */
        pdm_bypass_fir(PDM->regs, arg1);

        break;

    case ARM_PDM_PEAK_DETECTION_NODE:

        if(arg2 != NULL)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* To select the peak detect node */
        pdm_peak_detect(PDM->regs, arg1);

        break;

    case ARM_PDM_SAMPLE_ADVANCE:

        if(arg2 != NULL)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* To select the sample advance */
        pdm_sample_advance(PDM->regs, arg1);

        break;
    }
    return ARM_DRIVER_OK;
}

/**
@fn         int32_t PDMx_Receive(void *data, uint32_t num, PDM_RESOURCES *PDM)
@brief      -> clear and set the fifo clear bit
            -> Store the user enabled channel
            -> Store the fifo watermark value
            -> Enable the PDM IRQ
@param[in]  data     : Pointer storing buffer address
@param[in]  num      : Total number of samples
@param[in]  PDM      : Pointer to PDM resources
@return     ARM_DRIVER_OK : if function return successfully
*/
static int32_t PDMx_Receive(void *data, uint32_t num, PDM_RESOURCES *PDM)
{
    if(PDM->state.initialized == 0)
        return ARM_DRIVER_ERROR;

    if(PDM->state.powered == 0)
        return ARM_DRIVER_ERROR;

    /* clear the fifo clear bit */
    pdm_disable_fifo_clear(PDM->regs);

    PDM->transfer.total_cnt  = num;
    PDM->transfer.buf        = data;
    PDM->transfer.curr_cnt   = 0;
    PDM->status.rx_busy      = 1;
    PDM->status.rx_overflow  = 0;

#if PDM_DMA_ENABLE
    if(PDM->dma_enable)
    {
        uint32_t audio_ch;
        uint8_t channel_count = 0;
        ARM_DMA_PARAMS dma_params;

        audio_ch = pdm_get_active_channels(PDM->regs);

        /* Start the DMA engine for sending the data to PDM */
        dma_params.peri_reqno    = (int8_t)PDM->dma_cfg->dma_rx.dma_periph_req;
        dma_params.dir           = ARM_DMA_DEV_TO_MEM;
        dma_params.cb_event      = PDM->dma_cb;

        if(audio_ch & PDM_CHANNEL_0_1)
        {
            dma_params.src_addr = (void *)pdm_get_ch0_1_addr(PDM->regs);
            dma_params.dst_addr = data;
            channel_count++;
        }
        if(audio_ch & PDM_CHANNEL_2_3)
        {
            dma_params.src_addr = (void *)pdm_get_ch2_3_addr(PDM->regs);
            dma_params.dst_addr = data;
            channel_count++;
        }
        if(audio_ch & PDM_CHANNEL_4_5)
        {
            dma_params.src_addr = (void *)pdm_get_ch4_5_addr(PDM->regs);
            dma_params.dst_addr = data;
            channel_count++;
        }
        if(audio_ch & PDM_CHANNEL_6_7)
        {
            dma_params.src_addr = (void *)pdm_get_ch6_7_addr(PDM->regs);
            dma_params.dst_addr = data;
            channel_count++;
        }

        if(channel_count > PDM_MAX_DMA_CHANNEL)
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }

        /* Enable PDM DMA */
        pdm_dma_enable_irq(PDM->regs);

        /* Each PCM sample is represented by 16-bits resolution (2 bytes) */
        dma_params.num_bytes    = num * 2;
        dma_params.irq_priority = PDM->dma_irq_priority;
        dma_params.burst_len    = 1;
        dma_params.burst_size   = BS_BYTE_4;

        /* Start DMA transfer */
        if(PDM_DMA_Start(&PDM->dma_cfg->dma_rx, &dma_params) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
    }
    else
#endif
    {
#if PDM_BLOCKING_MODE_ENABLE

        /* Check if blocking mode (polling) is enabled */
        if(PDM->blocking_mode)
        {
            pdm_transfer_t *transfer = &(PDM->transfer);

            /* Block execution until PDM receives all PCM samples */
            pdm_receive_blocking(PDM->regs, transfer);

            /* Check for error detection status */
            if(transfer->status == PDM_ERROR_DETECT)
            {
                PDM->status.rx_overflow = 1U;
                transfer->status  = PDM_CAPTURE_STATUS_NONE;
            }

            /* Check for capture complete status */
            if(transfer->status == PDM_CAPTURE_STATUS_COMPLETE)
            {
                PDM->status.rx_busy = 0U;
                transfer->status  = PDM_CAPTURE_STATUS_NONE;
            }
        }
        else
#endif
        {
            /* Enable irq */
            pdm_enable_irq(PDM->regs);
        }
    }
    return ARM_DRIVER_OK;
}

/* RTE_PDM */
#if RTE_PDM

#if RTE_PDM_DMA_ENABLE
static void PDM_DMACallback(uint32_t event, int8_t peri_num);
static PDM_DMA_HW_CONFIG PDM0_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv         = &ARM_Driver_DMA_(PDM_DMA),
        .dma_periph_req  = PDM_DMA_PERIPH_REQ,
        .evtrtr_cfg =
        {
            .instance    = PDM_DMA,
            .group       = PDM_DMA_GROUP,
            .channel     = PDM_DMA_PERIPH_REQ,
            .enable_handshake = PDM_DMA_HANDSHAKE_ENABLE
        },
    }
};
#endif

static PDM_RESOURCES PDM = {
    .cb_event              = NULL,
    .regs                  = (PDM_Type *)PDM_BASE,
    .transfer              = {0},
    .state                 = {0},
    .instance              = PDM_INSTANCE_PDM0,
    .fifo_watermark        = RTE_PDM_FIFO_WATERMARK,
    .status                = {0},
    .error_irq             = (IRQn_Type)PDM_ERROR_IRQ_IRQn,
    .warning_irq           = (IRQn_Type)PDM_WARN_IRQ_IRQn,
    .audio_detect_irq      = (IRQn_Type)PDM_AUDIO_DET_IRQ_IRQn,
    .error_irq_priority    = (uint32_t)RTE_PDM_IRQ_PRIORITY,
    .warning_irq_priority  = (uint32_t)RTE_PDM_IRQ_PRIORITY,
    .audio_irq_priority    = (uint32_t)RTE_PDM_IRQ_PRIORITY,
#if RTE_PDM_DMA_ENABLE
    .dma_cb                = PDM_DMACallback,
    .dma_cfg               = &PDM0_DMA_HW_CONFIG,
    .dma_enable            = RTE_PDM_DMA_ENABLE,
    .dma_irq_priority      = RTE_PDM_DMA_IRQ_PRIORITY,
#endif
#if PDM_BLOCKING_MODE_ENABLE
    .blocking_mode         = RTE_PDM_BLOCKING_MODE_ENABLE
#endif
};

/* Function Name: PDM_Initialize */
static int32_t PDM_Initialize(ARM_PDM_SignalEvent_t cb_event)
{
    return (PDMx_Initialize(cb_event, &PDM));
}

/* Function Name: PDM_Uninitialize */
static int32_t PDM_Uninitialize(void)
{
    return (PDMx_Uninitialize(&PDM));
}

/* Function Name: PDM_PowerControl */
static int32_t PDM_PowerControl(ARM_POWER_STATE state)
{
    return (PDMx_PowerControl(state, &PDM));
}

/* Function Name: PDM_Channel_Config */
static int32_t PDM_Channel_Config(PDM_CH_CONFIG *cnfg)
{
    return (PDMx_Channel_Config(cnfg, &PDM));
}

/* Function Name: PDM_Control */
static int32_t PDM_Control(uint32_t control, uint32_t arg1, uint32_t arg2)
{
    return (PDMx_Control(control, arg1, arg2, &PDM));
}

/* Function Name: PDM_Receive */
static int32_t PDM_Receive(void *data, uint32_t num)
{
    return (PDMx_Receive(data, num, &PDM));
}

/*Function Name : PDM_WARNNING_IRQHANDLER */
void PDM_WARN_IRQHandler (void)
{
    PDM_WARNING_IRQ_handler(&PDM);
}

/*Function Name : PDM_ERROR_IRQHandler */
void PDM_ERROR_IRQHandler (void)
{
    PDM_ERROR_IRQ_handler(&PDM);
}

/*Function Name : PDM_AUDIO_DET_IRQHandler */
void PDM_AUDIO_DET_IRQHandler (void)
{
    PDM_AUDIO_DETECT_IRQ_handler(&PDM);
}

/* Function Name: PDM_GetStatus */
static ARM_PDM_STATUS PDM_GetStatus(void)
{
    return PDMx_GetStatus(&PDM);
}

#if RTE_PDM_DMA_ENABLE
void PDM_DMACallback(uint32_t event, int8_t peri_num)
{
    PDMx_DMACallback(event, peri_num, &PDM);
}
#endif

extern ARM_DRIVER_PDM Driver_PDM;
ARM_DRIVER_PDM Driver_PDM = {
    PDM_GetVersion,
    PDM_GetCapabilities,
    PDM_Initialize,
    PDM_Uninitialize,
    PDM_PowerControl,
    PDM_Control,
    PDM_Channel_Config,
    PDM_Receive,
    PDM_GetStatus
};
#endif /* RTE_PDM */

/* RTE_LPPDM */
#if RTE_LPPDM

#if RTE_LPPDM_DMA_ENABLE

static void LPPDM_DMACallback(uint32_t event, int8_t peri_num);
static PDM_DMA_HW_CONFIG LPPDM_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv         = &ARM_Driver_DMA_(LPPDM_DMA),
        .dma_periph_req  = LPPDM_DMA_PERIPH_REQ,
        .evtrtr_cfg =
        {
            .instance    = LPPDM_DMA,
            .group       = LPPDM_DMA_GROUP,
            .channel     = LPPDM_DMA_PERIPH_REQ,
            .enable_handshake = LPPDM_DMA_HANDSHAKE_ENABLE
        },
    }
};
#endif

static PDM_RESOURCES LPPDM  = {
    .cb_event              = NULL,
    .regs                  = (PDM_Type *)LPPDM_BASE,
    .transfer              = {0},
    .state                 = {0},
    .instance              = PDM_INSTANCE_LPPDM,
    .fifo_watermark        = RTE_LPPDM_FIFO_WATERMARK,
    .warning_irq           = (IRQn_Type)LPPDM_IRQ_IRQn,
    .warning_irq_priority  = (uint32_t)RTE_LPPDM_IRQ_PRIORITY,
#if RTE_LPPDM_DMA_ENABLE
    .dma_cb                = LPPDM_DMACallback,
    .dma_cfg               = &LPPDM_DMA_HW_CONFIG,
    .dma_enable            = RTE_LPPDM_DMA_ENABLE,
    .dma_irq_priority      = RTE_LPPDM_DMA_IRQ_PRIORITY,
#endif
#if PDM_BLOCKING_MODE_ENABLE
    .blocking_mode         = RTE_LPPDM_BLOCKING_MODE_ENABLE
#endif
};

/* Function Name: LPPDM_Initialize */
static int32_t LPPDM_Initialize(ARM_PDM_SignalEvent_t cb_event)
{
    return (PDMx_Initialize(cb_event, &LPPDM));
}

/* Function Name: LPPDM_Uninitialize */
static int32_t LPPDM_Uninitialize(void)
{
    return (PDMx_Uninitialize(&LPPDM));
}

/* Function Name: LPPDM_PowerControl */
static int32_t LPPDM_PowerControl(ARM_POWER_STATE status)
{
    return (PDMx_PowerControl(status, &LPPDM));
}

/* Function Name: LPPDM_Control */
static int32_t LPPDM_Control(uint32_t control, uint32_t arg1, uint32_t arg2)
{
    return (PDMx_Control(control, arg1, arg2, &LPPDM));
}

/* Function Name: LPPDM_Capture */
static int32_t LPPDM_Receive(void *data, uint32_t num)
{
    return (PDMx_Receive(data, num, &LPPDM));
}

/* Function Name: LPPDM_Channel_Config */
static int32_t LPPDM_Channel_Config(PDM_CH_CONFIG *cnfg)
{
    return (PDMx_Channel_Config(cnfg, &LPPDM));
}

/*Function Name : LPPDM_IRQHandler */
void LPPDM_IRQHandler (void)
{
    PDM_WARNING_IRQ_handler(&LPPDM);
}

/* Function Name: LPPDM_GetStatus */
static ARM_PDM_STATUS LPPDM_GetStatus(void)
{
    return PDMx_GetStatus(&LPPDM);
}

#if RTE_LPPDM_DMA_ENABLE
void LPPDM_DMACallback(uint32_t event, int8_t peri_num)
{
    PDMx_DMACallback(event, peri_num, &LPPDM);
}
#endif

extern ARM_DRIVER_PDM Driver_LPPDM;
ARM_DRIVER_PDM Driver_LPPDM = {
    PDM_GetVersion,
    PDM_GetCapabilities,
    LPPDM_Initialize,
    LPPDM_Uninitialize,
    LPPDM_PowerControl,
    LPPDM_Control,
    LPPDM_Channel_Config,
    LPPDM_Receive,
    LPPDM_GetStatus
};
#endif /* RTE_LPPDM */

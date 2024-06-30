/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
#include <Driver_MIPI_CSI2.h>
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
/**************************************************************************//**
 * @file     Driver_MIPI_CSI2.c
 * @author   Prasanna Ravi and Chandra Bhushan Singh
 * @email    prasanna.ravi@alifsemi.com and chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     15-April-2023
 * @brief    CMSIS-Driver for MIPI CSI2.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#include <math.h>

/* System Includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

/* CSI includes */
#include "Driver_MIPI_CSI2.h"
#include "csi.h"
#include "DPHY_CSI2.h"
#include "sys_ctrl_csi.h"
#include "Driver_CSI_Private.h"
#include "Camera_Sensor.h"

#if !(RTE_MIPI_CSI2)
#error "MIPI CSI2 is not enabled in the RTE_Device.h"
#endif
#if (!defined(RTE_Drivers_MIPI_CSI2))
#error "MIPI CSI2 not configured in RTE_Components.h!"
#endif

#define ARM_MIPI_CSI2_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)

/*Driver version*/
static const ARM_DRIVER_VERSION DriverVersion =
{
    ARM_MIPI_CSI2_API_VERSION,
    ARM_MIPI_CSI2_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_MIPI_CSI2_CAPABILITIES DriverCapabilities =
{
    0, /* Not supports reentrant_operation */
    1, /* IPI interface supported*/
    0, /* IDI interface not supported*/
    0  /* reserved (must be zero) */
};

/* CSI, CPI related Data mode settings table */
static const CSI_CPI_DATA_MODE_SETTINGS cpi_data_mode_settings[] =
{
    {CSI_DT_RAW6, CSI_IPI_COLOR_COM_TYPE_COLOR16, CPI_COLOR_MODE_CONFIG_IPI16_RAW6, CPI_DATA_MODE_BIT_8, 8},
    {CSI_DT_RAW7, CSI_IPI_COLOR_COM_TYPE_COLOR16, CPI_COLOR_MODE_CONFIG_IPI16_RAW7, CPI_DATA_MODE_BIT_8, 8},
    {CSI_DT_RAW8, CSI_IPI_COLOR_COM_TYPE_COLOR16, CPI_COLOR_MODE_CONFIG_IPI16_RAW8, CPI_DATA_MODE_BIT_8, 8},
    {CSI_DT_RAW10, CSI_IPI_COLOR_COM_TYPE_COLOR16, CPI_COLOR_MODE_CONFIG_IPI16_RAW10, CPI_DATA_MODE_BIT_16, 10},
    {CSI_DT_RAW12, CSI_IPI_COLOR_COM_TYPE_COLOR16, CPI_COLOR_MODE_CONFIG_IPI16_RAW12, CPI_DATA_MODE_BIT_16, 12},
    {CSI_DT_RAW14, CSI_IPI_COLOR_COM_TYPE_COLOR16, CPI_COLOR_MODE_CONFIG_IPI16_RAW14, CPI_DATA_MODE_BIT_16, 14},
    {CSI_DT_RAW16, CSI_IPI_COLOR_COM_TYPE_COLOR16, CPI_COLOR_MODE_CONFIG_IPI16_RAW16, CPI_DATA_MODE_BIT_16, 16},
    {CSI_DT_RGB444, CSI_IPI_COLOR_COM_TYPE_COLOR48, CPI_COLOR_MODE_CONFIG_IPI48_RGB444, CPI_DATA_MODE_BIT_16, 16},
    {CSI_DT_RGB555, CSI_IPI_COLOR_COM_TYPE_COLOR48, CPI_COLOR_MODE_CONFIG_IPI48_RGB555, CPI_DATA_MODE_BIT_16, 16},
    {CSI_DT_RGB565, CSI_IPI_COLOR_COM_TYPE_COLOR48, CPI_COLOR_MODE_CONFIG_IPI48_RGB565, CPI_DATA_MODE_BIT_16, 16},
    {CSI_DT_RGB666, CSI_IPI_COLOR_COM_TYPE_COLOR48, CPI_COLOR_MODE_CONFIG_IPI48_RGB666, CPI_DATA_MODE_BIT_32, 18},
    {CSI_DT_RGB888, CSI_IPI_COLOR_COM_TYPE_COLOR48, CPI_COLOR_MODE_CONFIG_IPI48_XRGB888, CPI_DATA_MODE_BIT_32, 24},
};

/* CSI, CPI config informations */
static CPI_INFO cpi_info;

/**
  \fn          ARM_DRIVER_VERSION MIPI_CSI2_GetVersion (void)
  \brief       Get MIPI CSI2 driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION MIPI_CSI2_GetVersion (void)
{
    return DriverVersion;
}

/**
  \fn          ARM_MIPI_CSI2_CAPABILITIES MIPI_CSI2_GetCapabilities (void)
  \brief       Get MIPI CSI2 driver capabilities
  \return      \ref ARM_MIPI_DPHY_CAPABILITIES
*/
static ARM_MIPI_CSI2_CAPABILITIES MIPI_CSI2_GetCapabilities (void)
{
    return DriverCapabilities;
}

/**
  \fn          int32_t CSI2_Initialize (ARM_MIPI_CSI2_SignalEvent_t cb_event,
                                        CSI_RESOURCES *CSI2)
  \brief       Initialize MIPI CSI2 Interface.
  \param[in]   cb_event Pointer to ARM_MIPI_CSI2_SignalEvent_t
  \param[in]   CSI2 Pointer to CSI resources
  \return      \ref execution_status
*/
static int32_t CSI2_Initialize (ARM_MIPI_CSI2_SignalEvent_t cb_event,
                                CSI_RESOURCES *CSI2)
{
    int32_t ret = ARM_DRIVER_OK;
    CAMERA_SENSOR_DEVICE *camera_sensor;
    CSI_IPI_INFO *ipi_info = CSI2->ipi_info;
    CSI_FRAME_INFO *frame_info = ipi_info->frame_info;
    CSI_INFO *csi_info;
    unsigned int index;
    float pixclock;

    if(CSI2->status.initialized == 1)
    {
        /* Driver already initialized */
        return ARM_DRIVER_OK;
    }

    if (!cb_event)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    camera_sensor = Get_Camera_Sensor();

    if (!(camera_sensor && camera_sensor->csi_info))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if(camera_sensor->interface != CAMERA_SENSOR_INTERFACE_MIPI)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    csi_info = camera_sensor->csi_info;

    /* Get Data type related informations */
    for( index = 0; index < ARRAY_SIZE(cpi_data_mode_settings) &&
         (cpi_data_mode_settings[index].data_type != csi_info->dt);
         index++);

    if(cpi_data_mode_settings[index].data_type != csi_info->dt)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    CSI2->pixel_data_type  = csi_info->dt;
    CSI2->n_lanes = csi_info->n_lanes;
    CSI2->vc_id = csi_info->vc_id;
    ipi_info->ipi_color_com = cpi_data_mode_settings[index].ipi_color_com;

    /* Balancing bandwidth by making output bandwidth is 20% greater then input bandwidth
     * pixel clock = (mipi_clk_freq * number_of_lane * 2 "DDR" * 1.2f "percentage" ) / (bits_per_pixel) */
    pixclock = (csi_info->frequency *
                csi_info->n_lanes * 2 * 1.2f) / cpi_data_mode_settings[index].bpp;

    /* Pixel clock divider */
    CSI2->csi_pixclk_div = (int)(((RTE_CSI2_PIX_CLK_SEL ? (float)PLL_CLK3 : (float)(PLL_CLK1/2))/pixclock) + 0.5f);

    /* Timing calculation for Camera mode */
    if(ipi_info->ipi_mode == CSI_IPI_MODE_CAM_TIMIMG)
    {
        float time_PPI_ns = (8.0f / csi_info->frequency) * 1000000000;
        float time_IPI_ns = (1.0f / pixclock) * 1000000000;
        float pkt2pkt_time_ns = csi_info->pkt2pkt_time.time_ns;
        uint32_t bytes_to_transmit = (camera_sensor->width * cpi_data_mode_settings[index].bpp) / 8;
        uint32_t mem_req_1, mem_req_2, min_memory_depth;

        if (!csi_info->pkt2pkt_time.line_sync_pkt_enable)
        {
            csi_info->pkt2pkt_time.time_ns = 0;
        }

        /* The rising edge of VSYNC comes at least 3 data clock cycles prior to DATA_EN (alias to HSYNC), So we adding 3 for HSA */
        frame_info->hsa_time = CSI2_HSA_MIN + 1;

        /* For the pixel data to be processed correctly, the back porch time must not be less than the time between the
         * packets plus some margin.
         * (IPI_HSA_TIME + IPI_HBP_TIME)*TIPI > Pkt2PktTime + (2*ShortPktBytes/N_LANES + 2)*TPPI-HS/BytesPerHsClk
         * IPI_HBP_TIME = ((((Pkt2PktTime + (2*ShortPktBytes/N_LANES + 2)*TPPI-HS/BytesPerHsClk) / TIPI) - IPI_HSA_TIME) + 1)
         */
        frame_info->hbp_time = ((ceil((pkt2pkt_time_ns + (2* CSI2_SHORT_PKT_BYTES / csi_info->n_lanes + 2)
                                   * time_PPI_ns / CSI2_BYTES_PER_HS_CLK) / time_IPI_ns) - frame_info->hsa_time) + 1);

        if(frame_info->hbp_time < CSI2_HBP_MIN)
        {
            frame_info->hbp_time = CSI2_HBP_MIN;
        }

        /* FIFO underflow occurs when the IPI has a higher throughput than the PHY. Configure the Horizontal Sync Delay (HSD)
         * to avoid FIFO underflow.
         * IPI_HSD_TIME > (Pkt2PktTime + (((LongPktBytes + Bytes2Transmit)/N_LANES)*TPPI-HS/BytesPerHsClk))/TIPI
         *                 - (IPI_HSA_TIME + IPI_HBP_TIME + IPI_HACT_TIME)
         * IPI_HSD_TIME = ((Pkt2PktTime + (((LongPktBytes + Bytes2Transmit)/N_LANES)*TPPI-HS/BytesPerHsClk))/TIPI
         *                 - (IPI_HSA_TIME + IPI_HBP_TIME + IPI_HACT_TIME) + 1)
         */
        frame_info->hsd_time = (ceil((pkt2pkt_time_ns + (((CSI2_LONG_PKT_BYTES + bytes_to_transmit) / csi_info->n_lanes)
                                 * time_PPI_ns / CSI2_BYTES_PER_HS_CLK)) / time_IPI_ns)
                                 - (frame_info->hsa_time + frame_info->hbp_time + camera_sensor->width) + 1);

        if(frame_info->hsd_time < CSI2_HSD_MIN)
        {
            frame_info->hsd_time = CSI2_HSD_MIN;
        }

        frame_info->hactive_time = 0;
        frame_info->vsa_line = 0;
        frame_info->vbp_line = 0;
        frame_info->vfp_line = 0;
        frame_info->vactive_line = 0;

        /* Memory calculation requirements
         * MEM_req_1 = (((IPI_HSD_TIME + IPI_HSA_TIME + IPI_HBP_TIME)*TIPI - Pkt2PktTime)/TPPI-HS)*N_LANES*BytesPerHsClk
         *
         * MEM_req_2 = (((IPI_HSD_TIME + IPI_HSA_TIME + IPI_HBP_TIME + IPI_HACT_TIME)*TIPI - Pkt2PktTime
         *              - (((LongPktBytes + Bytes2Transmit)/N_LANES)*TPPI-HS))/TPPI-HS)*N_LANES*BytesPerHsClk
         *
         * Min_Memory_Depth = MAX (MEM_req_1, MEM_req_2, 32)*8/ Memory_Width
         */
        mem_req_1 = (ceil(((frame_info->hsd_time + frame_info->hsa_time + frame_info->hbp_time)* time_IPI_ns - pkt2pkt_time_ns)
                            / time_PPI_ns) * csi_info->n_lanes * CSI2_BYTES_PER_HS_CLK);

        mem_req_2 = (ceil(((frame_info->hsd_time  + frame_info->hsa_time + frame_info->hbp_time + camera_sensor->width)* time_IPI_ns
                            - pkt2pkt_time_ns - (((CSI2_LONG_PKT_BYTES + bytes_to_transmit) / csi_info->n_lanes) * time_PPI_ns)) / time_PPI_ns)
                            * csi_info->n_lanes * CSI2_BYTES_PER_HS_CLK);

        min_memory_depth = MAX(mem_req_1, mem_req_2, 32) * 8 / CSI2_HOST_IPI_DWIDTH;

        if(CSI_IPI_FIFO_DEPTH < min_memory_depth)
        {
            return ARM_DRIVER_ERROR;
        }

    }

    /* Overwrite the CSI color mode on CPI */
    if(csi_info->cpi_cfg.override)
    {
        for(index = 0; index < ARRAY_SIZE(cpi_data_mode_settings) &&
            (cpi_data_mode_settings[index].cpi_color_mode != csi_info->cpi_cfg.cpi_color_mode); index++);

        if(cpi_data_mode_settings[index].cpi_color_mode != csi_info->cpi_cfg.cpi_color_mode)
        {
            return ARM_DRIVER_ERROR_PARAMETER;
        }
    }

    /* CPI config information */
    cpi_info.vsync_wait = CPI_WAIT_VSYNC_ENABLE;
    cpi_info.vsync_mode = CPI_CAPTURE_DATA_ENABLE_IF_HSYNC_HIGH;
    cpi_info.pixelclk_pol = CPI_SIG_POLARITY_INVERT_DISABLE;
    cpi_info.hsync_pol = CPI_SIG_POLARITY_INVERT_DISABLE;
    cpi_info.vsync_pol = CPI_SIG_POLARITY_INVERT_DISABLE;
    cpi_info.data_mode = cpi_data_mode_settings[index].cpi_data_mode;
    cpi_info.data_endianness = CPI_DATA_ENDIANNESS_LSB_FIRST;
    cpi_info.code10on8 = CPI_CODE10ON8_CODING_DISABLE;
    cpi_info.data_mask = CPI_DATA_MASK_BIT_16;
    cpi_info.csi_mode = cpi_data_mode_settings[index].cpi_color_mode;

    /* Registering CPI Info related to CSI */
    camera_sensor->cpi_info = &cpi_info;

    CSI2->cb_event = cb_event;

    /*DPHY initialization*/
    ret  = CSI2_DPHY_Initialize(csi_info->frequency, csi_info->n_lanes);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    CSI2->status.initialized = 1;

    return ret;
}

/**

  \fn          int32_t CSI2_Uninitialize (CSI_RESOURCES *CSI2)
  \brief       uninitialize MIPI CSI2 Interface.
  \param[in]   CSI2 Pointer to CSI resources
  \return      \ref execution_status
*/
static int32_t CSI2_Uninitialize (CSI_RESOURCES *CSI2)
{
    int32_t ret = ARM_DRIVER_OK;
    CSI2->cb_event = NULL;

    if (CSI2->status.initialized == 0)
    {
        /* Driver is already uninitialized */
        return ARM_DRIVER_OK;
    }

    if (CSI2->status.powered == 1)
    {
        /* Driver is not powered off */
        return ARM_DRIVER_ERROR;
    }

    /*DPHY Uninitialization*/
    ret  = CSI2_DPHY_Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Reset driver flags. */
    CSI2->status.initialized = 0;

    return ret;
}

/**

  \fn          int32_t CSI2_PowerControl (ARM_POWER_STATE state, CSI_RESOURCES *CSI2)
  \brief       Control CSI2 Interface Power.
  \param[in]   state  Power state
  \param[in]   CSI2  Pointer to CSI resources
  \return      \ref execution_status
*/
static int32_t CSI2_PowerControl (ARM_POWER_STATE state, CSI_RESOURCES *CSI2)
{
    if (CSI2->status.initialized == 0)
    {
        /* Driver is not initialized */
        return ARM_DRIVER_ERROR;
    }

    switch (state)
    {
        case ARM_POWER_OFF:
        {
            if (CSI2->status.powered == 0)
            {
                /* Driver is already powered off */
                return ARM_DRIVER_OK;
            }

            /*Disabling the IRQs*/
            csi_clear_phy_pkt_discard_intr_mask(CSI2->regs);
            csi_clear_phy_pkt_construction_intr_mask(CSI2->regs);
            csi_clear_phy_intr_mask(CSI2->regs);
            csi_clear_phy_line_construction_intr_mask(CSI2->regs);
            csi_clear_ipi_intr_mask(CSI2->regs);
            csi_clear_frame_bndry_err_intr_mask(CSI2->regs);
            csi_clear_frame_seq_err_intr_mask(CSI2->regs);
            csi_clear_frame_crc_err_intr_mask(CSI2->regs);
            csi_clear_frame_payload_err_intr_mask(CSI2->regs);
            csi_clear_dt_err_intr_mask(CSI2->regs);
            csi_clear_ecc_intr_mask(CSI2->regs);

            NVIC_DisableIRQ (CSI2->irq);
            NVIC_ClearPendingIRQ (CSI2->irq);

            clear_csi_pixel_clk();

            CSI2->status.powered = 0;

            break;
        }

        case ARM_POWER_FULL:
        {
            if (CSI2->status.powered == 1)
            {
                /* Driver is already powered ON */
                return ARM_DRIVER_OK;
            }

            NVIC_ClearPendingIRQ (CSI2->irq);
            NVIC_SetPriority (CSI2->irq, CSI2->irq_priority);
            NVIC_EnableIRQ (CSI2->irq);

            set_csi_pixel_clk(RTE_CSI2_PIX_CLK_SEL, CSI2->csi_pixclk_div);

            CSI2->status.powered = 1;

            break;
        }

        case ARM_POWER_LOW:

        default:
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }
    }

    return ARM_DRIVER_OK;
}

/**

  \fn          int32_t CSI2_ConfigureHost (uint32_t int_event, CSI_RESOURCES *CSI2)
  \brief       Configure CSI2 Host Interface.
  \param[in]   int_event interrupt event to be enabled.
  \param[in]   CSI2  Pointer to CSI resources
  \return      \ref execution_status
*/
static int32_t CSI2_ConfigureHost (uint32_t intr_event, CSI_RESOURCES *CSI2)
{
    if(CSI2->status.powered != 1)
    {
        return ARM_DRIVER_ERROR;
    }

    csi_enable_software_reset_state(CSI2->regs);

    if(intr_event & CSI2_EVENT_PHY_FATAL)
    {
        csi_set_phy_pkt_discard_intr_mask(CSI2->regs);
    }

    if(intr_event & CSI2_EVENT_PKT_FATAL)
    {
        csi_set_phy_pkt_construction_intr_mask(CSI2->regs);
    }

    if(intr_event & CSI2_EVENT_PHY)
    {
        csi_set_phy_intr_mask(CSI2->regs);
    }

    if(intr_event & CSI2_EVENT_LINE)
    {
        csi_set_phy_line_construction_intr_mask(CSI2->regs);
    }

    if(intr_event & CSI2_EVENT_IPI_FATAL)
    {
        csi_set_ipi_intr_mask(CSI2->regs);
    }

    if(intr_event & CSI2_EVENT_BNDRY_FRAME_FATAL)
    {
        csi_set_frame_bndry_err_intr_mask(CSI2->regs);
    }

    if(intr_event & CSI2_EVENT_SEQ_FRAME_FATAL)
    {
        csi_set_frame_seq_err_intr_mask(CSI2->regs);
    }

    if(intr_event & CSI2_EVENT_CRC_FRAME_FATAL)
    {
        csi_set_frame_crc_err_intr_mask(CSI2->regs);
    }

    if(intr_event & CSI2_EVENT_PLD_CRC_FATAL)
    {
        csi_set_frame_payload_err_intr_mask(CSI2->regs);
    }

    if(intr_event & CSI2_EVENT_DATA_ID)
    {
        csi_set_dt_err_intr_mask(CSI2->regs);
    }

    if(intr_event & CSI2_EVENT_ECC_CORRECT)
    {
        csi_set_ecc_intr_mask(CSI2->regs);
    }

    return ARM_DRIVER_OK;
}

/**

  \fn          void MIPI_CSI2_ISR (CSI_RESOURCES *CSI2)
  \brief       MIPI CSI2 interrupt service routine
  \param[in]   CSI2  Pointer to CSI resources
*/
static void MIPI_CSI2_ISR (CSI_RESOURCES *CSI2)
{
    uint32_t global_event_status = 0U;
    uint32_t event               = 0U;

    global_event_status = csi_get_interrupt_status(CSI2->regs);

    /*If cb_event is not registered Disable and clear the interrupt*/
    if(!CSI2->cb_event)
    {
        NVIC_DisableIRQ (CSI2->irq);
    }

    if(global_event_status & CSI_IRQ_PHY_FATAL)
    {
        (void)csi_get_phy_pkt_discard_intr_status(CSI2->regs);
        event |= CSI2_EVENT_PHY_FATAL;
    }

    if(global_event_status & CSI_IRQ_PKT_FATAL)
    {
        (void)csi_get_phy_pkt_construction_intr_status(CSI2->regs);
        event |= CSI2_EVENT_PKT_FATAL;
    }

    if(global_event_status & CSI_IRQ_BNDRY_FRAME_FATAL)
    {
        (void)csi_get_frame_bndry_err_intr_status(CSI2->regs);
        event |= CSI2_EVENT_BNDRY_FRAME_FATAL;
    }

    if(global_event_status & CSI_IRQ_SEQ_FRAME_FATAL)
    {
        (void)csi_get_frame_seq_err_intr_status(CSI2->regs);
        event |= CSI2_EVENT_SEQ_FRAME_FATAL;
    }

    if(global_event_status & CSI_IRQ_CRC_FRAME_FATAL)
    {
        (void)csi_get_frame_crc_err_intr_status(CSI2->regs);
        event |= CSI2_EVENT_CRC_FRAME_FATAL;
    }

    if(global_event_status & CSI_IRQ_PLD_CRC_FATAL)
    {
        (void)csi_get_frame_payload_err_intr_status(CSI2->regs);
        event |= CSI2_EVENT_PLD_CRC_FATAL;
    }

    if(global_event_status & CSI_IRQ_DATA_ID)
    {
        (void)csi_get_dt_err_intr_status(CSI2->regs);
        event |= CSI2_EVENT_DATA_ID;
    }

    if(global_event_status & CSI_IRQ_ECC_CORRECT)
    {
        (void)csi_get_ecc_intr_status(CSI2->regs);
        event |= CSI2_EVENT_ECC_CORRECT;
    }

    if(global_event_status & CSI_IRQ_PHY)
    {
        (void)csi_get_phy_intr_status(CSI2->regs);
        event |= CSI2_EVENT_PHY;
    }

    if(global_event_status & CSI_IRQ_LINE)
    {
        (void)csi_get_phy_line_construction_intr_status(CSI2->regs);
        event |= CSI2_EVENT_LINE;
    }

    if(global_event_status & CSI_IRQ_IPI_FATAL)
    {
        (void)csi_get_ipi_intr_status(CSI2->regs);
        event |= CSI2_EVENT_IPI_FATAL;
    }

    if(event != 0 && CSI2->cb_event)
    {
        CSI2->cb_event(event);
    }
}

/**
  \fn          int32_t  CSI2_ConfigureIPI (CSI_RESOURCES *CSI2)
  \brief       Configure CSI2 IPI Interface.
  \param[in]   CSI2  Pointer to CSI resources
  \return      \ref execution_status
*/
static int32_t CSI2_ConfigureIPI (CSI_RESOURCES *CSI2)
{
    uint32_t packet_config = 0;
    uint16_t hline_time;

    if(CSI2->status.powered != 1)
    {
        return ARM_DRIVER_ERROR;
    }

    csi_set_ipi_mode(CSI2->regs, CSI2->ipi_info->ipi_mode);

    csi_set_ipi_color_cop(CSI2->regs, CSI2->ipi_info->ipi_color_com);

    if(CSI2->ipi_info->ipi_memflush == ENABLE)
    {
        csi_enable_ipi_mem_flush_auto(CSI2->regs);
    }
    else
    {
        csi_set_ipi_mem_flush_manual(CSI2->regs);
    }

    csi_set_ipi_vc_id(CSI2->regs, CSI2->vc_id);

    csi_set_ipi_data_type(CSI2->regs, CSI2->pixel_data_type);

    csi_set_ipi_sync_event_type(CSI2->regs, CSI2->ipi_info->adv_features->sync_evnt_mode);

    csi_set_ipi_line_event_selection(CSI2->regs, CSI2->ipi_info->adv_features->event_sel);

    if(CSI2->ipi_info->adv_features->event_sel == ENABLE)
    {
        if(CSI2->ipi_info->adv_features->en_video == ENABLE)
        {
            packet_config |= CSI_IPI_EVENT_SELECTION_EN_VIDEO;
        }
        if(CSI2->ipi_info->adv_features->en_line_start == ENABLE)
        {
            packet_config |= CSI_IPI_EVENT_SELECTION_EN_LINE_START;
        }
        if(CSI2->ipi_info->adv_features->en_null == ENABLE)
        {
            packet_config |= CSI_IPI_EVENT_SELECTION_EN_NULL;
        }
        if(CSI2->ipi_info->adv_features->en_blanking == ENABLE)
        {
            packet_config |= CSI_IPI_EVENT_SELECTION_EN_BLANKING;
        }
        if(CSI2->ipi_info->adv_features->en_embedded == ENABLE)
        {
            packet_config |= CSI_IPI_EVENT_SELECTION_EN_EMBEDDED;
        }

        csi_set_packet_configuration(CSI2->regs, packet_config);
    }

    if(CSI2->ipi_info->adv_features->ipi_dt_overwrite == ENABLE)
    {
        csi_enable_ipi_dt_overwrite(CSI2->regs);
        csi_set_ipi_dt_overwrite(CSI2->regs, CSI2->ipi_info->adv_features->ipi_dt);
    }
    else
    {
        csi_disable_ipi_dt_overwrite(CSI2->regs);
    }

    hline_time = CSI2->ipi_info->frame_info->hactive_time + CSI2->ipi_info->frame_info->hsa_time
                 + CSI2->ipi_info->frame_info->hbp_time + CSI2->ipi_info->frame_info->hsd_time;

    csi_set_horizontal_timing(CSI2->regs, CSI2->ipi_info->frame_info->hsa_time,
                                          CSI2->ipi_info->frame_info->hbp_time,
                                          CSI2->ipi_info->frame_info->hsd_time,
                                          hline_time);

    csi_set_vertical_timing(CSI2->regs, CSI2->ipi_info->frame_info->vsa_line,
                                        CSI2->ipi_info->frame_info->vbp_line,
                                        CSI2->ipi_info->frame_info->vfp_line,
                                        CSI2->ipi_info->frame_info->vactive_line);

    csi_disable_software_reset_state(CSI2->regs);

    CSI2->status.csi_configured = 1;

    return ARM_DRIVER_OK;
}

/**

  \fn          int32_t  CSI2_StartIPI (CSI_RESOURCES *CSI2)
  \brief       Enable CSI2 IPI Interface.
  \param[in]   CSI2  Pointer to CSI resources
  \return      \ref execution_status
*/
static int32_t CSI2_StartIPI (CSI_RESOURCES *CSI2)
{
    if (CSI2->status.csi_configured == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    csi_enable_ipi_mode(CSI2->regs);

    return ARM_DRIVER_OK;
}

/**

  \fn          int32_t  CSI2_StopIPI (CSI_RESOURCES *CSI2)
  \brief       Disable CSI2 IPI Interface.
  \param[in]   CSI2  Pointer to CSI resources
  \return      \ref execution_status
*/
static int32_t CSI2_StopIPI (CSI_RESOURCES *CSI2)
{
    if (CSI2->status.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    csi_disable_ipi_mode(CSI2->regs);

    return ARM_DRIVER_OK;
}

/* CSI frame configuration */
CSI_FRAME_INFO CSI_FRAME_CFG =
{
    .hsa_time           = RTE_MIPI_CSI2_IPI_HSA_TIME,
    .hbp_time           = RTE_MIPI_CSI2_IPI_HBP_TIME,
    .hsd_time           = RTE_MIPI_CSI2_IPI_HSD_TIME,
    .hactive_time       = RTE_MIPI_CSI2_IPI_HACTIVE_TIME,
    .vsa_line           = RTE_MIPI_CSI2_IPI_VSA_LINE,
    .vbp_line           = RTE_MIPI_CSI2_IPI_VBP_LINE,
    .vfp_line           = RTE_MIPI_CSI2_IPI_VFP_LINE,
    .vactive_line       = RTE_MIPI_CSI2_IPI_VACTIVE_LINE,
};

/* CSI IPI advanced configuration */
CSI_IPI_ADV_INFO CSI_IPI_ADV_FEATURES_CFG =
{
    .sync_evnt_mode     = RTE_MIPI_CSI2_SYNC_ET_MODE,
    .event_sel          = RTE_MIPI_CSI2_SYNC_ET_SEL,
    .en_embedded        = RTE_MIPI_CSI2_EN_EMBEDDED,
    .en_blanking        = RTE_MIPI_CSI2_EN_BLANKING,
    .en_null            = RTE_MIPI_CSI2_EN_NULL,
    .en_line_start      = RTE_MIPI_CSI2_EN_LINE_START,
    .en_video           = RTE_MIPI_CSI2_EN_VIDEO,
    .ipi_dt             = RTE_MIPI_CSI2_EN_DT,
    .ipi_dt_overwrite   = RTE_MIPI_CSI2_EN_DT_OVERWRITE,
};

/* CSI IPI configuration */
CSI_IPI_INFO CSI_IPI_CFG =
{
    .ipi_mode           = RTE_MIPI_CSI2_IPI_MODE,
    .ipi_memflush       = RTE_MIPI_CSI2_MEMFLUSH,
    .frame_info         = &CSI_FRAME_CFG,
    .adv_features       = &CSI_IPI_ADV_FEATURES_CFG,
};

/* CSI resources */
CSI_RESOURCES CSI2 =
{
    .regs               = (CSI_Type *)CSI_BASE,
    .cb_event           = NULL,
    .ipi_info           = &CSI_IPI_CFG,
    .irq                = (IRQn_Type)CSI_IRQ_IRQn,
    .irq_priority       = RTE_MIPI_CSI2_IRQ_PRI,
};

static int32_t MIPI_CSI2_Initialize (ARM_MIPI_CSI2_SignalEvent_t cb_event)
{
    return CSI2_Initialize(cb_event, &CSI2);
}

static int32_t MIPI_CSI2_Uninitialize (void)
{
    return CSI2_Uninitialize(&CSI2);
}

static int32_t MIPI_CSI2_PowerControl (ARM_POWER_STATE state)
{
    return CSI2_PowerControl (state, &CSI2);
}

static int32_t MIPI_CSI2_ConfigureHost (uint32_t int_event)
{
    return CSI2_ConfigureHost (int_event, &CSI2);
}

static int32_t MIPI_CSI2_ConfigureIPI (void)
{
    return CSI2_ConfigureIPI(&CSI2);
}

static int32_t MIPI_CSI2_StartIPI(void)
{
    return CSI2_StartIPI(&CSI2);
}

static int32_t MIPI_CSI2_StopIPI(void)
{
    return CSI2_StopIPI(&CSI2);
}
void CSI_IRQHandler (void)
{
    MIPI_CSI2_ISR(&CSI2);
}

/**
\brief Access structure of the  MIPI CSI2 Driver.
*/
extern ARM_DRIVER_MIPI_CSI2 Driver_MIPI_CSI2;
ARM_DRIVER_MIPI_CSI2 Driver_MIPI_CSI2 =
{
    MIPI_CSI2_GetVersion,
    MIPI_CSI2_GetCapabilities,
    MIPI_CSI2_Initialize,
    MIPI_CSI2_Uninitialize,
    MIPI_CSI2_PowerControl,
    MIPI_CSI2_ConfigureHost,
    MIPI_CSI2_ConfigureIPI,
    MIPI_CSI2_StartIPI,
    MIPI_CSI2_StopIPI,
};

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/

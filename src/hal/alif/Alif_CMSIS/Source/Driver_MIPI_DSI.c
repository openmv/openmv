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
 * @file     Driver_MIPI_DSI.c
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     28-Sep-2023
 * @brief    CMSIS-Driver for MIPI DSI.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#include "Driver_MIPI_DSI.h"
#include "Driver_DSI_Private.h"
#include "RTE_Device.h"
#include "DPHY_DSI.h"
#include "display.h"

#define ARM_MIPI_DSI_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0) /*driver version*/

#if !(RTE_MIPI_DSI)
#error "MIPI DSI is not enabled in the RTE_Device.h"
#endif

#if (!defined(RTE_Drivers_MIPI_DSI))
#error "MIPI DSI not configured in RTE_Components.h!"
#endif

/* High speed transition timing range */
static const DSI_DPHY_HS_TRANSITION_TIMINGS_RANGE hstransition_timing_range[] =
{
    { 80, 21, 17, 15, 10 }, { 90, 23, 17, 16, 10 }, { 100, 22, 17, 16, 10 },
    { 110, 25, 18, 17, 11 }, { 120, 26, 20, 18, 11 }, { 130, 27, 19, 19, 11 },
    { 140, 27, 19, 19, 11 }, { 150, 28, 20, 20, 12 }, { 160, 30, 21, 22, 13 },
    { 170, 30, 21, 23, 13 }, { 180, 31, 21, 23, 13 }, { 190, 32, 22, 24, 13 },
    { 205, 35, 22, 25, 13 }, { 220, 37, 26, 27, 15 }, { 235, 38, 28, 27, 16 },
    { 250, 41, 29, 30, 17 }, { 275, 43, 29, 32, 18 }, { 300, 45, 32, 35, 19 },
    { 325, 48, 33, 36, 18 }, { 350, 51, 35, 40, 20 }, { 400, 59, 37, 44, 21 },
    { 450, 65, 40, 49, 23 }, { 500, 71, 41, 54, 24 }, { 550, 77, 44, 57, 26 },
    { 600, 82, 46, 64, 27 }, { 650, 87, 48, 67, 28}, { 700, 94, 52, 71, 29 },
    { 750, 99, 52, 75, 31 }, { 800, 105, 55, 82, 32 }, { 850, 110, 58, 85, 32 },
    { 900, 115, 58, 88, 35 }, { 950, 120, 62, 93, 36 }, { 1000, 128, 63, 99, 38 },
    { 1050, 132, 65, 102, 38 }, { 1100, 138, 67, 106, 39 }, { 1150, 146, 69, 112, 42 },
    { 1200, 151, 71, 117, 43 }, { 1250, 153, 74, 120, 45 }, { 1300, 160, 73, 124, 46 },
    { 1350, 165, 76, 130, 47 }, { 1400, 172, 78, 134, 49 }, { 1450, 177, 80, 138, 49 },
    { 1500, 183, 81, 143, 52 }, { 1550, 191, 84, 147, 52 }, { 1600, 194, 85, 152, 52 },
    { 1650, 201, 86, 155, 53 }, { 1700, 208, 88, 161, 53 }, { 1750, 212, 89, 165, 53 },
    { 1800, 220, 90, 171, 54 }, { 1850, 223, 92, 175, 54 }, { 1900, 231, 91, 180, 55 },
    { 1950, 236, 95, 185, 56 }, { 2000, 243, 97, 190, 56 }, { 2050, 248, 99, 194, 58 },
    { 2100, 252, 100, 199, 59 }, { 2150, 259, 102, 204, 61 }, { 2200, 266, 105, 210, 62 },
    { 2250, 269, 109, 213, 63 }, { 2300, 272, 109, 217, 65 }, { 2350, 281, 112, 225, 66},
    { 2400, 283, 115, 226, 66 }, { 2450, 282, 115, 226, 67 }, { 2500, 281, 118, 227, 67 }
};

/* CDC config informations */
static CDC_INFO cdc_info;

/*Driver Version*/
static const ARM_DRIVER_VERSION DriverVersion =
{
    ARM_MIPI_DSI_API_VERSION,
    ARM_MIPI_DSI_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_MIPI_DSI_CAPABILITIES DriverCapabilities =
{
    0, /* Not supports reentrant_operation */
    1, /* DPI interface supported*/
    0, /* DBI interface not supported*/
    0  /* reserved (must be zero) */
};

/**
  \fn          ARM_DRIVER_VERSION ARM_MIPI_DSI_GetVersion (void)
  \brief       Get MIPI DSI driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION ARM_MIPI_DSI_GetVersion (void)
{
    return DriverVersion;
}

/**
  \fn          ARM_MIPI_DSI_CAPABILITIES MIPI_DSI_GetCapabilities (void)
  \brief       Get MIPI DSI driver capabilities
  \return      \ref ARM_MIPI_DPHY_CAPABILITIES
*/
static ARM_MIPI_DSI_CAPABILITIES ARM_MIPI_DSI_GetCapabilities (void)
{
    return DriverCapabilities;
}
/**
  \fn          int32_t DSI_Initialize (ARM_MIPI_DSI_SignalEvent_t cb_event,
                                       DISPLAY_PANEL_DEVICE *display_panel, DSI_RESOURCES *dsi)
  \brief       Initialize MIPI DSI Interface.
  \param[in]   cb_event Pointer to ARM_MIPI_DSI_SignalEvent_t.
  \param[in]   display_panel Pointer to display panel resources.
  \param[in]   dsi Pointer to DSI resources.
  \return      \ref execution_status.
  */
static int32_t DSI_Initialize (ARM_MIPI_DSI_SignalEvent_t cb_event,
                               DISPLAY_PANEL_DEVICE *display_panel, DSI_RESOURCES *dsi)
{
    int32_t ret = ARM_DRIVER_OK;
    DSI_INFO *dsi_info;
    uint32_t pixclk;

    if(dsi->state.initialized == 1)
    {
        return ARM_DRIVER_OK;
    }

    if (!cb_event)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    dsi_info = display_panel->dsi_info;

    if (!(display_panel && dsi_info))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* LCD Manufacturer provides the Frame timing values
     *     HTOTAL = WIDTH + HSYNC + HFP + HBP
     *     VTOTAL = HEIGHT + VSYNC + VFP + VBP
     * Calculate the pixel clock for DPI controller
     *     PIXCLK = FPS x HTOTAL x VTOTAL
     */
    dsi->horizontal_timing = (display_panel->hsync_time
              + display_panel->hbp_time
              + display_panel->hfp_time
              + display_panel->hactive_time);

    dsi->vertical_timing = (display_panel->vsync_line
              + display_panel->vbp_line
              + display_panel->vfp_line
              + display_panel->vactive_line);

    pixclk = (dsi->horizontal_timing * dsi->vertical_timing * RTE_CDC200_DPI_FPS);

    /* SCALE = LANEBYTECLK / PIXCLK
     * MIPI data rate must be exactly equal, not greater than, for 1.5 scale to work
     * MIPI data rate + 33% allows for scaling times 2
     *    24 x 1.333 / 16 = 2
     * LANEBYTECLK = PIXCLK * SCALE
     * lanebyteclk frequency is 1/4th of the DPHY frequency
     * PLL frequency = LANEBYTECLK * 4
     *               = PIXCLK * SCALE * 4
     */
    dsi->frequency = pixclk * 2 * 4;

    /*Checking LCD Panel supports MIPI DSI DPHY data rate*/
    if((dsi->frequency * 2) > (dsi_info->max_bitrate * 1000000))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if(dsi_dpi_get_hsync_polarity(dsi->reg_base) == DSI_POLARITY_ACTIVE_LOW)
    {
        cdc_info.hsync_polarity = CDC_POLARITY_ACTIVE_LOW;
    }
    else
    {
        cdc_info.hsync_polarity = CDC_POLARITY_ACTIVE_HIGH;
    }

    if(dsi_dpi_get_vsync_polarity(dsi->reg_base) == DSI_POLARITY_ACTIVE_LOW)
    {
        cdc_info.vsync_polarity = CDC_POLARITY_ACTIVE_LOW;
    }
    else
    {
        cdc_info.vsync_polarity = CDC_POLARITY_ACTIVE_HIGH;
    }

    cdc_info.pclk_polarity  = CDC_PIXCLK_POLARITY_FEED_THROUGH;

    if(dsi_dpi_get_dataen_polarity(dsi->reg_base) == DSI_POLARITY_ACTIVE_LOW)
    {
        cdc_info.blank_polarity = CDC_POLARITY_ACTIVE_HIGH;
    }
    else
    {
        cdc_info.blank_polarity = CDC_POLARITY_ACTIVE_LOW;
    }

    /* Registering CPI Info related to CSI */
    display_panel->cdc_info = &cdc_info;

    dsi->cb_event = cb_event;

    /*DPHY initialization*/
    ret  = DSI_DPHY_Initialize(dsi->frequency, dsi_info->n_lanes);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /*LCD Panel Initialization*/
    ret = display_panel->ops->Init();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    dsi->dpi_info->vid_pkt_size = display_panel->hactive_time;

    dsi->state.initialized = 1;

    return ret;
}

/**
  \fn          int32_t DSI_Uninitialize (DISPLAY_PANEL_DEVICE *display_panel, DSI_RESOURCES *dsi)
  \brief       uninitialize MIPI DSI Interface.
  \param[in]   display_panel Pointer to display panel resources.
  \param[in]   dsi Pointer to DSI resources.
  \return      \ref execution_status.
  */
static int32_t DSI_Uninitialize (DISPLAY_PANEL_DEVICE *display_panel, DSI_RESOURCES *dsi)
{
    int32_t ret = ARM_DRIVER_OK;
    dsi->cb_event = NULL;

    if(dsi->state.initialized == 0)
    {
        return ARM_DRIVER_OK;
    }

    if (dsi->state.powered == 1)
    {
        return ARM_DRIVER_ERROR;
    }

    /*DPHY Uninitialization*/
    ret  = DSI_DPHY_Uninitialize();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /*LCD Panel Un-Initialization*/
    ret = display_panel->ops->Uninit();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    dsi->state.initialized = 0;
    return ret;
}

/**
  \fn          int32_t DSI_PowerControl (ARM_POWER_STATE state,
                                         DSI_RESOURCES *dsi)
  \brief       Control DSI Interface Power.
  \param[in]   state  Power state.
  \param[in]   dsi Pointer to DSI resources.
  \return      \ref execution_status.
  */
static int32_t DSI_PowerControl (ARM_POWER_STATE state,
                                 DSI_RESOURCES *dsi)
{
    if (dsi->state.initialized == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    switch (state)
    {
        case ARM_POWER_OFF:
        {
            if (dsi->state.powered == 0)
            {
                return ARM_DRIVER_OK;
            }

            NVIC_DisableIRQ (dsi->irq);
            NVIC_ClearPendingIRQ (dsi->irq);

            dsi->state.powered = 0;
            break;
        }

        case ARM_POWER_FULL:
        {
            if (dsi->state.powered == 1)
            {
                return ARM_DRIVER_OK;
            }

            NVIC_ClearPendingIRQ (dsi->irq);
            NVIC_SetPriority (dsi->irq, dsi->irq_priority);
            NVIC_EnableIRQ (dsi->irq);

            dsi->state.powered = 1;

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
  \fn          int32_t DSI_ConfigureHost (DSI_RESOURCES *dsi)
  \brief       Configure DSI Host Interface.
  \param[in]   dsi  Pointer to DSI resources.
  \return      \ref execution_status.
  */
static int32_t DSI_ConfigureHost (DSI_RESOURCES *dsi)
{
    if (dsi->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    dsi_irq0_enable(dsi->reg_base, DSI_IRQ0_ACK_WITH_ERR_0  | \
                                   DSI_IRQ0_ACK_WITH_ERR_1  | \
                                   DSI_IRQ0_ACK_WITH_ERR_2  | \
                                   DSI_IRQ0_ACK_WITH_ERR_3  | \
                                   DSI_IRQ0_ACK_WITH_ERR_4  | \
                                   DSI_IRQ0_ACK_WITH_ERR_5  | \
                                   DSI_IRQ0_ACK_WITH_ERR_6  | \
                                   DSI_IRQ0_ACK_WITH_ERR_7  | \
                                   DSI_IRQ0_ACK_WITH_ERR_8  | \
                                   DSI_IRQ0_ACK_WITH_ERR_9  | \
                                   DSI_IRQ0_ACK_WITH_ERR_10 | \
                                   DSI_IRQ0_ACK_WITH_ERR_11 | \
                                   DSI_IRQ0_ACK_WITH_ERR_12 | \
                                   DSI_IRQ0_ACK_WITH_ERR_13 | \
                                   DSI_IRQ0_ACK_WITH_ERR_14 | \
                                   DSI_IRQ0_ACK_WITH_ERR_15 | \
                                   DSI_IRQ0_DPHY_ERRORS_0   | \
                                   DSI_IRQ0_DPHY_ERRORS_1   | \
                                   DSI_IRQ0_DPHY_ERRORS_2   | \
                                   DSI_IRQ0_DPHY_ERRORS_3   | \
                                   DSI_IRQ0_DPHY_ERRORS_4);

    dsi_irq1_enable(dsi->reg_base, DSI_IRQ1_TO_HS_TX          | \
                                   DSI_IRQ1_TO_LP_RX          | \
                                   DSI_IRQ1_ECC_SINGLE_ERR    | \
                                   DSI_IRQ1_ECC_MILTI_ERR     | \
                                   DSI_IRQ1_CRC_ERR           | \
                                   DSI_IRQ1_PKT_SIZE_ERR      | \
                                   DSI_IRQ1_EOPT_ERR          | \
                                   DSI_IRQ1_DPI_PLD_WR_ERR    | \
                                   DSI_IRQ1_GEN_CMD_WR_ERR    | \
                                   DSI_IRQ1_GEN_PLD_WR_ERR    | \
                                   DSI_IRQ1_GEN_PLD_SEND_ERR  | \
                                   DSI_IRQ1_GEN_PLD_RD_ERR    | \
                                   DSI_IRQ1_GEN_PLD_RECEV_ERR);

    dsi->state.host_configured = 1;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t  DSI_ConfigureDPI (DISPLAY_PANEL_DEVICE *display_panel, DSI_RESOURCES *dsi)
  \brief       Configure DSI DPI Interface.
  \param[in]   display_panel Pointer to display panel resources.
  \param[in]   dsi  Pointer to DSI resources.
  \return      \ref execution_status.
*/
static int32_t DSI_ConfigureDPI (DISPLAY_PANEL_DEVICE *display_panel, DSI_RESOURCES *dsi)
{
    DSI_INFO *dsi_info;
    DSI_DPI_INFO *dpi_info = (DSI_DPI_INFO *)dsi->dpi_info;

    dsi_info = display_panel->dsi_info;

    if(dsi->state.host_configured == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    dsi_dpi_set_color_coding(dsi->reg_base, dsi_info->color_coding);

    dsi_set_video_packet_size(dsi->reg_base, dpi_info->vid_pkt_size);

    dsi_set_video_number_chunks(dsi->reg_base, dpi_info->vid_num_chunks);

    dsi_set_video_null_packet_size(dsi->reg_base, dpi_info->vid_null_size);

    dsi_set_video_hsa_time(dsi->reg_base, display_panel->hsync_time << 1);

    dsi_set_video_hbp_time(dsi->reg_base, display_panel->hbp_time << 1);

    dsi_set_video_hline_time(dsi->reg_base, ((display_panel->hsync_time << 1)    \
                                             + (display_panel->hbp_time << 1) \
                                             + (display_panel->hfp_time << 1) \
                                             + (display_panel->hactive_time << 1)));

    dsi_set_video_vsa_lines(dsi->reg_base, display_panel->vsync_line);

    dsi_set_video_vbp_lines(dsi->reg_base, display_panel->vbp_line);

    dsi_set_video_vfp_lines(dsi->reg_base, display_panel->vfp_line);

    dsi_set_video_vactive_lines(dsi->reg_base, display_panel->vactive_line);

    dsi->state.dpi_configured = 1;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t DSI_Control (ARM_MIPI_DSI_CONTROL control, uint32_t arg,
                                    DISPLAY_PANEL_DEVICE *display_panel, DSI_RESOURCES *dsi)
  \brief       Control DSI Interface.
  \param[in]   control DSI host and DPI Configuration.
  \param[in]   arg Argument of operation (optional)
  \param[in]   display_panel Pointer to display panel resources.
  \param[in]   dsi Pointer to DSI resources.
  \return      \ref execution_status.
  */
static int32_t DSI_Control (ARM_MIPI_DSI_CONTROL control, uint32_t arg,
                            DISPLAY_PANEL_DEVICE *display_panel,DSI_RESOURCES *dsi)
{
    ARG_UNUSED(arg);
    int32_t ret = ARM_DRIVER_OK;

    switch(control)
    {
        case DSI_CONFIGURE_HOST:
        {
            ret = DSI_ConfigureHost(dsi);
            if(ret != ARM_DRIVER_OK)
            {
                return ret;
            }
            break;
        }

        case DSI_CONFIGURE_DPI:
        {
            ret = DSI_ConfigureDPI(display_panel, dsi);
            if(ret != ARM_DRIVER_OK)
            {
                return ret;
            }
            break;
        }

        default:
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }
    }

    return ret;
}

/**
  \fn          int32_t  DSI_StartCommandMode (DISPLAY_PANEL_DEVICE *display_panel, DSI_RESOURCES *dsi)
  \brief       Configure DSI to start Command mode.
  \param[in]   display_panel Pointer to display panel resources.
  \param[in]   dsi  Pointer to DSI resources.
  \return      \ref execution_status.
*/
static int32_t DSI_StartCommandMode (DISPLAY_PANEL_DEVICE *display_panel, DSI_RESOURCES *dsi)
{
    int32_t ret  = ARM_DRIVER_OK;
    DSI_DPI_INFO *dpi_info = (DSI_DPI_INFO *)dsi->dpi_info;
    uint16_t clklp2hs_time, lp2hs_time, clkhs2lp_time, hs2lp_time;
    uint32_t bitrate_mbps;
    uint8_t range = 0;

    if(dsi->state.host_configured == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    bitrate_mbps = (dsi->frequency * 2)/1000000;

    for(range = 0; (range < ARRAY_SIZE(hstransition_timing_range) - 1) &&
        ((bitrate_mbps) > hstransition_timing_range[range].bitrate_mbps);
        ++range);

    clklp2hs_time = hstransition_timing_range[range].clklp2hs_time;
    clkhs2lp_time = hstransition_timing_range[range].clkhs2lp_time;
    lp2hs_time = hstransition_timing_range[range].lp2hs_time;
    hs2lp_time = hstransition_timing_range[range].hs2lp_time;

    dsi_power_up_disable(dsi->reg_base);

    dsi_auto_clklane_enable(dsi->reg_base);

    dsi_set_phy_clklp2hs_time(dsi->reg_base, clklp2hs_time);

    dsi_set_phy_clkhs2lp_time(dsi->reg_base, clkhs2lp_time);

    dsi_set_phy_lp2hs_time(dsi->reg_base, lp2hs_time);

    dsi_set_phy_hs2lp_time(dsi->reg_base, hs2lp_time);

    dsi_reception_enable(dsi->reg_base);

    dsi_command_mode_enable(dsi->reg_base);

    dsi_set_video_mode_type(dsi->reg_base, dpi_info->vid_mode);

    dsi_command_transmission_enable(dsi->reg_base);

    dsi_set_tx_escap_clock_divider(dsi->reg_base, dsi->tx_ecs_clk_div);

    dsi_set_command_mode_config(dsi->reg_base, DSI_CMD_MODE_CFG_GEN_SW_0P_TX | \
                                               DSI_CMD_MODE_CFG_GEN_SW_1P_TX | \
                                               DSI_CMD_MODE_CFG_GEN_SW_2P_TX | \
                                               DSI_CMD_MODE_CFG_GEN_SR_0P_TX | \
                                               DSI_CMD_MODE_CFG_GEN_SR_1P_TX | \
                                               DSI_CMD_MODE_CFG_GEN_SR_2P_TX | \
                                               DSI_CMD_MODE_CFG_GEN_LW_TX    | \
                                               DSI_CMD_MODE_CFG_DCS_SW_0P_TX | \
                                               DSI_CMD_MODE_CFG_DCS_SW_1P_TX | \
                                               DSI_CMD_MODE_CFG_DCS_SR_0P_TX | \
                                               DSI_CMD_MODE_CFG_DCS_LW_TX    | \
                                               DSI_CMD_MODE_CFG_MAX_RD_PKT_SIZE);

    dsi_power_up_enable(dsi->reg_base);

    /*Configure LCD Panel*/
    ret = display_panel->ops->Control(DISPALY_PANEL_CONFIG);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    dsi->state.panel_initialized = 1;

    return ret;
}

/**
  \fn          int32_t  DSI_StartVideoMode (DISPLAY_PANEL_DEVICE *display_panel,DSI_RESOURCES *dsi)
  \brief       Configure DSI to start Video mode.
  \param[in]   display_panel Pointer to display panel resources.
  \param[in]   dsi  Pointer to DSI resources.
  \return      \ref execution_status.
*/
static int32_t DSI_StartVideoMode (DISPLAY_PANEL_DEVICE *display_panel, DSI_RESOURCES *dsi)
{
    int32_t ret  = ARM_DRIVER_OK;

    if((dsi->state.dpi_configured == 0) && (dsi->state.panel_initialized) == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    dsi_power_up_disable(dsi->reg_base);

    dsi_auto_clklane_disable(dsi->reg_base);

    dsi_phy_txrequestclkhs_enable(dsi->reg_base);

    dsi_hs_eotp_enable(dsi->reg_base);

    dsi_video_mode_enable(dsi->reg_base);

    dsi_power_up_enable(dsi->reg_base);

    /*Start LCD Panel*/
    ret = display_panel->ops->Start();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    return ret;
}

/**
  \fn          int32_t  DSI_Stop (DISPLAY_PANEL_DEVICE *display_panel, DSI_RESOURCES *dsi)
  \brief       Shutdown DSI.
  \param[in]   display_panel Pointer to display panel resources.
  \param[in]   dsi  Pointer to DSI resources
  \return      \ref execution_status
*/
static int32_t DSI_Stop (DISPLAY_PANEL_DEVICE *display_panel, DSI_RESOURCES *dsi)
{
    int32_t ret  = ARM_DRIVER_OK;

    if(dsi->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    dsi_power_up_disable(dsi->reg_base);

    /*Stop LCD Panel*/
    ret = display_panel->ops->Stop();
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    return ret;
}

/**
  \fn          void MIPI_DSI_ISR (DSI_RESOURCES *dsi)
  \brief       MIPI DSI interrupt service routine
  \param[in]   dsi  Pointer to DSI resources
*/
static void MIPI_DSI_ISR (DSI_RESOURCES *dsi)
{
    uint32_t int_st = dsi_irq0_status(dsi->reg_base);

    if(int_st & (DSI_IRQ0_ACK_WITH_ERR_0  | DSI_IRQ0_ACK_WITH_ERR_1  | \
                 DSI_IRQ0_ACK_WITH_ERR_2  | DSI_IRQ0_ACK_WITH_ERR_3  | \
                 DSI_IRQ0_ACK_WITH_ERR_4  | DSI_IRQ0_ACK_WITH_ERR_5  | \
                 DSI_IRQ0_ACK_WITH_ERR_6  | DSI_IRQ0_ACK_WITH_ERR_7  | \
                 DSI_IRQ0_ACK_WITH_ERR_8  | DSI_IRQ0_ACK_WITH_ERR_9  | \
                 DSI_IRQ0_ACK_WITH_ERR_10 | DSI_IRQ0_ACK_WITH_ERR_11 | \
                 DSI_IRQ0_ACK_WITH_ERR_12 | DSI_IRQ0_ACK_WITH_ERR_13 | \
                 DSI_IRQ0_ACK_WITH_ERR_14 | DSI_IRQ0_ACK_WITH_ERR_15))
    {
        dsi->cb_event(DSI_PHY_ERROR_EVENT);
    }

    if(int_st & (DSI_IRQ0_DPHY_ERRORS_0 | DSI_IRQ0_DPHY_ERRORS_1 | \
                 DSI_IRQ0_DPHY_ERRORS_3 | DSI_IRQ0_DPHY_ERRORS_4))
    {
        dsi->cb_event(DSI_ACK_ERROR_EVENT);
    }

    int_st = dsi_irq1_status(dsi->reg_base);

    if(int_st & (DSI_IRQ1_TO_HS_TX | DSI_IRQ1_TO_LP_RX              | \
                 DSI_IRQ1_ECC_SINGLE_ERR | DSI_IRQ1_ECC_MILTI_ERR   | \
                 DSI_IRQ1_CRC_ERR | DSI_IRQ1_PKT_SIZE_ERR           | \
                 DSI_IRQ1_EOPT_ERR))
    {
        dsi->cb_event(DSI_PKT_ERROR_EVENT);
    }

    if(int_st & (DSI_IRQ1_DPI_PLD_WR_ERR | DSI_IRQ1_GEN_CMD_WR_ERR    | \
                 DSI_IRQ1_GEN_PLD_WR_ERR | DSI_IRQ1_GEN_PLD_SEND_ERR  | \
                 DSI_IRQ1_GEN_PLD_RD_ERR | DSI_IRQ1_GEN_PLD_RECEV_ERR | \
                 DSI_IRQ1_DPI_BUFF_PLD_UNDER))
    {
        dsi->cb_event(DSI_DPI_ERROR_EVENT);
    }
}

/* DSI LCD Panel access structure */
static DISPLAY_PANEL_DEVICE *display_panel;

static DSI_DPI_INFO DPI_INFO =
{
    .vid_mode       = RTE_MIPI_DSI_VID_MODE_TYPE,
    .vid_num_chunks = RTE_MIPI_DSI_VID_NUM_CHUNKS,
    .vid_null_size  = RTE_MIPI_DSI_VID_NULL_SIZE,
};

static DSI_RESOURCES DSI_RES =
{
    .reg_base          = (DSI_Type*)DSI_BASE,
    .tx_ecs_clk_div    = RTE_MIPI_DSI_TX_ESC_CLK_DIVISION,
    .dpi_info          = &DPI_INFO,
    .irq               = DSI_IRQ_IRQn,
    .irq_priority      = RTE_MIPI_DSI_IRQ_PRI,
};

/**
  \fn          void DSI_DCS_Short_Write (uint8_t cmd, uint8_t data)
  \brief       Perform MIPI DSI DCS Short write.
  \param[in]   cmd is DCS command info.
  \param[in]   data to send.
*/
void DSI_DCS_Short_Write (uint8_t cmd, uint8_t data)
{
    dsi_dcs_short_write(DSI_RES.reg_base, cmd, data, display_panel->dsi_info->vc_id);

    sys_busy_loop_us(100);
}

/**
  \fn          void DSI_DCS_CMD_Short_Write (uint8_t cmd)
  \brief       Perform MIPI DSI DCS Short write only command.
  \param[in]   cmd is DCS command info.
*/
void DSI_DCS_CMD_Short_Write (uint8_t cmd)
{
    dsi_dcs_cmd_short_write(DSI_RES.reg_base, cmd, display_panel->dsi_info->vc_id);

    sys_busy_loop_us(100);
}

/**
  \fn          void DSI_DCS_Long_Write (uint8_t cmd, uint32_t data)
  \brief       Perform MIPI DSI DCS Short write.
  \param[in]   data pointer to data buffer.
  \param[in]   len data buffer length.
*/
void DSI_DCS_Long_Write (uint8_t* data, uint32_t len)
{
    dsi_dcs_long_write(DSI_RES.reg_base, data, len, display_panel->dsi_info->vc_id);

    sys_busy_loop_us(100);
}

/**
  \fn          int32_t  ARM_MIPI_DSI_Initialize (ARM_MIPI_DSI_SignalEvent_t cb_event)
  \brief       Initialize MIPI DSI Interface.
  \param[in]   cb_event Pointer to ARM_MIPI_DSI_SignalEvent_t
  \return      \ref execution_status
  */
static int32_t ARM_MIPI_DSI_Initialize (ARM_MIPI_DSI_SignalEvent_t cb_event)
{
    display_panel = Get_Display_Panel();
    return DSI_Initialize (cb_event, display_panel, &DSI_RES);
}

/**
  \fn          int32_t ARM_MIPI_DSI_Uninitialize (void)
  \brief       uninitialize MIPI DSI Interface.
  \return      \ref execution_status
  */
static int32_t ARM_MIPI_DSI_Uninitialize (void)
{
    return DSI_Uninitialize (display_panel, &DSI_RES);
}

/**
  \fn          int32_t ARM_MIPI_DSI_PowerControl (ARM_POWER_STATE state)
  \brief       Control DSI Interface Power.
  \param[in]   state  Power state
  \return      \ref execution_status
  */
static int32_t ARM_MIPI_DSI_PowerControl (ARM_POWER_STATE state)
{
    return DSI_PowerControl (state, &DSI_RES);
}

/**
  \fn          int32_t ARM_MIPI_DSI_Control (ARM_MIPI_DSI_CONTROL control, uint32_t arg)
  \brief       Control DSI Interface.
  \param[in]   control DSI host and DPI Configuration.
  \param[in]   arg Argument of operation (optional)
  \return      \ref execution_status
  */
static int32_t ARM_MIPI_DSI_Control (ARM_MIPI_DSI_CONTROL control, uint32_t arg)
{
    return DSI_Control (control, arg, display_panel, &DSI_RES);
}

/**
  \fn          int32_t  ARM_MIPI_DSI_StartCommandMode (void)
  \brief       Configure DSI to start Command mode.
  \return      \ref execution_status
*/
static int32_t ARM_MIPI_DSI_StartCommandMode (void)
{
    return DSI_StartCommandMode (display_panel,&DSI_RES);
}

/**
  \fn          int32_t  ARM_MIPI_DSI_StartVideoMode (void)
  \brief       Configure DSI to start Video mode.
  \return      \ref execution_status
*/
static int32_t ARM_MIPI_DSI_StartVideoMode (void)
{
    return DSI_StartVideoMode (display_panel, &DSI_RES);
}

/**
  \fn          int32_t  ARM_MIPI_DSI_Stop (void)
  \brief       Shutdown DSI.
  \return      \ref execution_status
*/
static int32_t ARM_MIPI_DSI_Stop (void)
{
    return DSI_Stop (display_panel, &DSI_RES);
}

/**
  \fn          void DSI_IRQHandler (void)
  \brief       DSi IRQ Handler.
*/
void DSI_IRQHandler(void)
{
    MIPI_DSI_ISR (&DSI_RES);
}

/**
  \brief Access structure of the  MIPI DSI Driver.
  */
extern ARM_DRIVER_MIPI_DSI Driver_MIPI_DSI;

ARM_DRIVER_MIPI_DSI Driver_MIPI_DSI =
{
    ARM_MIPI_DSI_GetVersion,
    ARM_MIPI_DSI_GetCapabilities,
    ARM_MIPI_DSI_Initialize,
    ARM_MIPI_DSI_Uninitialize,
    ARM_MIPI_DSI_PowerControl,
    ARM_MIPI_DSI_Control,
    ARM_MIPI_DSI_StartCommandMode,
    ARM_MIPI_DSI_StartVideoMode,
    ARM_MIPI_DSI_Stop
};

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
 * @file     ILI6122_LCD_panel.c
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     04-Oct-23
 * @brief    Focus ILI6122 LCD panel driver.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

/* System Includes */
#include "RTE_Device.h"
#include "RTE_Components.h"

#if RTE_ILI6122_PANEL
#include "display.h"

#if (!defined(RTE_Drivers_CDC_ILI6122_PANEL))
#error "ILI6122 Display Panel not configured in RTE_Components.h!"
#endif

/* CDC polarities information assignment */
static CDC_INFO ili6122_cdc_info =
{
    .hsync_polarity = RTE_ILI6122_PANEL_CDC_HSYNC_ACTIVE_LOW,
    .vsync_polarity = RTE_ILI6122_PANEL_CDC_VSYNC_ACTIVE_LOW,
    .pclk_polarity  = RTE_ILI6122_PANEL_CDC_PIXCLK_FEED_THROUGH,
    .blank_polarity = RTE_ILI6122_PANEL_CDC_BLANK_ACTIVE_LOW,
};

/* Parallel display panel device informations assignment */
static DISPLAY_PANEL_DEVICE ILI6122_display_panel =
{
    .hsync_time   = RTE_PANEL_HSYNC_TIME,
    .hbp_time     = RTE_PANEL_HBP_TIME,
    .hfp_time     = RTE_PANEL_HFP_TIME,
    .hactive_time = RTE_PANEL_HACTIVE_TIME,
    .vsync_line   = RTE_PANEL_VSYNC_LINE,
    .vbp_line     = RTE_PANEL_VBP_LINE,
    .vfp_line     = RTE_PANEL_VFP_LINE,
    .vactive_line = RTE_PANEL_VACTIVE_LINE,
    .cdc_info     = &ili6122_cdc_info,
};

/* Registering Display Panel */
DISPLAY_PANEL(ILI6122_display_panel)

#endif /* RTE_ILI6122_PANEL */

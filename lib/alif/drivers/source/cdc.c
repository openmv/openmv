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
 * @file     cdc.c
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     10-April-2023
 * @brief    Low level driver Specific Source file.
 ******************************************************************************/

#include <cdc.h>

/**
 * @fn      void cdc_set_cfg (CDC_Type *const cdc, const cdc_cfg_info_t *const info)
 * @brief   Configure the CDC with given information.
 * @param   cdc   Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   info  Pointer to the cdc configuration structure. See {@ref cdc_cfg_info_t} for details.
 * @retval  none
 */
void cdc_set_cfg (CDC_Type *const cdc, const cdc_cfg_info_t *const info)
{
    /* The number of horizontal sync pixels and the number of vertical sync
     * lines (minus 1).*/
    const uint32_t sync_size = (((info->timing_info.hsync - 1) << 16 ) +
                                (info->timing_info.vsync - 1)           );

    /* The accumulated number of horizontal sync and back porch pixels
     * and the accumulated number of vertical sync and back porch lines
     * (minus 1).*/
    const uint32_t back_porch = ((info->timing_info.hbp << 16) +
                                 info->timing_info.vbp         +
                                 sync_size                       );

    /* The accumulated number of horizontal sync, back porch and active
     * pixels and the accumulated number of vertical sync, back porch
     * and active lines (minus 1).*/
    const uint32_t active_width = ((info->timing_info.hactive << 16) +
                                   info->timing_info.vactive         +
                                   back_porch                          );

    /* The accumulated number of horizontal sync, back porch, active
     * and front porch pixels and the accumulated number of vertical
     * sync, back porch, active and front porch lines (minus 1). */
    const uint32_t total_width = ((info->timing_info.hfp << 16) +
                                  info->timing_info.vfp         +
                                  active_width                    );

    /* Set frame timings */
    cdc->CDC_SYNC_SIZE_CFG = sync_size;
    cdc->CDC_BP_CFG = back_porch;
    cdc->CDC_ACTW_CFG = active_width;
    cdc->CDC_TOTALW_CFG = total_width;

    /* Set Background color */
    cdc->CDC_BACKGND_COLOR = ((info->bgc.red << 16)  |
                              (info->bgc.green << 8) |
                              info->bgc.blue          );

    /* Set scanline irq position */
    cdc->CDC_LINE_IRQ_POS = info->line_irq_pos;

    /* Trigger Shadow register update */
    cdc->CDC_SRCTRL = (1UL << info->sh_rld);
}

/**
 * @fn      void cdc_set_hsync_polarity(CDC_Type *const cdc, const CDC_POLARITY hsync_pol)
 * @brief   CDC hsync polarity.
 * @param   cdc  Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   hsync_pol  CDC HSYNC polarity
 * @retval  none.
 */
void cdc_set_hsync_polarity(CDC_Type *const cdc, const CDC_POLARITY hsync_pol)
{
    if(hsync_pol == CDC_POLARITY_ACTIVE_HIGH)
    {
        cdc->CDC_GLB_CTRL |= CDC_GLB_CTRL_HSPOL;
    }
    else
    {
        cdc->CDC_GLB_CTRL &= ~CDC_GLB_CTRL_HSPOL;
    }
}

/**
 * @fn      void cdc_set_vsync_polarity(CDC_Type *const cdc, const CDC_POLARITY vsync_pol)
 * @brief   CDC vsync polarity.
 * @param   cdc  Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   vsync_pol  CDC VSYNC polarity
 * @retval  none.
 */
void cdc_set_vsync_polarity(CDC_Type *const cdc, const CDC_POLARITY vsync_pol)
{
    if(vsync_pol == CDC_POLARITY_ACTIVE_HIGH)
    {
        cdc->CDC_GLB_CTRL |= CDC_GLB_CTRL_VSPOL;
    }
    else
    {
        cdc->CDC_GLB_CTRL &= ~CDC_GLB_CTRL_VSPOL;
    }
}

/**
 * @fn      void cdc_set_pclkout_polarity(CDC_Type *const cdc, const CDC_PIXCLK_POLARITY pclkout_pol)
 * @brief   CDC pixel clock output polarity.
 * @param   cdc  Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   pclkout_pol  CDC pixel clock output polarity
 * @retval  none.
 */
void cdc_set_pclkout_polarity(CDC_Type *const cdc, const CDC_PIXCLK_POLARITY pclkout_pol)
{
    if(pclkout_pol == CDC_PIXCLK_POLARITY_INVERTED)
    {
        cdc->CDC_GLB_CTRL |= CDC_GLB_CTRL_PCLKPOL;
    }
    else
    {
        cdc->CDC_GLB_CTRL &= ~CDC_GLB_CTRL_PCLKPOL;
    }
}

/**
 * @fn      void cdc_set_blank_polarity(CDC_Type *const cdc, const CDC_POLARITY blank_pol)
 * @brief   CDC blank polarity.
 * @param   cdc  Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   blank_pol  CDC blank polarity
 * @retval  none.
 */
void cdc_set_blank_polarity(CDC_Type *const cdc, const CDC_POLARITY blank_pol)
{
    if(blank_pol == CDC_POLARITY_ACTIVE_HIGH)
    {
        cdc->CDC_GLB_CTRL |= CDC_GLB_CTRL_BLPOL;
    }
    else
    {
        cdc->CDC_GLB_CTRL &= ~CDC_GLB_CTRL_BLPOL;
    }
}

/**
 * @fn      void cdc_set_layer_cfg (CDC_Type *const cdc, const CDC_LAYER layer, const cdc_layer_info_t *const info)
 * @brief   Configure the CDC layer with given information.
 * @param   cdc   Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   layer The layer number needs to be configure. See {@ref CDC_LAYER} for details.
 * @param   info  Pointer to the cdc layer information structure. See {@ref cdc_layer_info_t} for details.
 * @retval  none
 */
void cdc_set_layer_cfg (CDC_Type *const cdc, const CDC_LAYER layer, const cdc_layer_info_t *const info)
{
    /* Color FB Length variable */
    uint32_t fb_length;
    /* CDC Layer configuration */
    volatile CDC_CDC_LAYER_CFG_Type *cdc_layer = &(cdc->CDC_LAYER_CFG[layer]);

    /* Calculate frame buffer length */
    switch (info->pix_format)
    {
        case CDC_PIXEL_FORMAT_ARGB8888:
        case CDC_PIXEL_FORMAT_RGBA8888:
            /* In ARGB8888/RGBA8888 standard One pixel handled by 4 byte,
             * so line length size multiplied with 4 */
            fb_length = (((info->line_length_in_pixels * 4) << 16) |
                         ((info->line_length_in_pixels * 4) + BUS_WIDTH));
            break;
        case CDC_PIXEL_FORMAT_RGB888:
            /* In RGB888 standard One pixel handled by 3 byte,
             * so line length size multiplied with 3 */
            fb_length = (((info->line_length_in_pixels * 3) << 16) |
                         ((info->line_length_in_pixels * 3) + BUS_WIDTH));
            break;
        case CDC_PIXEL_FORMAT_RGB565:
        case CDC_PIXEL_FORMAT_ARGB4444:
        case CDC_PIXEL_FORMAT_ARGB1555:
            /* In RGB565/ARGB4444/ARGB1555 standard One pixel handled by 2 byte,
             * so width size multiplied with 2 */
            fb_length = (((info->line_length_in_pixels * 2) << 16) |
                         ((info->line_length_in_pixels * 2) + BUS_WIDTH));
            break;
        case CDC_PIXEL_FORMAT_AL44:
        case CDC_PIXEL_FORMAT_AL8:
            fb_length = (((info->line_length_in_pixels * 1) << 16) |
                         ((info->line_length_in_pixels * 1) + BUS_WIDTH));
            break;
        default:
            return ;
    }

    /* Mask the global shadow reload */
    cdc_layer->CDC_L_REL_CTRL = CDC_Ln_REL_CTRL_SH_MASK;

    /* Set layer Window */
    cdc_layer->CDC_L_WIN_HPOS = ((info->win_info.h_stop_pos << 16) |
                                 info->win_info.h_start_pos         );
    cdc_layer->CDC_L_WIN_VPOS = ((info->win_info.v_stop_pos << 16) |
                                 info->win_info.v_start_pos         );

    /* Set pixel format */
    cdc_layer->CDC_L_PIX_FORMAT = info->pix_format;

    /* Set constant Alpha */
    cdc_layer->CDC_L_CONST_ALPHA = info->const_alpha;

    /* Set layer blending factor */
    if (info->blend_factor == CDC_BLEND_FACTOR_CONST_ALPHA)
    {
        cdc_layer->CDC_L_BLEND_CFG = ((CDC_BLEND_CONST_ALPHA << CDC_Ln_BLEND_CFG_F1_SEL_SHIFT) |
                                                              (CDC_BLEND_CONST_ALPHA_INV)       );
    }
    else
    {
        cdc_layer->CDC_L_BLEND_CFG = ((CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA << CDC_Ln_BLEND_CFG_F1_SEL_SHIFT) |
                                                              (CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA_INV)       );
    }

    /* Set color frame buffer Address */
    cdc_layer->CDC_L_CFB_ADDR = info->fb_addr;

    /* Set the pitch and the line length of the color frame buffer*/
    cdc_layer->CDC_L_CFB_LENGTH = fb_length;

    /* Set color frame buffer lines */
    cdc_layer->CDC_L_CFB_LINES = info->num_lines;

    /* Trigger shadow register update */
    cdc_layer->CDC_L_REL_CTRL |= (1UL << info->sh_rld);
}

/**
 * @fn      void cdc_layer_on (CDC_Type *const cdc, const CDC_LAYER layer, const CDC_SHADOW_RELOAD sh_rld)
 * @brief   CDC layer on.
 * @param   cdc     Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   layer   The layer number needs to be configure. See {@ref CDC_LAYER} for details.
 * @param   sh_rld  The shadow register update method. See {@ref CDC_SHADOW_RELOAD} for details.
 * @retval  none
 */
void cdc_layer_on (CDC_Type *const cdc, const CDC_LAYER layer, const CDC_SHADOW_RELOAD sh_rld)
{
    /* Enable Layer on */
    cdc->CDC_LAYER_CFG[layer].CDC_L_CTRL |= CDC_Ln_CTRL_LAYER_EN;

    /* Trigger shadow register update */
    cdc->CDC_LAYER_CFG[layer].CDC_L_REL_CTRL |= (1UL << sh_rld);
}

/**
 * @fn      void cdc_layer_off (CDC_Type *const cdc, const CDC_LAYER layer, const CDC_SHADOW_RELOAD sh_rld)
 * @brief   CDC layer off.
 * @param   cdc     Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   layer   The layer number needs to be configure. See {@ref _CDC_LAYER} for details.
 * @param   sh_rld  The shadow register update method. See {@ref CDC_SHADOW_RELOAD} for details.
 * @retval  none
 */
void cdc_layer_off (CDC_Type *const cdc, const CDC_LAYER layer, const CDC_SHADOW_RELOAD sh_rld)
{
    /* Enable Layer off */
    cdc->CDC_LAYER_CFG[layer].CDC_L_CTRL &= ~CDC_Ln_CTRL_LAYER_EN;

    /* Trigger shadow register update */
    cdc->CDC_LAYER_CFG[layer].CDC_L_REL_CTRL |= (1UL << sh_rld);
}

/**
 * @fn      void cdc_set_layer_fb_addr (CDC_Type *const cdc, const CDC_LAYER layer,
 *                                      const CDC_SHADOW_RELOAD sh_rld, const uint32_t fb_addr)
 * @brief   Set layer frame buffer address..
 * @param   cdc      Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   layer    The layer number needs to be configure. See {@ref CDC_LAYER} for details.
 * @param   sh_rld   The shadow register update method. See {@ref CDC_SHADOW_RELOAD} for details.
 * @param   fb_addr  The Color FB start address.
 * @retval  none
 */
void cdc_set_layer_fb_addr (CDC_Type *const cdc, const CDC_LAYER layer,
                            const CDC_SHADOW_RELOAD sh_rld, const uint32_t fb_addr)
{
    /* Set layer frame buffer */
    cdc->CDC_LAYER_CFG[layer].CDC_L_CFB_ADDR = fb_addr;

    /* Trigger shadow register update */
    cdc->CDC_LAYER_CFG[layer].CDC_L_REL_CTRL |= (1UL << sh_rld);
}

/**
 * @fn      void cdc_set_layer_fb_window (CDC_Type *const cdc, const CDC_LAYER layer,
 *                                        const CDC_SHADOW_RELOAD sh_rld, const cdc_window_info_t *win_info)
 * @brief   Set layer frame buffer window.
 * @param   cdc       Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   layer     The layer number needs to be configure. See {@ref CDC_LAYER} for details.
 * @param   sh_rld    The shadow register update method. See {@ref CDC_SHADOW_RELOAD} for details.
 * @param   win_info  Pointer to the layer window information structure. See {@ref cdc_window_info_t} for details.
 * @retval  none
 */
void cdc_set_layer_fb_window (CDC_Type *const cdc, const CDC_LAYER layer,
                              const CDC_SHADOW_RELOAD sh_rld, const cdc_window_info_t *win_info)
{
    /* Set layer Window */
    cdc->CDC_LAYER_CFG[layer].CDC_L_WIN_HPOS = ((win_info->h_stop_pos << 16) |
                                                win_info->h_start_pos         );
    cdc->CDC_LAYER_CFG[layer].CDC_L_WIN_VPOS = ((win_info->v_stop_pos << 16) |
                                                win_info->v_start_pos         );

    /* Trigger shadow register update */
    cdc->CDC_LAYER_CFG[layer].CDC_L_REL_CTRL |= (1UL << sh_rld);
}

/**
 * @fn      void cdc_set_layer_blending (CDC_Type *const cdc, const CDC_LAYER layer, const CDC_SHADOW_RELOAD sh_rld,
 *                                       const uint8_t const_alpha, const CDC_BLEND_FACTOR blend_factor             )
 * @brief   Set layer blending.
 * @param   cdc           Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   layer         The layer number needs to be configure. See {@ref CDC_LAYER} for details.
 * @param   sh_rld        The shadow register update method. See {@ref CDC_SHADOW_RELOAD} for details.
 * @param   const_alpha   The layer constant alpha range from 0 (fully transparent) to 255 or 1.0 (fully opaque).
 * @param   blend_factor  The layer blending factor selection. See {@ref CDC_BLEND_FACTOR} for details.
 * @retval  none
 */
void cdc_set_layer_blending (CDC_Type *const cdc, const CDC_LAYER layer, const CDC_SHADOW_RELOAD sh_rld,
                             const uint8_t const_alpha, const CDC_BLEND_FACTOR blend_factor             )
{
    /* CDC Layer configuration */
    volatile CDC_CDC_LAYER_CFG_Type *cdc_layer = &(cdc->CDC_LAYER_CFG[layer]);

    /* Set constant alpha */
    cdc_layer->CDC_L_CONST_ALPHA = const_alpha;

    /* Set layer blending factor */
    if (blend_factor == CDC_BLEND_FACTOR_CONST_ALPHA)
    {
        cdc_layer->CDC_L_BLEND_CFG = ((CDC_BLEND_CONST_ALPHA  << CDC_Ln_BLEND_CFG_F1_SEL_SHIFT) |
                                      (CDC_BLEND_CONST_ALPHA_INV)                                );
    }
    else
    {
        cdc_layer->CDC_L_BLEND_CFG = ((CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA << CDC_Ln_BLEND_CFG_F1_SEL_SHIFT) |
                                      (CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA_INV)                               );
    }

    /* Trigger shadow register update */
    cdc_layer->CDC_L_REL_CTRL |= (1UL << sh_rld);
}

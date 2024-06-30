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
 * @file     cdc.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     10-April-2023
 * @brief    Low level driver Specific Header file.
 ******************************************************************************/

#ifndef CDC_H_
#define CDC_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>

/**
 * @struct CDC_CDC_LAYER_CFG_Type
 * @brief CDC_CDC_LAYER_CFG [CDC_LAYER_CFG] ([0..1])
 */
typedef struct {
    volatile const uint32_t  CDC_L_CFG1;                   /*!< (@ 0x00000000) Layer (n) Configuration 1 Register                         */
    volatile const uint32_t  CDC_L_CFG2;                   /*!< (@ 0x00000004) Layer (n) Configuration 2 Register                         */
    volatile       uint32_t  CDC_L_REL_CTRL;               /*!< (@ 0x00000008) Layer (n) Shadow Reload Control Register                   */
    volatile       uint32_t  CDC_L_CTRL;                   /*!< (@ 0x0000000C) Layer (n) Control Register                                 */
    volatile       uint32_t  CDC_L_WIN_HPOS;               /*!< (@ 0x00000010) Layer (n) Window Horizontal Position Register              */
    volatile       uint32_t  CDC_L_WIN_VPOS;               /*!< (@ 0x00000014) Layer (n) Window Vertical Position Register                */
    volatile       uint32_t  CDC_L_CKEY;                   /*!< (@ 0x00000018) Layer (n) Color Key Register                               */
    volatile       uint32_t  CDC_L_PIX_FORMAT;             /*!< (@ 0x0000001C) Layer (n) Pixel Format Register                            */
    volatile       uint32_t  CDC_L_CONST_ALPHA;            /*!< (@ 0x00000020) Layer (n) Constant Alpha Register                          */
    volatile       uint32_t  CDC_L_DFLT_COLOR;             /*!< (@ 0x00000024) Layer (n) Default Color Register                           */
    volatile       uint32_t  CDC_L_BLEND_CFG;              /*!< (@ 0x00000028) Layer (n) Blending Factors Register                        */
    volatile       uint32_t  CDC_L_FB_BCTRL;               /*!< (@ 0x0000002C) Layer (n) FB Bus Control Register                          */
    volatile const uint32_t  RESERVED;
    volatile       uint32_t  CDC_L_CFB_ADDR;               /*!< (@ 0x00000034) Layer (n) Color FB Address Register                        */
    volatile       uint32_t  CDC_L_CFB_LENGTH;             /*!< (@ 0x00000038) Layer (n) Color FB Length Register                         */
    volatile       uint32_t  CDC_L_CFB_LINES;              /*!< (@ 0x0000003C) Layer (n) Color FB Lines Register                          */
    volatile const uint32_t  RESERVED1[4];
    volatile       uint32_t  CDC_L_CLUT_WRACC;             /*!< (@ 0x00000050) Layer (n) CLUT Write Access Register                       */
    volatile const uint32_t  RESERVED2[43];
} CDC_CDC_LAYER_CFG_Type;                                  /*!< Size = 256 (0x100)                                                        */

/**
 * @struct CDC_Type
 * @brief  cdc register map structure.
 */
typedef struct {                                           /*!< (@ 0x49031000) CDC Structure                                              */
    volatile const uint32_t CDC_HW_VER;                    /*!< (@ 0x00000000) HW Version Register                                        */
    volatile const uint32_t CDC_LCNT;                      /*!< (@ 0x00000004) Layer Count Register                                       */
    volatile       uint32_t CDC_SYNC_SIZE_CFG;             /*!< (@ 0x00000008) Sync Size Register                                         */
    volatile       uint32_t CDC_BP_CFG;                    /*!< (@ 0x0000000C) Back Porch Register                                        */
    volatile       uint32_t CDC_ACTW_CFG;                  /*!< (@ 0x00000010) Active Width Register                                      */
    volatile       uint32_t CDC_TOTALW_CFG;                /*!< (@ 0x00000014) Total Width Register                                       */
    volatile       uint32_t CDC_GLB_CTRL;                  /*!< (@ 0x00000018) Global Control Register                                    */
    volatile const uint32_t CDC_CFG1;                      /*!< (@ 0x0000001C) Global Configuration 1 Register                            */
    volatile const uint32_t CDC_CFG2;                      /*!< (@ 0x00000020) Global Configuration 2 Register                            */
    volatile       uint32_t CDC_SRCTRL;                    /*!< (@ 0x00000024) Shadow Reload Control Register                             */
    volatile       uint32_t CDC_GAMMA_CORR;                /*!< (@ 0x00000028) Gamma Correction Register                                  */
    volatile       uint32_t CDC_BACKGND_COLOR;             /*!< (@ 0x0000002C) Background Color Register                                  */
    volatile const uint32_t RESERVED;
    volatile       uint32_t CDC_IRQ_MASK0;                 /*!< (@ 0x00000034) IRQ Enable 1 Register                                      */
    volatile const uint32_t CDC_IRQ_STATUS0;               /*!< (@ 0x00000038) IRQ Status 1 Register                                      */
    volatile       uint32_t CDC_IRQ_CLEAR0;                /*!< (@ 0x0000003C) IRQ Clear Register                                         */
    volatile       uint32_t CDC_LINE_IRQ_POS;              /*!< (@ 0x00000040) Line Number IRQ Control Register                           */
    volatile const uint32_t CDC_POS_STAT;                  /*!< (@ 0x00000044) Position Status Register                                   */
    volatile const uint32_t CDC_SYNC_BLANK_STAT;           /*!< (@ 0x00000048) Sync/Blank Status Register                                 */
    volatile const uint32_t RESERVED1[9];
    volatile       uint32_t CDC_SLINE_IRQ_POS;             /*!< (@ 0x00000070) Secure Line IRQ Position Control Register                  */
    volatile const uint32_t RESERVED2[35];
    volatile       CDC_CDC_LAYER_CFG_Type CDC_LAYER_CFG[2];/*!< (@ 0x00000100) [0..1]                                                     */
} CDC_Type;                                                /*!< Size = 768 (0x300)                                                        */

/*CDC configurations*/

/**
 * @brief The Bus width value 7 relates to the bus width (8 bytes in this configuration)
 *        and serves to correctly calculate the address of the last word of data for
 *        that line internally.
 */
#define BUS_WIDTH                         7U

/**
 * @brief Blend factor used for blending between layers
 */
#define CDC_BLEND_CONST_ALPHA                    4
#define CDC_BLEND_CONST_ALPHA_INV                5
#define CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA      6
#define CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA_INV  7

/*CDC Register Descriptions*/

/**
 * @brief CDC_GLB_CTRL bit parameters.
 */
#define CDC_GLB_CTRL_HSPOL               (1U << 31)  /* HSYNC polarity */
#define CDC_GLB_CTRL_VSPOL               (1U << 30)  /* VSYNC polarity */
#define CDC_GLB_CTRL_BLPOL               (1U << 29)  /* Blank polarity */
#define CDC_GLB_CTRL_PCLKPOL             (1U << 28)  /* Output Pixel clock polarity */
#define CDC_GLB_CTRL_EN                  (1U << 0)   /**< CDC global enable*/

/**
 * @brief  CDC_Ln_CTRL bit parameter.
 */
#define CDC_Ln_CTRL_LAYER_EN             1U          /**< CDC layer On */

/**
 * @brief CDC_IRQ control bit parameters
 */
#define CDC_IRQ_LINE                     (1U << 0)   /**< Line IRQ */
#define CDC_IRQ_FIFO_UNDERRUN_WARNING    (1U << 1)   /**< FIFO Underrun Warning IRQ */
#define CDC_IRQ_BUS_ERROR                (1U << 2)   /**< Bus error IRQ */
#define CDC_IRQ_REGISTER_RELOAD          (1U << 3)   /**< Register reload IRQ */
#define CDC_IRQ_SLAVE_TIMING_NO_SIGNAL   (1U << 4)   /**< Slave timing no signal IRQ */
#define CDC_IRQ_SLAVE_TIMING_NOT_IN_SYNC (1U << 5)   /**< Slave timing not in sync IRQ */
#define CDC_IRQ_FIFO_UNDERRUN_KILLING    (1U << 6)   /**< FIFO underrun killing IRQ */
#define CDC_IRQ_CRC                      (1U << 7)   /**< CRC IRQ */
#define CDC_IRQ_ROTATION_FIFO_ERROR      (1U << 8)   /**< Rotation FIFO error interrupt */

/**
 * @brief CDC_Ln_REL_CTRL bit parameter.
 */
#define CDC_Ln_REL_CTRL_SH_MASK          (1U << 2)   /**< CDC layer shadow reload mask */

/**
 * @brief CDC_Ln_BLEND_CFG bit parameter.
 */
#define CDC_Ln_BLEND_CFG_F1_SEL_SHIFT    (8U)       /**< CDC layer selection of blending factor f1 position bit*/

/**
 * enum   CDC_POLARITY
 * @brief CDC polarity.
 */
typedef enum _CDC_POLARITY{
    CDC_POLARITY_ACTIVE_LOW,                  /**< CDC polarity low  */
    CDC_POLARITY_ACTIVE_HIGH                  /**< CDC polarity high */
} CDC_POLARITY;

/**
 * enum   CDC_PIXCLK_POLARITY
 * @brief CDC pixel clock polarity.
 */
typedef enum _CDC_PIXCLK_POLARITY{
    CDC_PIXCLK_POLARITY_FEED_THROUGH,         /**<  Feed-through of PIXEL_CLK input */
    CDC_PIXCLK_POLARITY_INVERTED              /**<  Inverted PIXEL_CLK input        */
} CDC_PIXCLK_POLARITY;

/**
 * @enum   CDC_PIXEL_FORMAT
 * @brief  CDC Supported Pixel format.
 */
typedef enum _CDC_PIXEL_FORMAT
{
    CDC_PIXEL_FORMAT_ARGB8888,                /**< 32-bit ARGB */
    CDC_PIXEL_FORMAT_RGB888,                  /**< 24-bit RGB (A = 255) */
    CDC_PIXEL_FORMAT_RGB565,                  /**< 16-bit RGB (A = 255) */
    CDC_PIXEL_FORMAT_RGBA8888,                /**< 32-bit RGBA */
    CDC_PIXEL_FORMAT_AL44,                    /**< 8-bit alpha + luminance (lower channel on R, G and B) */
    CDC_PIXEL_FORMAT_AL8,                     /**< 8-bit single channel (value on A, R, G and B) */
    CDC_PIXEL_FORMAT_ARGB1555,                /**< 16-bit ARGB with 1 bit alpha */
    CDC_PIXEL_FORMAT_ARGB4444                 /**< 16-bit ARGB with 4 bits alpha */
} CDC_PIXEL_FORMAT;

/**
 * @enum   CDC_LAYER
 * @brief  CDC layers supported.
 */
typedef enum _CDC_LAYER
{
    CDC_LAYER_1,            /**< cdc layer 1 */
    CDC_LAYER_2,            /**< cdc layer 2 */
} CDC_LAYER;

/**
 * @enum   CDC_SHADOW_RELOAD
 * @brief  CDC shadow register immediate or delayed update.
 */
typedef enum _CDC_SHADOW_RELOAD
{
    CDC_SHADOW_RELOAD_IMR,      /**< cdc Immediate reload */
    CDC_SHADOW_RELOAD_VBR,      /**< cdc Vertical blanking reload */
} CDC_SHADOW_RELOAD;

/**
 * @enum   CDC_BLEND_FACTOR
 * @brief  CDC Selection of layer blending factor.
 */
typedef enum _CDC_BLEND_FACTOR
{
    CDC_BLEND_FACTOR_CONST_ALPHA,                /**< cdc blending factor selected to constant alpha */
    CDC_BLEND_FACTOR_PIXEL_ALPHA_X_CONST_ALPHA   /**< cdc blending factor selected to pixel alpha * constant alpha */
} CDC_BLEND_FACTOR;

/**
 * @struct  cdc_timing_info_t
 * @brief   CDC video timing parameters information.
 */
typedef struct _cdc_timing_info_t
{
    uint16_t  hsync;             /**< Horizontal sync width */
    uint16_t  hbp;               /**< Horizontal back porch */
    uint16_t  hfp;               /**< Horizontal front porch */
    uint16_t  hactive;           /**< Horizontal active width */
    uint16_t  vsync;             /**< Vertical sync width */
    uint16_t  vbp;               /**< Vertical back porch */
    uint16_t  vfp;               /**< Vertical front porch */
    uint16_t  vactive;           /**< Vertical active width */
} cdc_timing_info_t;

/**
 * @struct cdc_backgnd_info_t
 * @brief  CDC Background color information
 */
typedef struct _cdc_backgnd_color_info_t
{
    uint8_t  red;                       /**< Back ground color red */
    uint8_t  green;                     /**< Back ground color green */
    uint8_t  blue;                      /**< Back ground color blue */
} cdc_backgnd_color_info_t;

/**
 * @struct  cdc_cfg_info_t
 * @brief   CDC configuration parameters information.
 */
typedef struct _cdc_cfg_info_t
{
    cdc_timing_info_t         timing_info;   /**< CDC video timing parameters */
    uint16_t                  line_irq_pos;  /**< Line IRQ position */
    cdc_backgnd_color_info_t  bgc;           /**< CDC Background color */
    CDC_SHADOW_RELOAD         sh_rld;        /**< shadow register reload */
} cdc_cfg_info_t;

/**
 * @struct  cdc_window_info_t
 * @brief   CDC Layer Window information.
 */
typedef struct _cdc_window_info_t
{
    uint16_t v_start_pos;           /**< Vertical Start Position */
    uint16_t v_stop_pos;            /**< Vertical Stop Position */
    uint16_t h_start_pos;           /**< Horizontal Start Position */
    uint16_t h_stop_pos;            /**< Horizontal Stop Position */
} cdc_window_info_t;

/**
 * @struct  cdc_layer_info_t
 * @brief   CDC layer information.
 */
typedef struct _cdc_layer_info_t
{
    cdc_window_info_t  win_info;               /**< Layer Window information */
    CDC_PIXEL_FORMAT   pix_format;             /**< Layer Pixel format */
    uint8_t            const_alpha;            /**< Layer Constant Alpha */
    CDC_BLEND_FACTOR   blend_factor;           /**< Layer Blending factor */
    uint32_t           fb_addr;                /**< Layer Color FB Length Register */
    uint16_t           line_length_in_pixels;  /**< Layer Line length in pixels of color FB */
    uint16_t           num_lines;              /**< Layer number of lines in the color FB */
    CDC_SHADOW_RELOAD  sh_rld;                 /**< Shadow register reload */
} cdc_layer_info_t;

/**
 * @fn      static inline void cdc_global_enable (CDC_Type *const cdc)
 * @brief   CDC global enable.
 * @param   cdc  Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @retval  none.
 */
static inline void cdc_global_enable (CDC_Type *const cdc)
{
    cdc->CDC_GLB_CTRL |= CDC_GLB_CTRL_EN;
}

/**
 * @fn      static inline void cdc_global_disable (CDC_Type *const cdc)
 * @brief   CDC global disable.
 * @param   cdc  Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @retval  none.
 */
static inline void cdc_global_disable (CDC_Type *const cdc)
{
    cdc->CDC_GLB_CTRL &= ~CDC_GLB_CTRL_EN;
}

/**
 * @fn      static inline void cdc_set_backgnd_color (CDC_Type *const cdc, const CDC_SHADOW_RELOAD sh_rld,
                                                      const cdc_backgnd_color_info_t *bgc                 )
 * @brief   Set CDC Background color.
 * @param   cdc     Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   sh_rld  The shadow register update method. See {@ref CDC_SHADOW_RELOAD} for details.
 * @param   bgc     Pointer to the cdc background color information structure. See (@ref cdc_backgnd_color_info_t) for details.
 * @retval  none.
 */
static inline void cdc_set_backgnd_color (CDC_Type *const cdc, const CDC_SHADOW_RELOAD sh_rld,
                                          const cdc_backgnd_color_info_t *bgc                 )
{
    /* Set Background color */
    cdc->CDC_BACKGND_COLOR = ((bgc->red << 16)  |
                              (bgc->green << 8) |
                              bgc->blue          );

    /* Trigger Shadow register update */
    cdc->CDC_SRCTRL = (1UL << sh_rld);
}

/**
 * @fn      static inline void cdc_irq_enable (CDC_Type *const cdc, const uint32_t irq_mask)
 * @brief   Enable cdc interrupts.
 * @param   cdc       Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   irq_mask  cdc interrupts to enable. See {@ref CDC_IRQ_*} for details.
 * @return  none.
 */
static inline void cdc_irq_enable (CDC_Type *const cdc, const uint32_t irq_mask)
{
    cdc->CDC_IRQ_MASK0 |= irq_mask;
}

/**
 * @fn      static inline void cdc_irq_disable (CDC_Type *const cdc, const uint32_t irq_mask)
 * @brief   Disable cdc interrupts.
 * @param   cdc       Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   irq_mask  cdc interrupts to disable. See {@ref CDC_IRQ_*} for details.
 * @return  none.
 */
static inline void cdc_irq_disable (CDC_Type *const cdc, const uint32_t irq_mask)
{
    cdc->CDC_IRQ_MASK0 &= ~irq_mask;
}

/**
 * @fn      static inline uint32_t cdc_get_irq_status(CDC_Type *cdc)
 * @brief   Get the cdc irq status.
 * @param   cdc  Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @return  current irq status. See {@ref CDC_IRQ_*} for details.
*/
static inline uint32_t cdc_get_irq_status(CDC_Type *const cdc)
{
    return cdc->CDC_IRQ_STATUS0;
}

/**
 * @fn      static inline void cdc_irq_clear (CDC_Type *const cdc, const uint32_t irq_mask)
 * @brief   Clear cdc interrupts.
 * @param   cdc       Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   irq_mask  cdc interrupts to disable. See {@ref CDC_IRQ_*} for details.
 * @return  none.
 */
static inline void cdc_irq_clear (CDC_Type *const cdc, const uint32_t irq_mask)
{
    cdc->CDC_IRQ_CLEAR0 |= irq_mask;
    (void) cdc->CDC_IRQ_CLEAR0;
}

/**
 * @fn      static inline uint16_t cdc_get_y_position_status (CDC_Type *const cdc)
 * @brief   Get current Y position value.
 * @param   cdc  Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @return  current Y position.
 */
static inline uint16_t cdc_get_y_position_status (CDC_Type *const cdc)
{
    return (uint16_t)(cdc->CDC_POS_STAT);
}

/**
 * @fn      void cdc_set_cfg (CDC_Type *const cdc, const cdc_cfg_info_t *const info)
 * @brief   Configure the CDC with given information.
 * @param   cdc   Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   info  Pointer to the cdc configuration structure. See {@ref cdc_cfg_info_t} for details.
 * @retval  none
 */
void cdc_set_cfg (CDC_Type *const cdc, const cdc_cfg_info_t *const info);

/**
 * @fn      void cdc_set_hsync_polarity(CDC_Type *const cdc, const CDC_POLARITY hsync_pol)
 * @brief   CDC hsync polarity.
 * @param   cdc  Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   hsync_pol  CDC HSYNC polarity
 * @retval  none.
 */
void cdc_set_hsync_polarity(CDC_Type *const cdc, const CDC_POLARITY hsync_pol);

/**
 * @fn      void cdc_set_vsync_polarity(CDC_Type *const cdc, const CDC_POLARITY vsync_pol)
 * @brief   CDC vsync polarity.
 * @param   cdc  Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   vsync_pol  CDC VSYNC polarity
 * @retval  none.
 */
void cdc_set_vsync_polarity(CDC_Type *const cdc, const CDC_POLARITY vsync_pol);

/**
 * @fn      void cdc_set_pclkout_polarity(CDC_Type *const cdc, const CDC_PIXCLK_POLARITY pclkout_pol)
 * @brief   CDC pixel clock output polarity.
 * @param   cdc  Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   pclkout_pol  CDC pixel clock output polarity
 * @retval  none.
 */
void cdc_set_pclkout_polarity(CDC_Type *const cdc, const CDC_PIXCLK_POLARITY pclkout_pol);

/**
 * @fn      void cdc_set_blank_polarity(CDC_Type *const cdc, const CDC_POLARITY blank_pol)
 * @brief   CDC blank polarity.
 * @param   cdc  Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   blank_pol  CDC blank polarity
 * @retval  none.
 */
void cdc_set_blank_polarity(CDC_Type *const cdc, const CDC_POLARITY blank_pol);

/**
 * @fn      void cdc_set_layer_cfg (CDC_Type *const cdc, const CDC_LAYER layer, const cdc_layer_info_t *const info)
 * @brief   Configure the CDC layer with given information.
 * @param   cdc   Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   layer The layer number needs to be configure. See {@ref CDC_LAYER} for details.
 * @param   info  Pointer to the cdc layer information structure. See {@ref cdc_layer_info_t} for details.
 * @retval  none
 */
void cdc_set_layer_cfg (CDC_Type *const cdc, const CDC_LAYER layer, const cdc_layer_info_t *const info);

/**
 * @fn      void cdc_layer_on (CDC_Type *const cdc, const CDC_LAYER layer, const CDC_SHADOW_RELOAD sh_rld)
 * @brief   CDC layer on.
 * @param   cdc     Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   layer   The layer number needs to be configure. See {@ref CDC_LAYER} for details.
 * @param   sh_rld  The shadow register update method. See {@ref CDC_SHADOW_RELOAD} for details.
 * @retval  none
 */
void cdc_layer_on (CDC_Type *const cdc, const CDC_LAYER layer, const CDC_SHADOW_RELOAD sh_rld);

/**
 * @fn      void cdc_layer_off (CDC_Type *const cdc, const CDC_LAYER layer, const CDC_SHADOW_RELOAD sh_rld)
 * @brief   CDC layer off.
 * @param   cdc     Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   layer   The layer number needs to be configure. See {@ref CDC_LAYER} for details.
 * @param   sh_rld  The shadow register update method. See {@ref CDC_SHADOW_RELOAD} for details.
 * @retval  none
 */
void cdc_layer_off (CDC_Type *const cdc, const CDC_LAYER layer, const CDC_SHADOW_RELOAD sh_rld);

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
                            const CDC_SHADOW_RELOAD sh_rld, const uint32_t fb_addr);

/**
 * @fn      void cdc_set_layer_fb_window (CDC_Type *const cdc, const CDC_LAYER layer,
 *                                        const CDC_SHADOW_RELOAD sh_rld, const cdc_window_info_t *const win_info)
 * @brief   Set layer frame buffer window.
 * @param   cdc       Pointer to the cdc register map structure. See {@ref CDC_Type} for details.
 * @param   layer     The layer number needs to be configure. See {@ref CDC_LAYER} for details.
 * @param   sh_rld    The shadow register update method. See {@ref CDC_SHADOW_RELOAD} for details.
 * @param   win_info  Pointer to the layer window information structure. See {@ref cdc_window_info_t} for details.
 * @retval  none
 */
void cdc_set_layer_fb_window (CDC_Type *const cdc, const CDC_LAYER layer,
                              const CDC_SHADOW_RELOAD sh_rld, const cdc_window_info_t *const win_info);

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
                             const uint8_t const_alpha, const CDC_BLEND_FACTOR blend_factor             );

#ifdef __cplusplus
}
#endif
#endif /* CDC_H_ */

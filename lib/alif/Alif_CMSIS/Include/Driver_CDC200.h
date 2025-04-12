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
 * @file     Driver_CDC200.h
 * @author   Girish BN and Prasanna Ravi
 * @email    girish.bn@alifsemi.com and prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     30-Sep-2021
 * @brief    Display controller driver header.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#ifndef DRIVER_CDC200_H_
#define DRIVER_CDC200_H_

#include "Driver_Common.h"

#ifdef  __cplusplus
extern "C"
{
#endif

#define ARM_CDC200_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

/****** CDC200 Background color blue *****/
#define ARM_CDC200_BGC_BLUE_Pos        0UL       ///< bits 7..0
#define ARM_CDC200_BGC_BLUE_Msk       (0xFFUL << ARM_CDC200_BGC_BLUE_Pos)
#define ARM_CDC200_BGC_BLUE(x)      (((x)     << ARM_CDC200_BGC_BLUE_Pos) & ARM_CDC200_BGC_BLUE_Msk)

/****** CDC200 Background color green *****/
#define ARM_CDC200_BGC_GREEN_Pos       8UL       ///< bits 15..8
#define ARM_CDC200_BGC_GREEN_Msk      (0xFFUL << ARM_CDC200_BGC_GREEN_Pos)
#define ARM_CDC200_BGC_GREEN(x)     (((x)     << ARM_CDC200_BGC_GREEN_Pos) & ARM_CDC200_BGC_GREEN_Msk)

/****** CDC200 Background color red *****/
#define ARM_CDC200_BGC_RED_Pos         16UL       ///< bits 23..16
#define ARM_CDC200_BGC_RED_Msk        (0xFFUL << ARM_CDC200_BGC_RED_Pos)
#define ARM_CDC200_BGC_RED(x)       (((x)     << ARM_CDC200_BGC_RED_Pos) & ARM_CDC200_BGC_RED_Msk)

/****** CDC200 Control Function Operation codes *****/
#define CDC200_CONFIGURE_DISPLAY         (1U << 0)    ///< Configure Display
#define CDC200_FRAMEBUF_UPDATE           (1U << 1)    ///< Update layer Frame buffer
#define CDC200_FRAMEBUF_UPDATE_VSYNC     (1U << 2)    ///< Update layer Frame buffer on vertical blanking
#define CDC200_SCANLINE0_EVENT           (1U << 3)    ///< Enable/Disable Scanline0 event
#define CDC200_CONFIGURE_LAYER           (1U << 4)    ///< Configure Layer
#define CDC200_LAYER_ON                  (1U << 5)    ///< Turn On the Layer
#define CDC200_LAYER_OFF                 (1U << 6)    ///< Turn Off the Layer
#define CDC200_CONFIGURE_LAYER_WINDOW    (1U << 7)    ///< Configure Layer window
#define CDC200_CONFIGURE_BG_COLOR        (1U << 8)    ///< Configure Background color
#define CDC200_CONFIGURE_LAYER_BLENDING  (1U << 9)    ///< Configure Layer blending


/**
\brief CDC200 Layer index
*/
typedef enum _ARM_CDC200_LAYER_INDEX {
  ARM_CDC200_LAYER_1,                 ///< CDC200 Layer 1
  ARM_CDC200_LAYER_2                  ///< CDC200 Layer 2
} ARM_CDC200_LAYER_INDEX;

/**
\brief CDC200 Layer Pixel format
*/
typedef enum _ARM_CDC200_LAYER_PIXEL_FORMAT {
  ARM_CDC200_ARGB8888,                       ///< 32-bit ARGB
  ARM_CDC200_RGB888,                         ///< 24-bit RGB (A = 255)
  ARM_CDC200_RGB565,                         ///< 16-bit RGB (A = 255)
  ARM_CDC200_RGBA8888,                       ///< 32-bit RGBA
  ARM_CDC200_AL44,                           ///< 8-bit alpha + luminance (lower channel on R, G and B)
  ARM_CDC200_AL8,                            ///< 8-bit single channel (value on A, R, G and B)
  ARM_CDC200_ARGB1555,                       ///< 16-bit ARGB with 1 bit alpha
  ARM_CDC200_ARGB4444                        ///< 16-bit ARGB with 4 bits alpha
} ARM_CDC200_LAYER_PIXEL_FORMAT;

/**
\brief CDC200 Selection of layer blending factor
*/
typedef enum _CDC200_BLEND_FACTOR
{
    CDC200_BLEND_CONST_ALPHA,                ///< layer blending factor selected to constant alpha
    CDC200_BLEND_PIXEL_ALPHA_X_CONST_ALPHA   ///< layer blending factor selected to pixel alpha * constant alpha
} CDC200_BLEND_FACTOR;

/**
\brief CDC200 Layer window information
*/
typedef struct _ARM_CDC200_LAYER_WINDOW_INFO {
  uint16_t v_start_pos;                       ///< Vertical Start Position
  uint16_t v_stop_pos;                        ///< Vertical Stop Position
  uint16_t h_start_pos;                       ///< Horizontal Start Position
  uint16_t h_stop_pos;                        ///< Horizontal Stop Position
} ARM_CDC200_LAYER_WINDOW_INFO;

/**
\brief CDC Layer information
*/
typedef struct _ARM_CDC200_LAYER_INFO {
  ARM_CDC200_LAYER_INDEX         layer_idx;              ///< CDC200 Layer index
  ARM_CDC200_LAYER_WINDOW_INFO   win_info;               ///< CDC200 window information
  ARM_CDC200_LAYER_PIXEL_FORMAT  pix_format;             ///< CDC200 Layer pixel format
  uint8_t                        const_alpha;            ///< CDC200 Layer constant alpha
  CDC200_BLEND_FACTOR            blend_factor;           ///< CDC200 Selection of Layer blending factor
  uint32_t                       fb_addr;                ///< CDC200 Layer FB address
  uint16_t                       line_length_in_pixels;  ///< CDC200 Layer Line length in pixels of color FB
  uint16_t                       num_lines;              ///< CDC200 Layer number of lines in the color FB
} ARM_CDC200_LAYER_INFO;

/****** CDC200 events *****/
#define ARM_CDC_DSI_ERROR_EVENT      (1U << 0)    ///< DSI error event
#define ARM_CDC_SCANLINE0_EVENT      (1U << 1)    ///< Scanline0 irq event

// Function documentation
/**
  \fn          ARM_DRIVER_VERSION ARM_CDC200_GetVersion (void)
  \brief       Get CDC200 driver version.
  \return      \ref ARM_DRIVER_VERSION.

  \fn          ARM_CDC200_CAPABILITIES ARM_CDC200_GetCapabilities (void)
  \brief       Get CDC200 driver capabilities.
  \return      \ref ARM_CDC200_CAPABILITIES.

  \fn          int32_t ARM_CDC200_Initialize (ARM_CDC200_SignalEvent_t cb_event)
  \brief       Initialize CDC200 Interface.
  \param[in]   cb_event Pointer to ARM_CDC200_SignalEvent_t.
  \return      \ref execution_status.

  \fn          int32_t ARM_CDC200_Uninitialize (void)
  \brief       Uninitialize CDC200 Interface.
  \return      \ref execution_status.

  \fn          int32_t ARM_CDC200_PowerControl (ARM_POWER_STATE state)
  \brief       Control CDC200 Interface Power.
  \param[in]   state  Power state.
  \return      \ref execution_status.

  \fn          int32_t ARM_CDC200_Control (uint32_t control, uint32_t arg)
  \brief       Control the display controller.
  \param[in]   control CDC200 contol code operation.
                 - \ref CDC200_CONFIGURE_DISPLAY :         Configure Display
                 - \ref CDC200_FRAMEBUF_UPDATE :           Update layer Frame buffer
                 - \ref CDC200_FRAMEBUF_UPDATE_VSYNC :     Update layer Frame buffer on vertical blanking
                 - \ref CDC200_SCANLINE0_EVENT :           Enable/Disable Scanline0 event
                 - \ref CDC200_CONFIGURE_LAYER :           Configure Layer
                 - \ref CDC200_LAYER_ON :                  Turn On the Layer
                 - \ref CDC200_LAYER_OFF :                 Turn Off the Layer
                 - \ref CDC200_CONFIGURE_LAYER_WINDOW :    Configure Layer window
                 - \ref CDC200_CONFIGURE_BG_COLOR :        Configure Background color
                 - \ref CDC200_CONFIGURE_LAYER_BLENDING :  Configure Layer blending
  \param[in]   arg Argument of operation.
                - CDC200_CONFIGURE_DISPLAY :         Frame buffer address
                - CDC200_FRAMEBUF_UPDATE :           Frame buffer address
                - CDC200_FRAMEBUF_UPDATE_VSYNC :     Frame buffer address
                - CDC200_SCANLINE0_EVENT :           ENABLE/DISABLE
                - CDC200_CONFIGURE_LAYER :           Pointer to layer info \ref ARM_CDC200_LAYER_INFO
                - CDC200_LAYER_ON :                  layer index /ref ARM_CDC200_LAYER_INDEX
                - CDC200_LAYER_OFF :                 layer index /ref ARM_CDC200_LAYER_INDEX
                - CDC200_CONFIGURE_LAYER_WINDOW :    Pointer to layer info \ref ARM_CDC200_LAYER_INFO
                - CDC200_CONFIGURE_BG_COLOR :        Background color setting
                                                       - /ref ARM_CDC200_BGC_BLUE(x)
                                                       - /ref ARM_CDC200_BGC_GREEN(x)
                                                       - /ref ARM_CDC200_BGC_RED(x)
                - CDC200_CONFIGURE_LAYER_BLENDING :  Pointer to layer info \ref ARM_CDC200_LAYER_INFO
  \return      \ref execution_status.

  \fn          int32_t ARM_CDC200_GetVerticalPosition (void)
  \brief       Get current vertical position count.
  \return      return current vertical position.

  \fn          int32_t ARM_CDC200_StartDisplay (void)
  \brief       Start the display controller.
  \return      \ref execution_status.

  \fn          int32_t ARM_CDC200_StopDisplay (void)
  \brief       Stop the display controller.
  \return      \ref execution_status.

  \fn          void ARM_CDC200_SignalEvent (uint32_t int_event)
  \brief       Signal CDC200 Events.
  \param[in]   int_event  \ref CDC200 event types.
  \return      none.
*/

/**
\brief CDC200 signal event.
*/
typedef void (*ARM_CDC200_SignalEvent_t) (uint32_t  int_event); ///< Pointer to \ref ARM_CDC200_SignalEvent : Signal CDC200 Event.

/**
\brief CDC200 driver capabilities.
*/
typedef struct _ARM_CDC200_CAPABILITIES {
  uint32_t reentrant_operation         :1;    ///< Support for reentrant calls
  uint32_t dpi_interface               :1;    ///< Support video mode Interface
  uint32_t reserved                    :30;   ///< Reserved (must be zero)
} ARM_CDC200_CAPABILITIES;

/**
\brief Access structure of the CDC200 Driver.
*/
typedef struct _ARM_DRIVER_CDC200 {
  ARM_DRIVER_VERSION           (*GetVersion)          (void);                                     ///< Pointer to \ref ARM_CDC200_GetVersion : Get driver version.
  ARM_CDC200_CAPABILITIES      (*GetCapabilities)     (void);                                     ///< Pointer to \ref ARM_CDC200_GetCapabilities : Get CDC200 driver capabilities.
  int32_t                      (*Initialize)          (ARM_CDC200_SignalEvent_t cb_event);        ///< Pointer to \ref ARM_CDC200_Initialize : Initialize CDC200 Interface.
  int32_t                      (*Uninitialize)        (void);                                     ///< Pointer to \ref ARM_CDC200_Uninitialize : Uninitialize CDC200 Interface.
  int32_t                      (*PowerControl)        (ARM_POWER_STATE state);                    ///< Pointer to \ref ARM_CDC200_PowerControl : Control CDC200 Interface Power.
  int32_t                      (*Control)             (uint32_t control, uint32_t arg);           ///< Pointer to \ref ARM_CDC200_Control:  Control CDC200 Interface.
  int32_t                      (*GetVerticalPosition) (void);                                     ///< Pointer to \ref ARM_CDC200_GetVerticalPosition: Get current vertical position count.
  int32_t                      (*Start)               (void);                                     ///< Pointer to \ref ARM_CDC200_StartDisplay : Configure CDC200 to start Displaying.
  int32_t                      (*Stop)                (void);                                     ///< Pointer to \ref ARM_CDC200_StopDisplay : Configure CDC200 to stop Displaying.
} ARM_DRIVER_CDC200;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_CDC200_H_ */

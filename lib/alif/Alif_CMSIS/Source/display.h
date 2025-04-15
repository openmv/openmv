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
 * @file     display.h
 * @author   Girish BN and Prasanna Ravi and Chandra Bhushan Singh
 * @email    girish.bn@alifsemi.com and chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     28-Sep-2023
 * @brief    display driver header.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#ifdef  __cplusplus
extern "C"
{
#endif

#include "dsi.h"
#include "cdc.h"

#define DISPLAY_PANEL(panel) \
DISPLAY_PANEL_DEVICE *Get_Display_Panel(void) \
{ \
    return &panel; \
} \

/* Display Panel Control Codes */
#define DISPALY_PANEL_CONFIG  (0x01UL)      /* Display Panel configure */

/**
  \brief CDC Information structure
  */
typedef struct _CDC_INFO
{
    CDC_POLARITY        hsync_polarity;     /* CDC hsync polarity */
    CDC_POLARITY        vsync_polarity;     /* CDC vsync polarity */
    CDC_PIXCLK_POLARITY pclk_polarity;      /* CDC pixel clock output polarity */
    CDC_POLARITY        blank_polarity;     /* CDC blank polarity */
} CDC_INFO;

/**
  \ DSI information structure
  */
typedef struct _DSI_INFO
{
    uint32_t            max_bitrate;        /* Maximum bitrate Panel supports */
    uint8_t             n_lanes;            /* Number of lanes */
    DSI_VC_ID           vc_id;              /* Virtual channel ID */
    DSI_COLOR_CODING    color_coding;       /* color coding */
} DSI_INFO;

/**
  \brief Display Panel Device Operations.
  */
typedef struct _DISPLAY_PANEL_OPERATIONS
{
    int32_t (*Init)    (void);              /* Initialize Display Panel device. */
    int32_t (*Uninit)  (void);              /* De-initialize Display Panel device. */
    int32_t (*Start)   (void);              /* Start Display Panel device. */
    int32_t (*Stop)    (void);              /* Stop Display Panel device. */
    int32_t (*Control) (uint32_t control);  /* Control Display Panel device. */
} DISPLAY_PANEL_OPERATIONS;

/**
  \brief display panel device structure.
  */
typedef struct _DISPLAY_PANEL_DEVICE
{
    uint32_t                 hsync_time;    /* Panel hsync time */
    uint32_t                 hbp_time;      /* Panel hbp time */
    uint32_t                 hfp_time;      /* Panel hfp time */
    uint32_t                 hactive_time;  /* Panel hact time */
    uint32_t                 vsync_line;    /* Panel vsync line */
    uint32_t                 vbp_line;      /* Panel vbp line */
    uint32_t                 vfp_line;      /* Panel vfp line */
    uint32_t                 vactive_line;  /* Panel vact line */
    CDC_INFO                 *cdc_info;     /* CDC information */
    DSI_INFO                 *dsi_info;     /* DSI information */
    DISPLAY_PANEL_OPERATIONS *ops;          /* Display Panel operations. */
} DISPLAY_PANEL_DEVICE;

/** Get Display Panel information */
DISPLAY_PANEL_DEVICE *Get_Display_Panel(void);

#ifdef  __cplusplus
}
#endif

#endif /* __DISPLAY_H__ */

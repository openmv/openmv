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
 * @file     Driver_DSI_Private.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     17-April-2023
 * @brief    DSI driver Specific Header file.
 ******************************************************************************/

#ifndef DRIVER_DSI_PRIVATE_H_
#define DRIVER_DSI_PRIVATE_H_

#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_MIPI_DSI.h"

#include "dsi.h"
#include "cdc.h"

#ifdef  __cplusplus
extern "C"
{
#endif

/*Helper macro*/
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/** \brief DSI DPHY high speed transition timings */
typedef struct _DSI_DPHY_HS_TRANSITION_TIMINGS_RANGE {
    uint16_t bitrate_mbps;                             /**< DPHY data rate in mbps */
    uint16_t clklp2hs_time;                            /**< DPHY clock lane LP to HS transition time */
    uint16_t clkhs2lp_time;                            /**< DPHY clock lane HS to LP transition time */
    uint16_t lp2hs_time;                               /**< DPHY data lane LP to HS transition time */
    uint16_t hs2lp_time;                               /**< DPHY data lane HS to LP transition time */
} DSI_DPHY_HS_TRANSITION_TIMINGS_RANGE;

/** \brief DSI Driver states. */
typedef volatile struct _DSI_DRIVER_STATE {
    uint32_t initialized       : 1;                    /**< Driver Initialized */
    uint32_t powered           : 1;                    /**< Driver powered */
    uint32_t host_configured   : 1;                    /**< Driver host configured */
    uint32_t dpi_configured    : 1;                    /**< Driver DPI configured */
    uint32_t panel_initialized : 1;                    /**< Driver panel initialized */
    uint32_t reserved          : 27;                   /**< Reserved */
} DSI_DRIVER_STATE;

/** \brief DSI DPI Info */
typedef struct _DSI_DPI_INFO{
    DSI_VIDEO_MODE   vid_mode;                         /**< video mode */
    uint32_t         vid_pkt_size;                     /**< video packet size */
    uint32_t         vid_num_chunks;                   /**< video number of chunks */
    uint32_t         vid_null_size;                    /**< video null packet size */
}DSI_DPI_INFO;

/**
  \brief MIPI DSI driver info
  */
typedef struct _DSI_RESOURCES{
    DSI_Type                        *reg_base;         /**< Pointer to regs */
    ARM_MIPI_DSI_SignalEvent_t      cb_event;          /**< Pointer to call back function */
    uint32_t                        frequency;         /**< DSI PHY Frequency */
    uint32_t                        tx_ecs_clk_div;    /**< Tx escape clock divider value */
    uint32_t                        horizontal_timing; /**< Total horizontal timing */
    uint32_t                        vertical_timing;   /**< Total vertical timing */
    DSI_DPI_INFO                    *dpi_info;         /**< Pointer to DPI info */
    IRQn_Type                       irq;               /**< Interrupt number */
    uint32_t                        irq_priority;      /**< Interrupt priority */
    DSI_DRIVER_STATE                state;             /**< DSI driver status */
}DSI_RESOURCES;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_DSI_PRIVATE_H_ */

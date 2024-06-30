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
 * @file     Driver_CSI_Private.h
 * @author   Chandra Bhushan Singh
 * @email    chandrabhushan.singh@alifsemi.com
 * @version  V1.0.0
 * @date     12-April-2023
 * @brief    CMSIS Driver private header file.
 ******************************************************************************/

#ifndef DRIVER_CSI_PRIVATE_H_

#define DRIVER_CSI_PRIVATE_H_

#ifdef  __cplusplus
extern "C"
{
#endif

/* System includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

/* CSI includes */
#include "Driver_MIPI_CSI2.h"
#include "csi.h"
#include "cpi.h"

/*Helper macro*/
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define MAX(a, b, c) ((a) <= (b) ? ((b) <= (c) ? (c) : (b)) : ((a) <= (c) ? (c) : (a)))

/** \brief CSI IPI frame info */
typedef struct _CSI_FRAME_INFO
{
    uint32_t                         hsa_time;           /**< CSI IPI Horizontal Synchronism Active Period        */
    uint32_t                         hbp_time;           /**< CSI IPI Horizontal Back Porch Period                */
    uint32_t                         hsd_time;           /**< CSI IPI Horizontal Sync porch delay Period          */
    uint32_t                         hactive_time;       /**< CSI IPI Line Time Count Size                        */
    uint32_t                         vsa_line;           /**< CSI IPI Vertical Synchronism Active period          */
    uint32_t                         vbp_line;           /**< CSI IPI Vertical Back Porch period                  */
    uint32_t                         vfp_line;           /**< CSI IPI Vertical Front Porch period                 */
    uint32_t                         vactive_line;       /**< CSI IPI Vertical Active period                      */
} CSI_FRAME_INFO;

/** \brief CSI IPI advanced features */
typedef struct _CSI_IPI_ADV_INFO
{
    uint8_t                          sync_evnt_mode;     /**< CSI IPI sync event type                             */
    uint8_t                          event_sel;          /**< CSI IPI line event select                           */
    uint8_t                          en_embedded;        /**< CSI IPI embedded packet                             */
    uint8_t                          en_blanking;        /**< CSI IPI blank packet                                */
    uint8_t                          en_null;            /**< CSI IPI null packet                                 */
    uint8_t                          en_line_start;      /**< CSI IPI line start packet                           */
    uint8_t                          en_video;           /**< CSI IPI video packet                                */
    uint8_t                          ipi_dt;             /**< CSI IPI data to be overwrite                        */
    uint8_t                          ipi_dt_overwrite;   /**< CSI IPI data overwrite                              */
} CSI_IPI_ADV_INFO;

/** \brief CSI IPI info */
typedef struct _CSI_IPI_INFO
{
    uint32_t                         ipi_memflush;       /**< CSI IPI memory flush                                */
    uint8_t                          ipi_mode;           /**< CSI IPI mode                                        */
    uint8_t                          ipi_color_com;      /**< CSI IPI color component                             */
    CSI_FRAME_INFO                   *frame_info;        /**< CSI frame information                               */
    CSI_IPI_ADV_INFO                 *adv_features;      /**< CSI IPI advanced features                           */
} CSI_IPI_INFO;

/** \brief CSI State */
typedef struct _CSI_DRIVER_STATE
{
    uint32_t initialized    : 1;                         /**< CSI Driver Initialized                              */
    uint32_t powered        : 1;                         /**< CSI Driver powered                                  */
    uint32_t csi_configured : 1;                         /**< CSI configuration(host and IPI)                     */
    uint32_t reserved       : 29;                        /**< Reserved                                            */
} CSI_DRIVER_STATE;

/** \brief CSI CPI related data settings */
typedef struct _CSI_CPI_DATA_MODE_SETTINGS
{
    CSI_DATA_TYPE          data_type;                    /**< CSI data type                                       */
    CSI_IPI_COLOR_COM_TYPE ipi_color_com;                /**< CSI IPI Color component                             */
    CPI_COLOR_MODE_CONFIG  cpi_color_mode;               /**< CPI CSI color mode                                  */
    CPI_DATA_MODE          cpi_data_mode;                /**< CPI Data mode                                       */
    uint32_t               bpp;                          /**< bits per pixel                                      */
} CSI_CPI_DATA_MODE_SETTINGS;

/** \brief CSI driver resources */
typedef struct _CSI_RESOURCES
{
    CSI_Type                         *regs;              /**< CSI Register Base Address                           */
    ARM_MIPI_CSI2_SignalEvent_t      cb_event;           /**< CSI Application Event Callback                      */
    CSI_DRIVER_STATE                 status;             /**< CSI Status                                          */
    CSI_IPI_INFO                     *ipi_info;          /**< CSI IPI information                                 */
    IRQn_Type                        irq;                /**< CSI Interrupt Vector Number                         */
    uint8_t                          irq_priority;       /**< CSI Interrupt Priority                              */
    uint8_t                          pixel_data_type;    /**< CSI IPI pixel data type                             */
    uint8_t                          csi_pixclk_div;     /**< CSI clock divisor                                   */
    uint8_t                          n_lanes;            /**< CSI number of lanes select                          */
    uint8_t                          vc_id;              /**< CSI virtual channel ID                              */
} CSI_RESOURCES;

#endif /* DRIVER_CSI_PRIVATE_H_ */

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
 * @file     Camera_Sensor.h
 * @author   Tanay Rami and Chandra Bhushan Singh
 * @email    tanay@alifsemi.com and chandrabhushan.singh@alifsemi.com
 * @version  V1.1.0
 *             -Removed enums for clock source, interface, polarity, hsync mode,
 *             data mode, and data mask.
 *             -Included low level header file 'cpi.h'
 *             -Replaced data types in CAMERA_SENSOR_INFO structure with CPI low
 *             level file data types.
 * @date     19-April-2023
 * @brief    Camera Sensor Device definitions.
 ******************************************************************************/

#ifndef CAMERA_SENSOR_H_
#define CAMERA_SENSOR_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "cpi.h"
#include "csi.h"

/****** CAMERA_SENSOR used for registering camera sensor *****/
#define CAMERA_SENSOR(sensor) \
CAMERA_SENSOR_DEVICE *Get_Camera_Sensor(void) \
{ \
    return &sensor; \
} \

/****** LPCAMERA_SENSOR used for registering low power camera sensor *****/
#define LPCAMERA_SENSOR(sensor) \
CAMERA_SENSOR_DEVICE *Get_LPCamera_Sensor(void) \
{ \
    return &sensor; \
} \

/**
\brief Camera Sensor interface
*/
typedef enum _CAMERA_SENSOR_INTERFACE {
    CAMERA_SENSOR_INTERFACE_PARALLEL,          /* Camera sensor parallel interface */
    CAMERA_SENSOR_INTERFACE_MIPI               /* Camera sensor serial interface */
} CAMERA_SENSOR_INTERFACE;

/**
\brief CPI information structure
*/
typedef struct _CPI_INFO {
    CPI_WAIT_VSYNC           vsync_wait;       /* CPI VSYNC Wait */
    CPI_CAPTURE_DATA_ENABLE  vsync_mode;       /* CPI VSYNC Mode */
    CPI_SIG_POLARITY         pixelclk_pol;     /* CPI Pixel Clock Polarity */
    CPI_SIG_POLARITY         hsync_pol;        /* CPI HSYNC Polarity */
    CPI_SIG_POLARITY         vsync_pol;        /* CPI VSYNC Polarity */
    CPI_DATA_MODE            data_mode;        /* CPI Data Mode */
    CPI_DATA_ENDIANNESS      data_endianness;  /* CPI MSB/LSB */
    CPI_CODE10ON8_CODING     code10on8;        /* CPI code10on8 enable/disable */
    CPI_DATA_MASK            data_mask;        /* CPI Data Mask */
    CPI_COLOR_MODE_CONFIG    csi_mode;         /* CPI CSI Color mode */
} CPI_INFO;

/**
\brief CSI override CPI color mode structure
*/
typedef struct _CSI_OVERRIDE_CPI_COLOR {
    bool                  override;        /* CPI color mode override by CPI */
    CPI_COLOR_MODE_CONFIG cpi_color_mode;  /* CPI color mode to be override */
} CSI_OVERRIDE_CPI_COLOR;

/**
\brief CSI Pkt2PktTime, Time between Packets (includes the duration of the LS
       Packet + PHY LowPower to High-Speed time + any eventual camera added delay
       + PHY HighSpeed to Low-Power time).
*/
typedef struct _CSI_PKT2PKT_TIME{
    bool                  line_sync_pkt_enable;  /* LS/LE Packets are enabled */
    float                 time_ns;               /* Time between Packets in ns */
}CSI_PKT2PKT_TIME;

/**
\brief CSI information structure
*/
typedef struct _CSI_INFO {
    uint32_t                frequency;                /* CSI clock frequency */
    CSI_DATA_TYPE           dt;                       /* CSI data type */
    uint8_t                 n_lanes;                  /* CSI number of data lanes */
    CSI_VC_ID               vc_id;                    /* CSI virtual channel ID */
    CSI_OVERRIDE_CPI_COLOR  cpi_cfg;                  /* CSI override CPI color mode */
    CSI_PKT2PKT_TIME        pkt2pkt_time;             /* CSI Time between Packets */
} CSI_INFO;

/**
\brief CAMERA Sensor Device Operations
*/
typedef struct _CAMERA_SENSOR_OPERATIONS {
    int32_t (*Init)    (void);                           /* Initialize Camera Sensor device */
    int32_t (*Uninit)  (void);                           /* De-initialize Camera Sensor device */
    int32_t (*Start)   (void);                           /* Start Camera Sensor device */
    int32_t (*Stop)    (void);                           /* Stop Camera Sensor device */
    int32_t (*Control) (uint32_t control, uint32_t arg); /* Control Camera Sensor device */
} CAMERA_SENSOR_OPERATIONS;

/**
\brief CAMERA Sensor Device
*/
typedef struct _CAMERA_SENSOR_DEVICE {
    CAMERA_SENSOR_INTERFACE   interface; /* Camera Sensor interface */
    int                       width;     /* Frame Width */
    int                       height;    /* Frame Height */
    CPI_INFO                  *cpi_info; /* CPI Camera Sensor device Information */
    CSI_INFO                  *csi_info; /* CSI Camera Sensor device Information */
    CAMERA_SENSOR_OPERATIONS  *ops;      /* Camera Sensor device Operations */
} CAMERA_SENSOR_DEVICE;

/** Get CPI/LPCPI sensor information */
CAMERA_SENSOR_DEVICE *Get_Camera_Sensor(void);
CAMERA_SENSOR_DEVICE *Get_LPCamera_Sensor(void);

#ifdef  __cplusplus
}
#endif

#endif /* CAMERA_SENSOR_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/

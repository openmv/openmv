/*******************************************************************************
**
**    File NAME: LEPTON_OEM.h
**
**      AUTHOR:  David Dart
**
**      CREATED: 8/6/2012
**
**      DESCRIPTION: COntains OEM Interfaces
**
**      HISTORY:  8/6/2012 DWD - Initial Draft
**
**      Copyright 2011,2012,2013,2014 FLIR Systems - Commercial
**      Vision Systems.  All rights reserved.
**
**      Proprietary - PROPRIETARY - FLIR Systems Inc..
**
**      This document is controlled to FLIR Technology Level 2.
**      The information contained in this document pertains to a
**      dual use product Controlled for export by the Export
**      Administration Regulations (EAR). Diversion contrary to
**      US law is prohibited.  US Department of Commerce
**      authorization is not required prior to export or
**      transfer to foreign persons or parties unless otherwise
**      prohibited.
**
**      Redistribution and use in source and binary forms, with
**      or without modification, are permitted provided that the
**      following conditions are met:
**
**      Redistributions of source code must retain the above
**      copyright notice, this list of conditions and the
**      following disclaimer.
**
**      Redistributions in binary form must reproduce the above
**      copyright notice, this list of conditions and the
**      following disclaimer in the documentation and/or other
**      materials provided with the distribution.
**
**      Neither the name of the FLIR Systems Corporation nor the
**      names of its contributors may be used to endorse or
**      promote products derived from this software without
**      specific prior written permission.
**
**      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
**      CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
**      WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**      WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
**      PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
**      COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
**      DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
**      CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
**      PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
**      USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
**      CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
**      CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
**      NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
**      USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
**      OF SUCH DAMAGE.
**
*******************************************************************************/
#ifndef _LEPTON_OEM_H_
   #define _LEPTON_OEM_H_

   #ifdef __cplusplus
extern "C"
{
   #endif

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
   #include "LEPTON_Types.h"
   #include "LEPTON_ErrorCodes.h"

/******************************************************************************/
/** EXPORTED DEFINES                                                         **/
/******************************************************************************/
/* OEM Module Command IDs
*/
   #define LEP_OEM_MODULE_BASE                     0x0800

   #define LEP_CID_OEM_POWER_DOWN                  (LEP_OEM_MODULE_BASE + 0x4000 )
   #define LEP_CID_OEM_STANDBY                     (LEP_OEM_MODULE_BASE + 0x4004 )
   #define LEP_CID_OEM_LOW_POWER_MODE_1            (LEP_OEM_MODULE_BASE + 0x4008 )
   #define LEP_CID_OEM_LOW_POWER_MODE_2            (LEP_OEM_MODULE_BASE + 0x400C )
   #define LEP_CID_OEM_BIT_TEST                    (LEP_OEM_MODULE_BASE + 0x4010 )
   #define LEP_CID_OEM_MASK_REVISION               (LEP_OEM_MODULE_BASE + 0x4014 )
//#define LEP_CID_OEM_MASTER_ID                   (LEP_OEM_MODULE_BASE + 0x4018 )
   #define LEP_CID_OEM_FLIR_PART_NUMBER            (LEP_OEM_MODULE_BASE + 0x401C )
   #define LEP_CID_OEM_SOFTWARE_VERSION            (LEP_OEM_MODULE_BASE + 0x4020 )
   #define LEP_CID_OEM_VIDEO_OUTPUT_ENABLE         (LEP_OEM_MODULE_BASE + 0x4024 )
   #define LEP_CID_OEM_VIDEO_OUTPUT_FORMAT         (LEP_OEM_MODULE_BASE + 0x4028 )
   #define LEP_CID_OEM_VIDEO_OUTPUT_SOURCE         (LEP_OEM_MODULE_BASE + 0x402C )
   #define LEP_CID_OEM_VIDEO_OUTPUT_CHANNEL        (LEP_OEM_MODULE_BASE + 0x4030 )
   #define LEP_CID_OEM_VIDEO_GAMMA_ENABLE          (LEP_OEM_MODULE_BASE + 0x4034 )
   #define LEP_CID_OEM_CUST_PART_NUMBER            (LEP_OEM_MODULE_BASE + 0x4038 )
   #define LEP_CID_OEM_VIDEO_OUTPUT_CONSTANT       (LEP_OEM_MODULE_BASE + 0x403C )
   #define LEP_CID_OEM_REBOOT                      (LEP_OEM_MODULE_BASE + 0x4040 )
   #define LEP_CID_OEM_FFC_NORMALIZATION_TARGET    (LEP_OEM_MODULE_BASE + 0x4044 )
   #define LEP_CID_OEM_STATUS                      (LEP_OEM_MODULE_BASE + 0x4048 )
   #define LEP_CID_OEM_SCENE_MEAN_VALUE            (LEP_OEM_MODULE_BASE + 0x404C )
   #define LEP_CID_OEM_POWER_MODE                  (LEP_OEM_MODULE_BASE + 0x4050 )

   #define LEP_CID_OEM_GPIO_MODE_SELECT            (LEP_OEM_MODULE_BASE + 0x4054 )
   #define LEP_CID_OEM_GPIO_VSYNC_PHASE_DELAY      (LEP_OEM_MODULE_BASE + 0x4058 )

   #define LEP_CID_OEM_USER_DEFAULTS               (LEP_OEM_MODULE_BASE + 0x405C )
   #define LEP_CID_OEM_USER_DEFAULTS_RESTORE       (LEP_OEM_MODULE_BASE + 0x4060 )
   #define LEP_CID_OEM_SHUTTER_PROFILE_OBJ         (LEP_OEM_MODULE_BASE + 0x4064 )
   #define LEP_CID_OEM_THERMAL_SHUTDOWN_ENABLE_STATE (LEP_OEM_MODULE_BASE + 0x4068 )
   #define LEP_CID_OEM_BAD_PIXEL_REPLACE_CONTROL   (LEP_OEM_MODULE_BASE + 0x406C )
   #define LEP_CID_OEM_TEMPORAL_FILTER_CONTROL     (LEP_OEM_MODULE_BASE + 0x4070 )
   #define LEP_CID_OEM_COLUMN_NOISE_ESTIMATE_CONTROL (LEP_OEM_MODULE_BASE + 0x4074 )
   #define LEP_CID_OEM_PIXEL_NOISE_ESTIMATE_CONTROL (LEP_OEM_MODULE_BASE + 0x4078 )



   #define LEP_OEM_MAX_PART_NUMBER_CHAR_SIZE       32

/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/

/* Chip Mask Revision: An (8 bit depth) identifier for the chip
** mask revision located in bits 7-0 of the 16-bit word passed.
*/
typedef LEP_UINT16  LEP_OEM_MASK_REVISION_T,*LEP_OEM_MASK_REVISION_T_PTR;
typedef LEP_UINT16  LEP_OEM_FFC_NORMALIZATION_TARGET_T, *LEP_OEM_FFC_NORMALIZATION_TARGET_T_PTR;
typedef LEP_UINT16  LEP_OEM_FRAME_AVERAGE_T, *LEP_OEM_FRAME_AVERAGE_T_PTR;

//typedef LEP_UINT16  LEP_OEM_VENDORID_T;

/* Part Number: A (32 byte string) identifier unique to a
** specific configuration of module; essentially a module
** Configuration ID.
*/
   #if USE_DEPRECATED_PART_NUMBER_INTERFACE
typedef LEP_CHAR8 *LEP_OEM_PART_NUMBER_T, *LEP_OEM_PART_NUMBER_T_PTR;
   #else
typedef struct LEP_OEM_PART_NUMBER_T_TAG
{
   LEP_CHAR8 value[LEP_OEM_MAX_PART_NUMBER_CHAR_SIZE];
} LEP_OEM_PART_NUMBER_T, *LEP_OEM_PART_NUMBER_T_PTR;
   #endif
   #if 0
typedef struct
{
   LEP_CHAR8   value[32];    /* 32-byte string */

}
LEP_OEM_PART_NUMBER_T, *LEP_OEM_PART_NUMBER_T_PTR;
   #endif


/* Software Version ID: A (24 bit depth) identifier for
** the software version stored in OTP.
*/
typedef struct LEP_OEM_SW_VERSION_TAG
{
   LEP_UINT8   gpp_major;
   LEP_UINT8   gpp_minor;
   LEP_UINT8   gpp_build;
   LEP_UINT8   dsp_major;
   LEP_UINT8   dsp_minor;
   LEP_UINT8   dsp_build;
   LEP_UINT16  reserved;

}LEP_OEM_SW_VERSION_T, *LEP_OEM_SW_VERSION_T_PTR;

   #if 0
/* Chip Master ID: A (16 byte depth) identifier for the ISC1103
** signal processing ASIC.
*/
typedef struct LEP_OEM_MASTER_ID_T_TAG
{
   LEP_UINT8  value[16];        /* 16 byte value */

}
LEP_OEM_MASTER_ID_T, *LEP_OEM_MASTER_ID_T_PTR;
   #endif
/* Video Output Enable Enum
*/
typedef enum LEP_OEM_VIDEO_OUTPUT_ENABLE_TAG
{
   LEP_VIDEO_OUTPUT_DISABLE = 0,
   LEP_VIDEO_OUTPUT_ENABLE,
   LEP_END_VIDEO_OUTPUT_ENABLE,

   LEP_VIDEO_OUTPUT_ENABLE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_OEM_VIDEO_OUTPUT_ENABLE_E, *LEP_OEM_VIDEO_OUTPUT_ENABLE_E_PTR;

/* Video Output Format Selection
*/
typedef enum LEP_OEM_VIDEO_OUTPUT_FORMAT_TAG
{
   LEP_VIDEO_OUTPUT_FORMAT_RAW8 = 0,          // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW10,             // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW12,             // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RGB888,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RGB666,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RGB565,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_YUV422_8BIT,       // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW14,             // SUPPORTED in this release
   LEP_VIDEO_OUTPUT_FORMAT_YUV422_10BIT,      // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_USER_DEFINED,      // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW8_2,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW8_3,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW8_4,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW8_5,            // To be supported in later release
   LEP_VIDEO_OUTPUT_FORMAT_RAW8_6,            // To be supported in later release
   LEP_END_VIDEO_OUTPUT_FORMAT,

   LEP_VIDEO_OUTPUT_FORMAT_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_OEM_VIDEO_OUTPUT_FORMAT_E, *LEP_OEM_VIDEO_OUTPUT_FORMAT_E_PTR;

/* Video Output Source Selection
*/
typedef enum LEP_OEM_VIDEO_OUTPUT_SOURCE_TAG
{
   LEP_VIDEO_OUTPUT_SOURCE_RAW = 0,         /* Before video processing */
   LEP_VIDEO_OUTPUT_SOURCE_COOKED,        /* Post video processing - NORMAL MODE */
   LEP_VIDEO_OUTPUT_SOURCE_RAMP,          /* Software Ramp pattern - increase in X, Y */
   LEP_VIDEO_OUTPUT_SOURCE_CONSTANT,      /* Software Constant value pattern */
   LEP_VIDEO_OUTPUT_SOURCE_RAMP_H,        /* Software Ramp pattern - increase in X only */
   LEP_VIDEO_OUTPUT_SOURCE_RAMP_V,        /* Software Ramp pattern - increase in Y only */
   LEP_VIDEO_OUTPUT_SOURCE_RAMP_CUSTOM,   /* Software Ramp pattern - uses custom settings */

   /* Additions to support frame averaging, freeze frame, and data buffers
   */
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_CAPTURE,  // Average, Capture frame
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_FREEZE,   // Freeze-Frame Buffer

   /* RESERVED BUFFERS
   */
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_0,        // Reserved DATA Buffer
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_1,        // Reserved DATA Buffer
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_2,        // Reserved DATA Buffer
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_3,        // Reserved DATA Buffer
   LEP_VIDEO_OUTPUT_SOURCE_FRAME_4,        // Reserved DATA Buffer

   LEP_END_VIDEO_OUTPUT_SOURCE,

   LEP_VIDEO_OUTPUT_SOURCE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_OEM_VIDEO_OUTPUT_SOURCE_E, *LEP_OEM_VIDEO_OUTPUT_SOURCE_E_PTR;

/* Video Output Channel Selection
*/
typedef enum LEP_OEM_VIDEO_OUTPUT_CHANNEL_TAG
{
   LEP_VIDEO_OUTPUT_CHANNEL_MIPI = 0,
   LEP_VIDEO_OUTPUT_CHANNEL_VOSPI,
   LEP_END_VIDEO_OUTPUT_CHANNEL,

   LEP_VIDEO_OUTPUT_CHANNEL_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_OEM_VIDEO_OUTPUT_CHANNEL_E, *LEP_OEM_VIDEO_OUTPUT_CHANNEL_E_PTR;

/* Video Gamma Enable Enum
*/
typedef enum LEP_OEM_VIDEO_GAMMA_ENABLE_TAG
{
   LEP_VIDEO_GAMMA_DISABLE = 0,
   LEP_VIDEO_GAMMA_ENABLE,
   LEP_END_VIDEO_GAMMA_ENABLE,

   LEP_VIDEO_GAMMA_ENABLE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_OEM_VIDEO_GAMMA_ENABLE_E, *LEP_OEM_VIDEO_GAMMA_ENABLE_E_PTR;

typedef enum LEP_OEM_MEM_BUFFER_E_TAG
{
   LEP_OEM_MEM_OTP_ODAC = 0,
   LEP_OEM_MEM_OTP_GAIN,
   LEP_OEM_MEM_OTP_OFFSET_0,
   LEP_OEM_MEM_OTP_OFFSET_1,
   LEP_OEM_MEM_OTP_FFC,
   LEP_OEM_MEM_OTP_LG0,
   LEP_OEM_MEM_OTP_LG1,
   LEP_OEM_MEM_OTP_LG2,
   LEP_OEM_MEM_OTP_TFPA_LUT,
   LEP_OEM_MEM_OTP_TAUX_LUT,
   LEP_OEM_MEM_OTP_BAD_PIXEL_LIST,
   LEP_OEM_MEM_SRAM_ODAC,
   LEP_OEM_MEM_SRAM_BAD_PIXEL_LIST,
   LEP_OEM_MEM_SHARED_BUFFER_0,
   LEP_OEM_MEM_SHARED_BUFFER_1,
   LEP_OEM_MEM_SHARED_BUFFER_2,
   LEP_OEM_MEM_SHARED_BUFFER_3,
   LEP_OEM_MEM_SHARED_BUFFER_4,
   LEP_OEM_END_MEM_BUFFERS,

   LEP_OEM_MEM_BUFFERS_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_OEM_MEM_BUFFER_E,*LEP_OEM_MEM_BUFFER_E_PTR;

typedef enum
{
   LEP_OEM_STATUS_OTP_WRITE_ERROR = -2,
   LEP_OEM_STATUS_ERROR = -1,
   LEP_OEM_STATUS_READY = 0,
   LEP_OEM_STATUS_BUSY,
   LEP_OEM_FRAME_AVERAGE_COLLECTING_FRAMES,
   LEP_OEM_STATUS_END,

   LEP_OEM_STATUS_MAKE_32_BIT_ENUM = 0x7FFFFFFF
} LEP_OEM_STATUS_E, *LEP_OEM_STATUS_E_PTR;

typedef enum LEP_OEM_STATE_E_TAG
{
   LEP_OEM_DISABLE = 0,
   LEP_OEM_ENABLE,
   LEP_OEM_END_STATE,

   LEP_OEM_STATE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_OEM_STATE_E,*LEP_OEM_STATE_E_PTR;

typedef enum LEP_OEM_POWER_STATE_E_TAG
{
   LEP_OEM_POWER_MODE_NORMAL = 0,
   LEP_OEM_POWER_MODE_LOW_POWER_1,
   LEP_OEM_POWER_MODE_LOW_POWER_2,
   LEP_OEM_END_POWER_MODE,

   LEP_OEM_POWER_MODE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_OEM_POWER_STATE_E, *LEP_OEM_POWER_STATE_E_PTR;

typedef enum LEP_OEM_VSYNC_DELAY_E_TAG
{
   LEP_OEM_VSYNC_DELAY_MINUS_3 = -3,
   LEP_OEM_VSYNC_DELAY_MINUS_2 = -2,
   LEP_OEM_VSYNC_DELAY_MINUS_1 = -1,
   LEP_OEM_VSYNC_DELAY_NONE = 0,
   LEP_OEM_VSYNC_DELAY_PLUS_1 = 1,
   LEP_OEM_VSYNC_DELAY_PLUS_2 = 2,
   LEP_OEM_VSYNC_DELAY_PLUS_3 = 3,

   LEP_END_OEM_VSYNC_DELAY,
   
   LEP_OEM_VSYNC_DELAY_MAKE_32_BIT_ENUM = 0x7FFFFFFF
} LEP_OEM_VSYNC_DELAY_E, *LEP_OEM_VSYNC_DELAY_E_PTR;

typedef enum LEP_OEM_GPIO_MODE_E_TAG
{
   LEP_OEM_GPIO_MODE_GPIO = 0,
   LEP_OEM_GPIO_MODE_I2C_MASTER = 1,
   LEP_OEM_GPIO_MODE_SPI_MASTER_VLB_DATA = 2,
   LEP_OEM_GPIO_MODE_SPIO_MASTER_REG_DATA = 3,
   LEP_OEM_GPIO_MODE_SPI_SLAVE_VLB_DATA = 4,
   LEP_OEM_GPIO_MODE_VSYNC = 5,

   LEP_OEM_END_GPIO_MODE,
   
   LEP_OEM_GPIO_MODE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_OEM_GPIO_MODE_E, *LEP_OEM_GPIO_MODE_E_PTR;

typedef enum LEP_OEM_USER_PARAMS_STATE_E_TAG
{
   LEP_OEM_USER_PARAMS_STATE_NOT_WRITTEN = 0,
   LEP_OEM_USER_PARAMS_STATE_WRITTEN,

   LEP_OEM_END_USER_PARAMS_STATE,

   LEP_OEM_USER_PARAMS_STATE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_OEM_USER_PARAMS_STATE_E, *LEP_OEM_USER_PARAMS_STATE_E_PTR;




/* Shutter Profile Object
*/
typedef struct LEP_OEM_SHUTTER_PROFILE_OBJ_T_TAG
{
   LEP_UINT16 closePeriodInFrames;           /* in frame counts x1 */
   LEP_UINT16 openPeriodInFrames;            /* in frame counts x1 */

}LEP_OEM_SHUTTER_PROFILE_OBJ_T, *LEP_OEM_SHUTTER_PROFILE_OBJ_T_PTR;

typedef struct LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T_TAG
{
   LEP_OEM_STATE_E oemBadPixelReplaceEnable;          // Bad Pixel Replacment in the video path

}LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T, *LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T_PTR;
typedef struct LEP_OEM_TEMPORAL_FILTER_CONTROL_T_TAG
{
   LEP_OEM_STATE_E oemTemporalFilterEnable;           // Temporal Filter in the video path

}LEP_OEM_TEMPORAL_FILTER_CONTROL_T, *LEP_OEM_TEMPORAL_FILTER_CONTROL_T_PTR;

typedef struct LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T_TAG
{
   LEP_OEM_STATE_E oemColumnNoiseEstimateEnable;      // Column Noise Estimate in the video path

}LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T, *LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T_PTR;

typedef struct LEP_OEM_PIXEL_NOISE_SETTINGS_T_TAG
{
   LEP_OEM_STATE_E oemPixelNoiseEstimateEnable;         // Row Noise Estimate in the video path

}LEP_OEM_PIXEL_NOISE_SETTINGS_T, *LEP_OEM_PIXEL_NOISE_SETTINGS_T_PTR;

typedef struct LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T_TAG
{
   LEP_OEM_STATE_E oemThermalShutdownEnable;

}LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T, *LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T_PTR;

/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/
extern LEP_RESULT LEP_RunOemPowerDown( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
extern LEP_RESULT LEP_RunOemPowerOn( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
extern LEP_RESULT LEP_RunOemStandby( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
extern LEP_RESULT LEP_RunOemReboot( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
extern LEP_RESULT LEP_RunOemLowPowerMode1( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
extern LEP_RESULT LEP_RunOemLowPowerMode2( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
extern LEP_RESULT LEP_RunOemBit( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );

extern LEP_RESULT LEP_GetOemMaskRevision( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_OEM_MASK_REVISION_T *oemMaskRevisionPtr );
   #if 0
extern LEP_RESULT LEP_GetOemMasterID( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_OEM_MASTER_ID_T_PTR oemMasterIDPtr );
   #endif

   #if USE_DEPRECATED_PART_NUMBER_INTERFACE
/* Deprecated: use LEP_GetOemFlirPN instead */
extern LEP_RESULT LEP_GetOemFlirPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr );
/* Deprecated: use LEP_GetOemCustPN instead */
extern LEP_RESULT LEP_GetOemCustPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr );
   #else
extern LEP_RESULT LEP_GetOemFlirPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr );
extern LEP_RESULT LEP_GetOemCustPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr );
   #endif
extern LEP_RESULT LEP_GetOemSoftwareVersion( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_OEM_SW_VERSION_T *oemSoftwareVersionPtr );

   #if 0
extern LEP_RESULT LEP_GetOemVendorID(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_OEM_VENDORID_T *oemVendorIDPtr);
   #endif

extern LEP_RESULT LEP_GetOemVideoOutputEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_VIDEO_OUTPUT_ENABLE_E_PTR oemVideoOutputEnablePtr );
extern LEP_RESULT LEP_SetOemVideoOutputEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_VIDEO_OUTPUT_ENABLE_E oemVideoOutputEnable );

extern LEP_RESULT LEP_GetOemVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_VIDEO_OUTPUT_FORMAT_E_PTR oemVideoOutputFormatPtr );
extern LEP_RESULT LEP_SetOemVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_VIDEO_OUTPUT_FORMAT_E oemVideoOutputFormat );
extern LEP_RESULT LEP_GetOemVideoOutputSource( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_VIDEO_OUTPUT_SOURCE_E_PTR oemVideoOutputSourcePtr );
extern LEP_RESULT LEP_SetOemVideoOutputSource( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_VIDEO_OUTPUT_SOURCE_E oemVideoOutputSource );
extern LEP_RESULT LEP_GetOemVideoOutputSourceConstant( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                       LEP_UINT16 *oemVideoOutputSourceConstant );
extern LEP_RESULT LEP_SetOemVideoOutputSourceConstant( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                       LEP_UINT16 oemVideoOutputSourceConstant );


extern LEP_RESULT LEP_GetOemVideoOutputChannel( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_OEM_VIDEO_OUTPUT_CHANNEL_E_PTR oemVideoOutputChannelPtr );
extern LEP_RESULT LEP_SetOemVideoOutputChannel( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_OEM_VIDEO_OUTPUT_CHANNEL_E oemVideoOutputChannel );

extern LEP_RESULT LEP_GetOemVideoGammaEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_OEM_VIDEO_GAMMA_ENABLE_E_PTR oemVideoGammaEnablePtr );
extern LEP_RESULT LEP_SetOemVideoGammaEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_OEM_VIDEO_GAMMA_ENABLE_E oemVideoGammaEnable );

extern LEP_RESULT LEP_GetOemCalStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_STATUS_E_PTR calStatusPtr );

extern LEP_RESULT LEP_GetOemFFCNormalizationTarget( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_OEM_FFC_NORMALIZATION_TARGET_T_PTR ffcTargetPtr );
extern LEP_RESULT LEP_SetOemFFCNormalizationTarget( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_OEM_FFC_NORMALIZATION_TARGET_T ffcTarget );
extern LEP_RESULT LEP_RunOemFFCNormalization( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_OEM_FFC_NORMALIZATION_TARGET_T ffcTarget );

extern LEP_RESULT LEP_GetOemFrameMean( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_FRAME_AVERAGE_T_PTR frameAveragePtr );

extern LEP_RESULT LEP_GetOemPowerMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_POWER_STATE_E_PTR powerModePtr );
extern LEP_RESULT LEP_SetOemPowerMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_POWER_STATE_E powerMode );

extern LEP_RESULT LEP_RunOemFFC( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );


extern LEP_RESULT LEP_GetOemGpioMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_OEM_GPIO_MODE_E_PTR gpioModePtr );
extern LEP_RESULT LEP_SetOemGpioMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_OEM_GPIO_MODE_E gpioMode );
extern LEP_RESULT LEP_GetOemGpioVsyncPhaseDelay( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_OEM_VSYNC_DELAY_E_PTR numHsyncLinesPtr );
extern LEP_RESULT LEP_SetOemGpioVsyncPhaseDelay( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_OEM_VSYNC_DELAY_E numHsyncLines );


extern LEP_RESULT LEP_GetOemUserDefaultsState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_USER_PARAMS_STATE_E_PTR userParamsStatePtr );
extern LEP_RESULT LEP_RunOemUserDefaultsCopyToOtp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
extern LEP_RESULT LEP_RunOemUserDefaultsRestore( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );



extern LEP_RESULT LEP_GetOemThermalShutdownEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T_PTR ThermalShutdownEnableStatePtr );

extern LEP_RESULT LEP_SetOemThermalShutdownEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T ThermalShutdownEnableState );

extern LEP_RESULT LEP_GetOemShutterProfileObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_SHUTTER_PROFILE_OBJ_T_PTR ShutterProfileObjPtr );

extern LEP_RESULT LEP_SetOemShutterProfileObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_OEM_SHUTTER_PROFILE_OBJ_T ShutterProfileObj );

extern LEP_RESULT LEP_GetOemBadPixelReplaceControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T_PTR BadPixelReplaceControlPtr );
extern LEP_RESULT LEP_SetOemBadPixelReplaceControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T BadPixelReplaceControl );


extern LEP_RESULT LEP_GetOemTemporalFilterControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_OEM_TEMPORAL_FILTER_CONTROL_T_PTR TemporalFilterControlPtr );
extern LEP_RESULT LEP_SetOemTemporalFilterControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_OEM_TEMPORAL_FILTER_CONTROL_T TemporalFilterControl );


extern LEP_RESULT LEP_GetOemColumnNoiseEstimateControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                        LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T_PTR ColumnNoiseEstimateControlPtr );
extern LEP_RESULT LEP_SetOemColumnNoiseEstimateControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                        LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T ColumnNoiseEstimateControl );


extern LEP_RESULT LEP_GetOemPixelNoiseSettings( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_OEM_PIXEL_NOISE_SETTINGS_T_PTR PixelNoiseEstimateControlPtr );
extern LEP_RESULT LEP_SetOemPixelNoiseSettings( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_OEM_PIXEL_NOISE_SETTINGS_T PixelNoiseEstimateControl );


/******************************************************************************/
   #ifdef __cplusplus
}
   #endif

#endif  /* _LEPTON_OEM_H_ */


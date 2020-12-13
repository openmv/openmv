/*******************************************************************************
**
**    File NAME: LEPTON_SYS.h
**
**      AUTHOR:  David Dart
**
**      CREATED: 5/8/2012
**
**      DESCRIPTION: Lepton SDK SYS (system) Module Command
**      Interface
**
**      HISTORY:  5/8/2012 DWD - Initial Draft
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
#ifndef _LEPTON_SYS_H_
   #define _LEPTON_SYS_H_

   #ifdef __cplusplus
extern "C"
{
   #endif

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
   #include "LEPTON_Types.h"

/******************************************************************************/
/** EXPORTED DEFINES                                                         **/
/******************************************************************************/

/* SYS Module Command IDs
*/
   #define LEP_SYS_MODULE_BASE                     0x0200

   #define LEP_CID_SYS_PING                        (LEP_SYS_MODULE_BASE + 0x0000 )
   #define LEP_CID_SYS_CAM_STATUS                  (LEP_SYS_MODULE_BASE + 0x0004 )
   #define LEP_CID_SYS_FLIR_SERIAL_NUMBER          (LEP_SYS_MODULE_BASE + 0x0008 )
   #define LEP_CID_SYS_CAM_UPTIME                  (LEP_SYS_MODULE_BASE + 0x000C )
   #define LEP_CID_SYS_AUX_TEMPERATURE_KELVIN      (LEP_SYS_MODULE_BASE + 0x0010 )
   #define LEP_CID_SYS_FPA_TEMPERATURE_KELVIN      (LEP_SYS_MODULE_BASE + 0x0014 )
   #define LEP_CID_SYS_TELEMETRY_ENABLE_STATE      (LEP_SYS_MODULE_BASE + 0x0018 )
   #define LEP_CID_SYS_TELEMETRY_LOCATION          (LEP_SYS_MODULE_BASE + 0x001C )
   #define LEP_CID_SYS_EXECTUE_FRAME_AVERAGE       (LEP_SYS_MODULE_BASE + 0x0020 )
   #define LEP_CID_SYS_NUM_FRAMES_TO_AVERAGE       (LEP_SYS_MODULE_BASE + 0x0024 )
   #define LEP_CID_SYS_CUST_SERIAL_NUMBER          (LEP_SYS_MODULE_BASE + 0x0028 )
   #define LEP_CID_SYS_SCENE_STATISTICS            (LEP_SYS_MODULE_BASE + 0x002C )
   #define LEP_CID_SYS_SCENE_ROI                   (LEP_SYS_MODULE_BASE + 0x0030 )
   #define LEP_CID_SYS_THERMAL_SHUTDOWN_COUNT      (LEP_SYS_MODULE_BASE + 0x0034 )
   #define LEP_CID_SYS_SHUTTER_POSITION            (LEP_SYS_MODULE_BASE + 0x0038 )
   #define LEP_CID_SYS_FFC_SHUTTER_MODE_OBJ        (LEP_SYS_MODULE_BASE + 0x003C )
   #define FLR_CID_SYS_RUN_FFC                     (LEP_SYS_MODULE_BASE + 0x0042 )
   #define LEP_CID_SYS_FFC_STATUS                  (LEP_SYS_MODULE_BASE + 0x0044 )
   #define LEP_CID_SYS_GAIN_MODE                   (LEP_SYS_MODULE_BASE + 0x0048 )
   #define LEP_CID_SYS_FFC_STATE                   (LEP_SYS_MODULE_BASE + 0x004C )
   #define LEP_CID_SYS_GAIN_MODE_OBJ               (LEP_SYS_MODULE_BASE + 0x0050 )
   #define LEP_CID_SYS_GAIN_MODE_DESIRED_FLAG      (LEP_SYS_MODULE_BASE + 0x0054 )
   #define LEP_CID_SYS_BORESIGHT_VALUES            (LEP_SYS_MODULE_BASE + 0x0058 )

/* SYS Module Attribute Limits
*/

   #define LEP_SYS_MAX_FRAMES_TO_AVERAGE               128          /* Scale is 1x    */
   #define LEP_SYS_MAX_SERIAL_NUMBER_CHAR_SIZE         32

/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/


typedef LEP_UINT64  LEP_SYS_FLIR_SERIAL_NUMBER_T, *LEP_SYS_FLIR_SERIAL_NUMBER_T_PTR;
typedef LEP_UINT32  LEP_SYS_UPTIME_NUMBER_T, *LEP_SYS_UPTIME_NUMBER_T_PTR;
typedef LEP_FLOAT32 LEP_SYS_AUX_TEMPERATURE_CELCIUS_T, *LEP_SYS_AUX_TEMPERATURE_CELCIUS_T_PTR;
typedef LEP_FLOAT32 LEP_SYS_FPA_TEMPERATURE_CELCIUS_T, *LEP_SYS_FPA_TEMPERATURE_CELCIUS_T_PTR;
typedef LEP_UINT16  LEP_SYS_AUX_TEMPERATURE_KELVIN_T, *LEP_SYS_AUX_TEMPERATURE_KELVIN_T_PTR;
typedef LEP_UINT16  LEP_SYS_FPA_TEMPERATURE_KELVIN_T, *LEP_SYS_FPA_TEMPERATURE_KELVIN_T_PTR;
//typedef LEP_UINT8   LEP_SYS_NUM_AVERAGE_FRAMES_T, *LEP_SYS_NUM_AVERAGE_FRAMES_T_PTR;
typedef LEP_UINT16  LEP_SYS_THERMAL_SHUTDOWN_COUNTS_T, *LEP_SYS_THERMAL_SHUTDOWN_COUNTS_T_PTR;
typedef LEP_UINT16  LEP_SYS_THRESHOLD_T, *LEP_SYS_THRESHOLD_T_PTR;

   #if USE_DEPRECATED_SERIAL_NUMBER_INTERFACE
typedef LEP_CHAR8 *LEP_SYS_CUST_SERIAL_NUMBER_T, *LEP_SYS_CUST_SERIAL_NUMBER_T_PTR;
   #else
typedef struct LEP_SYS_CUST_SERIAL_NUMBER_T_TAG
{
   LEP_CHAR8 value[LEP_SYS_MAX_SERIAL_NUMBER_CHAR_SIZE];
} LEP_SYS_CUST_SERIAL_NUMBER_T, *LEP_SYS_CUST_SERIAL_NUMBER_T_PTR;
   #endif

/* SYS Camera System Status Enum
   Captures basic camera operation
*/
typedef enum LEP_SYSTEM_STATUS_STATES_E_TAG
{
   LEP_SYSTEM_READY = 0,
   LEP_SYSTEM_INITIALIZING,
   LEP_SYSTEM_IN_LOW_POWER_MODE,
   LEP_SYSTEM_GOING_INTO_STANDBY,
   LEP_SYSTEM_FLAT_FIELD_IN_PROCESS,
   LEP_SYSTEM_FLAT_FIELD_IMMINENT,
   LEP_SYSTEM_THERMAL_SHUTDOWN_IMMINENT,

   LEP_SYSTEM_END_STATES,

   LEP_SYSTEM_STATES_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_SYSTEM_STATUS_STATES_E, *LEP_SYSTEM_STATUS_STATES_E_PTR;

typedef struct LEP_STATUS_T_TAG
{
   LEP_SYSTEM_STATUS_STATES_E  camStatus;
   LEP_UINT16                  commandCount;
   LEP_UINT16                  reserved;

}LEP_STATUS_T, *LEP_STATUS_T_PTR;

typedef enum LEP_SYS_TELEMETRY_ENABLE_STATE_E_TAG
{
   LEP_TELEMETRY_DISABLED = 0,
   LEP_TELEMETRY_ENABLED,
   LEP_END_TELEMETRY_ENABLE_STATE,

   LEP_TELEMETRY_ENABLE_STATE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_SYS_TELEMETRY_ENABLE_STATE_E, *LEP_SYS_TELEMETRY_ENABLE_STATE_E_PTR;

typedef enum LEP_SYS_TELEMETRY_LOCATION_E_TAG
{
   LEP_TELEMETRY_LOCATION_HEADER = 0,
   LEP_TELEMETRY_LOCATION_FOOTER,
   LEP_END_TELEMETRY_LOCATION,

   LEP_TELEMETRY_LOCATION_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_SYS_TELEMETRY_LOCATION_E, *LEP_SYS_TELEMETRY_LOCATION_E_PTR;

typedef enum LEP_SYS_FRAME_AVERAGE_DIVISOR_E_TAG
{
   LEP_SYS_FA_DIV_1 = 0,
   LEP_SYS_FA_DIV_2,
   LEP_SYS_FA_DIV_4,
   LEP_SYS_FA_DIV_8,
   LEP_SYS_FA_DIV_16,
   LEP_SYS_FA_DIV_32,
   LEP_SYS_FA_DIV_64,
   LEP_SYS_FA_DIV_128,
   LEP_SYS_END_FA_DIV,

   LEP_SYS_FA_DIV_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_SYS_FRAME_AVERAGE_DIVISOR_E, *LEP_SYS_FRAME_AVERAGE_DIVISOR_E_PTR;

typedef struct LEP_SYS_SCENE_STATISTICS_T_TAG
{
   LEP_UINT16   meanIntensity;
   LEP_UINT16   maxIntensity;
   LEP_UINT16   minIntensity;
   LEP_UINT16   numPixels;

} LEP_SYS_SCENE_STATISTICS_T, *LEP_SYS_SCENE_STATISTICS_T_PTR;


typedef struct LEP_SYS_BAD_PIXEL_T_TAG
{
   LEP_UINT8 row;
   LEP_UINT8 col;
   LEP_UINT8 value;
   LEP_UINT8 value2;

}LEP_SYS_BAD_PIXEL_T, *LEP_SYS_BAD_PIXEL_T_PTR;

typedef struct LEP_SYS_VIDEO_ROI_T_TAG
{
   LEP_UINT16 startCol;
   LEP_UINT16 startRow;
   LEP_UINT16 endCol;
   LEP_UINT16 endRow;
} LEP_SYS_VIDEO_ROI_T, *LEP_SYS_VIDEO_ROI_T_PTR;

typedef enum LEP_SYS_ENABLE_E_TAG
{
   LEP_SYS_DISABLE = 0,
   LEP_SYS_ENABLE,

   LEP_END_SYS_ENABLE,

   LEP_SYS_ENABLE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_SYS_ENABLE_E, *LEP_SYS_ENABLE_E_PTR;

typedef enum LEP_SYS_SHUTTER_POSITION_E_TAG
{
   LEP_SYS_SHUTTER_POSITION_UNKNOWN = -1,
   LEP_SYS_SHUTTER_POSITION_IDLE = 0,
   LEP_SYS_SHUTTER_POSITION_OPEN,
   LEP_SYS_SHUTTER_POSITION_CLOSED,
   LEP_SYS_SHUTTER_POSITION_BRAKE_ON,
   LEP_SYS_SHUTTER_POSITION_END,

   LEP_SYS_SHUTTER_POSITION_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_SYS_SHUTTER_POSITION_E,  *LEP_SYS_SHUTTER_POSITION_E_PTR;

typedef enum LEP_SYS_FFC_SHUTTER_MODE_E_TAG
{
   LEP_SYS_FFC_SHUTTER_MODE_MANUAL = 0,
   LEP_SYS_FFC_SHUTTER_MODE_AUTO,
   LEP_SYS_FFC_SHUTTER_MODE_EXTERNAL,

   LEP_SYS_FFC_SHUTTER_MODE_END,

   LEP_SYS_FFC_SHUTTER_MODE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_SYS_FFC_SHUTTER_MODE_E, *LEP_SYS_FFC_SHUTTER_MODE_E_PTR;

typedef enum LEP_SYS_SHUTTER_TEMP_LOCKOUT_STATE_E_TAG
{
   LEP_SYS_SHUTTER_LOCKOUT_INACTIVE = 0,  /* not locked out */
   LEP_SYS_SHUTTER_LOCKOUT_HIGH,    /* lockout due to high temp */
   LEP_SYS_SHUTTER_LOCKOUT_LOW,     /* lockout due to low temp */

   LEP_SYS_SHUTTER_LOCKOUT_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_SYS_SHUTTER_TEMP_LOCKOUT_STATE_E,*LEP_SYS_SHUTTER_TEMP_LOCKOUT_STATE_E_PTR;

   #if 0
typedef struct LEP_SYS_FFC_SHUTTER_TEMP_LOCKOUT_T_TAG
{
   LEP_UINT16  lowTempThreshold;             /* in Kelvin */
   LEP_UINT16  highTempThreshold;            /* in Kelvin */
   LEP_SYS_SHUTTER_TEMP_LOCKOUT_STATE_E tempLockoutState;
}
LEP_SYS_FFC_SHUTTER_TEMP_LOCKOUT_T, *LEP_SYS_FFC_SHUTTER_TEMP_LOCKOUT_T_PTR;
   #endif

typedef struct LEP_SYS_FFC_SHUTTER_MODE_OBJ_T_TAG
{
   LEP_SYS_FFC_SHUTTER_MODE_E shutterMode;   /* defines current mode */

   LEP_SYS_SHUTTER_TEMP_LOCKOUT_STATE_E   tempLockoutState;
   LEP_SYS_ENABLE_E videoFreezeDuringFFC;
   LEP_SYS_ENABLE_E ffcDesired;              /* status of FFC desired */
   LEP_UINT32 elapsedTimeSinceLastFfc;       /* in milliseconds x1 */
   LEP_UINT32 desiredFfcPeriod;              /* in milliseconds x1 */
   LEP_BOOL   explicitCmdToOpen;             /* true or false */
   LEP_UINT16 desiredFfcTempDelta;           /* in Kelvin x100  */
   LEP_UINT16 imminentDelay;                 /* in frame counts x1 */


}LEP_SYS_FFC_SHUTTER_MODE_OBJ_T, *LEP_SYS_FFC_SHUTTER_MODE_OBJ_T_PTR;

/* SYS  Status Enum
   Captures the FFC operation status
*/
typedef enum LEP_SYS_STATUS_E_TAG
{
   LEP_SYS_STATUS_WRITE_ERROR = -2,
   LEP_SYS_STATUS_ERROR = -1,
   LEP_SYS_STATUS_READY = 0,
   LEP_SYS_STATUS_BUSY,
   LEP_SYS_FRAME_AVERAGE_COLLECTING_FRAMES,
   LEP_SYS_STATUS_END,

   LEP_SYS_STATUS_MAKE_32_BIT_ENUM = 0x7FFFFFFF
} LEP_SYS_STATUS_E, *LEP_SYS_STATUS_E_PTR;

typedef enum LEP_SYS_GAIN_MODE_E_TAG
{
   LEP_SYS_GAIN_MODE_HIGH = 0,
   LEP_SYS_GAIN_MODE_LOW,
   LEP_SYS_GAIN_MODE_AUTO,

   LEP_SYS_END_GAIN_MODE,
   LEP_SYS_GAIN_MODE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
} LEP_SYS_GAIN_MODE_E, *LEP_SYS_GAIN_MODE_E_PTR;


/* System Gain Mode ROI Structure
*/
typedef struct LEP_SYS_GAIN_MODE_ROI_T_TAG
{
   LEP_UINT16 startCol;
   LEP_UINT16 startRow;
   LEP_UINT16 endCol;
   LEP_UINT16 endRow;

}LEP_SYS_GAIN_MODE_ROI_T, *LEP_SYS_GAIN_MODE_ROI_T_PTR;

/* Gain Mode Support
*/
typedef struct LEP_SYS_GAIN_MODE_THRESHOLDS_T_TAG
{
   LEP_SYS_THRESHOLD_T sys_P_high_to_low;    /* Range: [0 – 100], percent   */
   LEP_SYS_THRESHOLD_T sys_P_low_to_high;    /* Range: [0 – 100], percent   */

   LEP_SYS_THRESHOLD_T sys_C_high_to_low;    /* Range: [0-600], degrees C   */
   LEP_SYS_THRESHOLD_T sys_C_low_to_high;    /* Range: [0-600], degrees C   */

   LEP_SYS_THRESHOLD_T sys_T_high_to_low;    /* Range: [0-900], Kelvin   */
   LEP_SYS_THRESHOLD_T sys_T_low_to_high;    /* Range: [0-900], Kelvin   */

}LEP_SYS_GAIN_MODE_THRESHOLDS_T, *LEP_SYS_GAIN_MODE_THRESHOLDS_T_PTR;

/* Gain Mode Object
*/
typedef struct LEP_SYS_GAIN_MODE_OBJ_T_TAG
{
   LEP_SYS_GAIN_MODE_ROI_T          sysGainModeROI;         /* Specified ROI to use for Gain Mode switching */
   LEP_SYS_GAIN_MODE_THRESHOLDS_T   sysGainModeThresholds;  /* Set of threshold triggers */
   LEP_UINT16                       sysGainRoiPopulation;   /* Population size in pixels within the ROI */
   LEP_UINT16                       sysGainModeTempEnabled; /* True if T-Linear is implemented */
   LEP_UINT16                       sysGainModeFluxThresholdLow;     /* calculated from desired temp */
   LEP_UINT16                       sysGainModeFluxThresholdHigh;    /* calculated from desired temp */

}LEP_SYS_GAIN_MODE_OBJ_T, *LEP_SYS_GAIN_MODE_OBJ_T_PTR;


/* SYS FFC States Enum
   Captures the current camera FFC operation state
*/
typedef enum LEP_SYS_FFC_STATES_E_TAG
{
   LEP_SYS_FFC_NEVER_COMMANDED = 0,
   LEP_SYS_FFC_IMMINENT,
   LEP_SYS_FFC_IN_PROCESS,
   LEP_SYS_FFC_DONE,

   LEP_SYS_END_FFC_STATES,

   LEP_SYS_FFC_STATES_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_SYS_FFC_STATES_E, *LEP_SYS_FFC_STATES_E_PTR;

typedef struct LEP_SYS_BORESIGHT_VALUES_T_TAG
{
    LEP_UINT16  targetRow;
    LEP_UINT16  targetCol;
    LEP_INT16   targetRotation;
    LEP_INT16   fovX;
    LEP_INT16   fovY;

} LEP_SYS_BORESIGHT_VALUES_T, *LEP_SYS_BORESIGHT_VALUES_T_PTR;

/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/

extern LEP_RESULT LEP_RunSysPing( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );
extern LEP_RESULT LEP_GetSysStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_STATUS_T_PTR sysStatusPtr );
extern LEP_RESULT LEP_GetSysFlirSerialNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_SYS_FLIR_SERIAL_NUMBER_T_PTR sysSerialNumberBufPtr );

/* Deprecated: Use LEP_GetSysCustSN instead */
   #if LEP_SYS_CUST_SERIAL_NUMBER_T
extern LEP_RESULT LEP_GetSysCustSerialNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_SYS_CUST_SERIAL_NUMBER_T_PTR sysSerialNumberPtr );
   #else
extern LEP_RESULT LEP_GetSysCustSerialNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_SYS_CUST_SERIAL_NUMBER_T_PTR sysCustSNPtr );
   #endif
extern LEP_RESULT LEP_GetSysCameraUpTime( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_SYS_UPTIME_NUMBER_T_PTR sysCameraUpTimePtr );

extern LEP_RESULT LEP_GetSysAuxTemperatureCelcius( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_SYS_AUX_TEMPERATURE_CELCIUS_T_PTR auxTemperaturePtr );

extern LEP_RESULT LEP_GetSysFpaTemperatureCelcius( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_SYS_FPA_TEMPERATURE_CELCIUS_T_PTR fpaTemperaturePtr );

extern LEP_RESULT LEP_GetSysAuxTemperatureKelvin( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_SYS_AUX_TEMPERATURE_KELVIN_T_PTR auxTemperaturePtr );

extern LEP_RESULT LEP_GetSysFpaTemperatureKelvin( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_SYS_FPA_TEMPERATURE_KELVIN_T_PTR fpaTemperaturePtr );

extern LEP_RESULT LEP_GetSysTelemetryEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_SYS_TELEMETRY_ENABLE_STATE_E_PTR enableStatePtr );
extern LEP_RESULT LEP_SetSysTelemetryEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_SYS_TELEMETRY_ENABLE_STATE_E enableState );

extern LEP_RESULT LEP_GetSysTelemetryLocation( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_SYS_TELEMETRY_LOCATION_E_PTR telemetryLocationPtr );
extern LEP_RESULT LEP_SetSysTelemetryLocation( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_SYS_TELEMETRY_LOCATION_E telemetryLocation );


extern LEP_RESULT LEP_RunSysAverageFrames( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_SYS_FRAME_AVERAGE_DIVISOR_E numFrameToAverage );
extern LEP_RESULT LEP_GetSysFramesToAverage( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_SYS_FRAME_AVERAGE_DIVISOR_E_PTR numFrameToAveragePtr );
extern LEP_RESULT LEP_SetSysFramesToAverage( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_SYS_FRAME_AVERAGE_DIVISOR_E numFrameToAverage );

extern LEP_RESULT LEP_GetSysSceneStatistics( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_SYS_SCENE_STATISTICS_T_PTR sceneStatisticsPtr );

extern LEP_RESULT LEP_RunFrameAverage( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );

extern LEP_RESULT LEP_GetSysSceneRoi( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_VIDEO_ROI_T_PTR sceneRoiPtr );
extern LEP_RESULT LEP_SetSysSceneRoi( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_VIDEO_ROI_T sceneRoi );

extern LEP_RESULT LEP_GetSysThermalShutdownCount( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_SYS_THERMAL_SHUTDOWN_COUNTS_T_PTR thermalCountsPtr );

extern LEP_RESULT LEP_GetSysShutterPosition( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_SYS_SHUTTER_POSITION_E_PTR shutterPositionPtr );

extern LEP_RESULT LEP_SetSysShutterPosition( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_SYS_SHUTTER_POSITION_E shutterPosition );

extern LEP_RESULT LEP_GetSysFfcShutterModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_SYS_FFC_SHUTTER_MODE_OBJ_T_PTR shutterModeObjPtr );

extern LEP_RESULT LEP_SetSysFfcShutterModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_SYS_FFC_SHUTTER_MODE_OBJ_T shutterModeObj );

extern LEP_RESULT LEP_GetSysFFCStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_SYS_STATUS_E_PTR ffcStatusPtr );

extern LEP_RESULT LEP_RunSysFFCNormalization( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );

extern LEP_RESULT LEP_GetSysGainMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_GAIN_MODE_E_PTR gainModePtr );
extern LEP_RESULT LEP_SetSysGainMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_GAIN_MODE_E gainMode );

extern LEP_RESULT LEP_GetSysFFCStates( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_SYS_FFC_STATES_E_PTR ffcStatePtr );

extern LEP_RESULT LEP_GetSysGainModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_SYS_GAIN_MODE_OBJ_T_PTR gainModeObjPtr );
extern LEP_RESULT LEP_SetSysGainModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_SYS_GAIN_MODE_OBJ_T gainModeObj );

extern LEP_RESULT LEP_GetSysBoresightValues( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_SYS_BORESIGHT_VALUES_T_PTR boresightValuesPtr);

/******************************************************************************/
   #ifdef __cplusplus
}
   #endif

#endif  /* _LEPTON_SYS_H_ */


//+++++++++++++++++++++++++++

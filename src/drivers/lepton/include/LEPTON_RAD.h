/*******************************************************************************
*
*    FILE: LEPTON_RAD.h
*
*       DESCRIPTION:
*
*       AUTHOR:
*
*       CREATED: 11/6/2013
*
*       HISTORY: 11/6/2013 DWD Initial Draft
*
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
#ifndef _LEPTON_RAD_H_
   #define _LEPTON_RAD_H_
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

   #define LEP_RAD_TEMPERATURE_SCALE_FACTOR         100         // All temperatures in degrees C are scaled by 100 1.20 is 120
//
   #define LEP_RAD_RBFO_SCALE_SHIFT                 13          // 2^13 = 8192

   #define LEP_RAD_LUT128_ENTRIES                   128
   #define LEP_RAD_LUT256_ENTRIES                   256


   #define LEP_RAD_MODULE_BASE                     0x4E00   // includes the OEM Bit set 0x4000

   #define LEP_CID_RAD_RBFO_INTERNAL               (LEP_RAD_MODULE_BASE + 0x0000 )  /* High Gain */
   #define LEP_CID_RAD_RBFO_EXTERNAL               (LEP_RAD_MODULE_BASE + 0x0004 )  /* High Gain */
   #define LEP_CID_RAD_DEBUG_TEMP                  (LEP_RAD_MODULE_BASE + 0x0008 )
   #define LEP_CID_RAD_DEBUG_FLUX                  (LEP_RAD_MODULE_BASE + 0x000C )
   #define LEP_CID_RAD_ENABLE_STATE                (LEP_RAD_MODULE_BASE + 0x0010 )
   #define LEP_CID_RAD_GLOBAL_OFFSET               (LEP_RAD_MODULE_BASE + 0x0014 )
   #define LEP_CID_RAD_TFPA_CTS_MODE               (LEP_RAD_MODULE_BASE + 0x0018 )
   #define LEP_CID_RAD_TFPA_CTS                    (LEP_RAD_MODULE_BASE + 0x001C )
   #define LEP_CID_RAD_TEQ_SHUTTER_LUT             (LEP_RAD_MODULE_BASE + 0x0020 )
   #define LEP_CID_RAD_TSHUTTER_MODE               (LEP_RAD_MODULE_BASE + 0x0024 )
   #define LEP_CID_RAD_TSHUTTER                    (LEP_RAD_MODULE_BASE + 0x0028 )
   #define LEP_CID_RAD_RUN_FFC                     (LEP_RAD_MODULE_BASE + 0x002C )
   #define LEP_CID_RAD_RUN_STATUS                  (LEP_RAD_MODULE_BASE + 0x0030 )
   #define LEP_CID_RAD_RESPONSIVITY_SHIFT          (LEP_RAD_MODULE_BASE + 0x0034 )
   #define LEP_CID_RAD_F_NUMBER                    (LEP_RAD_MODULE_BASE + 0x0038 )
   #define LEP_CID_RAD_TAU_LENS                    (LEP_RAD_MODULE_BASE + 0x003C )
   #define LEP_CID_RAD_RESPONSIVITY_VALUE_LUT      (LEP_RAD_MODULE_BASE + 0x0040 )
   #define LEP_CID_RAD_GLOBAL_GAIN                 (LEP_RAD_MODULE_BASE + 0x0044 )
   #define LEP_CID_RAD_RADIOMETRY_FILTER           (LEP_RAD_MODULE_BASE + 0x0048 )
   #define LEP_CID_RAD_TFPA_LUT                    (LEP_RAD_MODULE_BASE + 0x004C )
   #define LEP_CID_RAD_TAUX_LUT                    (LEP_RAD_MODULE_BASE + 0x0050 )
   #define LEP_CID_RAD_TAUX_CTS_MODE               (LEP_RAD_MODULE_BASE + 0x0054 )
   #define LEP_CID_RAD_TAUX_CTS                    (LEP_RAD_MODULE_BASE + 0x0058 )
   #define LEP_CID_RAD_TEQ_SHUTTER_FLUX            (LEP_RAD_MODULE_BASE + 0x005C )
   #define LEP_CID_RAD_MFFC_FLUX                   (LEP_RAD_MODULE_BASE + 0x0060 )
   #define LEP_CID_RAD_FRAME_MEDIAN_VALUE          (LEP_RAD_MODULE_BASE + 0x007C )
   #define LEP_CID_RAD_MLG_LUT                     (LEP_RAD_MODULE_BASE + 0x0088 )
   #define LEP_CID_RAD_THOUSING_TCP                (LEP_RAD_MODULE_BASE + 0x008C )
   #define LEP_CID_RAD_HOUSING_TCP                 (LEP_RAD_MODULE_BASE + 0x008C )
   #define LEP_CID_RAD_SHUTTER_TCP                 (LEP_RAD_MODULE_BASE + 0x0090 )
   #define LEP_CID_RAD_LENS_TCP                    (LEP_RAD_MODULE_BASE + 0x0094 )
   #define LEP_CID_RAD_PREVIOUS_GLOBAL_OFFSET      (LEP_RAD_MODULE_BASE + 0x0098 )
   #define LEP_CID_RAD_PREVIOUS_GLOBAL_GAIN        (LEP_RAD_MODULE_BASE + 0x009C )
   #define LEP_CID_RAD_GLOBAL_GAIN_FFC             (LEP_RAD_MODULE_BASE + 0x00A0 )
   #define LEP_CID_RAD_CNF_SCALE_FACTOR            (LEP_RAD_MODULE_BASE + 0x00A4 )
   #define LEP_CID_RAD_TNF_SCALE_FACTOR            (LEP_RAD_MODULE_BASE + 0x00A8 )
   #define LEP_CID_RAD_SNF_SCALE_FACTOR            (LEP_RAD_MODULE_BASE + 0x00AC )
   #define LEP_CID_RAD_ARBITRARY_OFFSET            (LEP_RAD_MODULE_BASE + 0x00B8 )

   #define LEP_CID_RAD_FLUX_LINEAR_PARAMS          (LEP_RAD_MODULE_BASE + 0x00BC)
   #define LEP_CID_RAD_TLINEAR_ENABLE_STATE        (LEP_RAD_MODULE_BASE + 0x00C0)
   #define LEP_CID_RAD_TLINEAR_RESOLUTION          (LEP_RAD_MODULE_BASE + 0x00C4)
   #define LEP_CID_RAD_TLINEAR_AUTO_RESOLUTION     (LEP_RAD_MODULE_BASE + 0x00C8)
   #define LEP_CID_RAD_SPOTMETER_ROI               (LEP_RAD_MODULE_BASE + 0x00CC)
   #define LEP_CID_RAD_SPOTMETER_OBJ_KELVIN        (LEP_RAD_MODULE_BASE + 0x00D0)

   #define LEP_CID_RAD_RBFO_INTERNAL_LG            (LEP_RAD_MODULE_BASE + 0x00D4 )  /* Low Gain */
   #define LEP_CID_RAD_RBFO_EXTERNAL_LG            (LEP_RAD_MODULE_BASE + 0x00D8 )  /* Low Gain */

   #define LEP_CID_RAD_ARBITRARY_OFFSET_MODE       (LEP_RAD_MODULE_BASE + 0x00DC)
   #define LEP_CID_RAD_ARBITRARY_OFFSET_PARAMS     (LEP_RAD_MODULE_BASE + 0x00E0)

   #define LEP_CID_RAD_RADIO_CAL_VALUES            (LEP_RAD_MODULE_BASE + 0x00E4)

/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/

typedef LEP_UINT16   LEP_RAD_RS_T, *LEP_RAD_RS_T_PTR;
typedef LEP_UINT16   LEP_RAD_FNUMBER_T, *LEP_RAD_FNUMBER_T_PTR;
typedef LEP_UINT16   LEP_RAD_TAULENS_T, *LEP_RAD_TAULENS_T_PTR;

typedef LEP_UINT16   LEP_RAD_FNUM_SHUTTER_T, *LEP_RAD_FNUM_SHUTTER_T_PTR;
typedef LEP_UINT16   LEP_RAD_RADIOMETRY_FILTER_T, *LEP_RAD_RADIOMETRY_FILTER_T_PTR;
typedef LEP_UINT16   LEP_RAD_MEDIAN_VALUE_T, *LEP_RAD_MEDIAN_VALUE_T_PTR;
typedef LEP_UINT16   LEP_RAD_PARAMETER_SCALE_FACTOR_T, *LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR;
typedef LEP_INT16    LEP_RAD_ARBITRARY_OFFSET_T, *LEP_RAD_ARBITRARY_OFFSET_T_PTR;
typedef LEP_UINT16   LEP_RAD_SPOTMETER_KELVIN_T, *LEP_RAD_SPOTMETER_KELVIN_T_PTR;


/* TFpa and TAux counts
*/
typedef LEP_UINT16  LEP_RAD_TEMPERATURE_COUNTS_T, *LEP_RAD_TEMPERATURE_COUNTS_T_PTR;

/* Standard temperature type for temperatures in degrees C
*  These are scaled by 100 so: 100xDegC as [s15.0]
*/
typedef LEP_UINT16  LEP_RAD_KELVIN_T, *LEP_RAD_KELVIN_T_PTR;

/* Flux format: s32 1000x
*/
typedef LEP_INT32   LEP_RAD_FLUX_T, *LEP_RAD_FLUX_T_PTR;

/* Global Gain  [3.13]
*/
typedef LEP_UINT16  LEP_RAD_GLOBAL_GAIN_T, *LEP_RAD_GLOBAL_GAIN_T_PTR;


/* Global Offset  [14.0]
*/
typedef LEP_UINT16  LEP_RAD_GLOBAL_OFFSET_T, *LEP_RAD_GLOBAL_OFFSET_T_PTR;

/* LUTs
*/
typedef LEP_INT16   LEP_RAD_SIGNED_LUT128_T, *LEP_RAD_SIGNED_LUT128_T_PTR;
typedef LEP_UINT16  LEP_RAD_LUT128_T, *LEP_RAD_LUT128_T_PTR;
typedef LEP_UINT16  LEP_RAD_LUT256_T, *LEP_RAD_LUT256_T_PTR;

/* Alpha and Beta
*/
typedef LEP_INT16   LEP_RAD_ALPHA_T, *LEP_RAD_ALPHA_T_PTR;
typedef LEP_INT16   LEP_RAD_BETA_T, *LEP_RAD_BETA_T_PTR;

/* TShutter Mode
*/
typedef enum LEP_RAD_TS_MODE_E_TAG
{
   LEP_RAD_TS_USER_MODE = 0,
   LEP_RAD_TS_CAL_MODE,
   LEP_RAD_TS_FIXED_MODE,
   LEP_RAD_TS_END_TS_MODE,

   LEP_RAD_TS_TS_MODE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_RAD_TS_MODE_E, *LEP_RAD_TS_MODE_E_PTR;

/* RBFO
*/
typedef struct LEP_RBFO_T_TAG
{
   LEP_UINT32 RBFO_R;   // value is not scaled
   LEP_UINT32 RBFO_B;   // value is scaled by X  << n
   LEP_UINT32 RBFO_F;
   LEP_INT32  RBFO_O;

}LEP_RBFO_T, *LEP_RBFO_T_PTR;

/* Linear Temperature correction parameters
*/
typedef struct LEP_RAD_LINEAR_TEMP_CORRECTION_T_TAG
{
   LEP_INT16   offset;     // [s8.7]
   LEP_INT16   gainAux;    // [s2.13]
   LEP_INT16   gainShutter; // [s2.13]
   LEP_UINT16  pad;

}LEP_RAD_LINEAR_TEMP_CORRECTION_T, *LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR;


/* Radiometry Enable state
*/
typedef enum LEP_RAD_ENABLE_E_TAG
{
   LEP_RAD_DISABLE = 0,
   LEP_RAD_ENABLE,
   LEP_END_RAD_ENABLE,

   LEP_RAD_ENABLE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_RAD_ENABLE_E, *LEP_RAD_ENABLE_E_PTR;

/* Temperature TFpa and TAux counts Update Mode
*/
typedef enum LEP_RAD_TEMPERATURE_UPDATE_E_TAG
{
   LEP_RAD_NORMAL_UPDATE = 0,
   LEP_RAD_NO_UPDATE,         // Fixed to last value
   LEP_RAD_UPDATE_END,

   LEP_RAD_UPDATE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_RAD_TEMPERATURE_UPDATE_E, *LEP_RAD_TEMPERATURE_UPDATE_E_PTR;

/* Run operation status
*/
typedef enum
{
   LEP_RAD_STATUS_ERROR = -1,
   LEP_RAD_STATUS_READY = 0,
   LEP_RAD_STATUS_BUSY,
   LEP_RAD_FRAME_AVERAGE_COLLECTING_FRAMES,
   LEP_RAD_STATUS_END,

   LEP_RAD_STATUS_MAKE_32_BIT_ENUM = 0x7FFFFFFF
} LEP_RAD_STATUS_E, *LEP_RAD_STATUS_E_PTR;

typedef struct LEP_RAD_FLUX_LINEAR_PARAMS_T_TAG
{
   /* Type     Field name              format   default  range       comment*/
   LEP_UINT16  sceneEmissivity;     /* 3.13     8192     [82, 8192] */
   LEP_UINT16  TBkgK;               /* 16.0     30000    [, ]        value in kelvin 100x*/
   LEP_UINT16  tauWindow;           /* 3.13     8192     [82, 8192] */
   LEP_UINT16  TWindowK;            /* 16.0     30000    [, ]        value in kelvin 100x*/
   LEP_UINT16  tauAtm;              /* 3.13     8192     [82, 8192] */
   LEP_UINT16  TAtmK;               /* 16.0     30000    [, ]        value in kelvin 100x*/
   LEP_UINT16  reflWindow;          /* 3.13     0        [0, 8192-tauWindow] */
   LEP_UINT16  TReflK;              /* 16.0     30000    [, ]        value in kelvin 100x*/

}LEP_RAD_FLUX_LINEAR_PARAMS_T, *LEP_RAD_FLUX_LINEAR_PARAMS_T_PTR;

typedef enum LEP_RAD_TLINEAR_RESOLUTION_E_TAG
{
   LEP_RAD_RESOLUTION_0_1 = 0,
   LEP_RAD_RESOLUTION_0_01,

   LEP_RAD_END_RESOLUTION,
   
   LEP_RAD_RESOLUTION_MAKE_32_BIT_ENUM = 0x7FFFFFFF
} LEP_RAD_TLINEAR_RESOLUTION_E, *LEP_RAD_TLINEAR_RESOLUTION_E_PTR;

typedef struct LEP_RAD_ROI_T_TAG
{
   LEP_UINT16 startRow;
   LEP_UINT16 startCol;
   LEP_UINT16 endRow;
   LEP_UINT16 endCol;
} LEP_RAD_ROI_T, *LEP_RAD_ROI_T_PTR;

typedef enum LEP_RAD_ARBITRARY_OFFSET_MODE_E_TAG
{
   LEP_RAD_ARBITRARY_OFFSET_MODE_MANUAL = 0,
   LEP_RAD_ARBITRARY_OFFSET_MODE_AUTO,
   LEP_RAD_END_ARBITRARY_OFFSET_MODE,

   LEP_RAD_ARBITRARY_OFFSET_MODE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
} LEP_RAD_ARBITRARY_OFFSET_MODE_E, *LEP_RAD_ARBITRARY_OFFSET_MODE_E_PTR;

typedef struct LEP_RAD_ARBITRARY_OFFSET_PARAMS_T_TAG
{
   LEP_INT16 amplitude;
   LEP_UINT16 decay;

} LEP_RAD_ARBITRARY_OFFSET_PARAMS_T, *LEP_RAD_ARBITRARY_OFFSET_PARAMS_T_PTR;


typedef struct LEP_RAD_SPOTMETER_OBJ_KELVIN_T_TAG
{
    LEP_RAD_SPOTMETER_KELVIN_T  radSpotmeterValue;
    LEP_UINT16                  radSpotmeterMaxValue;
    LEP_UINT16                  radSpotmeterMinValue;
    LEP_UINT16                  radSpotmeterPopulation;

} LEP_RAD_SPOTMETER_OBJ_KELVIN_T, *LEP_RAD_SPOTMETER_OBJ_KELVIN_T_PTR;

typedef struct LEP_RAD_RADIO_CAL_VALUES_T_TAG
{

   LEP_RAD_TEMPERATURE_COUNTS_T  radTauxCounts;
   LEP_RAD_TEMPERATURE_COUNTS_T  radTfpaCounts;

   LEP_RAD_KELVIN_T              radTauxKelvin;
   LEP_RAD_KELVIN_T              radTfpaKelvin;

} LEP_RAD_RADIO_CAL_VALUES_T, *LEP_RAD_RADIO_CAL_VALUES_T_PTR;

/******************************************************************************/
/** EXPORTED PUBLIC DATA DECLARATIONS                                        **/
/******************************************************************************/

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/

extern LEP_RESULT LEP_GetRadTShutterMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_TS_MODE_E_PTR radTShutterModePtr );

extern LEP_RESULT LEP_SetRadTShutterMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_TS_MODE_E radTShutterMode );

extern LEP_RESULT LEP_GetRadTShutter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_KELVIN_T_PTR radTShutterPtr );

extern LEP_RESULT LEP_SetRadTShutter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_KELVIN_T radTShutter );

extern LEP_RESULT LEP_RunRadFFC( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr );

extern LEP_RESULT LEP_GetRadRBFOInternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr );

extern LEP_RESULT LEP_SetRadRBFOInternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr );

extern LEP_RESULT LEP_GetRadRBFOExternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr );

extern LEP_RESULT LEP_SetRadRBFOExternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr );

extern LEP_RESULT LEP_GetRadResponsivityShift( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_RAD_RS_T_PTR radResponsivityShiftPtr );

extern LEP_RESULT LEP_SetRadResponsivityShift( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_RAD_RS_T radResponsivityShift );

extern LEP_RESULT LEP_GetRadFNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_FNUMBER_T_PTR radFNumberPtr );

extern LEP_RESULT LEP_SetRadFNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_FNUMBER_T radFNumber );

extern LEP_RESULT LEP_GetRadTauLens( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_TAULENS_T_PTR radTauLensPtr );

extern LEP_RESULT LEP_SetRadTauLens( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_TAULENS_T radTauLens );

extern LEP_RESULT LEP_GetRadRadometryFilter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_RAD_RADIOMETRY_FILTER_T_PTR radRadiometryFilterPtr );

extern LEP_RESULT LEP_SetRadRadometryFilter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_RAD_RADIOMETRY_FILTER_T radRadiometryFilter );

/* Deprecated: Use LEP_GetRadTFpaLut */
extern LEP_RESULT LEP_GetRadTFpaCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_LUT256_T_PTR radTFpaCLutPtr );
/* Deprecated: Use LEP_SetRadTFpaLut */
extern LEP_RESULT LEP_SetRadTFpaCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_LUT256_T_PTR radTFpaCLutPtr );
/* Deprecated: Use LEP_GetRadTAuxLut */
extern LEP_RESULT LEP_GetRadTAuxCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_LUT256_T_PTR radTAuxCLutPtr );
/* Deprecated: Use LEP_SetRadTAuxLut */
extern LEP_RESULT LEP_SetRadTAuxCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_LUT256_T_PTR radTAuxCLutPtr );

extern LEP_RESULT LEP_GetRadTFpaLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_LUT256_T_PTR radTFpaLutPtr );

extern LEP_RESULT LEP_SetRadTFpaLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_LUT256_T_PTR radTFpaLutPtr );

extern LEP_RESULT LEP_GetRadTAuxLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_LUT256_T_PTR radTAuxLutPtr );

extern LEP_RESULT LEP_SetRadTAuxLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_LUT256_T_PTR radTAuxLutPtr );

extern LEP_RESULT LEP_GetRadResponsivityValueLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RAD_LUT128_T_PTR radResponsivityValueLutPtr );

extern LEP_RESULT LEP_SetRadResponsivityValueLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RAD_LUT128_T_PTR radResponsivityValueLutPtr );

extern LEP_RESULT LEP_GetRadDebugTemp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_KELVIN_T_PTR radDebugTempPtr );

extern LEP_RESULT LEP_SetRadDebugTemp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_KELVIN_T radDebugTemp );

extern LEP_RESULT LEP_GetRadDebugFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_FLUX_T_PTR radDebugFluxPtr );

extern LEP_RESULT LEP_SetRadDebugFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_FLUX_T radDebugFlux );

extern LEP_RESULT LEP_GetRadEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_ENABLE_E_PTR radEnableStatePtr );

extern LEP_RESULT LEP_SetRadEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_ENABLE_E radEnableState );

extern LEP_RESULT LEP_GetRadGlobalGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_GLOBAL_GAIN_T_PTR radGlobalGainPtr );

extern LEP_RESULT LEP_SetRadGlobalGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_GLOBAL_GAIN_T radGlobalGain );

extern LEP_RESULT LEP_GetRadGlobalOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_GLOBAL_OFFSET_T_PTR radGlobalOffsetPtr );

extern LEP_RESULT LEP_SetRadGlobalOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_GLOBAL_OFFSET_T radGlobalOffset );

extern LEP_RESULT LEP_GetRadTFpaCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_TEMPERATURE_UPDATE_E_PTR radTFpaCtsModePtr );

extern LEP_RESULT LEP_SetRadTFpaCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_TEMPERATURE_UPDATE_E radTFpaCtsMode );

extern LEP_RESULT LEP_GetRadTAuxCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_TEMPERATURE_UPDATE_E_PTR radTAuxCtsModePtr );

extern LEP_RESULT LEP_SetRadTAuxCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_TEMPERATURE_UPDATE_E radTAuxCtsMode );

extern LEP_RESULT LEP_GetRadTFpaCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_TEMPERATURE_COUNTS_T_PTR radTFpaCtsPtr );

extern LEP_RESULT LEP_SetRadTFpaCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_TEMPERATURE_COUNTS_T radTFpaCts );

extern LEP_RESULT LEP_GetRadTAuxCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_TEMPERATURE_COUNTS_T_PTR radTAuxCtsPtr );

extern LEP_RESULT LEP_SetRadTAuxCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_TEMPERATURE_COUNTS_T radTAuxCts );

extern LEP_RESULT LEP_GetRadTEqShutterLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_LUT128_T_PTR radTEqShutterLutPtr );

extern LEP_RESULT LEP_SetRadTEqShutterLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_LUT128_T_PTR radTEqShutterLutPtr );

extern LEP_RESULT LEP_GetRadRunStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_STATUS_E_PTR radStatusPtr );

extern LEP_RESULT LEP_GetRadTEqShutterFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_FLUX_T_PTR radTEqShutterFluxPtr );

extern LEP_RESULT LEP_SetRadTEqShutterFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_FLUX_T radTEqShutterFlux );

extern LEP_RESULT LEP_GetRadMffcFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_FLUX_T_PTR radRadMffcFluxPtr  );

extern LEP_RESULT LEP_SetRadMffcFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_FLUX_T radRadMffcFlux );

extern LEP_RESULT LEP_GetRadFrameMedianPixelValue( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_RAD_MEDIAN_VALUE_T_PTR frameMedianPtr );

extern LEP_RESULT LEP_GetRadMLGLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RAD_SIGNED_LUT128_T_PTR radMLGLutPtr );

extern LEP_RESULT LEP_SetRadMLGLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RAD_SIGNED_LUT128_T_PTR radMLGLutPtr );

   #if USE_DEPRECATED_HOUSING_TCP_INTERFACE
extern LEP_RESULT LEP_GetRadTHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR radHousingTcp );
extern LEP_RESULT LEP_SetRadTHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_LINEAR_TEMP_CORRECTION_T radHousingTcp );
   #else
extern LEP_RESULT LEP_GetRadHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR radHousingTcp );
extern LEP_RESULT LEP_SetRadHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_LINEAR_TEMP_CORRECTION_T radHousingTcp );
   #endif


extern LEP_RESULT LEP_GetRadShutterTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR radShutterTcp );

extern LEP_RESULT LEP_SetRadShutterTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_LINEAR_TEMP_CORRECTION_T radShutterTcp );

extern LEP_RESULT LEP_GetRadLensTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR radLensTcp );

extern LEP_RESULT LEP_SetRadLensTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_LINEAR_TEMP_CORRECTION_T radLensTcp );

extern LEP_RESULT LEP_GetRadPreviousGlobalOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RAD_GLOBAL_OFFSET_T_PTR globalOffsetPtr );

extern LEP_RESULT LEP_GetRadPreviousGlobalGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_RAD_GLOBAL_GAIN_T_PTR globalGainPtr );

extern LEP_RESULT LEP_GetGlobalGainFFC( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_GLOBAL_GAIN_T_PTR globalGainFfcPtr );

extern LEP_RESULT LEP_GetRadCnfScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR scaleFactorPtr );
extern LEP_RESULT LEP_GetRadTnfScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR scaleFactorPtr );
extern LEP_RESULT LEP_GetRadSnfScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR scaleFactorPtr );

extern LEP_RESULT LEP_GetRadArbitraryOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_RAD_ARBITRARY_OFFSET_T_PTR arbitraryOffsetPtr );
extern LEP_RESULT LEP_SetRadArbitraryOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_RAD_ARBITRARY_OFFSET_T arbitraryOffset );

extern LEP_RESULT LEP_GetRadFluxLinearParams( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_RAD_FLUX_LINEAR_PARAMS_T_PTR fluxParamsPtr );

extern LEP_RESULT LEP_SetRadFluxLinearParams( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_RAD_FLUX_LINEAR_PARAMS_T fluxParams );

extern LEP_RESULT LEP_GetRadTLinearEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_RAD_ENABLE_E_PTR enableStatePtr );

extern LEP_RESULT LEP_SetRadTLinearEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_RAD_ENABLE_E enableState );

extern LEP_RESULT LEP_GetRadTLinearResolution( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_RAD_TLINEAR_RESOLUTION_E_PTR resolutionPtr );

extern LEP_RESULT LEP_SetRadTLinearResolution( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_RAD_TLINEAR_RESOLUTION_E resolution );

extern LEP_RESULT LEP_GetRadTLinearAutoResolution( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_RAD_ENABLE_E_PTR enableStatePtr );

extern LEP_RESULT LEP_SetRadTLinearAutoResolution( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_RAD_ENABLE_E enableState );

extern LEP_RESULT LEP_GetRadSpotmeterRoi( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_ROI_T_PTR spotmeterRoiPtr );

extern LEP_RESULT LEP_SetRadSpotmeterRoi( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_ROI_T spotmeterRoi );

extern LEP_RESULT LEP_GetRadSpotmeterObjInKelvinX100( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                      LEP_RAD_SPOTMETER_OBJ_KELVIN_T_PTR kelvinPtr );

extern LEP_RESULT LEP_GetRadArbitraryOffsetMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_RAD_ARBITRARY_OFFSET_MODE_E_PTR arbitraryOffsetModePtr );

extern LEP_RESULT LEP_SetRadArbitraryOffsetMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_RAD_ARBITRARY_OFFSET_MODE_E arbitraryOffsetMode );

extern LEP_RESULT LEP_GetRadArbitraryOffsetParams( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_RAD_ARBITRARY_OFFSET_PARAMS_T_PTR arbitraryOffsetParamsPtr);

extern LEP_RESULT LEP_SetRadArbitraryOffsetParams( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_RAD_ARBITRARY_OFFSET_PARAMS_T arbitraryOffsetParams);

extern LEP_RESULT LEP_GetRadInternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RBFO_T_PTR radRBFOPtr );

extern LEP_RESULT LEP_SetRadInternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RBFO_T_PTR radRBFOPtr );

extern LEP_RESULT LEP_GetRadExternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RBFO_T_PTR radRBFOPtr );

extern LEP_RESULT LEP_SetRadExternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_RBFO_T_PTR radRBFOPtr );

extern LEP_RESULT LEP_GetRadInternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_RBFO_T_PTR radRBFOPtr );

extern LEP_RESULT LEP_SetRadInternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_RBFO_T_PTR radRBFOPtr );

extern LEP_RESULT LEP_GetRadExternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_RBFO_T_PTR radRBFOPtr );

extern LEP_RESULT LEP_SetRadExternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_RBFO_T_PTR radRBFOPtr );

extern LEP_RESULT LEP_GetRadRadioCalValues( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_RADIO_CAL_VALUES_T_PTR radRadioCalValuesPtr);

extern LEP_RESULT LEP_SetRadRadioCalValues( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_RADIO_CAL_VALUES_T radRadioCalValues );

/******************************************************************************/
   #ifdef __cplusplus
}
   #endif
#endif   /* _LEPTON_RAD_H_  */


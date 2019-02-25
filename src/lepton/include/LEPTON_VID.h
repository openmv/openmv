/*******************************************************************************
**
**    File NAME: LEPTON_VID.h
**
**      AUTHOR:  David Dart
**
**      CREATED: 5/8/2012
**
**      DESCRIPTION: Lepton SDK VID (video) Module Command
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
#ifndef _LEPTON_VID_H_
   #define _LEPTON_VID_H_

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

   /* VID Module Command IDs
   */
   #define LEP_VID_MODULE_BASE                  0x0300

   #define LEP_CID_VID_POLARITY_SELECT         (LEP_VID_MODULE_BASE + 0x0000 )
   #define LEP_CID_VID_LUT_SELECT              (LEP_VID_MODULE_BASE + 0x0004 )
   #define LEP_CID_VID_LUT_TRANSFER            (LEP_VID_MODULE_BASE + 0x0008 )
   #define LEP_CID_VID_FOCUS_CALC_ENABLE       (LEP_VID_MODULE_BASE + 0x000C )
   #define LEP_CID_VID_FOCUS_ROI               (LEP_VID_MODULE_BASE + 0x0010 )
   #define LEP_CID_VID_FOCUS_THRESHOLD         (LEP_VID_MODULE_BASE + 0x0014 )
   #define LEP_CID_VID_FOCUS_METRIC            (LEP_VID_MODULE_BASE + 0x0018 )
   #define LEP_CID_VID_SBNUC_ENABLE            (LEP_VID_MODULE_BASE + 0x001C )
   #define LEP_CID_VID_GAMMA_SELECT            (LEP_VID_MODULE_BASE + 0x0020 )
   #define LEP_CID_VID_FREEZE_ENABLE           (LEP_VID_MODULE_BASE + 0x0024 )
   #define LEP_CID_VID_BORESIGHT_CALC_ENABLE   (LEP_VID_MODULE_BASE + 0x0028 )
   #define LEP_CID_VID_BORESIGHT_COORDINATES   (LEP_VID_MODULE_BASE + 0x002C )
   #define LEP_CID_VID_VIDEO_OUTPUT_FORMAT     (LEP_VID_MODULE_BASE + 0x0030 )
   #define LEP_CID_VID_LOW_GAIN_COLOR_LUT      (LEP_VID_MODULE_BASE + 0x0034 )

   /* VID Module Attribute Limits
   */


   /* VID Module Object Sizes
   */
   #define LEPTON_COLOR_LUT_SIZE               256     /* 8-bit LUT as 256 x 8-bits */

/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/

   typedef LEP_UINT32 LEP_VID_FOCUS_METRIC_T, *LEP_VID_FOCUS_METRIC_T_PTR;
   typedef LEP_UINT32 LEP_VID_FOCUS_METRIC_THRESHOLD_T, *LEP_VID_FOCUS_METRIC_THRESHOLD_T_PTR;


   typedef enum LEP_POLARITY_E_TAG
   {
      LEP_VID_WHITE_HOT=0,
      LEP_VID_BLACK_HOT,
      LEP_VID_END_POLARITY,

      LEP_VID_POLARITY_MAKE_32_BIT_ENUM = 0x7FFFFFFF
   }LEP_POLARITY_E, *LEP_POLARITY_E_PTR;


   /* Video Pseudo color LUT Enum
   */
   typedef enum LEP_PCOLOR_LUT_E_TAG
   {
      LEP_VID_WHEEL6_LUT=0,
      LEP_VID_FUSION_LUT,
      LEP_VID_RAINBOW_LUT,
      LEP_VID_GLOBOW_LUT,
      LEP_VID_SEPIA_LUT,
      LEP_VID_COLOR_LUT,
      LEP_VID_ICE_FIRE_LUT,
      LEP_VID_RAIN_LUT,
      LEP_VID_USER_LUT,
      LEP_VID_END_PCOLOR_LUT,

      LEP_VID_PCOLOR_LUT_MAKE_32_BIT_ENUM = 0x7FFFFFFF
   }LEP_PCOLOR_LUT_E, *LEP_PCOLOR_LUT_E_PTR;

   /* User-Defined color look-up table (LUT)
   */
   typedef struct LEP_VID_LUT_PIXEL_T_TAG
   {
      LEP_UINT8 reserved;
      LEP_UINT8 red;
      LEP_UINT8 green;
      LEP_UINT8 blue;
   } LEP_VID_LUT_PIXEL_T, *LEP_VID_LUT_PIXEL_T_PTR;
   typedef struct LEP_VID_LUT_BUFFER_T_TAG
   {
      LEP_VID_LUT_PIXEL_T bin[256];
   } LEP_VID_LUT_BUFFER_T, *LEP_VID_LUT_BUFFER_T_PTR;

#if 0
   typedef struct
   {
      LEP_UINT8 bin[LEPTON_COLOR_LUT_SIZE];

   }LEP_VID_LUT_BUFFER_T, *LEP_VID_LUT_BUFFER_T_PTR;
#endif

   /* Video Focus Metric Calculation Enable Enum
   */
   typedef enum LEP_VID_ENABLE_TAG
   {
      LEP_VID_FOCUS_CALC_DISABLE=0,
      LEP_VID_FOCUS_CALC_ENABLE,
      LEP_VID_END_FOCUS_CALC_ENABLE,

      LEP_VID_FOCUS_CALC_ENABLE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
   }LEP_VID_FOCUS_CALC_ENABLE_E, *LEP_VID_FOCUS_CALC_ENABLE_E_PTR;


   /* Video Focus ROI Structure
   */
   typedef struct LEP_VID_FOCUS_ROI_TAG
   {
      LEP_UINT16 startCol;
      LEP_UINT16 startRow;
      LEP_UINT16 endCol;
      LEP_UINT16 endRow;

   }LEP_VID_FOCUS_ROI_T, *LEP_VID_FOCUS_ROI_T_PTR;


   /* Video Scene-Based NUC Enable Enum
   */
   typedef enum LEP_VID_SBNUC_ENABLE_TAG
   {
      LEP_VID_SBNUC_DISABLE = 0,
      LEP_VID_SBNUC_ENABLE,
      LEP_VID_END_SBNUC_ENABLE,

      LEP_VID_SBNUC_ENABLE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
   }LEP_VID_SBNUC_ENABLE_E, *LEP_VID_SBNUC_ENABLE_E_PTR ;


   /* Video Freeze Output Enable Enum
   */
   typedef enum LEP_VID_FREEZE_ENABLE_TAG
   {
      LEP_VID_FREEZE_DISABLE = 0,
      LEP_VID_FREEZE_ENABLE,
      LEP_VID_END_FREEZE_ENABLE,

      LEP_VID_FREEZE_ENABLE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
   }LEP_VID_FREEZE_ENABLE_E, *LEP_VID_FREEZE_ENABLE_E_PTR ;

   typedef enum LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E_TAG
   {
      LEP_VID_BORESIGHT_CALC_DISABLED = 0,
      LEP_VID_BORESIGHT_CALC_ENABLED,
      LEP_VID_END_BORESIGHT_CALC_ENABLE_STATE,

      LEP_VID_BORESIGHT_CALC_ENABLE_STATE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
   } LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E, *LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E_PTR;

   typedef struct LEP_PIXEL_COORDINATE_T_TAG
   {
      LEP_UINT16  row;
      LEP_UINT16  col;

   }LEP_PIXEL_COORDINATE_T, *LEP_PIXEL_COORDINATE_T_PTR;

   typedef struct LEP_VID_BORESIGHT_COORDINATES_T_TAG
   {
      LEP_PIXEL_COORDINATE_T   top_0;        /* Top row 0 */
      LEP_PIXEL_COORDINATE_T   top_1;        /* Top row 1 */
      LEP_PIXEL_COORDINATE_T   bottom_0;     /* Bottom row 118 */
      LEP_PIXEL_COORDINATE_T   bottom_1;     /* Bottom row 119 */
      LEP_PIXEL_COORDINATE_T   left_0;       /* Left column 0 */
      LEP_PIXEL_COORDINATE_T   left_1;       /* Left column 1 */
      LEP_PIXEL_COORDINATE_T   right_0;      /* Right column 158 */
      LEP_PIXEL_COORDINATE_T   right_1;      /* Right column 159 */

   } LEP_VID_BORESIGHT_COORDINATES_T, *LEP_VID_BORESIGHT_COORDINATES_T_PTR;


   typedef struct LEP_VID_TARGET_POSITION_T_TAG
   {
      LEP_FLOAT32 row;
      LEP_FLOAT32 col;
      LEP_FLOAT32 rotation;

   }LEP_VID_TARGET_POSITION_T, *LEP_VID_TARGET_POSITION_T_PTR;

   typedef enum LEP_VID_VIDEO_OUTPUT_FORMAT_TAG
   {
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW8 = 0,          // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW10,             // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW12,             // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RGB888,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RGB666,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RGB565,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_YUV422_8BIT,       // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW14,             // SUPPORTED in this release
      LEP_VID_VIDEO_OUTPUT_FORMAT_YUV422_10BIT,      // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_USER_DEFINED,      // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW8_2,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW8_3,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW8_4,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW8_5,            // To be supported in later release
      LEP_VID_VIDEO_OUTPUT_FORMAT_RAW8_6,            // To be supported in later release
      LEP_END_VID_VIDEO_OUTPUT_FORMAT,

      LEP_VID_VIDEO_OUTPUT_FORMAT_MAKE_32_BIT_ENUM = 0x7FFFFFFF
   }LEP_VID_VIDEO_OUTPUT_FORMAT_E, *LEP_VID_VIDEO_OUTPUT_FORMAT_E_PTR;

/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/
   extern LEP_RESULT LEP_GetVidPolarity(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_POLARITY_E_PTR vidPolarityPtr);
   extern LEP_RESULT LEP_SetVidPolarity(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_POLARITY_E vidPolarity);

   extern LEP_RESULT LEP_GetVidPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_PCOLOR_LUT_E_PTR vidPcolorLutPtr);
   extern LEP_RESULT LEP_SetVidPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_PCOLOR_LUT_E vidPcolorLut);

   extern LEP_RESULT LEP_GetVidLowGainPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_PCOLOR_LUT_E_PTR vidPcolorLutPtr);
   extern LEP_RESULT LEP_SetVidLowGainPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_PCOLOR_LUT_E vidPcolorLut);

   extern LEP_RESULT LEP_GetVidUserLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_VID_LUT_BUFFER_T_PTR vidUserLutBufPtr);

   extern LEP_RESULT LEP_SetVidUserLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_VID_LUT_BUFFER_T_PTR vidUserLutBufPtr);

   extern LEP_RESULT LEP_GetVidFocusCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_VID_FOCUS_CALC_ENABLE_E_PTR vidEnableFocusCalcStatePtr);
   extern LEP_RESULT LEP_SetVidFocusCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_VID_FOCUS_CALC_ENABLE_E vidFocusCalcEnableState);

   extern LEP_RESULT LEP_GetVidBoresightCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                        LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E_PTR boresightCalcEnableStatePtr);
   extern LEP_RESULT LEP_SetVidBoresightCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                        LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E boresightCalcEnableState);
   extern LEP_RESULT LEP_GetVidBoresightCoordinates(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_VID_BORESIGHT_COORDINATES_T_PTR boresightCoordinatesPtr);
   extern LEP_RESULT LEP_GetVidTargetPosition(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_VID_TARGET_POSITION_T_PTR targetPositionPtr);

   extern LEP_RESULT LEP_GetVidROI(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_VID_FOCUS_ROI_T_PTR vidFocusROIPtr);
   extern LEP_RESULT LEP_SetVidROI(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_VID_FOCUS_ROI_T vidFocusROI);

   extern LEP_RESULT LEP_GetVidFocusMetric(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_VID_FOCUS_METRIC_T_PTR vidFocusMetricPtr);

   extern LEP_RESULT LEP_GetVidFocusMetricThreshold(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_VID_FOCUS_METRIC_THRESHOLD_T_PTR vidFocusMetricThresholdPtr);
   extern LEP_RESULT LEP_SetVidFocusMetricThreshold(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_VID_FOCUS_METRIC_THRESHOLD_T vidFocusMetricThreshold);

   extern LEP_RESULT LEP_GetVidSbNucEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_VID_SBNUC_ENABLE_E_PTR vidSbNucEnableStatePtr);
   extern LEP_RESULT LEP_SetVidSbNucEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_VID_SBNUC_ENABLE_E vidSbNucEnableState);

   //extern LEP_RESULT LEP_GetOemVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
   //                                               LEP_VID_VIDEO_OUTPUT_FORMAT_E_PTR vidVideoOutputFormatPtr );
   //extern LEP_RESULT LEP_SetOemVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
   //                                               LEP_VID_VIDEO_OUTPUT_FORMAT_E vidVideoOutputFormat );

#if (USE_BORESIGHT_MEASUREMENT_FUNCTIONS == 1)
   extern LEP_RESULT LEP_CalcVidBoresightAlignment(LEP_VID_BORESIGHT_COORDINATES_T boresightCoordinates,
                                                   LEP_VID_TARGET_POSITION_T_PTR targetPositionPtr);
#endif
/******************************************************************************/
   #ifdef __cplusplus
}
   #endif

#endif  /* _LEPTON_VID_H_ */


/*******************************************************************************
**
**    File NAME: LEPTON_AGC.h
**
**      AUTHOR:  David Dart
**
**      CREATED: 4/30/2012
**
**      DESCRIPTION: Lepton SDK AGC Module Command Interface
**
**      HISTORY:  4/30/2012 DWD - Initial Draft
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
#ifndef _LEPTON_AGC_H_
   #define _LEPTON_AGC_H_
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

/* AGC Module Command IDs
*/
   #define LEP_AGC_MODULE_BASE                     0x0100

   #define LEP_CID_AGC_ENABLE_STATE                (LEP_AGC_MODULE_BASE + 0x0000 )
   #define LEP_CID_AGC_POLICY                      (LEP_AGC_MODULE_BASE + 0x0004 )
   #define LEP_CID_AGC_ROI                         (LEP_AGC_MODULE_BASE + 0x0008 )
   #define LEP_CID_AGC_STATISTICS                  (LEP_AGC_MODULE_BASE + 0x000C )
   #define LEP_CID_AGC_HISTOGRAM_CLIP_PERCENT      (LEP_AGC_MODULE_BASE + 0x0010 )
   #define LEP_CID_AGC_HISTOGRAM_TAIL_SIZE         (LEP_AGC_MODULE_BASE + 0x0014 )
   #define LEP_CID_AGC_LINEAR_MAX_GAIN             (LEP_AGC_MODULE_BASE + 0x0018 )
   #define LEP_CID_AGC_LINEAR_MIDPOINT             (LEP_AGC_MODULE_BASE + 0x001C )
   #define LEP_CID_AGC_LINEAR_DAMPENING_FACTOR     (LEP_AGC_MODULE_BASE + 0x0020 )
   #define LEP_CID_AGC_HEQ_DAMPENING_FACTOR        (LEP_AGC_MODULE_BASE + 0x0024 )
   #define LEP_CID_AGC_HEQ_MAX_GAIN                (LEP_AGC_MODULE_BASE + 0x0028 )
   #define LEP_CID_AGC_HEQ_CLIP_LIMIT_HIGH         (LEP_AGC_MODULE_BASE + 0x002C )
   #define LEP_CID_AGC_HEQ_CLIP_LIMIT_LOW          (LEP_AGC_MODULE_BASE + 0x0030 )
   #define LEP_CID_AGC_HEQ_BIN_EXTENSION           (LEP_AGC_MODULE_BASE + 0x0034 )
   #define LEP_CID_AGC_HEQ_MIDPOINT                (LEP_AGC_MODULE_BASE + 0x0038 )
   #define LEP_CID_AGC_HEQ_EMPTY_COUNTS            (LEP_AGC_MODULE_BASE + 0x003C )
   #define LEP_CID_AGC_HEQ_NORMALIZATION_FACTOR    (LEP_AGC_MODULE_BASE + 0x0040 )
   #define LEP_CID_AGC_HEQ_SCALE_FACTOR            (LEP_AGC_MODULE_BASE + 0x0044 )
   #define LEP_CID_AGC_CALC_ENABLE_STATE           (LEP_AGC_MODULE_BASE + 0x0048 )
   #define LEP_CID_AGC_HEQ_LINEAR_PERCENT          (LEP_AGC_MODULE_BASE + 0x004C )

/* AGC Module Attribute Scaling and Module Attribute Limits
*/
/* Linear
*/
   #define LEP_AGC_MAX_HISTOGRAM_CLIP_PERCENT      100         /* Scale is 10x  100 == 10.0%  */
   #define LEP_AGC_MAX_HISTOGRAM_TAIL_SIZE         (80*60)     /* Scale is 1x  units: pixels  */
   #define LEP_AGC_MIN_LINEAR_MAX_GAIN              1          /* Scale is 1x    */
   #define LEP_AGC_MAX_LINEAR_MAX_GAIN              4          /* Scale is 1x    */
   #define LEP_AGC_MIN_LINEAR_MIDPOINT              1          /* Scale is 1x    */
   #define LEP_AGC_MAX_LINEAR_MIDPOINT              254        /* Scale is 1x    */
   #define LEP_AGC_MAX_LINEAR_DAMPENING_FACTOR      100        /* Scale is 1x  units: percent  */

/* Histogram Equalization
*/
   #define LEP_AGC_MAX_HEQ_DAMPENING_FACTOR         100        /* Scale is 1x  units: percent  */
   #define LEP_AGC_MIN_HEQ_MAX_GAIN                 1          /* Scale is 1x    */
   #define LEP_AGC_MAX_HEQ_MAX_GAIN                 4          /* Scale is 1x    */

   #define LEP_AGC_HEQ_CLIP_LIMIT_HIGH             (80*60)     /* Scale is 1x  units: pixels  */
   #define LEP_AGC_HEQ_CLIP_LIMIT_LOW              (80*60)     /* Scale is 1x  units: pixels  */
   #define LEP_AGC_HEQ_MAX_BIN_EXTENSION            16         /* Scale is 1x  units: bins  */

   #define LEP_AGC_MIN_HEQ_MIDPOINT                127         /* Scale is 1x    */
   #define LEP_AGC_MAX_HEQ_MIDPOINT                65534       /* Scale is 1x    */

/* ROI
*/
   #define LEP_AGC_MAX_COL                         79
   #define LEP_AGC_MAX_ROW                         59
   #define LEP_AGC_MIN_COL                         0
   #define LEP_AGC_MIN_ROW                         0

/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/
typedef LEP_UINT16  LEP_AGC_HEQ_EMPTY_COUNT_T, *LEP_AGC_HEQ_EMPTY_COUNT_T_PTR;
typedef LEP_UINT16  LEP_AGC_HEQ_NORMALIZATION_FACTOR_T, *LEP_AGC_HEQ_NORMALIZATION_FACTOR_T_PTR;

/* AGC Enable Enum
*/
typedef enum LEP_AGC_ENABLE_TAG
{
   LEP_AGC_DISABLE = 0,
   LEP_AGC_ENABLE,
   LEP_END_AGC_ENABLE,
   LEP_AGC_ENABLE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_AGC_ENABLE_E, *LEP_AGC_ENABLE_E_PTR;

/* AGC Policy Enum
*/
typedef enum LEP_AGC_POLICY_TAG
{
   LEP_AGC_LINEAR = 0,
   LEP_AGC_HEQ,
   LEP_END_AGC_POLICY,
   LEP_AGC_POLICY_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_AGC_POLICY_E, *LEP_AGC_POLICY_E_PTR;


/* AGC ROI Structure
*/
typedef struct LEP_AGC_ROI_TAG
{
   LEP_UINT16 startCol;
   LEP_UINT16 startRow;
   LEP_UINT16 endCol;
   LEP_UINT16 endRow;

}LEP_AGC_ROI_T, *LEP_AGC_ROI_T_PTR;

/* AGC Histogram Statistics Structure
*/
typedef struct LEP_AGC_HISTOGRAM_STATISTICS_TAG
{
   LEP_UINT16  minIntensity;
   LEP_UINT16  maxIntensity;
   LEP_UINT16  meanIntensity;
   LEP_UINT16  numPixels;

}LEP_AGC_HISTOGRAM_STATISTICS_T, *LEP_AGC_HISTOGRAM_STATISTICS_T_PTR;

/* AGC Output Scale Factor Structure
*/
typedef enum LEP_AGC_SCALE_FACTOR_E_TAG
{
   LEP_AGC_SCALE_TO_8_BITS = 0,
   LEP_AGC_SCALE_TO_14_BITS,
   LEP_AGC_END_SCALE_TO,
   LEP_AGC_SCALE_TO_MAKE_32_BIT_ENUM = 0x7FFFFFFF
}LEP_AGC_HEQ_SCALE_FACTOR_E, *LEP_AGC_HEQ_SCALE_FACTOR_E_PTR;


/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/

/* General AGC Controls
*/

extern LEP_RESULT LEP_GetAgcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_AGC_ENABLE_E_PTR agcEnableStatePtr );
extern LEP_RESULT LEP_SetAgcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_AGC_ENABLE_E agcEnableState );

extern LEP_RESULT LEP_GetAgcPolicy( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_AGC_POLICY_E_PTR agcPolicyPtr );
extern LEP_RESULT LEP_SetAgcPolicy( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_AGC_POLICY_E agcPolicy );

extern LEP_RESULT LEP_GetAgcROI( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_AGC_ROI_T_PTR agcROIPtr );
extern LEP_RESULT LEP_SetAgcROI( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_AGC_ROI_T agcROI );

extern LEP_RESULT LEP_GetAgcHistogramStatistics( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_AGC_HISTOGRAM_STATISTICS_T_PTR *agcHistogramStatisticsPtr );

/* Linear AGC Policy Controls
*/
extern LEP_RESULT LEP_GetAgcLinearHistogramTailSize( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                     LEP_UINT16 *agcLinearHistogramTailSizePtr );
extern LEP_RESULT LEP_SetAgcLinearHistogramTailSize( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                     LEP_UINT16 agcLinearHistogramTailSize );

extern LEP_RESULT LEP_GetAgcHistogramClipPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_UINT16 *agcLinearClipPercentPtr );
extern LEP_RESULT LEP_SetAgcHistogramClipPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_UINT16 agcLinearClipPercent );

extern LEP_RESULT LEP_GetAgcLinearMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_UINT16 *agcLinearMaxGainPtr );
extern LEP_RESULT LEP_SetAgcLinearMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_UINT16 agcLinearMaxGain );

extern LEP_RESULT LEP_GetAgcLinearMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_UINT16 *agcLinearMidPointPtr );
extern LEP_RESULT LEP_SetAgcLinearMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_UINT16 agcLinearMidPoint );

extern LEP_RESULT LEP_GetAgcLinearDampeningFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_UINT16 *agcLinearDampeningFactorPtr );
extern LEP_RESULT LEP_SetAgcLinearDampeningFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                   LEP_UINT16 agcLinearDampeningFactor );

/* Heq AGC Policy Controls
*/
extern LEP_RESULT LEP_GetAgcHeqDampingFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 *agcHeqDampingFactorPtr );
extern LEP_RESULT LEP_SetAgcHeqDampingFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 agcHeqDampingFactor );

extern LEP_RESULT LEP_GetAgcHeqMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_UINT16 *agcHeqMaxGainPtr );
extern LEP_RESULT LEP_SetAgcHeqMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_UINT16 agcHeqMaxGain );

extern LEP_RESULT LEP_GetAgcHeqClipLimitHigh( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 *agcHeqClipLimitHighPtr );
extern LEP_RESULT LEP_SetAgcHeqClipLimitHigh( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 agcHeqClipLimitHigh );

extern LEP_RESULT LEP_GetAgcHeqClipLimitLow( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_UINT16 *agcHeqClipLimitLowPtr );
extern LEP_RESULT LEP_SetAgcHeqClipLimitLow( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_UINT16 agcHeqClipLimitLow );

extern LEP_RESULT LEP_GetAgcHeqBinExtension( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_UINT16 *agcHeqBinExtensionPtr );
extern LEP_RESULT LEP_SetAgcHeqBinExtension( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_UINT16 agcHeqBinExtension );

extern LEP_RESULT LEP_GetAgcHeqMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_UINT16 *agcHeqMidPointPtr );
extern LEP_RESULT LEP_SetAgcHeqMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_UINT16 agcHeqMidPoint );

extern LEP_RESULT LEP_GetAgcHeqEmptyCount( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_AGC_HEQ_EMPTY_COUNT_T_PTR emptyCountPtr );
extern LEP_RESULT LEP_SetAgcHeqEmptyCount( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_AGC_HEQ_EMPTY_COUNT_T emptyCount );

extern LEP_RESULT LEP_GetAgcHeqNormalizationFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_AGC_HEQ_NORMALIZATION_FACTOR_T_PTR normalizationFactorPtr );
extern LEP_RESULT LEP_SetAgcHeqNormalizationFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                    LEP_AGC_HEQ_NORMALIZATION_FACTOR_T normalizationFactor );

extern LEP_RESULT LEP_GetAgcHeqScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_AGC_HEQ_SCALE_FACTOR_E_PTR scaleFactorPtr );
extern LEP_RESULT LEP_SetAgcHeqScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_AGC_HEQ_SCALE_FACTOR_E scaleFactor );

extern LEP_RESULT LEP_GetAgcCalcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_AGC_ENABLE_E_PTR agcCalculationEnableStatePtr );

extern LEP_RESULT LEP_SetAgcCalcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_AGC_ENABLE_E agcCalculationEnableState );

extern LEP_RESULT LEP_GetAgcHeqLinearPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 *agcHeqLinearPercentPtr);

extern LEP_RESULT LEP_SetAgcHeqLinearPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 agcHeqLinearPercent);

   #ifdef __cplusplus

}
   #endif

#endif  /* _LEPTON_AGC_H_ */


/*******************************************************************************
**
**    File NAME: LEPTON_VID.c
**
**      AUTHOR:  David Dart
**
**      CREATED: 5/4/2012
**
**      DESCRIPTION: Lepton SDK Video Module Command Interface
**
**      HISTORY:  5/4/2012 DWD - Initial Draft
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
/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include "LEPTON_SDK.h"
#include "LEPTON_VID.h"

#if (USE_BORESIGHT_MEASUREMENT_FUNCTIONS == 1)
#include "math.h"
#endif

/******************************************************************************/
/** LOCAL DEFINES                                                            **/
/******************************************************************************/

/******************************************************************************/
/** LOCAL TYPE DEFINITIONS                                                   **/
/******************************************************************************/

/******************************************************************************/
/** PRIVATE DATA DECLARATIONS                                                **/
/******************************************************************************/

/******************************************************************************/
/** PRIVATE FUNCTION DECLARATIONS                                            **/
/******************************************************************************/

/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/

LEP_RESULT LEP_GetVidPolarity(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_POLARITY_E_PTR vidPolarityPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidPolarityPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's Video Polarity
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_POLARITY_SELECT,
                              (LEP_ATTRIBUTE_T_PTR)vidPolarityPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetVidPolarity(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_POLARITY_E vidPolarity)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidPolarity >= LEP_VID_END_POLARITY )
   {
      return(LEP_RANGE_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's Video Polarity
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_POLARITY_SELECT,
                              (LEP_ATTRIBUTE_T_PTR)&vidPolarity,
                              attributeWordLength);
   return( result );
}


LEP_RESULT LEP_GetVidPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_PCOLOR_LUT_E_PTR vidPcolorLutPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidPcolorLutPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's current Video LUT selection
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_LUT_SELECT,
                              (LEP_ATTRIBUTE_T_PTR)vidPcolorLutPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetVidPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_PCOLOR_LUT_E vidPcolorLut)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidPcolorLut >= LEP_VID_END_PCOLOR_LUT )
   {
      return(LEP_RANGE_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's current Video LUT selection
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_LUT_SELECT,
                              (LEP_ATTRIBUTE_T_PTR)&vidPcolorLut,
                              attributeWordLength);
   return( result );
}
LEP_RESULT LEP_GetVidLowGainPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_PCOLOR_LUT_E_PTR vidPcolorLutPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidPcolorLutPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's current Video LUT selection
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_LOW_GAIN_COLOR_LUT,
                              (LEP_ATTRIBUTE_T_PTR)vidPcolorLutPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetVidLowGainPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_PCOLOR_LUT_E vidPcolorLut)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidPcolorLut >= LEP_VID_END_PCOLOR_LUT )
   {
      return(LEP_RANGE_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's current Video LUT selection
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_LOW_GAIN_COLOR_LUT,
                              (LEP_ATTRIBUTE_T_PTR)&vidPcolorLut,
                              attributeWordLength);
   return( result );
}

LEP_RESULT LEP_GetVidUserLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_VID_LUT_BUFFER_T_PTR vidUserLutBufPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 512; /* 512 16-bit values for 1024 byte LUT*/

   /* Validate Parameter(s)
   */
   if( vidUserLutBufPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's User Video LUT
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_LUT_TRANSFER,
                              (LEP_ATTRIBUTE_T_PTR)vidUserLutBufPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetVidUserLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_VID_LUT_BUFFER_T_PTR vidUserLutBufPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 512; /* 512 16-bit values for 1024 byte LUT*/

   /* Validate Parameter(s)
   */
   if( vidUserLutBufPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's User Video LUT
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_LUT_TRANSFER,
                              (LEP_ATTRIBUTE_T_PTR)vidUserLutBufPtr,
                              attributeWordLength);
   return( result );
}


LEP_RESULT LEP_GetVidFocusCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_VID_FOCUS_CALC_ENABLE_E_PTR vidEnableFocusCalcStatePtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidEnableFocusCalcStatePtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's Video Focus Metric calculation enable
   ** state
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_CALC_ENABLE,
                              (LEP_ATTRIBUTE_T_PTR)vidEnableFocusCalcStatePtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetVidFocusCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_VID_FOCUS_CALC_ENABLE_E vidFocusCalcEnableState)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidFocusCalcEnableState >= LEP_VID_END_FOCUS_CALC_ENABLE )
   {
      return(LEP_RANGE_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's Video Focus Metric Calculation enable
   ** state
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_CALC_ENABLE,
                              (LEP_ATTRIBUTE_T_PTR)&vidFocusCalcEnableState,
                              attributeWordLength);
   return( result );
}

LEP_RESULT LEP_GetVidBoresightCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E_PTR boresightCalcEnableStatePtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* Enums are 32-bit */

   if(boresightCalcEnableStatePtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute( portDescPtr,
                           (LEP_COMMAND_ID)LEP_CID_VID_BORESIGHT_CALC_ENABLE,
                           (LEP_ATTRIBUTE_T_PTR)boresightCalcEnableStatePtr,
                           attributeWordLength );


   return(result);
}

LEP_RESULT LEP_SetVidBoresightCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E boresightCalcEnableState)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* Enums are 32-bit */

   if(boresightCalcEnableState >= LEP_VID_END_BORESIGHT_CALC_ENABLE_STATE)
   {
      return(LEP_RANGE_ERROR);
   }

   result = LEP_SetAttribute( portDescPtr,
                           (LEP_COMMAND_ID)LEP_CID_VID_BORESIGHT_CALC_ENABLE,
                           (LEP_ATTRIBUTE_T_PTR)&boresightCalcEnableState,
                           attributeWordLength);

   return(result);
}
LEP_RESULT LEP_GetVidBoresightCoordinates(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_VID_BORESIGHT_COORDINATES_T_PTR boresightCoordinatesPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;

   if( boresightCoordinatesPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute( portDescPtr,
                        (LEP_COMMAND_ID)LEP_CID_VID_BORESIGHT_COORDINATES,
                        (LEP_ATTRIBUTE_T_PTR)boresightCoordinatesPtr,
                        attributeWordLength );

   return(result);
}

LEP_RESULT LEP_GetVidTargetPosition(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_VID_TARGET_POSITION_T_PTR targetPositionPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_VID_BORESIGHT_COORDINATES_T boresightCoordinates;
   //LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E calcEnableState;

   if(targetPositionPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }
#if (USE_BORESIGHT_MEASUREMENT_FUNCTIONS == 1)
   result = LEP_GetVidBoresightCoordinates(portDescPtr, &boresightCoordinates);
   if(result == LEP_OK)
   {
      LEP_CalcVidBoresightAlignment(boresightCoordinates, targetPositionPtr);
   }
#else
   return(LEP_FUNCTION_NOT_SUPPORTED);
#endif

   return(result);
}


LEP_RESULT LEP_GetVidROI(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                         LEP_VID_FOCUS_ROI_T_PTR vidROIPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* four 16-bit values */

   /* Validate Parameter(s)
   */
   if( vidROIPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's AGC ROI
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_ROI,
                              (LEP_ATTRIBUTE_T_PTR)vidROIPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetVidROI(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                         LEP_VID_FOCUS_ROI_T vidROI)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* four 16-bit values */

   /* Perform Command
   ** Writing the Camera's AGC ROI
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_ROI,
                              (LEP_ATTRIBUTE_T_PTR)&vidROI,
                              attributeWordLength);
   return( result );
}


LEP_RESULT LEP_GetVidFocusMetric(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_VID_FOCUS_METRIC_T_PTR vidFocusMetricPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values */

   /* Validate Parameter(s)
   */
   if( vidFocusMetricPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's current video Focus Metric calculation
   ** result
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_METRIC,
                              (LEP_ATTRIBUTE_T_PTR)vidFocusMetricPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_GetVidFocusMetricThreshold(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_VID_FOCUS_METRIC_THRESHOLD_T_PTR vidFocusMetricThresholdPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values */

   /* Validate Parameter(s)
   */
   if( vidFocusMetricThresholdPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's current video Focus Metric threshold
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_THRESHOLD,
                              (LEP_ATTRIBUTE_T_PTR)vidFocusMetricThresholdPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetVidFocusMetricThreshold(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_VID_FOCUS_METRIC_THRESHOLD_T vidFocusMetricThreshold)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit value*/

   /* Validate Parameter(s)
   */

   /* Perform Command
   ** Writing the Camera's current video Focus Metric threshold
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_THRESHOLD,
                              (LEP_ATTRIBUTE_T_PTR)&vidFocusMetricThreshold,
                              attributeWordLength);
   return( result );
}


LEP_RESULT LEP_GetVidSbNucEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_VID_SBNUC_ENABLE_E_PTR vidSbNucEnableStatePtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidSbNucEnableStatePtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's current Scene-Based NUC enable state
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_SBNUC_ENABLE,
                              (LEP_ATTRIBUTE_T_PTR)vidSbNucEnableStatePtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetVidSbNucEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_VID_SBNUC_ENABLE_E vidSbNucEnableState)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidSbNucEnableState >= LEP_VID_END_SBNUC_ENABLE )
   {
      return(LEP_RANGE_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's current Scene-Based NUC enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_SBNUC_ENABLE,
                              (LEP_ATTRIBUTE_T_PTR)&vidSbNucEnableState,
                              attributeWordLength);
   return( result );
}


LEP_RESULT LEP_GetVidFreezeEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_VID_FREEZE_ENABLE_E_PTR vidFreezeEnableStatePtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidFreezeEnableStatePtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's current video freeze enable state
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FREEZE_ENABLE,
                              (LEP_ATTRIBUTE_T_PTR)vidFreezeEnableStatePtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetVidFreezeEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_VID_FREEZE_ENABLE_E vidFreezeEnableState)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidFreezeEnableState >= LEP_VID_END_FREEZE_ENABLE )
   {
      return(LEP_RANGE_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's current video freeze enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FREEZE_ENABLE,
                              (LEP_ATTRIBUTE_T_PTR)&vidFreezeEnableState,
                              attributeWordLength);
   return( result );
}

LEP_RESULT LEP_GetVidVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_VID_VIDEO_OUTPUT_FORMAT_E_PTR vidVideoOutputFormatPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidVideoOutputFormatPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current video output format
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_VID_VIDEO_OUTPUT_FORMAT,
                              ( LEP_ATTRIBUTE_T_PTR )vidVideoOutputFormatPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetVidVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_VID_VIDEO_OUTPUT_FORMAT_E vidVideoOutputFormat )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidVideoOutputFormat >= LEP_END_VID_VIDEO_OUTPUT_FORMAT )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output format
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_VID_VIDEO_OUTPUT_FORMAT,
                              ( LEP_ATTRIBUTE_T_PTR ) & vidVideoOutputFormat,
                              attributeWordLength );
   return( result );
}


/******************************************************************************/
/** PRIVATE MODULE FUNCTIONS                                                 **/
/******************************************************************************/

#if (USE_BORESIGHT_MEASUREMENT_FUNCTIONS == 1)

LEP_RESULT LEP_CalcVidBoresightAlignment(LEP_VID_BORESIGHT_COORDINATES_T boresightCoordinates, LEP_VID_TARGET_POSITION_T_PTR targetPositionPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_FLOAT32 top, bot, left, right, rows, cols;

   top = (boresightCoordinates.top_0.col + boresightCoordinates.top_1.col) / 2.0;
   bot = (boresightCoordinates.bottom_0.col + boresightCoordinates.bottom_1.col) / 2.0;
   left = (boresightCoordinates.left_0.row + boresightCoordinates.left_1.row) / 2.0;
   right = (boresightCoordinates.right_0.row + boresightCoordinates.right_1.row) / 2.0;

   rows = boresightCoordinates.bottom_1.row - boresightCoordinates.top_0.row + 1.0;
   cols = boresightCoordinates.right_1.col - boresightCoordinates.left_0.col + 1.0;

   /* Both lines are perfect horizontal/vertical */
   if((left == right) && (top == bot))
   {
      targetPositionPtr->col = top;
      targetPositionPtr->row = left;
      targetPositionPtr->rotation = 0.0;
   }
   /* Line 1 is perfect horizontal and Line 2 has slope */
   else if ((left == right) && (top != bot))
   {
      targetPositionPtr->row = left;
      targetPositionPtr->col = (LEP_FLOAT32)(bot + ((top - bot) / rows) * left);
      targetPositionPtr->rotation = (LEP_FLOAT32)((57.2957802 * atanf((top - bot) / rows)) / 2.0);
   }
   /* Line 1 has slope and Line 2 is perfect vertical */
   else if ((left != right) && (top == bot))
   {
      targetPositionPtr->col = top;
      targetPositionPtr->row = (LEP_FLOAT32)(right + (right - left) / cols * top / cols);
      targetPositionPtr->rotation = (LEP_FLOAT32)((57.2957802 * atanf((right - left) / cols)) / 2.0);
   }
   /* Both lines have slope */
   else
   {
      targetPositionPtr->col = (LEP_FLOAT32)(((rows - rows * top / (top - bot)) - left) / ((right - left) / cols - rows / (top - bot)));
      targetPositionPtr->row = (LEP_FLOAT32)(left + targetPositionPtr->col * (right - left) / cols);
      targetPositionPtr->rotation = (LEP_FLOAT32)((57.2957802 * (atanf((right - left) / cols) + atanf((top - bot) / rows))) / 2.0);
   }

   return(result);
}
#endif

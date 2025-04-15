/*******************************************************************************
**
**    File NAME: LEPTON_AGC.c
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
/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include "LEPTON_SDK.h"
#include "LEPTON_AGC.h"

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

/**************************************/

/**
 * Retrieves the current AGC Enable state from the Camera. The
 * AGC enable state turns AGC On (enabled) or OFF (disabled).
 *
 * @param agcEnableStatePtr
 *               Pointer to variable to update with the camera's
 *               current state.
 *               Range:
 *                  LEP_AGC_DISABLE=0
 *                  LEP_AGC_ENABLE =1
 *
 * @return LEP_OK if all goes well, otherwise an error code.
 */
LEP_RESULT LEP_GetAgcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_AGC_ENABLE_E_PTR agcEnableStatePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* attribute is an enum, so 32-bit value */

   /* Validate Parameter(s)
   */
   if( agcEnableStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Read the Camera's AGC Enable State
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR )agcEnableStatePtr,
                              attributeWordLength );
   return( result );
}

/**
 * Sets the current AGC Enable state on the Camera.  The
 * AGC enable state turns AGC On (enabled) or OFF (disabled).
 *
 * @param agcEnableState
 *             Specifies the enable state to set the camera' AGC
 *             Range:
 *                LEP_AGC_DISABLE=0
 *                LEP_AGC_ENABLE =1
 *
 * @return LEP_OK if all goes well, otherwise an error code.
 */
LEP_RESULT LEP_SetAgcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_AGC_ENABLE_E agcEnableState )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* 2 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcEnableState >= LEP_END_AGC_ENABLE )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's AGC Enable State
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcEnableState,
                              attributeWordLength );
   return( result );
}




LEP_RESULT LEP_GetAgcPolicy( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_AGC_POLICY_E_PTR agcPolicyPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* 2 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcPolicyPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Policy
   */
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_POLICY,
                              ( LEP_ATTRIBUTE_T_PTR )agcPolicyPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetAgcPolicy( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_AGC_POLICY_E agcPolicy )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* 2 16-bit value */

   /* Bounds Check
   */
   if( agcPolicy >= LEP_END_AGC_POLICY )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's AGC Policy
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_POLICY,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcPolicy,
                              attributeWordLength );
   return( result );
}



LEP_RESULT LEP_GetAgcROI( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                          LEP_AGC_ROI_T_PTR agcROIPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* four 16-bit values */

   /* Validate Parameter(s)
   */
   if( agcROIPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC ROI
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_ROI,
                              ( LEP_ATTRIBUTE_T_PTR )agcROIPtr,
                              attributeWordLength );
   //return(sizeof(*agcROIPtr));
   return( result );
}


LEP_RESULT LEP_SetAgcROI( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                          LEP_AGC_ROI_T agcROI )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* four 16-bit values */

   /* Perform Command
   ** Writing the Camera's AGC ROI
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_ROI,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcROI,
                              attributeWordLength );


   return( result );
}

LEP_RESULT LEP_GetAgcLinearHistogramClipPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_UINT16 *agcLinearHistogramClipPercentPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcLinearHistogramClipPercentPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Linear Histogram Clip Percent
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HISTOGRAM_CLIP_PERCENT,
                              ( LEP_ATTRIBUTE_T_PTR )agcLinearHistogramClipPercentPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetAgcLinearHistogramClipPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_UINT16 agcLinearHistogramClipPercent )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC Linear Histogram Clip Percent
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HISTOGRAM_CLIP_PERCENT,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcLinearHistogramClipPercent,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LEP_GetAgcLinearHistogramTailSize( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 *agcLinearHistogramTailSizePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcLinearHistogramTailSizePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Linear Histogram Tail Size
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HISTOGRAM_TAIL_SIZE,
                              ( LEP_ATTRIBUTE_T_PTR )agcLinearHistogramTailSizePtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetAgcLinearHistogramTailSize( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 agcLinearHistogramTailSize )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC Linear Histogram Clip Percent
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HISTOGRAM_TAIL_SIZE,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcLinearHistogramTailSize,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LEP_GetAgcHistogramStatistics( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_AGC_HISTOGRAM_STATISTICS_T_PTR *agcHistogramStatisticsPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* four 16-bit values */

   /* Validate Parameter(s)
   */
   if( agcHistogramStatisticsPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Linear Histogram Statistics
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_STATISTICS,
                              ( LEP_ATTRIBUTE_T_PTR )agcHistogramStatisticsPtr,
                              attributeWordLength );
   return( result );
}



/* Linear Policy Controls
*/

LEP_RESULT LEP_GetAgcLinearMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_UINT16 *agcLinearMaxGainPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcLinearMaxGainPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Linear Max Gain
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_LINEAR_MAX_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR )agcLinearMaxGainPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetAgcLinearMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_UINT16 agcLinearMaxGain )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC Linear Max Gain
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_LINEAR_MAX_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcLinearMaxGain,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LEP_GetAgcLinearMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_UINT16 *agcLinearMidPointPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcLinearMidPointPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Linear Midpoint
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_LINEAR_MIDPOINT,
                              ( LEP_ATTRIBUTE_T_PTR )agcLinearMidPointPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetAgcLinearMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_UINT16 agcLinearMidPoint )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC Linear Midpoint
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_LINEAR_MIDPOINT,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcLinearMidPoint,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LEP_GetAgcLinearDampeningFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_UINT16 *agcLinearDampeningFactorPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcLinearDampeningFactorPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Linear Dampening Factor
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_LINEAR_DAMPENING_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )agcLinearDampeningFactorPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetAgcLinearDampeningFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_UINT16 agcLinearDampeningFactor )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC Linear Histogram Dampening Factor
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_LINEAR_DAMPENING_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcLinearDampeningFactor,
                              attributeWordLength
                              );
   return( result );
}



/* Heq Policy Controls
*/

LEP_RESULT LEP_GetAgcHeqDampingFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 *agcHeqDampingFactorPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcHeqDampingFactorPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC HEQ Dampening Factor
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_DAMPENING_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqDampingFactorPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetAgcHeqDampingFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 agcHeqDampingFactor )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC HEQ Histogram Dampening Factor
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_DAMPENING_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcHeqDampingFactor,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LEP_GetAgcHeqMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_UINT16 *agcHeqMaxGainPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcHeqMaxGainPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC HEQ Max Gain
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_MAX_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqMaxGainPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetAgcHeqMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_UINT16 agcHeqMaxGain )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC HEQ Max Gain
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_MAX_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcHeqMaxGain,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LEP_GetAgcHeqClipLimitHigh( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 *agcHeqClipLimitHighPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcHeqClipLimitHighPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC HEQ Clip Limit High
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_CLIP_LIMIT_HIGH,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqClipLimitHighPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetAgcHeqClipLimitHigh( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 agcHeqClipLimitHigh )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC HEQ Clip Limit High
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_CLIP_LIMIT_HIGH,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcHeqClipLimitHigh,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LEP_GetAgcHeqClipLimitLow( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_UINT16 *agcHeqClipLimitLowPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcHeqClipLimitLowPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC HEQ Clip Limit Low
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_CLIP_LIMIT_LOW,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqClipLimitLowPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetAgcHeqClipLimitLow( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_UINT16 agcHeqClipLimitLow )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC HEQ Clip Limit Low
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_CLIP_LIMIT_LOW,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcHeqClipLimitLow,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LEP_GetAgcHeqBinExtension( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_UINT16 *agcHeqBinExtensionPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcHeqBinExtensionPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC HEQ Bin Extension
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_BIN_EXTENSION,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqBinExtensionPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetAgcHeqBinExtension( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_UINT16 agcHeqBinExtension )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC HEQ Bin Extension
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_BIN_EXTENSION,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcHeqBinExtension,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LEP_GetAgcHeqMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_UINT16 *agcHeqMidPointPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcHeqMidPointPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC HEQ Midpoint
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_MIDPOINT,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqMidPointPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetAgcHeqMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_UINT16 agcHeqMidPoint )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC HEQ Midpoint
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_MIDPOINT,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcHeqMidPoint,
                              attributeWordLength
                              );
   return( result );
}

LEP_RESULT LEP_GetAgcHeqEmptyCount( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_AGC_HEQ_EMPTY_COUNT_T_PTR emptyCountPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_EMPTY_COUNTS,
                              ( LEP_ATTRIBUTE_T_PTR )emptyCountPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LEP_SetAgcHeqEmptyCount( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_AGC_HEQ_EMPTY_COUNT_T emptyCount )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_EMPTY_COUNTS,
                              ( LEP_ATTRIBUTE_T_PTR ) & emptyCount,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetAgcHeqNormalizationFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_AGC_HEQ_NORMALIZATION_FACTOR_T_PTR normalizationFactorPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_NORMALIZATION_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )normalizationFactorPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LEP_SetAgcHeqNormalizationFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_AGC_HEQ_NORMALIZATION_FACTOR_T normalizationFactor )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_NORMALIZATION_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR ) & normalizationFactor,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetAgcHeqScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_AGC_HEQ_SCALE_FACTOR_E_PTR scaleFactorPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_SCALE_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )scaleFactorPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LEP_SetAgcHeqScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_AGC_HEQ_SCALE_FACTOR_E scaleFactor )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_SCALE_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR ) & scaleFactor,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetAgcCalcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_AGC_ENABLE_E_PTR agcCalculationEnableStatePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if( agcCalculationEnableStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_CALC_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR )agcCalculationEnableStatePtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LEP_SetAgcCalcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_AGC_ENABLE_E agcCalculationEnableState )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if( agcCalculationEnableState >= LEP_END_AGC_ENABLE )
   {
      return( LEP_RANGE_ERROR );
   }

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_CALC_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcCalculationEnableState,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetAgcHeqLinearPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 *agcHeqLinearPercentPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   if( agcHeqLinearPercentPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_LINEAR_PERCENT,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqLinearPercentPtr,
                              attributeWordLength );

   return(result);
}

LEP_RESULT LEP_SetAgcHeqLinearPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 agcHeqLinearPercent)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_LINEAR_PERCENT,
                              ( LEP_ATTRIBUTE_T_PTR )&agcHeqLinearPercent,
                              attributeWordLength );

   return(result);
}

/******************************************************************************/
/** PRIVATE MODULE FUNCTIONS                                                 **/
/******************************************************************************/


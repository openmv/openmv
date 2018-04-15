/*******************************************************************************
*
*    FILE: LEPTON_RAD.c
*
*       DESCRIPTION:
*
*       AUTHOR: dart
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

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/

#include "LEPTON_SDK.h"
#include "LEPTON_RAD.h"

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

LEP_RESULT LEP_GetRadTShutterMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_RAD_TS_MODE_E_PTR radTShutterModePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( radTShutterModePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current TShutter mode
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TSHUTTER_MODE,
                              ( LEP_ATTRIBUTE_T_PTR )radTShutterModePtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetRadTShutterMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_RAD_TS_MODE_E radTShutterMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( radTShutterMode >= LEP_RAD_TS_END_TS_MODE )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current TShutter mode
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TSHUTTER_MODE,
                              ( LEP_ATTRIBUTE_T_PTR ) & radTShutterMode,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_GetRadTShutter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_KELVIN_T_PTR radTShutterPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radTShutterPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TSHUTTER,
                              ( LEP_ATTRIBUTE_T_PTR )radTShutterPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetRadTShutter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_KELVIN_T radTShutter )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TSHUTTER,
                              ( LEP_ATTRIBUTE_T_PTR ) & radTShutter,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_RunRadFFC( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_RAD_STATUS_E radStatus = LEP_RAD_STATUS_BUSY;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_RAD_RUN_FFC );
   if( result == LEP_OK )
   {
      //TODO: Add timeout check
      while( radStatus == LEP_RAD_STATUS_BUSY )
      {
         LEP_GetRadRunStatus( portDescPtr, &radStatus );
      }
   }
   return( result );
}


LEP_RESULT LEP_GetRadRBFOInternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_INTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadRBFOInternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_INTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadRBFOExternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_EXTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadRBFOExternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 4 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_EXTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadInternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_INTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadInternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_INTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadExternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_EXTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadExternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 4 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_EXTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadInternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_INTERNAL_LG,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadInternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_INTERNAL_LG,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadExternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_EXTERNAL_LG,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadExternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 4 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_EXTERNAL_LG,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetRadResponsivityShift( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_RS_T_PTR radResponsivityShiftPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radResponsivityShiftPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RESPONSIVITY_SHIFT,
                              ( LEP_ATTRIBUTE_T_PTR )radResponsivityShiftPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadResponsivityShift( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_RS_T radResponsivityShift )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RESPONSIVITY_SHIFT,
                              ( LEP_ATTRIBUTE_T_PTR ) & radResponsivityShift,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadFNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_FNUMBER_T_PTR radFNumberPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radFNumberPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_F_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )radFNumberPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadFNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_FNUMBER_T radFNumber )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_F_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR ) & radFNumber,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadTauLens( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_TAULENS_T_PTR radTauLensPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radTauLensPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAU_LENS,
                              ( LEP_ATTRIBUTE_T_PTR )radTauLensPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadTauLens( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_TAULENS_T radTauLens )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAU_LENS,
                              ( LEP_ATTRIBUTE_T_PTR ) & radTauLens,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadRadometryFilter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_RADIOMETRY_FILTER_T_PTR radRadiometryFilterPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radRadiometryFilterPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RADIOMETRY_FILTER,
                              ( LEP_ATTRIBUTE_T_PTR )radRadiometryFilterPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadRadometryFilter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_RADIOMETRY_FILTER_T radRadiometryFilter )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RADIOMETRY_FILTER,
                              ( LEP_ATTRIBUTE_T_PTR ) & radRadiometryFilter,
                              attributeWordLength );

   return( result );
}

/* Deprecated: Use LEP_GetRadTFpaLut */
LEP_RESULT LEP_GetRadTFpaCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_LUT256_T_PTR radTFpaCLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTFpaCLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTFpaCLutPtr,
                              attributeWordLength );

   return( result );
}

/* Deprecated: Use LEP_SetRadTFpaLut */
LEP_RESULT LEP_SetRadTFpaCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_LUT256_T_PTR radTFpaCLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTFpaCLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTFpaCLutPtr,
                              attributeWordLength );

   return( result );
}

/* Deprecated: Use LEP_GetRadTAuxLut */
LEP_RESULT LEP_GetRadTAuxCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_LUT256_T_PTR radTAuxCLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTAuxCLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTAuxCLutPtr,
                              attributeWordLength );

   return( result );
}

/* Deprecated: Use LEP_SetRadTAuxLut */
LEP_RESULT LEP_SetRadTAuxCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_LUT256_T_PTR radTAuxCLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTAuxCLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTAuxCLutPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetRadTFpaLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_LUT256_T_PTR radTFpaLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTFpaLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTFpaLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadTFpaLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_LUT256_T_PTR radTFpaLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTFpaLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTFpaLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadTAuxLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_LUT256_T_PTR radTAuxLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTAuxLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTAuxLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadTAuxLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_LUT256_T_PTR radTAuxLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTAuxLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTAuxLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadResponsivityValueLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_LUT128_T_PTR radResponsivityValueLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 128;    // 128 16-bit word LUT

   if( radResponsivityValueLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RESPONSIVITY_VALUE_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radResponsivityValueLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadResponsivityValueLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_LUT128_T_PTR radResponsivityValueLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 128;    // 128 16-bit word LUT

   if( radResponsivityValueLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RESPONSIVITY_VALUE_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radResponsivityValueLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadDebugTemp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_RAD_KELVIN_T_PTR radDebugTempPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radDebugTempPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_DEBUG_TEMP,
                              ( LEP_ATTRIBUTE_T_PTR )radDebugTempPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadDebugTemp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_RAD_KELVIN_T radDebugTemp )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_DEBUG_TEMP,
                              ( LEP_ATTRIBUTE_T_PTR ) & radDebugTemp,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadDebugFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_RAD_FLUX_T_PTR radDebugFluxPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words

   if( radDebugFluxPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_DEBUG_FLUX,
                              ( LEP_ATTRIBUTE_T_PTR )radDebugFluxPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadDebugFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_RAD_FLUX_T radDebugFlux )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_DEBUG_FLUX,
                              ( LEP_ATTRIBUTE_T_PTR ) & radDebugFlux,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_ENABLE_E_PTR radEnableStatePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words for an enum

   if( radEnableStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR )radEnableStatePtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_ENABLE_E radEnableState )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words for an enum

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR ) & radEnableState,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadGlobalGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_GLOBAL_GAIN_T_PTR radGlobalGainPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radGlobalGainPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_GLOBAL_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR )radGlobalGainPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadGlobalGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_GLOBAL_GAIN_T radGlobalGain )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_GLOBAL_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR ) & radGlobalGain,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadGlobalOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_RAD_GLOBAL_OFFSET_T_PTR radGlobalOffsetPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radGlobalOffsetPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_GLOBAL_OFFSET,
                              ( LEP_ATTRIBUTE_T_PTR )radGlobalOffsetPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadGlobalOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_RAD_GLOBAL_OFFSET_T radGlobalOffset )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_GLOBAL_OFFSET,
                              ( LEP_ATTRIBUTE_T_PTR ) & radGlobalOffset,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadTFpaCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_TEMPERATURE_UPDATE_E_PTR radTFpaCtsModePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words for an enum

   if( radTFpaCtsModePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_CTS_MODE,
                              ( LEP_ATTRIBUTE_T_PTR )radTFpaCtsModePtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadTFpaCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_TEMPERATURE_UPDATE_E radTFpaCtsMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words for an enum

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_CTS_MODE,
                              ( LEP_ATTRIBUTE_T_PTR ) & radTFpaCtsMode,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadTAuxCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_TEMPERATURE_UPDATE_E_PTR radTAuxCtsModePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words for an enum

   if( radTAuxCtsModePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_CTS_MODE,
                              ( LEP_ATTRIBUTE_T_PTR )radTAuxCtsModePtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadTAuxCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_TEMPERATURE_UPDATE_E radTAuxCtsMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words for an enum

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_CTS_MODE,
                              ( LEP_ATTRIBUTE_T_PTR ) & radTAuxCtsMode,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadTFpaCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_TEMPERATURE_COUNTS_T_PTR radTFpaCtsPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radTFpaCtsPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_CTS,
                              ( LEP_ATTRIBUTE_T_PTR )radTFpaCtsPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadTFpaCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_TEMPERATURE_COUNTS_T radTFpaCts )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_CTS,
                              ( LEP_ATTRIBUTE_T_PTR ) & radTFpaCts,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadTAuxCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_TEMPERATURE_COUNTS_T_PTR radTAuxCtsPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radTAuxCtsPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_CTS,
                              ( LEP_ATTRIBUTE_T_PTR )radTAuxCtsPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadTAuxCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_TEMPERATURE_COUNTS_T radTAuxCts )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_CTS,
                              ( LEP_ATTRIBUTE_T_PTR ) & radTAuxCts,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadTEqShutterLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RAD_LUT128_T_PTR radTEqShutterLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 128;    // 128 16-bit word LUT

   if( radTEqShutterLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TEQ_SHUTTER_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTEqShutterLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadTEqShutterLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RAD_LUT128_T_PTR radTEqShutterLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 128;    // 128 16-bit word LUT

   if( radTEqShutterLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TEQ_SHUTTER_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTEqShutterLutPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetRadRunStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_RAD_STATUS_E_PTR radStatusPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   if( radStatusPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RUN_STATUS,
                              ( LEP_ATTRIBUTE_T_PTR )radStatusPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadTEqShutterFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_FLUX_T_PTR radTEqShutterFluxPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // single 32-bit word

   if( radTEqShutterFluxPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TEQ_SHUTTER_FLUX,
                              ( LEP_ATTRIBUTE_T_PTR )radTEqShutterFluxPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadTEqShutterFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_FLUX_T radTEqShutterFlux )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // single 32-bit word

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TEQ_SHUTTER_FLUX,
                              ( LEP_ATTRIBUTE_T_PTR ) & radTEqShutterFlux,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadMffcFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_FLUX_T_PTR radRadMffcFluxPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // single 32-bit word

   if( radRadMffcFluxPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_MFFC_FLUX,
                              ( LEP_ATTRIBUTE_T_PTR )radRadMffcFluxPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadMffcFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_FLUX_T radRadMffcFlux )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // single 32-bit word

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_MFFC_FLUX,
                              ( LEP_ATTRIBUTE_T_PTR ) & radRadMffcFlux,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetRadFrameMedianPixelValue( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_MEDIAN_VALUE_T_PTR frameMedianPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit word */

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_FRAME_MEDIAN_VALUE,
                              ( LEP_ATTRIBUTE_T_PTR )frameMedianPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadMLGLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_RAD_SIGNED_LUT128_T_PTR radMLGLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 128;    // 256 16-bit word LUT

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_MLG_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radMLGLutPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_SetRadMLGLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_RAD_SIGNED_LUT128_T_PTR radMLGLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 128;    // 256 16-bit word LUT

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_MLG_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radMLGLutPtr,
                              attributeWordLength );

   return( result );
}
#if USE_DEPRECATED_HOUSING_TCP_INTERFACE
LEP_RESULT LEP_GetRadTHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR radHousingTcp )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    // LEP_RAD_LINEAR_TEMP_CORRECTION_T is 4 16-bit words

   if( radHousingTcp == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_THOUSING_TCP,
                              ( LEP_ATTRIBUTE_T_PTR )radHousingTcp,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LEP_SetRadTHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_LINEAR_TEMP_CORRECTION_T radHousingTcp )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_THOUSING_TCP,
                              ( LEP_ATTRIBUTE_T_PTR ) & radHousingTcp,
                              attributeWordLength );

   return( result );
}
#else
LEP_RESULT LEP_GetRadHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR RadHousingTcpPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    // LEP_RAD_LINEAR_TEMP_CORRECTION_T is 4 16-bit words

   if( RadHousingTcpPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_HOUSING_TCP,
                              ( LEP_ATTRIBUTE_T_PTR )RadHousingTcpPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_LINEAR_TEMP_CORRECTION_T RadHousingTcp )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_HOUSING_TCP,
                              ( LEP_ATTRIBUTE_T_PTR ) & RadHousingTcp,
                              attributeWordLength );

   return( result );
}
#endif




LEP_RESULT LEP_GetRadShutterTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR RadShutterTcpPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    // LEP_RAD_LINEAR_TEMP_CORRECTION_T is 4 16-bit words

   if( RadShutterTcpPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_SHUTTER_TCP,
                              ( LEP_ATTRIBUTE_T_PTR )RadShutterTcpPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadShutterTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_LINEAR_TEMP_CORRECTION_T RadShutterTcp )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_SHUTTER_TCP,
                              ( LEP_ATTRIBUTE_T_PTR ) & RadShutterTcp,
                              attributeWordLength );

   return( result );
}




LEP_RESULT LEP_GetRadLensTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR RadLensTcpPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    // LEP_RAD_LINEAR_TEMP_CORRECTION_T is 4 16-bit words

   if( RadLensTcpPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_LENS_TCP,
                              ( LEP_ATTRIBUTE_T_PTR )RadLensTcpPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_SetRadLensTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_LINEAR_TEMP_CORRECTION_T RadLensTcp )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* 4 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_LENS_TCP,
                              ( LEP_ATTRIBUTE_T_PTR ) & RadLensTcp,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetRadPreviousGlobalOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_GLOBAL_OFFSET_T_PTR globalOffsetPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_PREVIOUS_GLOBAL_OFFSET,
                              ( LEP_ATTRIBUTE_T_PTR )globalOffsetPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetRadPreviousGlobalGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_GLOBAL_GAIN_T_PTR globalGainPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_PREVIOUS_GLOBAL_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR )globalGainPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetGlobalGainFFC( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_GLOBAL_GAIN_T_PTR globalGainFfcPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_GLOBAL_GAIN_FFC,
                              ( LEP_ATTRIBUTE_T_PTR )globalGainFfcPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetRadCnfScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR scaleFactorPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   if( scaleFactorPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_CNF_SCALE_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )scaleFactorPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LEP_GetRadTnfScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR scaleFactorPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   if( scaleFactorPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TNF_SCALE_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )scaleFactorPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LEP_GetRadSnfScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR scaleFactorPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   if( scaleFactorPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_SNF_SCALE_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )scaleFactorPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetRadArbitraryOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_ARBITRARY_OFFSET_T_PTR arbitraryOffsetPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   if( arbitraryOffsetPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_ARBITRARY_OFFSET,
                              ( LEP_ATTRIBUTE_T_PTR )arbitraryOffsetPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_SetRadArbitraryOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_ARBITRARY_OFFSET_T arbitraryOffset )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_ARBITRARY_OFFSET,
                              ( LEP_ATTRIBUTE_T_PTR ) & arbitraryOffset,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetRadFluxLinearParams(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_FLUX_LINEAR_PARAMS_T_PTR fluxParamsPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;

   if(fluxParamsPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_FLUX_LINEAR_PARAMS,
                             (LEP_ATTRIBUTE_T_PTR)fluxParamsPtr,
                             attributeWordLength);

   return(result);
}

LEP_RESULT LEP_SetRadFluxLinearParams(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_FLUX_LINEAR_PARAMS_T fluxParams)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_FLUX_LINEAR_PARAMS,
                             (LEP_ATTRIBUTE_T_PTR)&fluxParams,
                             attributeWordLength);

   return(result);
}

LEP_RESULT LEP_GetRadTLinearEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_ENABLE_E_PTR enableStatePtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if(enableStatePtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_TLINEAR_ENABLE_STATE,
                             (LEP_ATTRIBUTE_T_PTR)enableStatePtr,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LEP_SetRadTLinearEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_ENABLE_E enableState)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_TLINEAR_ENABLE_STATE,
                             (LEP_ATTRIBUTE_T_PTR)&enableState,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LEP_GetRadTLinearResolution(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_TLINEAR_RESOLUTION_E_PTR resolutionPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if(resolutionPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_TLINEAR_RESOLUTION,
                             (LEP_ATTRIBUTE_T_PTR)resolutionPtr,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LEP_SetRadTLinearResolution(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_TLINEAR_RESOLUTION_E resolution)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_TLINEAR_RESOLUTION,
                             (LEP_ATTRIBUTE_T_PTR)&resolution,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LEP_GetRadTLinearAutoResolution(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_ENABLE_E_PTR enableStatePtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if(enableStatePtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_TLINEAR_AUTO_RESOLUTION,
                             (LEP_ATTRIBUTE_T_PTR)enableStatePtr,
                             attributeWordLength);
   return(result);
}
LEP_RESULT LEP_SetRadTLinearAutoResolution(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_ENABLE_E enableState)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_TLINEAR_AUTO_RESOLUTION,
                             (LEP_ATTRIBUTE_T_PTR)&enableState,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LEP_GetRadSpotmeterRoi(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_ROI_T_PTR spotmeterRoiPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;

   if(spotmeterRoiPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_SPOTMETER_ROI,
                             (LEP_ATTRIBUTE_T_PTR)spotmeterRoiPtr,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LEP_SetRadSpotmeterRoi(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_ROI_T spotmeterRoi)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_SPOTMETER_ROI,
                             (LEP_ATTRIBUTE_T_PTR)&spotmeterRoi,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LEP_GetRadSpotmeterObjInKelvinX100(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_RAD_SPOTMETER_OBJ_KELVIN_T_PTR kelvinPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;

   if(kelvinPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_SPOTMETER_OBJ_KELVIN,
                             (LEP_ATTRIBUTE_T_PTR)kelvinPtr,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LEP_GetRadArbitraryOffsetMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_ARBITRARY_OFFSET_MODE_E_PTR arbitraryOffsetModePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if(arbitraryOffsetModePtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_ARBITRARY_OFFSET_MODE,
                             (LEP_ATTRIBUTE_T_PTR)arbitraryOffsetModePtr,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LEP_SetRadArbitraryOffsetMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_ARBITRARY_OFFSET_MODE_E arbitraryOffsetMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_ARBITRARY_OFFSET_MODE,
                             (LEP_ATTRIBUTE_T_PTR)&arbitraryOffsetMode,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LEP_GetRadArbitraryOffsetParams( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_ARBITRARY_OFFSET_PARAMS_T_PTR arbitraryOffsetParamsPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if(arbitraryOffsetParamsPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_ARBITRARY_OFFSET_PARAMS,
                             (LEP_ATTRIBUTE_T_PTR)arbitraryOffsetParamsPtr,
                             attributeWordLength);
   return(result);

}

LEP_RESULT LEP_SetRadArbitraryOffsetParams( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_ARBITRARY_OFFSET_PARAMS_T arbitraryOffsetParams)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_ARBITRARY_OFFSET_PARAMS,
                             (LEP_ATTRIBUTE_T_PTR)&arbitraryOffsetParams,
                             attributeWordLength);
   return(result);
}

extern LEP_RESULT LEP_GetRadRadioCalValues( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_RADIO_CAL_VALUES_T_PTR radRadioCalValuesPtr)
{
    LEP_RESULT result = LEP_OK;
    LEP_UINT16 attributeWordLength = 4;

    if(radRadioCalValuesPtr == NULL)
    {
       return(LEP_BAD_ARG_POINTER_ERROR);
    }

    result = LEP_GetAttribute(portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_RAD_RADIO_CAL_VALUES,
                              (LEP_ATTRIBUTE_T_PTR)radRadioCalValuesPtr,
                              attributeWordLength);
    return(result);
}

extern LEP_RESULT LEP_SetRadRadioCalValues( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_RADIO_CAL_VALUES_T radRadioCalValues )
{
    LEP_RESULT result = LEP_OK;
    LEP_UINT16 attributeWordLength = 4;

    result = LEP_SetAttribute(portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_RAD_RADIO_CAL_VALUES,
                              (LEP_ATTRIBUTE_T_PTR)&radRadioCalValues,
                              attributeWordLength);
    return(result);
}

/******************************************************************************/
/** PRIVATE MODULE FUNCTIONS                                                 **/
/******************************************************************************/


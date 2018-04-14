/*******************************************************************************
**
**    File NAME: LEPTON_SYS.c
**
**      AUTHOR:  David Dart
**
**      CREATED: 5/8/2012
**
**      DESCRIPTION: Lepton SDK System Module Command Interface
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
/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include "LEPTON_SDK.h"
#include "LEPTON_SYS.h"

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


LEP_RESULT LEP_RunSysPing( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_SYS_PING );

   return( result );
}

LEP_RESULT LEP_GetSysStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_STATUS_T_PTR sysStatusPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* 4 16-bit values */

   /* Validate Parameter(s)
   */
   if( sysStatusPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Status
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_CAM_STATUS,
                              ( LEP_ATTRIBUTE_T_PTR )sysStatusPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_GetSysFlirSerialNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_SYS_FLIR_SERIAL_NUMBER_T_PTR sysSerialNumberBufPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* 8 bytes values */

   /* Validate Parameter(s)
   */
   if( sysSerialNumberBufPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Serial Number
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_FLIR_SERIAL_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )sysSerialNumberBufPtr,
                              attributeWordLength );
   return( result );
}
#if USE_DEPRECATED_SERIAL_NUMBER_INTERFACE
LEP_RESULT LEP_GetSysCustSerialNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_SYS_CUST_SERIAL_NUMBER_T_PTR sysSerialNumberPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;   /* 32 byte string */

   if( sysSerialNumberPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_CUST_SERIAL_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )sysSerialNumberPtr,
                              attributeWordLength );

   return( result );
}
#else

LEP_RESULT LEP_GetSysCustSerialNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_SYS_CUST_SERIAL_NUMBER_T_PTR sysCustSNPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;      /*32 byte string */

   if( sysCustSNPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_CUST_SERIAL_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )sysCustSNPtr,
                              attributeWordLength );

   return( result );
}
#endif
LEP_RESULT LEP_GetSysCameraUpTime( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_SYS_UPTIME_NUMBER_T_PTR sysCameraUpTimePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values */

   /* Validate Parameter(s)
   */
   if( sysCameraUpTimePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Serial Number
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_CAM_UPTIME,
                              ( LEP_ATTRIBUTE_T_PTR )sysCameraUpTimePtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetSysAuxTemperatureCelcius( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_SYS_AUX_TEMPERATURE_CELCIUS_T_PTR auxTemperaturePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_SYS_AUX_TEMPERATURE_KELVIN_T unitsKelvin;

   /* Validate Parameter(s)
   */
   if( auxTemperaturePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetSysAuxTemperatureKelvin( portDescPtr, &unitsKelvin );
   *auxTemperaturePtr = ( LEP_SYS_AUX_TEMPERATURE_CELCIUS_T )( ( ( unitsKelvin / 100 ) + ( ( unitsKelvin % 100 ) * .01 ) ) - 273.15 );

   return( result );
}


LEP_RESULT LEP_GetSysFpaTemperatureCelcius( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_SYS_FPA_TEMPERATURE_CELCIUS_T_PTR fpaTemperaturePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_SYS_FPA_TEMPERATURE_KELVIN_T unitsKelvin;

   /* Validate Parameter(s)
   */
   if( fpaTemperaturePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetSysFpaTemperatureKelvin( portDescPtr, &unitsKelvin );
   *fpaTemperaturePtr = ( LEP_SYS_FPA_TEMPERATURE_CELCIUS_T )( ( ( unitsKelvin / 100 ) + ( ( unitsKelvin % 100 ) * .01 ) ) - 273.15 );

   return( result );
}

LEP_RESULT LEP_GetSysAuxTemperatureKelvin( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_SYS_AUX_TEMPERATURE_KELVIN_T_PTR auxTemperaturePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* one 16-bit values */

   /* Validate Parameter(s)
   */
   if( auxTemperaturePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Serial Number
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_AUX_TEMPERATURE_KELVIN,
                              ( LEP_ATTRIBUTE_T_PTR )auxTemperaturePtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetSysFpaTemperatureKelvin( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_SYS_FPA_TEMPERATURE_KELVIN_T_PTR fpaTemperaturePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* one 16-bit values */

   /* Validate Parameter(s)
   */
   if( fpaTemperaturePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Serial Number
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_FPA_TEMPERATURE_KELVIN,
                              ( LEP_ATTRIBUTE_T_PTR )fpaTemperaturePtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetSysTelemetryEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_SYS_TELEMETRY_ENABLE_STATE_E_PTR enableStatePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* enums are 32-bits */

   if( enableStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   **
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_TELEMETRY_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR )enableStatePtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_SetSysTelemetryEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_SYS_TELEMETRY_ENABLE_STATE_E enableState )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* enums are 32-bits */

   if( enableState >= LEP_END_TELEMETRY_ENABLE_STATE )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   **
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_TELEMETRY_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR ) & enableState,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetSysTelemetryLocation( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_SYS_TELEMETRY_LOCATION_E_PTR telemetryLocationPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* enums are 32-bits */

   if( telemetryLocationPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   **
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_TELEMETRY_LOCATION,
                              ( LEP_ATTRIBUTE_T_PTR )telemetryLocationPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_SetSysTelemetryLocation( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_SYS_TELEMETRY_LOCATION_E telemetryLocation )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* enums are 32-bits */

   if( telemetryLocation >= LEP_END_TELEMETRY_LOCATION )
   {
      return( LEP_RANGE_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_TELEMETRY_LOCATION,
                              ( LEP_ATTRIBUTE_T_PTR ) & telemetryLocation,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_RunFrameAverage( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result;

   /* Run the frame averaging command
   */
   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_SYS_EXECTUE_FRAME_AVERAGE );

   return( result );
}

LEP_RESULT LEP_RunSysAverageFrames( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_SYS_FRAME_AVERAGE_DIVISOR_E numFrameToAverage )
{
   LEP_RESULT  result = LEP_OK;

   /* Set the requested frames to average
   */
   result = LEP_SetSysFramesToAverage( portDescPtr, numFrameToAverage );

   if( result == LEP_OK )
   {
      /* Run the frame averaging command
      */
      result = LEP_RunFrameAverage( portDescPtr );
   }

   return( result );
}

LEP_RESULT LEP_GetSysFramesToAverage( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_FRAME_AVERAGE_DIVISOR_E_PTR numFrameToAveragePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one 32-bit value enum */

   /* Validate Parameter(s)
   */
   if( numFrameToAveragePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current number of frames to average
   ** step
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_NUM_FRAMES_TO_AVERAGE,
                              ( LEP_ATTRIBUTE_T_PTR )numFrameToAveragePtr,
                              attributeWordLength );


   return( result );
}

LEP_RESULT LEP_SetSysFramesToAverage( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_FRAME_AVERAGE_DIVISOR_E numFrameToAverage )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* enums are 32-bit */

   /* Validate Parameter(s)
   */
   if( numFrameToAverage >= LEP_SYS_END_FA_DIV )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current number of frames to average
   ** step
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_NUM_FRAMES_TO_AVERAGE,
                              ( LEP_ATTRIBUTE_T_PTR ) & numFrameToAverage,
                              attributeWordLength );


   return( result );
}

LEP_RESULT LEP_GetSysSceneStatistics( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_SCENE_STATISTICS_T_PTR sceneStatisticsPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    /* struct contains 4 16-bit values */

   if( sceneStatisticsPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_SCENE_STATISTICS,
                              ( LEP_ATTRIBUTE_T_PTR )sceneStatisticsPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetSysSceneRoi( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_SYS_VIDEO_ROI_T_PTR sceneRoiPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    /* roi consists of 4 16-bit values */

   if( sceneRoiPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_SCENE_ROI,
                              ( LEP_ATTRIBUTE_T_PTR )sceneRoiPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LEP_SetSysSceneRoi( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_SYS_VIDEO_ROI_T sceneRoi )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    /* roi consists of 4 16-bit values */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_SCENE_ROI,
                              ( LEP_ATTRIBUTE_T_PTR ) & sceneRoi,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetSysThermalShutdownCount( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_SYS_THERMAL_SHUTDOWN_COUNTS_T_PTR thermalCountsPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   if( thermalCountsPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_THERMAL_SHUTDOWN_COUNT,
                              ( LEP_ATTRIBUTE_T_PTR )thermalCountsPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetSysShutterPosition( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_SHUTTER_POSITION_E_PTR shutterPositionPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( shutterPositionPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_SHUTTER_POSITION,
                              ( LEP_ATTRIBUTE_T_PTR )shutterPositionPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_SetSysShutterPosition( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_SHUTTER_POSITION_E shutterPosition )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;     /* enums are 32-bit */

   if( shutterPosition >= LEP_SYS_SHUTTER_POSITION_END )
   {
      return( LEP_RANGE_ERROR );
   }

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_SHUTTER_POSITION,
                              ( LEP_ATTRIBUTE_T_PTR ) & shutterPosition,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetSysFfcShutterModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_SYS_FFC_SHUTTER_MODE_OBJ_T_PTR shutterModeObjPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;

   if( shutterModeObjPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_FFC_SHUTTER_MODE_OBJ,
                              ( LEP_ATTRIBUTE_T_PTR )shutterModeObjPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_SetSysFfcShutterModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_SYS_FFC_SHUTTER_MODE_OBJ_T shutterModeObj )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_FFC_SHUTTER_MODE_OBJ,
                              ( LEP_ATTRIBUTE_T_PTR ) & shutterModeObj,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_RunSysFFCNormalization( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_SYS_STATUS_E sysStatus = LEP_SYS_STATUS_BUSY;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )FLR_CID_SYS_RUN_FFC );
   while( sysStatus == LEP_SYS_STATUS_BUSY )
   {
      LEP_GetSysFFCStatus( portDescPtr, &sysStatus );
   }

   return( result );
}

LEP_RESULT LEP_GetSysFFCStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_SYS_STATUS_E_PTR ffcStatusPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if( ffcStatusPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_FFC_STATUS,
                              ( LEP_ATTRIBUTE_T_PTR )ffcStatusPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetSysGainMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_SYS_GAIN_MODE_E_PTR gainModePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* enums are 32-bit */

   if( gainModePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_GAIN_MODE,
                              ( LEP_ATTRIBUTE_T_PTR )gainModePtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_SetSysGainMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_SYS_GAIN_MODE_E gainMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_GAIN_MODE,
                              ( LEP_ATTRIBUTE_T_PTR ) & gainMode,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LEP_GetSysGainModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_SYS_GAIN_MODE_OBJ_T_PTR gainModeObjPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 14;

   if( gainModeObjPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_GAIN_MODE_OBJ,
                              ( LEP_ATTRIBUTE_T_PTR )gainModeObjPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_SetSysGainModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_SYS_GAIN_MODE_OBJ_T gainModeObj )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 14;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_GAIN_MODE_OBJ,
                              ( LEP_ATTRIBUTE_T_PTR ) & gainModeObj,
                              attributeWordLength );

   return( result );
}



LEP_RESULT LEP_GetSysFFCStates( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_SYS_FFC_STATES_E_PTR ffcStatePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if( ffcStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_FFC_STATE,
                              ( LEP_ATTRIBUTE_T_PTR )ffcStatePtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetSysBoresightValues( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_BORESIGHT_VALUES_T_PTR boresightValuesPtr)
{
    LEP_RESULT  result = LEP_OK;
    LEP_UINT16 attributeWordLength = 6;

    if( boresightValuesPtr == NULL )
    {
       return( LEP_BAD_ARG_POINTER_ERROR );
    }

    result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_BORESIGHT_VALUES,
                              ( LEP_ATTRIBUTE_T_PTR )boresightValuesPtr,
                              attributeWordLength );

    return( result );
}


/******************************************************************************/
/** PRIVATE MODULE FUNCTIONS                                                 **/
/******************************************************************************/



/*******************************************************************************
**
**    File NAME: LEPTON_OEM.c
**
**      AUTHOR:  David Dart
**
**      CREATED: 8/6/2012
**
**      DESCRIPTION: Contains OEM Interfaces
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
/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include "LEPTON_SDK.h"
#include "LEPTON_OEM.h"

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
/**
 * Power Down the Camera by asserting the POWERDOWN condition.
 *
 * @param portDescPtr
 *
 * @return
 */
LEP_RESULT LEP_RunOemPowerDown( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_POWER_DOWN );

   return( result );
}

LEP_RESULT LEP_RunOemPowerOn( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;

   result = LEP_DirectWriteRegister( portDescPtr, 0x0, 0x0 );

   return( result );
}

/**
 * Places the Camera into the Stand By condition.
 *
 * @param portDescPtr
 *
 * @return
 */
LEP_RESULT LEP_RunOemStandby( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result = LEP_OK;


   //result= LEP_RunCommand( portDescPtr, (LEP_COMMAND_ID)LEP_CID_OEM_STANDBY );

   return( result );
}

LEP_RESULT LEP_RunOemReboot( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_REBOOT );

   return( result );
}

LEP_RESULT LEP_RunOemLowPowerMode1( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_LOW_POWER_MODE_1 );

   return( result );
}

LEP_RESULT LEP_RunOemLowPowerMode2( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_LOW_POWER_MODE_2 );

   return( result );
}

LEP_RESULT LEP_RunOemBit( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_BIT_TEST );

   return( result );
}

LEP_RESULT LEP_GetOemMaskRevision( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_OEM_MASK_REVISION_T_PTR oemMaskRevisionPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* one 16-bit value */

   /* Validate Parameter(s)
   */
   if( oemMaskRevisionPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Chip Mask Revision
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_MASK_REVISION,
                              ( LEP_ATTRIBUTE_T_PTR )oemMaskRevisionPtr,
                              attributeWordLength );
   return( result );
}
#if 0
LEP_RESULT LEP_GetOemMasterID( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_OEM_MASTER_ID_T_PTR oemMasterIDPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 48; /* 96 bytes  */

   /* Validate Parameter(s)
   */
   if( oemMasterIDPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Chip Master ID
   */
   result = LEP_GetAttribute( portDescPtr,
                             ( LEP_COMMAND_ID )LEP_CID_OEM_MASTER_ID,
                             ( LEP_ATTRIBUTE_T_PTR )oemMasterIDPtr,
                             attributeWordLength );
   return( result );
}
#endif

#if USE_DEPRECATED_PART_NUMBER_INTERFACE
LEP_RESULT LEP_GetOemFlirPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16; /* 32 bytes */

   /* Validate Parameter(s)
   */
   if( oemPartNumberPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Part Number
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_FLIR_PART_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )oemPartNumberPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetOemCustPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;   /* 32 bytes */

   if( oemPartNumberPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_CUST_PART_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )oemPartNumberPtr,
                              attributeWordLength );
   return( result );
}
#else
LEP_RESULT LEP_GetOemFlirPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16; /* 32 bytes */

   /* Validate Parameter(s)
   */
   if( oemPartNumberPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Part Number
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_FLIR_PART_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )oemPartNumberPtr,
                              attributeWordLength );
   return( result );
}
LEP_RESULT LEP_GetOemCustPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;   /* 32 bytes */

   if( oemPartNumberPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_CUST_PART_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )oemPartNumberPtr,
                              attributeWordLength );

   return( result );
}
#endif

LEP_RESULT LEP_GetOemSoftwareVersion( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_OEM_SW_VERSION_T *oemSoftwareVersionPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* 4 16-bit words to contain 64-bits */

   /* Validate Parameter(s)
   */
   if( oemSoftwareVersionPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Software Version
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_SOFTWARE_VERSION,
                              ( LEP_ATTRIBUTE_T_PTR )oemSoftwareVersionPtr,
                              attributeWordLength );
   return( result );
}


#if 0
LEP_RESULT LEP_GetOemVendorID(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_OEM_VENDORID_T *oemVendorIDPtr)
{
   LEP_RESULT  result = LEP_OK;

   return( result );
}
#endif



LEP_RESULT LEP_GetOemVideoOutputEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_VIDEO_OUTPUT_ENABLE_E_PTR oemVideoOutputEnablePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoOutputEnablePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current video output enable state
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_ENABLE,
                              ( LEP_ATTRIBUTE_T_PTR )oemVideoOutputEnablePtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetOemVideoOutputEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_VIDEO_OUTPUT_ENABLE_E oemVideoOutputEnable )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoOutputEnable >= LEP_END_VIDEO_OUTPUT_ENABLE )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_ENABLE,
                              ( LEP_ATTRIBUTE_T_PTR ) & oemVideoOutputEnable,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetOemVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_VIDEO_OUTPUT_FORMAT_E_PTR oemVideoOutputFormatPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoOutputFormatPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current video output format
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_FORMAT,
                              ( LEP_ATTRIBUTE_T_PTR )oemVideoOutputFormatPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetOemVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_VIDEO_OUTPUT_FORMAT_E oemVideoOutputFormat )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoOutputFormat >= LEP_END_VIDEO_OUTPUT_FORMAT )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output format
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_FORMAT,
                              ( LEP_ATTRIBUTE_T_PTR ) & oemVideoOutputFormat,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetOemVideoOutputSource( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_VIDEO_OUTPUT_SOURCE_E_PTR oemVideoOutputSourcePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoOutputSourcePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current video output source selection
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_SOURCE,
                              ( LEP_ATTRIBUTE_T_PTR )oemVideoOutputSourcePtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetOemVideoOutputSource( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_VIDEO_OUTPUT_SOURCE_E oemVideoOutputSource )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoOutputSource >= LEP_END_VIDEO_OUTPUT_SOURCE )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output source selection
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_SOURCE,
                              ( LEP_ATTRIBUTE_T_PTR ) & oemVideoOutputSource,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetOemVideoOutputSourceConstant( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_UINT16 oemVideoOutputSourceConstant )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* 1 16-bit values for constant value */


   /* Perform Command
   ** Reading the Camera's current video output source selection
   */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_CONSTANT,
                              ( LEP_ATTRIBUTE_T_PTR ) & oemVideoOutputSourceConstant,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetOemVideoOutputSourceConstant( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_UINT16 *oemVideoOutputSourceConstPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* 1 16-bit values for constant value */

   if( oemVideoOutputSourceConstPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output source selection
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_CONSTANT,
                              ( LEP_ATTRIBUTE_T_PTR )oemVideoOutputSourceConstPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_GetOemVideoOutputChannel( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_OEM_VIDEO_OUTPUT_CHANNEL_E_PTR oemVideoOutputChannelPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoOutputChannelPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current video output channel selection
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_CHANNEL,
                              ( LEP_ATTRIBUTE_T_PTR )oemVideoOutputChannelPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetOemVideoOutputChannel( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_OEM_VIDEO_OUTPUT_CHANNEL_E oemVideoOutputChannel )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoOutputChannel >= LEP_END_VIDEO_OUTPUT_CHANNEL )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output channel selection
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_CHANNEL,
                              ( LEP_ATTRIBUTE_T_PTR ) & oemVideoOutputChannel,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetOemVideoGammaEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_VIDEO_GAMMA_ENABLE_E_PTR oemVideoGammaEnablePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoGammaEnablePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current video gamma correction enable
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_GAMMA_ENABLE,
                              ( LEP_ATTRIBUTE_T_PTR )oemVideoGammaEnablePtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetOemVideoGammaEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_VIDEO_GAMMA_ENABLE_E oemVideoGammaEnable )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoGammaEnable >= LEP_END_VIDEO_GAMMA_ENABLE )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output channel selection
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_GAMMA_ENABLE,
                              ( LEP_ATTRIBUTE_T_PTR ) & oemVideoGammaEnable,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetOemCalStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_OEM_STATUS_E_PTR calStatusPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if( calStatusPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_STATUS,
                              ( LEP_ATTRIBUTE_T_PTR )calStatusPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetOemFFCNormalizationTarget( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_OEM_FFC_NORMALIZATION_TARGET_T_PTR ffcTargetPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* FFC Target is a single 16-bit value */

   if( ffcTargetPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_FFC_NORMALIZATION_TARGET,
                              ( LEP_ATTRIBUTE_T_PTR )ffcTargetPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_SetOemFFCNormalizationTarget( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_OEM_FFC_NORMALIZATION_TARGET_T ffcTarget )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_FFC_NORMALIZATION_TARGET,
                              ( LEP_ATTRIBUTE_T_PTR ) & ffcTarget,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_RunOemFFCNormalization( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_FFC_NORMALIZATION_TARGET_T ffcTarget )
{
   LEP_RESULT result = LEP_OK;
   LEP_OEM_STATUS_E oemStatus = LEP_OEM_STATUS_BUSY;

   result = LEP_SetOemFFCNormalizationTarget( portDescPtr, ffcTarget );
   if( result == LEP_OK )
   {
      result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_FFC_NORMALIZATION_TARGET );
      while( oemStatus == LEP_OEM_STATUS_BUSY )
      {
         LEP_GetOemCalStatus( portDescPtr, &oemStatus );
      }
   }

   return( result );

}

LEP_RESULT LEP_RunOemFFC( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_OEM_STATUS_E oemStatus = LEP_OEM_STATUS_BUSY;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_FFC_NORMALIZATION_TARGET );
   while( oemStatus == LEP_OEM_STATUS_BUSY )
   {
      LEP_GetOemCalStatus( portDescPtr, &oemStatus );
   }

   return( result );

}

LEP_RESULT LEP_GetOemFrameMean( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_OEM_FRAME_AVERAGE_T_PTR frameAveragePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   if( frameAveragePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_SCENE_MEAN_VALUE,
                              ( LEP_ATTRIBUTE_T_PTR )frameAveragePtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetOemPowerMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_OEM_POWER_STATE_E_PTR powerModePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( powerModePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_POWER_MODE,
                              ( LEP_ATTRIBUTE_T_PTR )powerModePtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_SetOemPowerMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_OEM_POWER_STATE_E powerMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( powerMode >= LEP_OEM_END_POWER_MODE )
   {
      return( LEP_RANGE_ERROR );
   }

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_POWER_MODE,
                              ( LEP_ATTRIBUTE_T_PTR ) & powerMode,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetOemGpioMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_OEM_GPIO_MODE_E_PTR gpioModePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( gpioModePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_GPIO_MODE_SELECT,
                              ( LEP_ATTRIBUTE_T_PTR )gpioModePtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LEP_SetOemGpioMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_OEM_GPIO_MODE_E gpioMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( gpioMode >= LEP_OEM_END_GPIO_MODE )
   {
      return( LEP_RANGE_ERROR );
   }

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_GPIO_MODE_SELECT,
                              ( LEP_ATTRIBUTE_T_PTR ) & gpioMode,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LEP_GetOemGpioVsyncPhaseDelay( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_OEM_VSYNC_DELAY_E_PTR numHsyncLinesPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( numHsyncLinesPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_GPIO_VSYNC_PHASE_DELAY,
                              ( LEP_ATTRIBUTE_T_PTR )numHsyncLinesPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LEP_SetOemGpioVsyncPhaseDelay( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_OEM_VSYNC_DELAY_E numHsyncLines )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( numHsyncLines >= LEP_END_OEM_VSYNC_DELAY )
   {
      return( LEP_RANGE_ERROR );
   }

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_GPIO_VSYNC_PHASE_DELAY,
                              ( LEP_ATTRIBUTE_T_PTR ) & numHsyncLines,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetOemUserDefaultsState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_USER_PARAMS_STATE_E_PTR userParamsStatePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( userParamsStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_USER_DEFAULTS,
                              ( LEP_ATTRIBUTE_T_PTR )userParamsStatePtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_RunOemUserDefaultsCopyToOtp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_USER_DEFAULTS );

   return( result );
}

LEP_RESULT LEP_RunOemUserDefaultsRestore( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_USER_DEFAULTS_RESTORE );

   return( result );
}



LEP_RESULT LEP_SetOemThermalShutdownEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T ThermalShutdownEnableState )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum = two words */

   if( ThermalShutdownEnableState.oemThermalShutdownEnable >= LEP_OEM_END_STATE )
   {
      return( LEP_RANGE_ERROR );
   }
   /* Perform Command
   ** Writing the Camera's current video freeze enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_THERMAL_SHUTDOWN_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR ) & ThermalShutdownEnableState,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetOemThermalShutdownEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T_PTR ThermalShutdownEnableStatePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum = two words */

   if( ThermalShutdownEnableStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_THERMAL_SHUTDOWN_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR )ThermalShutdownEnableStatePtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetOemShutterProfileObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_SHUTTER_PROFILE_OBJ_T ShutterProfileObj )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* 2 words */

   /* Perform Command
   ** Writing the Camera's current video freeze enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_SHUTTER_PROFILE_OBJ,
                              ( LEP_ATTRIBUTE_T_PTR ) & ShutterProfileObj,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetOemShutterProfileObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_SHUTTER_PROFILE_OBJ_T_PTR ShutterProfileObjPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* 2 words */

   if( ShutterProfileObjPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_SHUTTER_PROFILE_OBJ,
                              ( LEP_ATTRIBUTE_T_PTR )ShutterProfileObjPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LEP_SetOemBadPixelReplaceControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T BadPixelReplaceControl )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two word enums */

   /* Perform Command
   ** Writing the Camera's current video freeze enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_BAD_PIXEL_REPLACE_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR ) & BadPixelReplaceControl,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LEP_GetOemBadPixelReplaceControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T_PTR BadPixelReplaceControlPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum */
   if( BadPixelReplaceControlPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_BAD_PIXEL_REPLACE_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR )BadPixelReplaceControlPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetOemTemporalFilterControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_TEMPORAL_FILTER_CONTROL_T TemporalFilterControl )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum = two words */

   /* Perform Command
   ** Writing the Camera's current video freeze enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_TEMPORAL_FILTER_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR ) & TemporalFilterControl,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetOemTemporalFilterControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_TEMPORAL_FILTER_CONTROL_T_PTR TemporalFilterControlPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum = two words */
   if( TemporalFilterControlPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_TEMPORAL_FILTER_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR )TemporalFilterControlPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_SetOemColumnNoiseEstimateControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T ColumnNoiseEstimateControl )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum = two words */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_COLUMN_NOISE_ESTIMATE_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR ) & ColumnNoiseEstimateControl,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetOemColumnNoiseEstimateControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T_PTR ColumnNoiseEstimateControlPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum = two words */
   if( ColumnNoiseEstimateControlPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_COLUMN_NOISE_ESTIMATE_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR )ColumnNoiseEstimateControlPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LEP_GetOemPixelNoiseSettings( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_OEM_PIXEL_NOISE_SETTINGS_T_PTR pixelNoiseSettingsPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* struct size 4 bytes */

   if( pixelNoiseSettingsPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_PIXEL_NOISE_ESTIMATE_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR )pixelNoiseSettingsPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LEP_SetOemPixelNoiseSettings( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_OEM_PIXEL_NOISE_SETTINGS_T pixelNoiseSettings )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* struct size 4 bytes */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_PIXEL_NOISE_ESTIMATE_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR ) & pixelNoiseSettings,
                              attributeWordLength );

   return( result );
}

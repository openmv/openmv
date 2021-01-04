/*******************************************************************************
*
*    FILE: LEPTON_SDK.c
*
*    DESCRIPTION: Lepton SDK
*
*    AUTHOR:
*
*    CREATED: 3/1/2012
*
*    HISTORY: 3/1/2012 DWD Initial Draft
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
#include "LEPTON_I2C_Protocol.h"



#ifdef LEP_USE_DYNAMIC_ALLOCATION
    #include "stdlib.h"
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
//static LEP_RESULT _LEP_DelayCounts(LEP_UINT32 counts);

/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/
LEP_SDK_VERSION_T sdkVersion;
/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/

LEP_RESULT LEP_GetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                            LEP_COMMAND_ID commandID,
                            LEP_ATTRIBUTE_T_PTR attributePtr,
                            LEP_UINT16 attributeWordLength)
{
    LEP_RESULT  result = LEP_OK;

    /* Validate the port descriptor
    */
    if( portDescPtr == NULL )
    {
        return(LEP_COMM_PORT_NOT_OPEN);
    }

    /* Validate input pointer
    */
    if( attributePtr == NULL )
    {
        return(LEP_BAD_ARG_POINTER_ERROR);
    }

    /* Modify the passed-in command ID to add the Get type
    */
    commandID |= LEP_GET_TYPE;

    /* Perform Command using the active Port
    */
    if( portDescPtr->portType == LEP_CCI_TWI )
    {
        /* Use the Lepton TWI/CCI Port
        */
        result = LEP_I2C_GetAttribute( portDescPtr,
                                       commandID,
                                       attributePtr,
                                       attributeWordLength );
    }
    else if( portDescPtr->portType == LEP_CCI_SPI )
    {

        /* Use the Lepton SPI Port
        */

    }
    else
        result = LEP_COMM_INVALID_PORT_ERROR;

    return(result);
}

/**
 * Sets the value of a camera attribute.
 *
 * @return
 */
LEP_RESULT LEP_SetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                            LEP_COMMAND_ID commandID,
                            LEP_ATTRIBUTE_T_PTR attributePtr,
                            LEP_UINT16 attributeWordLength)
{
    LEP_RESULT  result = LEP_OK;

    /* Validate the port descriptor
    */
    if( portDescPtr == NULL )
    {
        return(LEP_COMM_PORT_NOT_OPEN);
    }

    /* Modify the passed-in command ID to add the Get type
    */
    commandID |= LEP_SET_TYPE;

    /* Issue Command to the Active Port
    */
    if( portDescPtr->portType == LEP_CCI_TWI )
    {
        /* Use the Lepton TWI/CCI Port
        */
        result = LEP_I2C_SetAttribute( portDescPtr,
                                       commandID,
                                       attributePtr,
                                       attributeWordLength );
    }
    else if( portDescPtr->portType == LEP_CCI_SPI )
    {
        /* Use the Lepton SPI Port
        */

    }
    else
        result = LEP_COMM_INVALID_PORT_ERROR;

    return(result);
}

LEP_RESULT LEP_RunCommand(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                          LEP_COMMAND_ID commandID)
{
    LEP_RESULT  result = LEP_OK;

    /* Validate the port descriptor
    */
    if( portDescPtr == NULL )
    {
        return(LEP_COMM_PORT_NOT_OPEN);
    }

    /* Modify the passed-in command ID to add the Run type
    */
    commandID |= LEP_RUN_TYPE;

    /* Perform Command
    */
    if( portDescPtr->portType == LEP_CCI_TWI )
    {
        /* Use the Lepton TWI/CCI Port
        */
        result = LEP_I2C_RunCommand( portDescPtr,
                                     commandID);
    }
    else if( portDescPtr->portType == LEP_CCI_SPI )
    {
        /* Use the Lepton SPI Port
        */

    }
    else
        result = LEP_COMM_INVALID_PORT_ERROR;

    return(result);
}


LEP_RESULT LEP_SelectDevice(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                            LEP_PROTOCOL_DEVICE_E device)
{
    LEP_RESULT result = LEP_OK;

    /* Validate the port descriptor
    */
    if( portDescPtr == NULL )
    {
        return(LEP_COMM_PORT_NOT_OPEN);
    }

    /* Select Device
    */
    if( portDescPtr->portType == LEP_CCI_TWI )
    {
        result = LEP_I2C_SelectDevice(portDescPtr, device);
    }
    else if( portDescPtr->portType == LEP_CCI_SPI )
    {

    }
    else
        result = LEP_COMM_INVALID_PORT_ERROR;

    return(result);
}

/******************************************************************************/
/**
 * Opens a Lepton commnications port of the specified type and
 * is assigned the passed ID.
 * This function dynamically allocates a new descriptor from the
 * system heap.
 *
 * @param portType    LEP_CAMERA_PORT_E  Specifies the Lepton
 *                    Communications Port type.
 *
 * @param pDescriptor LEP_CAMERA_PORT_DESC_T_PTR  Lepton Port
 *                    descriptor. This is a handle to a valid
 *                    Lepton port is fusseccful, NULL otherwise.
 *
 * @return LEP_RESULT  Lepton Error Code.  LEP_OK if all goes well,
 *         otherise and Lepton error code is retunred.
 */
LEP_RESULT LEP_OpenPort(cambus_t *bus,
                        LEP_CAMERA_PORT_E portType,
                        LEP_UINT16   portBaudRate,
                        LEP_CAMERA_PORT_DESC_T_PTR portDescPtr)
{
    LEP_RESULT result = LEP_OK;
    LEP_UINT8 deviceAddress;

    /* Attempt to acquire memory
    **   Dynamic creation using malloc() or static allocation
    **   Our reference will us dynamic creation
    */
#ifdef LEP_USE_DYNAMIC_ALLOCATION
    /* Allocate from the heap
    */
    portDescPtr = (LEP_CAMERA_PORT_DESC_T_PTR)malloc( sizeof(LEP_CAMERA_PORT_DESC_T));
#else
    /* Allocate from static memory
    */
#endif

    /* Validate the port descriptor
    */
    if( portDescPtr != NULL )
    {
        /* Open Port driver
        */
        switch( portType )
        {
            case LEP_CCI_TWI:
                result = LEP_I2C_OpenPort(bus, &portBaudRate, &deviceAddress);
                if( result == LEP_OK )
                {
                    portDescPtr->portBaudRate = portBaudRate;
                    portDescPtr->bus = bus;
                    portDescPtr->portType = portType;
                    portDescPtr->deviceAddress = deviceAddress;
                }

#ifdef LEP_USE_DYNAMIC_ALLOCATION
            else
               free( portDescPtr);
#endif


                break;

            case LEP_CCI_SPI:
                break;

            default:
                break;
        }
    }
    else
        result = LEP_ERROR_CREATING_COMM;


    return(result);
}

LEP_RESULT LEP_ClosePort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr)
{
    LEP_RESULT result = LEP_OK;

    /* Validate the port descriptor
    */
    if( portDescPtr == NULL )
    {
        return(LEP_COMM_PORT_NOT_OPEN);
    }

    /* Close Port driver
    */
    if( portDescPtr->portType == LEP_CCI_TWI )
    {
        result = LEP_I2C_ClosePort(portDescPtr);
    }
    else if( portDescPtr->portType == LEP_CCI_SPI )
    {

    }
    else
        result = LEP_COMM_INVALID_PORT_ERROR;



#ifdef LEP_USE_DYNAMIC_ALLOCATION
    free( portDescPtr );
#endif

    return(result);
}


LEP_RESULT LEP_ResetPort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr)
{
    LEP_RESULT result;

    /* Validate the port descriptor
    */
    if( portDescPtr == NULL )
    {
        return(LEP_COMM_PORT_NOT_OPEN);
    }

    if( portDescPtr->portType == LEP_CCI_TWI )
    {
        LEP_I2C_ResetPort( portDescPtr );
    }
    else if( portDescPtr->portType == LEP_CCI_SPI )
    {

    }
    else
        result = LEP_COMM_INVALID_PORT_ERROR;

    return(result);
}


LEP_RESULT LEP_GetPortStatus(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_UINT16 *status)
{
    LEP_RESULT result = LEP_OK;


    return(result);
}

LEP_RESULT LEP_DirectReadRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_UINT16 registerAddress,
                                  LEP_UINT16* regValue)
{
   LEP_RESULT result = LEP_OK;


   if( portDescPtr->portType == LEP_CCI_TWI )
   {
     /* Use the Lepton TWI/CCI Port
     */
      result = LEP_I2C_DirectReadRegister(portDescPtr, registerAddress, regValue);
   }
   else if( portDescPtr->portType == LEP_CCI_SPI )
   {
     /* Use the Lepton SPI Port
     */

   }
   else
     result = LEP_COMM_INVALID_PORT_ERROR;

   return(result);
}

LEP_RESULT LEP_GetDeviceAddress(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_UINT8* deviceAddress)
{
   LEP_RESULT result = LEP_OK;

   if(portDescPtr->portType == LEP_CCI_TWI)
   {
      result = LEP_I2C_GetDeviceAddress(portDescPtr, deviceAddress);
   }

   return(result);
}


LEP_RESULT LEP_DirectWriteRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_UINT16 registerAddress,
                                   LEP_UINT16 regValue)
{
   LEP_RESULT result = LEP_OK;
   /* Validate the port descriptor
   */
   if( portDescPtr == NULL )
   {
     return(LEP_COMM_PORT_NOT_OPEN);
   }

   /* Issue Command to the Active Port
   */
   if( portDescPtr->portType == LEP_CCI_TWI )
   {
     /* Use the Lepton TWI/CCI Port
     */
      result = LEP_I2C_DirectWriteRegister(portDescPtr, registerAddress, regValue);

   }
   else if( portDescPtr->portType == LEP_CCI_SPI )
   {
     /* Use the Lepton SPI Port
     */

   }
   else
     result = LEP_COMM_INVALID_PORT_ERROR;
   return(result);
}

LEP_RESULT LEP_DirectWriteBuffer(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_ATTRIBUTE_T_PTR attributePtr,
                                 LEP_UINT16 attributeWordLength)
{
   LEP_RESULT result = LEP_OK;
   /* Validate the port descriptor
   */
   if( portDescPtr == NULL )
   {
     return(LEP_COMM_PORT_NOT_OPEN);
   }

   /* Issue Command to the Active Port
   */
   if( portDescPtr->portType == LEP_CCI_TWI )
   {
     /* Use the Lepton TWI/CCI Port
     */
     result = LEP_I2C_DirectWriteBuffer(portDescPtr,
                                        attributePtr,
                                        attributeWordLength );
   }
   else if( portDescPtr->portType == LEP_CCI_SPI )
   {
     /* Use the Lepton SPI Port
     */

   }
   else
     result = LEP_COMM_INVALID_PORT_ERROR;
   return(result);
}

LEP_RESULT LEP_GetSDKVersion(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_SDK_VERSION_T_PTR sdkVersionPtr)
{
   LEP_RESULT result = LEP_OK;

   if(sdkVersionPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }
   sdkVersionPtr->major = LEP_SDK_VERSION_MAJOR;
   sdkVersionPtr->minor = LEP_SDK_VERSION_MINOR;
   sdkVersionPtr->build = LEP_SDK_VERSION_BUILD;

   return(result);
}

LEP_RESULT LEP_GetCameraBootStatus(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_SDK_BOOT_STATUS_E_PTR bootStatusPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 regValue;

   result = LEP_DirectReadRegister(portDescPtr, 0x2, &regValue);

   if(result == LEP_OK && regValue & 4)
   {
      *bootStatusPtr = LEP_BOOT_STATUS_BOOTED;
   }
   else
   {
      *bootStatusPtr = LEP_BOOT_STATUS_NOT_BOOTED;
   }

   return(result);
}

/******************************************************************************/
/** PRIVATE MODULE FUNCTIONS                                                 **/
/******************************************************************************/

//LEP_RESULT _LEP_DelayCounts(LEP_UINT32 counts)
//{
//    LEP_UINT32 a;
//    while( counts-- )
//    {
//        a=counts;
//    }
//    if( a )
//    {
//        return(LEP_TIMEOUT_ERROR) ;
//    }
//    return(LEP_OK);
//}

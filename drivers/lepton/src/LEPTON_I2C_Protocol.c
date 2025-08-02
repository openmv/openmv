/*******************************************************************************
**
**    File NAME: LEPTON_I2C_Protocol.c
**
**      AUTHOR:  David Dart
**
**      CREATED: 5/3/2012
**
**      DESCRIPTION: Lepton I2C Protocol Layer
**
**      HISTORY:  5/3/2012 DWD - Initial Draft
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
#include "LEPTON_Types.h"
#include "LEPTON_ErrorCodes.h"
#include "LEPTON_I2C_Protocol.h"
#include "LEPTON_I2C_Reg.h"
#include "crc16.h"

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

LEP_RESULT LEP_I2C_SelectDevice(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_PROTOCOL_DEVICE_E device)
{
    LEP_RESULT result;

    result = LEP_I2C_MasterSelectDevice( portDescPtr, device );

    return(result);
}

LEP_RESULT LEP_I2C_OpenPort(omv_i2c_t *bus,
                            LEP_UINT16 *baudRateInkHz,
                            LEP_UINT8* deviceAddress)
{
   LEP_RESULT result;
   LEP_UINT16 statusReg;

   result = LEP_I2C_MasterOpen( bus, baudRateInkHz );
   if(result != LEP_OK)
   {
      return(LEP_COMM_INVALID_PORT_ERROR);
   }

   *deviceAddress = 0x2a;
   result = LEP_I2C_MasterReadData( bus,
                                    *deviceAddress,
                                    LEP_I2C_STATUS_REG,
                                    &statusReg,
                                    1 );

   if(result != LEP_OK)
   {
      /*
       *    Try 0x00 as the device address if 0x2a didn't work. In this case, we are in Virgin Boot Mode.
       *
       */
      *deviceAddress = 0x00;
      result = LEP_I2C_MasterReadData( bus,
                                       *deviceAddress,
                                       LEP_I2C_STATUS_REG,
                                       &statusReg,
                                       1 );
      if(result != LEP_OK)
      {
         return(LEP_COMM_NO_DEV);
      }
   }

    return(result);
}


LEP_RESULT LEP_I2C_ClosePort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr)
{
    LEP_RESULT result;

    result =LEP_I2C_MasterClose( portDescPtr );

    return(result);
}


LEP_RESULT LEP_I2C_ResetPort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr)
{
    LEP_RESULT result;

    result = LEP_I2C_MasterReset( portDescPtr );

    return(result);
}


LEP_RESULT LEP_I2C_GetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_COMMAND_ID commandID,
                                LEP_ATTRIBUTE_T_PTR attributePtr,
                                LEP_UINT16 attributeWordLength)
{
    LEP_RESULT result;
    LEP_UINT16 statusReg;
    LEP_INT16 statusCode;
    LEP_UINT32 done;
    LEP_UINT16 crcExpected, crcActual;

    /* Implement the Lepton TWI READ Protocol
    */
    /* First wait until the Camera is ready to receive a new
    ** command by polling the STATUS REGISTER BUSY Bit until it
    ** reports NOT BUSY.
    */

    do
    {
        /* Read the Status REGISTER and peek at the BUSY Bit
        */
        result = LEP_I2C_MasterReadData( portDescPtr->bus,
                                         portDescPtr->deviceAddress,
                                         LEP_I2C_STATUS_REG,
                                         &statusReg,
                                         1 );
        if(result != LEP_OK)
        {
           return(result);
        }
        done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;

    }while( !done );

    /* Set the Lepton's DATA LENGTH REGISTER first to inform the
    ** Lepton Camera how many 16-bit DATA words we want to read.
    */
    result = LEP_I2C_MasterWriteData( portDescPtr->bus,
                                      portDescPtr->deviceAddress,
                                      LEP_I2C_DATA_LENGTH_REG,
                                      &attributeWordLength,
                                      1);
    if(result != LEP_OK)
    {
       return(result);
    }
    /* Now issue the GET Attribute Command
    */
    result = LEP_I2C_MasterWriteData( portDescPtr->bus,
                                      portDescPtr->deviceAddress,
                                      LEP_I2C_COMMAND_REG,
                                      &commandID,
                                      1);

    if(result != LEP_OK)
    {
       return(result);
    }

    /* Now wait until the Camera has completed this command by
    ** polling the statusReg REGISTER BUSY Bit until it reports NOT
    ** BUSY.
    */
    do
    {
        /* Read the statusReg REGISTER and peek at the BUSY Bit
        */
        result = LEP_I2C_MasterReadData( portDescPtr->bus,
                                         portDescPtr->deviceAddress,
                                         LEP_I2C_STATUS_REG,
                                         &statusReg,
                                         1 );

        if(result != LEP_OK)
        {
           return(result);
        }
        done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;

    }while( !done );


    /* Check statusReg word for Errors?
    */
    statusCode = (statusReg >> 8) ? ((statusReg >> 8) | 0xFF00) : 0;
    if(statusCode)
    {
      return((LEP_RESULT)statusCode);
    }

    /* If NO Errors then READ the DATA from the DATA REGISTER(s)
    */
    if( attributeWordLength <= 16 )
    {
        /* Read from the DATA Registers - always start from DATA 0
        ** Little Endean
        */
        result = LEP_I2C_MasterReadData(portDescPtr->bus,
                                        portDescPtr->deviceAddress,
                                        LEP_I2C_DATA_0_REG,
                                        attributePtr,
                                        attributeWordLength );
    }
    else if( attributeWordLength <= 1024 )
    {
        /* Read from the DATA Block Buffer
        */
      result = LEP_I2C_MasterReadData(portDescPtr->bus,
                                      portDescPtr->deviceAddress,
                                      LEP_I2C_DATA_BUFFER_0,
                                      attributePtr,
                                      attributeWordLength );
    }
    if(result == LEP_OK && attributeWordLength > 0)
    {
       /* Check CRC */
       result = LEP_I2C_MasterReadData( portDescPtr->bus,
                                        portDescPtr->deviceAddress,
                                        LEP_I2C_DATA_CRC_REG,
                                        &crcExpected,
                                        1);
       crcActual = (LEP_UINT16)CalcCRC16Words(attributeWordLength, (short*)attributePtr);

       /* Check for 0 in the register in case the camera does not support CRC check
       */
       if(crcExpected != 0 && crcExpected != crcActual)
       {
          return(LEP_CHECKSUM_ERROR);
       }

    }


    return(result);
}


LEP_RESULT LEP_I2C_SetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_COMMAND_ID commandID,
                                LEP_ATTRIBUTE_T_PTR attributePtr,
                                LEP_UINT16 attributeWordLength)
{
    LEP_RESULT result;
    LEP_UINT16 statusReg;
    LEP_INT16 statusCode;
    LEP_UINT32 done;
    LEP_UINT16 timeoutCount = LEPTON_I2C_COMMAND_BUSY_WAIT_COUNT;

    /* Implement the Lepton TWI WRITE Protocol
    */
    /* First wait until the Camera is ready to receive a new
    ** command by polling the STATUS REGISTER BUSY Bit until it
    ** reports NOT BUSY.
    */
    do
    {
        /* Read the Status REGISTER and peek at the BUSY Bit
        */
        result = LEP_I2C_MasterReadData( portDescPtr->bus,
                                         portDescPtr->deviceAddress,
                                         LEP_I2C_STATUS_REG,
                                         &statusReg,
                                         1 );
        if(result != LEP_OK)
        {
           return(result);
        }
        done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;
        /* Add timout check */
        if( timeoutCount-- == 0 )
        {
            /* Timed out waiting for command busy to go away
            */
          return(LEP_TIMEOUT_ERROR);

        }
    }while( !done );

    if( result == LEP_OK )
    {
        /* Now WRITE the DATA to the DATA REGISTER(s)
        */
        if( attributeWordLength <= 16 )
        {
            /* WRITE to the DATA Registers - always start from DATA 0
            */
            result = LEP_I2C_MasterWriteData(portDescPtr->bus,
                                             portDescPtr->deviceAddress,
                                             LEP_I2C_DATA_0_REG,
                                             attributePtr,
                                             attributeWordLength );
        }
        else if( attributeWordLength <= 1024 )
        {
            /* WRITE to the DATA Block Buffer
            */
            result = LEP_I2C_MasterWriteData(portDescPtr->bus,
                                             portDescPtr->deviceAddress,
                                             LEP_I2C_DATA_BUFFER_0,
                                             attributePtr,
                                             attributeWordLength );

        }
        else
            result = LEP_RANGE_ERROR;
    }

    if( result == LEP_OK )
    {
        /* Set the Lepton's DATA LENGTH REGISTER first to inform the
        ** Lepton Camera how many 16-bit DATA words we want to read.
        */
        result = LEP_I2C_MasterWriteData( portDescPtr->bus,
                                          portDescPtr->deviceAddress,
                                          LEP_I2C_DATA_LENGTH_REG,
                                          &attributeWordLength,
                                          1);

        if( result == LEP_OK )
        {
            /* Now issue the SET Attribute Command
            */
            result = LEP_I2C_MasterWriteData( portDescPtr->bus,
                                              portDescPtr->deviceAddress,
                                              LEP_I2C_COMMAND_REG,
                                              &commandID,
                                              1);

            if( result == LEP_OK )
            {
                /* Now wait until the Camera has completed this command by
                ** polling the statusReg REGISTER BUSY Bit until it reports NOT
                ** BUSY.
                */
                do
                {
                    /* Read the statusReg REGISTER and peek at the BUSY Bit
                    */
                    result = LEP_I2C_MasterReadData( portDescPtr->bus,
                                                     portDescPtr->deviceAddress,
                                                     LEP_I2C_STATUS_REG,
                                                     &statusReg,
                                                     1 );
                    if(result != LEP_OK)
                    {
                       return(result);
                    }
                    done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;

                }while( !done );

                    /* Check statusReg word for Errors?
                   */
                   statusCode = (statusReg >> 8) ? ((statusReg >> 8) | 0xFF00) : 0;
                   if(statusCode)
                   {
                     return((LEP_RESULT)statusCode);
                   }

            }
        }
    }

    /* Check statusReg word for Errors?
    */

    return(result);
}

LEP_RESULT LEP_I2C_RunCommand(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_COMMAND_ID commandID)
{
    LEP_RESULT result;
    LEP_UINT16 statusReg;
    LEP_INT16 statusCode;
    LEP_UINT16 timeoutCount = LEPTON_I2C_COMMAND_BUSY_WAIT_COUNT;

    /* Implement the Lepton TWI WRITE Protocol
    */
    /* First wait until the Camera is ready to receive a new
    ** command by polling the STATUS REGISTER BUSY Bit until it
    ** reports NOT BUSY.
    */
    do {
        /* Read the Status REGISTER and peek at the BUSY Bit
        */
        result = LEP_I2C_MasterReadRegister( portDescPtr->bus,
                                             portDescPtr->deviceAddress,
                                             LEP_I2C_STATUS_REG,
                                             &statusReg);
        if (result != LEP_OK) {
            return result;
        }

        if (timeoutCount-- == 0) {
            return LEP_ERROR;
        }
    } while((statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK));

    timeoutCount = LEPTON_I2C_COMMAND_BUSY_WAIT_COUNT;

    if( result == LEP_OK ) {
        /* Set the Lepton's DATA LENGTH REGISTER first to inform the
        ** Lepton Camera no 16-bit DATA words being transferred.
        */
        result = LEP_I2C_MasterWriteRegister( portDescPtr->bus,
                                              portDescPtr->deviceAddress,
                                              LEP_I2C_DATA_LENGTH_REG,
                                              (LEP_UINT16)0);

        if (result == LEP_OK) {
            /* Now issue the Run Command
            */
            result = LEP_I2C_MasterWriteRegister( portDescPtr->bus,
                                                  portDescPtr->deviceAddress,
                                                  LEP_I2C_COMMAND_REG,
                                                  commandID);
            if( result == LEP_OK ) {
                /* Now wait until the Camera has completed this command by
                ** polling the statusReg REGISTER BUSY Bit until it reports NOT
                ** BUSY.
                */
                do {
                    /* Read the statusReg REGISTER and peek at the BUSY Bit
                    */
                    result = LEP_I2C_MasterReadRegister( portDescPtr->bus,
                                                         portDescPtr->deviceAddress,
                                                         LEP_I2C_STATUS_REG,
                                                         &statusReg);
                    if (result != LEP_OK) {
                        return(result);
                    }

                    if (timeoutCount-- == 0) {
                        return LEP_ERROR;
                    }
                } while(statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK);

                statusCode = (statusReg >> 8) ? ((statusReg >> 8) | 0xFF00) : 0;
                if (statusCode) {
                    return (LEP_RESULT) statusCode;
                }
            }
        }
    }

    return result;
}

LEP_RESULT LEP_I2C_DirectReadRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_UINT16 regAddress,
                                      LEP_UINT16 *regValue)
{
   LEP_RESULT result = LEP_OK;

   result = LEP_I2C_MasterReadRegister( portDescPtr->bus,
                                        portDescPtr->deviceAddress,
                                        regAddress,
                                        regValue);

   return(result);
}

LEP_RESULT LEP_I2C_GetPortStatus(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr)
{
    LEP_RESULT result = LEP_OK;

    return(result);
}

LEP_RESULT LEP_I2C_GetDeviceAddress(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_UINT8* deviceAddress)
{
   LEP_RESULT result = LEP_OK;

   if(deviceAddress == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }
   *deviceAddress = portDescPtr->deviceAddress;

   return(result);
}


LEP_RESULT LEP_I2C_DirectWriteBuffer(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_ATTRIBUTE_T_PTR attributePtr,
                                     LEP_UINT16 attributeWordLength)
{
   LEP_RESULT result = LEP_OK;

   /* WRITE to the DATA Block Buffer
   */
   result = LEP_I2C_MasterWriteData(portDescPtr->bus,
                                    portDescPtr->deviceAddress,
                                    LEP_I2C_DATA_BUFFER_0,
                                    attributePtr,
                                    attributeWordLength );



   return(result);
}

LEP_RESULT LEP_I2C_DirectWriteRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 regAddress,
                                       LEP_UINT16 regValue)
{
   LEP_RESULT result = LEP_OK;

   result = LEP_I2C_MasterWriteRegister(portDescPtr->bus,
                                        portDescPtr->deviceAddress,
                                        regAddress,
                                        regValue);
   return(result);
}

/******************************************************************************/
/** PRIVATE MODULE FUNCTIONS                                                 **/
/******************************************************************************/

/*******************************************************************************
**
**    File NAME: LEPTON_I2C_Protocol.h
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
#ifndef _LEPTON_I2C_PROTOCOL_H_
    #define _LEPTON_I2C_PROTOCOL_H_

    #ifdef __cplusplus
extern "C"
{
    #endif
/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
    #include "LEPTON_Types.h"
    #include "LEPTON_ErrorCodes.h"

    #include "LEPTON_I2C_Service.h"
/******************************************************************************/
/** EXPORTED DEFINES                                                         **/
/******************************************************************************/

    /* Timeout count to wait for I2C command to complete
    */
    #define LEPTON_I2C_COMMAND_BUSY_WAIT_COUNT              1000

/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/


    typedef enum LEP_I2C_COMMAND_STATUS_TAG
    {
        LEP_I2C_COMMAND_NOT_BUSY = 0,
        LEP_I2C_COMMAND_IS_BUSY,
        LEP_I2C_END_COMMAND_STATUS,

        LEP_I2C_COMMAND_STATUS_MAKE_32_BIT_ENUM = 0x7FFFFFFF
    }LEP_I2C_COMMAND_STATUS_E, *LEP_I2C_COMMAND_STATUS_E_PTR;

/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/

    LEP_RESULT LEP_I2C_GetCommandBusyStatus(LEP_I2C_COMMAND_STATUS_E_PTR commandStatus);

    LEP_RESULT LEP_I2C_SetCommandRegister(LEP_COMMAND_ID commandID,
                                          LEP_UINT16 *transactionStatus);

    extern LEP_RESULT LEP_I2C_SelectDevice(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_PROTOCOL_DEVICE_E device);

    extern LEP_RESULT LEP_I2C_OpenPort(cambus_t *bus,
                                       LEP_UINT16 *baudRateInkHz,
                                       LEP_UINT8 *deviceAddress);

    extern LEP_RESULT LEP_I2C_ClosePort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    extern LEP_RESULT LEP_I2C_ResetPort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    extern LEP_RESULT LEP_I2C_GetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_COMMAND_ID commandID,
                                           LEP_ATTRIBUTE_T_PTR attributePtr,
                                           LEP_UINT16 attributeWordLength);

    extern LEP_RESULT LEP_I2C_SetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_COMMAND_ID commandID,
                                           LEP_ATTRIBUTE_T_PTR attributePtr,
                                           LEP_UINT16 attributeWordLength);

    extern LEP_RESULT LEP_I2C_RunCommand(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_COMMAND_ID commandID);

    extern LEP_RESULT LEP_I2C_ReadData(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    extern LEP_RESULT LEP_I2C_WriteData(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    extern LEP_RESULT LEP_I2C_GetPortStatus(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    extern LEP_RESULT LEP_I2C_GetDeviceAddress(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                               LEP_UINT8* deviceAddress);

    extern LEP_RESULT LEP_I2C_DirectWriteRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                  LEP_UINT16 regAddress,
                                                  LEP_UINT16 regValue);
    extern LEP_RESULT LEP_I2C_DirectWriteBuffer(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_ATTRIBUTE_T_PTR attributePtr,
                                                LEP_UINT16 attributeWordLength);
    extern LEP_RESULT LEP_I2C_DirectReadRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_UINT16 regAddress,
                                                 LEP_UINT16 *regValue);

/******************************************************************************/
    #ifdef __cplusplus
}
    #endif

#endif  /* _LEPTON_I2C_PROTOCOL_H_ */

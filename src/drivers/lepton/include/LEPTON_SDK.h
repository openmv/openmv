/*******************************************************************************
*
*    FILE: LEPTON_SDK.h
*
*    DESCRIPTION:
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
#ifndef _LEPTON_SDK_H_
    #define _LEPTON_SDK_H_

    #ifdef __cplusplus
extern "C"
{
    #endif

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
    #include "LEPTON_Types.h"
    #include "LEPTON_ErrorCodes.h"
    #include "LEPTON_SDKConfig.h"
    #include "LEPTON_I2C_Protocol.h"

/******************************************************************************/
    /**
     * If defined then the CCI and SPI port drivers require opening
     * and closing with each access; otherwise the ports are opened
     * once and left open for future accesses
     */
//    #define LEP_USE_DYNAMIC_ALLOCATION

     #define LEP_JOVA_I2C
     #define LEP_SDK_VERSION_MAJOR         3
     #define LEP_SDK_VERSION_MINOR         3
     #define LEP_SDK_VERSION_BUILD         13

    /* SDK Module Command IDs
    */
    #define LEP_SDK_MODULE_BASE            0x0000

    #define LEP_SDK_ENABLE_STATE           (LEP_SDK_MODULE_BASE + 0x0000 )


/******************************************************************************/
/** EXPORTED TYPEDEFS                                                        **/
/******************************************************************************/
   typedef struct LEP_SDK_VERSION_TAG
   {
      LEP_UINT8   major;
      LEP_UINT8   minor;
      LEP_UINT8   build;
      LEP_UINT8   reserved;

   }LEP_SDK_VERSION_T, *LEP_SDK_VERSION_T_PTR;

   typedef enum LEP_SDK_BOOT_STATUS_E_TAG
   {
      LEP_BOOT_STATUS_NOT_BOOTED = 0,
      LEP_BOOT_STATUS_BOOTED = 1,

      LEP_END_BOOT_STATUS,
      LEP_BOOT_STATUS_MAKE_32_BIT_ENUM = 0x7FFFFFFF
   }LEP_SDK_BOOT_STATUS_E, *LEP_SDK_BOOT_STATUS_E_PTR;
/******************************************************************************/
/** EXPORTED DEFINES                                                         **/
/******************************************************************************/

/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/
   extern LEP_SDK_VERSION_T sdkVersion;
/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/

    extern LEP_RESULT LEP_SelectDevice(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_PROTOCOL_DEVICE_E device);

    extern LEP_RESULT LEP_OpenPort(cambus_t *bus,
                                   LEP_CAMERA_PORT_E portType,
                                   LEP_UINT16   portBaudRate,
                                   LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    extern LEP_RESULT LEP_ClosePort(LEP_CAMERA_PORT_DESC_T_PTR pd);

    extern LEP_RESULT LEP_ResetPort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr);

    extern LEP_RESULT LEP_GetPortStatus(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_UINT16 *status);

    extern LEP_RESULT LEP_GetDeviceAddress(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_UINT8* deviceAddress);

    extern LEP_RESULT LEP_GetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_COMMAND_ID commandID,
                                       LEP_ATTRIBUTE_T_PTR attributePtr,
                                       LEP_UINT16 attributeWordLength);

    extern LEP_RESULT LEP_SetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_COMMAND_ID commandID,
                                       LEP_ATTRIBUTE_T_PTR attributePtr,
                                       LEP_UINT16 attributeWordLength);

    extern LEP_RESULT LEP_RunCommand(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_COMMAND_ID commandID);

    extern LEP_RESULT LEP_DirectWriteBuffer(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_ATTRIBUTE_T_PTR attributePtr,
                                            LEP_UINT16 attributeWordLength);

    extern LEP_RESULT LEP_DirectWriteRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 registerAddress,
                                              LEP_UINT16 regValue);

    extern LEP_RESULT LEP_DirectReadRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_UINT16 registerAddress,
                                             LEP_UINT16* regValue);

    extern LEP_RESULT LEP_GetSDKVersion(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_SDK_VERSION_T_PTR sdkVersionPtr);

    extern LEP_RESULT LEP_GetCameraBootStatus(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_SDK_BOOT_STATUS_E_PTR bootStatusPtr);

/******************************************************************************/

    #ifdef __cplusplus
}
    #endif

#endif /* _LEPTON_SDK_H_ */

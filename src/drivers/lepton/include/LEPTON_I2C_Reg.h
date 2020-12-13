/*******************************************************************************
**
**    File NAME: LEPTON_I2C_Reg.h
**
**      AUTHOR:  David Dart
**
**      CREATED: 7/31/2012
**
**      DESCRIPTION: Defines the Lepton I2C Interface for Host
**                   Command and Control
**
**      HISTORY:  7/31/2012 DWD - Initial Draft
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
#ifndef _LEPTON_I2C_REG_H_
    #define _LEPTON_I2C_REG_H_

    #ifdef __cplusplus
extern "C"
{
    #endif

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/

/******************************************************************************/
/** EXPORTED DEFINES                                                         **/
/******************************************************************************/

/* DEVICE ADDRESSES
*/
/* The Lepton camera's device address
*/
    #define LEP_I2C_DEVICE_ADDRESS              0x2A

/* Block Data Buffers
*/
    #define LEP_DATA_BUFFER_0_BASE_ADDR         0xF800
    #define LEP_DATA_BUFFER_1_BASE_ADDR         0xFC00


/* The Lepton I2C Registers Sub-Addresses
*/
    #define LEP_I2C_REG_BASE_ADDR               0x0000

    /* Host On Switch when camera is in stand by of off
    */
    #define LEP_I2C_POWER_REG                  (LEP_I2C_REG_BASE_ADDR + 0x0000 )

    /* Host Command Interface over I2C
    */
    #define LEP_I2C_STATUS_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0002 )
    #define LEP_I2C_COMMAND_REG                (LEP_I2C_REG_BASE_ADDR + 0x0004 )
    #define LEP_I2C_DATA_LENGTH_REG            (LEP_I2C_REG_BASE_ADDR + 0x0006 )
    #define LEP_I2C_DATA_0_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0008 )
    #define LEP_I2C_DATA_1_REG                 (LEP_I2C_REG_BASE_ADDR + 0x000A )
    #define LEP_I2C_DATA_2_REG                 (LEP_I2C_REG_BASE_ADDR + 0x000C )
    #define LEP_I2C_DATA_3_REG                 (LEP_I2C_REG_BASE_ADDR + 0x000E )
    #define LEP_I2C_DATA_4_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0010 )
    #define LEP_I2C_DATA_5_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0012 )
    #define LEP_I2C_DATA_6_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0014 )
    #define LEP_I2C_DATA_7_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0016 )
    #define LEP_I2C_DATA_8_REG                 (LEP_I2C_REG_BASE_ADDR + 0x0018 )
    #define LEP_I2C_DATA_9_REG                 (LEP_I2C_REG_BASE_ADDR + 0x001A )
    #define LEP_I2C_DATA_10_REG                (LEP_I2C_REG_BASE_ADDR + 0x001C )
    #define LEP_I2C_DATA_11_REG                (LEP_I2C_REG_BASE_ADDR + 0x001E )
    #define LEP_I2C_DATA_12_REG                (LEP_I2C_REG_BASE_ADDR + 0x0020 )
    #define LEP_I2C_DATA_13_REG                (LEP_I2C_REG_BASE_ADDR + 0x0022 )
    #define LEP_I2C_DATA_14_REG                (LEP_I2C_REG_BASE_ADDR + 0x0024 )
    #define LEP_I2C_DATA_15_REG                (LEP_I2C_REG_BASE_ADDR + 0x0026 )

    #define LEP_I2C_DATA_CRC_REG               (LEP_I2C_REG_BASE_ADDR + 0x0028 )

    #define LEP_I2C_DATA_BUFFER_0              (LEP_DATA_BUFFER_0_BASE_ADDR )
    #define LEP_I2C_DATA_BUFFER_0_END          (LEP_DATA_BUFFER_0_BASE_ADDR + 0x03FF )
    #define LEP_I2C_DATA_BUFFER_0_LENGTH        0x400

    #define LEP_I2C_DATA_BUFFER_1              (LEP_DATA_BUFFER_1_BASE_ADDR )
    #define LEP_I2C_DATA_BUFFER_1_END          (LEP_DATA_BUFFER_1_BASE_ADDR + 0x03FF )
    #define LEP_I2C_DATA_BUFFER_1_LENGTH        0x400

    #define LEP_I2C_STATUS_BUSY_BIT_MASK        0x0001   /* Bit 0 is the Busy Bit */


/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/

/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/


/******************************************************************************/
    #ifdef __cplusplus
}
    #endif

#endif  /* _LEPTON_I2C_REG_H_ */


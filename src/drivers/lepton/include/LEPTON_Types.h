/*******************************************************************************
**
**    File NAME: LEPTON_Types.h
**
**      AUTHOR:  pwolf
**
**      CREATED: 2/01/2011
**
**      DESCRIPTION: typedefs for coding style/conventions
**
**      HISTORY:  2/01/2012 PW - Initial Draft
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
#ifndef _LEPTON_TYPES_H_
    #define _LEPTON_TYPES_H_

    #ifdef __cplusplus
extern "C"
{
    #endif

    //  #define AVR_IAR_BUILD
/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/

    #ifdef AVR_IAR_BUILD
        #include "avr_compiler.h"
    #endif

    #if defined(_WIN32) || defined (_WIN64)
       #include <windows.h>
       #define WINDOWSS 1;
    #else
       #include <stdlib.h>
       #include <stdbool.h>
    #endif

    #include "omv_i2c.h"

/******************************************************************************/
/** EXPORTED DEFINES                                                         **/
/******************************************************************************/
    #define LEP_FAILURE    -1
    #define LEP_SUCCESS    0
    #define LEP_TRUE    1
    #define LEP_FALSE      0
    #define LEP_NULL    0

/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/

    #ifdef _STDINT_H
    typedef uint8_t             LEP_UINT8;
    typedef uint8_t             LEP_UCHAR;
    typedef int8_t              LEP_INT8;
    typedef char                LEP_CHAR8;

    typedef uint16_t            LEP_UINT16;
    typedef uint16_t            LEP_USHORT;
    typedef int16_t             LEP_INT16;
    typedef short               LEP_SHORT;

    typedef uint32_t            LEP_UINT32;
    typedef uint32_t            LEP_UINT;
    typedef int32_t             LEP_INT32;
    typedef int                 LEP_INT;

    typedef uint64_t            LEP_UINT64;
    typedef uint64_t            LEP_ULONG64;
    typedef uint32_t            LEP_ULONG32;
    typedef uint32_t            LEP_ULONG;
    typedef int64_t             LEP_LONG32;
    typedef long                LEP_LONG;

    typedef float               LEP_FLOAT32;
    typedef double              LEP_FLOAT64;
    #else
    typedef unsigned char       LEP_UINT8;
    typedef unsigned char       LEP_UCHAR;
    typedef signed char         LEP_INT8;
    typedef char                LEP_CHAR8;

    typedef unsigned short      LEP_UINT16;
    typedef unsigned short      LEP_USHORT;
    typedef signed short        LEP_INT16;
    typedef short               LEP_SHORT;

    typedef unsigned int        LEP_UINT32;
    typedef unsigned int        LEP_UINT;
    typedef signed int          LEP_INT32;
    typedef int                 LEP_INT;

    typedef unsigned long long  LEP_UINT64;
    typedef unsigned long long  LEP_ULONG64;
    typedef unsigned long       LEP_ULONG32;
    typedef unsigned long       LEP_ULONG;
    typedef signed long         LEP_LONG32;
    typedef long                LEP_LONG;

    typedef float               LEP_FLOAT32;
    typedef double              LEP_FLOAT64;
    #endif


    #ifdef _STDBOOL_H
    typedef bool                LEP_BOOL, *LEP_BOOL_PTR;
    #else
    typedef unsigned char       LEP_BOOL, *LEP_BOOL_PTR;
    #endif

    /* NULL
    */
    #ifndef NULL
        #define NULL '\0'
    #endif


    typedef LEP_UINT16          LEP_COMMAND_ID;
    typedef LEP_UINT16          LEP_ATTRIBUTE_T,*LEP_ATTRIBUTE_T_PTR;

    #define LEP_GET_TYPE        0x0000
    #define LEP_SET_TYPE        0x0001
    #define LEP_RUN_TYPE        0x0002

    typedef enum
    {
        LEP_LSB_FIRST=0,
        LEP_MSB_FIRST,

        LEP_BYTE_ORDER_MAKE_32_BIT_ENUM = 0x7FFFFFFF
    }LEP_BYTE_ORDER_T, *LEP_BYTE_ORDER_T_PTR;

    typedef enum
    {
        LEP_READY = 0,
        LEP_BUSY,
        LEP_WAITING,

        LEP_OPERATION_STATE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
    }LEP_OPERATION_STATE;

    typedef enum
    {
        LEP_DISABLED = 0,
        LEP_ENABLED,

        LEP_ENABLE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
    }LEP_ENABLE_STATE;

    typedef enum
    {
        LEP_OFF = 0,
        LEP_ON,

        LEP_ON_STATE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
    }LEP_ON_STATE;


    /* Lepton physical tranport interfaces
    */
    typedef enum LEP_CAMERA_PORT_E_TAG
    {
        LEP_CCI_TWI=0,
        LEP_CCI_SPI,
        LEP_END_CCI_PORTS,

        LEP_CCI_PORTS_MAKE_32_BIT_ENUM = 0x7FFFFFFF
    }LEP_CAMERA_PORT_E, *LEP_CAMERA_PORT_E_PTR;

    /* Device entries
    */
    typedef enum LEP_PROTOCOL_DEVICE_E_TAG
    {
        /* I2C Devices */
        AARDVARK_I2C = 0,
        DEV_BOARD_FTDI_V2,
        //C232HM_DDHSL_0,
      TCP_IP,

        /* SPI Devices */

        LEP_END_PROTOCOL_DEVICE,

        LEP_PROTOCOL_DEVICE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
    } LEP_PROTOCOL_DEVICE_E, *LEP_PROTOCOL_DEVICE_E_PTR;

    /* Lepton supported TWI  clock rates
    */
    typedef enum LEP_TWI_CLOCK_RATE_T_TAG
    {
        LEP_TWI_CLOCK_100KHZ=0,
        LEP_TWI_CLOCK_400KHZ,
        LEP_TWI_CLOCK_1MHZ,
        LEP_END_TWI_CLOCK_RATE,

        LEP_TWI_CLOCK_RATE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
    }LEP_TWI_CLOCK_RATE_T, *LEP_TWI_CLOCK_RATE_T_PTR;

    /* Lepton supported SPI  clock rates
    */
    typedef enum LEP_SPI_CLOCK_RATE_T_TAG
    {
        LEP_SPI_CLOCK_2MHZ=0,
        LEP_SPI_CLOCK_10MHZ,
        LEP_SPI_CLOCK_20MHZ,
        LEP_END_SPI_CLOCK_RATE,

        LEP_SPI_CLOCK_RATE_MAKE_32_BIT_ENUM = 0x7FFFFFFF
    }LEP_SPI_CLOCK_RATE_T, *LEP_SPI_CLOCK_RATE_T_PTR;

    /* Communications Port Descriptor Type
    */
    typedef struct  LEP_CAMERA_PORT_DESC_T_TAG
    {
        omv_i2c_t *bus;
        LEP_CAMERA_PORT_E   portType;
        LEP_UINT16  portBaudRate;
        LEP_UINT8 deviceAddress;
    }LEP_CAMERA_PORT_DESC_T, *LEP_CAMERA_PORT_DESC_T_PTR;


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
#endif   // _FLIR_TYPES_H_

/*******************************************************************************
**
**    File NAME: LEPTON_Macros.h
**
**      AUTHOR:  David Dart
**
**      CREATED: 7/11/2012
**
**      DESCRIPTION:
**
**      HISTORY:  7/11/2012 DWD - Initial Draft
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
#ifndef _LEP_MACROS_H_
    #define _LEP_MACROS_H_

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

/******************************************************************
***    USEFUL MACROS                                            ***
******************************************************************/

    #ifndef MIN
        #define MIN(a, b) ((a) < (b)? (a): (b))
    #endif
    #ifndef MAX
        #define MAX(a, b) ((a) > (b)? (a): (b))
    #endif

    #ifndef LOW_WORD
        #define LOW_WORD(longVariable) ((LEP_UINT16)longVariable)
    #endif
    #ifndef HIGH_WORD
        #define HIGH_WORD(longVariable) ((LEP_UINT16)(longVariable>>16))
    #endif
    #ifndef LOW_BYTE
        #define LOW_BYTE(w)           ((LEP_UINT8)(w))
    #endif
    #ifndef HIGH_BYTE
        #define HIGH_BYTE(w)           ((LEP_UINT8)(((w) >> 8) & 0xFF))
    #endif

    #ifndef LOW_NIBBLE
        #define LOW_NIBBLE(w)           ((LEP_UINT8)(w) & 0x0F)
    #endif
    #ifndef HIGH_NIBBLE
        #define HIGH_NIBBLE(w)           ((LEP_UINT8)(((w) >> 4) & 0x0F))
    #endif

    #define CLR_BIT(_port,_bit)         ((_port) & ~(_bit))


    #define REVERSE_ENDIENESS_UINT16(uint16Var) \
           ( ( ((LEP_UINT16)LOW_BYTE(uint16Var))<<8) + (LEP_UINT16)HIGH_BYTE(uint16Var))

    #define REVERSE_ENDIENESS_UINT32(uint32Var) \
           ( ((LEP_UINT32)REVERSE_ENDIENESS_UINT16(LOW_WORD(uint32Var)) << 16) + \
             (LEP_UINT32)REVERSE_ENDIENESS_UINT16(HIGH_WORD(uint32Var) ) )

    #define REVERSE_NIBBLE_UINT8(uint8Var) \
           ( ( ((LEP_UINT8)LOW_NIBBLE(uint8Var))<<4) + (LEP_UINT8)HIGH_NIBBLE(uint8Var))

    #define REVERSE_BYTEORDER_UINT32(uint32Var) \
           ( (((LEP_UINT32)LOW_BYTE(uint32Var))<<24) + (((LEP_UINT32)HIGH_BYTE(uint32Var))<<16) + \
             (((LEP_UINT32)LOW_BYTE(HIGH_WORD(uint32Var)))<<8) + (LEP_UINT32)HIGH_BYTE(HIGH_WORD(uint32Var)) )

    #define WORD_SWAP_16(uint32Var)  \
            ( ((LEP_UINT16)LOW_WORD(uint32Var) << 16) + ((LEP_UINT16)HIGH_WORD(uint32Var)) )


    #ifndef NUM_ELEMENTS_IN_ARRAY
        #define NUM_ELEMENTS_IN_ARRAY(array) (sizeof (array) / sizeof ((array) [0]))
    #endif /* NUM_ELEMENTS_IN_ARRAY */

    #ifndef NELEMENTS
        #define NELEMENTS(array)      /* number of elements in an array */ \
              (sizeof (array) / sizeof ((array) [0]))
    #endif /* NELEMENTS */


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

#endif  /* _LEP_MACROS_H_ */


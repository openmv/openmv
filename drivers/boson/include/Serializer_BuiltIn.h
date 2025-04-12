/******************************************************************************/
/*                                                                            */
/*  Copyright (C) 2018, FLIR Systems                                          */
/*  All rights reserved.                                                      */
/*                                                                            */
/*  This document is controlled to FLIR Technology Level 2. The information   */
/*  contained in this document pertains to a dual use product controlled for  */
/*  export by the Export Administration Regulations (EAR). Diversion contrary */
/*  to US law is prohibited. US Department of Commerce authorization is not   */
/*  required prior to export or transfer to foreign persons or parties unless */
/*  otherwise prohibited.                                                     */
/*                                                                            */
/******************************************************************************/

#ifndef BOSON_BUILTIN_HANDLERS_H
#define BOSON_BUILTIN_HANDLERS_H

#include <stdint.h>

extern void byteToBOOL(const uint8_t*inBuff, _Bool *outVal);
extern void byteToCHAR(const uint8_t*inBuff, int8_t *outVal);
extern void byteToUCHAR(const uint8_t*inBuff, uint8_t *outVal);
extern void byteToINT_16(const uint8_t*inBuff, int16_t *outVal);
extern void byteToUINT_16(const uint8_t*inBuff, uint16_t *outVal);
extern void byteToINT_32(const uint8_t*inBuff, int32_t *outVal);
extern void byteToUINT_32(const uint8_t*inBuff, uint32_t *outVal);
extern void byteToFLOAT(const uint8_t*inBuff, float *outVal);
extern void byteToDOUBLE(const uint8_t*inBuff, double *outVal);

extern void byteToBOOLArray(const uint8_t*inBuff, _Bool *outVal,uint16_t length);
extern void byteToCHARArray(const uint8_t*inBuff, int8_t *outVal,uint16_t length);
extern void byteToUCHARArray(const uint8_t*inBuff, uint8_t *outVal,uint16_t length);
extern void byteToINT_16Array(const uint8_t*inBuff, int16_t *outVal,uint16_t length);
extern void byteToUINT_16Array(const uint8_t*inBuff, uint16_t *outVal,uint16_t length);
extern void byteToINT_32Array(const uint8_t*inBuff, int32_t *outVal,uint16_t length);
extern void byteToUINT_32Array(const uint8_t*inBuff, uint32_t *outVal,uint16_t length);
extern void byteToFLOATArray(const uint8_t*inBuff, float *outVal,uint16_t length);
extern void byteToDOUBLEArray(const uint8_t*inBuff, double *outVal,uint16_t length);

extern void BOOLToByte(const _Bool inVal, const uint8_t *outBuff); 
extern void CHARToByte(const int8_t inVal, const uint8_t *outBuff );
extern void UCHARToByte(const uint8_t inVal, const uint8_t *outBuff);
extern void INT_16ToByte(const int16_t inVal, const uint8_t *outBuff);
extern void UINT_16ToByte(const uint16_t inVal, const uint8_t *outBuff);
extern void INT_32ToByte(const int32_t inVal, const uint8_t *outBuff);
extern void UINT_32ToByte(const uint32_t inVal, const uint8_t *outBuff);
extern void FLOATToByte(const float inVal, const uint8_t *outBuff);
extern void DOUBLEToByte(const double inVal, const uint8_t *outBuff);

extern void BOOLArrayToByte(const _Bool *inVal, uint16_t length, const uint8_t *outBuff); 
extern void CHARArrayToByte(const int8_t *inVal, uint16_t length, const uint8_t *outBuff );
extern void UCHARArrayToByte(const uint8_t *inVal, uint16_t length, const uint8_t *outBuff);
extern void INT_16ArrayToByte(const int16_t *inVal, uint16_t length, const uint8_t *outBuff);
extern void UINT_16ArrayToByte(const uint16_t *inVal, uint16_t length, const uint8_t *outBuff);
extern void INT_32ArrayToByte(const int32_t *inVal, uint16_t length, const uint8_t *outBuff);
extern void UINT_32ArrayToByte(const uint32_t *inVal, uint16_t length, const uint8_t *outBuff);
extern void FLOATArrayToByte(const float *inVal, uint16_t length, const uint8_t *outBuff);
extern void DOUBLEArrayToByte(const double *inVal, uint16_t length, const uint8_t *outBuff);


#endif //BOSON_BUILTIN_HANDLERS_H
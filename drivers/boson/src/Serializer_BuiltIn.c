/******************************************************************************/
/*                                                                            */
/*  Copyright (C) 2017, FLIR Systems                                          */
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

#include "Serializer_BuiltIn.h"
//#include <stdio.h>

void byteToBOOL(const uint8_t *inBuff, _Bool *outVal){
	*outVal = (_Bool) *inBuff;
}
void byteToBOOLArray(const uint8_t *inBuff, _Bool *outVal,uint16_t length){
	uint16_t i;
	for (i=0;i<length;i++) {
		byteToBOOL(inBuff+i,&(outVal[i]));
	}	
}

void byteToCHAR(const uint8_t *inBuff, int8_t *outVal){
	*outVal = (int8_t) *inBuff;
}
void byteToCHARArray(const uint8_t *inBuff, int8_t *outVal,uint16_t length){
	uint16_t i;
	for (i=0;i<length;i++) {
		byteToCHAR(inBuff+i,&(outVal[i]));
	}	
}

void byteToUCHAR(const uint8_t *inBuff, uint8_t *outVal){
	*outVal = (uint8_t) *inBuff;
}
void byteToUCHARArray(const uint8_t *inBuff, uint8_t *outVal,uint16_t length){
	uint16_t i;
	for (i=0;i<length;i++) {
		byteToUCHAR(inBuff+i,&(outVal[i]));
	}	
}

void byteToINT_16(const uint8_t *inBuff, int16_t *outVal){
	uint8_t *ptr = (uint8_t *)inBuff;
	*outVal = (int16_t) *ptr++ << 8;
	*outVal = *outVal | *ptr;
}
void byteToINT_16Array(const uint8_t *inBuff, int16_t *outVal,uint16_t length){
	uint16_t i;
	for (i=0;i<length;i++) {
		byteToINT_16(inBuff+(i*2),&(outVal[i]));
	}	
}

void byteToUINT_16(const uint8_t *inBuff, uint16_t *outVal){
	uint8_t *ptr = (uint8_t *)inBuff;
	*outVal = (uint16_t) *ptr++ << 8;
	*outVal = *outVal | *ptr;
}
void byteToUINT_16Array(const uint8_t *inBuff, uint16_t *outVal,uint16_t length){
	uint16_t i;
	for (i=0;i<length;i++) {
		byteToUINT_16(inBuff+(i*2),&(outVal[i]));
	}	
}

void byteToINT_32(const uint8_t *inBuff, int32_t *outVal){
	uint8_t *ptr = (uint8_t *)inBuff;
	*outVal = (int32_t) (*ptr++)<<24;
	*outVal |= ((int32_t) (*ptr++)<<16);
	*outVal |= ((int32_t) (*ptr++)<<8);
	*outVal |=*ptr;
}	
void byteToINT_32Array(const uint8_t *inBuff, int32_t *outVal,uint16_t length){
	uint16_t i;
	for (i=0;i<length;i++) {
		byteToINT_32(inBuff+(i*4),&(outVal[i]));
	}	
}

void byteToUINT_32(const uint8_t *inBuff, uint32_t *outVal){
	uint8_t *ptr = (uint8_t *)inBuff;
	*outVal = (uint32_t) (*ptr++)<<24;
	*outVal |= ((uint32_t) (*ptr++)<<16);
	*outVal |= ((uint32_t) (*ptr++)<<8);
	*outVal |= *ptr;
}	
void byteToUINT_32Array(const uint8_t *inBuff, uint32_t *outVal,uint16_t length){
	uint16_t i;
	for (i=0;i<length;i++) {
		byteToUINT_32(inBuff+(i*4),&(outVal[i]));
	}	
}

void byteToFLOAT(const uint8_t *inBuff, float *outVal){
	uint8_t *ptr = (uint8_t *)inBuff;
	uint32_t tempVal = 0;
	tempVal = (uint32_t) (*ptr++)<<24;
	tempVal |= ((uint32_t) (*ptr++)<<16);
	tempVal |= ((uint32_t) (*ptr++)<<8);
	tempVal |= *ptr;
	*outVal = *((float *) (&tempVal));
}	
void byteToFLOATArray(const uint8_t *inBuff, float *outVal,uint16_t length){
	uint16_t i;
	for (i=0;i<length;i++) {
		byteToFLOAT(inBuff+(i*4),&(outVal[i]));
	}	
}

void byteToDOUBLE(const uint8_t *inBuff, double *outVal){
	uint8_t *ptr = (uint8_t *)inBuff;
	uint64_t tempVal = 0;
	tempVal = (uint64_t) (*ptr++)<<56;
	tempVal |= ((uint64_t) (*ptr++)<<48);
	tempVal |= ((uint64_t) (*ptr++)<<40);
	tempVal |= ((uint64_t) (*ptr++)<<32);
	tempVal |= ((uint64_t) (*ptr++)<<24);
	tempVal |= ((uint64_t) (*ptr++)<<16);
	tempVal |= ((uint64_t) (*ptr++)<<8);
	tempVal |= *ptr;
	*outVal = *((double *) (&tempVal));
}
void byteToDOUBLEArray(const uint8_t *inBuff, double *outVal,uint16_t length){
	uint16_t i;
	for (i=0;i<length;i++) {
		byteToDOUBLE(inBuff+(i*8),&(outVal[i]));
	}	
}



void BOOLToByte(const _Bool inVal, const uint8_t *outBuff){
	uint8_t *outPtr = (uint8_t *)outBuff;
	*outPtr = *((uint8_t *) (&inVal));
}
void BOOLArrayToByte(const _Bool *inVal,uint16_t length, const uint8_t *outBuff){
	uint16_t i;
	for (i=0;i<length;i++){
		BOOLToByte(inVal[i],outBuff+i);
	}
}

void CHARToByte(const int8_t inVal, const uint8_t *outBuff ){
	uint8_t *outPtr = (uint8_t *)outBuff;
	*outPtr = *((uint8_t *) (&inVal));
}
void CHARArrayToByte(const int8_t *inVal,uint16_t length, const uint8_t *outBuff){
	uint16_t i;
	for (i=0;i<length;i++){
		CHARToByte(inVal[i],outBuff+i);
	}
}

void UCHARToByte(const uint8_t inVal, const uint8_t *outBuff){
	uint8_t *outPtr = (uint8_t *)outBuff;
	*outPtr = inVal;
}
void UCHARArrayToByte(const uint8_t *inVal,uint16_t length, const uint8_t *outBuff){
	uint16_t i;
	for (i=0;i<length;i++){
		UCHARToByte(inVal[i],outBuff+i);
	}
}

void INT_16ToByte(const int16_t inVal, const uint8_t *outBuff){
	uint16_t tempBytes = *((uint16_t *) (&inVal));
	uint8_t *outPtr = (uint8_t *)outBuff;
	*outPtr++ = (tempBytes>>8 & 0xff);
	*outPtr = (uint8_t) (tempBytes & 0xff);
}
void INT_16ArrayToByte(const int16_t *inVal,uint16_t length, const uint8_t *outBuff){
	uint16_t i;
	for (i=0;i<length;i++){
		INT_16ToByte(inVal[i],outBuff+(i*2));
	}
}

void UINT_16ToByte(const uint16_t inVal, const uint8_t *outBuff){
	uint8_t *outPtr = (uint8_t *)outBuff;
	*outPtr++ = (uint8_t) (inVal>>8 & 0xff);
	*outPtr = (uint8_t) (inVal & 0xff);
}
void UINT_16ArrayToByte(const uint16_t *inVal,uint16_t length, const uint8_t *outBuff){
	uint16_t i;
	for (i=0;i<length;i++){
		UINT_16ToByte(inVal[i],outBuff+(i*2));
	}
}

void INT_32ToByte(const int32_t inVal, const uint8_t *outBuff){
	uint32_t tempBytes = *((uint32_t *) (&inVal));
	uint8_t *outPtr = (uint8_t *)outBuff;
	*outPtr++ = (uint8_t) (tempBytes>>24 & 0xff);
	*outPtr++ = (uint8_t) (tempBytes>>16 & 0xff);
	*outPtr++ = (uint8_t) (tempBytes>>8 & 0xff);
	*outPtr = (uint8_t) (tempBytes & 0xff);
}
void INT_32ArrayToByte(const int32_t *inVal,uint16_t length, const uint8_t *outBuff){
	uint16_t i;
	for (i=0;i<length;i++){
		INT_32ToByte(inVal[i],outBuff+(i*4));
	}
}

void UINT_32ToByte(const uint32_t inVal, const uint8_t *outBuff){
	uint8_t *outPtr = (uint8_t *)outBuff;
	*outPtr++ = (uint8_t) (inVal>>24 & 0xff);
	*outPtr++ = (uint8_t) (inVal>>16 & 0xff);
	*outPtr++ = (uint8_t) (inVal>>8 & 0xff);
	*outPtr = (uint8_t) (inVal & 0xff);
}
void UINT_32ArrayToByte(const uint32_t *inVal,uint16_t length, const uint8_t *outBuff){
	uint16_t i;
	for (i=0;i<length;i++){
		UINT_32ToByte(inVal[i],outBuff+(i*4));
	}
}

void FLOATToByte(const float inVal, const uint8_t *outBuff){
	uint32_t tempBytes = *((uint32_t *) (&inVal));
	uint8_t *outPtr = (uint8_t *)outBuff;
	*outPtr++ = (uint8_t) (tempBytes>>24 & 0xff);
	*outPtr++ = (uint8_t) (tempBytes>>16 & 0xff);
	*outPtr++ = (uint8_t) (tempBytes>>8 & 0xff);
	*outPtr = (uint8_t) (tempBytes & 0xff);
}
void FLOATArrayToByte(const float *inVal,uint16_t length, const uint8_t *outBuff){
	uint16_t i;
	for (i=0;i<length;i++){
		FLOATToByte(inVal[i],outBuff+(i*4));
	}
}

void DOUBLEToByte(const double inVal, const uint8_t *outBuff){
	uint64_t tempBytes = *((uint64_t *) (&inVal));
	uint8_t *outPtr = (uint8_t *)outBuff;
	*outPtr++ = (uint8_t) (tempBytes>>56 & 0xff);
	*outPtr++ = (uint8_t) (tempBytes>>48 & 0xff);
	*outPtr++ = (uint8_t) (tempBytes>>40 & 0xff);
	*outPtr++ = (uint8_t) (tempBytes>>32 & 0xff);
	*outPtr++ = (uint8_t) (tempBytes>>24 & 0xff);
	*outPtr++ = (uint8_t) (tempBytes>>16 & 0xff);
	*outPtr++ = (uint8_t) (tempBytes>>8 & 0xff);
	*outPtr = (uint8_t) (tempBytes & 0xff);
}
void DOUBLEArrayToByte(const double *inVal,uint16_t length, const uint8_t *outBuff){
	uint16_t i;
	for (i=0;i<length;i++){
		DOUBLEToByte(inVal[i],outBuff+(i*8));
	}
}

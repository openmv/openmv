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

#ifndef _FLIR_CRC_H_
#define _FLIR_CRC_H_

#include <stdint.h>

#define FLIR_CRC_INITIAL_VALUE      (0x1D0F)

uint16_t calcFlirCRC16Words(unsigned int count, short *buffer);
uint16_t calcFlirCRC16Bytes(unsigned int count, uint8_t *buffer);
int calcFlirByteCRC16(int value, int crcin);
#endif // _FLIR_CRC_H_

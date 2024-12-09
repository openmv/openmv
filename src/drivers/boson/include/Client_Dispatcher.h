//  /////////////////////////////////////////////////////
//  // DO NOT EDIT.  This is a machine generated file. //
//  /////////////////////////////////////////////////////
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

#ifndef CLIENT_DISPATCHER_H
#define CLIENT_DISPATCHER_H

#include <stdint.h>
#include "ReturnCodes.h"
#include "Serializer_Struct.h"
#include "FunctionCodes.h"
#include "Client_Interface.h"

FLR_RESULT CLIENT_dispatcher_Tx(uint32_t seqNum, FLR_FUNCTION fnID, const uint8_t *sendData, const uint32_t sendBytes, const uint8_t *receiveData, uint32_t *receiveBytes);
FLR_RESULT CLIENT_dispatcher_Rx(uint32_t *seqNum, uint32_t *fnID, const uint8_t *sendData, const uint32_t sendBytes, const uint8_t *receiveData, uint32_t *receiveBytes);
FLR_RESULT CLIENT_dispatcher(uint32_t seqNum, FLR_FUNCTION fnID, const uint8_t *sendData, const uint32_t sendBytes, const uint8_t *receiveData, uint32_t *receiveBytes);
FLR_RESULT CheckReadyDataCommandId(uint32_t receiveBytes, const uint8_t *receiveData, uint32_t *commandId);


#endif

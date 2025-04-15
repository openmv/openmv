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

#ifndef MULTI_SERVICE_SUPPORT_H
#define MULTI_SERVICE_SUPPORT_H

#include <stdio.h>
#include <stdint.h>
#include "ReturnCodes.h"

typedef int32_t svcDataReadyCallback_t(uint32_t receiveBytes, const uint8_t* receiveData);
#define MAX_SERVICES (10)

FLR_RESULT RegisterServiceDataCallback(uint8_t svc, svcDataReadyCallback_t *callback);
/*
 * This should be called as ofter as possible, but by one thread only.
 * It should also be the only place where the port is ever read.
 */
FLR_RESULT CheckServiceDataReady(void);

#endif // MULTI_SERVICE_SUPPORT_H

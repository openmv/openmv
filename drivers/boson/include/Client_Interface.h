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

#ifndef CLIENT_INTERFACE_H
#define CLIENT_INTERFACE_H

#include <stdint.h>
#include <stdlib.h>
#include "ReturnCodes.h"

FLR_RESULT CLIENT_interface_readFrame(uint8_t *readData, uint32_t *readBytes);
FLR_RESULT CLIENT_interface_writeFrame(uint8_t *writeData, uint32_t writeBytes);

#endif

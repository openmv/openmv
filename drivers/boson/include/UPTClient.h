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

#ifndef UPT_CLIENT_H
#define UPT_CLIENT_H

#include <stdio.h>
#include <stdint.h>
#include "ReturnCodes.h"

enum _uptRecipient {
    UPT_USB_HOST = (int32_t) 0,
    UPT_UART_HOST = (int32_t) 1,

    UPT_HOST_COUNT,
};

typedef enum _uptRecipient UPT_RECIPIENT;

#define MAX_PAYLOAD_BYTES      (768 - 1)

FLR_RESULT passToRecipient(UPT_RECIPIENT recipient, const uint8_t *data, uint32_t length);
FLR_RESULT getMyData(uint8_t *data, uint32_t *length);

#endif // UPT_CLIENT_H

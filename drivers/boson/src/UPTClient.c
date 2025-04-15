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

#include <string.h>

#include "UART_Connector.h"
#include "UPTClient.h"

uint8_t sendPayload[1 + MAX_PAYLOAD_BYTES];

FLR_RESULT passToRecipient(UPT_RECIPIENT recipient, const uint8_t *data, uint32_t length)
{
    if (!data)
        return FLR_ERROR;

    if (length > MAX_PAYLOAD_BYTES)
        return FLR_DATA_SIZE_ERROR;

    sendPayload[0] = (uint8_t)recipient;

    memcpy(&sendPayload[1], data, length);

    SendToCamera(0xaa, (length + 1), sendPayload);

    return R_SUCCESS;
}

FLR_RESULT getMyData(uint8_t *data, uint32_t *length)
{
    if (!data || !length)
        return FLR_ERROR;

    ReadFrame(0xaa, length, data);

    return R_SUCCESS;
}

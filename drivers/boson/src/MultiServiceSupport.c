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


#include "MultiServiceSupport.h"
#include "UART_Connector.h"

typedef struct {
    uint8_t svc;
    svcDataReadyCallback_t *callback;
} CALLBACK_CONFIG;

#define SERVICE_UNRELATED_SVC_ID    (0xff)
static CALLBACK_CONFIG callbacks[MAX_SERVICES];

FLR_RESULT RegisterServiceDataCallback(uint8_t svc, svcDataReadyCallback_t *callback)
{
    int i;
    if (!callback)
        return FLR_ERROR;
    
    for (i = 0; i < MAX_SERVICES; i++) {
        if (callbacks[i].callback == 0) {
            callbacks[i].callback = callback;
            callbacks[i].svc = svc;
            break;
        }
    }
    if (i == MAX_SERVICES)
        return FLR_RANGE_ERROR;

    return R_SUCCESS;
}

FLR_RESULT CheckServiceDataReady(void)
{
    const uint8_t *rawData = 0;
    uint32_t length = 0;
    uint8_t svc = 0;
    int32_t ret = CheckDataReady(&svc, &length, &rawData);
    if (ret < 0)
        return FLR_ERROR;

    if (length) {
        int i;
        for (i = 0; i < MAX_SERVICES; i++) {
            if ((callbacks[i].svc == svc) && (callbacks[i].callback)) {
                ret = callbacks[i].callback(length, rawData);
                if (ret) printf("Error in callback for svc [0x%02x] : [%ld]\n", callbacks[i].svc, ret);
            }
        }
    }
    return R_SUCCESS;
}

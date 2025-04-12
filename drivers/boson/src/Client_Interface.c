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

#include "Client_Interface.h"

#ifdef USE_I2C_SLAVE_CP
#include "I2C_Connector.h"
#else
#include "UART_Connector.h"
static uint8_t CommandChannel = 0x00;
#endif

FLR_RESULT CLIENT_interface_readFrame(uint8_t *readData, uint32_t *readBytes)
{
    if(readData == NULL || readBytes == NULL)
        return FLR_BAD_ARG_POINTER_ERROR;

#ifdef USE_I2C_SLAVE_CP
    if(I2C_readFrame(readData, readBytes) != FLR_OK)
        return FLR_COMM_ERROR_READING_COMM;
#else
    ReadFrame(CommandChannel, readBytes, readData);
#endif
    return FLR_OK;
}

FLR_RESULT CLIENT_interface_writeFrame(uint8_t *writeData, uint32_t writeBytes)
{
    if(writeData == NULL)
        return FLR_BAD_ARG_POINTER_ERROR;

#ifdef USE_I2C_SLAVE_CP
    if(I2C_writeFrame(writeData, writeBytes) != FLR_OK)
        return FLR_COMM_ERROR_WRITING_COMM;
#else
    SendToCamera(CommandChannel, writeBytes, writeData);
#endif
    return FLR_OK;
}

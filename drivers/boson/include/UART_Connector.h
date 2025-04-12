/******************************************************************************/
/*                                                                            */
/*  Copyright (C) 2015, FLIR Systems                                          */
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

#ifndef UART_CONNECTOR_H
#define UART_CONNECTOR_H

#include <stdint.h>
#include "ReturnCodes.h"

void ReadTimeoutSet(unsigned int timeout);
void SendToCamera( uint8_t channelID,  uint32_t sendBytes, uint8_t *sendData);
void ReadFrame( uint8_t channelID, uint32_t *receiveBytes, uint8_t *receiveData);
void ReadUnframed(uint32_t *receiveBytes, uint8_t *receiveData);
FLR_RESULT Initialize();
void Close();
int32_t CheckDataReady(uint8_t *channel_ID, uint32_t *receiveBytes, const uint8_t **receiveData);

#endif //UART_CONNECTOR_H

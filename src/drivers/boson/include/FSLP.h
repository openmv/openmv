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

#ifndef _FSLP_H
#define _FSLP_H

#include <stdint.h>

#include "serialPortAdapter.h"

void FSLP_send_to_camera(uint8_t channel_ID, uint32_t sendBytes, uint8_t *sendPayload);//, uint32_t *receiveBytes, uint8_t *receivePayload);
// void read_command(uint8_t channel_ID, uint32_t sendBytes, uint8_t *sendPayload, uint32_t *receiveBytes, uint8_t *receivePayload);
int32_t FSLP_read_frame(uint8_t channel_ID, uint16_t start_byte_ms,uint32_t *receiveBytes, uint8_t *receiveBuffer);
void FSLP_read_unframed(uint16_t start_byte_ms,uint32_t *receiveBytes, uint8_t *receiveBuffer);
int32_t FSLP_check_data_ready(uint8_t *channel_ID, uint16_t start_byte_ms, uint32_t *receiveBytes, const uint8_t **receiveBuffer);

#endif //_FSLP_H

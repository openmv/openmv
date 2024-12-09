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

#include "flirChannels.h"

static uint8_t is_initialized = 0;

int32_t get_channel(uint8_t channel_ID, CHANNEL_T **return_channel){
	int i;
	for (i = 1; i < NUM_CHANNELS; i++){
		if (channel_ID == channel_list[i].channel) {
			*return_channel = &(channel_list[i]);
			return 0;
		}
	}
	*return_channel = 0;
	return -1;
}

void get_unframed(CHANNEL_T **return_channel){
	*return_channel = &(channel_list[0]);
	return;
}

void add_byte(uint8_t inbyte,CHANNEL_T *channel_ptr){
	uint32_t index;
	uint16_t start = (channel_ptr->start);
	if (channel_ptr->len != CHANNEL_BUF_SIZ){
		index = start + (channel_ptr->len);
		(channel_ptr->buff)[index] = inbyte;
		(channel_ptr->len)++;
	} else {
		(channel_ptr->buff)[start] = inbyte;
		(channel_ptr->start) = (start + 1)%CHANNEL_BUF_SIZ;
	}
}

int32_t get_byte(uint8_t *outbyte,CHANNEL_T *channel_ptr){
	//return remaining length if success, -1 if channel already empty
	if (channel_ptr->len == 0) {
		return -1;
	} else {
		*outbyte = (channel_ptr->buff)[(channel_ptr->start)];
		(channel_ptr->start)++;
		return --(channel_ptr->len);
	}
}

void initialize_channels(){
	if (is_initialized==0) {
                CHANNEL_INIT(0, 0x00); // unframed "channel" always slot 0
                CHANNEL_INIT(1, 0x00); // command channel
                CHANNEL_INIT(2, 0x99); // "0x99" debug channel
                CHANNEL_INIT(3, 0x63); // "99" alt debug channel
                CHANNEL_INIT(4, 0xaa); // "0xaa" Uart Pass Through channel
                CHANNEL_INIT(5, 0x01); // "1" Data channel

                CHANNEL_INIT(6, SERVICE_UNRELATED_CHANNEL_ID); /*
                                                                * all-in - allows for framed reads of whatever has
                                                                * collected in the port (unrelated to any service)
                                                                */
		is_initialized = 1;
	}
}

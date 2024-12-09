
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

#include "UART_Connector.h"
#include "serialPortAdapter.h"
#include "FSLP.h"

/* Defaults to 1000ms, can be increased by the user for any commands that take
   longer to respond... */
static int readTimeout = 1000;

FLR_RESULT Initialize()
{
	if (FSLP_open_port()) return R_UART_PORT_FAILURE;
	return FLR_COMM_OK; // 0 == success.
}

void Close()
{
	FSLP_close_port();
}

void SendToCamera( uint8_t channelID,  uint32_t sendBytes, uint8_t *sendData)
{
	FSLP_send_to_camera(channelID, sendBytes, sendData);
}

void ReadFrame(uint8_t channelID, uint32_t *receiveBytes, uint8_t *receiveData)
{
	FSLP_read_frame(channelID, readTimeout, receiveBytes, receiveData);
}

void ReadUnframed(uint32_t *receiveBytes, uint8_t *receiveData)
{
	// hardcoded 25ms polling delay for now
	FSLP_read_unframed(25, receiveBytes,receiveData);
}

void ReadTimeoutSet(unsigned int timeout)
{
	readTimeout = timeout;
}

int32_t CheckDataReady(uint8_t *channel_ID, uint32_t *receiveBytes, const uint8_t **receiveData)
{
	return FSLP_check_data_ready(channel_ID, readTimeout, receiveBytes, receiveData);
}

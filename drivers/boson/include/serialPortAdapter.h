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

#ifndef _SERIALPORT_ADAPTER_H
#define _SERIALPORT_ADAPTER_H

#include <stdlib.h>
#include <stdint.h>
#include "omv_csi.h"

// Set the omv_csi_t pointer to use for serial port operations.
void FSLP_set_csi(omv_csi_t *csi);

// open port by id, using specified baud_rate.
// passes library errors up, 0 on success.
uint8_t FSLP_open_port();

// close port via id
void FSLP_close_port();

// read single byte with timeout.
// -1 on timeout, 0-255 for valid byte value
int16_t FSLP_read_byte_with_timeout(double timeout);

// flush tx buffer
void FSLP_flush_write_queue();

// write buffer of bytes
// return number of bytes written
int32_t FSLP_write_buffer(uint8_t *frame_buf, int32_t len);

#endif //_SERIALPORT_ADAPTER_H

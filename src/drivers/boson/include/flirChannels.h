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

#ifndef FLIR_CHANNELS_H
#define FLIR_CHANNELS_H

#include <stdint.h>
#define CHANNEL_BUF_SIZ       1

#define SERVICE_UNRELATED_CHANNEL_ID    (0xff)

struct t_CHANNEL_T {
    uint8_t channel;
    uint16_t start;
    uint16_t len;
    uint8_t buff[CHANNEL_BUF_SIZ];
};
typedef struct t_CHANNEL_T CHANNEL_T;

#define CHANNEL_INIT(_channel_list_index, _channel_id)    \
    do {                                                  \
        chan_ptr = &(channel_list[_channel_list_index]);  \
        chan_ptr->channel = (_channel_id);                \
        chan_ptr->len = 0;                                \
        chan_ptr->start = 0;                              \
    } while (0)


#define NUM_CHANNELS 7

static CHANNEL_T *chan_ptr;
static CHANNEL_T *unframed_ptr;
static CHANNEL_T channel_list[NUM_CHANNELS];

extern void initialize_channels();
extern int32_t get_channel(uint8_t channel_ID, CHANNEL_T **return_channel);
extern void get_unframed(CHANNEL_T **return_channel);
extern void add_byte(uint8_t inbyte,CHANNEL_T *channel_ptr);
extern int32_t get_byte(uint8_t *outbyte,CHANNEL_T *channel_ptr);

/* Maybe later if number of channels becomes large.
int16_t channel_nums[256] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x00->0x09
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x0A->0x13
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x14->0x1D
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x1E->0x27
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x28->0x31
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x32->0x3B
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x3C->0x45
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x46->0x4F
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x50->0x59
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x5A->0x63
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x64->0x6D
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x6E->0x77
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x78->0x81
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x82->0x8B
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x8C->0x95
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0x96->0x9F
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0xA0->0xA9
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0xAA->0xB3
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0xB4->0xBD
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0xBE->0xC7
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0xC8->0xD1
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0xD2->0xDB
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0xDC->0xE5
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0xE6->0xEF
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0xF0->0xF9
	-1,-1,-1,-1,-1,-1,             //0xFA->0xFF
}
*/


#endif //FLIR_CHANNELS_H

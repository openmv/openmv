/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2009 STMicroelectronics
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * CEC driver.
 */
#ifndef __PY_LCD_CEC_H__
#define __PY_LCD_CEC_H__
int cec_init(void);
void cec_deinit(void);
int cec_send_frame(uint8_t InitiatorAddress, uint8_t FollowerAddress, uint8_t MessageLength, uint8_t *Message);
int cec_receive_frame(uint8_t *Message, size_t *MessageLength, uint8_t FollowerAddress, uint8_t *InitiatorAddress);
#endif // __PY_LCD_CEC_H__

/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2020 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2020 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * LCD Python module.
 */
#ifndef __PY_LCD_CEC_H__
#define __PY_LCD_CEC_H__
void lcd_cec_init();
void lcd_cec_deinit();
void lcd_cec_send_frame(mp_obj_t dst_addr, mp_obj_t src_addr, mp_obj_t bytes);
mp_obj_t lcd_cec_receive_frame(uint n_args, const mp_obj_t *args, mp_map_t *kw_args);
void lcd_cec_register_cec_receive_cb(mp_obj_t cb, mp_obj_t dst_addr);
mp_obj_t lcd_cec_received_frame_src_addr();
mp_obj_t lcd_cec_received_frame_bytes();
#endif // __PY_LCD_CEC_H__

/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2022 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2022 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Tinyusb CDC debugger helper code.
 */
#ifndef __TUSBDBG_H__
void USBD_IRQHandler(void);
int  tinyusb_debug_init(void);
bool tinyusb_debug_enabled(void);
void tinyusb_debug_tx_strn(const char *str, mp_uint_t len);
#endif // __TUSBDBG_H__

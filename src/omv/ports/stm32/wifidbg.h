/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * WiFi debugger.
 */
#ifndef __WIFI_DBG_H__
#define __WIFI_DBG_H__
#include "winc.h"

typedef struct wifidbg_config {
    winc_mode_t mode;
    winc_security_t client_security, access_point_security;
    char client_key[WINC_MAX_PSK_LEN + 1], access_point_key[WINC_MAX_PSK_LEN + 1];
    char client_ssid[WINC_MAX_SSID_LEN + 1], access_point_ssid[WINC_MAX_SSID_LEN + 1];
    uint8_t client_channel, access_point_channel;
    char board_name[WINC_MAX_BOARD_NAME_LEN + 1];
} wifidbg_config_t;

void wifidbg_dispatch();
void wifidbg_pendsv_callback(void);
void wifidbg_systick_callback(uint32_t ticks_ms);
int wifidbg_init(wifidbg_config_t *config);
void wifidbg_set_irq_enabled(bool enable);
#endif /* __WIFIDBG_H__ */

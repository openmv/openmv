/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
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

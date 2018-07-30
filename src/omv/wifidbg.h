/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2018 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */
#ifndef __WIFI_DBG_H__
#define __WIFI_DBG_H__
#include "winc.h"

typedef struct wifidbg_config {
    winc_mode_t mode;
    winc_security_t security;
    char key[WINC_MAX_PSK_LEN + 1];
    char ssid[WINC_MAX_SSID_LEN + 1];
    uint8_t channel;
} wifidbg_config_t;

void wifidbg_dispatch();
int wifidbg_init(wifidbg_config_t *config);
#endif /* __WIFIDBG_H__ */

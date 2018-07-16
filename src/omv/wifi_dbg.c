/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2018 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include "winc.h"
#include "socket/include/socket.h"
#include "driver/include/m2m_wifi.h"
#include "wifi_dbg.h"
#include STM32_HAL_H

int wifi_dbg_init(wifi_dbg_config_t *config)
{
    // Initialize WiFi in AP mode.
    if (winc_init(WINC_MODE_AP) != 0) {
        return -1;
    }

    // Start WiFi in AP mode.
    if (winc_start_ap(config->ssid, config->security, config->key, config->channel) != 0) {
        return -2;
    }

    return 0;
}

void *wifidbg_thread_entry(void *args_in)
{
    while (true) {
        // TODO implement wifi debugging code.
        HAL_Delay(1000);
    }
}

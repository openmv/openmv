/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2018 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */
#ifndef __WIFI_DBG_H__
#define __WIFI_DBG_H__

#define SSID_MAX 63
#define PASS_MAX 63
#define NAME_MAX 63

#define OPENMVCAM_BROADCAST_PORT 0xABD1

typedef enum wifi_dbg_wifi_mode
{
    WIFI_DBG_DISABLED = 0,
    WIFI_DBG_ENABLED_CLIENT_MODE = 1,
    WIFI_DBG_ENABLED_AP_MODE = 2
}
wifi_dbg_wifi_mode_t;

typedef enum wifi_dbg_wifi_type
{
    WIFI_DBG_OPEN,
    WIFI_DBG_WPA,
    WIFI_DBG_WEP
}
wifi_dbg_wifi_type_t;

typedef struct wifi_dbg_settings
{
    wifi_dbg_wifi_mode_t wifi_mode;
    char wifi_client_ssid[SSID_MAX+1];
    char wifi_client_pass[PASS_MAX+1];
    wifi_dbg_wifi_type_t wifi_client_type;
    char wifi_ap_ssid[SSID_MAX+1];
    char wifi_ap_pass[PASS_MAX+1];
    wifi_dbg_wifi_type_t wifi_ap_type;
    char wifi_board_name[NAME_MAX+1];
}
wifi_dbg_settings_t;

void wifi_dbg_init();
void wifi_dbg_apply_settings(wifi_dbg_settings_t *settings);
void wifi_dbg_beacon();

#endif /* __WIFI_DBG_H__ */

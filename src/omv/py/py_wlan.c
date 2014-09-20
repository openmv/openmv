/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * WLAN Python module.
 *
 */
#include <mp.h>
#include <cc3k.h>
#include <stm32f4xx_hal.h>
#include "py_led.h"
#define IS_WLAN_SEC(sec) \
    (sec>WLAN_SEC_UNSEC && sec<=WLAN_SEC_WPA2)
#define MAX_PACKET_LENGTH (1024)

static volatile int fd_state=0;
static volatile int ip_obtained = 0;
static volatile int wlan_connected = 0;

int wlan_get_fd_state(int fd)
{
    return (fd_state & (1<<fd));
}

void wlan_clear_fd_state(int fd)
{
    // reset socket state
    fd_state &= ~(1<<fd);
}

void sWlanCallback(long lEventType, char * data, unsigned char length)
{
    switch (lEventType) {
        case HCI_EVNT_WLAN_UNSOL_CONNECT:
            wlan_connected = 1;
            break;
        case HCI_EVNT_WLAN_UNSOL_DISCONNECT:
            /* Link down */
            wlan_connected = 0;
            led_state(LED_RED, 1);
            break;
        case HCI_EVNT_WLAN_UNSOL_DHCP:
            ip_obtained = 1;
            break;
        case HCI_EVNT_BSD_TCP_CLOSE_WAIT:
            // mark socket for closure
            fd_state |= (1<<((uint8_t)data[0]));
            break;
    }
}

static mp_obj_t mod_wlan_init()
{
    /* Initialize WLAN module */
    wlan_init(sWlanCallback, NULL, NULL, NULL,
            ReadWlanInterruptPin, SpiResumeSpi, SpiPauseSpi, WriteWlanPin);

    /* Start WLAN module */
    if (wlan_start(0) != 0) {
        nlr_raise(mp_obj_new_exception_msg(
                    &mp_type_OSError, "Failed to init wlan module"));
    }

    /* Set connection policy */
    // wlan_ioctl_set_connection_policy(0, 0, 0);

    /* Mask out all non-required events from the CC3000 */
    wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|
                        HCI_EVNT_WLAN_UNSOL_INIT|
                        HCI_EVNT_WLAN_ASYNC_PING_REPORT|
                        HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE);

    return mp_const_none;
}

static mp_obj_t mod_wlan_ifconfig()
{
    tNetappIpconfigRetArgs ipconfig;
    uint8_t *ip = &ipconfig.aucIP[0];
    uint8_t *mask= &ipconfig.aucSubnetMask[0];
    uint8_t *gw= &ipconfig.aucDefaultGateway[0];
    uint8_t *dhcp= &ipconfig.aucDHCPServer[0];
    uint8_t *dns= &ipconfig.aucDNSServer[0];
    uint8_t *mac= &ipconfig.uaMacAddr[0];
//    uint8_t *ssid= &ipconfig.uaSSID[0]; //32

    netapp_ipconfig(&ipconfig);

    printf ("IP:%d.%d.%d.%d\n"  \
            "Mask:%d.%d.%d.%d\n"\
            "GW:%d.%d.%d.%d\n"  \
            "DHCP:%d.%d.%d.%d\n"\
            "DNS:%d.%d.%d.%d\n" \
            "MAC:%02X:%02X:%02X:%02X:%02X:%02X\n",
            ip[3], ip[2], ip[1], ip[0],
            mask[3], mask[2], mask[1], mask[0],
            gw[3], gw[2], gw[1], gw[0],
            dhcp[3], dhcp[2], dhcp[1], dhcp[0],
            dns[3], dns[2], dns[1], dns[0],
            mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    return mp_const_none;
}

static mp_obj_t mod_wlan_connect(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    int ssid_len =0;
    const char *ssid = NULL;
    const char *bssid = NULL;

    int key_len =0;
    int sec = WLAN_SEC_UNSEC;
    const char *key = NULL;

    mp_map_elem_t *kw_key, *kw_sec, *kw_bssid;

    ssid = mp_obj_str_get_str(args[0]);
    ssid_len = strlen(ssid);

    /* get KW args */
    kw_key = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("key")), MP_MAP_LOOKUP);
    kw_sec = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("sec")), MP_MAP_LOOKUP);
    kw_bssid = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("bssid")), MP_MAP_LOOKUP);

    /* get key and sec */
    if (kw_key && kw_sec) {
        key = mp_obj_str_get_str(kw_key->value);
        key_len = strlen(key);

        sec = mp_obj_get_int(kw_sec->value);
        if (!IS_WLAN_SEC(sec)) {
            nlr_raise(mp_obj_new_exception_msg(
                        &mp_type_ValueError, "Invalid security mode"));
            return mp_const_false;
        }

    }

    /* get bssid */
    if (kw_bssid != NULL) {
        bssid = mp_obj_str_get_str(kw_bssid->value);
    }

    /* connect to AP */
    if (wlan_connect(sec, (char*) ssid, ssid_len, (uint8_t*)bssid, (uint8_t*)key, key_len) != 0) {
        return mp_const_false;
    }

    return mp_const_true;
}

static mp_obj_t mod_wlan_connected()
{
    if (wlan_connected && ip_obtained) {
        return mp_const_true;
    }
    return mp_const_false;
}

static mp_obj_t mod_wlan_patch_version()
{
    uint8_t pver[2];
    mp_obj_tuple_t *t_pver = mp_obj_new_tuple(2, NULL);

    nvmem_read_sp_version(pver);
    t_pver->items[0] = mp_obj_new_int(pver[0]);
    t_pver->items[1] = mp_obj_new_int(pver[1]);
    return t_pver;
}

static mp_obj_t mod_wlan_patch_program()
{
    patch_prog_start();
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0 (py_wlan_init_obj,         mod_wlan_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0 (py_wlan_ifconfig_obj,     mod_wlan_ifconfig);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_wlan_connect_obj, 1,   mod_wlan_connect);
STATIC MP_DEFINE_CONST_FUN_OBJ_0 (py_wlan_connected_obj,    mod_wlan_connected);
STATIC MP_DEFINE_CONST_FUN_OBJ_0 (py_wlan_patch_version_obj,mod_wlan_patch_version);
STATIC MP_DEFINE_CONST_FUN_OBJ_0 (py_wlan_patch_program_obj,mod_wlan_patch_program);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_wlan) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_WEP),             MP_OBJ_NEW_SMALL_INT(WLAN_SEC_WEP)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_WPA),             MP_OBJ_NEW_SMALL_INT(WLAN_SEC_WPA)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_WPA2),            MP_OBJ_NEW_SMALL_INT(WLAN_SEC_WPA2)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_init),            (mp_obj_t)&py_wlan_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ifconfig),        (mp_obj_t)&py_wlan_ifconfig_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_connect),         (mp_obj_t)&py_wlan_connect_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_connected),       (mp_obj_t)&py_wlan_connected_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_patch_version),   (mp_obj_t)&py_wlan_patch_version_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_patch_program),   (mp_obj_t)&py_wlan_patch_program_obj },
};

static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t wlan_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_wlan,
    .globals = (mp_obj_t)&globals_dict,
};

const mp_obj_module_t *py_wlan_init()
{
    return &wlan_module;
}

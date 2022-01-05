/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * NINA-W10 Python module.
 */
#if MICROPY_PY_NINAW10
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include "py/nlr.h"
#include "py/mphal.h"
#include "py/objtuple.h"
#include "py/objlist.h"
#include "py/stream.h"
#include "py/runtime.h"

#include "mperrno.h"
#include "shared/netutils/netutils.h"
#include "modnetwork.h"

#include "nina.h"

#define NINA_MIN(a,b)           \
({                              \
    __typeof__ (a) _a = (a);    \
    __typeof__ (b) _b = (b);    \
    _a < _b ? _a : _b;          \
})


#define NINA_MAX(a,b)           \
({                              \
    __typeof__ (a) _a = (a);    \
    __typeof__ (b) _b = (b);    \
    _a > _b ? _a : _b;          \
})

typedef struct _nina_obj_t {
    mp_obj_base_t base;
    bool active;
    uint32_t itf;
} nina_obj_t;

// For auto-binding UDP sockets
#define BIND_PORT_RANGE_MIN (65000)
#define BIND_PORT_RANGE_MAX (65535)

static uint16_t bind_port = BIND_PORT_RANGE_MIN;
static nina_obj_t nina_obj = {{(mp_obj_type_t*) &mod_network_nic_type_nina}, false, NINA_WIFI_MODE_STA};

// Initialise the module using the given SPI bus and pins and return a nina object.
static mp_obj_t py_nina_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 0, 1, false);

    nina_obj.active = false;
    if (n_args == 0) {
        nina_obj.itf = NINA_WIFI_MODE_STA;
    } else {
        nina_obj.itf = mp_obj_get_int(args[0]);
    }

    // Reset autobind port.
    bind_port = BIND_PORT_RANGE_MIN;

    // Register with network module
    mod_network_register_nic(MP_OBJ_FROM_PTR(&nina_obj));

    return MP_OBJ_FROM_PTR(&nina_obj);
}

static mp_obj_t py_nina_active(size_t n_args, const mp_obj_t *args)
{
    nina_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if (n_args == 2) {
        bool active = mp_obj_is_true(args[1]);
        if (active) {
            int error = 0;
            if ((error = nina_init()) != 0) {
                mp_raise_msg_varg(&mp_type_OSError,
                        MP_ERROR_TEXT("Failed to initialize Nina-W10 module, error: %d\n"), error);
            }
            // check firmware version
            uint8_t fw_ver[NINA_FW_VER_LEN];
            if (nina_fw_version(fw_ver) != 0) {
                nina_deinit();
                mp_raise_msg_varg(&mp_type_OSError,
                        MP_ERROR_TEXT("Failed to read firmware version, error: %d\n"), error);
            }

            // Check fw version matches the driver.
            if ((fw_ver[NINA_FW_VER_MAJOR_OFFS] - 48) != NINA_FW_VER_MAJOR ||
                (fw_ver[NINA_FW_VER_MINOR_OFFS] - 48) != NINA_FW_VER_MINOR ||
                (fw_ver[NINA_FW_VER_PATCH_OFFS] - 48) != NINA_FW_VER_PATCH) {
                mp_printf(&mp_plat_print,
                        "Warning: firmware version mismatch, expected %d.%d.%d found: %d.%d.%d\n",
                        NINA_FW_VER_MAJOR, NINA_FW_VER_MINOR, NINA_FW_VER_PATCH,
                        fw_ver[NINA_FW_VER_MAJOR_OFFS] - 48,
                        fw_ver[NINA_FW_VER_MINOR_OFFS] - 48,
                        fw_ver[NINA_FW_VER_PATCH_OFFS] - 48);
            }
        } else {
            nina_deinit();
        }
        self->active = active;
    }
    return mp_obj_new_bool(self->active);
}

// method connect(ssid, key=None, *, security=WPA2, bssid=None)
static mp_obj_t py_nina_connect(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_essid, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_key, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_security, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = NINA_SEC_WPA_PSK} },
        { MP_QSTR_channel,  MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
    };

    // parse args
    nina_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get ssid
    const char *ssid = mp_obj_str_get_str(args[0].u_obj);

    if (strlen(ssid) == 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("SSID can't be empty!"));
    }

    // get key and sec
    const char *key = NULL;
    mp_uint_t security = NINA_SEC_OPEN;

    if (args[1].u_obj != mp_const_none) {
        key = mp_obj_str_get_str(args[1].u_obj);
        security = args[2].u_int;
    }

    if (security != NINA_SEC_OPEN && strlen(key) == 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Key can't be empty!"));
    }

    // Disconnect active connections first.
    if (nina_isconnected()) {
        nina_disconnect();
    }

    if (self->itf == NINA_WIFI_MODE_STA) {
        // Initialize WiFi in Station mode.
        if (nina_connect(ssid, security, key, 0) != 0) {
            mp_raise_msg_varg(&mp_type_OSError,
                    MP_ERROR_TEXT("could not connect to ssid=%s, sec=%d, key=%s\n"), ssid, security, key);
        }
    } else {
        mp_uint_t channel = args[3].u_int;

        if (security != NINA_SEC_OPEN && security != NINA_SEC_WEP) {
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("AP mode supports WEP security only."));
        }

        // Initialize WiFi in AP mode.
        if (nina_start_ap(ssid, security, key, channel) != 0) {
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("failed to start in AP mode"));
        }
    }
    return mp_const_none;
}

static mp_obj_t py_nina_disconnect(mp_obj_t self_in)
{
    nina_disconnect();
    return mp_const_none;
}

static mp_obj_t py_nina_isconnected(mp_obj_t self_in)
{
    return mp_obj_new_bool(nina_isconnected());
}

static mp_obj_t py_nina_connected_sta(mp_obj_t self_in)
{
    uint32_t sta_ip = 0;
    mp_obj_t sta_list = mp_obj_new_list(0, NULL);

    if (nina_connected_sta(&sta_ip) == 0) {
        mp_obj_list_append(sta_list, netutils_format_inet_addr(
                    (uint8_t*)&sta_ip, 0, NETUTILS_BIG));
    }

    return sta_list;
}

static mp_obj_t py_nina_wait_for_sta(mp_obj_t self_in, mp_obj_t timeout_in)
{
    uint32_t sta_ip;
    // get timeout
    mp_obj_t sta_list = mp_obj_new_list(0, NULL);
    mp_uint_t timeout = mp_obj_get_int(timeout_in);

    if (nina_wait_for_sta(&sta_ip, timeout) == 0) {
        mp_obj_list_append(sta_list, netutils_format_inet_addr(
                    (uint8_t*)&sta_ip, 0, NETUTILS_BIG));
    }

    return sta_list;
}

static mp_obj_t py_nina_ifconfig(size_t n_args, const mp_obj_t *args)
{
    nina_ifconfig_t ifconfig;
    if (n_args == 1) {
        // get ifconfig info
        nina_ifconfig(&ifconfig, false);
        mp_obj_t tuple[4] = {
            netutils_format_ipv4_addr(ifconfig.ip_addr, NETUTILS_BIG),
            netutils_format_ipv4_addr(ifconfig.subnet_addr, NETUTILS_BIG),
            netutils_format_ipv4_addr(ifconfig.gateway_addr, NETUTILS_BIG),
            netutils_format_ipv4_addr(ifconfig.dns_addr, NETUTILS_BIG),
        };
        return mp_obj_new_tuple(4, tuple);
    } else {
        // set ifconfig info
        mp_obj_t *items;
        mp_obj_get_array_fixed_n(args[1], 4, &items);
        netutils_parse_ipv4_addr(items[0], ifconfig.ip_addr, NETUTILS_BIG);
        netutils_parse_ipv4_addr(items[1], ifconfig.subnet_addr, NETUTILS_BIG);
        netutils_parse_ipv4_addr(items[2], ifconfig.gateway_addr, NETUTILS_BIG);
        netutils_parse_ipv4_addr(items[3], ifconfig.dns_addr, NETUTILS_BIG);
        nina_ifconfig(&ifconfig, true);
        return mp_const_none;
    }
}

static mp_obj_t py_nina_netinfo(mp_obj_t self_in)
{
    nina_netinfo_t netinfo;
    nina_netinfo(&netinfo);

    // Format MAC address
    VSTR_FIXED(bssid_vstr, 18);
    vstr_printf(&bssid_vstr, "%02x:%02x:%02x:%02x:%02x:%02x",
            netinfo.bssid[0], netinfo.bssid[1], netinfo.bssid[2],
            netinfo.bssid[3], netinfo.bssid[4], netinfo.bssid[5]);

    // Add connection info
    mp_obj_t tuple[4] = {
        mp_obj_new_int(netinfo.rssi),
        mp_obj_new_int(netinfo.security),
        mp_obj_new_str(netinfo.ssid, strlen(netinfo.ssid)),
        mp_obj_new_str(bssid_vstr.buf, bssid_vstr.len),
    };
    return mp_obj_new_tuple(4, tuple);
}

static int nina_scan_callback(nina_scan_result_t *scan_result, void *arg)
{
    mp_obj_t scan_list = (mp_obj_t)arg;
    mp_obj_t ap[6] = {
        mp_obj_new_bytes((uint8_t *)scan_result->ssid, strlen(scan_result->ssid)),
        mp_obj_new_bytes(scan_result->bssid, sizeof(scan_result->bssid)),
        mp_obj_new_int(scan_result->channel),
        mp_obj_new_int(scan_result->rssi),
        mp_obj_new_int(scan_result->security),
        MP_OBJ_NEW_SMALL_INT(1), // N
    };
    mp_obj_list_append(scan_list, mp_obj_new_tuple(MP_ARRAY_SIZE(ap), ap));
    return 0;
}

static mp_obj_t py_nina_scan(mp_obj_t self_in)
{
    mp_obj_t scan_list;
    scan_list = mp_obj_new_list(0, NULL);
    nina_scan(nina_scan_callback, scan_list, 10000);
    return scan_list;
}

static mp_obj_t py_nina_get_rssi(mp_obj_t self_in)
{
    return mp_obj_new_int(nina_get_rssi());
}

static mp_obj_t py_nina_fw_version(mp_obj_t self_in)
{
    uint8_t fwver[NINA_FW_VER_LEN];
    nina_fw_version(fwver);

    // Firmware version is a string like "1.4.7"
    mp_obj_tuple_t *t_fwver = mp_obj_new_tuple(3, NULL);
    for (int i=0; i<3; i++) {
        t_fwver->items[i] = mp_obj_new_int(fwver[i*2] - 48);
    }
    return t_fwver;
}

static int py_nina_gethostbyname(mp_obj_t nic, const char *name, mp_uint_t len, uint8_t *out_ip)
{
    return nina_gethostbyname(name, out_ip);
}

static int py_nina_socket_socket(mod_network_socket_obj_t *socket, int *_errno)
{
    uint8_t type;

    if (socket->domain != MOD_NETWORK_AF_INET) {
        *_errno = MP_EAFNOSUPPORT;
        return -1;
    }

    switch (socket->type) {
        case MOD_NETWORK_SOCK_STREAM:
            type = NINA_SOCKET_TYPE_TCP;
            break;

        case MOD_NETWORK_SOCK_DGRAM:
            type = NINA_SOCKET_TYPE_UDP;
            break;

        default:
            *_errno = MP_EINVAL;
            return -1;
    }

    // open socket
    int fd = nina_socket_socket(type);
    if (fd < 0) {
        *_errno = fd;
        return -1;
    }

    // store state of this socket
    socket->fd = fd;
    socket->timeout = 0; // blocking
    socket->port  = 0;
    socket->bound = false;
    return 0;
}

static void py_nina_socket_close(mod_network_socket_obj_t *socket)
{
    if (socket->fd >= 0) {
        nina_socket_close(socket->fd);
        socket->fd = -1; // Mark socket FD as invalid
    }
}

static int py_nina_socket_bind(mod_network_socket_obj_t *socket, byte *ip, mp_uint_t port, int *_errno)
{
    uint8_t type;
    switch (socket->type) {
        case MOD_NETWORK_SOCK_STREAM:
            type = NINA_SOCKET_TYPE_TCP;
            break;

        case MOD_NETWORK_SOCK_DGRAM:
            type = NINA_SOCKET_TYPE_UDP;
            break;

        default:
            *_errno = MP_EINVAL;
            return -1;
    }

    int ret = nina_socket_bind(socket->fd, ip, port, type);
    if (ret < 0) {
        *_errno = ret;
        py_nina_socket_close(socket);
        return -1;
    }

    // Set bound port and mark socket as bound to avoid auto-binding.
    socket->port  = port;
    socket->bound = true;
    return 0;
}

static int py_nina_socket_listen(mod_network_socket_obj_t *socket, mp_int_t backlog, int *_errno)
{
    int ret = nina_socket_listen(socket->fd, backlog);
    if (ret < 0) {
        *_errno = ret;
        py_nina_socket_close(socket);
        return -1;
    }
    return 0;
}

static int py_nina_socket_accept(mod_network_socket_obj_t *socket,
        mod_network_socket_obj_t *socket2, byte *ip, mp_uint_t *port, int *_errno)
{
    int fd = 0;
    // Call accept.
    int ret = nina_socket_accept(socket->fd, ip, (uint16_t*) port, &fd, socket->timeout);
    if (ret < 0) {
        *_errno = ret;
        py_nina_socket_close(socket);
        return -1;
    }

    // Set default socket timeout.
    socket2->fd = fd;
    socket2->timeout = 0;
    socket2->bound = false;
    return 0;
}

static int py_nina_socket_connect(mod_network_socket_obj_t *socket, byte *ip, mp_uint_t port, int *_errno)
{
    int ret = nina_socket_connect(socket->fd, ip, port, socket->timeout);
    if (ret < 0) {
        *_errno = ret;
        py_nina_socket_close(socket);
        return -1;
    }
    return 0;
}

static mp_uint_t py_nina_socket_send(mod_network_socket_obj_t *socket, const byte *buf, mp_uint_t len, int *_errno)
{
    int ret = nina_socket_send(socket->fd, buf, len, socket->timeout);
    if (ret == NINA_ERROR_TIMEOUT) {
        // The socket is Not closed on timeout when calling functions that accept a timeout.
        *_errno = MP_ETIMEDOUT;
        return -1;
    } else if (ret < 0) {
        // Close the socket on any other errors.
        *_errno = ret;
        py_nina_socket_close(socket);
        return -1;
    }
    return ret;
}

static mp_uint_t py_nina_socket_recv(mod_network_socket_obj_t *socket, byte *buf, mp_uint_t len, int *_errno)
{
    int ret = nina_socket_recv(socket->fd, buf, len, socket->timeout);
    if (ret == NINA_ERROR_TIMEOUT) {
        // The socket is Not closed on timeout when calling functions that accept a timeout.
        *_errno = MP_ETIMEDOUT;
        return -1;
    } else if (ret < 0) {
        // Close the socket on any other errors.
        *_errno = ret;
        py_nina_socket_close(socket);
        return -1;
    }
    return ret;
}

static mp_uint_t py_nina_socket_auto_bind(mod_network_socket_obj_t *socket, int *_errno)
{
    if (socket->bound == false) {
        if (py_nina_socket_bind(socket, NULL, bind_port, _errno) != 0) {
            return -1;
        }
        bind_port++;
        bind_port = NINA_MIN(NINA_MAX(bind_port, BIND_PORT_RANGE_MIN), BIND_PORT_RANGE_MAX);
    }
    return 0;
}

static mp_uint_t py_nina_socket_sendto(mod_network_socket_obj_t *socket,
        const byte *buf, mp_uint_t len, byte *ip, mp_uint_t port, int *_errno)
{
    // Auto-bind the socket first if the socket is unbound.
    if (py_nina_socket_auto_bind(socket, _errno) != 0) {
        return -1;
    }

    int ret = nina_socket_sendto(socket->fd, buf, len, ip, port, socket->timeout);
    if (ret == NINA_ERROR_TIMEOUT) {
        // The socket is Not closed on timeout when calling functions that accept a timeout.
        *_errno = MP_ETIMEDOUT;
        return -1;
    } else if (ret < 0) {
        *_errno = ret;
        py_nina_socket_close(socket);
        return -1;
    }
    return ret;
}

static mp_uint_t py_nina_socket_recvfrom(mod_network_socket_obj_t *socket,
        byte *buf, mp_uint_t len, byte *ip, mp_uint_t *port, int *_errno)
{
    // Auto-bind the socket first if the socket is unbound.
    if (py_nina_socket_auto_bind(socket, _errno) != 0) {
        return -1;
    }

    int ret = nina_socket_recvfrom(socket->fd, buf, len, ip, (uint16_t *) port, socket->timeout);
    if (ret == NINA_ERROR_TIMEOUT) {
        // The socket is Not closed on timeout when calling functions that accept a timeout.
        *_errno = MP_ETIMEDOUT;
        return -1;
    } else if (ret < 0) {
        // Close the socket on any other errors.
        *_errno = ret;
        py_nina_socket_close(socket);
        return -1;
    }
    return ret;
}

static int py_nina_socket_setsockopt(mod_network_socket_obj_t *socket, mp_uint_t
        level, mp_uint_t opt, const void *optval, mp_uint_t optlen, int *_errno)
{
    int ret = nina_socket_setsockopt(socket->fd, level, opt, optval, optlen);
    if (ret < 0) {
        *_errno = ret;
        py_nina_socket_close(socket);
        return -1;
    }
    return 0;
}

static int py_nina_socket_settimeout(mod_network_socket_obj_t *socket, mp_uint_t timeout_ms, int *_errno)
{
    if (timeout_ms == UINT32_MAX) {
        // no timeout is given, set the socket to blocking mode.
        timeout_ms = 0;
    }
    socket->timeout = timeout_ms;
    return 0;
}

static int py_nina_socket_ioctl(mod_network_socket_obj_t *socket, mp_uint_t request, mp_uint_t arg, int *_errno)
{
    *_errno = MP_EIO;
    return -1;
}

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_nina_active_obj, 1, 2, py_nina_active);
static MP_DEFINE_CONST_FUN_OBJ_KW(py_nina_connect_obj, 1,   py_nina_connect);
static MP_DEFINE_CONST_FUN_OBJ_1(py_nina_disconnect_obj,    py_nina_disconnect);
static MP_DEFINE_CONST_FUN_OBJ_1(py_nina_isconnected_obj,   py_nina_isconnected);
static MP_DEFINE_CONST_FUN_OBJ_1(py_nina_connected_sta_obj, py_nina_connected_sta);
static MP_DEFINE_CONST_FUN_OBJ_2(py_nina_wait_for_sta_obj,  py_nina_wait_for_sta);
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_nina_ifconfig_obj, 1, 2, py_nina_ifconfig);
static MP_DEFINE_CONST_FUN_OBJ_1(py_nina_netinfo_obj,       py_nina_netinfo);
static MP_DEFINE_CONST_FUN_OBJ_1(py_nina_scan_obj,          py_nina_scan);
static MP_DEFINE_CONST_FUN_OBJ_1(py_nina_get_rssi_obj,      py_nina_get_rssi);
static MP_DEFINE_CONST_FUN_OBJ_1(py_nina_fw_version_obj,    py_nina_fw_version);

static const mp_rom_map_elem_t nina_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_active),              MP_ROM_PTR(&py_nina_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_connect),             MP_ROM_PTR(&py_nina_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_config),              MP_ROM_PTR(&py_nina_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_start_ap),            MP_ROM_PTR(&py_nina_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_disconnect),          MP_ROM_PTR(&py_nina_disconnect_obj) },
    { MP_ROM_QSTR(MP_QSTR_isconnected),         MP_ROM_PTR(&py_nina_isconnected_obj) },
    { MP_ROM_QSTR(MP_QSTR_connected_sta),       MP_ROM_PTR(&py_nina_connected_sta_obj) },
    { MP_ROM_QSTR(MP_QSTR_wait_for_sta),        MP_ROM_PTR(&py_nina_wait_for_sta_obj) },
    { MP_ROM_QSTR(MP_QSTR_ifconfig),            MP_ROM_PTR(&py_nina_ifconfig_obj) },
    { MP_ROM_QSTR(MP_QSTR_netinfo),             MP_ROM_PTR(&py_nina_netinfo_obj) },
    { MP_ROM_QSTR(MP_QSTR_scan),                MP_ROM_PTR(&py_nina_scan_obj) },
    { MP_ROM_QSTR(MP_QSTR_rssi),                MP_ROM_PTR(&py_nina_get_rssi_obj) },
    { MP_ROM_QSTR(MP_QSTR_fw_version),          MP_ROM_PTR(&py_nina_fw_version_obj) },
    // Network is not secured.
    { MP_ROM_QSTR(MP_QSTR_OPEN),                MP_ROM_INT(NINA_SEC_OPEN) },
    // Security type WEP (40 or 104).
    { MP_ROM_QSTR(MP_QSTR_WEP),                 MP_ROM_INT(NINA_SEC_WEP) },
    // Network secured with WPA/WPA2 personal(PSK).
    { MP_ROM_QSTR(MP_QSTR_WPA_PSK),             MP_ROM_INT(NINA_SEC_WPA_PSK) },
};

static MP_DEFINE_CONST_DICT(nina_locals_dict, nina_locals_dict_table);

const mod_network_nic_type_t mod_network_nic_type_nina = {
    .base = {
        { &mp_type_type },
        .name = MP_QSTR_nina,
        .make_new = py_nina_make_new,
        .locals_dict = (mp_obj_t)&nina_locals_dict,
    },
    .gethostbyname = py_nina_gethostbyname,
    .socket     = py_nina_socket_socket,
    .close      = py_nina_socket_close,
    .bind       = py_nina_socket_bind,
    .listen     = py_nina_socket_listen,
    .accept     = py_nina_socket_accept,
    .connect    = py_nina_socket_connect,
    .send       = py_nina_socket_send,
    .recv       = py_nina_socket_recv,
    .sendto     = py_nina_socket_sendto,
    .recvfrom   = py_nina_socket_recvfrom,
    .setsockopt = py_nina_socket_setsockopt,
    .settimeout = py_nina_socket_settimeout,
    .ioctl      = py_nina_socket_ioctl,
};
#endif // MICROPY_PY_NINAW10

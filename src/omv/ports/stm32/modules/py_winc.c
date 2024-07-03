/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * WINC1500 Python module.
 */
#if MICROPY_PY_WINC1500
#include <string.h>
#include <stdarg.h>
#include "mperrno.h"

#include "py/nlr.h"
#include "py/objtuple.h"
#include "py/objlist.h"
#include "py/stream.h"
#include "py/runtime.h"
#include "shared/netutils/netutils.h"
#include "extmod/modnetwork.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "omv_common.h"
#include "py_helper.h"
#include "file_utils.h"

#include "winc.h"
#include "socket/include/socket.h"
#include "driver/include/m2m_wifi.h"

typedef struct _winc_obj_t {
    mp_obj_base_t base;
    uint8_t active;
    uint8_t itf;
} winc_obj_t;

static winc_obj_t winc_obj = {{(mp_obj_type_t *) &mod_network_nic_type_winc}, false, WINC_MODE_STA};

static int py_winc_mperrno(int32_t err) {
    switch (err) {
        case SOCK_ERR_NO_ERROR:
            return MP_ENOTCONN;
        case SOCK_ERR_MAX_TCP_SOCK:
        case SOCK_ERR_MAX_UDP_SOCK:
        case SOCK_ERR_MAX_LISTEN_SOCK:
            return MP_EMFILE;
        case SOCK_ERR_CONN_ABORTED:
            return MP_ECONNABORTED;
        case SOCK_ERR_TIMEOUT:
            return MP_EWOULDBLOCK;
        case SOCK_ERR_BUFFER_FULL:
            return MP_ENOBUFS;
        case SOCK_ERR_ADDR_ALREADY_IN_USE:
            return MP_EADDRINUSE;
        case SOCK_ERR_INVALID:
        case SOCK_ERR_INVALID_ARG:
        case SOCK_ERR_INVALID_ADDRESS:
        case SOCK_ERR_ADDR_IS_REQUIRED:
            return MP_EINVAL;
        default:
            return err;
    }
}

// Initialise the module using the given SPI bus and pins and return a winc object.
static mp_obj_t py_winc_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode,  MP_ARG_INT, {.u_int = WINC_MODE_STA } },
    };

    // parse args
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args), allowed_args, args);

    // Init WINC
    winc_obj.active = false;
    winc_obj.itf = args[0].u_int;
    int error = winc_init(winc_obj.itf);
    if (error != 0) {
        mp_raise_msg_varg(&mp_type_OSError,
                          MP_ERROR_TEXT("Failed to initialize WINC1500 module: %s\n"), winc_strerror(error));
    }
    winc_obj.active = true;

    switch (winc_obj.itf) {
        case WINC_MODE_BSP:
            printf("Running in BSP mode...\n");
            break;
        case WINC_MODE_FIRMWARE:
            printf("Running in Firmware Upgrade mode...\n");
            break;
        case WINC_MODE_AP:
        case WINC_MODE_STA:
            // Register with network module
            mod_network_register_nic((mp_obj_t) &winc_obj);
            break;
        default:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("WiFi mode is not supported!"));
    }
    return (mp_obj_t) &winc_obj;
}

static mp_obj_t py_winc_active(size_t n_args, const mp_obj_t *args) {
    winc_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if (n_args == 2) {
        bool active = mp_obj_is_true(args[1]);
        if (active) {
            if (self->active) {
                // Nothing to do
            } else {
                int error = winc_init(winc_obj.itf);
                if (error != 0) {
                    mp_raise_msg_varg(&mp_type_OSError,
                                      MP_ERROR_TEXT("Failed to initialize WINC1500 module: %s\n"), winc_strerror(error));
                }
            }
        } else {
            winc_init(WINC_MODE_BSP);
        }
        self->active = mp_obj_is_true(args[1]);
    }
    return mp_obj_new_bool(self->active);
}

// method connect(ssid, key=None, *, security=WPA2, bssid=None)
static mp_obj_t py_winc_connect(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_ssid,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_key,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_security, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = M2M_WIFI_SEC_WPA_PSK} },
        { MP_QSTR_channel,  MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
    };

    // parse args
    winc_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get ssid
    const char *ssid = mp_obj_str_get_str(args[0].u_obj);

    if (strlen(ssid) == 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("SSID can't be empty!"));
    }

    // get key and sec
    const char *key = NULL;
    mp_uint_t security = M2M_WIFI_SEC_OPEN;

    if (args[1].u_obj != mp_const_none) {
        key = mp_obj_str_get_str(args[1].u_obj);
        security = args[2].u_int;
    }

    if (security != M2M_WIFI_SEC_OPEN && strlen(key) == 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Key can't be empty!"));
    }

    // Activate the interface if it's inactive.
    if (!self->active) {
        mp_obj_t args[2] = { pos_args[0], MP_OBJ_NEW_SMALL_INT(1) };
        py_winc_active(2, args);
    }

    if (self->itf == WINC_MODE_STA) {
        // Initialize WiFi in STA mode.
        if (winc_connect(ssid, security, key, M2M_WIFI_CH_ALL) != 0) {
            mp_raise_msg_varg(&mp_type_OSError,
                              MP_ERROR_TEXT("could not connect to ssid=%s, sec=%d, key=%s\n"), ssid, security, key);
        }
    } else {
        // Initialize WiFi in AP mode.
        mp_uint_t channel = args[3].u_int;
        if (winc_start_ap(ssid, security, key, channel) != 0) {
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("failed to start in AP mode"));
        }
    }
    return mp_const_none;
}

static mp_obj_t py_winc_disconnect(mp_obj_t self_in) {
    winc_disconnect();
    return mp_const_none;
}

static mp_obj_t py_winc_isconnected(mp_obj_t self_in) {
    return mp_obj_new_bool(winc_isconnected());
}

static mp_obj_t py_winc_connected_sta(mp_obj_t self_in) {
    uint32_t sta_ip = 0;
    mp_obj_t sta_list = mp_obj_new_list(0, NULL);

    if (winc_connected_sta(&sta_ip) == 0) {
        mp_obj_list_append(sta_list, netutils_format_inet_addr(
                               (uint8_t *) &sta_ip, 0, NETUTILS_BIG));
    }

    return sta_list;
}

static mp_obj_t py_winc_wait_for_sta(mp_obj_t self_in, mp_obj_t timeout_in) {
    uint32_t sta_ip;
    int32_t timeout_ms;

    // get timeout
    mp_obj_t sta_list = mp_obj_new_list(0, NULL);
    if (timeout_in == mp_const_none) {
        timeout_ms = -1;
    } else {
        timeout_ms = mp_obj_get_int(timeout_in);
    }
    if (winc_wait_for_sta(&sta_ip, timeout_ms) == 0) {
        mp_obj_list_append(sta_list, netutils_format_inet_addr(
                               (uint8_t *) &sta_ip, 0, NETUTILS_BIG));
    }

    return sta_list;
}

static mp_obj_t py_winc_ifconfig(size_t n_args, const mp_obj_t *args) {
    winc_ifconfig_t ifconfig;
    if (n_args == 1) {
        // get ifconfig info
        winc_ifconfig(&ifconfig, false);
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
        winc_ifconfig(&ifconfig, true);
        return mp_const_none;
    }
}

static mp_obj_t py_winc_netinfo(mp_obj_t self_in) {
    winc_netinfo_t netinfo;
    winc_netinfo(&netinfo);

    // Format MAC address
    VSTR_FIXED(mac_vstr, 18);
    vstr_printf(&mac_vstr, "%02x:%02x:%02x:%02x:%02x:%02x",
                netinfo.mac_addr[0], netinfo.mac_addr[1], netinfo.mac_addr[2],
                netinfo.mac_addr[3], netinfo.mac_addr[4], netinfo.mac_addr[5]);

    // Format IP address
    VSTR_FIXED(ip_vstr, 16);
    vstr_printf(&ip_vstr, "%d.%d.%d.%d", netinfo.ip_addr[0],
                netinfo.ip_addr[1], netinfo.ip_addr[2], netinfo.ip_addr[3]);

    // Add connection info

    mp_obj_t tuple[5] = {
        mp_obj_new_int(netinfo.rssi),
        mp_obj_new_int(netinfo.security),
        mp_obj_new_str(netinfo.ssid, strlen(netinfo.ssid)),
        mp_obj_new_str(mac_vstr.buf, mac_vstr.len),
        mp_obj_new_str(ip_vstr.buf, ip_vstr.len),
    };
    return mp_obj_new_tuple(5, tuple);
}

static int winc_scan_callback(winc_scan_result_t *scan_result, void *arg) {
    mp_obj_t scan_list = (mp_obj_t) arg;
    mp_obj_t ap[6] = {
        mp_obj_new_bytes((uint8_t *) scan_result->ssid, strlen(scan_result->ssid)),
        mp_obj_new_bytes(scan_result->bssid, sizeof(scan_result->bssid)),
        mp_obj_new_int(scan_result->channel),
        mp_obj_new_int(scan_result->rssi),
        mp_obj_new_int(scan_result->security),
        MP_OBJ_NEW_SMALL_INT(1), // N
    };
    mp_obj_list_append(scan_list, mp_obj_new_tuple(MP_ARRAY_SIZE(ap), ap));
    return 0;
}

static mp_obj_t py_winc_scan(mp_obj_t self_in) {
    mp_obj_t scan_list;
    scan_list = mp_obj_new_list(0, NULL);
    winc_scan(winc_scan_callback, scan_list);
    return scan_list;
}

static mp_obj_t py_winc_get_rssi(mp_obj_t self_in) {
    return mp_obj_new_int(winc_get_rssi());
}

static mp_obj_t py_winc_fw_version(mp_obj_t self_in) {
    winc_fwver_t fwver;
    mp_obj_tuple_t *t_fwver;

    winc_fw_version(&fwver);

    t_fwver = mp_obj_new_tuple(7, NULL);
    t_fwver->items[0] = mp_obj_new_int(fwver.fw_major);         // Firmware version major number.
    t_fwver->items[1] = mp_obj_new_int(fwver.fw_minor);         // Firmware version minor number.
    t_fwver->items[2] = mp_obj_new_int(fwver.fw_patch);         // Firmware version patch number.
    t_fwver->items[3] = mp_obj_new_int(fwver.drv_major);        // Driver version major number.
    t_fwver->items[4] = mp_obj_new_int(fwver.drv_minor);        // Driver version minor number.
    t_fwver->items[5] = mp_obj_new_int(fwver.drv_patch);        // Driver version patch number.
    t_fwver->items[6] = mp_obj_new_int(fwver.chip_id);          // HW revision number (chip ID).
    return t_fwver;
}

static mp_obj_t py_winc_fw_dump(mp_obj_t self_in, mp_obj_t path_in) {
    printf("Dumping flash...\n");
    const char *path = mp_obj_str_get_str(path_in);
    if (winc_flash_dump(path) != 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Failed to dump flash!"));
    }

    return mp_const_none;
}

static mp_obj_t py_winc_fw_update(mp_obj_t self_in, mp_obj_t path_in) {
    FRESULT res;
    FILINFO fno;
    const char *path = mp_obj_str_get_str(path_in);

    if ((res = file_ll_stat(path, &fno)) != FR_OK) {
        mp_raise_msg(&mp_type_OSError, (mp_rom_error_text_t) ffs_strerror(res));
    }

    // Erase the WINC1500 flash.
    printf("Erasing flash...\n");
    if (winc_flash_erase() != 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Failed to erase the flash!"));
    }

    // Program the firmware on the WINC1500 flash.
    printf("Programming firmware image...\n");
    if (winc_flash_write(path) != 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Failed to write the firmware!"));
    }

    // Verify the firmware on the WINC1500 flash.
    printf("Verifying firmware image...\n");
    if (winc_flash_verify(path) != 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Failed to verify the firmware!"));
    }

    printf("All task completed successfully.\n");
    return mp_const_none;
}

static int py_winc_gethostbyname(mp_obj_t nic, const char *name, mp_uint_t len, uint8_t *out_ip) {
    return winc_gethostbyname(name, out_ip);
}

static int py_winc_socket_socket(mod_network_socket_obj_t *socket, int *_errno) {
    uint8_t type;

    if (socket->domain != MOD_NETWORK_AF_INET) {
        *_errno = MP_EAFNOSUPPORT;
        return -1;
    }

    switch (socket->type) {
        case MOD_NETWORK_SOCK_STREAM:
            type = SOCK_STREAM;
            break;

        case MOD_NETWORK_SOCK_DGRAM:
            type = SOCK_DGRAM;
            break;

        default:
            *_errno = MP_EINVAL;
            return -1;
    }

    // open socket
    int fd = winc_socket_socket(type);
    if (fd < 0) {
        *_errno = py_winc_mperrno(fd);
        return -1;
    }

    // store state of this socket
    socket->fileno = fd;
    socket->_private = m_new0(winc_socket_buf_t, 1);
    return 0;
}

static void py_winc_socket_close(mod_network_socket_obj_t *socket) {
    if (socket->fileno >= 0) {
        winc_socket_close(socket->fileno);
        socket->fileno = -1; // Mark socket FD as invalid
        m_del(winc_socket_buf_t, socket->_private, 1);
        socket->_private = NULL;
        socket->nic = MP_OBJ_NULL;
    }
}

static int py_winc_socket_bind(mod_network_socket_obj_t *socket, byte *ip, mp_uint_t port, int *_errno) {
    MAKE_SOCKADDR(addr, ip, port)
    int ret = winc_socket_bind(socket->fileno, &addr);
    if (ret < 0) {
        *_errno = py_winc_mperrno(ret);
        py_winc_socket_close(socket);
        return -1;
    }
    return 0;
}

static int py_winc_socket_listen(mod_network_socket_obj_t *socket, mp_int_t backlog, int *_errno) {
    int ret = winc_socket_listen(socket->fileno, backlog);
    if (ret < 0) {
        *_errno = py_winc_mperrno(ret);
        py_winc_socket_close(socket);
        return -1;
    }
    return 0;
}

static int py_winc_socket_accept(mod_network_socket_obj_t *socket,
                                 mod_network_socket_obj_t *socket2, byte *ip, mp_uint_t *port, int *_errno) {
    int fd;
    sockaddr addr;

    // Call accept.
    int ret = winc_socket_accept(socket->fileno, &addr, &fd, socket->timeout);
    if (ret < 0) {
        *_errno = py_winc_mperrno(ret);
        if (ret != SOCK_ERR_TIMEOUT) {
            py_winc_socket_close(socket);
        }
        return -1;
    }

    // Set default socket timeout.
    socket2->fileno = fd;
    socket2->_private = m_new0(winc_socket_buf_t, 1);
    UNPACK_SOCKADDR((&addr), ip, *port);

    return 0;
}

static int py_winc_socket_connect(mod_network_socket_obj_t *socket, byte *ip, mp_uint_t port, int *_errno) {
    MAKE_SOCKADDR(addr, ip, port)
    int ret = winc_socket_connect(socket->fileno, &addr, socket->timeout);
    if (ret < 0) {
        *_errno = py_winc_mperrno(ret);
        py_winc_socket_close(socket);
        return -1;
    }
    return 0;
}

static mp_uint_t py_winc_socket_send(mod_network_socket_obj_t *socket, const byte *buf, mp_uint_t len, int *_errno) {
    int ret = winc_socket_send(socket->fileno, buf, len, socket->timeout);
    if (ret < 0) {
        *_errno = py_winc_mperrno(ret);
        // The socket is Not closed on timeout.
        if (ret != SOCK_ERR_TIMEOUT) {
            py_winc_socket_close(socket);
        }
        ret = -1;
    }
    return ret;
}

static mp_uint_t py_winc_socket_recv(mod_network_socket_obj_t *socket, byte *buf, mp_uint_t len, int *_errno) {
    int ret = winc_socket_recv(socket->fileno, buf, len, socket->_private, socket->timeout);
    if (ret <= 0) {
        // NOTE: 0 return from recv() means connection closed.
        *_errno = py_winc_mperrno(ret);
        // The socket is Not closed on timeout.
        if (ret != SOCK_ERR_TIMEOUT) {
            py_winc_socket_close(socket);
        }
        ret = -1;
    }
    return ret;
}

static mp_uint_t py_winc_socket_sendto(mod_network_socket_obj_t *socket,
                                       const byte *buf, mp_uint_t len, byte *ip, mp_uint_t port, int *_errno) {
    MAKE_SOCKADDR(addr, ip, port)
    int ret = winc_socket_sendto(socket->fileno, buf, len, &addr, socket->timeout);
    if (ret < 0) {
        *_errno = py_winc_mperrno(ret);
        // The socket is Not closed on timeout.
        if (ret != SOCK_ERR_TIMEOUT) {
            py_winc_socket_close(socket);
        }
        ret = -1;
    }
    return ret;
}

static mp_uint_t py_winc_socket_recvfrom(mod_network_socket_obj_t *socket,
                                         byte *buf, mp_uint_t len, byte *ip, mp_uint_t *port, int *_errno) {
    sockaddr addr;
    int ret = winc_socket_recvfrom(socket->fileno, buf, len, &addr, socket->timeout);
    UNPACK_SOCKADDR((&addr), ip, *port);
    if (ret < 0) {
        *_errno = py_winc_mperrno(ret);
        // The socket is Not closed on timeout.
        if (ret != SOCK_ERR_TIMEOUT) {
            py_winc_socket_close(socket);
        }
        ret = -1;
    }
    return ret;
}

static int py_winc_socket_setsockopt(mod_network_socket_obj_t *socket, mp_uint_t
                                     level, mp_uint_t opt, const void *optval, mp_uint_t optlen, int *_errno) {
    int ret = winc_socket_setsockopt(socket->fileno, level, opt, optval, optlen);
    if (ret < 0) {
        *_errno = py_winc_mperrno(ret);
        py_winc_socket_close(socket);
        return -1;
    }
    return 0;
}

static int py_winc_socket_settimeout(mod_network_socket_obj_t *socket, mp_uint_t timeout_ms, int *_errno) {
    socket->timeout = timeout_ms;
    return 0;
}

static int py_winc_socket_ioctl(mod_network_socket_obj_t *socket, mp_uint_t request, mp_uint_t arg, int *_errno) {
    *_errno = MP_EIO;
    return -1;
}

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_winc_active_obj, 1, 2, py_winc_active);
static MP_DEFINE_CONST_FUN_OBJ_KW(py_winc_connect_obj, 1,   py_winc_connect);
static MP_DEFINE_CONST_FUN_OBJ_1(py_winc_disconnect_obj,    py_winc_disconnect);
static MP_DEFINE_CONST_FUN_OBJ_1(py_winc_isconnected_obj,   py_winc_isconnected);
static MP_DEFINE_CONST_FUN_OBJ_1(py_winc_connected_sta_obj, py_winc_connected_sta);
static MP_DEFINE_CONST_FUN_OBJ_2(py_winc_wait_for_sta_obj,  py_winc_wait_for_sta);
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_winc_ifconfig_obj, 1, 2, py_winc_ifconfig);
static MP_DEFINE_CONST_FUN_OBJ_1(py_winc_netinfo_obj,       py_winc_netinfo);
static MP_DEFINE_CONST_FUN_OBJ_1(py_winc_scan_obj,          py_winc_scan);
static MP_DEFINE_CONST_FUN_OBJ_1(py_winc_get_rssi_obj,      py_winc_get_rssi);
static MP_DEFINE_CONST_FUN_OBJ_1(py_winc_fw_version_obj,    py_winc_fw_version);
static MP_DEFINE_CONST_FUN_OBJ_2(py_winc_fw_dump_obj,       py_winc_fw_dump);
static MP_DEFINE_CONST_FUN_OBJ_2(py_winc_fw_update_obj,     py_winc_fw_update);

static const mp_rom_map_elem_t winc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_active),        MP_ROM_PTR(&py_winc_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_connect),       MP_ROM_PTR(&py_winc_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_config),        MP_ROM_PTR(&py_winc_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_start_ap),      MP_ROM_PTR(&py_winc_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_disconnect),    MP_ROM_PTR(&py_winc_disconnect_obj) },
    { MP_ROM_QSTR(MP_QSTR_isconnected),   MP_ROM_PTR(&py_winc_isconnected_obj) },
    { MP_ROM_QSTR(MP_QSTR_connected_sta), MP_ROM_PTR(&py_winc_connected_sta_obj) },
    { MP_ROM_QSTR(MP_QSTR_wait_for_sta),  MP_ROM_PTR(&py_winc_wait_for_sta_obj) },
    { MP_ROM_QSTR(MP_QSTR_ifconfig),      MP_ROM_PTR(&py_winc_ifconfig_obj) },
    { MP_ROM_QSTR(MP_QSTR_netinfo),       MP_ROM_PTR(&py_winc_netinfo_obj) },
    { MP_ROM_QSTR(MP_QSTR_scan),          MP_ROM_PTR(&py_winc_scan_obj) },
    { MP_ROM_QSTR(MP_QSTR_rssi),          MP_ROM_PTR(&py_winc_get_rssi_obj) },
    { MP_ROM_QSTR(MP_QSTR_fw_version),    MP_ROM_PTR(&py_winc_fw_version_obj) },
    { MP_ROM_QSTR(MP_QSTR_fw_dump),       MP_ROM_PTR(&py_winc_fw_dump_obj) },
    { MP_ROM_QSTR(MP_QSTR_fw_update),     MP_ROM_PTR(&py_winc_fw_update_obj) },

    { MP_ROM_QSTR(MP_QSTR_OPEN),          MP_OBJ_NEW_SMALL_INT(M2M_WIFI_SEC_OPEN) },   // Network is not secured.
    { MP_ROM_QSTR(MP_QSTR_WPA_PSK),       MP_OBJ_NEW_SMALL_INT(M2M_WIFI_SEC_WPA_PSK) },// Network secured with WPA/WPA2 personal(PSK).
    { MP_ROM_QSTR(MP_QSTR_802_1X),        MP_OBJ_NEW_SMALL_INT(M2M_WIFI_SEC_802_1X) }, // Network is secured with WPA/WPA2 Enterprise.
    { MP_ROM_QSTR(MP_QSTR_MODE_STA),      MP_OBJ_NEW_SMALL_INT(WINC_MODE_STA) },       // Start in Station mode.
    { MP_ROM_QSTR(MP_QSTR_MODE_AP),       MP_OBJ_NEW_SMALL_INT(WINC_MODE_AP) },        // Start in Access Point mode.
    { MP_ROM_QSTR(MP_QSTR_MODE_P2P),      MP_OBJ_NEW_SMALL_INT(WINC_MODE_P2P) },       // Start in P2P (WiFi Direct) mode.
    { MP_ROM_QSTR(MP_QSTR_MODE_BSP),      MP_OBJ_NEW_SMALL_INT(WINC_MODE_BSP) },       // Init BSP.
    { MP_ROM_QSTR(MP_QSTR_MODE_FIRMWARE), MP_OBJ_NEW_SMALL_INT(WINC_MODE_FIRMWARE) },  // Start in Firmware Upgrade mode.
};

static MP_DEFINE_CONST_DICT(winc_locals_dict, winc_locals_dict_table);

static const mod_network_nic_protocol_t mod_network_nic_protocol_winc = {
    .gethostbyname = py_winc_gethostbyname,
    .socket = py_winc_socket_socket,
    .close = py_winc_socket_close,
    .bind = py_winc_socket_bind,
    .listen = py_winc_socket_listen,
    .accept = py_winc_socket_accept,
    .connect = py_winc_socket_connect,
    .send = py_winc_socket_send,
    .recv = py_winc_socket_recv,
    .sendto = py_winc_socket_sendto,
    .recvfrom = py_winc_socket_recvfrom,
    .setsockopt = py_winc_socket_setsockopt,
    .settimeout = py_winc_socket_settimeout,
    .ioctl = py_winc_socket_ioctl,
};

MP_DEFINE_CONST_OBJ_TYPE(
    mod_network_nic_type_winc,
    MP_QSTR_WINC,
    MP_TYPE_FLAG_NONE,
    make_new, py_winc_make_new,
    locals_dict, &winc_locals_dict,
    protocol, &mod_network_nic_protocol_winc
    );
#endif // MICROPY_PY_WINC1500

/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * WINC1500 Python module.
 *
 */
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "py/nlr.h"
#include "py/objtuple.h"
#include "py/objlist.h"
#include "py/stream.h"
#include "py/runtime.h"
#include "lib/netutils/netutils.h"
#include "modnetwork.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "spi.h"
#include "common.h"
#include "py_helper.h"
#include "ff_wrapper.h"

// WINC's includes
#include "driver/include/nmasic.h"
#include "socket/include/socket.h"
#include "programmer/programmer.h"
#include "driver/include/m2m_wifi.h"

#define MAKE_SOCKADDR(addr, ip, port) \
    struct sockaddr addr; \
    addr.sa_family = AF_INET; \
    addr.sa_data[0] = port >> 8; \
    addr.sa_data[1] = port; \
    addr.sa_data[2] = ip[0]; \
    addr.sa_data[3] = ip[1]; \
    addr.sa_data[4] = ip[2]; \
    addr.sa_data[5] = ip[3];

#define UNPACK_SOCKADDR(addr, ip, port) \
    port = (addr->sa_data[0] << 8) | addr->sa_data[1]; \
    ip[0] = addr->sa_data[2]; \
    ip[1] = addr->sa_data[3]; \
    ip[2] = addr->sa_data[4]; \
    ip[3] = addr->sa_data[5];

static volatile bool ip_obtained = false;
static volatile bool wlan_connected = false;
static volatile uint32_t connected_sta_ip = false;

static void *async_request_data;
static uint8_t async_request_type=0;
static volatile bool async_request_done = false;

typedef struct {
    int size;
    struct sockaddr_in addr;
} recv_from_t;

typedef struct {
    int sock;
    struct sockaddr_in addr;
} accept_t;

typedef enum {
    WINC_MODE_STA,
    WINC_MODE_AP,
    WINC_MODE_P2P,
    WINC_MODE_BSP,
    WINC_MODE_FIRMWARE,
}winc_mode_t;

/**
 * DNS Callback.
 *
 * host: Domain name.
 * ip: Server IP.
 */
static void resolve_callback(uint8_t *host, uint32_t ip)
{
    async_request_done = true;
    *((uint32_t*) async_request_data) = ip;
}

/**
 * Sockets Callback.
 *
 * sock: Socket descriptor.
 * msg_type: Type of Socket notification. Possible types are:
 *  SOCKET_MSG_BIND
 *  SOCKET_MSG_LISTEN
 *  SOCKET_MSG_ACCEPT
 *  SOCKET_MSG_CONNECT
 *  SOCKET_MSG_SEND
 *  SOCKET_MSG_RECV
 *  SOCKET_MSG_SENDTO
 *  SOCKET_MSG_RECVFROM
 *
 * msg: A structure contains notification informations.
 *  tstrSocketBindMsg
 *  tstrSocketListenMsg
 *  tstrSocketAcceptMsg
 *  tstrSocketConnectMsg
 *  tstrSocketRecvMsg
 */
static void socket_callback(SOCKET sock, uint8_t msg_type, void *msg)
{
    if (async_request_type != msg_type) {
        debug_printf("spurious message received!"
                " expected: (%d) received: (%d)\n", async_request_type, msg_type);
        return;
    }

    switch (msg_type) {
        // Socket bind.
        case SOCKET_MSG_BIND: {
            tstrSocketBindMsg *pstrBind = (tstrSocketBindMsg *)msg;
            if (pstrBind->status == 0) {
                *((int*) async_request_data) = 0;
                debug_printf("bind success.\n");
            } else {
                *((int*) async_request_data) = -1;
                debug_printf("bind error!\n");
            }
            async_request_done = true;
            break;
        }

        // Socket listen.
        case SOCKET_MSG_LISTEN: {
            tstrSocketListenMsg *pstrListen = (tstrSocketListenMsg *)msg;
            if (pstrListen->status == 0) {
                *((int*) async_request_data) = 0;
                debug_printf("listen success.\n");
            } else {
                *((int*) async_request_data) = -1;
                debug_printf("listen error!\n");
            }
            async_request_done = true;
            break;
        }

        // Connect accept.
        case SOCKET_MSG_ACCEPT: {
            accept_t *acpt = (accept_t*) async_request_data;
            tstrSocketAcceptMsg *pstrAccept = (tstrSocketAcceptMsg *)msg;
            if(pstrAccept->sock >= 0) {
                acpt->sock = pstrAccept->sock;
				acpt->addr.sin_port = pstrAccept->strAddr.sin_port;
				acpt->addr.sin_addr = pstrAccept->strAddr.sin_addr;
                debug_printf("accept success %d.\n", pstrAccept->sock);
            } else {
                acpt->sock = pstrAccept->sock;
                debug_printf("accept error!\n");
            }
            async_request_done = true;
            break;
        }

        // Socket connected.
        case SOCKET_MSG_CONNECT: {
            tstrSocketConnectMsg *pstrConnect = (tstrSocketConnectMsg *)msg;
            if (pstrConnect->s8Error == 0) {
                *((int*) async_request_data) = 0;
                debug_printf("connect success.\n");
            } else {
                *((int*) async_request_data) = -1;
                debug_printf("connect error!\n");
            }
            async_request_done = true;
            break;
        }

        // Message send.
        case SOCKET_MSG_SEND:
        case SOCKET_MSG_SENDTO: {
            // Sent bytes set in msg.
            *((int*) async_request_data) = *((int16_t*) msg);
            async_request_done = true;
            break;
        }

        // Message receive.
        case SOCKET_MSG_RECV: {
            tstrSocketRecvMsg *pstrRecv = (tstrSocketRecvMsg *)msg;
            if (pstrRecv->s16BufferSize > 0) {
                *((int*) async_request_data) = pstrRecv->s16BufferSize;
                debug_printf("recv %d\n", pstrRecv->s16BufferSize);
            } else {
                *((int*) async_request_data) = -1;
                debug_printf("recv error! %d\n", pstrRecv->s16BufferSize);
            }
            async_request_done = true;
            break;
        }

        case SOCKET_MSG_RECVFROM: {
			tstrSocketRecvMsg *pstrRecv = (tstrSocketRecvMsg*) msg;
            recv_from_t *rfrom = (recv_from_t*) async_request_data;

            if (pstrRecv->s16BufferSize > 0) {
				// Get the remote host address and port number
                rfrom->size = pstrRecv->s16BufferSize;
				rfrom->addr.sin_port = pstrRecv->strRemoteAddr.sin_port;
				rfrom->addr.sin_addr = pstrRecv->strRemoteAddr.sin_addr;
				debug_printf("recvfrom size: %d addr:%lu port:%d\n",
                        pstrRecv->s16BufferSize, rfrom->addr.sin_addr.s_addr, rfrom->addr.sin_port);
			} else {
                rfrom->size = -1;
				debug_printf("recvfrom error:%d\n", pstrRecv->s16BufferSize);
			}
            async_request_done = true;
            break;
        }

        default:
            debug_printf("Unknown message type: %d\n", msg_type);
            break;
    }
}

/**
 * WiFi Callback in AP mode.
 *
 * msg_type: type of Wi-Fi notification. Possible types are:
 *  M2M_WIFI_RESP_CON_STATE_CHANGED
 *  M2M_WIFI_RESP_CONN_INFO
 *  M2M_WIFI_REQ_DHCP_CONF
 *  M2M_WIFI_REQ_WPS
 *  M2M_WIFI_RESP_IP_CONFLICT
 *  M2M_WIFI_RESP_SCAN_DONE
 *  M2M_WIFI_RESP_SCAN_RESULT
 *  M2M_WIFI_RESP_CURRENT_RSSI
 *  M2M_WIFI_RESP_CLIENT_INFO
 *  M2M_WIFI_RESP_PROVISION_INFO
 *  M2M_WIFI_RESP_DEFAULT_CONNECT
 *
 * In case Bypass mode is defined :
 * 	M2M_WIFI_RESP_ETHERNET_RX_PACKET
 *
 * In case Monitoring mode is used:
 * 	M2M_WIFI_RESP_WIFI_RX_PACKET
 *
 * msg: A pointer to a buffer containing the notification parameters (if any).
 * It should be casted to the correct data type corresponding to the notification type.
 */

static void wifi_callback_ap(uint8_t msg_type, void *msg)
{
    switch (msg_type) {
        case M2M_WIFI_RESP_CON_STATE_CHANGED: {
            tstrM2mWifiStateChanged *wifi_state = (tstrM2mWifiStateChanged *)msg;
            if (wifi_state->u8CurrState == M2M_WIFI_CONNECTED) {
                debug_printf("Station connected\r\n");
            } else if (wifi_state->u8CurrState == M2M_WIFI_DISCONNECTED) {
                connected_sta_ip = 0;
                debug_printf("Station disconnected\r\n");
            }
            break;
        }

        case M2M_WIFI_REQ_DHCP_CONF: {
            uint8_t *ip = (uint8_t *)msg;
            debug_printf("Station connected. IP is %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
            ((uint8_t*) &connected_sta_ip)[0] = ip[0];
            ((uint8_t*) &connected_sta_ip)[1] = ip[1];
            ((uint8_t*) &connected_sta_ip)[2] = ip[2];
            ((uint8_t*) &connected_sta_ip)[3] = ip[3];
            break;
        }

        default:
            debug_printf("Unknown message type: %d\n", msg_type);
            break;
    }
}

/**
 * WiFi Callback in STA mode.
 *
 * msg_type: type of Wi-Fi notification. Possible types are:
 *  M2M_WIFI_RESP_CON_STATE_CHANGED
 *  M2M_WIFI_RESP_CONN_INFO
 *  M2M_WIFI_REQ_DHCP_CONF
 *  M2M_WIFI_REQ_WPS
 *  M2M_WIFI_RESP_IP_CONFLICT
 *  M2M_WIFI_RESP_SCAN_DONE
 *  M2M_WIFI_RESP_SCAN_RESULT
 *  M2M_WIFI_RESP_CURRENT_RSSI
 *  M2M_WIFI_RESP_CLIENT_INFO
 *  M2M_WIFI_RESP_PROVISION_INFO
 *  M2M_WIFI_RESP_DEFAULT_CONNECT
 *
 * In case Bypass mode is defined :
 * 	M2M_WIFI_RESP_ETHERNET_RX_PACKET
 * 
 * In case Monitoring mode is used:
 * 	M2M_WIFI_RESP_WIFI_RX_PACKET
 *
 * msg: A pointer to a buffer containing the notification parameters (if any).
 * It should be casted to the correct data type corresponding to the notification type.
 */
static void wifi_callback_sta(uint8_t msg_type, void *msg)
{
    // Index of scan list to request scan result.
    static uint8_t scan_request_index = 0;

    switch (msg_type) {

        case M2M_WIFI_RESP_CURRENT_RSSI: {
            int rssi = *((int8_t *)msg);
            *((int*)async_request_data) = rssi;
            async_request_done = true;
		    break;
	    }

        case M2M_WIFI_RESP_CON_STATE_CHANGED: {
            tstrM2mWifiStateChanged *wifi_state = (tstrM2mWifiStateChanged *)msg;
            if (wifi_state->u8CurrState == M2M_WIFI_CONNECTED) {
                wlan_connected = true;
                m2m_wifi_request_dhcp_client();
            } else if (wifi_state->u8CurrState == M2M_WIFI_DISCONNECTED) {
                ip_obtained = false;
                wlan_connected = false;
                async_request_done = true;
            }
            break;
        }

        case M2M_WIFI_REQ_DHCP_CONF: {
            ip_obtained = true;
            async_request_done = true;
            break;
        }

        case M2M_WIFI_RESP_CONN_INFO: {
            // Connection info
            tstrM2MConnInfo	*con_info = (tstrM2MConnInfo*) msg;

    	    // Get MAC Address.
            uint8_t mac_addr[M2M_MAC_ADDRES_LEN];
	        m2m_wifi_get_mac_address(mac_addr);
            
            // Format MAC address
            VSTR_FIXED(mac_vstr, 18);
            vstr_printf(&mac_vstr, "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0],
                mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

            // Format IP address
            VSTR_FIXED(ip_vstr, 16);
            vstr_printf(&ip_vstr, "%d.%d.%d.%d", con_info->au8IPAddr[0],
                    con_info->au8IPAddr[1], con_info->au8IPAddr[2], con_info->au8IPAddr[3]);

            // Add connection info
            mp_obj_t info_list = (mp_obj_t) async_request_data;
            mp_obj_list_append(info_list, mp_obj_new_int(con_info->s8RSSI));
            mp_obj_list_append(info_list, mp_obj_new_int(con_info->u8SecType));
            mp_obj_list_append(info_list, mp_obj_new_str(con_info->acSSID, strlen(con_info->acSSID), false));
            mp_obj_list_append(info_list, mp_obj_new_str(mac_vstr.buf, mac_vstr.len, false));
            mp_obj_list_append(info_list, mp_obj_new_str(ip_vstr.buf, ip_vstr.len, false));

            async_request_done = true;
			break;
        }

        case M2M_WIFI_RESP_SCAN_DONE: {
            scan_request_index = 0;
            tstrM2mScanDone *scan_info = (tstrM2mScanDone*) msg;

            // The number of APs found in the last scan request.
            if (scan_info->u8NumofCh <= 0) {
                // Nothing found.
                async_request_done = true;
            } else {
                // Found APs, request scan results.
                m2m_wifi_req_scan_result(scan_request_index++);
            }

            break;
        }

        case M2M_WIFI_RESP_SCAN_RESULT: {
            tstrM2mWifiscanResult *scan_result;
            scan_result = (tstrM2mWifiscanResult*) msg;

            // Format MAC address
            VSTR_FIXED(mac_vstr, 18);
            vstr_printf(&mac_vstr, "%02X:%02X:%02X:%02X:%02X:%02X",
                    scan_result->au8BSSID[0], scan_result->au8BSSID[1], scan_result->au8BSSID[2],
                    scan_result->au8BSSID[3], scan_result->au8BSSID[4], scan_result->au8BSSID[5]);

            mp_obj_t ap[5] = {
                mp_obj_new_int(scan_result->u8ch),
                mp_obj_new_int(scan_result->s8rssi),
                mp_obj_new_int(scan_result->u8AuthType),
                mp_obj_new_str(mac_vstr.buf, mac_vstr.len, false),
                mp_obj_new_str((const char*) scan_result->au8SSID, strlen((const char*) scan_result->au8SSID), false),
            };

            mp_obj_t scan_list = (mp_obj_t) async_request_data;
            mp_obj_list_append(scan_list, mp_obj_new_tuple(MP_ARRAY_SIZE(ap), ap));

            int num_found_ap = m2m_wifi_get_num_ap_found();
            if (num_found_ap == scan_request_index) {
                async_request_done = true;
            } else {
                // Request next scan result
                m2m_wifi_req_scan_result(scan_request_index++);
            }
            break;
        }

        default:
            debug_printf("Unknown message type: %d\n", msg_type);
            break;
    }
}

static int winc_async_request(uint8_t msg_type, void *ret, uint32_t timeout)
{
    // Do async request.
    async_request_data = ret;
    async_request_done = false;
    async_request_type = msg_type;
    uint32_t tick_start = HAL_GetTick();

    // Wait for async request to finish.
    while (async_request_done == false) {
        // Handle pending events from network controller.
        m2m_wifi_handle_events(NULL);
        if (timeout && ((HAL_GetTick() - tick_start) >= timeout)) {
            return -ETIMEDOUT;
        }
    }

    return SOCK_ERR_NO_ERROR;
}

static int winc_gethostbyname(mp_obj_t nic, const char *name, mp_uint_t len, uint8_t *out_ip)
{
    int ret;
    uint32_t ip=0;
    ret = WINC1500_EXPORT(gethostbyname)((uint8_t*) name);
    if (ret == SOCK_ERR_NO_ERROR) {
        ret = winc_async_request(0, &ip, 5000);
    } else {
        return -1;
    }

    if (ip == 0) {
        // unknown host
        return -ENOENT;
    }

    out_ip[0] = ip;
    out_ip[1] = ip >> 8;
    out_ip[2] = ip >> 16;
    out_ip[3] = ip >> 24;
    return 0;
}

static int winc_socket_socket(mod_network_socket_obj_t *socket, int *_errno)
{
    uint8_t type;

    if (socket->u_param.domain != MOD_NETWORK_AF_INET) {
        *_errno = EAFNOSUPPORT;
        return -1;
    }

    switch (socket->u_param.type) {
        case MOD_NETWORK_SOCK_STREAM:
            type = SOCK_STREAM;
            break;

        case MOD_NETWORK_SOCK_DGRAM:
            type = SOCK_DGRAM;
            break;

        default:
            *_errno = EINVAL;
            return -1;
    }

    // open socket
    int fd = WINC1500_EXPORT(socket)(AF_INET, type, 0);
    if (fd < 0) {
        *_errno = fd;
        return -1;
    }

    // store state of this socket
    socket->fd = fd;
    socket->timeout = 0; // blocking
    return 0;
}

static void winc_socket_close(mod_network_socket_obj_t *socket)
{
    if (socket->fd >= 0) {
        WINC1500_EXPORT(close)(socket->fd);
        socket->fd = -1; // Mark socket FD as invalid
    }
}

static int winc_socket_bind(mod_network_socket_obj_t *socket, byte *ip, mp_uint_t port, int *_errno)
{
    MAKE_SOCKADDR(addr, ip, port)
    // Call bind and check HIF errors.
    int ret = WINC1500_EXPORT(bind)(socket->fd, &addr, sizeof(addr));
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_BIND, &ret, 1000);
    }

    if (ret != SOCK_ERR_NO_ERROR) {
        *_errno = ret;
        winc_socket_close(socket);
    }
    return ret;
}

static int winc_socket_listen(mod_network_socket_obj_t *socket, mp_int_t backlog, int *_errno)
{
    // Call listen and check HIF errors.
    int ret = WINC1500_EXPORT(listen)(socket->fd, backlog);
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_LISTEN, &ret, 1000);
    }

    if (ret != SOCK_ERR_NO_ERROR) {
        *_errno = ret;
        winc_socket_close(socket);
    }
    return ret;
}

static int winc_socket_accept(mod_network_socket_obj_t *socket, mod_network_socket_obj_t *socket2, byte *ip, mp_uint_t *port, int *_errno)
{
    // Call accept and check HIF errors.
    int ret = WINC1500_EXPORT(accept)(socket->fd, NULL, 0);
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        accept_t acpt;
        ret = winc_async_request(SOCKET_MSG_ACCEPT, &acpt, socket->timeout);

        // Check async request status.
        if (ret == SOCK_ERR_NO_ERROR && acpt.sock >= 0) {
            // Set socket state 
            socket2->fd = acpt.sock;
            socket2->timeout = 0;
            // Set ip and port
            UNPACK_SOCKADDR(((struct sockaddr*) &acpt.addr), ip, *port);
        }
    }

    if (ret != SOCK_ERR_NO_ERROR) {
        *_errno = ret;
        winc_socket_close(socket);
    }
    return ret;
}

static int winc_socket_connect(mod_network_socket_obj_t *socket, byte *ip, mp_uint_t port, int *_errno)
{
    MAKE_SOCKADDR(addr, ip, port)
    int ret = WINC1500_EXPORT(connect)(socket->fd, &addr, sizeof(addr));
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_CONNECT, &ret, socket->timeout);
    }

    if (ret != SOCK_ERR_NO_ERROR) {
        *_errno = ret;
        winc_socket_close(socket);
    }
    return ret;
}

static mp_uint_t winc_socket_send(mod_network_socket_obj_t *socket, const byte *buf, mp_uint_t len, int *_errno)
{
    mp_int_t bytes = 0;

    while (bytes < len) {
        // Split the packet into smaller ones.
        int n = MIN((len - bytes), SOCKET_BUFFER_MAX_LENGTH); 
        int ret = WINC1500_EXPORT(send)(socket->fd, (uint8_t*)buf + bytes, n, socket->timeout);

        if (ret == SOCK_ERR_NO_ERROR) {
            // Do async request
            ret = winc_async_request(SOCKET_MSG_SEND, &n, socket->timeout);

            // Check sent bytes returned from async request.
            if (ret != SOCK_ERR_NO_ERROR || n <= 0) {
                *_errno = (n <= 0)? n : ret;
                winc_socket_close(socket);
                return -1;
            }
        }
        bytes += n;
    }

    return bytes;
}

static mp_uint_t winc_socket_recv(mod_network_socket_obj_t *socket, byte *buf, mp_uint_t len, int *_errno)
{
    // cap length at SOCKET_BUFFER_MAX_LENGTH
    len = MIN(len, SOCKET_BUFFER_MAX_LENGTH);

    // do the recv
    int ret = WINC1500_EXPORT(recv)(socket->fd, buf, len, socket->timeout);
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_RECV, &len, socket->timeout);
    }

    if (ret != SOCK_ERR_NO_ERROR) {
        *_errno = ret;
        winc_socket_close(socket);
        return -1;
    }
    return len;
}

static mp_uint_t winc_socket_sendto(mod_network_socket_obj_t *socket,
        const byte *buf, mp_uint_t len, byte *ip, mp_uint_t port, int *_errno)
{
    MAKE_SOCKADDR(addr, ip, port)
    int ret = WINC1500_EXPORT(sendto)(socket->fd, (byte*)buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_SENDTO, &ret, socket->timeout);
    }

    if (ret != SOCK_ERR_NO_ERROR) {
        *_errno = ret;
        winc_socket_close(socket);
    }
    return ret;
}

static mp_uint_t winc_socket_recvfrom(mod_network_socket_obj_t *socket,
        byte *buf, mp_uint_t len, byte *ip, mp_uint_t *port, int *_errno)
{
    recv_from_t rfrom;
    int ret = WINC1500_EXPORT(recvfrom)(socket->fd, buf, len, socket->timeout);
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_RECVFROM, &rfrom, socket->timeout);
    }

    if (ret != SOCK_ERR_NO_ERROR || rfrom.size <= 0) {
        *_errno = (rfrom.size <= 0)? rfrom.size : ret;
        winc_socket_close(socket);
    } else {
        UNPACK_SOCKADDR(((struct sockaddr*) &rfrom.addr), ip, *port);
    }

    return rfrom.size;
}

static int winc_socket_setsockopt(mod_network_socket_obj_t *socket, mp_uint_t
        level, mp_uint_t opt, const void *optval, mp_uint_t optlen, int *_errno)
{
    int ret = WINC1500_EXPORT(setsockopt)(socket->fd, level, opt, optval, optlen);
    if (ret != SOCK_ERR_NO_ERROR) {
        *_errno = ret;
        winc_socket_close(socket);
    }
    return ret;
}

static int winc_socket_settimeout(mod_network_socket_obj_t *socket, mp_uint_t timeout_ms, int *_errno)
{
    socket->timeout = timeout_ms;
    return 0;
}

//static int winc_socket_ioctl(mod_network_socket_obj_t *socket, mp_uint_t request, mp_uint_t arg, int *_errno)
//{
//    return -1;
//}

/******************************************************************************/
// Micro Python bindings; WINC class

typedef struct _winc_obj_t {
    mp_obj_base_t base;
} winc_obj_t;

static const winc_obj_t winc_obj = {{(mp_obj_type_t*)&mod_network_nic_type_winc}};

// Initialise the module using the given SPI bus and pins and return a winc object.
static mp_obj_t winc_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = WINC_MODE_STA } },
    };

    // parse args
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args), allowed_args, args);

    // get mode
    winc_mode_t winc_mode = args[0].u_int;

	// Initialize the BSP.
	nm_bsp_init();

    switch (winc_mode) {
        case WINC_MODE_BSP: {
	        // Initialize the BSP and return.
            break;
        }

        case WINC_MODE_FIRMWARE: {
            // Enter download mode.
            printf("Running in Firmware Upgrade mode...\n");
            if (m2m_wifi_download_mode() != M2M_SUCCESS) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to enter download mode!"));
            }
            break;
        }

        case WINC_MODE_AP:
        case WINC_MODE_STA: {
            tstrWifiInitParam param;
            // Initialize Wi-Fi parameters structure.
            memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));
            if (winc_mode == WINC_MODE_AP) {
                param.pfAppWifiCb = wifi_callback_ap;
            } else {
                param.pfAppWifiCb = wifi_callback_sta;
            }

            // Initialize Wi-Fi driver with data and status callbacks.
            if (m2m_wifi_init(&param) != M2M_SUCCESS) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "failed to init WINC1500 module"));
            }

            uint8_t mac_addr_valid;
            uint8_t mac_addr[M2M_MAC_ADDRES_LEN];
            // Get MAC Address from OTP.
            m2m_wifi_get_otp_mac_address(mac_addr, &mac_addr_valid);

            if (!mac_addr_valid) {
                // User define MAC Address.
                const char main_user_define_mac_address[] = {0xf8, 0xf0, 0x05, 0x20, 0x0b, 0x09};
                // Cannot found MAC Address from OTP. Set user define MAC address.
                m2m_wifi_set_mac_address((uint8_t *) main_user_define_mac_address);
            }

            // Initialize socket layer.
            socketDeinit();
            socketInit();

            // Register sockets callback functions
            registerSocketCallback(socket_callback, resolve_callback);

            // Register with network module
            mod_network_register_nic((mp_obj_t)&winc_obj);

            if (winc_mode == WINC_MODE_AP) {
                printf("Running in Access Point mode...\n");
            } else {
                printf("Running in Station mode...\n");
            }

            break;
        }

        case WINC_MODE_P2P:
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "P2P mode is not supported yet!"));
            break;
        default:
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "WiFi mode is not supported!"));
    }

    return (mp_obj_t)&winc_obj;
}

// method connect(ssid, key=None, *, security=WPA2, bssid=None)
static mp_obj_t winc_connect(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_ssid, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_key, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_security, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = M2M_WIFI_SEC_WPA_PSK} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get ssid
    mp_uint_t ssid_len;
    const char *ssid = mp_obj_str_get_data(args[0].u_obj, &ssid_len);

    // get key and sec
    mp_uint_t key_len = 0;
    const char *key = NULL;
    mp_uint_t sec = M2M_WIFI_SEC_OPEN;
    if (args[1].u_obj != mp_const_none) {
        key = mp_obj_str_get_data(args[1].u_obj, &key_len);
        sec = args[2].u_int;
    }

    // connect to AP
    if (m2m_wifi_connect((char*)ssid, ssid_len, sec, (void*)key, M2M_WIFI_CH_ALL) != 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_OSError, "could not connect to ssid=%s, sec=%d, key=%s\n", ssid, sec, key));
    }

    async_request_done = false;
	while (async_request_done == false) {
		// Handle pending events from network controller.
		m2m_wifi_handle_events(NULL);
	}
    return mp_const_none;
}

// method start_ap(ssid, key=None, security=OPEN)
static mp_obj_t winc_start_ap(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_ssid,     MP_ARG_REQUIRED| MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_key,      MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_security, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = M2M_WIFI_SEC_OPEN } }, //M2M_WIFI_SEC_WPA_PSK
        { MP_QSTR_channel,  MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get ssid
    mp_uint_t ssid_len;
    const char *ssid = mp_obj_str_get_data(args[0].u_obj, &ssid_len);

    // get key and security
    mp_uint_t key_len = 0;
    const char *key = NULL;
    mp_uint_t security = args[2].u_int;

    if (security != M2M_WIFI_SEC_OPEN && security != M2M_WIFI_SEC_WEP) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "AP mode supports WEP security only."));
    }

    if (security == M2M_WIFI_SEC_WEP && args[1].u_obj == MP_OBJ_NULL) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Missing WEP key!"));
    }

    key = mp_obj_str_get_data(args[1].u_obj, &key_len);

    // get channel
    mp_uint_t channel = args[3].u_int;

    tstrM2MAPConfig apconfig;
    memset(&apconfig, 0, sizeof(tstrM2MAPConfig));
    strcpy((char *)&apconfig.au8SSID, ssid);
    apconfig.u8ListenChannel      = channel;
    apconfig.u8SecType            = security;
    apconfig.au8DHCPServerIP[0]   = 192;
    apconfig.au8DHCPServerIP[1]   = 168;
    apconfig.au8DHCPServerIP[2]   = 1;
    apconfig.au8DHCPServerIP[3]   = 1;

    if (security != M2M_WIFI_SEC_OPEN) {
        strcpy((char *)&apconfig.au8WepKey, key);
        apconfig.u8KeySz = WEP_40_KEY_STRING_SIZE;
        apconfig.u8KeyIndx = 1;
    }

    // Initialize WiFi in AP mode.
    if (m2m_wifi_enable_ap(&apconfig) != M2M_SUCCESS) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "failed to start in AP mode"));
    }

    printf("AP mode started. You can connect to %s.\r\n", ssid);
    return mp_const_none;
}

static mp_obj_t winc_disconnect(mp_obj_t self_in)
{
    m2m_wifi_disconnect();
    return mp_const_none;
}

static mp_obj_t winc_isconnected(mp_obj_t self_in)
{
    return mp_obj_new_bool(wlan_connected && ip_obtained);
}

static mp_obj_t winc_connected_sta(mp_obj_t self_in)
{
    mp_obj_t sta_list = mp_obj_new_list(0, NULL);

    if (connected_sta_ip == 0) {
		m2m_wifi_handle_events(NULL);
    }

    if (connected_sta_ip != 0) {
        mp_obj_list_append(sta_list, netutils_format_inet_addr(
                    (uint8_t*)&connected_sta_ip, 0, NETUTILS_BIG));
    }

    return sta_list;
}

static mp_obj_t winc_wait_for_sta(mp_obj_t self_in, mp_obj_t timeout_in)
{
    // get timeout
    mp_obj_t sta_list = mp_obj_new_list(0, NULL);
    mp_uint_t timeout = mp_obj_get_int(timeout_in);

    uint32_t tick_start = HAL_GetTick();
    while (connected_sta_ip == 0) {
        // Handle pending events from network controller.
        m2m_wifi_handle_events(NULL);
        if (timeout && ((HAL_GetTick() - tick_start) >= timeout)) {
            break;
        }
    }

    if (connected_sta_ip != 0) {
        mp_obj_list_append(sta_list, netutils_format_inet_addr(
                    (uint8_t*)&connected_sta_ip, 0, NETUTILS_BIG));
    }

    return sta_list;

}
static mp_obj_t winc_ifconfig(mp_obj_t self_in)
{
    mp_obj_t info_list;
    info_list = mp_obj_new_list(0, NULL);

    async_request_done = false;
    async_request_data = info_list;
	
    // Request connection info
    m2m_wifi_get_connection_info();

	while (async_request_done == false) {
		// Handle pending events from network controller.
		m2m_wifi_handle_events(NULL);
	}

    return info_list;
}

static mp_obj_t winc_scan(mp_obj_t self_in)
{
    mp_obj_t scan_list;
    scan_list = mp_obj_new_list(0, NULL);

    async_request_done = false;
    async_request_data = scan_list;
	
    // Request scan.
	m2m_wifi_request_scan(M2M_WIFI_CH_ALL);

	while (async_request_done == false) {
		// Handle pending events from network controller.
		m2m_wifi_handle_events(NULL);
	}

    return scan_list;
}

static mp_obj_t winc_rssi(mp_obj_t self_in)
{
    int rssi;
    async_request_done = false;
    async_request_data = &rssi;
	
    // Request RSSI.
    m2m_wifi_req_curr_rssi();

	while (async_request_done == false) {
		// Handle pending events from network controller.
		m2m_wifi_handle_events(NULL);
	}

    return mp_obj_new_int(rssi); 
}

static mp_obj_t winc_fw_version(mp_obj_t self_in)
{
    tstrM2mRev fwver;
    mp_obj_tuple_t *t_fwver;

    // Read FW, Driver and HW versions.
    m2m_wifi_get_firmware_version(&fwver);

    t_fwver = mp_obj_new_tuple(7, NULL);
	t_fwver->items[0] = mp_obj_new_int(fwver.u8FirmwareMajor);     // Firmware version major number.
	t_fwver->items[1] = mp_obj_new_int(fwver.u8FirmwareMinor);     // Firmware version minor number.
	t_fwver->items[2] = mp_obj_new_int(fwver.u8FirmwarePatch);     // Firmware version patch number.
	t_fwver->items[3] = mp_obj_new_int(fwver.u8DriverMajor);       // Driver version major number.
	t_fwver->items[4] = mp_obj_new_int(fwver.u8DriverMinor);       // Driver version minor number.
	t_fwver->items[5] = mp_obj_new_int(fwver.u8DriverPatch);       // Driver version patch number.
	t_fwver->items[6] = mp_obj_new_int(fwver.u32Chipid);           // HW revision number (chip ID).
    return t_fwver;
}

static mp_obj_t winc_fw_dump(mp_obj_t self_in, mp_obj_t path_in)
{
    // Erase the WINC1500 flash.
    printf("Dumping firmware...\n");

    const char *path = mp_obj_str_get_str(path_in);
    if (dump_firmware(path) != M2M_SUCCESS) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to dump flash!"));
    }

    return mp_const_none;
}

static mp_obj_t winc_fw_update(mp_obj_t self_in, mp_obj_t path_in)
{
    FRESULT res;
    FILINFO fno;
    const char *path = mp_obj_str_get_str(path_in);

    if ((res = f_stat_helper(path, &fno)) != FR_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    // Erase the WINC1500 flash.
    printf("Erasing flash...\n");
    if (programmer_erase_all() != M2M_SUCCESS) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to erase entire flash!"));
    }

    // Program the firmware on the WINC1500 flash.
    printf("Programming firmware...\n");
    if (burn_firmware(path) != M2M_SUCCESS) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Error while writing firmware!"));
    }

    // Verify the firmware on the WINC1500 flash.
    printf("Verifying firmware image...\n");
    if (verify_firmware(path) != M2M_SUCCESS) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to verify firmware section!"));
    }

    printf("All task completed successfully.\n");
    return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_KW(winc_connect_obj, 1,  winc_connect);
static MP_DEFINE_CONST_FUN_OBJ_KW(winc_start_ap_obj,1,  winc_start_ap);
static MP_DEFINE_CONST_FUN_OBJ_1(winc_disconnect_obj,   winc_disconnect);
static MP_DEFINE_CONST_FUN_OBJ_1(winc_isconnected_obj,  winc_isconnected);
static MP_DEFINE_CONST_FUN_OBJ_1(winc_connected_sta_obj,winc_connected_sta);
static MP_DEFINE_CONST_FUN_OBJ_2(winc_wait_for_sta_obj, winc_wait_for_sta);
static MP_DEFINE_CONST_FUN_OBJ_1(winc_ifconfig_obj,     winc_ifconfig);
static MP_DEFINE_CONST_FUN_OBJ_1(winc_scan_obj,         winc_scan);
static MP_DEFINE_CONST_FUN_OBJ_1(winc_rssi_obj,         winc_rssi);
static MP_DEFINE_CONST_FUN_OBJ_1(winc_fw_version_obj,   winc_fw_version);
static MP_DEFINE_CONST_FUN_OBJ_2(winc_fw_dump_obj,      winc_fw_dump);
static MP_DEFINE_CONST_FUN_OBJ_2(winc_fw_update_obj,    winc_fw_update);

static const mp_map_elem_t winc_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_connect),         (mp_obj_t)&winc_connect_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start_ap),        (mp_obj_t)&winc_start_ap_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_disconnect),      (mp_obj_t)&winc_disconnect_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_isconnected),     (mp_obj_t)&winc_isconnected_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_connected_sta),   (mp_obj_t)&winc_connected_sta_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_wait_for_sta),    (mp_obj_t)&winc_wait_for_sta_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ifconfig),        (mp_obj_t)&winc_ifconfig_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_scan),            (mp_obj_t)&winc_scan_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_rssi),            (mp_obj_t)&winc_rssi_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_fw_version),      (mp_obj_t)&winc_fw_version_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_fw_dump),         (mp_obj_t)&winc_fw_dump_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_fw_update),       (mp_obj_t)&winc_fw_update_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_OPEN),            MP_OBJ_NEW_SMALL_INT(M2M_WIFI_SEC_OPEN) },      // Network is not secured.
    { MP_OBJ_NEW_QSTR(MP_QSTR_WEP),             MP_OBJ_NEW_SMALL_INT(M2M_WIFI_SEC_WEP) },       // Security type WEP (40 or 104) OPEN OR SHARED.
    { MP_OBJ_NEW_QSTR(MP_QSTR_WPA_PSK),         MP_OBJ_NEW_SMALL_INT(M2M_WIFI_SEC_WPA_PSK) },   // Network is secured with WPA/WPA2 personal(PSK).
    { MP_OBJ_NEW_QSTR(MP_QSTR_802_1X),          MP_OBJ_NEW_SMALL_INT(M2M_WIFI_SEC_802_1X) },    // Network is secured with WPA/WPA2 Enterprise.

    { MP_OBJ_NEW_QSTR(MP_QSTR_MODE_STA),        MP_OBJ_NEW_SMALL_INT(WINC_MODE_STA) },          // Start in Staion mode.
    { MP_OBJ_NEW_QSTR(MP_QSTR_MODE_AP),         MP_OBJ_NEW_SMALL_INT(WINC_MODE_AP) },           // Start in Access Point mode.
    { MP_OBJ_NEW_QSTR(MP_QSTR_MODE_P2P),        MP_OBJ_NEW_SMALL_INT(WINC_MODE_P2P) },          // Start in P2P (WiFi Direct) mode.
    { MP_OBJ_NEW_QSTR(MP_QSTR_MODE_BSP),        MP_OBJ_NEW_SMALL_INT(WINC_MODE_BSP) },          // Init BSP.
    { MP_OBJ_NEW_QSTR(MP_QSTR_MODE_FIRMWARE),   MP_OBJ_NEW_SMALL_INT(WINC_MODE_FIRMWARE) },     // Start in Firmware Upgrade mode.
};

static MP_DEFINE_CONST_DICT(winc_locals_dict, winc_locals_dict_table);

const mod_network_nic_type_t mod_network_nic_type_winc = {
    .base = {
        { &mp_type_type },
        .name = MP_QSTR_WINC,
        .make_new = winc_make_new,
        .locals_dict = (mp_obj_t)&winc_locals_dict,
    },
    .gethostbyname = winc_gethostbyname,
    .socket = winc_socket_socket,
    .close = winc_socket_close,
    .bind = winc_socket_bind,
    .listen = winc_socket_listen,
    .accept = winc_socket_accept,
    .connect = winc_socket_connect,
    .send = winc_socket_send,
    .recv = winc_socket_recv,
    .sendto = winc_socket_sendto,
    .recvfrom = winc_socket_recvfrom,
    .setsockopt = winc_socket_setsockopt,
    .settimeout = winc_socket_settimeout,
    //.ioctl = winc_socket_ioctl,
};

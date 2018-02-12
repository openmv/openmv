/*/
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * WINC1500 driver.
 *
 */
#include "winc.h"
#include "common.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include STM32_HAL_H

// WINC's includes
#include "driver/include/nmasic.h"
#include "socket/include/socket.h"
#include "programmer/programmer.h"
#include "driver/include/m2m_wifi.h"

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

typedef struct {
    void *arg;
    winc_scan_callback_t cb;
} scan_arg_t;

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
            winc_ifconfig_t *ifconfig = (winc_ifconfig_t *) async_request_data;

            // Set rssi and security
            ifconfig->rssi      = con_info->s8RSSI;
            ifconfig->security  = con_info->u8SecType;

    	    // Get MAC Address.
	        m2m_wifi_get_mac_address(ifconfig->mac_addr);
            
            // Copy IP address.
            memcpy(ifconfig->ip_addr, con_info->au8IPAddr, WINC_IP_ADDR_LEN);

            // Copy SSID.
            strncpy(ifconfig->ssid, con_info->acSSID, WINC_MAX_SSID_LEN-1);

            async_request_done = true;
			break;
        }

        case M2M_WIFI_RESP_SCAN_DONE: {
            scan_request_index = 0;
            tstrM2mScanDone *scan_result = (tstrM2mScanDone*) msg;

            // The number of APs found in the last scan request.
            if (scan_result->u8NumofCh <= 0) {
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
            
            winc_scan_result_t wscan_result;
            scan_arg_t *scan_arg = (scan_arg_t*) async_request_data;

            // Set channel, rssi and security.
            wscan_result.channel    = scan_result->u8ch;
            wscan_result.rssi       = scan_result->s8rssi;
            wscan_result.security   = scan_result->u8AuthType;

            // Copy BSSID and SSID.
            memcpy(wscan_result.bssid, scan_result->au8BSSID, WINC_MAC_ADDR_LEN);
            strncpy((char*) wscan_result.ssid, (const char *) scan_result->au8SSID, WINC_MAX_SSID_LEN-1);

            // Call scan result callback
            scan_arg->cb(&wscan_result, scan_arg->arg);

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

int winc_init(winc_mode_t winc_mode)
{
	// Initialize the BSP.
	nm_bsp_init();

    switch (winc_mode) {
        case WINC_MODE_BSP: {
	        // Initialize the BSP and return.
            break;
        }

        case WINC_MODE_FIRMWARE: {
            // Enter download mode.
            if (m2m_wifi_download_mode() != M2M_SUCCESS) {
                return -1;
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
                return -1;
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
            break;
        }
        default:
            break;
    }

    return 0;
}

int winc_connect(const char *ssid, uint8_t security, const char *key, uint16_t channel)
{
    // Connect to AP
    if (m2m_wifi_connect((char*)ssid, strlen(ssid), security, (void*)key, M2M_WIFI_CH_ALL) != 0) {
        return -1;
    }

    async_request_done = false;
	while (async_request_done == false) {
		// Handle pending events from network controller.
		m2m_wifi_handle_events(NULL);
	}
    return 0;
}

int winc_start_ap(const char *ssid, uint8_t security, const char *key, uint16_t channel)
{
    tstrM2MAPConfig apconfig;

    if (security == M2M_WIFI_SEC_WEP && key == NULL) {
        return -1;
    }

    if (security != M2M_WIFI_SEC_OPEN && security != M2M_WIFI_SEC_WEP) {
        return -1;
    }

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
        return -1;
    }

    return 0;
}

int winc_disconnect()
{
    return m2m_wifi_disconnect();
}

int winc_isconnected()
{
    return (wlan_connected && ip_obtained);
}

int winc_connected_sta(uint32_t *sta_ip)
{
    if (connected_sta_ip == 0) {
		m2m_wifi_handle_events(NULL);
    }

    *sta_ip = connected_sta_ip;
    return 0; 
}

int winc_wait_for_sta(uint32_t *sta_ip, uint32_t timeout)
{
    uint32_t tick_start = HAL_GetTick();
    while (connected_sta_ip == 0) {
        // Handle pending events from network controller.
        m2m_wifi_handle_events(NULL);
        if (timeout && ((HAL_GetTick() - tick_start) >= timeout)) {
            break;
        }
    }

    *sta_ip = connected_sta_ip;
    return 0; 
}

int winc_ifconfig(winc_ifconfig_t *ifconfig)
{
    async_request_done = false;
    async_request_data = ifconfig;
	
    // Request connection info
    m2m_wifi_get_connection_info();

	while (async_request_done == false) {
		// Handle pending events from network controller.
		m2m_wifi_handle_events(NULL);
	}

    return 0;
}

int winc_scan(winc_scan_callback_t cb, void *arg)
{
    scan_arg_t scan_arg = {arg, cb};
    async_request_done = false;
    async_request_data = &scan_arg;
	
    // Request scan.
	m2m_wifi_request_scan(M2M_WIFI_CH_ALL);

	while (async_request_done == false) {
		// Handle pending events from network controller.
		m2m_wifi_handle_events(NULL);
	}

    return 0;
}

int winc_get_rssi()
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

    return rssi; 
}

int winc_fw_version(winc_fwver_t *wfwver)
{
    tstrM2mRev fwver;

    // Read FW, Driver and HW versions.
    m2m_wifi_get_firmware_version(&fwver);

	wfwver->fw_major  = fwver.u8FirmwareMajor;     // Firmware version major number.
	wfwver->fw_minor  = fwver.u8FirmwareMinor;     // Firmware version minor number.
	wfwver->fw_patch  = fwver.u8FirmwarePatch;     // Firmware version patch number.
	wfwver->drv_major = fwver.u8DriverMajor;       // Driver version major number.
	wfwver->drv_minor = fwver.u8DriverMinor;       // Driver version minor number.
	wfwver->drv_patch = fwver.u8DriverPatch;       // Driver version patch number.
	wfwver->chip_id   = fwver.u32Chipid;           // HW revision number (chip ID).
    return 0;
}

int winc_flash_dump(const char *path)
{
    if (dump_firmware(path) != M2M_SUCCESS) {
        return -1;
    }
    return 0;
}

int winc_flash_erase()
{
    // Erase the WINC1500 flash.
    if (programmer_erase_all() != M2M_SUCCESS) {
        return -1;
    }
    return 0;
}

int winc_flash_write(const char *path)
{
    // Program the firmware on the WINC1500 flash.
    if (burn_firmware(path) != M2M_SUCCESS) {
        return -1;
    }
    return 0;
}

int winc_flash_verify(const char *path)
{
    // Verify the firmware on the WINC1500 flash.
    if (verify_firmware(path) != M2M_SUCCESS) {
        return -1;
    }
    return 0;
}

int winc_gethostbyname(const char *name, uint8_t *out_ip)
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

int winc_socket_socket(uint8_t type)
{
    // open socket
    return WINC1500_EXPORT(socket)(AF_INET, type, 0);
}

void winc_socket_close(int fd)
{
    WINC1500_EXPORT(close)(fd);
}

int winc_socket_bind(int fd, sockaddr *addr)
{
    // Call bind and check HIF errors.
    int ret = WINC1500_EXPORT(bind)(fd, addr, sizeof(*addr));
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_BIND, &ret, 1000);
    }

    return ret;
}

int winc_socket_listen(int fd, uint32_t backlog)
{
    // Call listen and check HIF errors.
    int ret = WINC1500_EXPORT(listen)(fd, backlog);
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_LISTEN, &ret, 1000);
    }

    return ret;
}

int winc_socket_accept(int fd, sockaddr *addr, int *fd_out, uint32_t timeout)
{
    accept_t acpt;

    // Call accept and check HIF errors.
    int ret = WINC1500_EXPORT(accept)(fd, NULL, 0);

    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_ACCEPT, &acpt, timeout);

        // Check async request status.
        if (ret == SOCK_ERR_NO_ERROR && acpt.sock >= 0) {
            *fd_out = acpt.sock;
            *addr = *((sockaddr*) &acpt.addr);
        }
    }

    return ret;
}

int winc_socket_connect(int fd, sockaddr *addr, uint32_t timeout)
{
    int ret = WINC1500_EXPORT(connect)(fd, addr, sizeof(*addr));
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_CONNECT, &ret, timeout);
    }

    return ret;
}

int winc_socket_send(int fd, const uint8_t *buf, uint32_t len, uint32_t timeout)
{
    int bytes = 0;

    while (bytes < len) {
        // Split the packet into smaller ones.
        int n = MIN((len - bytes), SOCKET_BUFFER_MAX_LENGTH); 
        int ret = WINC1500_EXPORT(send)(fd, (uint8_t*)buf + bytes, n, timeout);

        if (ret == SOCK_ERR_NO_ERROR) {
            // Do async request
            ret = winc_async_request(SOCKET_MSG_SEND, &n, timeout);

            // Check sent bytes returned from async request.
            if (ret != SOCK_ERR_NO_ERROR || n <= 0) {
                return (n <= 0)? n : ret;
            }
        }
        bytes += n;
    }

    return bytes;
}

int winc_socket_recv(int fd, uint8_t *buf, uint32_t len, uint32_t timeout)
{
    // cap length at SOCKET_BUFFER_MAX_LENGTH
    len = MIN(len, SOCKET_BUFFER_MAX_LENGTH);

    // do the recv
    int ret = WINC1500_EXPORT(recv)(fd, buf, len, timeout);
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_RECV, &len, timeout);
    }

    if (ret != SOCK_ERR_NO_ERROR) {
        return ret;
    }
    return len;
}

int winc_socket_sendto(int fd, const uint8_t *buf, uint32_t len, sockaddr *addr, uint32_t timeout)
{
    int ret = WINC1500_EXPORT(sendto)(fd, (uint8_t*)buf, len, 0, addr, sizeof(*addr));
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_SENDTO, &ret, timeout);
    }

    return ret;
}

int winc_socket_recvfrom(int fd, uint8_t *buf, uint32_t len, sockaddr *addr, uint32_t timeout)
{
    recv_from_t rfrom;
    int ret = WINC1500_EXPORT(recvfrom)(fd, buf, len, timeout);
    if (ret == SOCK_ERR_NO_ERROR) {
        // Do async request
        ret = winc_async_request(SOCKET_MSG_RECVFROM, &rfrom, timeout);
    }

    if (ret != SOCK_ERR_NO_ERROR || rfrom.size <= 0) {
        return (rfrom.size <= 0)? rfrom.size : ret;
    }

    *addr = *((struct sockaddr*) &rfrom.addr);
    return rfrom.size;
}

int winc_socket_setsockopt(int fd, uint32_t level, uint32_t opt, const void *optval, uint32_t optlen)
{
    return WINC1500_EXPORT(setsockopt)(fd, level, opt, optval, optlen);
}

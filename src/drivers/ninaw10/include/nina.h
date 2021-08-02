/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * NINA-W10 driver.
 */
#ifndef __NINA_H__
#define __NINA_H__
#define NINA_FW_VER_LEN         (6)
#define NINA_IPV4_ADDR_LEN      (4)
#define NINA_MAC_ADDR_LEN       (6)
#define NINA_MAX_SSID_LEN       (32)
#define NINA_MAX_WEP_LEN        (13)
#define NINA_MAX_WPA_LEN        (63)
#define NINA_MAX_NETWORK_LIST   (10)
#define	NINA_MAX_SOCKET         (10)

#define NINA_FW_VER_MAJOR       (1)
#define NINA_FW_VER_MINOR       (4)
#define NINA_FW_VER_PATCH       (8)

#define NINA_FW_VER_MAJOR_OFFS  (0)
#define NINA_FW_VER_MINOR_OFFS  (2)
#define NINA_FW_VER_PATCH_OFFS  (4)

typedef enum {
    NINA_SEC_INVALID = 0,
    NINA_SEC_OPEN,
    NINA_SEC_WPA_PSK,
    NINA_SEC_WEP
} nina_security_t;

typedef enum {
    NINA_SOCKET_TYPE_TCP = 0,
    NINA_SOCKET_TYPE_UDP,
    NINA_SOCKET_TYPE_TLS,
    NINA_SOCKET_TYPE_UDP_MULTICAST,
    NINA_SOCKET_TYPE_TLS_BEARSSL
} nina_socket_type_t;

typedef enum {
    NINA_ERROR_IO       = -1,
    NINA_ERROR_TIMEOUT  = -2,
} nina_error_t;


typedef enum {
    NINA_WIFI_MODE_STA = 0,
    NINA_WIFI_MODE_AP,
} nina_wifi_mode_t;

typedef struct {
    uint8_t ip_addr[NINA_IPV4_ADDR_LEN];
    uint8_t subnet_addr[NINA_IPV4_ADDR_LEN];
    uint8_t gateway_addr[NINA_IPV4_ADDR_LEN];
    uint8_t dns_addr[NINA_IPV4_ADDR_LEN];
} nina_ifconfig_t;

typedef struct {
    int8_t  rssi;
    uint8_t security;
    uint8_t channel;
    uint8_t bssid[NINA_MAC_ADDR_LEN];
    char    ssid[NINA_MAX_SSID_LEN];
} nina_scan_result_t;

typedef struct {
    int8_t  rssi;
    uint8_t security;
    char    ssid[NINA_MAX_SSID_LEN];
    uint8_t bssid[NINA_MAC_ADDR_LEN];
} nina_netinfo_t;

typedef int (*nina_scan_callback_t) (nina_scan_result_t *, void *);

int nina_init();
int nina_deinit();
int nina_connect(const char *ssid, uint8_t security, const char *key, uint16_t channel);
int nina_start_ap(const char *ssid, uint8_t security, const char *key, uint16_t channel);
int nina_disconnect();
int nina_isconnected();
int nina_connected_sta(uint32_t *sta_ip);
int nina_wait_for_sta(uint32_t *sta_ip, uint32_t timeout);
int nina_ifconfig(nina_ifconfig_t *ifconfig, bool set);
int nina_netinfo(nina_netinfo_t *netinfo);
int nina_scan(nina_scan_callback_t scan_callback, void *arg, uint32_t timeout);
int nina_get_rssi();
int nina_fw_version(uint8_t *fw_ver);
int nina_set_hostname(const char *name);
int nina_gethostbyname(const char *name, uint8_t *out_ip);
int nina_socket_socket(uint8_t type);
int nina_socket_close(int fd);
int nina_socket_bind(int fd, uint8_t *ip, uint16_t port, int type);
int nina_socket_listen(int fd, uint32_t backlog);
int nina_socket_accept(int fd, uint8_t *ip, uint16_t *port, int *fd_out, uint32_t timeout);
int nina_socket_connect(int fd, uint8_t *ip, uint16_t port, uint32_t timeout);
int nina_socket_send(int fd, const uint8_t *buf, uint32_t len, uint32_t timeout);
int nina_socket_recv(int fd, uint8_t *buf, uint32_t len, uint32_t timeout);
int nina_socket_sendto(int fd, const uint8_t *buf, uint32_t len, uint8_t *ip, uint16_t port, uint32_t timeout);
int nina_socket_recvfrom(int fd, uint8_t *buf, uint32_t len, uint8_t *ip, uint16_t *port, uint32_t timeout);
int nina_socket_setsockopt(int fd, uint32_t level, uint32_t opt, const void *optval, uint32_t optlen);
#endif //define __NINA_H__

/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * WINC1500 driver.
 */
#ifndef __WINC_H__
#define __WINC_H__
#include <stdint.h>
#include <stdbool.h>
#define WINC_IPV4_ADDR_LEN      (4)
#define WINC_MAC_ADDR_LEN       (6)
#define WINC_MAX_SSID_LEN       (33)
#define WINC_MAX_PSK_LEN        (65)
#define WINC_MAX_BOARD_NAME_LEN (33)
// NOTE: Due to the way the WINC1500 HIF is designed, a single recv() call reads all the data received on the socket, which
// can result in multiple callbacks if the received data is more than the buffer size passed to the recv() call. As a result,
// the async call handler will keep overwriting the user-provided buffer in subsequent callbacks. The socket buffer is used
// as workaround to this issue to allow receiving partial packets. Note the maximum size of WINC internal socket buffer seems
// to change with host-driver/firmware updates. It's very important to make sure this value is still valid after an update.
#define WINC_SOCKBUF_MAX_SIZE   (1480)
#define WINC_REQUEST_TIMEOUT    (5000)

#define MAKE_SOCKADDR(addr, ip, port) \
    struct sockaddr addr; \
    addr.sa_family = AF_INET; \
    addr.sa_data[0] = (uint8_t)(port >> 8); \
    addr.sa_data[1] = (uint8_t)(port); \
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

typedef enum {
    WINC_MODE_STA,
    WINC_MODE_AP,
    WINC_MODE_P2P,
    WINC_MODE_BSP,
    WINC_MODE_FIRMWARE,
} winc_mode_t;

typedef enum {
    WINC_SEC_INVALID = 0,
    WINC_SEC_OPEN,
    WINC_SEC_WPA_PSK,
    WINC_SEC_WEP,
    WINC_SEC_802_1X
} winc_security_t;

typedef struct {
    uint8_t ip_addr[WINC_IPV4_ADDR_LEN];
    uint8_t subnet_addr[WINC_IPV4_ADDR_LEN];
    uint8_t gateway_addr[WINC_IPV4_ADDR_LEN];
    uint8_t dns_addr[WINC_IPV4_ADDR_LEN];
} winc_ifconfig_t;

typedef struct {
    int8_t  rssi;
    uint8_t security;
    uint8_t channel;
    uint8_t bssid[6];
    char    ssid[WINC_MAX_SSID_LEN];
} winc_scan_result_t;

typedef struct {
    int8_t  rssi;
    uint8_t security;
    char    ssid[WINC_MAX_SSID_LEN];
    uint8_t ip_addr[WINC_IPV4_ADDR_LEN];
    uint8_t mac_addr[WINC_MAC_ADDR_LEN];
} winc_netinfo_t;

typedef int (*winc_scan_callback_t) (winc_scan_result_t *, void *);

typedef struct {
    uint8_t fw_major;       // Firmware version major number.
    uint8_t fw_minor;       // Firmware version minor number.
    uint8_t fw_patch;       // Firmware version patch number.
    uint8_t drv_major;      // Driver version major number.
    uint8_t drv_minor;      // Driver version minor number.
    uint8_t drv_patch;      // Driver version patch number.
    uint32_t chip_id;       // HW revision number (chip ID).
} winc_fwver_t;

typedef struct {
    int16_t  fd;
    uint16_t timeout;
} winc_socket_t;

// Sock buffer workaround for WINC's recv issue.
typedef struct {
    int idx;
    int size;
    uint8_t buf[WINC_SOCKBUF_MAX_SIZE];
} winc_socket_buf_t;

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

const char *winc_strerror(int error);
int winc_init(winc_mode_t winc_mode);
int winc_connect(const char *ssid, uint8_t security, const char *key, uint16_t channel);
int winc_start_ap(const char *ssid, uint8_t security, const char *key, uint16_t channel);
int winc_disconnect();
int winc_isconnected();
int winc_connected_sta(uint32_t *sta_ip);
int winc_wait_for_sta(uint32_t *sta_ip, uint32_t timeout);
int winc_ifconfig(winc_ifconfig_t *ifconfig, bool set);
int winc_netinfo(winc_netinfo_t *netinfo);
int winc_scan(winc_scan_callback_t cb, void *arg);
int winc_get_rssi();
int winc_fw_version(winc_fwver_t *wfwver);
int winc_flash_dump(const char *path);
int winc_flash_erase();
int winc_flash_write(const char *path);
int winc_flash_verify(const char *path);
int winc_gethostbyname(const char *name, uint8_t *out_ip);
int winc_socket_socket(uint8_t type);
void winc_socket_close(int fd);
int winc_socket_bind(int fd, sockaddr *addr);
int winc_socket_listen(int fd, uint32_t backlog);
int winc_socket_accept(int fd, sockaddr *addr, int *fd_out, uint32_t timeout);
int winc_socket_connect(int fd, sockaddr *addr, uint32_t timeout);
int winc_socket_send(int fd, const uint8_t *buf, uint32_t len, uint32_t timeout);
int winc_socket_recv(int fd, uint8_t *buf, uint32_t len, winc_socket_buf_t *sockbuf, uint32_t timeout);
int winc_socket_sendto(int fd, const uint8_t *buf, uint32_t len, sockaddr *addr, uint32_t timeout);
int winc_socket_recvfrom(int fd, uint8_t *buf, uint32_t len, sockaddr *addr, uint32_t timeout);
int winc_socket_setsockopt(int fd, uint32_t level, uint32_t opt, const void *optval, uint32_t optlen);

#endif //__WINC_H__

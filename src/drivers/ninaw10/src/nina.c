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
#if MICROPY_PY_NINAW10
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "py/mphal.h"
#include "nina.h"
#include "nina_bsp.h"

#define DATA_FLAG               (0x40)

#define SPI_ACK                 (1)
#define SPI_ERR                 (0xFF)

#define NO_SOCKET_AVAIL         (255)

#define CMD_START               (0xE0)
#define CMD_END                 (0xEE)
#define CMD_ERROR               (0xEF)
#define CMD_REPLY               (1<<7)

#define ARG_8BITS               (1)
#define ARG_16BITS              (2)

#define ARG_STR(x)              {strlen(x), (const void *) x}
#define ARG_BYTE(x)             {1, (uint8_t  [1]){x}}
#define ARG_SHORT(x)            {2, (uint16_t [1]){x}}
#define ARG_WORD(x)             {4, (uint32_t [1]){x}}

#define NINA_ARGS(...)          (nina_args_t []){__VA_ARGS__}
#define NINA_VALS(...)          (nina_vals_t []){__VA_ARGS__}

#define NINA_SSELECT_TIMEOUT    (10000)
#define NINA_RESP_TIMEOUT       (1000)
#define NINA_CONNECT_TIMEOUT    (10000)

#if NINA_DEBUG
#define debug_printf(...) mp_printf(&mp_plat_print, __VA_ARGS__)
#else
#define debug_printf(...)
#endif

typedef struct {
    uint16_t  size;
    const void *data;
} nina_args_t;

typedef struct {
    uint16_t *size;
    void  *data;
} nina_vals_t;

typedef enum {
    SET_NET_CMD             = 0x10,
    SET_PASSPHRASE_CMD      = 0x11,
    SET_KEY_CMD             = 0x12,
    //TEST_CMD              = 0x13,
    SET_IP_CONFIG_CMD       = 0x14,
    SET_DNS_CONFIG_CMD      = 0x15,
    SET_HOSTNAME_CMD        = 0x16,
    SET_POWER_MODE_CMD      = 0x17,
    SET_AP_NET_CMD          = 0x18,
    SET_AP_PASSPHRASE_CMD   = 0x19,
    SET_DEBUG_CMD           = 0x1A,
    GET_TEMPERATURE_CMD     = 0x1B,
    GET_REASON_CODE_CMD     = 0x1F,

    GET_CONN_STATUS_CMD     = 0x20,
    GET_IPADDR_CMD          = 0x21,
    GET_MACADDR_CMD         = 0x22,
    GET_CURR_SSID_CMD       = 0x23,
    GET_CURR_BSSID_CMD      = 0x24,
    GET_CURR_RSSI_CMD       = 0x25,
    GET_CURR_ENCT_CMD       = 0x26,
    SCAN_NETWORKS           = 0x27,
    START_SERVER_TCP_CMD    = 0x28,
    GET_STATE_TCP_CMD       = 0x29,
    DATA_SENT_TCP_CMD       = 0x2A,
    AVAIL_DATA_TCP_CMD      = 0x2B,
    GET_DATA_TCP_CMD        = 0x2C,
    START_CLIENT_TCP_CMD    = 0x2D,
    STOP_CLIENT_TCP_CMD     = 0x2E,
    GET_CLIENT_STATE_TCP_CMD= 0x2F,
    DISCONNECT_CMD          = 0x30,
    //GET_IDX_SSID_CMD      = 0x31,
    GET_IDX_RSSI_CMD        = 0x32,
    GET_IDX_ENCT_CMD        = 0x33,
    REQ_HOST_BY_NAME_CMD    = 0x34,
    GET_HOST_BY_NAME_CMD    = 0x35,
    START_SCAN_NETWORKS     = 0x36,
    GET_FW_VERSION_CMD      = 0x37,
    //GET_TEST_CMD          = 0x38,
    SEND_DATA_UDP_CMD       = 0x39,
    GET_REMOTE_DATA_CMD     = 0x3A,
    GET_TIME_CMD            = 0x3B,
    GET_IDX_BSSID_CMD       = 0x3C,
    GET_IDX_CHANNEL_CMD     = 0x3D,
    PING_CMD                = 0x3E,
    GET_SOCKET_CMD          = 0x3F,

    // All command with DATA_FLAG 0x40 send a 16bit Len
    SET_ENT_CMD             = 0x40,
    SEND_DATA_TCP_CMD       = 0x44,
    GET_DATABUF_TCP_CMD     = 0x45,
    INSERT_DATABUF_CMD      = 0x46,

    // regular format commands
    SET_PIN_MODE            = 0x50,
    SET_DIGITAL_WRITE       = 0x51,
    SET_ANALOG_WRITE        = 0x52,
    GET_DIGITAL_READ        = 0x53,
    GET_ANALOG_READ         = 0x54,

    // regular format commands
    CMD_WRITE_FILE          = 0x60,
    CMD_READ_FILE           = 0x61,
    CMD_DELETE_FILE         = 0x62,
    CMD_EXISTS_FILE         = 0x63,
    CMD_DOWNLOAD_FILE       = 0x64,
    CMD_APPLY_OTA_COMMAND   = 0x65,
    CMD_RENAME_FILE         = 0x66,
    CMD_DOWNLOAD_OTA        = 0x67,
} nina_cmd_t;

typedef enum {
    WL_NO_SHIELD        = 255,
    WL_NO_MODULE        = WL_NO_SHIELD,
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL,
    WL_SCAN_COMPLETED,
    WL_CONNECTED,
    WL_CONNECT_FAILED,
    WL_CONNECTION_LOST,
    WL_DISCONNECTED,
    WL_AP_LISTENING,
    WL_AP_CONNECTED,
    WL_AP_FAILED
} nina_wl_status_t;

typedef enum  {
    SOCKET_STATE_CLOSED = 0,
    SOCKET_STATE_LISTEN,
    SOCKET_STATE_SYN_SENT,
    SOCKET_STATE_SYN_RCVD,
    SOCKET_STATE_ESTABLISHED,
    SOCKET_STATE_FIN_WAIT_1,
    SOCKET_STATE_FIN_WAIT_2,
    SOCKET_STATE_CLOSE_WAIT,
    SOCKET_STATE_CLOSING,
    SOCKET_STATE_LAST_ACK,
    SOCKET_STATE_TIME_WAIT
} nina_sock_state_t;

static uint8_t nina_bsp_spi_read_byte()
{
    uint8_t byte = 0;
    nina_bsp_spi_transfer(NULL, &byte, 1);
    return byte;
}

static int nina_wait_for_cmd(uint8_t cmd, uint32_t timeout)
{
    uint8_t buf = 0;
    for (mp_uint_t start = mp_hal_ticks_ms(); ;) {
        buf = nina_bsp_spi_read_byte();
        if (buf == CMD_ERROR || buf == cmd
                || ((mp_hal_ticks_ms() - start) >= timeout)) {
            break;
        }
        mp_hal_delay_ms(1);
    }

    return ((buf == cmd) ? 0 : -1);
}

static int nina_send_command(uint32_t cmd, uint32_t nargs, uint32_t width, nina_args_t *args)
{
    int ret = -1;
    uint32_t length = 4; // 3 bytes header + 1 end byte

    debug_printf("nina_send_command (cmd 0x%x nargs %d width %d): ", cmd, nargs, width);

    if (nina_bsp_spi_slave_select(NINA_SSELECT_TIMEOUT) != 0) {
        return -1;
    }

    // Send command header.
    uint8_t cmdbuf_hdr[3] = {CMD_START, cmd, nargs}; 
    if (nina_bsp_spi_transfer(cmdbuf_hdr, NULL, sizeof(cmdbuf_hdr)) != 0) {
        goto error_out;
    }

    // Send command arg(s).
    for (uint32_t i=0; i<nargs; i++) {
        // Send size MSB first if 2 bytes.
        uint16_t size = (width == ARG_8BITS) ? args[i].size : __REVSH(args[i].size);

        // Send arg length.
        if (nina_bsp_spi_transfer((uint8_t *) &size, NULL, width) != 0) {
            goto error_out;
        }

        // Send arg value.
        if (nina_bsp_spi_transfer(args[i].data, NULL, args[i].size) != 0) {
            goto error_out;
        }
        length += args[i].size + width;
    }
   
    // Send END byte + padding to multiple of 4.
    uint8_t cmdbuf_end[4] = {CMD_END, 0xFF, 0xFF, 0xFF};
    if (nina_bsp_spi_transfer(cmdbuf_end, NULL, 1 + (length % 4)) != 0) {
        goto error_out;
    }

    // All good
    ret = 0;

error_out:
    debug_printf("\n");
    nina_bsp_spi_slave_deselect();
    return ret;
}

static int nina_read_response(uint32_t cmd, uint32_t nvals, uint32_t width, nina_vals_t *vals)
{
    int ret = -1;

    debug_printf("nina_read_response(cmd 0x%x nvals %d width %d): ", cmd, nvals, width);

    // Read reply
    if (nina_bsp_spi_slave_select(NINA_SSELECT_TIMEOUT) != 0) {
        return -1;
    }

    // Wait for CMD_START
    if (nina_wait_for_cmd(CMD_START, NINA_RESP_TIMEOUT) != 0) {
        goto error_out;
    }

    // Should return CMD + REPLY flag.
    if (nina_bsp_spi_read_byte() != (cmd | CMD_REPLY)) {
        goto error_out;
    }

    // Sanity check the number of returned values.
    // NOTE: This is to handle the special case for the scan command.
    uint32_t rvals = nina_bsp_spi_read_byte();
    if (nvals > rvals) {
        nvals = rvals;
    }

    // Read return value(s).
    for (uint32_t i=0; i<nvals; i++) {
        // Read return value size.
        uint16_t bytes = nina_bsp_spi_read_byte();
        if (width == ARG_16BITS) {
            bytes = (bytes << 8) | nina_bsp_spi_read_byte();
        }

        // Check the val fits the buffer.
        if (*(vals[i].size) < bytes) {
            goto error_out;
        }

        // Read the returned value.
        if (nina_bsp_spi_transfer(NULL, vals[i].data, bytes) != 0) {
            goto error_out;
        }

        // Set the size.
        *(vals[i].size) = bytes;
    }

    if (nina_bsp_spi_read_byte() != CMD_END) {
        goto error_out;
    }

    // All good
    ret = 0;

error_out:
    debug_printf("\n");
    nina_bsp_spi_slave_deselect();
    return ret;
}

static int nina_send_command_read_ack(uint32_t cmd, uint32_t nargs, uint32_t width, nina_args_t *args)
{
    uint16_t size = 1;
    uint8_t  rval = SPI_ERR;
    if (nina_send_command(cmd, nargs, width, args) != 0 ||
            nina_read_response(cmd, 1, ARG_8BITS, NINA_VALS({&size, &rval})) != 0) {
        return -1;
    }
    return rval;
}

static int nina_send_command_read_vals(uint32_t cmd, uint32_t nargs,
        uint32_t argsw, nina_args_t *args, uint32_t nvals, uint32_t valsw, nina_vals_t *vals)
{

    if (nina_send_command(cmd, nargs, argsw, args) != 0 ||
        nina_read_response(cmd, nvals, valsw, vals) != 0) {
        return -1;
    }
    return 0;
}

int nina_init()
{
    // Initialize the BSP.
    nina_bsp_init();
    return 0; 
}

int nina_deinit()
{
    return nina_bsp_deinit();
}

static int nina_connection_status()
{
    return nina_send_command_read_ack(GET_CONN_STATUS_CMD, 0, ARG_8BITS, NULL);
}

static int nina_socket_status(uint8_t fd)
{
    return nina_send_command_read_ack(GET_CLIENT_STATE_TCP_CMD,
            1, ARG_8BITS, NINA_ARGS(ARG_BYTE(fd)));
}

static int nina_server_socket_status(uint8_t fd)
{
    return nina_send_command_read_ack(GET_STATE_TCP_CMD,
            1, ARG_8BITS, NINA_ARGS(ARG_BYTE(fd)));
}

int nina_connect(const char *ssid, uint8_t security, const char *key, uint16_t channel)
{
    uint8_t status = WL_CONNECT_FAILED;

    if (key == NULL && security != NINA_SEC_OPEN) {
        return -1;
    }

    switch (security) {
        case NINA_SEC_OPEN:
            if (nina_send_command_read_ack(SET_NET_CMD,
                        1, ARG_8BITS, NINA_ARGS(ARG_STR(ssid))) != SPI_ACK) {
                return -1;
            }
            break;
        case NINA_SEC_WEP:
            if (nina_send_command_read_ack(SET_PASSPHRASE_CMD,
                        2, ARG_8BITS, NINA_ARGS(ARG_STR(ssid), ARG_STR(key))) != SPI_ACK) {
                return -1;
            }
            break;
        case NINA_SEC_WPA_PSK:
            if (nina_send_command_read_ack(SET_KEY_CMD,
                        3, ARG_8BITS, NINA_ARGS(ARG_STR(ssid), ARG_BYTE(0), ARG_STR(key))) != SPI_ACK) {
                return -1;
            }
            break;
        default:
            return -1;
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); ; mp_hal_delay_ms(10)) {
        status = nina_connection_status();
        if ((status != WL_IDLE_STATUS) && (status != WL_NO_SSID_AVAIL) && (status != WL_SCAN_COMPLETED)) {
            break;
        }

        if ((mp_hal_ticks_ms() - start) >= NINA_CONNECT_TIMEOUT) {
            break;
        }
    }

    return (status == WL_CONNECTED) ? 0 : -1;
}

int nina_start_ap(const char *ssid, uint8_t security, const char *key, uint16_t channel)
{
    uint8_t status = WL_AP_FAILED;

    if ((key == NULL && security != NINA_SEC_OPEN) ||
            (security != NINA_SEC_OPEN && security != NINA_SEC_WEP)) {
        return -1;
    }

    switch (security) {
        case NINA_SEC_OPEN:
            if (nina_send_command_read_ack(SET_AP_NET_CMD,
                        2, ARG_8BITS, NINA_ARGS(ARG_STR(ssid), ARG_BYTE(channel))) != SPI_ACK) {
                return -1;
            }
            break;
        case NINA_SEC_WEP:
            if (nina_send_command_read_ack(SET_AP_PASSPHRASE_CMD,
                        3, ARG_8BITS, NINA_ARGS(ARG_STR(ssid), ARG_STR(key), ARG_BYTE(channel))) != SPI_ACK) {
                return -1;
            }
            break;
        default:
            return -1;
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); ; mp_hal_delay_ms(10)) {
        status = nina_connection_status();
        if ((status != WL_IDLE_STATUS) && (status != WL_NO_SSID_AVAIL) && (status != WL_SCAN_COMPLETED)) {
            break;
        }

        if ((mp_hal_ticks_ms() - start) >= NINA_CONNECT_TIMEOUT) {
            break;
        }
    }

    return (status == WL_AP_LISTENING) ? 0 : -1;
}

int nina_disconnect()
{
    if (nina_send_command_read_ack(DISCONNECT_CMD,
                1, ARG_8BITS, NINA_ARGS(ARG_BYTE(0xFF))) != SPI_ACK) {
        return -1;
    }
    return 0;
}

int nina_isconnected()
{
    int status = nina_connection_status();
    if (status == -1) {
        return -1;
    }
    return (status == WL_CONNECTED);
}

int nina_connected_sta(uint32_t *sta_ip)
{
    return -1;
}

int nina_wait_for_sta(uint32_t *sta_ip, uint32_t timeout)
{
    return NINA_ERROR_TIMEOUT;
}

int nina_ifconfig(nina_ifconfig_t *ifconfig, bool set)
{
    uint16_t ip_len  = NINA_IPV4_ADDR_LEN;
    uint16_t sub_len = NINA_IPV4_ADDR_LEN;
    uint16_t gw_len  = NINA_IPV4_ADDR_LEN;
    uint16_t dns_len = NINA_IPV4_ADDR_LEN;

    if (set) {
        if (nina_send_command_read_ack(SET_IP_CONFIG_CMD,
                    4, ARG_8BITS,
                    NINA_ARGS(
                        ARG_BYTE(3), // Valid number of args.
                        {ip_len,  ifconfig->ip_addr},
                        {gw_len,  ifconfig->gateway_addr},
                        {sub_len, ifconfig->subnet_addr})) != SPI_ACK) {
            return -1;
        }

        if (nina_send_command_read_ack(SET_DNS_CONFIG_CMD,
                    3, ARG_8BITS,
                    NINA_ARGS(
                        ARG_BYTE(1), // Valid number of args.
                        {dns_len, ifconfig->dns_addr}, 
                        {dns_len, ifconfig->dns_addr})) != SPI_ACK) {
            return -1;
        }

    } else {
        if (nina_send_command_read_vals(GET_IPADDR_CMD,
                    1, ARG_8BITS, NINA_ARGS(ARG_BYTE(0xFF)),
                    3, ARG_8BITS,
                    NINA_VALS(
                        {&ip_len,  ifconfig->ip_addr},
                        {&sub_len, ifconfig->subnet_addr},
                        {&gw_len,  ifconfig->gateway_addr})) != 0) {
            return -1;
        }
        // No command to get DNS ?
        memcpy(ifconfig->dns_addr, ifconfig->gateway_addr, NINA_IPV4_ADDR_LEN);
    }
    return 0;
}

int nina_netinfo(nina_netinfo_t *netinfo)
{
    uint16_t rssi_len  = 4;
    uint16_t sec_len   = 1;
    uint16_t ssid_len  = NINA_MAX_SSID_LEN;
    uint16_t bssid_len = NINA_MAC_ADDR_LEN;

    if (nina_send_command_read_vals(GET_CURR_RSSI_CMD,
                1, ARG_8BITS, NINA_ARGS(ARG_BYTE(0xFF)),
                1, ARG_8BITS, NINA_VALS({&rssi_len, &netinfo->rssi})) != 0) {
        return -1;
    }

    if (nina_send_command_read_vals(GET_CURR_ENCT_CMD,
                1, ARG_8BITS, NINA_ARGS(ARG_BYTE(0xFF)),
                1, ARG_8BITS, NINA_VALS({&sec_len, &netinfo->security})) != 0) {
        return -1;
    }

    if (nina_send_command_read_vals(GET_CURR_SSID_CMD,
                1, ARG_8BITS, NINA_ARGS(ARG_BYTE(0xFF)),
                1, ARG_8BITS, NINA_VALS({&ssid_len, &netinfo->ssid})) != 0) {
        return -1;
    }

    if (nina_send_command_read_vals(GET_CURR_BSSID_CMD,
                1, ARG_8BITS, NINA_ARGS(ARG_BYTE(0xFF)),
                1, ARG_8BITS, NINA_VALS({&bssid_len, &netinfo->bssid})) != 0) {
        return -1;
    }
    
    return 0;
}

int nina_scan(nina_scan_callback_t scan_callback, void *arg, uint32_t timeout)
{
    uint16_t sizes[NINA_MAX_NETWORK_LIST];
    char  ssids[NINA_MAX_NETWORK_LIST][NINA_MAX_SSID_LEN];
    nina_vals_t vals[NINA_MAX_NETWORK_LIST];

    // Initialize the values list.
    for (int i=0; i< NINA_MAX_NETWORK_LIST; i++) {
        sizes[i] = NINA_MAX_SSID_LEN - 1;
        memset(ssids[i], 0, NINA_MAX_SSID_LEN);
        vals[i].size = &sizes[i];
        vals[i].data =  ssids[i];
    }

    if (nina_send_command_read_ack(START_SCAN_NETWORKS,
                0, ARG_8BITS, NULL) != SPI_ACK) {
        return -1;
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); ;) {
        if (nina_send_command_read_vals(SCAN_NETWORKS,
                    0, ARG_8BITS, NULL,
                    NINA_MAX_NETWORK_LIST, ARG_8BITS, vals) != 0) {
            return -1;
        }

        if (ssids[0][0] != 0) {
            // Found at least 1 network.
            break;
        }

        if (timeout && (mp_hal_ticks_ms() - start) >= timeout) {
            // Timeout, no networks.
            return NINA_ERROR_TIMEOUT;
        }

        mp_hal_delay_ms(100);
    }

    for (int i=0; i<NINA_MAX_NETWORK_LIST; i++) {
        uint16_t rssi_len  = 4;
        uint16_t sec_len   = 1;
        uint16_t chan_len  = 1;
        uint16_t bssid_len = NINA_MAC_ADDR_LEN;
        nina_scan_result_t scan_result;

        if (ssids[i][0] == 0) {
            break;
        }

        // Set AP SSID
        strncpy(scan_result.ssid, ssids[i], NINA_MAX_SSID_LEN);

        // Read AP RSSI
        if (nina_send_command_read_vals(GET_IDX_RSSI_CMD,
                    1, ARG_8BITS, NINA_ARGS(ARG_BYTE(i)),
                    1, ARG_8BITS, NINA_VALS({&rssi_len, &scan_result.rssi})) != 0) {
            return -1;
        }

        // Read AP encryption type
        if (nina_send_command_read_vals(GET_IDX_ENCT_CMD,
                    1, ARG_8BITS, NINA_ARGS(ARG_BYTE(i)),
                    1, ARG_8BITS, NINA_VALS({&sec_len, &scan_result.security})) != 0) {
            return -1;
        }

        // Read AP channel
        if (nina_send_command_read_vals(GET_IDX_CHANNEL_CMD,
                    1, ARG_8BITS, NINA_ARGS(ARG_BYTE(i)),
                    1, ARG_8BITS, NINA_VALS({&chan_len, &scan_result.channel})) != 0) {
            return -1;
        }

        // Read AP bssid
        if (nina_send_command_read_vals(GET_IDX_BSSID_CMD,
                    1, ARG_8BITS, NINA_ARGS(ARG_BYTE(i)),
                    1, ARG_8BITS, NINA_VALS({&bssid_len, scan_result.bssid})) != 0) {
            return -1;
        }

        scan_callback(&scan_result, arg);
    }

    return 0;
}

int nina_get_rssi()
{
    uint16_t size = 4;
    uint16_t rssi = 0;
    if (nina_send_command_read_vals(GET_CURR_RSSI_CMD,
                1, ARG_8BITS, NINA_ARGS(ARG_BYTE(0xFF)),
                1, ARG_8BITS, NINA_VALS({&size, &rssi})) != 0) {
        return -1;
    }

    return rssi;
}

int nina_fw_version(uint8_t *fw_ver)
{
    uint16_t size = NINA_FW_VER_LEN;
    if (nina_send_command_read_vals(GET_FW_VERSION_CMD,
                0, ARG_8BITS, NULL,
                1, ARG_8BITS, NINA_VALS({&size, fw_ver})) != 0) {
        return -1;
    }
    return 0;
}

int nina_set_hostname(const char *hostname)
{
    if (nina_send_command_read_ack(SET_HOSTNAME_CMD,
                1, ARG_8BITS, NINA_ARGS(ARG_STR(hostname))) != SPI_ACK) {
        return -1;
    }
    return 0;
}

int nina_gethostbyname(const char *name, uint8_t *out_ip)
{
    uint16_t size = 4;

    if (nina_send_command_read_ack(REQ_HOST_BY_NAME_CMD,
                1, ARG_8BITS, NINA_ARGS(ARG_STR(name))) != SPI_ACK) {
        return -1;
    }

    if (nina_send_command_read_vals(GET_HOST_BY_NAME_CMD,
                0, ARG_8BITS, NULL,
                1, ARG_8BITS, NINA_VALS({&size, out_ip})) != 0) {
        return -1;
    }
    return 0;
}

int nina_socket_socket(uint8_t type)
{
    uint16_t size = 1;
    uint8_t  sock = 0;

    if (nina_send_command_read_vals(GET_SOCKET_CMD,
                0, ARG_8BITS, NULL,
                1, ARG_8BITS, NINA_VALS({&size, &sock})) != 0) {
        return -1;
    }
    return sock;
}

int nina_socket_close(int fd)
{
    if (fd > 0 && fd < 255) {
        if (nina_send_command_read_ack(STOP_CLIENT_TCP_CMD,
                    1, ARG_8BITS, NINA_ARGS(ARG_BYTE(fd))) != SPI_ACK) {
            return -1;
        }
        for (mp_uint_t start = mp_hal_ticks_ms(); ;mp_hal_delay_ms(10)) {
            if (nina_socket_status(fd) == SOCKET_STATE_CLOSED) {
                break;
            }
            if ((mp_hal_ticks_ms() - start) >= 5000) {
                return NINA_ERROR_TIMEOUT;
            }
        }
    }
    return 0;
}

int nina_socket_bind(int fd, uint8_t *ip, uint16_t port, int type)
{
    if (nina_send_command_read_ack(START_SERVER_TCP_CMD,
                3, ARG_8BITS,
                NINA_ARGS(
                    ARG_SHORT(__REVSH(port)),
                    ARG_BYTE(fd),
                    ARG_BYTE(type))) != SPI_ACK) {
        return -1;
    }

    // Only TCP sockets' states should be checked.
    if (type == NINA_SOCKET_TYPE_TCP &&
            nina_server_socket_status(fd) != SOCKET_STATE_LISTEN) {
        return -1;
    }
    return 0;
}

int nina_socket_listen(int fd, uint32_t backlog)
{
    return 0; // No listen ?
}

int nina_socket_accept(int fd, uint8_t *ip, uint16_t *port, int *fd_out, uint32_t timeout)
{
    uint16_t size = 2;
    uint16_t sock = NO_SOCKET_AVAIL;

    if (nina_server_socket_status(fd) != SOCKET_STATE_LISTEN) {
        return -1;
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); sock == 0 || sock == NO_SOCKET_AVAIL; mp_hal_delay_ms(10)) {
        if (nina_send_command_read_vals(AVAIL_DATA_TCP_CMD,
                    1, ARG_8BITS, NINA_ARGS(ARG_BYTE(fd)),
                    1, ARG_8BITS, NINA_VALS({&size, &sock})) != 0) {
            return -1;
        }

        if (timeout && (mp_hal_ticks_ms() - start) >= timeout) {
            return NINA_ERROR_TIMEOUT;
        }
    }

    uint16_t port_len = 2;
    uint16_t ip_len = NINA_IPV4_ADDR_LEN;
    if (nina_send_command_read_vals(GET_REMOTE_DATA_CMD,
                1, ARG_8BITS, NINA_ARGS(ARG_BYTE(sock)),
                2, ARG_8BITS, NINA_VALS({&ip_len, ip}, {&port_len, port})) != 0) {
        return -1;
    }
    *fd_out = sock;
    *port = __REVSH(*port);
    return 0;
}

int nina_socket_connect(int fd, uint8_t *ip, uint16_t port, uint32_t timeout)
{
    if (nina_send_command_read_ack(START_CLIENT_TCP_CMD,
                4, ARG_8BITS,
                NINA_ARGS(
                    ARG_WORD((*(uint32_t*)ip)),
                    ARG_SHORT(__REVSH(port)),
                    ARG_BYTE(fd),
                    ARG_BYTE(NINA_SOCKET_TYPE_TCP))) != SPI_ACK) {
        return -1;
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); ; mp_hal_delay_ms(10)) {
        int state = nina_socket_status(fd);
        if (state == -1) {
            return -1;
        }

        if (state == SOCKET_STATE_ESTABLISHED) {
            break;
        }

        if (timeout && (mp_hal_ticks_ms() - start) >= timeout) {
            return NINA_ERROR_TIMEOUT;
        }
    }

    return 0;
}

int nina_socket_send(int fd, const uint8_t *buf, uint32_t len, uint32_t timeout)
{
    uint16_t size = 2;
    uint16_t bytes = 0;

    if (nina_socket_status(fd) != SOCKET_STATE_ESTABLISHED) {
        return -1;
    }

    if (nina_send_command_read_vals(SEND_DATA_TCP_CMD,
                2, ARG_16BITS, NINA_ARGS(ARG_BYTE(fd), {len, buf}),
                1, ARG_8BITS, NINA_VALS({&size, &bytes})) != 0 || bytes <= 0) {
        return -1;
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); ;) {
        int resp = nina_send_command_read_ack(DATA_SENT_TCP_CMD,
                1, ARG_8BITS, NINA_ARGS(ARG_BYTE(fd)));

        if (resp == -1) {
            return -1;
        }

        if (resp == SPI_ACK) {
            break;
        }

        if (timeout && (mp_hal_ticks_ms() - start) >= timeout) {
            return NINA_ERROR_TIMEOUT;
        }
        mp_hal_delay_ms(1);
    }

    return bytes;
}

int nina_socket_recv(int fd, uint8_t *buf, uint32_t len, uint32_t timeout)
{
    uint16_t bytes = 0;

    if (nina_socket_status(fd) != SOCKET_STATE_ESTABLISHED) {
        return -1;
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); bytes == 0; mp_hal_delay_ms(1)) {
        bytes = len;
        if (nina_send_command_read_vals(GET_DATABUF_TCP_CMD,
                    2, ARG_16BITS, NINA_ARGS(ARG_BYTE(fd), ARG_SHORT(bytes)),
                    1, ARG_16BITS, NINA_VALS({&bytes, buf})) != 0) {
            return -1;
        }

        if (timeout && (mp_hal_ticks_ms() - start) >= timeout) {
            return NINA_ERROR_TIMEOUT;
        }
    }
    return bytes;
}

// Check from the upper layer if the socket is bound, if not then auto-bind it first.
int nina_socket_sendto(int fd, const uint8_t *buf, uint32_t len, uint8_t *ip, uint16_t port, uint32_t timeout)
{
    // TODO do we need to split the packet somewhere?
    if (nina_send_command_read_ack(START_CLIENT_TCP_CMD,
                4, ARG_8BITS,
                NINA_ARGS(
                    ARG_WORD((*(uint32_t*)ip)),
                    ARG_SHORT(__REVSH(port)),
                    ARG_BYTE(fd),
                    ARG_BYTE(NINA_SOCKET_TYPE_UDP))) != SPI_ACK) {
        return -1;
    }

    // Buffer length and socket number are passed as 16bits.
    if (nina_send_command_read_ack(INSERT_DATABUF_CMD,
                2, ARG_16BITS, NINA_ARGS(ARG_BYTE(fd), {len, buf})) != SPI_ACK) {
        return -1;
    }

    if (nina_send_command_read_ack(SEND_DATA_UDP_CMD,
                1, ARG_8BITS, NINA_ARGS(ARG_BYTE(fd))) != SPI_ACK) {
        return -1;
    }

    return 0;
}

// Check from the upper layer if the socket is bound, if not then auto-bind it first.
int nina_socket_recvfrom(int fd, uint8_t *buf, uint32_t len, uint8_t *ip, uint16_t *port, uint32_t timeout)
{
    uint16_t bytes = 0;
    uint16_t port_len = 2;
    uint16_t ip_len = NINA_IPV4_ADDR_LEN;

    for (mp_uint_t start = mp_hal_ticks_ms(); bytes == 0; mp_hal_delay_ms(1)) {
        bytes = len;
        if (nina_send_command_read_vals(GET_DATABUF_TCP_CMD,
                    2, ARG_16BITS, NINA_ARGS(ARG_BYTE(fd), ARG_SHORT(bytes)),
                    1, ARG_16BITS, NINA_VALS({&bytes, buf})) != 0) {
            return -1;
        }

        if (timeout && (mp_hal_ticks_ms() - start) >= timeout) {
            return NINA_ERROR_TIMEOUT;
        }
    }
    if (nina_send_command_read_vals(GET_REMOTE_DATA_CMD,
                1, ARG_8BITS, NINA_ARGS(ARG_BYTE(fd)),
                2, ARG_8BITS, NINA_VALS({&ip_len, ip}, {&port_len, port})) != 0) {
        return -1;
    }

    return bytes;
}

int nina_socket_setsockopt(int fd, uint32_t level, uint32_t opt, const void *optval, uint32_t optlen)
{
    return -1;
}
#endif //MICROPY_PY_NINAW10

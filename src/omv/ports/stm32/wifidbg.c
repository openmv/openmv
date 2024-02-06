/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * WiFi debugger.
 */
#include "omv_boardconfig.h"
#if OMV_WIFIDBG_ENABLE && MICROPY_PY_WINC1500

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include "py/mphal.h"
#include "pendsv.h"
#include "systick.h"
#include "usb.h"

#include "winc.h"
#include "socket/include/socket.h"
#include "driver/include/m2m_wifi.h"
#include "usbdbg.h"
#include "sensor.h"
#include "framebuffer.h"
#include "wifidbg.h"

#include STM32_HAL_H

#ifndef WIFIDBG_MIN
#define WIFIDBG_MIN(x, y)            ((x) < (y) ? (x) : (y))
#endif

#define WIFIDBG_BCAST_ADDR           ((uint8_t [4]) {255, 255, 255, 255})
#define WIFIDBG_BCAST_PORT           (0xABD1)
#define WIFIDBG_SERVER_ADDR          ((uint8_t [4]) {192, 168, 1, 1})
#define WIFIDBG_SERVER_PORT          (9000)
#define WIFIDBG_BUFFER_SIZE          (SOCKET_BUFFER_MAX_LENGTH)

#define WIFIDBG_BCAST_STRING         "%d.%d.%d.%d:%d:%s"
#define WIFIDBG_BCAST_STRING_SIZE    4 + 4 + 4 + 4 + 6 + WINC_MAX_BOARD_NAME_LEN + 1
#define WIFIDBG_BCAST_INTERVAL_MS    (1000)
#define WIFIDBG_POLL_INTERVAL_MS     (10)

#define WIFIDBG_SOCKET_TIMEOUT(x)    (x == -ETIMEDOUT || x == SOCK_ERR_TIMEOUT)

typedef struct wifidbg {
    int client_fd;
    int server_fd;
    int bcast_fd;
    uint32_t last_bcast;
    uint32_t last_dispatch;
    uint8_t ipaddr[WINC_IPV4_ADDR_LEN];
    char bcast_packet[WIFIDBG_BCAST_STRING_SIZE];
    winc_socket_buf_t sockbuf;
} wifidbg_t;

static wifidbg_t wifidbg;

void wifidbg_close_sockets(wifidbg_t *wifidbg) {
    winc_socket_close(wifidbg->client_fd);
    winc_socket_close(wifidbg->server_fd);
    winc_socket_close(wifidbg->bcast_fd);
    wifidbg->client_fd = -1;
    wifidbg->server_fd = -1;
    wifidbg->bcast_fd = -1;
}

int wifidbg_broadcast(wifidbg_t *wifidbg) {
    if ((systick_current_millis() - wifidbg->last_bcast) > WIFIDBG_BCAST_INTERVAL_MS) {
        MAKE_SOCKADDR(bcast_sockaddr, WIFIDBG_BCAST_ADDR, WIFIDBG_BCAST_PORT);
        if (winc_socket_sendto(wifidbg->bcast_fd, (uint8_t *) wifidbg->bcast_packet,
                               strlen(wifidbg->bcast_packet) + 1, &bcast_sockaddr, 10) < 0) {
            return -1;
        }
        wifidbg->last_bcast = systick_current_millis();
    }
    return 0;
}

void wifidbg_pendsv_callback(void) {
    int ret = 0;
    uint32_t request = 0;
    uint8_t buf[WIFIDBG_BUFFER_SIZE];

    // Disable systick dispatch
    wifidbg_set_irq_enabled(false);

    if (wifidbg.bcast_fd < 0) {
        // Create broadcast socket.
        MAKE_SOCKADDR(bcast_sockaddr, WIFIDBG_BCAST_ADDR, WIFIDBG_BCAST_PORT);

        if ((wifidbg.bcast_fd = winc_socket_socket(SOCK_DGRAM)) < 0) {
            goto exit_dispatch_error;
        }

        if ((ret = winc_socket_bind(wifidbg.bcast_fd, &bcast_sockaddr)) < 0) {
            goto exit_dispatch_error;
        }
    }

    if (wifidbg.server_fd < 0) {
        // Create server socket
        MAKE_SOCKADDR(server_sockaddr, wifidbg.ipaddr, WIFIDBG_SERVER_PORT);

        if ((wifidbg.server_fd = winc_socket_socket(SOCK_STREAM)) < 0) {
            goto exit_dispatch_error;
        }

        if ((ret = winc_socket_bind(wifidbg.server_fd, &server_sockaddr)) < 0) {
            goto exit_dispatch_error;
        }

        if ((ret = winc_socket_listen(wifidbg.server_fd, 1)) < 0) {
            goto exit_dispatch_error;
        }
    }

    if (wifidbg.client_fd < 0) {
        wifidbg.sockbuf.idx = 0;
        wifidbg.sockbuf.size = 0;
        sockaddr client_sockaddr;
        // Try to connect to a client
        ret = winc_socket_accept(wifidbg.server_fd, &client_sockaddr, &wifidbg.client_fd, 10);
        if (WIFIDBG_SOCKET_TIMEOUT(ret)) {
            // Timeout, broadcast another message to the IDE.
            wifidbg.client_fd = -1;
            ret = wifidbg_broadcast(&wifidbg);
        }
        if (ret < 0) {
            // Error on server socket, close sockets and exit.
            goto exit_dispatch_error;
        }
        goto exit_dispatch;
    }

    uint8_t cmdbuf[6];
    // We have a connected client
    ret = winc_socket_recv(wifidbg.client_fd, cmdbuf, 6, &wifidbg.sockbuf, 5);
    if (WIFIDBG_SOCKET_TIMEOUT(ret)) {
        goto exit_dispatch;
    } else if (ret < 0 || ret != 6 || cmdbuf[0] != 0x30) {
        goto exit_dispatch_error;
    }

    // Process the request command.
    request = cmdbuf[1];
    uint32_t xfer_length = *((uint32_t *) (cmdbuf + 2));
    usbdbg_control(NULL, request, xfer_length);

    while (xfer_length) {
        if (request & 0x80) {
            // Device-to-host data phase
            int bytes = WIFIDBG_MIN(xfer_length, WIFIDBG_BUFFER_SIZE);
            xfer_length -= bytes;
            usbdbg_data_in(buf, bytes);
            if ((ret = winc_socket_send(wifidbg.client_fd, buf, bytes, 500)) < 0) {
                goto exit_dispatch_error;
            }
        } else {
            // Host-to-device data phase
            int bytes = WIFIDBG_MIN(xfer_length, WIFIDBG_BUFFER_SIZE);
            if ((ret = winc_socket_recv(wifidbg.client_fd, buf, bytes, &wifidbg.sockbuf, 500)) < 0) {
                goto exit_dispatch_error;
            }
            xfer_length -= ret;
            usbdbg_data_out(buf, ret);
        }
    }

    goto exit_dispatch;
exit_dispatch_error:
    wifidbg_close_sockets(&wifidbg);
exit_dispatch:
    if (usbdbg_get_irq_enabled()) {
        // Re-enable Systick dispatch if IDE interrupts are still enabled.
        wifidbg_set_irq_enabled(true);
    }
    return;
}

void wifidbg_systick_callback(uint32_t ticks_ms) {
    if (usb_cdc_debug_mode_enabled() == false &&
        (ticks_ms - wifidbg.last_dispatch) > WIFIDBG_POLL_INTERVAL_MS) {
        pendsv_schedule_dispatch(PENDSV_DISPATCH_WINC, wifidbg_pendsv_callback);
        wifidbg.last_dispatch = systick_current_millis();
    }
}

int wifidbg_init(wifidbg_config_t *config) {
    memset(&wifidbg, 0, sizeof(wifidbg_t));

    wifidbg.client_fd = -1;
    wifidbg.server_fd = -1;
    wifidbg.bcast_fd = -1;

    if (!config->mode) {
        // STA Mode

        // Initialize WiFi in STA mode.
        if (winc_init(WINC_MODE_STA) != 0) {
            return -1;
        }

        // Connect to network.
        if (winc_connect(config->client_ssid,
                         config->client_security,
                         config->client_key,
                         config->client_channel) != 0) {
            return -2;
        }

        winc_ifconfig_t ifconfig;

        if (winc_ifconfig(&ifconfig, false) < 0) {
            return -3;
        }

        memcpy(wifidbg.ipaddr, ifconfig.ip_addr, WINC_IPV4_ADDR_LEN);

    } else {
        // AP Mode

        // Initialize WiFi in AP mode.
        if (winc_init(WINC_MODE_AP) != 0) {
            return -1;
        }

        // Start WiFi in AP mode.
        if (winc_start_ap(config->access_point_ssid,
                          config->access_point_security,
                          config->access_point_key,
                          config->access_point_channel) != 0) {
            return -2;
        }

        memcpy(wifidbg.ipaddr, WIFIDBG_SERVER_ADDR, WINC_IPV4_ADDR_LEN);
    }

    snprintf(wifidbg.bcast_packet, WIFIDBG_BCAST_STRING_SIZE, WIFIDBG_BCAST_STRING,
             wifidbg.ipaddr[0], wifidbg.ipaddr[1], wifidbg.ipaddr[2], wifidbg.ipaddr[3],
             WIFIDBG_SERVER_PORT, config->board_name);

    return 0;
}

void wifidbg_set_irq_enabled(bool enable) {
    if (enable) {
        // Re-enable Systick dispatch.
        systick_enable_dispatch(SYSTICK_DISPATCH_WINC, wifidbg_systick_callback);
    } else {
        systick_disable_dispatch(SYSTICK_DISPATCH_WINC);
    }
}
#endif // OMV_WIFIDBG_ENABLE && MICROPY_PY_WINC1500

// Old timer dispatch, will be removed.
void wifidbg_dispatch() {
}

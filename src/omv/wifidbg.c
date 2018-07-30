/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2018 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include "winc.h"
#include "socket/include/socket.h"
#include "driver/include/m2m_wifi.h"
#include "mp.h"
#include "irq.h"
#include "lexer.h"
#include "parse.h"
#include "compile.h"
#include "runtime.h"
#include "stackctrl.h"
#include "usbdbg.h"
#include "sensor.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"
#include "lib/utils/pyexec.h"
#include "wifidbg.h"
#include "led.h"

#include STM32_HAL_H

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#define SERVER_ADDR     ((uint8_t [5]){192, 168, 1, 1})
#define SERVER_PORT     (9000)
#define BUFFER_SIZE     (512)

#define close_all_sockets()             \
    do {                                \
        winc_socket_close(client_fd);   \
        winc_socket_close(server_fd);   \
        client_fd = -1;                 \
        server_fd = -1;                 \
    } while (0)

#define close_server_socket()           \
    do {                                \
        winc_socket_close(server_fd);   \
        server_fd = -1;                 \
    } while (0)

#define TIMEDOUT(x) (x == -ETIMEDOUT || x == SOCK_ERR_TIMEOUT)

static int client_fd = -1;
static int server_fd = -1;

int wifidbg_init(wifidbg_config_t *config)
{
    client_fd = -1;
    server_fd = -1;

    // Initialize WiFi in AP mode.
    if (winc_init(WINC_MODE_AP) != 0) {
        return -1;
    }

    // Start WiFi in AP mode.
    if (winc_start_ap(config->ssid, config->security, config->key, config->channel) != 0) {
        return -2;
    }

    return 0;
}

void wifidbg_dispatch()
{
    int ret;
    uint8_t buf[BUFFER_SIZE];
    sockaddr client_sockaddr;

    if (server_fd < 0) {
        // Create server socket
        MAKE_SOCKADDR(server_sockaddr, SERVER_ADDR, SERVER_PORT)

        if ((server_fd = winc_socket_socket(SOCK_STREAM)) < 0) {
            return;
        }

        if ((ret = winc_socket_bind(server_fd, &server_sockaddr)) < 0) {
            close_server_socket();
            return;
        }

        if ((ret = winc_socket_listen(server_fd, 1)) < 0) {
            close_server_socket();
            return;
        }
    }

    if (client_fd < 0) {
        // Call accept.
        if ((ret = winc_socket_accept(server_fd,
                        &client_sockaddr, &client_fd, 10)) < 0) {
            if (TIMEDOUT(ret)) {
                client_fd = -1;
            } else {
                close_server_socket();
            }
            return;
        }

    }

    if ((ret = winc_socket_recv(client_fd, buf, 8, 10)) < 0) {
        if (TIMEDOUT(ret)) {
            return;
        } else {
            close_server_socket();
            return;
        }
    }

    uint8_t request = buf[1];
    uint32_t xfer_length = *((uint32_t*)(buf+2));
    usbdbg_control(buf+6, request, xfer_length);

    while (xfer_length) {
        if (request & 0x80) {
            // Device-to-host data phase
            int bytes = MIN(xfer_length, BUFFER_SIZE);
            xfer_length -= bytes;
            usbdbg_data_in(buf, bytes);
            if ((ret = winc_socket_send(client_fd, buf, bytes, 500)) < 0) {
                close_all_sockets();
                return;
            }
        } else {
            // Host-to-device data phase
            int bytes = MIN(xfer_length, BUFFER_SIZE);
            if ((ret = winc_socket_recv(client_fd, buf, bytes, 500)) < 0) {
                close_all_sockets();
                return;
            }
            xfer_length -= ret;
            usbdbg_data_out(buf, ret);
        }
    }
}

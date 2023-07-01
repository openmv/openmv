/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * VOSPI driver.
 */
#include "omv_boardconfig.h"
#if OMV_ENABLE_VOSPI || OMV_ENABLE_LEPTON

#include <stdint.h>
#include "py/mphal.h"

#include "vospi.h"
#include "crc16.h"
#include "common.h"
#include "omv_spi.h"

#define VOSPI_LINE_PIXELS         (80)
#define VOSPI_NUMBER_PACKETS      (60)
#define VOSPI_SPECIAL_PACKET      (20)
#define VOSPI_LINE_SIZE           (80 * 2)
#define VOSPI_HEADER_SIZE         (4)
#define VOSPI_PACKET_SIZE         (VOSPI_HEADER_SIZE + VOSPI_LINE_SIZE)
#define VOSPI_FIRST_PACKET        (0)
#define VOSPI_FIRST_SEGMENT       (1)
#ifndef VOSPI_PACKET_ALIGNMENT
#define VOSPI_PACKET_ALIGNMENT    (4)
#endif
#define VOSPI_HEADER_SEG(buf)     (((buf[0] >> 4) & 0x7))
#define VOSPI_HEADER_PID(buf)     (((buf[0] << 8) | (buf[1] << 0)) & 0x0FFF)
#define VOSPI_HEADER_CRC(buf)     (((buf[2] << 8) | (buf[3] << 0)))

typedef enum {
    VOSPI_FLAGS_RESET  = (1 << 0),
    VOSPI_FLAGS_RESYNC = (1 << 1),
} vospi_flags_t;

typedef struct _vospi_state {
    volatile uint32_t pid;
    volatile uint32_t sid;
    uint8_t *buffer;
    uint32_t n_packets;
    omv_spi_t spi_bus;
    volatile uint32_t flags;
} vospi_state_t;

static vospi_state_t vospi;
static uint8_t OMV_ATTR_SECTION(
    OMV_ATTR_ALIGNED(vospi_packet[VOSPI_PACKET_SIZE], VOSPI_PACKET_ALIGNMENT), ".dma_buffer"
    );

static void vospi_callback(omv_spi_t *spi, void *data);
#if (OMV_ENABLE_VOSPI_CRC)
static uint16_t vospi_calc_crc(uint8_t *buf) {
    buf[0] &= 0x0F;
    buf[1] &= 0xFF;
    buf[2] = 0;
    buf[3] = 0;
    return CalcCRC16Bytes(VOSPI_PACKET_SIZE, (char *) buf);
}
#endif

static void vospi_do_resync() {
    omv_spi_transfer_t spi_xfer = {
        .txbuf = NULL,
        .rxbuf = vospi_packet,
        .size = VOSPI_PACKET_SIZE,
        .timeout = 0,
        .flags = OMV_SPI_XFER_DMA | OMV_SPI_XFER_CIRCULAR,
        .userdata = NULL,
        .callback = vospi_callback,
    };

    omv_spi_transfer_abort(&vospi.spi_bus);
    mp_hal_delay_ms(200);
    vospi.flags = VOSPI_FLAGS_RESET;
    omv_spi_transfer_start(&vospi.spi_bus, &spi_xfer);
    debug_printf("vospi resync...\n");
}

static void vospi_callback(omv_spi_t *spi, void *data) {
    if (vospi.flags & VOSPI_FLAGS_RESYNC) {
        // Captured a packet before an resync is complete.
        return;
    }

    if (vospi.flags & VOSPI_FLAGS_RESET) {
        vospi.pid = VOSPI_FIRST_PACKET;
        vospi.sid = VOSPI_FIRST_SEGMENT;
        vospi.flags &= ~(VOSPI_FLAGS_RESET);
    }

    if (vospi.pid < vospi.n_packets && (vospi_packet[0] & 0xF) != 0xF) {
        uint32_t pid = VOSPI_HEADER_PID(vospi_packet);
        uint32_t sid = VOSPI_HEADER_SEG(vospi_packet);
        if (pid != (vospi.pid % VOSPI_NUMBER_PACKETS)) {
            if (vospi.pid != VOSPI_FIRST_PACKET) {
                vospi.flags |= VOSPI_FLAGS_RESYNC; // lost sync
                debug_printf("lost sync, packet id:%lu expected id:%lu \n", pid, vospi.pid);
            }
        } else if (vospi.n_packets > VOSPI_NUMBER_PACKETS
                   && pid == VOSPI_SPECIAL_PACKET && sid != vospi.sid) {
            if (vospi.sid != VOSPI_FIRST_SEGMENT) {
                vospi.flags |= VOSPI_FLAGS_RESYNC; // lost sync
                debug_printf("lost sync, segment id:%lu expected id:%lu\n", sid, vospi.sid);
            }
        } else {
            memcpy(vospi.buffer + vospi.pid * VOSPI_LINE_SIZE,
                   vospi_packet + VOSPI_HEADER_SIZE, VOSPI_LINE_SIZE);
            if ((++vospi.pid % VOSPI_NUMBER_PACKETS) == 0) {
                vospi.sid++;
            }
        }
    }
}

int vospi_init(uint32_t n_packets, void *buffer) {
    memset(&vospi, 0, sizeof(vospi_state_t));
    vospi.buffer = buffer;
    vospi.n_packets = n_packets;
    // resync on first snapshot.
    vospi.flags = VOSPI_FLAGS_RESYNC;

    omv_spi_config_t spi_config;
    omv_spi_default_config(&spi_config, ISC_SPI_ID);

    spi_config.bus_mode = OMV_SPI_BUS_RX;
    spi_config.baudrate = ISC_SPI_BAUDRATE;
    spi_config.dma_enable = true;

    if (omv_spi_init(&vospi.spi_bus, &spi_config) != 0) {
        return -1;
    }
    return 0;
}

int vospi_snapshot(uint32_t timeout_ms) {
    // Restart counters to capture a new frame.
    vospi.flags |= VOSPI_FLAGS_RESET;

    // Snapshot start tick
    mp_uint_t tick_start = mp_hal_ticks_ms();

    do {
        if (vospi.flags & VOSPI_FLAGS_RESYNC) {
            vospi_do_resync();
        }

        if ((mp_hal_ticks_ms() - tick_start) >= timeout_ms) {
            // Timeout error.
            return -1;
        }

        MICROPY_EVENT_POLL_HOOK
    } while (vospi.pid < vospi.n_packets);

    return 0;
}
#endif

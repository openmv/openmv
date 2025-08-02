/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * VOSPI driver.
 */
#include "omv_boardconfig.h"
#if OMV_ENABLE_VOSPI || OMV_LEPTON_ENABLE

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "py/mphal.h"

#include "vospi.h"
#include "crc16.h"
#include "omv_common.h"
#include "omv_spi.h"

#define VOSPI_HEADER_WORDS              (2) // 16-bits
#define VOSPI_PID_SIZE_PIXELS           (80) // w, 16-bits per pixel
#define VOSPI_PIDS_PER_SID              (60) // h
#define VOSPI_SIDS_PER_FRAME(vospi)     (VOSPI_IS_LEPTON3(vospi) ? 4 : 1)
#define VOSPI_PACKET_SIZE               (VOSPI_HEADER_WORDS + VOSPI_PID_SIZE_PIXELS) // 16-bits
#define VOSPI_SID_SIZE_PIXELS           (VOSPI_PIDS_PER_SID * VOSPI_PID_SIZE_PIXELS) // 16-bits

#define VOSPI_BUFFER_SIZE               (VOSPI_PACKET_SIZE * 2) // 16-bits
#define VOSPI_CLOCK_SPEED               20000000 // hz
#define VOSPI_SYNC_DELAY_MS             250 // ms
#define VOSPI_SYNC_TIMEOUT_MS           100

// Packet Header Format (32 bits total: ID Field + CRC)
// ┌───┬───────────┬──────────────────┬───────────────┐
// │ 0 │ 3-bit SID │ 12-bit PID (0–20)│   16-bit CRC  │
// └───┴───────────┴──────────────────┴───────────────┘
// Notes:
// - Packet numbers restart at 0 on each new segment.
// - Discard packets with PID 0x0Fxx.
// - On packet 20, the SID bits encode the segment number (1–4).
#define VOSPI_HEADER_PID(id)            ((id) & 0x0FFFu)
#define VOSPI_HEADER_SID(id)            (((id) >> 12u) & 0x7u)
#define VOSPI_IS_LEPTON3(vospi)         (vospi.n_packets > VOSPI_PIDS_PER_SID)
#define VOSPI_IS_PID_VALID(vospi, pid)  (((pid) & 0x0F00u) != 0x0F00u)
#define VOSPI_IS_SID_VALID(vospi, pid)  (VOSPI_IS_LEPTON3(vospi) && pid == 20)

typedef enum {
    VOSPI_FLAG_STREAM   = (1 << 0),
    VOSPI_FLAG_CAPTURE  = (1 << 1),
    VOSPI_FLAG_SYNC_ERROR = (1 << 2),
} vospi_flags_t;

typedef struct _vospi_state {
    size_t pid;
    size_t sid;
    framebuffer_t *fb;
    omv_spi_t spi_bus;
    size_t n_packets;
    uint32_t last_abort_ms;
    size_t last_sync_ms;
    volatile uint32_t flags;
} vospi_state_t;

static vospi_state_t vospi;
static uint16_t OMV_ATTR_SEC_ALIGN(vospi_buf[VOSPI_BUFFER_SIZE], OMV_VOSPI_DMA_BUFFER, OMV_DMA_ALIGNMENT);

static bool vospi_check_crc(const uint16_t *base) {
    #if defined(OMV_ENABLE_VOSPI_CRC)
    int id = base[0];
    int packet_crc = base[1];
    int crc = ByteCRC16((id >> 8) & 0x0F, 0);
    crc = ByteCRC16(id, crc);
    crc = ByteCRC16(0, crc);
    crc = ByteCRC16(0, crc);

    for (int i = VOSPI_HEADER_WORDS; i < VOSPI_PACKET_SIZE; i++) {
        int value = base[i];
        crc = ByteCRC16(value >> 8, crc);
        crc = ByteCRC16(value, crc);
    }

    return packet_crc == crc;
    #else
    return true;
    #endif
}

static void vospi_callback(omv_spi_t *spi, void *userdata, void *buf) {
    vbuffer_t *buffer = NULL;
    const uint16_t *base = (uint16_t *) buf;

    if (!(vospi.flags & VOSPI_FLAG_CAPTURE) ||
        !(vospi.flags & VOSPI_FLAG_STREAM)) {
        return;
    }

    size_t pid = VOSPI_HEADER_PID(base[0]);
    size_t sid = VOSPI_HEADER_SID(base[0]) - 1; // SID is one-based

    // Discard invalid/don't care packets.
    if (!VOSPI_IS_PID_VALID(vospi, pid)) {
        return;
    }

    // Abort the VoSPI stream if not sync'ing for too long.
    if ((mp_hal_ticks_ms() - vospi.last_sync_ms) >= VOSPI_SYNC_TIMEOUT_MS) {
        vospi.flags |= VOSPI_FLAG_SYNC_ERROR;
        vospi_abort();
        return;
    }

    // Discard packets with a pid != 0 when waiting for the first packet.
    if (vospi.pid == 0 && pid != 0) {
        return;
    }

    // Discard segments with a sid != 0 when waiting for the first segment.
    if (vospi.sid == 0 && sid != 0 && VOSPI_IS_SID_VALID(vospi, pid)) {
        vospi.pid = 0;
        return;
    }

    // Check if packet is in sync with the expected VoSPI stream
    if (pid != vospi.pid || !vospi_check_crc(base) ||
        (VOSPI_IS_SID_VALID(vospi, pid) && sid != vospi.sid)) {
        vospi_abort();
        return;
    }

    // Update sync timestamp.
    vospi.last_sync_ms = mp_hal_ticks_ms();

    if (!(buffer = framebuffer_acquire(vospi.fb, FB_FLAG_FREE | FB_FLAG_PEEK))) {
        vospi.flags &= ~VOSPI_FLAG_CAPTURE;
        return;
    }

    memcpy(((uint16_t *) buffer->data)
           + (vospi.pid * VOSPI_PID_SIZE_PIXELS)
           + (vospi.sid * VOSPI_SID_SIZE_PIXELS),
           base + VOSPI_HEADER_WORDS, VOSPI_PID_SIZE_PIXELS * sizeof(uint16_t));

    if (++vospi.pid == VOSPI_PIDS_PER_SID) {
        vospi.pid = 0;
        if (++vospi.sid == VOSPI_SIDS_PER_FRAME(vospi)) {
            vospi.sid = 0;
            framebuffer_release(vospi.fb, FB_FLAG_FREE);
        }
    }
}

int vospi_init(size_t n_packets, framebuffer_t *fb) {
    memset(&vospi, 0, sizeof(vospi_state_t));
    vospi.fb = fb;
    vospi.n_packets = n_packets;

    omv_spi_config_t spi_config;
    omv_spi_default_config(&spi_config, OMV_CSI_SPI_ID);

    spi_config.bus_mode = OMV_SPI_BUS_RX;
    spi_config.datasize = 16;
    spi_config.baudrate = VOSPI_CLOCK_SPEED;
    spi_config.clk_pol = OMV_SPI_CPOL_HIGH;
    spi_config.clk_pha = OMV_SPI_CPHA_2EDGE;
    spi_config.dma_flags = OMV_SPI_DMA_CIRCULAR | OMV_SPI_DMA_DOUBLE;

    return omv_spi_init(&vospi.spi_bus, &spi_config);
}

int vospi_deinit() {
    return omv_spi_deinit(&vospi.spi_bus);
}

bool vospi_active(void) {
    return (vospi.flags & VOSPI_FLAG_CAPTURE) &&
           (vospi.flags & VOSPI_FLAG_STREAM);
}

int vospi_abort(void) {
    vospi.flags = 0;
    omv_spi_deinit(&vospi.spi_bus);
    vospi.last_abort_ms = mp_hal_ticks_ms();
    return 0;
}

void vospi_restart(void) {
    // Update sync timestamp.
    vospi.last_sync_ms = mp_hal_ticks_ms();

    // Resume streaming.
    vospi.flags |= VOSPI_FLAG_CAPTURE;
    vospi.flags &= ~VOSPI_FLAG_SYNC_ERROR;

    if (!(vospi.flags & VOSPI_FLAG_STREAM) &&
        (mp_hal_ticks_ms() - vospi.last_abort_ms) >= VOSPI_SYNC_DELAY_MS) {
        omv_spi_transfer_t spi_xfer = {
            .rxbuf = vospi_buf,
            .size = VOSPI_BUFFER_SIZE,
            .flags = OMV_SPI_XFER_DMA,
            .callback = vospi_callback,
        };

        vospi_init(vospi.n_packets, vospi.fb);

        // Restart VoSPI transfer.
        vospi.flags |= VOSPI_FLAG_STREAM;
        vospi.last_sync_ms = mp_hal_ticks_ms();
        omv_spi_transfer_start(&vospi.spi_bus, &spi_xfer);
    }
}
#endif

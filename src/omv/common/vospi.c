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
#include "py/mphal.h"

#include "crc16.h"

#include "omv_common.h"
#include "omv_spi.h"

static volatile bool waiting_for_data = false;
static uint16_t *framebuffer = 0;

static bool vospi_lepton_3 = false;

static omv_spi_t spi_bus = {};

#define VOSPI_HEADER_WORDS       (2) // 16-bits
#define VOSPI_PID_SIZE_PIXELS    (80) // w, 16-bits per pixel
#define VOSPI_PIDS_PER_SEG       (60) // h
#define VOSPI_SEGS_PER_FRAME     (4)
#define VOSPI_PACKET_SIZE        (VOSPI_HEADER_WORDS + VOSPI_PID_SIZE_PIXELS) // 16-bits
#define VOSPI_SEG_SIZE_PIXELS    (VOSPI_PIDS_PER_SEG * VOSPI_PID_SIZE_PIXELS) // 16-bits

#define VOSPI_BUFFER_SIZE        (VOSPI_PACKET_SIZE * 2) // 16-bits
#define VOSPI_CLOCK_SPEED        20000000 // hz
#define VOSPI_SYNC_MS            200 // ms

static int vospi_rx_cb_expected_pid = 0;
static int vospi_rx_cb_expected_seg = 0;
static uint16_t OMV_ATTR_SECTION(OMV_ATTR_ALIGNED_DMA(vospi_buf[VOSPI_BUFFER_SIZE]), ".dma_buffer");
static void vospi_callback(omv_spi_t *spi, void *userdata, void *rxbuf);

static volatile bool vospi_resync_callback_flag = true;
static void vospi_resync_callback() {
    omv_spi_transfer_t spi_xfer = {
        .rxbuf = vospi_buf,
        .size = VOSPI_BUFFER_SIZE,
        .flags = OMV_SPI_XFER_DMA | OMV_SPI_XFER_16_BIT,
        .callback = vospi_callback,
    };

    mp_hal_delay_ms(VOSPI_SYNC_MS);
    omv_spi_transfer_start(&spi_bus, &spi_xfer);
}

#if defined(OMV_ENABLE_VOSPI_CRC)
static bool vospi_check_crc(const uint16_t *base) {
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
}
#endif


void vospi_callback(omv_spi_t *spi, void *userdata, void *rxbuf) {
    if (!waiting_for_data) {
        return;
    }

    const uint16_t *base = (uint16_t *) rxbuf;

    int id = base[0];

    // Ignore don't care packets.
    if ((id & 0x0F00) == 0x0F00) {
        return;
    }

    int pid = id & 0x0FFF;
    int seg = ((id >> 12) & 0x7) - 1;

    // Discard packets with a pid != 0 when waiting for the first packet.
    if ((vospi_rx_cb_expected_pid == 0) && (pid != 0)) {
        return;
    }

    // Discard segments with a seg != 0 when waiting for the first segment.
    if (vospi_lepton_3 && (pid == 20) && (vospi_rx_cb_expected_seg == 0) && (seg != 0)) {
        vospi_rx_cb_expected_pid = 0;
        return;
    }

    // Are we in sync with the flir lepton?
    if ((pid != vospi_rx_cb_expected_pid)
    #if defined(OMV_ENABLE_VOSPI_CRC)
        || (!vospi_check_crc(base))
    #endif
        || (vospi_lepton_3 && (pid == 20) && (seg != vospi_rx_cb_expected_seg))) {
        vospi_rx_cb_expected_pid = 0;
        vospi_rx_cb_expected_seg = 0;
        omv_spi_transfer_abort(&spi_bus);
        vospi_resync_callback_flag = true;
        return;
    }

    memcpy(framebuffer
           + (vospi_rx_cb_expected_pid * VOSPI_PID_SIZE_PIXELS)
           + (vospi_rx_cb_expected_seg * VOSPI_SEG_SIZE_PIXELS),
           base + VOSPI_HEADER_WORDS, VOSPI_PID_SIZE_PIXELS * sizeof(uint16_t));

    vospi_rx_cb_expected_pid += 1;
    if (vospi_rx_cb_expected_pid == VOSPI_PIDS_PER_SEG) {
        vospi_rx_cb_expected_pid = 0;

        bool frame_ready = false;

        // For the FLIR Lepton 3 we have to receive all the pids in all the segments.
        if (vospi_lepton_3) {
            vospi_rx_cb_expected_seg += 1;
            if (vospi_rx_cb_expected_seg == VOSPI_SEGS_PER_FRAME) {
                vospi_rx_cb_expected_seg = 0;
                frame_ready = true;
            }
            // For the FLIR Lepton 1/2 we just have to receive all the pids.
        } else {
            frame_ready = true;
        }

        if (frame_ready) {
            waiting_for_data = false;
        }
    }
}

int vospi_init(uint32_t n_packets, void *buffer) {
    vospi_lepton_3 = n_packets > VOSPI_PIDS_PER_SEG;
    framebuffer = buffer;

    omv_spi_config_t spi_config;
    omv_spi_default_config(&spi_config, ISC_SPI_ID);

    spi_config.baudrate = VOSPI_CLOCK_SPEED;
    spi_config.bus_mode = OMV_SPI_BUS_RX;
    spi_config.dma_flags = OMV_SPI_DMA_CIRCULAR | OMV_SPI_DMA_DOUBLE;

    if (omv_spi_init(&spi_bus, &spi_config) != 0) {
        return -1;
    }
    return 0;
}

int vospi_snapshot(uint32_t timeout_ms) {
    waiting_for_data = true;

    mp_uint_t tick_start = mp_hal_ticks_ms();

    for (;;) {
        if (vospi_resync_callback_flag) {
            vospi_resync_callback_flag = false;
            vospi_resync_callback();
        }

        if (!waiting_for_data) {
            break;
        }

        MICROPY_EVENT_POLL_HOOK

        if ((mp_hal_ticks_ms() - tick_start) > timeout_ms) {
            omv_spi_transfer_abort(&spi_bus);
            waiting_for_data = false;
            vospi_rx_cb_expected_pid = 0;
            vospi_rx_cb_expected_seg = 0;
            vospi_resync_callback_flag = true;
            return -1;
        }
    }

    return 0;
}

#endif

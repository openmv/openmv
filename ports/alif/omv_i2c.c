/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alif I2C driver.
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "py/mphal.h"
#include "py/runtime.h"

#include "omv_portconfig.h"
#include "omv_boardconfig.h"
#include "alif_hal.h"
#include "omv_gpio.h"
#include "omv_common.h"
#include "omv_i2c.h"
#include "sys_ctrl_i3c.h"

#define I2C_SCAN_TIMEOUT    (10)
#define I2C_XFER_TIMEOUT    (1000)

#define I2C_DAT_INDEX       (0)
#define I2C_IC_STATUS_RFNE  I2C_IC_STATUS_RECEIVE_FIFO_NOT_EMPTY
#define I2C_IC_STATUS_TFNF  I2C_IC_STATUS_TRANSMIT_FIFO_NOT_FULL

#define I2C_IC_CON_MASTER_TX_EMPTY_CTRL (1 << 8)
#define I2C_CON_SPEED(speed) \
    ((speed == I2C_SPEED_STANDARD) ? I2C_IC_CON_SPEED_STANDARD : I2C_IC_CON_SPEED_FAST)

#define I2C_TX_FIFO_LEN     (I2C_FIFO_DEPTH / 2)
#define I2C_RX_FIFO_LEN     (I2C_FIFO_DEPTH / 2)

#define I2C_STAT_ERRORS     (I2C_IC_INTR_STAT_TX_ABRT | I2C_IC_INTR_STAT_TX_OVER | \
                             I2C_IC_INTR_STAT_RX_OVER | I2C_IC_INTR_STAT_RX_UNDER)

#ifdef NDEBUG
#define I2C_CHECK_ERRORS(base)                       \
    if (base->I2C_RAW_INTR_STAT & I2C_STAT_ERRORS) { \
        (void) base->I2C_RAW_INTR_STAT;              \
        (void) base->I2C_CLR_TX_ABRT;                \
        return -1;                                   \
    }
#else
#define I2C_CHECK_ERRORS(base)                                                \
    if (base->I2C_RAW_INTR_STAT & I2C_STAT_ERRORS) {                          \
        uint32_t status = base->I2C_RAW_INTR_STAT;                            \
        printf("status: 0x%lx raw_int: 0x%lx abort: 0x%lx line: %d\n",        \
               base->I2C_STATUS, status, base->I2C_TX_ABRT_SOURCE, __LINE__); \
        (void) base->I2C_CLR_TX_ABRT;                                         \
        return -1;                                                            \
    }
#endif

typedef struct {
    uint8_t *data;
    size_t size;
    uint32_t flags;
    uint32_t direction;
    uint8_t address;
} i2c_transfer_t;

typedef enum {
    I2C_TRANSFER_READ   = (1 << 0),
    I2C_TRANSFER_WRITE  = (1 << 1),
} i2c_transfer_direction_t;

static int omv_ixc_transfer_timeout(omv_i2c_t *i2c, i2c_transfer_t *xfer, uint32_t timeout);

int omv_i2c_init(omv_i2c_t *i2c, uint32_t bus_id, uint32_t speed) {
    i2c->id = bus_id;
    i2c->initialized = false;
    i2c->is_i3c = (bus_id == OMV_I3C0_ID);
    i2c->cw_size = 0;
    i2c->speed = speed;

    switch (speed) {
        case OMV_I2C_SPEED_STANDARD:
            speed = i2c->is_i3c ? I3C_I2C_SPEED_MODE_SS_100_KBPS : I2C_SPEED_STANDARD;    // 100 kbit/s
            break;
        case OMV_I2C_SPEED_FULL:
            speed = i2c->is_i3c ? I3C_I2C_SPEED_MODE_FM_400_KBPS : I2C_SPEED_FAST;   // 400 kbit/s
            break;
        case OMV_I2C_SPEED_FAST:
            speed = i2c->is_i3c ? I3C_I2C_SPEED_MODE_FMP_1_MBPS : I2C_SPEED_FASTPLUS;    // 1000 kbit/s
            break;
        default:
            return -1;
    }

    switch (bus_id) {
        #if defined(OMV_I3C0_ID)
        case OMV_I3C0_ID: {
            i2c->inst = (void *) I3C_BASE;
            i2c->scl_pin = OMV_I3C0_SCL_PIN;
            i2c->sda_pin = OMV_I3C0_SDA_PIN;
            break;
        }
        #endif
        #if defined(OMV_I2C0_ID)
        case OMV_I2C0_ID: {
            i2c->inst = (void *) I2C0_BASE;
            i2c->scl_pin = OMV_I2C0_SCL_PIN;
            i2c->sda_pin = OMV_I2C0_SDA_PIN;
            break;
        }
        #endif
        #if defined(OMV_I2C1_ID)
        case OMV_I2C1_ID: {
            i2c->inst = (void *) I2C1_BASE;
            i2c->scl_pin = OMV_I2C1_SCL_PIN;
            i2c->sda_pin = OMV_I2C1_SDA_PIN;
            break;
        }
        #endif
        #if defined(OMV_I2C2_ID)
        case OMV_I2C2_ID: {
            i2c->inst = (void *) I2C2_BASE;
            i2c->scl_pin = OMV_I2C2_SCL_PIN;
            i2c->sda_pin = OMV_I2C2_SDA_PIN;
            break;
        }
        #endif
        #if defined(OMV_I2C3_ID)
        case OMV_I2C3_ID: {
            i2c->inst = (void *) I2C3_BASE;
            i2c->scl_pin = OMV_I2C3_SCL_PIN;
            i2c->sda_pin = OMV_I2C3_SDA_PIN;
            break;
        }
        #endif

        default:
            return -1;
    }

    alif_hal_i2c_init(bus_id);

    if (i2c->is_i3c) {
        // I3C controller in I2C/legacy mode.
        I3C_Type *inst = i2c->inst;

        // Core soft-reset.
        inst->I3C_RESET_CTRL = 0x1;
        while (inst->I3C_RESET_CTRL & 0x1) {
        }

        i3c_master_set_dynamic_addr(inst);

        i3c_master_enable_interrupts(inst);

        // Initialize I2C controller
        i3c_master_init(inst);

        // Configure clock and speed.
        i2c_clk_cfg(inst, SystemAPBClock, speed);
    } else {
        // I2C controller.
        I2C_Type *inst = i2c->inst;
        // Disable I2C.
        i2c_disable(inst);
        // Initialize I2C controller
        inst->I2C_CON = I2C_IC_CON_ENABLE_MASTER_MODE |
                        I2C_IC_CON_MASTER_RESTART_EN |
                        I2C_IC_CON_MASTER_TX_EMPTY_CTRL |
                        I2C_CON_SPEED(speed);

        // Set TX/RX FIFO threshold (i2c must be disabled)
        inst->I2C_TX_TL = I2C_TX_FIFO_LEN;
        inst->I2C_RX_TL = I2C_RX_FIFO_LEN;

        // Configure clock.
        i2c_master_set_clock(inst, SystemAPBClock / 1000, speed);
        // Re-enable I2C.
        i2c_enable(inst);
    }
    i2c->initialized = true;
    return 0;
}

int omv_i2c_deinit(omv_i2c_t *i2c) {
    if (i2c->initialized) {
        // TODO
        i2c->initialized = false;
    }
    return 0;
}

int omv_i2c_scan(omv_i2c_t *i2c, uint8_t *list, uint8_t size) {
    uint32_t idx = 0;

    for (uint8_t addr = 0x29, data = 0; addr < 0x78; addr++) {
        i2c_transfer_t xfer = {
            .data = &data,
            .size = 1,
            .flags = 0,
            .direction = I2C_TRANSFER_READ,
            .address = addr
        };

        if (omv_ixc_transfer_timeout(i2c, &xfer, I2C_SCAN_TIMEOUT) == 0) {
            if (list == NULL || size == 0) {
                idx = (addr << 1);
                break;
            } else if (idx < size) {
                list[idx++] = (addr << 1);
            } else {
                break;
            }
        }
        mp_event_handle_nowait();
    }

    return idx;
}

int omv_i2c_enable(omv_i2c_t *i2c, bool enable) {
    if (i2c->is_i3c) {
        //TODO: For I3C this causes a lockup.
        I3C_Type *inst = i2c->inst;
        if (enable) {
            inst->I3C_DEVICE_CTRL = inst->I3C_DEVICE_CTRL & ~I3C_DEVICE_CTRL_ENABLE;
            while (inst->I3C_DEVICE_CTRL & I3C_DEVICE_CTRL_ENABLE) {
            }

        } else {
            inst->I3C_DEVICE_CTRL = inst->I3C_DEVICE_CTRL | I3C_DEVICE_CTRL_ENABLE;
            while (!(inst->I3C_DEVICE_CTRL & I3C_DEVICE_CTRL_ENABLE)) {
            }
            i3c_resume(inst);
        }
    } else {
        I2C_Type *inst = i2c->inst;
        if (enable) {
            i2c_enable(inst);
        } else {
            i2c_disable(inst);
        }
    }
    return 0;
}

int omv_i2c_gencall(omv_i2c_t *i2c, uint8_t cmd) {
    int ret = 0;
    ret |= omv_i2c_write(i2c, 0, &cmd, 1, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

static int i2c_poll_flags(I2C_Type *base, uint32_t flags, uint32_t timeout) {
    mp_uint_t tick_start = mp_hal_ticks_ms();
    while (!(base->I2C_STATUS & flags)) {
        I2C_CHECK_ERRORS(base);
        if ((mp_hal_ticks_ms() - tick_start) >= timeout) {
            // Should not raise exception as we're not always in nlr context.
            return -1;
        }
        mp_event_handle_nowait();
    }
    return 0;
}

static int omv_i2c_transfer_timeout(omv_i2c_t *i2c, i2c_transfer_t *xfer, uint32_t timeout) {
    I2C_Type *base = i2c->inst;

    i2c_clear_all_interrupt(base);
    i2c_set_target_addr(base, xfer->address, I2C_7BIT_ADDRESS, 0);

    // Write buffered transfer (if any) first.
    for (size_t cw_idx = 0; cw_idx < i2c->cw_size;) {
        // Write data to FIFO
        if (base->I2C_STATUS & I2C_IC_STATUS_TFNF) {
            base->I2C_DATA_CMD = (uint16_t) i2c->cw_buf[cw_idx++];
            I2C_CHECK_ERRORS(base);
        }
    }

    size_t tx_size = (xfer->direction == I2C_TRANSFER_WRITE) ? xfer->size : 0;
    for (size_t tx_idx = 0; tx_idx < tx_size; ) {
        // Write data to FIFO
        if (base->I2C_STATUS & I2C_IC_STATUS_TFNF) {
            base->I2C_DATA_CMD = (uint16_t) xfer->data[tx_idx++];
            I2C_CHECK_ERRORS(base);
        }
        // Wait for TX FIFO empty
        if (tx_idx == tx_size && i2c_poll_flags(base, I2C_IC_STATUS_TFE, 10) != 0) {
            return -1;
        }
    }

    size_t rx_size = (xfer->direction == I2C_TRANSFER_READ) ? xfer->size : 0;
    for (size_t tx_idx = 0, rx_idx = 0; rx_idx < rx_size; ) {
        // Write command to FIFO
        if ((base->I2C_STATUS & I2C_IC_STATUS_TFNF) &&
            !(base->I2C_STATUS & I2C_IC_STATUS_RFNE) && tx_idx++ < rx_size) {
            base->I2C_DATA_CMD = I2C_IC_DATA_CMD_READ_REQ;
            I2C_CHECK_ERRORS(base);
        }

        // Wait for RX FIFO not empty
        if (i2c_poll_flags(base, I2C_IC_STATUS_RFNE, timeout) != 0) {
            return -1;
        }

        // Read data from FIFO
        while ((base->I2C_STATUS & I2C_IC_STATUS_RFNE) && rx_idx < rx_size) {
            xfer->data[rx_idx++] = base->I2C_DATA_CMD & 0xFF;
        }
    }

    return 0;
}

static int omv_i3c_transfer_timeout(omv_i2c_t *i2c, i2c_transfer_t *xfer, uint32_t timeout) {
    int ret = 0;
    i3c_xfer_t i3c_xfer = {0};
    I3C_Type *base = i2c->inst;

    i3c_add_slv_to_dat(base, I2C_DAT_INDEX, 0, xfer->address);

    i3c_xfer.xfer_cmd.speed = I3C_SPEED_SDR0;
    // i3c_xfer.addr = xfer->address;

    i3c_xfer.xfer_cmd.addr_index = I2C_DAT_INDEX;
    // i3c_xfer.xfer_cmd.addr_depth = 1U;
    i3c_xfer.xfer_cmd.data_len = xfer->size;

    if (xfer->direction == I2C_TRANSFER_READ) {
        i3c_xfer.rx_buf = xfer->data;
        i3c_xfer.rx_len = xfer->size;
        i3c_master_rx(base, &i3c_xfer);
    } else {
        i3c_xfer.tx_buf = xfer->data;
        i3c_xfer.tx_len = xfer->size;
        i3c_master_tx(base, &i3c_xfer);
    }

    // Wait for the transfer to finish.
    mp_uint_t tick_start = mp_hal_ticks_ms();
    /* Waits till some response received */
    while (base->I3C_INTR_STATUS == 0) {
        if ((mp_hal_ticks_ms() - tick_start) >= timeout) {
            // Should not raise exception as we're not always in nlr context.
            ret = -1;
            goto cleanup;
        }
        mp_event_handle_nowait();
    }

    uint32_t status = base->I3C_INTR_STATUS;
    // See Table 15-81 Response Data Structure
    uint32_t resp = base->I3C_RESPONSE_QUEUE_PORT;

    if ((status & I3C_INTR_STATUS_TRANSFER_ERR_STS) || I3C_RESPONSE_QUEUE_PORT_ERR_STATUS(resp)) {
        ret = -1;
        goto cleanup;
    }

    if (xfer->direction == I2C_TRANSFER_READ && xfer->data) {
        for (uint32_t i = 0, dr = 0; i < xfer->size; i++, dr >>= 8) {
            if ((i % 4) == 0) {
                dr = base->I3C_RX_DATA_PORT;
            }
            xfer->data[i] = dr & 0xFF;
        }
    }

cleanup:
    i3c_clear_xfer_error(base);
    i3c_resume(base);
    return ret;
}

static int omv_ixc_transfer_timeout(omv_i2c_t *i2c, i2c_transfer_t *xfer, uint32_t timeout) {
    int ret = 0;
    if (i2c->is_i3c) {
        ret = omv_i3c_transfer_timeout(i2c, xfer, timeout);
    } else {
        uint32_t xfer_cont = (OMV_I2C_XFER_NO_STOP | OMV_I2C_XFER_SUSPEND);
        if (xfer->direction == I2C_TRANSFER_WRITE && (xfer->flags & xfer_cont)) {
            if (xfer->size > sizeof(i2c->cw_buf)) {
                return -1;
            }
            i2c->cw_size = xfer->size;
            memcpy(i2c->cw_buf, xfer->data, xfer->size);
        } else {
            ret = omv_i2c_transfer_timeout(i2c, xfer, timeout);
            i2c->cw_size = 0;
        }
    }
    return ret;
}


int omv_i2c_read(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags) {
    i2c_transfer_t xfer = {
        .data = buf,
        .size = len,
        .flags = flags,
        .direction = I2C_TRANSFER_READ,
        .address = (slv_addr >> 1)
    };
    return omv_ixc_transfer_timeout(i2c, &xfer, I2C_XFER_TIMEOUT);
}

int omv_i2c_write(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags) {
    i2c_transfer_t xfer = {
        .data = buf,
        .size = len,
        .flags = flags,
        .direction = I2C_TRANSFER_WRITE,
        .address = (slv_addr >> 1)
    };
    return omv_ixc_transfer_timeout(i2c, &xfer, I2C_XFER_TIMEOUT);
}

int omv_i2c_pulse_scl(omv_i2c_t *i2c) {
    if (i2c->initialized && i2c->scl_pin) {
        omv_i2c_deinit(i2c);
        omv_gpio_config(i2c->scl_pin, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        // Pulse SCL to recover stuck device.
        for (int i = 0; i < 1000; i++) {
            omv_gpio_write(i2c->scl_pin, 1);
            mp_hal_delay_us(10);
            omv_gpio_write(i2c->scl_pin, 0);
            mp_hal_delay_us(10);
        }
        omv_i2c_init(i2c, i2c->id, i2c->speed);
    }
    return 0;
}

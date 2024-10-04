/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2021-2024 OpenMV, LLC.
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
 * I2C port for rp2.
 */
#include <stdio.h>
#include <stdbool.h>
#include "py/mphal.h"

#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "omv_boardconfig.h"
#include "omv_i2c.h"

#define I2C_TIMEOUT         (100 * 1000)
#define I2C_SCAN_TIMEOUT    (1 * 1000)

int omv_i2c_init(omv_i2c_t *i2c, uint32_t bus_id, uint32_t speed) {
    i2c->id = bus_id;
    i2c->initialized = false;

    switch (speed) {
        case OMV_I2C_SPEED_STANDARD:
            i2c->speed = 100 * 1000;    ///< 100 kbps
            break;
        case OMV_I2C_SPEED_FULL:
            i2c->speed = 250 * 1000;    ///< 250 kbps
            break;
        case OMV_I2C_SPEED_FAST:
            i2c->speed = 1000 * 1000;   ///< 1000 kbps
            break;
        default:
            return -1;
    }

    switch (bus_id) {
        case 0: {
            i2c->inst = i2c0;
            i2c->scl_pin = OMV_I2C0_SCL_PIN;
            i2c->sda_pin = OMV_I2C0_SDA_PIN;
            break;
        }
        case 1: {
            i2c->inst = i2c1;
            i2c->scl_pin = OMV_I2C1_SCL_PIN;
            i2c->sda_pin = OMV_I2C1_SDA_PIN;
            break;
        }
        default:
            return -1;
    }

    i2c_init(i2c->inst, i2c->speed);
    gpio_set_function(i2c->scl_pin, GPIO_FUNC_I2C);
    gpio_set_function(i2c->sda_pin, GPIO_FUNC_I2C);

    i2c->initialized = true;
    return 0;
}

int omv_i2c_deinit(omv_i2c_t *i2c) {
    if (i2c->initialized) {
        i2c_deinit(i2c->inst);
        i2c->initialized = false;
    }
    return 0;
}

int omv_i2c_scan(omv_i2c_t *i2c, uint8_t *list, uint8_t size) {
    int idx = 0;
    for (uint8_t addr = 0x20, rxdata; addr <= 0x77; addr++) {
        if (i2c_read_timeout_us(i2c->inst, addr, &rxdata, 1, false, I2C_SCAN_TIMEOUT) >= 0) {
            if (list == NULL || size == 0) {
                return (addr << 1);
            } else if (idx < size) {
                list[idx++] = (addr << 1);
            } else {
                break;
            }
        }
    }
    return idx;
}

int omv_i2c_enable(omv_i2c_t *i2c, bool enable) {
    return 0;
}

int omv_i2c_gencall(omv_i2c_t *i2c, uint8_t cmd) {
    int bytes = 0;
    bytes += i2c_write_timeout_us(i2c->inst, 0x00, &cmd, 1, false, I2C_TIMEOUT);
    return (bytes == 1) ? 0 : -1;
}

int omv_i2c_readb(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr,  uint8_t *reg_data) {
    int bytes = 0;
    slv_addr = slv_addr >> 1;

    bytes += i2c_write_timeout_us(i2c->inst, slv_addr, &reg_addr, 1, false, I2C_TIMEOUT);
    bytes += i2c_read_timeout_us(i2c->inst, slv_addr, reg_data, 1, false, I2C_TIMEOUT);

    return (bytes == 2) ? 0 : -1;
}

int omv_i2c_writeb(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data) {
    int bytes = 0;
    slv_addr = slv_addr >> 1;

    uint8_t buf[] = {reg_addr, reg_data};
    bytes = i2c_write_timeout_us(i2c->inst, slv_addr, buf, 2, false, I2C_TIMEOUT);

    return (bytes == 2) ? 0 : -1;
}

int omv_i2c_read_bytes(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags) {
    int bytes = 0;
    slv_addr = slv_addr >> 1;
    bool nostop = false;

    if (flags & OMV_I2C_XFER_NO_STOP) {
        nostop = true;
    }

    bytes = i2c_read_timeout_us(i2c->inst, slv_addr, buf, len, nostop, I2C_TIMEOUT);

    return (bytes == len) ? 0 : -1;
}

int omv_i2c_write_bytes(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags) {
    int bytes = 0;
    slv_addr = slv_addr >> 1;
    bool nostop = false;

    if (flags & OMV_I2C_XFER_NO_STOP) {
        nostop = true;
    }

    bytes = i2c_write_timeout_us(i2c->inst, slv_addr, buf, len, nostop, I2C_TIMEOUT);

    return (bytes == len) ? 0 : -1;
}

int omv_i2c_pulse_scl(omv_i2c_t *i2c) {
    omv_i2c_deinit(i2c);

    // Configure SCL as GPIO
    gpio_init(i2c->scl_pin);
    gpio_set_dir(i2c->scl_pin, GPIO_OUT);

    // Pulse SCL to recover stuck device.
    for (int i = 0; i < 10000; i++) {
        gpio_put(i2c->scl_pin, 1);
        mp_hal_delay_us(10);
        gpio_put(i2c->scl_pin, 0);
        mp_hal_delay_us(10);
    }
    return 0;
}

/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
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
 * VL53L9CX platform implementation.
 */
#include "board_config.h"
#if (OMV_VL53L9CX_ENABLE == 1)

#include <string.h>
#include "py/mphal.h"
#include "omv_i3c.h"
#include "umalloc.h"
#include "platform.h"
#include "vl53l9_platform.h"

// Writes up to this size are sent from the stack.
#define VL53L9_STACK_BUF_SIZE   (32)

int vl53l9_read(void *const p_dev, uint16_t address, uint8_t *p_values, uint32_t size) {
    vl53l9_omv_dev_t *dev = p_dev;
    uint8_t addr_buf[] = { address >> 8, address };
    omv_i3c_msg_t msgs[] = {
        { addr_buf, sizeof(addr_buf), 0 },
        { p_values, size, OMV_I3C_MSG_READ },
    };

    if (omv_i3c_transfer(dev->bus, dev->addr, msgs, 2) != 0) {
        return VL53L9_ERROR_PLATFORM;
    }
    return VL53L9_ERROR_NONE;
}

int vl53l9_read8(void *const p_dev, uint16_t address, uint8_t *p_value) {
    return vl53l9_read(p_dev, address, p_value, 1);
}

int vl53l9_read16(void *const p_dev, uint16_t address, uint16_t *p_value) {
    uint8_t buf[2];
    int ret = vl53l9_read(p_dev, address, buf, sizeof(buf));
    *p_value = buf[0] | (buf[1] << 8);
    return ret;
}

int vl53l9_read32(void *const p_dev, uint16_t address, uint32_t *p_value) {
    uint8_t buf[4];
    int ret = vl53l9_read(p_dev, address, buf, sizeof(buf));
    *p_value = buf[0] | (buf[1] << 8) | (buf[2] << 16) | ((uint32_t) buf[3] << 24);
    return ret;
}

// Unused: frames are read by the sensor driver with omv_i3c_transfer_async().
int vl53l9_read_async(void *const p_dev, uint16_t address, volatile uint8_t *p_values, uint32_t size) {
    return vl53l9_read(p_dev, address, (uint8_t *) p_values, size);
}

int vl53l9_write(void *const p_dev, uint16_t address, uint8_t *p_values, uint32_t size) {
    vl53l9_omv_dev_t *dev = p_dev;
    uint8_t stack_buf[VL53L9_STACK_BUF_SIZE];
    uint8_t *buf = stack_buf;
    int ret = VL53L9_ERROR_NONE;

    // The address and data must be sent as one message.
    if (size + 2 > sizeof(stack_buf)) {
        buf = uma_malloc(size + 2, UMA_FAST | UMA_MAYBE);
        if (buf == NULL) {
            return VL53L9_ERROR_PLATFORM;
        }
    }

    buf[0] = address >> 8;
    buf[1] = address;
    memcpy(buf + 2, p_values, size);

    omv_i3c_msg_t msg = { buf, size + 2, 0 };
    if (omv_i3c_transfer(dev->bus, dev->addr, &msg, 1) != 0) {
        ret = VL53L9_ERROR_PLATFORM;
    }

    if (buf != stack_buf) {
        uma_free(buf);
    }
    return ret;
}

int vl53l9_write8(void *const p_dev, uint16_t address, uint8_t value) {
    return vl53l9_write(p_dev, address, &value, 1);
}

int vl53l9_write16(void *const p_dev, uint16_t address, uint16_t value) {
    uint8_t buf[] = { value, value >> 8 };
    return vl53l9_write(p_dev, address, buf, sizeof(buf));
}

int vl53l9_write32(void *const p_dev, uint16_t address, uint32_t value) {
    uint8_t buf[] = { value, value >> 8, value >> 16, value >> 24 };
    return vl53l9_write(p_dev, address, buf, sizeof(buf));
}

int vl53l9_wait_ms(void *const p_dev, uint32_t delay_ms) {
    mp_hal_delay_ms(delay_ms);
    return VL53L9_ERROR_NONE;
}

int vl53l9_get_config_vddio(void *const p_dev, vl53l9_vddio_t *voltage) {
    *voltage = VDDIO_1V8;
    return VL53L9_ERROR_NONE;
}

int vl53l9_get_config_vdda(void *const p_dev, vl53l9_vdda_t *voltage) {
    *voltage = VDDA_2V8;
    return VL53L9_ERROR_NONE;
}

int vl53l9_get_config_ext_clock(void *const p_dev, uint32_t *ext_clock) {
    *ext_clock = ((vl53l9_omv_dev_t *) p_dev)->ext_clock;
    return VL53L9_ERROR_NONE;
}
#endif // (OMV_VL53L9CX_ENABLE == 1)

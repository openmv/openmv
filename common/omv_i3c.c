/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2026 OpenMV, LLC.
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
 * I3C bus abstraction layer.
 */
#include "omv_i3c.h"

__attribute__((weak))
int omv_i3c_read_reg(omv_i2c_t *i3c, uint8_t slv_addr,
                     uint32_t reg_addr, uint8_t addr_size,
                     void *data, uint8_t data_size) {
    uint8_t addr_buf[4];
    for (int i = 0; i < addr_size; i++) {
        addr_buf[i] = (reg_addr >> (8 * (addr_size - 1 - i))) & 0xFF;
    }

    int ret = omv_i3c_write(i3c, slv_addr, addr_buf, addr_size, OMV_I2C_XFER_NO_STOP);

    uint8_t data_buf[4] = {0};
    ret |= omv_i3c_read(i3c, slv_addr, data_buf, data_size, OMV_I2C_XFER_NO_FLAGS);

    // Convert from big-endian to host byte order
    uint32_t value = 0;
    for (int i = 0; i < data_size; i++) {
        value = (value << 8) | data_buf[i];
    }

    switch (data_size) {
        case 1:
            *(uint8_t *) data = value;
            break;
        case 2:
            *(uint16_t *) data = value;
            break;
        case 4:
            *(uint32_t *) data = value;
            break;
    }

    return ret;
}

__attribute__((weak))
int omv_i3c_write_reg(omv_i2c_t *i3c, uint8_t slv_addr,
                      uint32_t reg_addr, uint8_t addr_size,
                      uint32_t data, uint8_t data_size) {
    uint8_t buf[8];
    int idx = 0;

    // Serialize register address (big-endian)
    for (int i = 0; i < addr_size; i++) {
        buf[idx++] = (reg_addr >> (8 * (addr_size - 1 - i))) & 0xFF;
    }

    // Serialize data (big-endian)
    for (int i = 0; i < data_size; i++) {
        buf[idx++] = (data >> (8 * (data_size - 1 - i))) & 0xFF;
    }

    return omv_i3c_write(i3c, slv_addr, buf, idx, OMV_I2C_XFER_NO_FLAGS);
}
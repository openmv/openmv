/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
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
 * VL53L8CX platform implementation.
 */
#include "omv_boardconfig.h"
#if (OMV_TOF_VL53L8CX_ENABLE == 1)

#include "py/mphal.h"
#include "omv_i2c.h"
#include "omv_gpio.h"
#include "vl53l8cx_api.h"

void vl53l8cx_reset(VL53L8CX_Platform *platform) {
    #if defined(OMV_TOF_RESET_PIN)
    omv_gpio_config(OMV_TOF_RESET_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_TOF_RESET_PIN, 1);
    mp_hal_delay_ms(10);
    omv_gpio_write(OMV_TOF_RESET_PIN, 0);
    mp_hal_delay_ms(10);
    #elif defined(OMV_TOF_POWER_PIN)
    omv_gpio_config(OMV_TOF_POWER_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_TOF_POWER_PIN, 0);
    mp_hal_delay_ms(10);
    omv_gpio_write(OMV_TOF_POWER_PIN, 1);
    mp_hal_delay_ms(10);
    #endif
}

void vl53l8cx_shutdown(VL53L8CX_Platform *platform) {
    #if defined(OMV_TOF_POWER_PIN)
    omv_gpio_config(OMV_TOF_POWER_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_TOF_POWER_PIN, 0);
    #endif
}

void vl53l8cx_swap(uint8_t *buf, uint16_t size) {
    for (size_t i=0; i<size; i++) {
        ((uint32_t *) buf)[i] = __REV(((uint32_t *) buf)[i]);
    }
}

uint8_t vl53l8cx_read(VL53L8CX_Platform *platform, uint16_t addr, uint8_t *buf, uint32_t size) {
    addr = __REVSH(addr);

    if (omv_i2c_write(platform->bus, platform->address, (uint8_t *) &addr, 2, OMV_I2C_XFER_NO_STOP) != 0) {
        return VL53L8CX_STATUS_ERROR;
    }

    if (omv_i2c_read(platform->bus, platform->address, buf, size, OMV_I2C_XFER_NO_FLAGS) != 0) {
        return VL53L8CX_STATUS_ERROR;
    }
    return 0;
}

uint8_t vl53l8cx_write(VL53L8CX_Platform *platform, uint16_t addr, uint8_t *buf, uint32_t size) {
    addr = __REVSH(addr);

    if (omv_i2c_write(platform->bus, platform->address, (uint8_t*) &addr, 2, OMV_I2C_XFER_SUSPEND) != 0) {
        return VL53L8CX_STATUS_ERROR;
    }

    if (omv_i2c_write(platform->bus, platform->address, buf, size, OMV_I2C_XFER_NO_FLAGS) != 0) {
        return VL53L8CX_STATUS_ERROR;
    }

    return 0;
}
#endif // #if (OMV_TOF_VL53L8CX_ENABLE == 1)

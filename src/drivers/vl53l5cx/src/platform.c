/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * VL53L5CX platform implementation.
 */

#include "omv_boardconfig.h"
#if (OMV_TOF_VL53L5CX_ENABLE == 1)

#include "py/mphal.h"
#include "omv_i2c.h"
#include "omv_gpio.h"
#include "vl53l5cx_api.h"

void vl53l5cx_reset(VL53L5CX_Platform *platform) {
    omv_gpio_config(OMV_TOF_RESET_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_TOF_RESET_PIN, 1);
    mp_hal_delay_ms(10);
    omv_gpio_write(OMV_TOF_RESET_PIN, 0);
    mp_hal_delay_ms(10);
}

void vl53l5cx_swap(uint8_t *buf, uint16_t size) {
    for (size_t i=0; i<size; i++) {
        ((uint32_t *) buf)[i] = __REV(((uint32_t *) buf)[i]);
    }
}

uint8_t vl53l5cx_read(VL53L5CX_Platform *platform, uint16_t addr, uint8_t *buf, uint32_t size) {
    addr = __REVSH(addr);

    if (omv_i2c_write_bytes(platform->bus, platform->address, (uint8_t *) &addr, 2, OMV_I2C_XFER_NO_STOP) != 0) {
        return VL53L5CX_STATUS_ERROR;
    }

    if (omv_i2c_read_bytes(platform->bus, platform->address, buf, size, OMV_I2C_XFER_NO_FLAGS) != 0) {
        return VL53L5CX_STATUS_ERROR;
    }
    return 0;
}

uint8_t vl53l5cx_write(VL53L5CX_Platform *platform, uint16_t addr, uint8_t *buf, uint32_t size) {
    addr = __REVSH(addr);

    if (omv_i2c_write_bytes(platform->bus, platform->address, (uint8_t*) &addr, 2, OMV_I2C_XFER_SUSPEND) != 0) {
        return VL53L5CX_STATUS_ERROR;
    }

    if (omv_i2c_write_bytes(platform->bus, platform->address, buf, size, OMV_I2C_XFER_NO_FLAGS) != 0) {
        return VL53L5CX_STATUS_ERROR;
    }

    return 0;
}
#endif // #if (OMV_TOF_VL53L5CX_ENABLE == 1)

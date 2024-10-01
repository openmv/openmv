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

#ifndef __VL53L5CX_PLATFORM_H__
#define __VL53L5CX_PLATFORM_H__
#include <stdint.h>
#include <string.h>
#include <omv_i2c.h>
#include "py/mphal.h"

// Driver configuration
#define VL53L5CX_NB_TARGET_PER_ZONE (1U)
#define VL53L5CX_DISABLE_AMBIENT_PER_SPAD
#define VL53L5CX_DISABLE_NB_SPADS_ENABLED
#define VL53L5CX_DISABLE_NB_TARGET_DETECTED
#define VL53L5CX_DISABLE_SIGNAL_PER_SPAD
#define VL53L5CX_DISABLE_RANGE_SIGMA_MM
#define VL53L5CX_DISABLE_REFLECTANCE_PERCENT
#define VL53L5CX_DISABLE_TARGET_STATUS
#define VL53L5CX_DISABLE_MOTION_INDICATOR

// Platform struct.
typedef struct {
    omv_i2c_t *bus;
    uint16_t address;
} VL53L5CX_Platform;

void vl53l5cx_reset(VL53L5CX_Platform *platform);
void vl53l5cx_swap(uint8_t *buf, uint16_t size);
uint8_t vl53l5cx_read(VL53L5CX_Platform *platform, uint16_t addr, uint8_t *buf, uint32_t size);
uint8_t vl53l5cx_write(VL53L5CX_Platform *platform, uint16_t addr, uint8_t *buf, uint32_t size);

// Platform API.
#define VL53L5CX_CHECK_STATUS(status) ({ \
        if (status != VL53L5CX_STATUS_OK) { \
            return -1; \
        } \
        status; \
    })
#define VL53L5CX_RdByte(platform, addr, buf) ({ \
        uint8_t status = vl53l5cx_read(platform, addr, buf, 1); \
        VL53L5CX_CHECK_STATUS(status); \
    })

#define VL53L5CX_WrByte(platform, addr, buf) ({ \
        uint8_t status = vl53l5cx_write(platform, addr, (uint8_t [1]){ buf }, 1); \
        VL53L5CX_CHECK_STATUS(status); \
    })

#define VL53L5CX_RdMulti(platform, addr, buf, size) ({ \
        uint8_t status = vl53l5cx_read(platform, addr, buf, size); \
        VL53L5CX_CHECK_STATUS(status); \
    })

#define VL53L5CX_WrMulti(platform, addr, buf, size) ({ \
        uint8_t status = vl53l5cx_write(platform, addr, buf, size); \
        VL53L5CX_CHECK_STATUS(status); \
    })
#define VL53L5CX_WaitMs(platform, ms) ({ mp_hal_delay_ms(ms); 0; })
#define VL53L5CX_SwapBuffer(buf, size) vl53l5cx_swap(buf, size)
#endif	// __VL53L5CX_PLATFORM_H__

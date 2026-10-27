/**
 ******************************************************************************
 * @file    vl53l9_platform.h
 * @author  IMD Software Team
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#ifndef VL53L9_PLATFORM_H
#define VL53L9_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vl53l9.h"
#include <stdint.h>

/**
 * An implementation of the following methods must be provided by the integrator to enable register-level operations.
 * The implementation must return VL53L9_ERROR_NONE on success, or VL53L9_ERROR_PLATFORM code otherwise.
 */

int vl53l9_read(void *const p_dev, uint16_t address, uint8_t *p_values, uint32_t size);
int vl53l9_read8(void *const p_dev, uint16_t address, uint8_t *p_value);
int vl53l9_read16(void *const p_dev, uint16_t address, uint16_t *p_value);
int vl53l9_read32(void *const p_dev, uint16_t address, uint32_t *p_value);

int vl53l9_read_async(void *const p_dev, uint16_t address, volatile uint8_t *p_values, uint32_t size);

int vl53l9_write(void *const p_dev, uint16_t address, uint8_t *p_values, uint32_t size);
int vl53l9_write8(void *const p_dev, uint16_t address, uint8_t value);
int vl53l9_write16(void *const p_dev, uint16_t address, uint16_t value);
int vl53l9_write32(void *const p_dev, uint16_t address, uint32_t value);

int vl53l9_wait_ms(void *const p_dev, uint32_t delay_ms);

int vl53l9_get_config_vddio(void *const p_dev, vl53l9_vddio_t *voltage);
int vl53l9_get_config_vdda(void *const p_dev, vl53l9_vdda_t *voltage);
int vl53l9_get_config_ext_clock(void *const p_dev, uint32_t *ext_clock);

#ifdef __cplusplus
}
#endif

#endif // VL53L9_PLATFORM_H

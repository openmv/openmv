/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * I2C bus abstraction layer.
 */
#ifndef __OMV_I2C_H__
#define __OMV_I2C_H__
#include <stdint.h>
#include <stdbool.h>
#include "omv_portconfig.h"

// Transfer speeds
typedef enum _omv_i2c_speed {
    OMV_I2C_SPEED_STANDARD = (0U),
    OMV_I2C_SPEED_FULL     = (1U),
    OMV_I2C_SPEED_FAST     = (2U),
    OMV_I2C_SPEED_MAX      = (3U)
} omv_i2c_speed_t;

// For use with read/write_bytes
typedef enum omv_i2c_xfer_flags {
    // Stop condition after the transfer.
    // Normal transfer with start condition, address, data and stop condition.
    OMV_I2C_XFER_NO_FLAGS =   (0 << 0),
    // No stop condition after the transfer.
    // This flag allows the next transfer to change direction with repeated start.
    OMV_I2C_XFER_NO_STOP =   (1 << 0),
    // No stop condition after the transfer.
    // This flag allows chaining multiple writes or reads with the same direction.
    OMV_I2C_XFER_SUSPEND =   (1 << 1),
} omv_i2c_xfer_flags_t;

typedef struct _omv_i2c {
    uint32_t id;
    uint32_t speed;
    uint32_t initialized;
    omv_gpio_t scl_pin;
    omv_gpio_t sda_pin;
    omv_i2c_dev_t inst;
    #ifdef OMV_I2C_PORT_BITS
    // Additional port-specific fields like device base pointer,
    // dma handles, more I/Os etc... are included directly here,
    // so that they can be accessible from this struct.
    OMV_I2C_PORT_BITS
    #endif
} omv_i2c_t;

int omv_i2c_init(omv_i2c_t *i2c, uint32_t bus_id, uint32_t speed);
int omv_i2c_deinit(omv_i2c_t *i2c);
int omv_i2c_scan(omv_i2c_t *i2c, uint8_t *list, uint8_t size);
int omv_i2c_enable(omv_i2c_t *i2c, bool enable);
int omv_i2c_gencall(omv_i2c_t *i2c, uint8_t cmd);
int omv_i2c_pulse_scl(omv_i2c_t *i2c);
int omv_i2c_readb(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr,  uint8_t *reg_data);
int omv_i2c_writeb(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data);
int omv_i2c_readb2(omv_i2c_t *i2c, uint8_t slv_addr, uint16_t reg_addr,  uint8_t *reg_data);
int omv_i2c_writeb2(omv_i2c_t *i2c, uint8_t slv_addr, uint16_t reg_addr, uint8_t reg_data);
int omv_i2c_readw(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr,  uint16_t *reg_data);
int omv_i2c_writew(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr, uint16_t reg_data);
int omv_i2c_readw2(omv_i2c_t *i2c, uint8_t slv_addr, uint16_t reg_addr,  uint16_t *reg_data);
int omv_i2c_writew2(omv_i2c_t *i2c, uint8_t slv_addr, uint16_t reg_addr, uint16_t reg_data);
int omv_i2c_read_bytes(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags);
int omv_i2c_write_bytes(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags);
#endif // __OMV_I2C_H__

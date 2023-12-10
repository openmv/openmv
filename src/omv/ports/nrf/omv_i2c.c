/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * I2C port for nRF.
 */
#include <string.h>
#include <stdbool.h>
#include "py/mphal.h"

#include "omv_boardconfig.h"
#include "omv_i2c.h"
#include "omv_common.h"

#define I2C_TIMEOUT         (1000)
#define I2C_SCAN_TIMEOUT    (100)

int omv_i2c_init(omv_i2c_t *i2c, uint32_t bus_id, uint32_t speed) {
    i2c->id = bus_id;
    i2c->initialized = false;

    switch (speed) {
        case OMV_I2C_SPEED_STANDARD:
            i2c->speed = TWI_FREQUENCY_FREQUENCY_K100; ///< 100 kbps
            break;
        case OMV_I2C_SPEED_FULL:
            i2c->speed = TWI_FREQUENCY_FREQUENCY_K250; ///< 250 kbps
            break;
        case OMV_I2C_SPEED_FAST:
            i2c->speed = TWI_FREQUENCY_FREQUENCY_K400;  ///< 400 kbps
            break;
        default:
            return -1;
    }

    switch (bus_id) {
        case 0: {
            i2c->scl_pin = TWI0_SCL_PIN;
            i2c->sda_pin = TWI0_SDA_PIN;
            i2c->inst = (nrfx_twi_t) NRFX_TWI_INSTANCE(0);
            break;
        }
        case 1: {
            i2c->scl_pin = TWI1_SCL_PIN;
            i2c->sda_pin = TWI1_SDA_PIN;
            i2c->inst = (nrfx_twi_t) NRFX_TWI_INSTANCE(1);
            break;
        }
        default:
            return -1;
    }

    nrfx_twi_config_t config = {
        .scl = i2c->scl_pin,
        .sda = i2c->sda_pin,
        .frequency = i2c->speed,
        .interrupt_priority = 4,
        .hold_bus_uninit = false
    };

    if (nrfx_twi_init(&i2c->inst, &config, NULL, NULL) != NRFX_SUCCESS) {
        return -1;
    }

    // This bus needs to be enabled for suspended transfers.
    nrfx_twi_enable(&i2c->inst);

    i2c->initialized = true;
    return 0;
}

int omv_i2c_deinit(omv_i2c_t *i2c) {
    if (i2c->initialized) {
        nrfx_twi_disable(&i2c->inst);
        nrfx_twi_uninit(&i2c->inst);
        i2c->initialized = false;
    }
    return 0;
}

int omv_i2c_scan(omv_i2c_t *i2c, uint8_t *list, uint8_t size) {
    int idx = 0;
    uint8_t data;
    uint32_t xfer_flags = 0;
    for (uint8_t addr = 0x09; addr <= 0x77; addr++) {
        nrfx_twi_xfer_desc_t desc = NRFX_TWI_XFER_DESC_RX(addr, &data, 1);
        if (nrfx_twi_xfer(&i2c->inst, &desc, xfer_flags) == NRFX_SUCCESS) {
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
    if (i2c->initialized) {
        if (enable) {
            nrfx_twi_enable(&i2c->inst);
        } else {
            nrfx_twi_disable(&i2c->inst);
        }
    }
    return 0;
}

int omv_i2c_gencall(omv_i2c_t *i2c, uint8_t cmd) {
    uint32_t xfer_flags = 0;
    nrfx_twi_xfer_desc_t desc = NRFX_TWI_XFER_DESC_TX(0x00, &cmd, 1);
    if (nrfx_twi_xfer(&i2c->inst, &desc, xfer_flags) != NRFX_SUCCESS) {
        return -1;
    }
    return 0;
}

int omv_i2c_readb(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t *reg_data) {
    int ret = 0;
    slv_addr = slv_addr >> 1;

    nrfx_twi_enable(&i2c->inst);
    nrfx_twi_xfer_desc_t desc1 = NRFX_TWI_XFER_DESC_TX(slv_addr, &reg_addr, 1);
    if (nrfx_twi_xfer(&i2c->inst, &desc1, 0) != NRFX_SUCCESS) {
        ret = -1;
        goto i2c_error;
    }

    nrfx_twi_xfer_desc_t desc2 = NRFX_TWI_XFER_DESC_RX(slv_addr, reg_data, 1);
    if (nrfx_twi_xfer(&i2c->inst, &desc2, 0) != NRFX_SUCCESS) {
        ret = -1;
    }

i2c_error:
    nrfx_twi_disable(&i2c->inst);
    return ret;
}

int omv_i2c_writeb(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data) {
    int ret = 0;
    slv_addr = slv_addr >> 1;

    nrfx_twi_enable(&i2c->inst);
    nrfx_twi_xfer_desc_t desc1 = NRFX_TWI_XFER_DESC_TX(slv_addr, &reg_addr, 1);
    if (nrfx_twi_xfer(&i2c->inst, &desc1, NRFX_TWI_FLAG_SUSPEND) != NRFX_SUCCESS) {
        ret = -1;
        goto i2c_error;
    }

    nrfx_twi_xfer_desc_t desc2 = NRFX_TWI_XFER_DESC_TX(slv_addr, &reg_data, 1);
    if (nrfx_twi_xfer(&i2c->inst, &desc2, 0) != NRFX_SUCCESS) {
        ret = -1;
    }

i2c_error:
    nrfx_twi_disable(&i2c->inst);
    return ret;
}

int omv_i2c_read_bytes(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags) {
    int ret = 0;
    slv_addr = slv_addr >> 1;
    uint32_t xfer_flags = 0;
    if (flags & OMV_I2C_XFER_SUSPEND) {
        xfer_flags |= NRFX_TWI_FLAG_SUSPEND;
    }

    nrfx_twi_xfer_desc_t desc = NRFX_TWI_XFER_DESC_RX(slv_addr, buf, len);
    if (nrfx_twi_xfer(&i2c->inst, &desc, xfer_flags) != NRFX_SUCCESS) {
        ret = -1;
    }
    return ret;
}

int omv_i2c_write_bytes(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags) {
    int ret = 0;
    slv_addr = slv_addr >> 1;
    uint32_t xfer_flags = 0;
    if (flags & OMV_I2C_XFER_NO_STOP) {
        xfer_flags |= NRFX_TWI_FLAG_TX_NO_STOP;
    } else if (flags & OMV_I2C_XFER_SUSPEND) {
        xfer_flags |= NRFX_TWI_FLAG_SUSPEND;
    }

    nrfx_twi_xfer_desc_t desc = NRFX_TWI_XFER_DESC_TX(slv_addr, buf, len);
    if (nrfx_twi_xfer(&i2c->inst, &desc, xfer_flags) != NRFX_SUCCESS) {
        ret = -1;
    }
    return ret;
}

int omv_i2c_pulse_scl(omv_i2c_t *i2c) {
    omv_i2c_deinit(i2c);
    for (int i = 0; i < 10000; i++) {
        nrfx_twi_bus_recover(i2c->scl_pin, i2c->sda_pin);
    }
    return 0;
}

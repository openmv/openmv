/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Cambus driver nRF port.
 */
#include <string.h>
#include <stdbool.h>
#include "py/mphal.h"

#include "omv_boardconfig.h"
#include "cambus.h"
#include "common.h"

#define I2C_TIMEOUT             (1000)
#define I2C_SCAN_TIMEOUT        (100)

int cambus_init(cambus_t *bus, uint32_t bus_id, uint32_t speed)
{
    bus->id = bus_id;
    bus->initialized = false;

    switch (speed) {
        case CAMBUS_SPEED_STANDARD:
            bus->speed = TWI_FREQUENCY_FREQUENCY_K100; ///< 100 kbps
            break;
        case CAMBUS_SPEED_FULL:
            bus->speed = TWI_FREQUENCY_FREQUENCY_K250; ///< 250 kbps
            break;
        case CAMBUS_SPEED_FAST:
            bus->speed = TWI_FREQUENCY_FREQUENCY_K400;  ///< 400 kbps
            break;
        default:
            return -1;
    }

    switch (bus_id) {
        case 0: {
            bus->scl_pin = TWI0_SCL_PIN;
            bus->sda_pin = TWI0_SDA_PIN;
            bus->i2c = (nrfx_twi_t) NRFX_TWI_INSTANCE(0);
            break;
        }
        case 1: {
            bus->scl_pin = TWI1_SCL_PIN;
            bus->sda_pin = TWI1_SDA_PIN;
            bus->i2c = (nrfx_twi_t) NRFX_TWI_INSTANCE(1);
            break;
        }
        default:
            return -1;
    }

    nrfx_twi_config_t config = {
       .scl                = bus->scl_pin,
       .sda                = bus->sda_pin,
       .frequency          = bus->speed,
       .interrupt_priority = 4,
       .hold_bus_uninit    = false
    };

    if (nrfx_twi_init(&bus->i2c, &config, NULL, NULL) != NRFX_SUCCESS) {
        return -1;
    }

    // This bus needs to be enabled for suspended transfers.
    nrfx_twi_enable(&bus->i2c);

    bus->initialized = true;
    return 0;
}

int cambus_deinit(cambus_t *bus)
{
    if (bus->initialized) {
        nrfx_twi_disable(&bus->i2c);
        nrfx_twi_uninit(&bus->i2c);
        bus->initialized = false;
    }
    return 0;
}

int cambus_scan(cambus_t *bus)
{
    uint8_t data;
    uint32_t xfer_flags = 0;
    for (uint8_t addr=0x09; addr<=0x77; addr++) {
        nrfx_twi_xfer_desc_t desc = NRFX_TWI_XFER_DESC_RX(addr, &data, 1);
        if (nrfx_twi_xfer(&bus->i2c, &desc, xfer_flags) == NRFX_SUCCESS) {
            return (addr << 1);
        }
    }
    return 0;
}

int cambus_enable(cambus_t *bus, bool enable)
{
    if (bus->initialized) {
        if (enable) {
            nrfx_twi_enable(&bus->i2c);
        } else {
            nrfx_twi_disable(&bus->i2c);
        }
    }
    return 0;
}

int cambus_gencall(cambus_t *bus, uint8_t cmd)
{
    uint32_t xfer_flags = 0;
    nrfx_twi_xfer_desc_t desc = NRFX_TWI_XFER_DESC_TX(0x00, &cmd, 1);
    if (nrfx_twi_xfer(&bus->i2c, &desc, xfer_flags) != NRFX_SUCCESS) {
        return -1;
    }
    return 0;
}

int cambus_readb(cambus_t *bus, uint8_t slv_addr, uint8_t reg_addr, uint8_t *reg_data)
{
    int ret = 0;
    slv_addr = slv_addr >> 1;

    nrfx_twi_enable(&bus->i2c);
    nrfx_twi_xfer_desc_t desc1 = NRFX_TWI_XFER_DESC_TX(slv_addr, &reg_addr, 1);
    if (nrfx_twi_xfer(&bus->i2c, &desc1, 0) != NRFX_SUCCESS) {
        ret = -1;
        goto i2c_error;
    }

    nrfx_twi_xfer_desc_t desc2 = NRFX_TWI_XFER_DESC_RX(slv_addr, reg_data, 1);
    if (nrfx_twi_xfer(&bus->i2c, &desc2, 0) != NRFX_SUCCESS) {
        ret = -1;
    }

i2c_error:
    nrfx_twi_disable(&bus->i2c);
    return ret;
}

int cambus_writeb(cambus_t *bus, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data)
{
    int ret = 0;
    slv_addr = slv_addr >> 1;

    nrfx_twi_enable(&bus->i2c);
    nrfx_twi_xfer_desc_t desc1 = NRFX_TWI_XFER_DESC_TX(slv_addr, &reg_addr, 1);
    if (nrfx_twi_xfer(&bus->i2c, &desc1, NRFX_TWI_FLAG_SUSPEND) != NRFX_SUCCESS) {
        ret = -1;
        goto i2c_error;
    }

    nrfx_twi_xfer_desc_t desc2 = NRFX_TWI_XFER_DESC_TX(slv_addr, &reg_data, 1);
    if (nrfx_twi_xfer(&bus->i2c, &desc2, 0) != NRFX_SUCCESS) {
        ret = -1;
    }

i2c_error:
    nrfx_twi_disable(&bus->i2c);
    return ret;
}

int cambus_read_bytes(cambus_t *bus, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags)
{
    int ret = 0;
    slv_addr = slv_addr >> 1;
    uint32_t xfer_flags = 0;
    if (flags & CAMBUS_XFER_SUSPEND) {
        xfer_flags |= NRFX_TWI_FLAG_SUSPEND;
    }

    nrfx_twi_xfer_desc_t desc = NRFX_TWI_XFER_DESC_RX(slv_addr, buf, len);
    if (nrfx_twi_xfer(&bus->i2c, &desc, xfer_flags) != NRFX_SUCCESS) {
        ret = -1;
    }
    return ret;
}

int cambus_write_bytes(cambus_t *bus, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags)
{
    int ret = 0;
    slv_addr = slv_addr >> 1;
    uint32_t xfer_flags = 0;
    if (flags & CAMBUS_XFER_NO_STOP) {
        xfer_flags |= NRFX_TWI_FLAG_TX_NO_STOP;
    } else if (flags & CAMBUS_XFER_SUSPEND) {
        xfer_flags |= NRFX_TWI_FLAG_SUSPEND;
    }

    nrfx_twi_xfer_desc_t desc = NRFX_TWI_XFER_DESC_TX(slv_addr, buf, len);
    if (nrfx_twi_xfer(&bus->i2c, &desc, xfer_flags) != NRFX_SUCCESS) {
        ret = -1;
    }
    return ret;
}

int cambus_pulse_scl(cambus_t *bus)
{
    cambus_deinit(bus);
    for (int i=0; i<10000; i++) {
        nrfx_twi_bus_recover(bus->scl_pin, bus->sda_pin);
    }
    return 0;
}

/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
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
    bus->speed = speed;
    bus->scl_pin = FIR_I2C_SCL_PIN;
    bus->sda_pin = FIR_I2C_SDA_PIN;
    bus->initialized = false;

    switch (bus_id) {
        case 0: {
            nrfx_twi_t _twi = NRFX_TWI_INSTANCE(0);
            memcpy(&bus->twi, &_twi, sizeof(nrfx_twi_t));
            break;
        }
        case 1: {
            nrfx_twi_t _twi = NRFX_TWI_INSTANCE(1);
            memcpy(&bus->twi, &_twi, sizeof(nrfx_twi_t));
            break;
        }
        default:
            return -1;
    }

    nrfx_twi_config_t config = {
       .scl                = FIR_I2C_SCL_PIN,
       .sda                = FIR_I2C_SDA_PIN,
       .frequency          = speed,
       .interrupt_priority = 4,
       .hold_bus_uninit    = false
    };

    if (nrfx_twi_init(&bus->twi, &config, NULL, NULL) != NRFX_SUCCESS) {
        return -1;
    }

    bus->initialized = true;
    return 0;
}

int cambus_deinit(cambus_t *bus)
{
    if (bus->initialized) {
        nrfx_twi_uninit(&bus->twi);
        bus->initialized = false;
    }
    return 0;
}

int cambus_scan(cambus_t *bus)
{
    return 0;
}

int cambus_gencall(cambus_t *bus, uint8_t cmd)
{
    return 0;
}

int cambus_read_bytes(cambus_t *bus, uint8_t slv_addr, uint8_t reg_addr, uint8_t *buf, int len)
{
    int ret = 0;
    slv_addr = slv_addr >> 1;

    nrfx_twi_enable(&bus->twi);
    nrfx_twi_xfer_desc_t desc1 = NRFX_TWI_XFER_DESC_TX(slv_addr, &reg_addr, 1);
    if (nrfx_twi_xfer(&bus->twi, &desc1, NRFX_TWI_FLAG_TX_NO_STOP) != NRFX_SUCCESS) {
        ret = -1;
        goto i2c_error;
    }

    nrfx_twi_xfer_desc_t desc2 = NRFX_TWI_XFER_DESC_RX(slv_addr, buf, len);
    if (nrfx_twi_xfer(&bus->twi, &desc2, 0) != NRFX_SUCCESS) {
        ret = -1;
    }

i2c_error:
    nrfx_twi_disable(&bus->twi);
    return ret;
}

int cambus_write_bytes(cambus_t *bus, uint8_t slv_addr, uint8_t reg_addr, uint8_t *buf, int len)
{
    int ret = 0;
    slv_addr = slv_addr >> 1;

    nrfx_twi_enable(&bus->twi);
    nrfx_twi_xfer_desc_t desc1 = NRFX_TWI_XFER_DESC_TX(slv_addr, &reg_addr, 1);
    if (nrfx_twi_xfer(&bus->twi, &desc1, NRFX_TWI_FLAG_SUSPEND) != NRFX_SUCCESS) {
        ret = -1;
        goto i2c_error;
    }

    nrfx_twi_xfer_desc_t desc2 = NRFX_TWI_XFER_DESC_TX(slv_addr, buf, len);
    if (nrfx_twi_xfer(&bus->twi, &desc2, 0) != NRFX_SUCCESS) {
        ret = -1;
    }

i2c_error:
    nrfx_twi_disable(&bus->twi);
    return ret;
}

int cambus_readw_bytes(cambus_t *bus, uint8_t slv_addr, uint16_t reg_addr, uint8_t *buf, int len)
{
    return 0;
}

int cambus_writew_bytes(cambus_t *bus, uint8_t slv_addr, uint16_t reg_addr, uint8_t *buf, int len)
{
    return 0;
}

int cambus_read_bytes_seq(cambus_t *bus, uint8_t slv_addr, uint8_t *buf, int len, bool nostop)
{
    int ret = 0;
    slv_addr = slv_addr >> 1;
    nrfx_twi_enable(&bus->twi);
    nrfx_twi_xfer_desc_t desc = NRFX_TWI_XFER_DESC_RX(slv_addr, buf, len);
    if (nrfx_twi_xfer(&bus->twi, &desc, 0) != NRFX_SUCCESS) {
        ret = -1;
    }
    nrfx_twi_disable(&bus->twi);
    return ret;
}

int cambus_write_bytes_seq(cambus_t *bus, uint8_t slv_addr, uint8_t *buf, int len, bool nostop)
{
    int ret = 0;
    slv_addr = slv_addr >> 1;
    nrfx_twi_enable(&bus->twi);
    nrfx_twi_xfer_desc_t desc = NRFX_TWI_XFER_DESC_TX(slv_addr, buf, len);
    if (nrfx_twi_xfer(&bus->twi, &desc, (nostop == true) ? NRFX_TWI_FLAG_TX_NO_STOP:0) != NRFX_SUCCESS) {
        ret = -1;
    }
    nrfx_twi_disable(&bus->twi);
    return ret;
}

int cambus_pulse_scl(cambus_t *bus)
{
    for (int i=0; i<10000; i++) {
        cambus_deinit(bus);
        nrfx_twi_bus_recover(bus->scl_pin, bus->sda_pin);
    }
    return 0;
}

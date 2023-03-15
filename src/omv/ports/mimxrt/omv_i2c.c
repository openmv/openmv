/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * I2C port for mimxrt.
 */

#include <stdio.h>
#include <stdbool.h>
#include "py/mphal.h"

#include "fsl_gpio.h"
#include "fsl_lpi2c.h"
#include "fsl_iomuxc.h"
#include CLOCK_CONFIG_H

#include "omv_portconfig.h"
#include "omv_boardconfig.h"
#include "mimxrt_hal.h"
#include "omv_gpio.h"
#include "omv_i2c.h"

#define I2C_TIMEOUT             (3*1000)
#define I2C_SCAN_TIMEOUT        (1*1000)

typedef struct {
    volatile uint32_t flags;
    volatile status_t error;
} lpi2c_transfer_status_t;

typedef enum {
    LPI2C_TRANSFER_COMPLETE = (1 << 0),
    LPI2C_TRANSFER_ERROR    = (1 << 1),
} lpi2c_xfer_flags_t;

int omv_i2c_init(omv_i2c_t *i2c, uint32_t bus_id, uint32_t speed)
{
    i2c->id = bus_id;
    i2c->initialized = false;

    switch (speed) {
        case OMV_I2C_SPEED_STANDARD:
            i2c->speed = 100 * 1000;    ///< 100 kbps
            break;
        case OMV_I2C_SPEED_FULL:
            i2c->speed = 400 * 1000;    ///< 400 kbps
            break;
        case OMV_I2C_SPEED_FAST:
            i2c->speed = 1000 * 1000;   ///< 1000 kbps
            break;
        default:
            return -1;
    }

    switch (bus_id) {
        case LPI2C1_ID: {
            i2c->inst = LPI2C1;
            i2c->scl_pin = LPI2C1_SCL_PIN;
            i2c->sda_pin = LPI2C1_SDA_PIN;
            break;
        }
        #if defined(LPI2C2_ID)
        case LPI2C2_ID: {
            i2c->inst = LPI2C2;
            i2c->scl_pin = LPI2C2_SCL_PIN;
            i2c->sda_pin = LPI2C2_SDA_PIN;
            break;
        }
        #endif
        #if defined(LPI2C3_ID)
        case LPI2C3_ID: {
            i2c->inst = LPI2C3;
            i2c->scl_pin = LPI2C3_SCL_PIN;
            i2c->sda_pin = LPI2C3_SDA_PIN;
            break;
        }
        #endif
        #if defined(LPI2C4_ID)
        case LPI2C4_ID: {
            i2c->inst = LPI2C4;
            i2c->scl_pin = LPI2C4_SCL_PIN;
            i2c->sda_pin = LPI2C4_SDA_PIN;
            break;
        }
        #endif
        default:
            return -1;
    }

    lpi2c_master_config_t lpi2c_config = {0};
    LPI2C_MasterGetDefaultConfig(&lpi2c_config);

    lpi2c_config.ignoreAck = true;
    lpi2c_config.baudRate_Hz = i2c->speed;

    mimxrt_hal_i2c_init(bus_id);
    LPI2C_MasterInit(i2c->inst, &lpi2c_config, BOARD_BOOTCLOCKRUN_LPI2C_CLK_ROOT);
    i2c->initialized = true;
    return 0;
}

int omv_i2c_deinit(omv_i2c_t *i2c)
{
    if (i2c->initialized) {
        // TODO
        i2c->initialized = false;
    }
    return 0;
}

int omv_i2c_scan(omv_i2c_t *i2c, uint8_t *list, uint8_t size)
{
    uint32_t idx = 0;
    lpi2c_master_transfer_t xfer = {0};
    xfer.direction      = kLPI2C_Write;
    xfer.flags          = kLPI2C_TransferDefaultFlag;

    for (uint8_t addr=0x20; addr<0x78; addr++) {
        xfer.slaveAddress = addr;
        if (LPI2C_MasterTransferBlocking(i2c->inst, &xfer) == kStatus_Success) {
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

int omv_i2c_enable(omv_i2c_t *i2c, bool enable)
{
    return 0;
}

int omv_i2c_gencall(omv_i2c_t *i2c, uint8_t cmd)
{
    int ret = 0;
    ret |= omv_i2c_write_bytes(i2c, 0, &cmd, 1, OMV_I2C_XFER_NO_FLAGS);
    return ret;

}

int omv_i2c_readb(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr,  uint8_t *reg_data)
{
    int ret = 0;
    ret |= omv_i2c_write_bytes(i2c, slv_addr, &reg_addr, 1, OMV_I2C_XFER_NO_FLAGS);
    ret |= omv_i2c_read_bytes(i2c, slv_addr, reg_data, 1, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

int omv_i2c_writeb(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data)
{
    int ret = 0;
    uint8_t buf[] = {reg_addr, reg_data};
    ret |= omv_i2c_write_bytes(i2c, slv_addr, buf, 2, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

int omv_i2c_readb2(omv_i2c_t *i2c, uint8_t slv_addr, uint16_t reg_addr, uint8_t *reg_data)
{
    int ret = 0;
    uint8_t buf[] = {(reg_addr >> 8), reg_addr};
    ret |= omv_i2c_write_bytes(i2c, slv_addr, buf, 2, OMV_I2C_XFER_NO_STOP);
    ret |= omv_i2c_read_bytes(i2c, slv_addr, reg_data, 1, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

int omv_i2c_writeb2(omv_i2c_t *i2c, uint8_t slv_addr, uint16_t reg_addr, uint8_t reg_data)
{
    int ret = 0;
    uint8_t buf[] = {(reg_addr >> 8), reg_addr, reg_data};
    ret |= omv_i2c_write_bytes(i2c, slv_addr, buf, 3, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

int omv_i2c_readw(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr, uint16_t *reg_data)
{
    int ret = 0;
    ret |= omv_i2c_write_bytes(i2c, slv_addr, &reg_addr, 1, OMV_I2C_XFER_NO_STOP);
    ret |= omv_i2c_read_bytes(i2c, slv_addr, (uint8_t *) reg_data, 2, OMV_I2C_XFER_NO_FLAGS);
    *reg_data = (*reg_data << 8) | (*reg_data >> 8);
    return ret;
}

int omv_i2c_writew(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr, uint16_t reg_data)
{
    int ret = 0;
    uint8_t buf[] = {reg_addr, (reg_data >> 8), reg_data};
    ret |= omv_i2c_write_bytes(i2c, slv_addr, buf, 3, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

int omv_i2c_readw2(omv_i2c_t *i2c, uint8_t slv_addr, uint16_t reg_addr, uint16_t *reg_data)
{
    int ret = 0;
    uint8_t buf[] = {(reg_addr >> 8), reg_addr};
    ret |= omv_i2c_write_bytes(i2c, slv_addr, buf, 2, OMV_I2C_XFER_NO_STOP);
    ret |= omv_i2c_read_bytes(i2c, slv_addr, (uint8_t *) reg_data, 2, OMV_I2C_XFER_NO_FLAGS);
    *reg_data = (*reg_data << 8) | (*reg_data >> 8);
    return ret;
}

int omv_i2c_writew2(omv_i2c_t *i2c, uint8_t slv_addr, uint16_t reg_addr, uint16_t reg_data)
{
    int ret = 0;
    uint8_t buf[] = {(reg_addr >> 8), reg_addr, (reg_data >> 8), reg_data};
    ret |= omv_i2c_write_bytes(i2c, slv_addr, buf, 4, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

static void lpi2c_transfer_callback(LPI2C_Type *base,
        lpi2c_master_handle_t *handle, status_t status, void *data)
{
    lpi2c_transfer_status_t *xfer_status = (lpi2c_transfer_status_t *) data;
    xfer_status->flags |= LPI2C_TRANSFER_COMPLETE;
    if (status != kStatus_Success) {
        xfer_status->flags |= LPI2C_TRANSFER_ERROR;
        xfer_status->error = status;
    }
}

static int omv_i2c_transfer_timeout(omv_i2c_t *i2c, lpi2c_master_transfer_t *transfer)
{
    lpi2c_master_handle_t lpi2c_handle;
    lpi2c_transfer_status_t xfer_status = {0};

    // Create the handle, needed for non-blocking mode.
    LPI2C_MasterTransferCreateHandle(i2c->inst, &lpi2c_handle, lpi2c_transfer_callback, &xfer_status);

    // Start non-blocking transfer.
    if (LPI2C_MasterTransferNonBlocking(i2c->inst, &lpi2c_handle, transfer) != kStatus_Success) {
        return -1;
    }

    // Wait for the transfer to finish.
    mp_uint_t tick_start = mp_hal_ticks_ms();
    while (!(xfer_status.flags & LPI2C_TRANSFER_COMPLETE)) {
        if ((mp_hal_ticks_ms() - tick_start) >= I2C_TIMEOUT) {
            xfer_status.flags |= LPI2C_TRANSFER_ERROR;
            break;
        }
        MICROPY_EVENT_POLL_HOOK
    }

    // Terminate non-blocking transfer.
    if (xfer_status.flags & LPI2C_TRANSFER_ERROR) {
        LPI2C_MasterTransferAbort(i2c->inst, &lpi2c_handle);
        return -1;
    }

    return 0;
}

int omv_i2c_read_bytes(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags)
{
    lpi2c_master_transfer_t xfer = {
        .data           = buf,
        .dataSize       = len,
        .flags          = kLPI2C_TransferDefaultFlag,
        .direction      = kLPI2C_Read,
        .subaddress     = 0,
        .subaddressSize = 0,
        .slaveAddress   = (slv_addr >> 1)
    };
    if (flags & (OMV_I2C_XFER_NO_STOP | OMV_I2C_XFER_SUSPEND)) {
        xfer.flags |= kLPI2C_TransferNoStopFlag;
    }
    return omv_i2c_transfer_timeout(i2c, &xfer);
}

int omv_i2c_write_bytes(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags)
{
    lpi2c_master_transfer_t xfer = {
        .data           = buf,
        .dataSize       = len,
        .flags          = kLPI2C_TransferDefaultFlag,
        .direction      = kLPI2C_Write,
        .subaddress     = 0,
        .subaddressSize = 0,
        .slaveAddress   = (slv_addr >> 1)
    };
    if (flags & (OMV_I2C_XFER_NO_STOP | OMV_I2C_XFER_SUSPEND)) {
        xfer.flags |= kLPI2C_TransferNoStopFlag;
    }
    return omv_i2c_transfer_timeout(i2c, &xfer);
}

int omv_i2c_pulse_scl(omv_i2c_t *i2c)
{
    if (i2c->initialized && i2c->scl_pin) {
        omv_i2c_deinit(i2c);
        omv_gpio_config(i2c->scl_pin, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        // Pulse SCL to recover stuck device.
        for (int i=0; i<10000; i++) {
            omv_gpio_write(i2c->scl_pin, 1);
            mp_hal_delay_us(10);
            omv_gpio_write(i2c->scl_pin, 0);
            mp_hal_delay_us(10);
        }
    }
    return 0;
}

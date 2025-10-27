/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alif I3C driver.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "py/mphal.h"
#include "py/runtime.h"

#include "omv_portconfig.h"
#include "omv_boardconfig.h"
#include "alif_hal.h"
#include "omv_gpio.h"
#include "omv_common.h"
#include "omv_i3c.h"
#include "sys_ctrl_i3c.h"

#define I3C_SCAN_TIMEOUT    (10)
#define I3C_XFER_TIMEOUT    (1000)

int omv_i3c_init(omv_i2c_t *i3c, uint32_t bus_id, uint32_t speed) {
    i3c->id = bus_id;
    i3c->initialized = false;
    i3c->cw_size = 0;

    switch (bus_id) {
        #if defined(OMV_I3C0_ID)
        case OMV_I3C0_ID: {
            i3c->inst = (void *) I3C_BASE;
            i3c->scl_pin = OMV_I3C0_SCL_PIN;
            i3c->sda_pin = OMV_I3C0_SDA_PIN;
            break;
        }
        #endif

        default:
            return -1;
    }

    alif_hal_i3c_init(bus_id);

    // I3C controller in non-legacy mode.
    I3C_Type *base = i3c->inst;

    // Core soft-reset.
    base->I3C_RESET_CTRL = 0x1;
    while (base->I3C_RESET_CTRL & 0x1) {
    }

    i3c_master_set_dynamic_addr(base);

    i3c_master_enable_interrupts(base);

    /* Sets Slave Interrupt Request acceptability at master side */
    i3c_master_setup_slv_intr_req_ctrl(base, false);

    /* Sets Master Request acceptability at master side */
    i3c_master_setup_mst_req_ctrl(base, false);

    /* Sets up HJ acceptability at master side */
    i3c_master_setup_hot_join_ctrl(base, false);

    // Initialize I3C controller
    i3c_master_init(base);

    // Configure clock and speed
    if (omv_i3c_set_scl(i3c, speed) != 0) {
        return -1;
    }

    i3c->initialized = true;
    return 0;
}

int omv_i3c_deinit(omv_i2c_t *i3c) {
    if (i3c->initialized) {
        // TODO
        i3c->initialized = false;
    }
    return 0;
}

int omv_i3c_set_scl(omv_i2c_t *i3c, uint32_t speed) {
    if (!i3c->initialized || speed != i3c->speed) {
        switch (speed) {
            case OMV_I2C_SPEED_STANDARD:
                speed = I3C_I2C_SPEED_MODE_SS_100_KBPS;    // 100 kbit/s
                break;
            case OMV_I2C_SPEED_FULL:
                speed = I3C_I2C_SPEED_MODE_FM_400_KBPS;   // 400 kbit/s
                break;
            case OMV_I2C_SPEED_FAST:
                speed = I3C_I2C_SPEED_MODE_FMP_1_MBPS;    // 1000 kbit/s
                break;
            case OMV_I3C_SPEED_SDR:
                speed = I3C_BUS_SDR0_SCL_RATE;      // 12.5 Mbit/s
                break;
            default:
                return -1;
        }

        I3C_Type *base = i3c->inst;
        // Configure clock and speed
        if (speed == I3C_BUS_SDR0_SCL_RATE) {
            i3c_normal_bus_clk_cfg(base, speed);
        } else {
            i2c_clk_cfg(base, SystemAPBClock, speed);
        }

        i3c->speed = speed;
    }

    return 0;
}

static int omv_i3c_get_addr_pos(omv_i2c_t *i3c, uint8_t addr) {
    size_t pos;

    for (pos = 0; pos < i3c->cw_size; pos++) {
        if (addr == i3c->cw_buf[pos]) {
            return (int) pos;
        }
    }

    return -1;
}

static void omv_i3c_add_dyn_addr_parity(uint8_t *dyn_addr) {
    uint8_t bit_iter = 0U;
    uint8_t xor_value = 0U;

    /* XOR the 1st 7 bits of dynamic address*/
    xor_value = ((*dyn_addr) & (1U << 0U));

    for (bit_iter = 1U; bit_iter < 7U; bit_iter++) {
        xor_value ^= (((*dyn_addr) >> bit_iter) & 1U);
    }

    /* Assign the negated XOR value to 7th
     * bit of dynamic address */
    *dyn_addr |= ((~xor_value) << 7U);
}

static int32_t omv_i3c_gen_dyn_addr(omv_i2c_t *i3c, uint8_t *ref_dyn_addr) {
    if (i3c->cw_size >= I3C_MAX_DEVS) {
        return -1;
    }

    /* we start assigning addresses from 0x09 */
    *ref_dyn_addr = (uint8_t) i3c->cw_size + I3C_NEXT_SLAVE_ADDR_OFFSET;
    while (true) {
        /* Checks if new address is not a self address and
         * not already assigned to some slave. If true then,
         * assign it otherwise increment it */
        if ((omv_i3c_get_addr_pos(i3c, *ref_dyn_addr) < 0) && (i3c_get_dynamic_addr(i3c->inst) != *ref_dyn_addr)) {
            break;
        }

        (*ref_dyn_addr)++;
    }

    i3c->cw_buf[i3c->cw_size] = *ref_dyn_addr;
    // i3c->cw_buf[i3c->cw_size] &= (~I3C_TARGET_SLAVE_TYPE_I2C);

    return (int32_t) i3c->cw_size++;
}

static int omv_i3c_transfer_wait(I3C_Type *base, i3c_xfer_t *xfer, uint32_t timeout) {
    // Wait for the transfer to finish.
    mp_uint_t tick_start = mp_hal_ticks_ms();
    /* Waits till some response received */
    while (!(base->I3C_QUEUE_STATUS_LEVEL & I3C_QUEUE_STATUS_LEVEL_RESP_BUF_BLR_Msk)) {
        if ((mp_hal_ticks_ms() - tick_start) >= timeout) {
            // Should not raise exception as we're not always in nlr context.
            return -1;
        }
        mp_event_handle_nowait();
    }

    // See Table 15-81 Response Data Structure
    uint32_t resp = base->I3C_RESPONSE_QUEUE_PORT;

    if ((I3C_RESPONSE_QUEUE_PORT_ERR_STATUS(resp)) ||
        (I3C_RESPONSE_QUEUE_PORT_TID(resp) != xfer->xfer_cmd.port_id)) {
        return -1;
    }

    return 0;
}

static void omv_i3c_transfer_end(I3C_Type *base) {
    i3c_clear_xfer_error(base);
    i3c_resume(base);
}

static int omv_i3c_send_command(I3C_Type *base, uint8_t type, uint8_t cmd_id, uint8_t addr_index, uint8_t addr_depth) {
    i3c_xfer_t xfer = {0};

    xfer.xfer_cmd.cmd_id = cmd_id;
    xfer.xfer_cmd.addr_index = addr_index;
    xfer.xfer_cmd.addr_depth = addr_depth;

    xfer.error = 0U;

    xfer.rx_len = 0U;
    xfer.xfer_cmd.cmd_type = type;
    xfer.xfer_cmd.def_byte = 0U;
    xfer.xfer_cmd.data_len = 0U;

    i3c_send_xfer_cmd(base, &xfer);
    if (omv_i3c_transfer_wait(base, &xfer, I3C_SCAN_TIMEOUT) != 0) {
        omv_i3c_transfer_end(base);
        return -1;
    }

    return 0;
}

int omv_i3c_assign(omv_i2c_t *i3c, uint8_t static_addr, uint8_t *ref_dyn_addr) {
    int32_t pos = 0U;
    I3C_Type *base = i3c->inst;
    /*Returns error if slave static address is invalid */
    if (!static_addr) {
        return -1;
    }

    /* Find the first unused index in freepos, note that this also
     * corresponds to the first unused location in the DAT
     */
    pos = omv_i3c_gen_dyn_addr(i3c, ref_dyn_addr);

    /* the dat is full */
    if (pos < 0) {
        return -1;
    }

    /* We have space in the dat,
     * program the dat in index pos */
    i3c_add_slv_to_dat(base, pos, *ref_dyn_addr, static_addr);

    if (omv_i3c_send_command(base, I3C_XFER_TYPE_ADDR_ASSIGN, I3C_CCC_SETDASA, (uint8_t) pos, 1U) != 0) {
        return -1;
    }

    return 0;
}

int omv_i3c_scan_assign(omv_i2c_t *i3c, uint8_t *list, uint8_t size) {
    int32_t pos = 0U;
    uint8_t init_pos = 0xFFU;
    I3C_Type *base = i3c->inst;
    uint8_t dyn_addr = 0U;
    uint8_t iter = 0U;

    for (iter = 0U; iter < size; iter++) {
        pos = omv_i3c_gen_dyn_addr(i3c, &dyn_addr);

        /* the dat is full */
        if (pos < 0) {
            if (init_pos == 0xFFU) {
                return -1;
            } else{
                break;
            }
        } else {
            if (pos >= size) {
                break;
            }

            list[pos] = dyn_addr;

            omv_i3c_add_dyn_addr_parity(&dyn_addr);

            /* We have space in the dat,
             * program the dat in index pos */
            i3c_add_slv_to_dat(base, pos, dyn_addr, 0);
        }

        /* Stores first found free address position in
         * init position*/
        if (init_pos == 0xFFU) {
            init_pos = (uint8_t) pos;
        }
    }

    if (omv_i3c_send_command(base, I3C_XFER_TYPE_ADDR_ASSIGN, I3C_CCC_ENTDAA, init_pos, pos + 1) != 0) {
        return -1;
    }

    return 0;
}

int omv_i3c_enable(omv_i2c_t *i3c, bool enable) {
    //TODO: For I3C this causes a lockup.
    I3C_Type *base = i3c->inst;
    if (enable) {
        base->I3C_DEVICE_CTRL = base->I3C_DEVICE_CTRL & ~I3C_DEVICE_CTRL_ENABLE;
        while (base->I3C_DEVICE_CTRL & I3C_DEVICE_CTRL_ENABLE) {
        }

    } else {
        base->I3C_DEVICE_CTRL = base->I3C_DEVICE_CTRL | I3C_DEVICE_CTRL_ENABLE;
        while (!(base->I3C_DEVICE_CTRL & I3C_DEVICE_CTRL_ENABLE)) {
        }
        i3c_resume(base);
    }

    return 0;
}

int omv_i3c_reset(omv_i2c_t *i3c, uint8_t tgt_addr) {
    int32_t pos = 0U;
    uint8_t num_devs = I3C_MAX_DEVS;

    uint8_t command = I3C_CCC_RSTDAA(true);

    if (tgt_addr != 0x7E) {
        if (tgt_addr < 0x29 || tgt_addr >= 0x78) {
            return -1;
        }
        pos = omv_i3c_get_addr_pos(i3c, tgt_addr);
        if (pos < 0) {
            return -1;
        }
        command = I3C_CCC_RSTDAA(false);
        num_devs = 1U;
    }

    if (omv_i3c_send_command(i3c->inst, I3C_XFER_CCC_SET, command, (uint8_t) pos, num_devs) != 0) {
        return -1;
    }

    return 0;
}

int omv_i3c_readb(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr,  uint8_t *reg_data) {
    int ret = 0;
    ret |= omv_i3c_write_bytes(i3c, tgt_addr, &reg_addr, 1, OMV_I2C_XFER_NO_STOP);
    ret |= omv_i3c_read_bytes(i3c, tgt_addr, reg_data, 1, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

int omv_i3c_writeb(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr, uint8_t reg_data) {
    int ret = 0;
    uint8_t buf[] = {reg_addr, reg_data};
    ret |= omv_i3c_write_bytes(i3c, tgt_addr, buf, 2, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

int omv_i3c_readb2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr, uint8_t *reg_data) {
    int ret = 0;
    uint8_t buf[] = {(reg_addr >> 8), reg_addr};
    ret |= omv_i3c_write_bytes(i3c, tgt_addr, buf, 2, OMV_I2C_XFER_NO_STOP);
    ret |= omv_i3c_read_bytes(i3c, tgt_addr, reg_data, 1, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

int omv_i3c_writeb2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr, uint8_t reg_data) {
    int ret = 0;
    uint8_t buf[] = {(reg_addr >> 8), reg_addr, reg_data};
    ret |= omv_i3c_write_bytes(i3c, tgt_addr, buf, 3, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

int omv_i3c_readw(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr, uint16_t *reg_data) {
    int ret = 0;
    ret |= omv_i3c_write_bytes(i3c, tgt_addr, &reg_addr, 1, OMV_I2C_XFER_NO_STOP);
    ret |= omv_i3c_read_bytes(i3c, tgt_addr, (uint8_t *) reg_data, 2, OMV_I2C_XFER_NO_FLAGS);
    *reg_data = (*reg_data << 8) | (*reg_data >> 8);
    return ret;
}

int omv_i3c_writew(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr, uint16_t reg_data) {
    int ret = 0;
    uint8_t buf[] = {reg_addr, (reg_data >> 8), reg_data};
    ret |= omv_i3c_write_bytes(i3c, tgt_addr, buf, 3, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

int omv_i3c_readw2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr, uint16_t *reg_data) {
    int ret = 0;
    uint8_t buf[] = {(reg_addr >> 8), reg_addr};
    ret |= omv_i3c_write_bytes(i3c, tgt_addr, buf, 2, OMV_I2C_XFER_NO_STOP);
    ret |= omv_i3c_read_bytes(i3c, tgt_addr, (uint8_t *) reg_data, 2, OMV_I2C_XFER_NO_FLAGS);
    *reg_data = (*reg_data << 8) | (*reg_data >> 8);
    return ret;
}

int omv_i3c_writew2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr, uint16_t reg_data) {
    int ret = 0;
    uint8_t buf[] = {(reg_addr >> 8), reg_addr, (reg_data >> 8), reg_data};
    ret |= omv_i3c_write_bytes(i3c, tgt_addr, buf, 4, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

int omv_i3c_readdw(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr, uint32_t *reg_data) {
    int ret = 0;
    uint8_t data_1, data_2, data_3, data_4;
    ret |= omv_i3c_write_bytes(i3c, tgt_addr, &reg_addr, 1, OMV_I2C_XFER_NO_STOP);
    ret |= omv_i3c_read_bytes(i3c, tgt_addr, (uint8_t *) reg_data, 4, OMV_I2C_XFER_NO_FLAGS);
    data_1 = (*reg_data) & 0xFF;
    data_2 = (*reg_data) & 0xFF00;
    data_3 = (*reg_data) & 0xFF0000;
    data_4 = (*reg_data) & 0xFF000000;
    *reg_data = (data_1 << 24) | (data_2 << 8) | (data_3 >> 8) | (data_4 >> 24);
    return ret;
}

int omv_i3c_writedw(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr, uint32_t reg_data) {
    int ret = 0;
    uint8_t buf[] = {reg_addr, (reg_data >> 24), (reg_data >> 16), (reg_data >> 8), reg_data};
    ret |= omv_i3c_write_bytes(i3c, tgt_addr, buf, 5, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

int omv_i3c_readdw2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr, uint32_t *reg_data) {
    int ret = 0;
    uint8_t data_1, data_2, data_3, data_4;
    uint8_t buf[] = {(reg_addr >> 8), reg_addr};
    ret |= omv_i3c_write_bytes(i3c, tgt_addr, buf, 2, OMV_I2C_XFER_NO_STOP);
    ret |= omv_i3c_read_bytes(i3c, tgt_addr, (uint8_t *) reg_data, 4, OMV_I2C_XFER_NO_FLAGS);
    data_1 = (*reg_data) & 0xFF;
    data_2 = (*reg_data) & 0xFF00;
    data_3 = (*reg_data) & 0xFF0000;
    data_4 = (*reg_data) & 0xFF000000;
    *reg_data = (data_1 << 24) | (data_2 << 8) | (data_3 >> 8) | (data_4 >> 24);
    return ret;
}

int omv_i3c_writedw2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr, uint32_t reg_data) {
    int ret = 0;
    uint8_t buf[] = {(reg_addr >> 8), reg_addr, (reg_data >> 24), (reg_data >> 16), (reg_data >> 8), reg_data};
    ret |= omv_i3c_write_bytes(i3c, tgt_addr, buf, 6, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

static int omv_i3c_check_transfer(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t *buf, int len) {
    if (!buf || !len) {
        return -1;
    }

    if (len > I3C_MAX_DATA_BUF_SIZE) {
        return -1;
    }

    return omv_i3c_get_addr_pos(i3c, tgt_addr);
}

int omv_i3c_read_bytes(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t *buf, int len, uint32_t flags) {
    int32_t pos;
    i3c_xfer_t xfer = {0};
    I3C_Type *base = (I3C_Type *) i3c->inst;

    pos = omv_i3c_check_transfer(i3c, tgt_addr, buf, len);
    if (pos < 0) {
        return -1;
    }

    xfer.error = 0U;
    xfer.tx_buf = NULL;
    xfer.tx_len = 0U;
    xfer.xfer_cmd.addr_index = pos;
    xfer.xfer_cmd.data_len = len;
    xfer.rx_len = len;

    xfer.rx_buf = buf;

    /* Invoke master receive api */
    if (flags & (OMV_I2C_XFER_NO_STOP | OMV_I2C_XFER_SUSPEND)) {
        i3c_master_rx_blocking(base, &xfer);
    } else {
        i3c_master_rx(base, &xfer);
    }

    if (omv_i3c_transfer_wait(base, &xfer, I3C_XFER_TIMEOUT) != 0) {
        omv_i3c_transfer_end(base);
        return -1;
    }

    // if (xfer.rx_buf) {
    //     for (uint32_t i = 0, dr = 0; i < xfer.rx_len; i++, dr >>= 8) {
    //         if ((i % 4) == 0) {
    //             dr = base->I3C_RX_DATA_PORT;
    //         }
    //         xfer.rx_buf[i] = dr & 0xFF;
    //     }
    // }

    return 0;
}

int omv_i3c_write_bytes(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t *buf, int len, uint32_t flags) {
    int32_t pos;
    i3c_xfer_t xfer = {0};
    I3C_Type *base = (I3C_Type *) i3c->inst;

    pos = omv_i3c_check_transfer(i3c, tgt_addr, buf, len);
    if (pos < 0) {
        return -1;
    }

    xfer.error = 0U;
    xfer.rx_buf = NULL;
    xfer.rx_len = 0U;
    xfer.xfer_cmd.addr_index = pos;
    xfer.xfer_cmd.data_len = len;

    xfer.tx_buf = buf;
    xfer.tx_len = len;

    /* Invoke master send api */
    if (flags & (OMV_I2C_XFER_NO_STOP | OMV_I2C_XFER_SUSPEND)) {
        i3c_master_tx_blocking(base, &xfer);
    } else {
        i3c_master_tx(base, &xfer);
    }

    if (omv_i3c_transfer_wait(base, &xfer, I3C_XFER_TIMEOUT) != 0) {
        omv_i3c_transfer_end(base);
        return -1;
    }

    return 0;
}

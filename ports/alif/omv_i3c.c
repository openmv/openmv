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

/* I3C CCC (Common Command Codes) related definitions */
#define I3C_CCC_DIRECT                                  BIT(7)

#define I3C_CCC_ID(id, broadcast)                       ((id) | ((broadcast) ? 0 : I3C_CCC_DIRECT))

/* Commands valid in both broadcast and unicast modes */
#define I3C_CCC_ENEC(broadcast)                         I3C_CCC_ID(0x0, broadcast)
#define I3C_CCC_DISEC(broadcast)                        I3C_CCC_ID(0x1, broadcast)
#define I3C_CCC_ENTAS(as, broadcast)                    I3C_CCC_ID(0x2 + (as), broadcast)
#define I3C_CCC_RSTDAA(broadcast)                       I3C_CCC_ID(0x6, broadcast)
#define I3C_CCC_SETMWL(broadcast)                       I3C_CCC_ID(0x9, broadcast)
#define I3C_CCC_SETMRL(broadcast)                       I3C_CCC_ID(0xa, broadcast)
#define I3C_CCC_SETXTIME(broadcast)                     ((broadcast) ? 0x28 : 0x98)
#define I3C_CCC_VENDOR(id, broadcast)                   ((id) + ((broadcast) ? 0x61 : 0xe0))

/* Broadcast-only commands */
#define I3C_CCC_ENTDAA                                  I3C_CCC_ID(0x7, true)
#define I3C_CCC_DEFSLVS                                 I3C_CCC_ID(0x8, true)
#define I3C_CCC_ENTTM                                   I3C_CCC_ID(0xb, true)
#define I3C_CCC_ENTHDR(x)                               I3C_CCC_ID(0x20 + (x), true)
#define I3C_CCC_SETAASA                                 I3C_CCC_ID(0x29, true)

/* Unicast-only commands */
#define I3C_CCC_SETDASA                                 I3C_CCC_ID(0x7, false)
#define I3C_CCC_SETNEWDA                                I3C_CCC_ID(0x8, false)
#define I3C_CCC_GETMWL                                  I3C_CCC_ID(0xb, false)
#define I3C_CCC_GETMRL                                  I3C_CCC_ID(0xc, false)
#define I3C_CCC_GETPID                                  I3C_CCC_ID(0xd, false)
#define I3C_CCC_GETBCR                                  I3C_CCC_ID(0xe, false)
#define I3C_CCC_GETDCR                                  I3C_CCC_ID(0xf, false)
#define I3C_CCC_GETSTATUS                               I3C_CCC_ID(0x10, false)
#define I3C_CCC_GETACCMST                               I3C_CCC_ID(0x11, false)
#define I3C_CCC_SETBRGTGT                               I3C_CCC_ID(0x13, false)
#define I3C_CCC_GETMXDS                                 I3C_CCC_ID(0x14, false)
#define I3C_CCC_GETHDRCAP                               I3C_CCC_ID(0x15, false)
#define I3C_CCC_GETXTIME                                I3C_CCC_ID(0x19, false)

/* List of some Defining byte values */
#define I3C_CCC_DEF_BYTE_SYNC_TICK                      0x7F
#define I3C_CCC_DEF_BYTE_DELAY_TIME                     0xBF
#define I3C_CCC_DEF_BYTE_ASYNC_MODE0                    0xDF
#define I3C_CCC_DEF_BYTE_ASYNC_MODE1                    0xEF
#define I3C_CCC_DEF_BYTE_ASYNC_MODE2                    0xF7
#define I3C_CCC_DEF_BYTE_ASYNC_MODE3                    0xFB
#define I3C_CCC_DEF_BYTE_ASYNC_TRIG                     0xFD
#define I3C_CCC_DEF_BYTE_TPH                            0x3F
#define I3C_CCC_DEF_BYTE_TU                             0x9F
#define I3C_CCC_DEF_BYTE_ODR                            0x8F

#ifdef NDEBUG
#define I3C_CHECK_ERRORS(base)                       \
    if (base->I3C_RAW_INTR_STAT & I3C_STAT_ERRORS) { \
        (void) base->I3C_RAW_INTR_STAT;              \
        (void) base->I3C_CLR_TX_ABRT;                \
        return -1;                                   \
    }
#else
#define I3C_CHECK_ERRORS(base)                                                \
    if (base->I3C_RAW_INTR_STAT & I3C_STAT_ERRORS) {                          \
        uint32_t status = base->I3C_RAW_INTR_STAT;                            \
        printf("status: 0x%lx raw_int: 0x%lx abort: 0x%lx line: %d\n",        \
               base->I3C_STATUS, status, base->I3C_TX_ABRT_SOURCE, __LINE__); \
        (void) base->I3C_CLR_TX_ABRT;                                         \
        return -1;                                                            \
    }
#endif

int omv_i3c_init(omv_i2c_t *i3c, uint32_t bus_id, uint32_t speed) {
    i3c->id = bus_id;
    i3c->initialized = false;
    i3c->cw_size = 0;
    i3c->speed = speed;

    switch (speed) {
        case OMV_I3C_SPEED_SDR:
            speed = I3C_BUS_SDR0_SCL_RATE;      // 12.5 Mbit/s
            break;
        case OMV_I3C_SPEED_HDR:
            speed = I3C_BUS_MAX_I3C_SCL_RATE;   // 12.9 kbit/s
            break;
        default:
            return -1;
    }

    switch (bus_id) {
        #if defined(OMV_I3C0_ID)
        case OMV_I3C0_ID: {
            i3c->inst = (I3C_Type *) I3C_BASE;
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

    // Initialize I3C controller
    i3c_master_init(base);

    // Configure clock and speed.
    i3c_normal_bus_clk_cfg(base, speed);

    i3c_master_set_dynamic_addr(base);

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

static int omv_i2c_transfer_wait(I3C_Type *base, uint32_t timeout) {
    // Wait for the transfer to finish.
    mp_uint_t tick_start = mp_hal_ticks_ms();
    while (base->I3C_INTR_STATUS == 0) {
        if ((mp_hal_ticks_ms() - tick_start) >= timeout) {
            // Should not raise exception as we're not always in nlr context.
            return -1;
        }
        mp_event_handle_nowait();
    }

    uint32_t status = base->I3C_INTR_STATUS;
    // See Table 15-81 Response Data Structure
    uint32_t resp = base->I3C_RESPONSE_QUEUE_PORT;

    if ((status & I3C_INTR_STATUS_TRANSFER_ERR_STS) || I3C_RESPONSE_QUEUE_PORT_ERR_STATUS(resp)) {
        return -1;
    }

    return 0;
}

static void omv_i2c_transfer_end(I3C_Type *base) {
    i3c_clear_xfer_error(base);
    i3c_resume(base);
}

static int omv_i3c_send_command_assign(I3C_Type *base, uint8_t cmd_id, uint8_t addr_index, uint8_t addr_depth) {
    i3c_xfer_t xfer = {0};

    xfer.xfer_cmd.cmd_id = cmd_id;
    xfer.xfer_cmd.addr_index = addr_index;
    xfer.xfer_cmd.addr_depth = addr_depth;

    xfer.error = 0U;

    xfer.rx_len = 0U;
    xfer.xfer_cmd.cmd_type = I3C_XFER_TYPE_ADDR_ASSIGN;
    xfer.xfer_cmd.def_byte = 0U;
    xfer.xfer_cmd.data_len = 0U;

    i3c_send_xfer_cmd(base, &xfer);
    if (omv_i2c_transfer_wait(base, I3C_SCAN_TIMEOUT) != 0) {
        omv_i2c_transfer_end(base);
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

    if (omv_i3c_send_command_assign(base, I3C_CCC_SETDASA, (uint8_t) pos, 1U) != 0) {
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

    if (omv_i3c_send_command_assign(base, I3C_CCC_ENTDAA, init_pos, pos + 1) != 0) {
        return -1;
    }

    return 0;
}

// int omv_i2c_scan(omv_i2c_t *i3c, uint8_t *list, uint8_t size) {
//     uint32_t idx = 0;

//     for (uint8_t addr = 0x29, data = 0; addr < 0x78; addr++) {
//         i3c_transfer_t xfer = {
//             .data = &data,
//             .size = 1,
//             .flags = 0,
//             .direction = I3C_TRANSFER_READ,
//             .address = addr
//         };

//         if (omv_ixc_transfer_timeout(i3c, &xfer, I3C_SCAN_TIMEOUT) == 0) {
//             if (list == NULL || size == 0) {
//                 idx = (addr << 1);
//                 break;
//             } else if (idx < size) {
//                 list[idx++] = (addr << 1);
//             } else {
//                 break;
//             }
//         }
//         mp_event_handle_nowait();
//     }

//     return idx;
// }

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

int omv_i3c_gencall(omv_i2c_t *i3c, uint8_t cmd) {
    int ret = 0;
    ret |= omv_i3c_write_bytes(i3c, 0, &cmd, 1, OMV_I2C_XFER_NO_FLAGS);
    return ret;

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

static int omv_i3c_check_transfer(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t *buf, int len) {
    uint8_t addr = tgt_addr >> 1;

    if (!buf || !len) {
        return -1;
    }

    if (len > I3C_MAX_DATA_BUF_SIZE) {
        return -1;
    }

    return omv_i3c_get_addr_pos(i3c, addr);
}

int omv_i3c_read_bytes(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t *buf, int len, uint32_t flags) {
    int32_t index;
    i3c_xfer_t xfer = {0};
    I3C_Type *base = (I3C_Type *) i3c->inst;

    index = omv_i3c_check_transfer(i3c, tgt_addr, buf, len);
    if (index < 0) {
        return -1;
    }

    xfer.error = 0U;
    xfer.tx_buf = NULL;
    xfer.tx_len = 0U;
    xfer.xfer_cmd.addr_index = index;
    xfer.xfer_cmd.data_len = len;
    xfer.rx_len = len;

    xfer.rx_buf = buf;

    /* Invoke master receive api */
    if (flags & (OMV_I2C_XFER_NO_STOP | OMV_I2C_XFER_SUSPEND)) {
        i3c_master_rx_blocking(base, &xfer);
    } else {
        i3c_master_rx(base, &xfer);
        if (omv_i2c_transfer_wait(base, I3C_XFER_TIMEOUT) != 0) {
            omv_i2c_transfer_end(base);
            return -1;
        }
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
    int32_t index;
    i3c_xfer_t xfer = {0};
    I3C_Type *base = (I3C_Type *) i3c->inst;

    index = omv_i3c_check_transfer(i3c, tgt_addr, buf, len);
    if (index < 0) {
        return -1;
    }

    xfer.error = 0U;
    xfer.rx_buf = NULL;
    xfer.rx_len = 0U;
    xfer.xfer_cmd.addr_index = index;
    xfer.xfer_cmd.data_len = len;

    xfer.tx_buf = buf;
    xfer.tx_len = len;

    /* Invoke master send api */
    if (flags & (OMV_I2C_XFER_NO_STOP | OMV_I2C_XFER_SUSPEND)) {
        i3c_master_tx_blocking(base, &xfer);
    } else {
        i3c_master_tx(base, &xfer);
        if (omv_i2c_transfer_wait(base, I3C_XFER_TIMEOUT) != 0) {
            omv_i2c_transfer_end(base);
            return -1;
        }
    }

    return 0;
}

int omv_i3c_pulse_scl(omv_i2c_t *i3c) {
    if (i3c->initialized && i3c->scl_pin) {
        omv_i3c_deinit(i3c);
        omv_gpio_config(i3c->scl_pin, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        // Pulse SCL to recover stuck device.
        for (int i = 0; i < 1000; i++) {
            omv_gpio_write(i3c->scl_pin, 1);
            mp_hal_delay_us(10);
            omv_gpio_write(i3c->scl_pin, 0);
            mp_hal_delay_us(10);
        }
        omv_i3c_init(i3c, i3c->id, i3c->speed);
    }
    return 0;
}

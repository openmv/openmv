/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * I2C port for stm32.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "py/mphal.h"

#include "omv_boardconfig.h"
#include "omv_common.h"
#include "omv_gpio.h"
#include "omv_i2c.h"

#define I2C_TIMEOUT         (1000)
#define I2C_SCAN_TIMEOUT    (100)

// If an I2C handle is already defined in MicroPython, reuse that handle to allow
// MicroPython to process I2C IRQs, otherwise define a new handle and handle IRQs here.
#if defined(I2C1)
#if MICROPY_PY_PYB_LEGACY && defined(MICROPY_HW_I2C1_SCL)
extern I2C_HandleTypeDef I2CHandle1;
#else
static I2C_HandleTypeDef I2CHandle1;
void I2C1_EV_IRQHandler(void) {
    HAL_I2C_EV_IRQHandler(&I2CHandle1);
}
void I2C1_ER_IRQHandler(void) {
    HAL_I2C_ER_IRQHandler(&I2CHandle1);
}
#endif
#endif // I2C1

#if defined(I2C2)
#if MICROPY_PY_PYB_LEGACY && defined(MICROPY_HW_I2C2_SCL)
extern I2C_HandleTypeDef I2CHandle2;
#else
static I2C_HandleTypeDef I2CHandle2;
void I2C2_EV_IRQHandler(void) {
    HAL_I2C_EV_IRQHandler(&I2CHandle2);
}
void I2C2_ER_IRQHandler(void) {
    HAL_I2C_ER_IRQHandler(&I2CHandle2);
}
#endif
#endif // I2C2

#if defined(I2C3)
#if MICROPY_PY_PYB_LEGACY && defined(MICROPY_HW_I2C3_SCL)
extern I2C_HandleTypeDef I2CHandle3;
#else
static I2C_HandleTypeDef I2CHandle3;
void I2C3_EV_IRQHandler(void) {
    HAL_I2C_EV_IRQHandler(&I2CHandle3);
}
void I2C3_ER_IRQHandler(void) {
    HAL_I2C_ER_IRQHandler(&I2CHandle3);
}
#endif
#endif // I2C3

#if defined(I2C4)
#if MICROPY_PY_PYB_LEGACY && defined(MICROPY_HW_I2C4_SCL)
extern I2C_HandleTypeDef I2CHandle4;
#else
static I2C_HandleTypeDef I2CHandle4;
void I2C4_EV_IRQHandler(void) {
    HAL_I2C_EV_IRQHandler(&I2CHandle4);
}
void I2C4_ER_IRQHandler(void) {
    HAL_I2C_ER_IRQHandler(&I2CHandle4);
}
#endif
#endif // I2C4

static const uint32_t omv_i2c_timing[OMV_I2C_SPEED_MAX] = {
#if defined(STM32F4)
    100000U, 400000U, 400000U,
#elif defined(STM32F7)
    0x1090699B, 0x70330309, 0x50100103,
#elif defined(STM32H7)
    0x20D09DE7, 0x40900C22, 0x4030040B,
#elif defined(STM32N6)
    0x10C0ECFF, 0x00602173, 0x00300B29,
#else
#error "no I2C timings for this MCU"
#endif
};

int omv_i2c_init(omv_i2c_t *i2c, uint32_t bus_id, uint32_t speed) {
    i2c->id = bus_id;
    i2c->speed = speed;
    i2c->inst = NULL;
    i2c->initialized = false;
    i2c->scl_pin = NULL;
    i2c->sda_pin = NULL;

    switch (bus_id) {
        #if defined(OMV_I2C1_ID)
        case 1: {
            i2c->inst = &I2CHandle1;
            i2c->inst->Instance = I2C1;
            i2c->scl_pin = OMV_I2C1_SCL_PIN;
            i2c->sda_pin = OMV_I2C1_SDA_PIN;
            break;
        }
        #endif
        #if defined(OMV_I2C2_ID)
        case 2: {
            i2c->inst = &I2CHandle2;
            i2c->inst->Instance = I2C2;
            i2c->scl_pin = OMV_I2C2_SCL_PIN;
            i2c->sda_pin = OMV_I2C2_SDA_PIN;
            break;
        }
        #endif
        #if defined(OMV_I2C3_ID)
        case 3: {
            i2c->inst = &I2CHandle3;
            i2c->inst->Instance = I2C3;
            i2c->scl_pin = OMV_I2C3_SCL_PIN;
            i2c->sda_pin = OMV_I2C3_SDA_PIN;
            break;
        }
        #endif
        #if defined(OMV_I2C4_ID)
        case 4: {
            i2c->inst = &I2CHandle4;
            i2c->inst->Instance = I2C4;
            i2c->scl_pin = OMV_I2C4_SCL_PIN;
            i2c->sda_pin = OMV_I2C4_SDA_PIN;
            break;
        }
        #endif
        default:
            return -1;
    }

    if (speed < 0 || speed >= OMV_I2C_SPEED_MAX) {
        return -1;
    }

    // Configure the I2C handle
    i2c->inst->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    #if !defined(STM32F4)
    i2c->inst->Init.Timing = omv_i2c_timing[speed];
    #else
    i2c->inst->Init.ClockSpeed = omv_i2c_timing[speed];
    i2c->inst->Init.DutyCycle = I2C_DUTYCYCLE_2;
    #endif
    i2c->inst->Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
    i2c->inst->Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    i2c->inst->Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
    i2c->inst->Init.OwnAddress1 = 0xFE;
    i2c->inst->Init.OwnAddress2 = 0xFE;
    #if !defined(STM32F4)
    i2c->inst->Init.OwnAddress2Masks = 0;
    #endif

    HAL_I2C_DeInit(i2c->inst);
    if (HAL_I2C_Init(i2c->inst) != HAL_OK) {
        i2c->inst = NULL;
        i2c->scl_pin = NULL;
        i2c->sda_pin = NULL;
        return -1;
    }

    #if !defined(IS_I2C_FASTMODEPLUS)
    assert_param(speed != OMV_I2C_SPEED_FAST);
    #else
    if (speed == OMV_I2C_SPEED_FAST) {
        #if defined(STM32N6)
        HAL_I2CEx_ConfigFastModePlus(i2c->inst, I2C_FASTMODEPLUS_ENABLE);
        #else
        // Enable FAST mode plus.
        switch (bus_id) {
            #if defined(I2C_FASTMODEPLUS_I2C1)
            case 1:
                HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
                break;
            #endif
            #if defined(I2C_FASTMODEPLUS_I2C2)
            case 2:
                HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C2);
                break;
            #endif
            #if defined(I2C_FASTMODEPLUS_I2C3)
            case 3:
                HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C3);
                break;
            #endif
            #if defined(I2C_FASTMODEPLUS_I2C4)
            case 4:
                HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C4);
                break;
            #endif
        }
        #endif  // defined(STM32N6)
    }
    #endif // !defined(IS_I2C_FASTMODEPLUS)

    i2c->initialized = true;
    return 0;
}

static int omv_i2c_set_irq_state(omv_i2c_t *i2c, bool enabled) {
    IRQn_Type ev_irqn;
    IRQn_Type er_irqn;
    switch (i2c->id) {
        #if defined(OMV_I2C1_ID)
        case 1: {
            ev_irqn = I2C1_EV_IRQn;
            er_irqn = I2C1_ER_IRQn;
            break;
        }
        #endif
        #if defined(OMV_I2C2_ID)
        case 2: {
            ev_irqn = I2C2_EV_IRQn;
            er_irqn = I2C2_ER_IRQn;
            break;
        }
        #endif
        #if defined(OMV_I2C3_ID)
        case 3: {
            ev_irqn = I2C3_EV_IRQn;
            er_irqn = I2C3_ER_IRQn;
            break;
        }
        #endif
        #if defined(OMV_I2C4_ID)
        case 4: {
            ev_irqn = I2C4_EV_IRQn;
            er_irqn = I2C4_ER_IRQn;
            break;
        }
        #endif
        default:
            return -1;
    }

    if (enabled) {
        HAL_NVIC_EnableIRQ(ev_irqn);
        HAL_NVIC_EnableIRQ(er_irqn);
    } else {
        HAL_NVIC_DisableIRQ(ev_irqn);
        HAL_NVIC_DisableIRQ(er_irqn);
    }
    return 0;
}

static int omv_i2c_wait_timeout(omv_i2c_t *i2c, uint32_t timeout) {
    mp_uint_t tick_start;
    tick_start = mp_hal_ticks_ms();
    while (HAL_I2C_GetState(i2c->inst) != HAL_I2C_STATE_READY) {
        if ((mp_hal_ticks_ms() - tick_start) >= I2C_TIMEOUT) {
            return -1;
        }
        __WFI();
    }
    return 0;
}

int omv_i2c_deinit(omv_i2c_t *i2c) {
    if (i2c->initialized) {
        HAL_I2C_DeInit(i2c->inst);
        i2c->inst->Instance = NULL;
    }
    i2c->inst = NULL;
    i2c->scl_pin = NULL;
    i2c->sda_pin = NULL;
    i2c->initialized = false;
    return 0;
}

int omv_i2c_scan(omv_i2c_t *i2c, uint8_t *list, uint8_t size) {
    int idx = 0;
    for (uint8_t addr = 0x09; addr <= 0x77; addr++) {
        if (HAL_I2C_IsDeviceReady(i2c->inst, addr << 1, 1, I2C_SCAN_TIMEOUT) == HAL_OK) {
            if (list == NULL || size == 0) {
                return (addr << 1);
            } else if (idx < size) {
                list[idx++] = (addr << 1);
            } else {
                break;
            }
        }
    }

    #if defined(STM32H7) || defined(STM32N6)
    // After a failed scan the bus can get stuck. Re-initializing the bus fixes
    // it, but it seems disabling and re-enabling the bus is all that's needed.
    if (idx == 0) {
        __HAL_I2C_DISABLE(i2c->inst);
        mp_hal_delay_ms(10);
        __HAL_I2C_ENABLE(i2c->inst);
    }
    #endif
    return idx;
}

static int omv_i2c_reset(omv_i2c_t *i2c) {
    __HAL_I2C_DISABLE(i2c->inst);
    mp_hal_delay_ms(10);
    __HAL_I2C_ENABLE(i2c->inst);
    i2c->inst->ErrorCode = 0;
    i2c->inst->State = HAL_I2C_STATE_READY;
    return 0;
}

int omv_i2c_enable(omv_i2c_t *i2c, bool enable) {
    if (i2c->initialized) {
        if (enable) {
            __HAL_I2C_ENABLE(i2c->inst);
        } else {
            __HAL_I2C_DISABLE(i2c->inst);
        }
    }
    return 0;
}

int omv_i2c_gencall(omv_i2c_t *i2c, uint8_t cmd) {
    if (HAL_I2C_Master_Transmit(i2c->inst, 0x00, &cmd, 1, I2C_TIMEOUT) != HAL_OK) {
        omv_i2c_reset(i2c);
        return -1;
    }
    return 0;
}

int omv_i2c_read(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags) {
    int ret = 0;
    uint32_t xfer_flags = 0;
    if (flags & OMV_I2C_XFER_NO_STOP) {
        xfer_flags |= I2C_FIRST_FRAME;
    } else if (flags & OMV_I2C_XFER_SUSPEND) {
        xfer_flags |= I2C_NEXT_FRAME;
    } else {
        xfer_flags |= I2C_LAST_FRAME;
    }

    omv_i2c_set_irq_state(i2c, true);

    if (HAL_I2C_Master_Seq_Receive_IT(i2c->inst, slv_addr, buf, len, xfer_flags) != HAL_OK
        || omv_i2c_wait_timeout(i2c, I2C_TIMEOUT) != 0) {
        omv_i2c_reset(i2c);
        ret = -1;
    }

    omv_i2c_set_irq_state(i2c, false);
    return ret;
}

int omv_i2c_write(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags) {
    int ret = 0;
    uint32_t xfer_flags = 0;
    if (flags & OMV_I2C_XFER_NO_STOP) {
        xfer_flags |= I2C_FIRST_FRAME;
    } else if (flags & OMV_I2C_XFER_SUSPEND) {
        xfer_flags |= I2C_NEXT_FRAME;
    } else {
        xfer_flags |= I2C_LAST_FRAME;
    }

    omv_i2c_set_irq_state(i2c, true);

    if (HAL_I2C_Master_Seq_Transmit_IT(i2c->inst, slv_addr, buf, len, xfer_flags) != HAL_OK
        || omv_i2c_wait_timeout(i2c, I2C_TIMEOUT) != 0) {
        omv_i2c_reset(i2c);
        ret = -1;
    }

    omv_i2c_set_irq_state(i2c, false);
    return ret;
}

int omv_i2c_pulse_scl(omv_i2c_t *i2c) {
    if (i2c->initialized && i2c->scl_pin) {
        // Configure SCL as GPIO
        omv_gpio_config(i2c->scl_pin, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);

        // Pulse SCL to recover stuck device.
        for (int i = 0; i < 10000; i++) {
            omv_gpio_write(i2c->scl_pin, 1);
            mp_hal_delay_us(10);
            omv_gpio_write(i2c->scl_pin, 0);
            mp_hal_delay_us(10);
        }

        // Clear ARLO flag if it's set.
        __HAL_I2C_CLEAR_FLAG(i2c->inst, I2C_FLAG_ARLO);
        debug_printf("reset stuck i2c device\n");
    }
    return 0;
}

int omv_i2c_read_sccb(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t *data) {
    // SCCB protocol requires STOP between write and read phases.
    if (HAL_I2C_Master_Transmit(i2c->inst, slv_addr, &reg_addr, 1, I2C_TIMEOUT) != HAL_OK) {
        omv_i2c_reset(i2c);
        return -1;
    }

    if (HAL_I2C_Master_Receive(i2c->inst, slv_addr, data, 1, I2C_TIMEOUT) != HAL_OK) {
        omv_i2c_reset(i2c);
        return -1;
    }

    return 0;
}

int omv_i2c_write_sccb(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t data) {
    uint8_t buf[] = {reg_addr, data};
    if (HAL_I2C_Master_Transmit(i2c->inst, slv_addr, buf, 2, I2C_TIMEOUT) != HAL_OK) {
        omv_i2c_reset(i2c);
        return -1;
    }
    return 0;
}

int omv_i2c_read_reg(omv_i2c_t *i2c, uint8_t slv_addr,
                     uint32_t reg_addr, uint8_t addr_size,
                     void *data, uint8_t data_size) {
    uint16_t mem_addr_size = (addr_size == 1) ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT;

    uint8_t data_buf[4] = {0};
    if (HAL_I2C_Mem_Read(i2c->inst, slv_addr, reg_addr, mem_addr_size,
                         data_buf, data_size, I2C_TIMEOUT) != HAL_OK) {
        omv_i2c_reset(i2c);
        return -1;
    }

    // Convert from big-endian to host byte order
    uint32_t value = 0;
    for (int i = 0; i < data_size; i++) {
        value = (value << 8) | data_buf[i];
    }

    switch (data_size) {
        case 1:
            *(uint8_t *) data = value;
            break;
        case 2:
            *(uint16_t *) data = value;
            break;
        case 4:
            *(uint32_t *) data = value;
            break;
    }

    return 0;
}

int omv_i2c_write_reg(omv_i2c_t *i2c, uint8_t slv_addr,
                      uint32_t reg_addr, uint8_t addr_size,
                      uint32_t data, uint8_t data_size) {
    uint16_t mem_addr_size = (addr_size == 1) ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT;

    // Serialize data to big-endian
    uint8_t data_buf[4];
    for (int i = 0; i < data_size; i++) {
        data_buf[i] = (data >> (8 * (data_size - 1 - i))) & 0xFF;
    }

    if (HAL_I2C_Mem_Write(i2c->inst, slv_addr, reg_addr, mem_addr_size,
                          data_buf, data_size, I2C_TIMEOUT) != HAL_OK) {
        omv_i2c_reset(i2c);
        return -1;
    }

    return 0;
}

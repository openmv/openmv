/*
 * Copyright (C) 2026 OpenMV, LLC.
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
 * STM32 I3C driver.
 */
#if defined(STM32N6)

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "py/mphal.h"

#include "omv_portconfig.h"
#include "omv_boardconfig.h"
#include "omv_gpio.h"
#include "omv_common.h"
#include "omv_i3c.h"

#include "stm32n6xx_hal.h"
#include "stm32n6xx_util_i3c.h"

#define I3C_SCAN_TIMEOUT        (10)
#define I3C_XFER_TIMEOUT        (1000)
#define I3C_FIRST_DYN_ADDR      (0x09)

// Static I3C handles and IRQ handlers.
#if defined(OMV_I3C1_ID)
static I3C_HandleTypeDef i3c1_handle;
void I3C1_EV_IRQHandler(void) {
    HAL_I3C_EV_IRQHandler(&i3c1_handle);
}
void I3C1_ER_IRQHandler(void) {
    HAL_I3C_ER_IRQHandler(&i3c1_handle);
}
#endif

#if defined(OMV_I3C2_ID)
static I3C_HandleTypeDef i3c2_handle;
void I3C2_EV_IRQHandler(void) {
    HAL_I3C_EV_IRQHandler(&i3c2_handle);
}
void I3C2_ER_IRQHandler(void) {
    HAL_I3C_ER_IRQHandler(&i3c2_handle);
}
#endif

// Dynamic address assignment state for ENTDAA.
static uint8_t *daa_list;
static uint8_t daa_count;
static uint8_t daa_max;
static uint8_t dyn_addr_next;

static I3C_HandleTypeDef *omv_i3c_handle(omv_i2c_t *i3c) {
    return (I3C_HandleTypeDef *) i3c->inst;
}

static int omv_i3c_set_irq_state(omv_i2c_t *i3c, bool enabled) {
    IRQn_Type ev_irqn;
    IRQn_Type er_irqn;
    switch (i3c->id) {
        #if defined(OMV_I3C1_ID)
        case OMV_I3C1_ID: {
            ev_irqn = I3C1_EV_IRQn;
            er_irqn = I3C1_ER_IRQn;
            break;
        }
        #endif
        #if defined(OMV_I3C2_ID)
        case OMV_I3C2_ID: {
            ev_irqn = I3C2_EV_IRQn;
            er_irqn = I3C2_ER_IRQn;
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

static int omv_i3c_wait_ready(I3C_HandleTypeDef *base, uint32_t timeout) {
    mp_uint_t tick_start = mp_hal_ticks_ms();
    while (HAL_I3C_GetState(base) != HAL_I3C_STATE_READY) {
        if ((mp_hal_ticks_ms() - tick_start) >= timeout) {
            return -1;
        }
        __WFI();
    }
    return 0;
}

int omv_i3c_init(omv_i2c_t *i3c, uint32_t bus_id, uint32_t speed) {
    I3C_HandleTypeDef *base = NULL;
    I3C_FifoConfTypeDef fifo_config = {0};
    I3C_CtrlConfTypeDef ctrl_config = {0};

    i3c->id = bus_id;
    i3c->initialized = false;

    switch (bus_id) {
        #if defined(OMV_I3C1_ID)
        case OMV_I3C1_ID: {
            base = &i3c1_handle;
            base->Instance = I3C1;
            i3c->scl_pin = OMV_I3C1_SCL_PIN;
            i3c->sda_pin = OMV_I3C1_SDA_PIN;
            break;
        }
        #endif
        #if defined(OMV_I3C2_ID)
        case OMV_I3C2_ID: {
            base = &i3c2_handle;
            base->Instance = I3C2;
            i3c->scl_pin = OMV_I3C2_SCL_PIN;
            i3c->sda_pin = OMV_I3C2_SDA_PIN;
            break;
        }
        #endif
        default:
            return -1;
    }

    // Configure as I3C controller with default bus characteristics.
    // These will be overwritten by omv_i3c_set_scl below.
    base->Mode = HAL_I3C_MODE_CONTROLLER;
    base->Init.CtrlBusCharacteristic.SDAHoldTime = HAL_I3C_SDA_HOLD_TIME_1_5;
    base->Init.CtrlBusCharacteristic.WaitTime = HAL_I3C_OWN_ACTIVITY_STATE_0;
    base->Init.CtrlBusCharacteristic.SCLPPLowDuration = 0x7C;
    base->Init.CtrlBusCharacteristic.SCLI3CHighDuration = 0x7C;
    base->Init.CtrlBusCharacteristic.SCLODLowDuration = 0x7C;
    base->Init.CtrlBusCharacteristic.SCLI2CHighDuration = 0x00;
    base->Init.CtrlBusCharacteristic.BusFreeDuration = 0x0C;
    base->Init.CtrlBusCharacteristic.BusIdleDuration = 0xF8;

    HAL_I3C_DeInit(base);
    if (HAL_I3C_Init(base) != HAL_OK) {
        return -1;
    }

    fifo_config.RxFifoThreshold = HAL_I3C_RXFIFO_THRESHOLD_1_4;
    fifo_config.TxFifoThreshold = HAL_I3C_TXFIFO_THRESHOLD_1_4;
    fifo_config.ControlFifo = HAL_I3C_CONTROLFIFO_ENABLE;
    fifo_config.StatusFifo = HAL_I3C_STATUSFIFO_ENABLE;
    if (HAL_I3C_SetConfigFifo(base, &fifo_config) != HAL_OK) {
        return -1;
    }

    ctrl_config.DynamicAddr = 0;
    ctrl_config.StallTime = 0x00;
    ctrl_config.HotJoinAllowed = DISABLE;
    ctrl_config.ACKStallState = DISABLE;
    ctrl_config.CCCStallState = DISABLE;
    ctrl_config.TxStallState = DISABLE;
    ctrl_config.RxStallState = DISABLE;
    ctrl_config.HighKeeperSDA = DISABLE;
    if (HAL_I3C_Ctrl_Config(base, &ctrl_config) != HAL_OK) {
        return -1;
    }

    i3c->inst = (omv_i2c_dev_t) base;
    dyn_addr_next = I3C_FIRST_DYN_ADDR;

    if (omv_i3c_set_scl(i3c, speed) != 0) {
        return -1;
    }

    i3c->initialized = true;
    return 0;
}

int omv_i3c_deinit(omv_i2c_t *i3c) {
    if (i3c->initialized) {
        HAL_I3C_DeInit(omv_i3c_handle(i3c));
    }
    i3c->inst = NULL;
    i3c->scl_pin = NULL;
    i3c->sda_pin = NULL;
    i3c->initialized = false;
    return 0;
}

int omv_i3c_set_scl(omv_i2c_t *i3c, uint32_t speed) {
    I3C_HandleTypeDef *base = omv_i3c_handle(i3c);
    I3C_CtrlTimingTypeDef ctrl_timing = {0};
    LL_I3C_CtrlBusConfTypeDef ctrl_bus_config = {0};

    if (!i3c->initialized || speed != i3c->speed) {
        uint32_t periph_clk = 0;
        #if defined(OMV_I3C1_ID)
        if (base == &i3c1_handle) {
            periph_clk = RCC_PERIPHCLK_I3C1;
        }
        #endif
        #if defined(OMV_I3C2_ID)
        if (base == &i3c2_handle) {
            periph_clk = RCC_PERIPHCLK_I3C2;
        }
        #endif
        if (periph_clk == 0) {
            return -1;
        }

        ctrl_timing.clockSrcFreq = HAL_RCCEx_GetPeriphCLKFreq(periph_clk);
        ctrl_timing.dutyCycle = 50;

        switch (speed) {
            case OMV_I2C_SPEED_STANDARD:
                ctrl_timing.i3cPPFreq = 12500000;
                ctrl_timing.i2cODFreq = 100000;
                ctrl_timing.busType = I3C_MIXED_BUS;
                break;
            case OMV_I2C_SPEED_FULL:
                ctrl_timing.i3cPPFreq = 12500000;
                ctrl_timing.i2cODFreq = 400000;
                ctrl_timing.busType = I3C_MIXED_BUS;
                break;
            case OMV_I2C_SPEED_FAST:
                ctrl_timing.i3cPPFreq = 12500000;
                ctrl_timing.i2cODFreq = 1000000;
                ctrl_timing.busType = I3C_MIXED_BUS;
                break;
            case OMV_I3C_SPEED_SDR:
                ctrl_timing.i3cPPFreq = 12500000;
                ctrl_timing.i2cODFreq = 0;
                ctrl_timing.busType = I3C_PURE_I3C_BUS;
                break;
            default:
                return -1;
        }

        I3C_CtrlTimingComputation(&ctrl_timing, &ctrl_bus_config);
        HAL_I3C_Ctrl_BusCharacteristicConfig(base, &ctrl_bus_config);

        i3c->speed = speed;
    }

    return 0;
}

int omv_i3c_assign(omv_i2c_t *i3c, uint8_t static_addr, uint8_t *ref_dyn_addr) {
    I3C_HandleTypeDef *base = omv_i3c_handle(i3c);

    if (!static_addr) {
        return -1;
    }

    uint8_t dyn_addr = dyn_addr_next++;
    uint8_t dasa_data = dyn_addr << 1;

    I3C_CCCTypeDef ccc_desc = {
        static_addr, I3C_CCC_SETDASA, {&dasa_data, 1}, HAL_I3C_DIRECTION_WRITE
    };

    uint32_t ctrl_buf[2];
    uint8_t tx_buf[1];
    I3C_XferTypeDef xfer = {0};
    xfer.CtrlBuf.pBuffer = ctrl_buf;
    xfer.CtrlBuf.Size = ARRAY_SIZE(ctrl_buf);
    xfer.TxBuf.pBuffer = tx_buf;
    xfer.TxBuf.Size = sizeof(tx_buf);

    if (HAL_I3C_AddDescToFrame(base, &ccc_desc, NULL, &xfer,
                               1, I3C_DIRECT_WITHOUT_DEFBYTE_STOP) != HAL_OK) {
        return -1;
    }

    if (HAL_I3C_Ctrl_TransmitCCC(base, &xfer, I3C_XFER_TIMEOUT) != HAL_OK) {
        return -1;
    }

    if (HAL_I3C_Ctrl_IsDeviceI3C_Ready(base, dyn_addr,
                                       300, I3C_XFER_TIMEOUT) != HAL_OK) {
        return -1;
    }

    *ref_dyn_addr = dyn_addr;
    return 0;
}

void HAL_I3C_TgtReqDynamicAddrCallback(I3C_HandleTypeDef *hi3c, uint64_t targetPayload) {
    if (daa_count < daa_max) {
        uint8_t dyn_addr = dyn_addr_next++;
        HAL_I3C_Ctrl_SetDynAddr(hi3c, dyn_addr);
        if (daa_list) {
            daa_list[daa_count] = dyn_addr;
        }
        daa_count++;
    }
}

int omv_i3c_scan_assign(omv_i2c_t *i3c, uint8_t *list, uint8_t size) {
    I3C_HandleTypeDef *base = omv_i3c_handle(i3c);
    uint64_t target_payload;

    daa_list = list;
    daa_count = 0;
    daa_max = size;

    if (HAL_I3C_Ctrl_DynAddrAssign(base, &target_payload,
                                   I3C_RSTDAA_THEN_ENTDAA,
                                   I3C_XFER_TIMEOUT) != HAL_OK) {
        return -1;
    }

    return 0;
}

int omv_i3c_enable(omv_i2c_t *i3c, bool enable) {
    I3C_HandleTypeDef *base = omv_i3c_handle(i3c);
    if (enable) {
        if (HAL_I3C_Init(base) != HAL_OK) {
            return -1;
        }
    } else {
        if (HAL_I3C_DeInit(base) != HAL_OK) {
            return -1;
        }
    }
    return 0;
}

int omv_i3c_reset(omv_i2c_t *i3c, uint8_t tgt_addr) {
    I3C_HandleTypeDef *base = omv_i3c_handle(i3c);
    uint8_t command;
    uint32_t option;

    if (tgt_addr == 0x7E) {
        command = I3C_CCC_RSTDAA(true);
        option = I3C_BROADCAST_WITHOUT_DEFBYTE_STOP;
    } else {
        if (tgt_addr < 0x09 || tgt_addr >= 0x78) {
            return -1;
        }
        command = I3C_CCC_RSTDAA(false);
        option = I3C_DIRECT_WITHOUT_DEFBYTE_STOP;
    }

    I3C_CCCTypeDef ccc_desc = {
        tgt_addr, command, {NULL, 0}, HAL_I3C_DIRECTION_WRITE
    };

    uint32_t ctrl_buf[2];
    I3C_XferTypeDef xfer = {0};
    xfer.CtrlBuf.pBuffer = ctrl_buf;
    xfer.CtrlBuf.Size = ARRAY_SIZE(ctrl_buf);

    if (HAL_I3C_AddDescToFrame(base, &ccc_desc, NULL, &xfer, 1, option) != HAL_OK) {
        return -1;
    }

    if (HAL_I3C_Ctrl_TransmitCCC(base, &xfer, I3C_XFER_TIMEOUT) != HAL_OK) {
        return -1;
    }

    return 0;
}

int omv_i3c_read(omv_i2c_t *i3c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags) {
    I3C_HandleTypeDef *base = omv_i3c_handle(i3c);

    I3C_PrivateTypeDef desc = {
        slv_addr, {NULL, 0}, {buf, len}, HAL_I3C_DIRECTION_READ
    };

    uint32_t ctrl_buf[2];
    I3C_XferTypeDef xfer = {0};
    xfer.CtrlBuf.pBuffer = ctrl_buf;
    xfer.CtrlBuf.Size = ARRAY_SIZE(ctrl_buf);
    xfer.RxBuf.pBuffer = buf;
    xfer.RxBuf.Size = len;

    if (HAL_I3C_AddDescToFrame(base, NULL, &desc, &xfer,
                               1, I3C_PRIVATE_WITH_ARB_STOP) != HAL_OK) {
        return -1;
    }

    if (HAL_I3C_Ctrl_Receive(base, &xfer, I3C_XFER_TIMEOUT) != HAL_OK) {
        return -1;
    }

    return 0;
}

int omv_i3c_write(omv_i2c_t *i3c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags) {
    I3C_HandleTypeDef *base = omv_i3c_handle(i3c);

    I3C_PrivateTypeDef desc = {
        slv_addr, {buf, len}, {NULL, 0}, HAL_I3C_DIRECTION_WRITE
    };

    uint32_t ctrl_buf[2];
    I3C_XferTypeDef xfer = {0};
    xfer.CtrlBuf.pBuffer = ctrl_buf;
    xfer.CtrlBuf.Size = ARRAY_SIZE(ctrl_buf);
    xfer.TxBuf.pBuffer = buf;
    xfer.TxBuf.Size = len;

    if (HAL_I3C_AddDescToFrame(base, NULL, &desc, &xfer,
                               1, I3C_PRIVATE_WITH_ARB_STOP) != HAL_OK) {
        return -1;
    }

    if (HAL_I3C_Ctrl_Transmit(base, &xfer, I3C_XFER_TIMEOUT) != HAL_OK) {
        return -1;
    }

    return 0;
}

int omv_i3c_read_reg(omv_i2c_t *i3c, uint8_t slv_addr,
                     uint32_t reg_addr, uint8_t addr_size,
                     void *data, uint8_t data_size) {
    int ret = 0;
    I3C_HandleTypeDef *base = omv_i3c_handle(i3c);

    // Serialize register address (big-endian).
    uint8_t addr_buf[4];
    for (int i = 0; i < addr_size; i++) {
        addr_buf[i] = (reg_addr >> (8 * (addr_size - 1 - i))) & 0xFF;
    }

    uint8_t data_buf[4] = {0};

    // Two-frame transfer: write register address, restart, read data.
    I3C_PrivateTypeDef desc[2] = {
        {slv_addr, {addr_buf, addr_size}, {NULL, 0}, HAL_I3C_DIRECTION_WRITE},
        {slv_addr, {NULL, 0}, {data_buf, data_size}, HAL_I3C_DIRECTION_READ},
    };

    uint32_t ctrl_buf[2];
    I3C_XferTypeDef xfer = {0};
    xfer.CtrlBuf.pBuffer = ctrl_buf;
    xfer.CtrlBuf.Size = ARRAY_SIZE(ctrl_buf);
    xfer.TxBuf.pBuffer = addr_buf;
    xfer.TxBuf.Size = addr_size;
    xfer.RxBuf.pBuffer = data_buf;
    xfer.RxBuf.Size = data_size;

    if (HAL_I3C_AddDescToFrame(base, NULL, desc, &xfer,
                               2, I3C_PRIVATE_WITH_ARB_RESTART) != HAL_OK) {
        return -1;
    }

    omv_i3c_set_irq_state(i3c, true);

    if (HAL_I3C_Ctrl_MultipleTransfer_IT(base, &xfer) != HAL_OK
        || omv_i3c_wait_ready(base, I3C_XFER_TIMEOUT) != 0) {
        ret = -1;
    }

    omv_i3c_set_irq_state(i3c, false);

    if (ret != 0) {
        return ret;
    }

    // Convert from big-endian to host byte order.
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

int omv_i3c_write_reg(omv_i2c_t *i3c, uint8_t slv_addr,
                      uint32_t reg_addr, uint8_t addr_size,
                      uint32_t data, uint8_t data_size) {
    uint8_t buf[8];
    int idx = 0;

    // Serialize register address (big-endian).
    for (int i = 0; i < addr_size; i++) {
        buf[idx++] = (reg_addr >> (8 * (addr_size - 1 - i))) & 0xFF;
    }

    // Serialize data (big-endian).
    for (int i = 0; i < data_size; i++) {
        buf[idx++] = (data >> (8 * (data_size - 1 - i))) & 0xFF;
    }

    return omv_i3c_write(i3c, slv_addr, buf, idx, OMV_I2C_XFER_NO_FLAGS);
}

#endif // STM32N6

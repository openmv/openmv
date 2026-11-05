/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
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
 * STM32 I3C controller driver.
 */
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "py/mphal.h"
#include "irq.h"

#include "board_config.h"
#include "omv_common.h"
#include "omv_i3c.h"
#include "stm_dma.h"

#if defined(OMV_I3C1_ID) || defined(OMV_I3C2_ID)
#include "stm32n6xx_util_i3c.h"

#define I3C_TIMEOUT         (1000)
#define I3C_SCAN_TIMEOUT    (10)
#define I3C_MAX_MSGS        (4)
#define I3C_MAX_DEVS        (4)
#define I3C_TX_BUF_SIZE     (32)
#define I3C_DYN_ADDR_FIRST  (0x08)
#define I3C_DYN_ADDR_LAST   (0x3D)

#ifndef OMV_I3C_PP_FREQUENCY
#define OMV_I3C_PP_FREQUENCY    (12500000)
#endif

typedef struct {
    I3C_XferTypeDef xfer;
    I3C_PrivateTypeDef descs[I3C_MAX_MSGS];
    size_t count;
    uint32_t option;
    uint8_t *rx_buf;
    uint32_t rx_len;
} i3c_frame_t;

typedef struct {
    I3C_HandleTypeDef hi3c;
    LL_I3C_CtrlBusConfTypeDef daa_timing;
    omv_i2c_t *bus;
    volatile bool busy;
    volatile int status;
    // Blocking transfers.
    i3c_frame_t sync;
    uint32_t sync_ctrl[I3C_MAX_MSGS];
    uint8_t sync_tx[I3C_TX_BUF_SIZE];
    // Asynchronous transfers.
    i3c_frame_t async;
    uint32_t *async_ctrl;
    uint8_t *async_tx;
    volatile bool async_pending;
    bool async_active;
    omv_i3c_callback_t callback;
    void *arg;
    DMA_HandleTypeDef dma_cr;
    DMA_HandleTypeDef dma_tx;
    DMA_HandleTypeDef dma_rx;
} i3c_state_t;

#if defined(OMV_I3C1_ID)
static i3c_state_t i3c1_state;
static uint32_t OMV_ATTR_SEC_ALIGN(i3c1_dma_ctrl[I3C_MAX_MSGS], ".dma_buffer", OMV_DMA_ALIGNMENT);
static uint8_t OMV_ATTR_SEC_ALIGN(i3c1_dma_tx[I3C_TX_BUF_SIZE], ".dma_buffer", OMV_DMA_ALIGNMENT);

void I3C1_EV_IRQHandler(void) {
    HAL_I3C_EV_IRQHandler(&i3c1_state.hi3c);
}

void I3C1_ER_IRQHandler(void) {
    HAL_I3C_ER_IRQHandler(&i3c1_state.hi3c);
}
#endif

#if defined(OMV_I3C2_ID)
static i3c_state_t i3c2_state;
static uint32_t OMV_ATTR_SEC_ALIGN(i3c2_dma_ctrl[I3C_MAX_MSGS], ".dma_buffer", OMV_DMA_ALIGNMENT);
static uint8_t OMV_ATTR_SEC_ALIGN(i3c2_dma_tx[I3C_TX_BUF_SIZE], ".dma_buffer", OMV_DMA_ALIGNMENT);

void I3C2_EV_IRQHandler(void) {
    HAL_I3C_EV_IRQHandler(&i3c2_state.hi3c);
}

void I3C2_ER_IRQHandler(void) {
    HAL_I3C_ER_IRQHandler(&i3c2_state.hi3c);
}
#endif

static inline i3c_state_t *i3c_state(omv_i2c_t *bus) {
    return (i3c_state_t *) bus->i3c;
}

static i3c_state_t *i3c_state_from_handle(I3C_HandleTypeDef *hi3c) {
    return (i3c_state_t *) ((uint8_t *) hi3c - offsetof(i3c_state_t, hi3c));
}

static bool i3c_addr_reserved(uint8_t addr) {
    // 0x7E and the addresses one bit-flip away from it.
    return addr == 0x3E || addr == 0x5E || addr == 0x6E || addr == 0x76 ||
           addr == 0x7A || addr == 0x7C || addr == 0x7E || addr == 0x7F;
}

static int i3c_dma_init(i3c_state_t *s, DMA_HandleTypeDef *dma, void *channel,
                        uint32_t request, uint32_t direction, uint32_t size) {
    uint32_t ports = (direction == DMA_PERIPH_TO_MEMORY) ?
                     (DMA_SRC_ALLOCATED_PORT1 | DMA_DEST_ALLOCATED_PORT0) :
                     (DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1);

    if (stm_dma_init(dma, channel, request, direction, size, size, ports, &stm_dma_spi_init, false)) {
        return -1;
    }

    dma->Parent = &s->hi3c;
    stm_dma_set_irq_descr(channel, dma);
    NVIC_SetPriority(stm_dma_channel_to_irqn(channel), IRQ_PRI_DMA);
    HAL_NVIC_EnableIRQ(stm_dma_channel_to_irqn(channel));
    return 0;
}

static int i3c_set_timing(i3c_state_t *s, uint32_t clk, uint32_t speed) {
    I3C_CtrlTimingTypeDef timing = {
        .clockSrcFreq = HAL_RCCEx_GetPeriphCLKFreq(clk),
        .i3cPPFreq = OMV_I3C_PP_FREQUENCY,
        // Keeps tLOW within spec for legacy targets at all speeds.
        .dutyCycle = 40,
        .busType = I3C_MIXED_BUS,
    };
    LL_I3C_CtrlBusConfTypeDef config = { 0 };

    switch (speed) {
        case OMV_I2C_SPEED_STANDARD:
            timing.i2cODFreq = 100000;
            break;
        case OMV_I2C_SPEED_FULL:
            timing.i2cODFreq = 400000;
            break;
        case OMV_I2C_SPEED_FAST:
            timing.i2cODFreq = 1000000;
            break;
        default:
            return -1;
    }

    if (I3C_CtrlTimingComputation(&timing, &config) != SUCCESS) {
        return -1;
    }

    config.WaitTime = HAL_I3C_OWN_ACTIVITY_STATE_0;
    s->hi3c.Init.CtrlBusCharacteristic = config;
    s->daa_timing = config;

    #if defined(OMV_I3C_DAA_SCL_HIGH_MIN_NS)
    // Level shifters can't pass the open-drain DAA phases at the 41 ns
    // high time the spec allows, so DAA runs with a longer one.
    uint32_t clk_mhz = timing.clockSrcFreq / 1000000;
    uint32_t high_min = (OMV_I3C_DAA_SCL_HIGH_MIN_NS * clk_mhz + 999) / 1000 - 1;
    s->daa_timing.SCLI3CHighDuration = OMV_MAX(config.SCLI3CHighDuration, high_min);
    #endif
    return 0;
}

int omv_i3c_init(omv_i2c_t *bus, uint32_t bus_id, uint32_t speed) {
    i3c_state_t *s = NULL;
    uint32_t clk = 0;
    IRQn_Type ev_irqn;
    IRQn_Type er_irqn;
    void *dma_cr = NULL;
    void *dma_tx = NULL;
    void *dma_rx = NULL;
    uint32_t req_cr = 0;
    uint32_t req_tx = 0;
    uint32_t req_rx = 0;

    bus->id = bus_id;
    bus->speed = speed;
    bus->initialized = false;
    bus->is_i3c = true;
    bus->cw_size = 0;

    switch (bus_id) {
        #if defined(OMV_I3C1_ID)
        case OMV_I3C1_ID:
            s = &i3c1_state;
            s->hi3c.Instance = I3C1;
            s->async_ctrl = i3c1_dma_ctrl;
            s->async_tx = i3c1_dma_tx;
            bus->scl_pin = OMV_I3C1_SCL_PIN;
            bus->sda_pin = OMV_I3C1_SDA_PIN;
            clk = RCC_PERIPHCLK_I3C1;
            ev_irqn = I3C1_EV_IRQn;
            er_irqn = I3C1_ER_IRQn;
            #if defined(OMV_I3C1_DMA_CHANNEL_RX)
            dma_cr = OMV_I3C1_DMA_CHANNEL_CR;
            dma_tx = OMV_I3C1_DMA_CHANNEL_TX;
            dma_rx = OMV_I3C1_DMA_CHANNEL_RX;
            req_cr = HPDMA1_REQUEST_I3C1_TC;
            req_tx = HPDMA1_REQUEST_I3C1_TX;
            req_rx = HPDMA1_REQUEST_I3C1_RX;
            #endif
            break;
        #endif
        #if defined(OMV_I3C2_ID)
        case OMV_I3C2_ID:
            s = &i3c2_state;
            s->hi3c.Instance = I3C2;
            s->async_ctrl = i3c2_dma_ctrl;
            s->async_tx = i3c2_dma_tx;
            bus->scl_pin = OMV_I3C2_SCL_PIN;
            bus->sda_pin = OMV_I3C2_SDA_PIN;
            clk = RCC_PERIPHCLK_I3C2;
            ev_irqn = I3C2_EV_IRQn;
            er_irqn = I3C2_ER_IRQn;
            #if defined(OMV_I3C2_DMA_CHANNEL_RX)
            dma_cr = OMV_I3C2_DMA_CHANNEL_CR;
            dma_tx = OMV_I3C2_DMA_CHANNEL_TX;
            dma_rx = OMV_I3C2_DMA_CHANNEL_RX;
            req_cr = HPDMA1_REQUEST_I3C2_TC;
            req_tx = HPDMA1_REQUEST_I3C2_TX;
            req_rx = HPDMA1_REQUEST_I3C2_RX;
            #endif
            break;
        #endif
        default:
            return -1;
    }

    s->bus = bus;
    s->busy = false;
    s->async_pending = false;
    s->async_active = false;
    bus->i3c = s;

    if (i3c_set_timing(s, clk, speed)) {
        return -1;
    }

    // Deinit clears the mode, so set it afterwards.
    HAL_I3C_DeInit(&s->hi3c);
    s->hi3c.Mode = HAL_I3C_MODE_CONTROLLER;
    if (HAL_I3C_Init(&s->hi3c) != HAL_OK) {
        return -1;
    }

    I3C_FifoConfTypeDef fifo = {
        .RxFifoThreshold = HAL_I3C_RXFIFO_THRESHOLD_1_4,
        .TxFifoThreshold = HAL_I3C_TXFIFO_THRESHOLD_1_4,
        .ControlFifo = HAL_I3C_CONTROLFIFO_DISABLE,
        .StatusFifo = HAL_I3C_STATUSFIFO_DISABLE,
    };

    I3C_CtrlConfTypeDef ctrl = {
        .DynamicAddr = 0,
        .StallTime = 0,
        .HotJoinAllowed = DISABLE,
        .ACKStallState = DISABLE,
        .CCCStallState = DISABLE,
        .TxStallState = DISABLE,
        .RxStallState = DISABLE,
        .HighKeeperSDA = DISABLE,
    };

    if (HAL_I3C_SetConfigFifo(&s->hi3c, &fifo) != HAL_OK ||
        HAL_I3C_Ctrl_Config(&s->hi3c, &ctrl) != HAL_OK) {
        return -1;
    }

    if (dma_rx) {
        if (i3c_dma_init(s, &s->dma_cr, dma_cr, req_cr, DMA_MEMORY_TO_PERIPH, 4) ||
            i3c_dma_init(s, &s->dma_tx, dma_tx, req_tx, DMA_MEMORY_TO_PERIPH, 1) ||
            i3c_dma_init(s, &s->dma_rx, dma_rx, req_rx, DMA_PERIPH_TO_MEMORY, 1)) {
            return -1;
        }
        s->hi3c.hdmacr = &s->dma_cr;
        s->hi3c.hdmatx = &s->dma_tx;
        s->hi3c.hdmarx = &s->dma_rx;
    }

    NVIC_SetPriority(ev_irqn, IRQ_PRI_I2C);
    NVIC_SetPriority(er_irqn, IRQ_PRI_I2C);
    HAL_NVIC_EnableIRQ(ev_irqn);
    HAL_NVIC_EnableIRQ(er_irqn);

    bus->initialized = true;
    return 0;
}

int omv_i3c_deinit(omv_i2c_t *bus) {
    i3c_state_t *s = i3c_state(bus);

    if (bus->initialized) {
        omv_i3c_abort(bus);
        HAL_I3C_DeInit(&s->hi3c);
        if (s->hi3c.hdmarx) {
            HAL_DMA_DeInit(&s->dma_cr);
            HAL_DMA_DeInit(&s->dma_tx);
            HAL_DMA_DeInit(&s->dma_rx);
        }
    }

    bus->i3c = NULL;
    bus->scl_pin = NULL;
    bus->sda_pin = NULL;
    bus->initialized = false;
    return 0;
}

bool omv_i3c_is_i3c(omv_i2c_t *bus) {
    return bus->is_i3c;
}

int omv_i3c_scan(omv_i2c_t *bus, uint8_t *list, uint8_t size) {
    i3c_state_t *s = i3c_state(bus);
    int idx = 0;

    if (s->busy) {
        return -1;
    }

    for (uint8_t addr = 0x09; addr <= 0x77; addr++) {
        if (HAL_I3C_Ctrl_IsDeviceI2C_Ready(&s->hi3c, addr, 1, I3C_SCAN_TIMEOUT) == HAL_OK) {
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

static int i3c_daa(i3c_state_t *s, uint8_t *addrs, uint64_t *pids, size_t count) {
    uint8_t free_addrs[I3C_MAX_DEVS];
    size_t n_free = 0;

    // Pick dynamic addresses that no legacy target answers to.
    for (uint8_t addr = I3C_DYN_ADDR_FIRST; addr <= I3C_DYN_ADDR_LAST && n_free < I3C_MAX_DEVS; addr++) {
        if (!i3c_addr_reserved(addr) &&
            HAL_I3C_Ctrl_IsDeviceI2C_Ready(&s->hi3c, addr, 1, I3C_SCAN_TIMEOUT) != HAL_OK) {
            free_addrs[n_free++] = addr;
        }
    }

    I3C_DeviceConfTypeDef devs[I3C_MAX_DEVS] = { 0 };
    uint32_t option = I3C_RSTDAA_THEN_ENTDAA;
    size_t n = 0;

    for (;;) {
        uint64_t payload = 0;
        HAL_StatusTypeDef ret = HAL_I3C_Ctrl_DynAddrAssign(&s->hi3c, &payload, option, I3C_TIMEOUT);
        option = I3C_ONLY_ENTDAA;

        if (ret == HAL_OK) {
            break;
        } else if (ret != HAL_BUSY || n == n_free) {
            HAL_I3C_Abort_IT(&s->hi3c);
            return -1;
        }

        // A target sent its payload and waits for an address.
        if (HAL_I3C_Ctrl_SetDynAddr(&s->hi3c, free_addrs[n]) != HAL_OK) {
            return -1;
        }

        if (n < count) {
            uint64_t pid = 0;
            for (size_t i = 0; i < 6; i++) {
                pid = (pid << 8) | ((payload >> (i * 8)) & 0xFF);
            }
            addrs[n] = free_addrs[n] << 1;
            pids[n] = pid;
        }

        devs[n].DeviceIndex = n + 1;
        devs[n].TargetDynamicAddr = free_addrs[n];
        devs[n].IBIAck = DISABLE;
        devs[n].IBIPayload = DISABLE;
        devs[n].CtrlRoleReqAck = DISABLE;
        devs[n].CtrlStopTransfer = DISABLE;
        n++;
    }

    if (n && HAL_I3C_Ctrl_ConfigBusDevices(&s->hi3c, devs, n) != HAL_OK) {
        return -1;
    }

    return n;
}

int omv_i3c_daa(omv_i2c_t *bus, uint8_t *addrs, uint64_t *pids, size_t count) {
    i3c_state_t *s = i3c_state(bus);

    if (s->busy ||
        HAL_I3C_Ctrl_BusCharacteristicConfig(&s->hi3c, &s->daa_timing) != HAL_OK) {
        return -1;
    }

    int ret = i3c_daa(s, addrs, pids, count);

    if (HAL_I3C_Ctrl_BusCharacteristicConfig(&s->hi3c, &s->hi3c.Init.CtrlBusCharacteristic) != HAL_OK) {
        return -1;
    }
    return ret;
}

// Builds a frame from msgs. Writes are concatenated into tx, unless a single
// write can be sent from its own buffer.
static int i3c_frame_init(i3c_frame_t *f, uint8_t addr, omv_i3c_msg_t *msgs, size_t count,
                          uint32_t option, uint8_t *tx, uint32_t *ctrl, bool copy) {
    uint32_t tx_len = 0;
    size_t n_writes = 0;

    if (count == 0 || count > I3C_MAX_MSGS) {
        return -1;
    }

    f->count = count;
    f->option = option;
    f->rx_buf = NULL;
    f->rx_len = 0;

    for (size_t i = 0; i < count; i++) {
        if (!(msgs[i].flags & OMV_I3C_MSG_READ)) {
            tx_len += msgs[i].len;
            n_writes++;
        }
    }

    if ((copy || n_writes > 1) && tx_len > I3C_TX_BUF_SIZE) {
        return -1;
    }

    uint32_t tx_off = 0;
    for (size_t i = 0; i < count; i++) {
        I3C_PrivateTypeDef *d = &f->descs[i];
        memset(d, 0, sizeof(*d));
        d->TargetAddr = addr >> 1;

        if (msgs[i].flags & OMV_I3C_MSG_READ) {
            // All reads land back-to-back in one buffer.
            if (f->rx_buf && msgs[i].buf != f->rx_buf + f->rx_len) {
                return -1;
            }
            if (!f->rx_buf) {
                f->rx_buf = msgs[i].buf;
            }
            f->rx_len += msgs[i].len;
            d->Direction = HAL_I3C_DIRECTION_READ;
            d->RxBuf.pBuffer = msgs[i].buf;
            d->RxBuf.Size = msgs[i].len;
        } else {
            uint8_t *buf = msgs[i].buf;
            if (copy || n_writes > 1) {
                memcpy(tx + tx_off, msgs[i].buf, msgs[i].len);
                buf = tx + tx_off;
            } else {
                tx = buf;
            }
            tx_off += msgs[i].len;
            d->Direction = HAL_I3C_DIRECTION_WRITE;
            d->TxBuf.pBuffer = buf;
            d->TxBuf.Size = msgs[i].len;
        }
    }

    // The HAL copies each write into TxBuf, which is a no-op here.
    memset(&f->xfer, 0, sizeof(f->xfer));
    f->xfer.CtrlBuf.pBuffer = ctrl;
    f->xfer.CtrlBuf.Size = count;
    f->xfer.TxBuf.pBuffer = tx;
    f->xfer.TxBuf.Size = tx_len;
    f->xfer.RxBuf.pBuffer = f->rx_buf;
    f->xfer.RxBuf.Size = f->rx_len;
    return 0;
}

// Claims the bus and starts a frame. Must be called with IRQs disabled.
static HAL_StatusTypeDef i3c_frame_start(i3c_state_t *s, i3c_frame_t *f, bool dma) {
    if (s->busy) {
        return HAL_BUSY;
    }

    if (HAL_I3C_AddDescToFrame(&s->hi3c, NULL, f->descs, &f->xfer, f->count, f->option) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef ret = dma ?
                            HAL_I3C_Ctrl_MultipleTransfer_DMA(&s->hi3c, &f->xfer) :
                            HAL_I3C_Ctrl_MultipleTransfer_IT(&s->hi3c, &f->xfer);

    if (ret == HAL_OK) {
        s->busy = true;
        s->status = 0;
    }
    return ret;
}

static HAL_StatusTypeDef i3c_async_start(i3c_state_t *s) {
    // Invalidate before the DMA so no dirty line is evicted on top of it.
    if (s->async.rx_buf) {
        SCB_InvalidateDCache_by_Addr(s->async.rx_buf, s->async.rx_len);
    }

    HAL_StatusTypeDef ret = i3c_frame_start(s, &s->async, true);
    if (ret != HAL_BUSY) {
        s->async_pending = false;
        s->async_active = (ret == HAL_OK);
    }
    return ret;
}

static void i3c_frame_done(I3C_HandleTypeDef *hi3c, int status) {
    i3c_state_t *s = i3c_state_from_handle(hi3c);

    s->busy = false;
    s->status = status;

    if (s->async_active) {
        s->async_active = false;
        if (s->async.rx_buf) {
            SCB_InvalidateDCache_by_Addr(s->async.rx_buf, s->async.rx_len);
        }
        s->callback(s->bus, status, s->arg);
    }

    // Start a transfer that was queued behind this one.
    if (s->async_pending && i3c_async_start(s) == HAL_ERROR) {
        s->callback(s->bus, -1, s->arg);
    }
}

void HAL_I3C_CtrlMultipleXferCpltCallback(I3C_HandleTypeDef *hi3c) {
    i3c_frame_done(hi3c, 0);
}

// Leaves the controller ready for the next frame after an error.
static void i3c_recover(i3c_state_t *s) {
    I3C_TypeDef *i3c = s->hi3c.Instance;

    if (s->hi3c.hdmarx) {
        HAL_DMA_Abort(&s->dma_cr);
        HAL_DMA_Abort(&s->dma_tx);
        HAL_DMA_Abort(&s->dma_rx);
    }

    LL_I3C_DisableDMAReq_Control(i3c);
    LL_I3C_DisableDMAReq_TX(i3c);
    LL_I3C_DisableDMAReq_RX(i3c);
    LL_I3C_RequestControlFIFOFlush(i3c);
    LL_I3C_RequestTxFIFOFlush(i3c);
    LL_I3C_RequestRxFIFOFlush(i3c);
    LL_I3C_RequestStatusFIFOFlush(i3c);
}

void HAL_I3C_ErrorCallback(I3C_HandleTypeDef *hi3c) {
    i3c_recover(i3c_state_from_handle(hi3c));
    i3c_frame_done(hi3c, -1);
}

void HAL_I3C_AbortCpltCallback(I3C_HandleTypeDef *hi3c) {
    i3c_frame_done(hi3c, -1);
}

static int i3c_transfer(omv_i2c_t *bus, uint8_t addr, omv_i3c_msg_t *msgs, size_t count, uint32_t option) {
    i3c_state_t *s = i3c_state(bus);
    mp_uint_t tick_start = mp_hal_ticks_ms();

    if (!bus->initialized ||
        i3c_frame_init(&s->sync, addr, msgs, count, option, s->sync_tx, s->sync_ctrl, false)) {
        return -1;
    }

    // Wait for asynchronous transfers to release the bus.
    for (HAL_StatusTypeDef ret = HAL_BUSY; ret == HAL_BUSY;) {
        uint32_t irq_state = disable_irq();
        ret = s->async_pending ? HAL_BUSY : i3c_frame_start(s, &s->sync, false);
        enable_irq(irq_state);

        if (ret == HAL_ERROR || (mp_hal_ticks_ms() - tick_start) >= I3C_TIMEOUT) {
            return -1;
        } else if (ret == HAL_BUSY) {
            __WFI();
        }
    }

    while (s->busy) {
        if ((mp_hal_ticks_ms() - tick_start) >= I3C_TIMEOUT) {
            HAL_I3C_Abort_IT(&s->hi3c);
            while (s->busy) {
                __WFI();
            }
            return -1;
        }
        __WFI();
    }

    return s->status;
}

int omv_i3c_transfer(omv_i2c_t *bus, uint8_t addr, omv_i3c_msg_t *msgs, size_t count) {
    return i3c_transfer(bus, addr, msgs, count, I3C_PRIVATE_WITHOUT_ARB_RESTART);
}

int omv_i3c_transfer_i2c(omv_i2c_t *bus, uint8_t addr, omv_i3c_msg_t *msgs, size_t count) {
    return i3c_transfer(bus, addr, msgs, count, I2C_PRIVATE_WITHOUT_ARB_RESTART);
}

int omv_i3c_transfer_async(omv_i2c_t *bus, uint8_t addr, omv_i3c_msg_t *msgs, size_t count,
                           omv_i3c_callback_t callback, void *arg) {
    i3c_state_t *s = i3c_state(bus);
    int ret = 0;

    if (!bus->initialized || !s->hi3c.hdmarx) {
        return -1;
    }

    uint32_t irq_state = disable_irq();
    if (s->async_active || s->async_pending ||
        i3c_frame_init(&s->async, addr, msgs, count, I3C_PRIVATE_WITHOUT_ARB_RESTART,
                       s->async_tx, s->async_ctrl, true)) {
        ret = -1;
    } else {
        s->callback = callback;
        s->arg = arg;
        s->async_pending = true;
        if (i3c_async_start(s) == HAL_ERROR) {
            ret = -1;
        }
    }
    enable_irq(irq_state);
    return ret;
}

void omv_i3c_abort(omv_i2c_t *bus) {
    i3c_state_t *s = i3c_state(bus);

    uint32_t irq_state = disable_irq();
    s->async_pending = false;
    bool active = s->async_active;
    s->async_active = false;
    if (active) {
        HAL_I3C_Abort_IT(&s->hi3c);
    }
    enable_irq(irq_state);

    while (active && s->busy) {
        __WFI();
    }
}
#endif // defined(OMV_I3C1_ID) || defined(OMV_I3C2_ID)

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
 * I3C port for stm32.
 */
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include STM32_HAL_H
#include "py/mphal.h"
#include "irq.h"

#include "board_config.h"
#include "omv_common.h"
#include "omv_i2c.h"
#include "omv_i3c.h"
#include "stm_dma.h"

#if defined(HAL_I3C_MODULE_ENABLED)
#include "stm32n6xx_util_i3c.h"

// The longest transfers, a frame or the VL53L9CX's firmware patch, take
// about 20 ms. Blocking waits don't service events, so they're kept short.
#define I3C_TIMEOUT         (100)
#define I3C_SCAN_TIMEOUT    (10)
// An aborted frame runs to its end on the bus, so aborts wait for the
// longest one. Forcing the bus idle earlier would leave its bytes to the
// next frame.
#define I3C_ABORT_TIMEOUT   (100)
#define I3C_TX_BUF_SIZE     (32)
#define I3C_DYN_ADDR_FIRST  (0x08)
#define I3C_DYN_ADDR_LAST   (0x3D)
#define I3C_BCR_IBI_PAYLOAD (1 << 2)

// Push-pull SCL frequency, which boards can lower for slow targets.
#ifndef OMV_I3C_PP_FREQUENCY
#define OMV_I3C_PP_FREQUENCY    (12500000)
#endif

static void omv_i3c_ev_irq_handler(I3C_HandleTypeDef *descr);

// DMA transfers copy writes to a non-cacheable buffer, so the caller's buffer
// can be released on return.
#define DEFINE_I3C_INSTANCE(n)                                                                      \
    static I3C_HandleTypeDef I3CHandle##n;                                                          \
    static uint32_t OMV_ATTR_SEC_ALIGN(omv_i3c##n##_dma_ctrl[2], ".dma_buffer", OMV_DMA_ALIGNMENT); \
    static uint8_t OMV_ATTR_SEC_ALIGN(omv_i3c##n##_dma_tx[I3C_TX_BUF_SIZE], ".dma_buffer",          \
                                      OMV_DMA_ALIGNMENT);                                           \
    void I3C##n##_EV_IRQHandler(void) { omv_i3c_ev_irq_handler(&I3CHandle##n); }                    \
    void I3C##n##_ER_IRQHandler(void) { HAL_I3C_ER_IRQHandler(&I3CHandle##n); }

#if defined(OMV_I3C1_ID)
DEFINE_I3C_INSTANCE(1)
#endif

#if defined(OMV_I3C2_ID)
DEFINE_I3C_INSTANCE(2)
#endif

#define INITIALIZE_I3C_DESCR(i3c, i3c_number)                                              \
    do {                                                                                   \
        (i3c)->ev_irqn = I3C##i3c_number##_EV_IRQn;                                        \
        (i3c)->er_irqn = I3C##i3c_number##_ER_IRQn;                                        \
        (i3c)->clk = RCC_PERIPHCLK_I3C##i3c_number;                                        \
        (i3c)->descr = &I3CHandle##i3c_number;                                             \
        (i3c)->descr->Instance = I3C##i3c_number;                                          \
        (i3c)->dma_descr_cr = (DMA_HandleTypeDef)                                          \
        { OMV_I3C##i3c_number##_DMA_CHANNEL_CR, { HPDMA1_REQUEST_I3C##i3c_number##_TC } }; \
        (i3c)->dma_descr_tx = (DMA_HandleTypeDef)                                          \
        { OMV_I3C##i3c_number##_DMA_CHANNEL_TX, { HPDMA1_REQUEST_I3C##i3c_number##_TX } }; \
        (i3c)->dma_descr_rx = (DMA_HandleTypeDef)                                          \
        { OMV_I3C##i3c_number##_DMA_CHANNEL_RX, { HPDMA1_REQUEST_I3C##i3c_number##_RX } }; \
        (i3c)->dma_ctrl = omv_i3c##i3c_number##_dma_ctrl;                                  \
        (i3c)->dma_tx = omv_i3c##i3c_number##_dma_tx;                                      \
    } while (0)

static omv_i3c_t *omv_i3c_descr_all[2] = { NULL };

static omv_i3c_t *omv_i3c_descr_get(I3C_HandleTypeDef *hi3c) {
    omv_i3c_t *i3c = NULL;
    if (0) {
    #if defined(OMV_I3C1_ID)
    } else if (hi3c->Instance == I3C1) {
        i3c = omv_i3c_descr_all[0];
    #endif
    #if defined(OMV_I3C2_ID)
    } else if (hi3c->Instance == I3C2) {
        i3c = omv_i3c_descr_all[1];
    #endif
    }
    return i3c;
}

omv_i3c_t *omv_i3c_get(uint32_t bus_id) {
    omv_i3c_t *i3c = NULL;
    if (0) {
    #if defined(OMV_I3C1_ID)
    } else if (bus_id == OMV_I3C1_ID) {
        i3c = omv_i3c_descr_all[0];
    #endif
    #if defined(OMV_I3C2_ID)
    } else if (bus_id == OMV_I3C2_ID) {
        i3c = omv_i3c_descr_all[1];
    #endif
    }
    return i3c;
}

// In-band interrupts are handled here, as the HAL only handles them while
// listening for events, and not during transfers.
static void omv_i3c_ev_irq_handler(I3C_HandleTypeDef *descr) {
    omv_i3c_t *i3c = omv_i3c_descr_get(descr);
    I3C_TypeDef *instance = descr->Instance;

    if (i3c && LL_I3C_IsEnabledIT_IBI(instance) && LL_I3C_IsActiveFlag_IBI(instance)) {
        uint8_t addr = LL_I3C_GetIBITargetAddr(instance) << 1;
        LL_I3C_ClearFlag_IBI(instance);
        if (i3c->ibi_callback && addr == i3c->ibi_addr) {
            i3c->ibi_callback(i3c->ibi_data);
        }
    }

    // An aborted read runs to its end on the bus, and stalls it once the RX
    // FIFO is full. The HAL doesn't read the FIFO while aborting, so its
    // interrupt would fire forever, so the bytes are dropped here.
    if (descr->State == HAL_I3C_STATE_ABORT) {
        while (LL_I3C_IsActiveFlag_RXFNE(instance)) {
            (void) LL_I3C_ReceiveData8(instance);
        }
    }

    HAL_I3C_EV_IRQHandler(descr);
}

static bool omv_i3c_addr_reserved(uint8_t addr) {
    // 0x7E and the addresses one bit-flip away from it.
    return addr == 0x3E || addr == 0x5E || addr == 0x6E || addr == 0x76 ||
           addr == 0x7A || addr == 0x7C || addr == 0x7E || addr == 0x7F;
}

// I3C targets are reached by their dynamic address, others are legacy targets.
static uint32_t omv_i3c_xfer_option(omv_i3c_t *i3c, uint8_t slv_addr) {
    for (size_t i = 0; i < i3c->dyn_count; i++) {
        if (i3c->dyn_devs[i].TargetDynamicAddr == (slv_addr >> 1)) {
            return I3C_PRIVATE_WITHOUT_ARB_RESTART;
        }
    }
    return I2C_PRIVATE_WITHOUT_ARB_RESTART;
}

static int omv_i3c_set_timing(omv_i3c_t *i3c, uint32_t speed) {
    I3C_CtrlTimingTypeDef timing = {
        .clockSrcFreq = HAL_RCCEx_GetPeriphCLKFreq(i3c->clk),
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
    i3c->descr->Init.CtrlBusCharacteristic = config;
    i3c->daa_timing = config;

    #if defined(OMV_I3C_DAA_SCL_HIGH_MIN_NS)
    // Level shifters can't pass the open-drain DAA phases at the 41 ns
    // high time the spec allows, so DAA runs with a longer one.
    uint32_t clk_mhz = timing.clockSrcFreq / 1000000;
    uint32_t high_min = (OMV_I3C_DAA_SCL_HIGH_MIN_NS * clk_mhz + 999) / 1000 - 1;
    i3c->daa_timing.SCLI3CHighDuration = OMV_MAX(config.SCLI3CHighDuration, high_min);
    #endif
    return 0;
}

static int omv_i3c_dma_init(DMA_HandleTypeDef *dma_descr, uint32_t direction, uint32_t size) {
    uint32_t ports = (direction == DMA_PERIPH_TO_MEMORY) ?
                     (DMA_SRC_ALLOCATED_PORT1 | DMA_DEST_ALLOCATED_PORT0) :
                     (DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1);

    // Initialize DMA.
    if (stm_dma_init(dma_descr, dma_descr->Instance, dma_descr->Init.Request, direction,
                     size, size, ports, &stm_dma_spi_init, false)) {
        return -1;
    }

    // Set the DMA handle used by the DMA channel's IRQ handler.
    stm_dma_set_irq_descr(dma_descr->Instance, dma_descr);

    // Same priority as the I3C IRQs, so the HAL's error handling isn't preempted.
    uint8_t dma_irqn = stm_dma_channel_to_irqn(dma_descr->Instance);
    NVIC_SetPriority(dma_irqn, IRQ_PRI_I2C);
    HAL_NVIC_EnableIRQ(dma_irqn);
    return 0;
}

static int omv_i3c_bus_init(omv_i3c_t *i3c) {
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

    // Deinit clears the mode, so set it afterwards.
    HAL_I3C_DeInit(i3c->descr);
    i3c->descr->Mode = HAL_I3C_MODE_CONTROLLER;

    if (HAL_I3C_Init(i3c->descr) != HAL_OK ||
        HAL_I3C_SetConfigFifo(i3c->descr, &fifo) != HAL_OK ||
        HAL_I3C_Ctrl_Config(i3c->descr, &ctrl) != HAL_OK) {
        return -1;
    }
    return 0;
}

int omv_i3c_init(omv_i3c_t *i3c, uint32_t bus_id, uint32_t speed) {
    size_t idx = 0;

    // Targets keep their dynamic addresses across a re-init, until the next
    // DAA, so the DAA table isn't cleared.
    i3c->id = bus_id;
    i3c->speed = speed;
    i3c->initialized = false;
    i3c->xfer_flags = 0;
    i3c->busy = false;
    i3c->dma_pending = false;
    i3c->dma_active = false;
    i3c->cw_size = 0;

    if (0) {
    #if defined(OMV_I3C1_ID)
    } else if (bus_id == OMV_I3C1_ID) {
        INITIALIZE_I3C_DESCR(i3c, 1);
        idx = 0;
    #endif
    #if defined(OMV_I3C2_ID)
    } else if (bus_id == OMV_I3C2_ID) {
        INITIALIZE_I3C_DESCR(i3c, 2);
        idx = 1;
    #endif
    } else {
        return -1;
    }

    if (omv_i3c_set_timing(i3c, speed) != 0 ||
        omv_i3c_bus_init(i3c) != 0) {
        return -1;
    }

    if (i3c->dyn_count && HAL_I3C_Ctrl_ConfigBusDevices(i3c->descr, i3c->dyn_devs, i3c->dyn_count) != HAL_OK) {
        return -1;
    }

    if (omv_i3c_dma_init(&i3c->dma_descr_cr, DMA_MEMORY_TO_PERIPH, 4) ||
        omv_i3c_dma_init(&i3c->dma_descr_tx, DMA_MEMORY_TO_PERIPH, 1) ||
        omv_i3c_dma_init(&i3c->dma_descr_rx, DMA_PERIPH_TO_MEMORY, 1)) {
        return -1;
    }

    // Link the DMA handles to the I3C handle.
    __HAL_LINKDMA(i3c->descr, hdmacr, i3c->dma_descr_cr);
    __HAL_LINKDMA(i3c->descr, hdmatx, i3c->dma_descr_tx);
    __HAL_LINKDMA(i3c->descr, hdmarx, i3c->dma_descr_rx);

    // Configure and enable I3C IRQ channels.
    NVIC_SetPriority(i3c->ev_irqn, IRQ_PRI_I2C);
    NVIC_SetPriority(i3c->er_irqn, IRQ_PRI_I2C);
    HAL_NVIC_EnableIRQ(i3c->ev_irqn);
    HAL_NVIC_EnableIRQ(i3c->er_irqn);

    i3c->initialized = true;
    omv_i3c_descr_all[idx] = i3c;
    return 0;
}

int omv_i3c_deinit(omv_i3c_t *i3c) {
    if (i3c && i3c->initialized) {
        // The abort completes in the IRQs, which find the controller in the
        // descriptor table, so it's removed afterwards.
        omv_i3c_transfer_abort(i3c);
        i3c->initialized = false;
        for (size_t i = 0; i < OMV_ARRAY_SIZE(omv_i3c_descr_all); i++) {
            if (omv_i3c_descr_all[i] == i3c) {
                omv_i3c_descr_all[i] = NULL;
            }
        }
        HAL_NVIC_DisableIRQ(i3c->ev_irqn);
        HAL_NVIC_DisableIRQ(i3c->er_irqn);
        HAL_I3C_DeInit(i3c->descr);
        HAL_DMA_DeInit(&i3c->dma_descr_cr);
        HAL_DMA_DeInit(&i3c->dma_descr_tx);
        HAL_DMA_DeInit(&i3c->dma_descr_rx);
    }
    return 0;
}

int omv_i3c_scan(omv_i3c_t *i3c, uint8_t *list, uint8_t size) {
    int idx = 0;

    if (!i3c->initialized || i3c->busy) {
        return -1;
    }

    for (uint8_t addr = 0x09; addr <= 0x77; addr++) {
        if (HAL_I3C_Ctrl_IsDeviceI2C_Ready(i3c->descr, addr, 1, I3C_SCAN_TIMEOUT) == HAL_OK) {
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

static int omv_i3c_assign_addrs(omv_i3c_t *i3c, uint8_t *list, uint64_t *pids, uint8_t size) {
    uint8_t free_addrs[OMV_I3C_MAX_DEVS];
    size_t n_free = 0;

    // Pick dynamic addresses that no legacy target answers to.
    for (uint8_t addr = I3C_DYN_ADDR_FIRST; addr <= I3C_DYN_ADDR_LAST && n_free < OMV_I3C_MAX_DEVS; addr++) {
        if (!omv_i3c_addr_reserved(addr) &&
            HAL_I3C_Ctrl_IsDeviceI2C_Ready(i3c->descr, addr, 1, I3C_SCAN_TIMEOUT) != HAL_OK) {
            free_addrs[n_free++] = addr;
        }
    }

    I3C_DeviceConfTypeDef *devs = i3c->dyn_devs;
    uint32_t option = I3C_RSTDAA_THEN_ENTDAA;
    size_t idx = 0;

    for (;;) {
        uint64_t payload = 0;
        HAL_StatusTypeDef ret = HAL_I3C_Ctrl_DynAddrAssign(i3c->descr, &payload, option, I3C_TIMEOUT);
        option = I3C_ONLY_ENTDAA;

        if (ret == HAL_OK) {
            break;
        } else if (ret != HAL_BUSY || idx == n_free) {
            HAL_I3C_Abort_IT(i3c->descr);
            return -1;
        }

        // A target sent its payload and waits for an address.
        if (HAL_I3C_Ctrl_SetDynAddr(i3c->descr, free_addrs[idx]) != HAL_OK) {
            HAL_I3C_Abort_IT(i3c->descr);
            return -1;
        }

        if (idx < size) {
            uint64_t pid = 0;
            for (size_t i = 0; i < 6; i++) {
                pid = (pid << 8) | ((payload >> (i * 8)) & 0xFF);
            }
            list[idx] = free_addrs[idx] << 1;
            pids[idx] = pid;
        }

        // In-band interrupts are acknowledged, with their payload if the
        // target's BCR says it sends one.
        devs[idx].DeviceIndex = idx + 1;
        devs[idx].TargetDynamicAddr = free_addrs[idx];
        devs[idx].IBIAck = ENABLE;
        devs[idx].IBIPayload = ((payload >> 48) & I3C_BCR_IBI_PAYLOAD) ? ENABLE : DISABLE;
        devs[idx].CtrlRoleReqAck = DISABLE;
        devs[idx].CtrlStopTransfer = DISABLE;
        i3c->dyn_count = ++idx;
    }

    if (idx && HAL_I3C_Ctrl_ConfigBusDevices(i3c->descr, devs, idx) != HAL_OK) {
        return -1;
    }

    return idx;
}

int omv_i3c_daa(omv_i3c_t *i3c, uint8_t *list, uint64_t *pids, uint8_t size) {
    if (!i3c->initialized || i3c->busy) {
        return -1;
    }

    // RSTDAA clears all dynamic addresses.
    i3c->dyn_count = 0;

    if (HAL_I3C_Ctrl_BusCharacteristicConfig(i3c->descr, &i3c->daa_timing) != HAL_OK) {
        return -1;
    }

    int ret = omv_i3c_assign_addrs(i3c, list, pids, size);

    if (HAL_I3C_Ctrl_BusCharacteristicConfig(i3c->descr, &i3c->descr->Init.CtrlBusCharacteristic) != HAL_OK) {
        return -1;
    }
    return ret;
}

static void omv_i3c_frame_init(omv_i3c_frame_t *frame, omv_i3c_transfer_t *xfer,
                               uint32_t option, uint32_t *ctrl, uint8_t *txbuf) {
    memset(frame, 0, sizeof(omv_i3c_frame_t));
    frame->option = option;

    if (xfer->txsize) {
        I3C_PrivateTypeDef *descr = &frame->descr[frame->count++];
        descr->TargetAddr = xfer->slv_addr >> 1;
        descr->Direction = HAL_I3C_DIRECTION_WRITE;
        descr->TxBuf.pBuffer = txbuf;
        descr->TxBuf.Size = xfer->txsize;
    }

    if (xfer->rxsize) {
        I3C_PrivateTypeDef *descr = &frame->descr[frame->count++];
        descr->TargetAddr = xfer->slv_addr >> 1;
        descr->Direction = HAL_I3C_DIRECTION_READ;
        descr->RxBuf.pBuffer = xfer->rxbuf;
        descr->RxBuf.Size = xfer->rxsize;
    }

    // The HAL copies the write into TxBuf, which is a no-op here.
    frame->xfer.CtrlBuf.pBuffer = ctrl;
    frame->xfer.CtrlBuf.Size = frame->count;
    frame->xfer.TxBuf.pBuffer = txbuf;
    frame->xfer.TxBuf.Size = xfer->txsize;
    frame->xfer.RxBuf.pBuffer = xfer->rxbuf;
    frame->xfer.RxBuf.Size = xfer->rxsize;
}

// Blocking transfers start from threads, and DMA transfers usually from the
// I3C interrupts, which share one priority. So threads only change the bus
// state within atomic sections.

// Claims the bus and starts a frame. Must be called within an atomic section.
static HAL_StatusTypeDef omv_i3c_frame_start(omv_i3c_t *i3c, omv_i3c_frame_t *frame, bool dma) {
    if (i3c->busy) {
        return HAL_BUSY;
    }

    if (HAL_I3C_AddDescToFrame(i3c->descr, NULL, frame->descr, &frame->xfer,
                               frame->count, frame->option) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef ret = dma ?
                            HAL_I3C_Ctrl_MultipleTransfer_DMA(i3c->descr, &frame->xfer) :
                            HAL_I3C_Ctrl_MultipleTransfer_IT(i3c->descr, &frame->xfer);

    if (ret == HAL_OK) {
        i3c->busy = true;
    }
    return ret;
}

static HAL_StatusTypeDef omv_i3c_dma_start(omv_i3c_t *i3c) {
    I3C_DataTypeDef *rx = &i3c->dma_frame.xfer.RxBuf;

    // Invalidate before the DMA so no dirty line is evicted on top of it.
    if (rx->Size) {
        SCB_InvalidateDCache_by_Addr(rx->pBuffer, rx->Size);
    }

    HAL_StatusTypeDef ret = omv_i3c_frame_start(i3c, &i3c->dma_frame, true);
    if (ret != HAL_BUSY) {
        i3c->dma_pending = false;
        i3c->dma_active = (ret == HAL_OK);
    }
    return ret;
}

static void omv_i3c_callback(omv_i3c_t *i3c, bool failed) {
    i3c->busy = false;

    if (i3c->dma_active) {
        I3C_DataTypeDef *rx = &i3c->dma_frame.xfer.RxBuf;

        if (rx->Size) {
            SCB_InvalidateDCache_by_Addr(rx->pBuffer, rx->Size);
        }

        i3c->dma_active = false;
        i3c->xfer_flags |= failed ? OMV_I3C_XFER_FAILED : OMV_I3C_XFER_COMPLETE;
        i3c->callback(i3c, i3c->userdata, rx->pBuffer);
    } else {
        i3c->failed = failed;
        i3c->done = true;
    }

    // Start a DMA transfer that was queued behind this one.
    if (i3c->dma_pending && omv_i3c_dma_start(i3c) == HAL_ERROR) {
        i3c->xfer_flags |= OMV_I3C_XFER_FAILED;
        i3c->callback(i3c, i3c->userdata, i3c->dma_frame.xfer.RxBuf.pBuffer);
    }
}

// Leaves the controller ready for the next frame after an error or abort.
static void omv_i3c_reset(omv_i3c_t *i3c) {
    I3C_TypeDef *instance = i3c->descr->Instance;

    HAL_DMA_Abort(&i3c->dma_descr_cr);
    HAL_DMA_Abort(&i3c->dma_descr_tx);
    HAL_DMA_Abort(&i3c->dma_descr_rx);

    LL_I3C_DisableDMAReq_Control(instance);
    LL_I3C_DisableDMAReq_TX(instance);
    LL_I3C_DisableDMAReq_RX(instance);
    LL_I3C_RequestControlFIFOFlush(instance);
    LL_I3C_RequestTxFIFOFlush(instance);
    LL_I3C_RequestRxFIFOFlush(instance);
    LL_I3C_RequestStatusFIFOFlush(instance);

    // No transfer handles its interrupts until the next frame, which would
    // otherwise fire forever.
    WRITE_REG(instance->IER, READ_REG(instance->IER) & I3C_IER_IBIIE);
    i3c->descr->XferISR = NULL;
}

void HAL_I3C_CtrlMultipleXferCpltCallback(I3C_HandleTypeDef *descr) {
    omv_i3c_t *i3c = omv_i3c_descr_get(descr);
    if (i3c) {
        omv_i3c_callback(i3c, false);
    }
}

void HAL_I3C_ErrorCallback(I3C_HandleTypeDef *descr) {
    omv_i3c_t *i3c = omv_i3c_descr_get(descr);
    if (i3c) {
        omv_i3c_reset(i3c);
        omv_i3c_callback(i3c, true);
    }
}

// Bytes of an aborted read left in the FIFOs would be read by the next frame.
void HAL_I3C_AbortCpltCallback(I3C_HandleTypeDef *descr) {
    omv_i3c_t *i3c = omv_i3c_descr_get(descr);
    if (i3c) {
        omv_i3c_reset(i3c);
        omv_i3c_callback(i3c, true);
    }
}

// Aborts the frame on the bus. If the HAL never reports the abort, which it
// doesn't when only the control word DMA is running, the bus is forced idle.
static void omv_i3c_abort_frame(omv_i3c_t *i3c) {
    // The frame may have ended since the caller checked. Aborting an idle
    // controller would leave the HAL waiting for an abort that never ends.
    mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    if (i3c->busy) {
        HAL_I3C_Abort_IT(i3c->descr);
    }
    MICROPY_END_ATOMIC_SECTION(atomic_state);

    for (mp_uint_t tick_start = mp_hal_ticks_ms(); i3c->busy;) {
        if ((mp_hal_ticks_ms() - tick_start) >= I3C_ABORT_TIMEOUT) {
            break;
        }
        __WFI();
    }

    // Force the bus idle if the HAL never reported the abort.
    atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    if (i3c->busy || i3c->descr->State == HAL_I3C_STATE_ABORT) {
        omv_i3c_reset(i3c);
        i3c->descr->State = HAL_I3C_STATE_READY;
        i3c->busy = false;
    }
    MICROPY_END_ATOMIC_SECTION(atomic_state);
}

// Like omv_i2c, waits without servicing events, as a callback or exception
// could reuse or leave the bus while a frame is in flight.
static int omv_i3c_wait_timeout(omv_i3c_t *i3c, uint32_t timeout) {
    mp_uint_t tick_start = mp_hal_ticks_ms();
    while (!i3c->done) {
        if ((mp_hal_ticks_ms() - tick_start) >= timeout) {
            omv_i3c_abort_frame(i3c);
            return -1;
        }
        __WFI();
    }
    return i3c->failed ? -1 : 0;
}

int omv_i3c_transfer_start(omv_i3c_t *i3c, omv_i3c_transfer_t *xfer) {
    int ret = 0;

    if (!i3c->initialized || (xfer->txsize == 0 && xfer->rxsize == 0)) {
        return -1;
    }

    uint32_t option = omv_i3c_xfer_option(i3c, xfer->slv_addr);

    if (xfer->flags & OMV_I3C_XFER_BLOCKING) {
        omv_i3c_frame_init(&i3c->frame, xfer, option, i3c->ctrl, xfer->txbuf);
        i3c->done = false;

        // Wait for DMA transfers to release the bus.
        for (mp_uint_t tick_start = mp_hal_ticks_ms(); ;) {
            mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
            HAL_StatusTypeDef status = i3c->dma_pending ? HAL_BUSY : omv_i3c_frame_start(i3c, &i3c->frame, false);
            MICROPY_END_ATOMIC_SECTION(atomic_state);

            if (status == HAL_OK) {
                break;
            } else if (status == HAL_ERROR || (mp_hal_ticks_ms() - tick_start) >= I3C_TIMEOUT) {
                return -1;
            }
            __WFI();
        }

        // A queued DMA transfer may start as soon as this one is done.
        ret = omv_i3c_wait_timeout(i3c, I3C_TIMEOUT);
    } else if (xfer->flags & OMV_I3C_XFER_DMA) {
        if (xfer->txsize > I3C_TX_BUF_SIZE) {
            return -1;
        }

        mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
        if (i3c->dma_active || i3c->dma_pending) {
            ret = -1;
        } else {
            // The write is copied, so it can be released on return.
            memcpy(i3c->dma_tx, xfer->txbuf, xfer->txsize);
            omv_i3c_frame_init(&i3c->dma_frame, xfer, option, i3c->dma_ctrl, i3c->dma_tx);
            i3c->callback = xfer->callback;
            i3c->userdata = xfer->userdata;
            i3c->xfer_flags = xfer->flags & ~(OMV_I3C_XFER_FAILED | OMV_I3C_XFER_COMPLETE);
            i3c->dma_pending = true;
            if (omv_i3c_dma_start(i3c) == HAL_ERROR) {
                ret = -1;
            }
        }
        MICROPY_END_ATOMIC_SECTION(atomic_state);
    } else {
        ret = -1;
    }

    return ret;
}

// Aborts a DMA transfer without calling its callback.
int omv_i3c_transfer_abort(omv_i3c_t *i3c) {
    if (i3c->initialized) {
        mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
        bool active = i3c->dma_active;
        i3c->dma_pending = false;
        i3c->dma_active = false;
        MICROPY_END_ATOMIC_SECTION(atomic_state);

        if (active) {
            omv_i3c_abort_frame(i3c);
        }
    }
    return 0;
}

// Writes without a STOP are cached and sent with the next transfer, as one
// frame with a repeated START, like on the I2C controller.
static int omv_i3c_xfer(omv_i3c_t *i3c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags, bool read) {
    bool no_stop = flags & (OMV_I2C_XFER_NO_STOP | OMV_I2C_XFER_SUSPEND);
    omv_i3c_transfer_t xfer = {
        .slv_addr = slv_addr,
        .flags = OMV_I3C_XFER_BLOCKING,
    };

    if (!read && (no_stop || i3c->cw_size)) {
        if (i3c->cw_size + len > sizeof(i3c->cw_buf)) {
            i3c->cw_size = 0;
            return -1;
        }

        memcpy(i3c->cw_buf + i3c->cw_size, buf, len);
        i3c->cw_size += len;

        if (no_stop) {
            return 0;
        }

        buf = i3c->cw_buf;
        len = i3c->cw_size;
        i3c->cw_size = 0;
    }

    if (read) {
        xfer.txbuf = i3c->cw_buf;
        xfer.txsize = i3c->cw_size;
        xfer.rxbuf = buf;
        xfer.rxsize = len;
    } else {
        xfer.txbuf = buf;
        xfer.txsize = len;
    }

    i3c->cw_size = 0;
    return omv_i3c_transfer_start(i3c, &xfer);
}

int omv_i3c_read(omv_i3c_t *i3c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags) {
    return omv_i3c_xfer(i3c, slv_addr, buf, len, flags, true);
}

int omv_i3c_write(omv_i3c_t *i3c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags) {
    return omv_i3c_xfer(i3c, slv_addr, buf, len, flags, false);
}

int omv_i3c_ibi_register(omv_i3c_t *i3c, uint8_t slv_addr, omv_i3c_ibi_callback_t callback, void *data) {
    if (!i3c->initialized) {
        return -1;
    }

    omv_i3c_ibi_enable(i3c, false);
    i3c->ibi_addr = slv_addr;
    i3c->ibi_callback = callback;
    i3c->ibi_data = data;
    return 0;
}

// Disabling drops the interrupt received, if any. Later ones stay flagged
// until enabled.
int omv_i3c_ibi_enable(omv_i3c_t *i3c, bool enable) {
    if (!i3c->initialized) {
        return -1;
    }

    if (enable) {
        LL_I3C_EnableIT_IBI(i3c->descr->Instance);
    } else {
        LL_I3C_DisableIT_IBI(i3c->descr->Instance);
        LL_I3C_ClearFlag_IBI(i3c->descr->Instance);
    }
    return 0;
}
#endif // defined(HAL_I3C_MODULE_ENABLED)

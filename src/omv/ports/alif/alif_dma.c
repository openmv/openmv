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
 * Alif DMA driver.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "irq.h"
#include CMSIS_MCU_H

#include "evtrtr.h"
#include "dma_op.h"
#include "dma_ctrl.h"
#include "dma_config.h"
#include "dma_opcode.h"
#include "dma_mapping.h"

#include "sys_ctrl_i2s.h"
#include "sys_ctrl_dma.h"

#include "py/mphal.h"
#include "runtime.h"
#include "alif_hal.h"
#include "alif_dma.h"

#define DMA_SHARED_INDEX    (0)
#define DMA_LOCAL_INDEX     (1)

#define DMA_SHARED_FILTER   (0)
#define DMA_LOCAL_FILTER    (0)

#define DMA_IS_LOCAL(dma)   ((dma == ((DMA_Type *) DMALOCAL_SEC_BASE)) || (dma == ((DMA_Type *) DMALOCAL_NS_BASE)))
#define DMA_IS_SECURE(dma)  ((dma == ((DMA_Type *) DMALOCAL_SEC_BASE)) || (dma == ((DMA_Type *) DMA0_SEC_BASE)))
#define DMA_DESCR_INDEX(dma) (DMA_IS_LOCAL(dma) ? DMA_LOCAL_INDEX : DMA_SHARED_INDEX)

#define IRQ_PRI_DMA         NVIC_EncodePriority(NVIC_PRIORITYGROUP_7, 2, 0)

typedef struct {
    DMA_Type *dma_inst;
    bool is_local;
    bool is_secure;
    bool initialized;
    IRQn_Type irqn;
    uint32_t channels;
    dma_callback_t callback[DMA_MAX_CHANNELS];
    dma_config_info_t config;
} dma_descr_t;

typedef dma_channel_info_t chn_descr_t;

static dma_descr_t dma_descr_all[2] __attribute__((section(".bss.sram0")));

// DMA helper functions.
static dma_descr_t *dma_get_dma_descr(DMA_Type *inst) {
    return &dma_descr_all[DMA_DESCR_INDEX(inst)];
}

static chn_descr_t *dma_get_chn_descr(DMA_Type *inst, int8_t index) {
    dma_descr_t *dma_descr = dma_get_dma_descr(inst);
    return &dma_descr->config.channel_thread[index].channel_info;
}

static bool dma_thread_stopped(DMA_Type *inst, uint8_t index) {
    return (dma_get_channel_status(inst, index) == DMA_THREAD_STATUS_STOPPED);
}

static bool dma_thread_allocated(DMA_Type *inst, uint8_t index) {
    dma_descr_t *dma_descr = dma_get_dma_descr(inst);
    return dma_descr->config.channel_thread[index].in_use;
}

static void dma_event_handler(dma_descr_t *dma_descr, uint8_t event) {
    uint8_t index = dma_descr->config.event_map[event];
    dma_clear_interrupt(dma_descr->dma_inst, event);
    // Call event callback if set.
    if (dma_descr->callback[index]) {
        dma_descr->callback[index](DMA_EVENT_COMPLETE);
    }
}

static void dma_fault_handler(dma_descr_t *dma_descr) {
    if (dma_manager_is_faulting(dma_descr->dma_inst)) {
        // Requires a software reset.
    }
    for (uint8_t index = 0; index < DMA_MAX_CHANNELS; index++) {
        if (dma_get_channel_fault_status(dma_descr->dma_inst, index)) {
            dma_channel_t channel = {
                .index = index,
                .inst = dma_descr->dma_inst
            };
            // Abort channel.
            dma_abort(&channel, true);
            // Call event callback if set.
            if (dma_descr->callback[index]) {
                dma_descr->callback[index](DMA_EVENT_ABORTED);
            }
        }
    }
}

static int dma_execute_opcode(DMA_Type *dma, const dma_opcode_buf *opcode, uint8_t channel, uint8_t dthread) {
    uint8_t *opcode_buf = opcode->buf;
    uint8_t opcode_len = opcode->buf_size;

    dma_dbginst0_t dma_dbginst0 = { .dbginst0 = 0 };
    dma_dbginst0.dbginst0_b.chn_num = channel;
    dma_dbginst0.dbginst0_b.dbg_thrd = dthread;
    dma_dbginst0.dbginst0_b.ins_byte0 = opcode_buf[0];
    if (opcode_len > DMA_OP_1BYTE_LEN) {
        dma_dbginst0.dbginst0_b.ins_byte1 = opcode_buf[1];
    }

    dma_dbginst1_t dma_dbginst1 = { .dbginst1 = 0 };
    if (opcode_len > DMA_OP_2BYTE_LEN) {
        dma_dbginst1.dbginst1_b.ins_byte2 = opcode_buf[2];
    }
    if (opcode_len > DMA_OP_3BYTE_LEN) {
        dma_dbginst1.dbginst1_b.ins_byte3 = opcode_buf[3];
    }
    if (opcode_len == DMA_OP_6BYTE_LEN) {
        dma_dbginst1.dbginst1_b.ins_byte4 = opcode_buf[4];
        dma_dbginst1.dbginst1_b.ins_byte5 = opcode_buf[5];
    }

    dma_execute(dma, dma_dbginst0.dbginst0, dma_dbginst1.dbginst1);
    return 0;
}

static int dma_init(dma_descr_t *dma_descr, DMA_Type *inst) {
    bool is_local = DMA_IS_LOCAL(inst);
    bool is_secure = DMA_IS_SECURE(inst);
    uint32_t ns_mask = is_secure ? 0 : 0xFFFFFFFF;

    // Reset events and chans_descr.
    dma_reset_all_events(&dma_descr->config);
    dma_reset_all_channels(&dma_descr->config);

    // Initialize DMA
    if (is_local) {
        dmalocal_set_glitch_filter(DMA_LOCAL_FILTER);
        dmalocal_enable_periph_clk();
        evtrtrlocal_enable_dma_req();
        if (is_secure) {
            dmalocal_set_boot_manager_secure();
        } else {
            dmalocal_set_boot_manager_nonsecure();
        }
        dmalocal_set_boot_irq_ns_mask(ns_mask);
        dmalocal_set_boot_periph_ns_mask(ns_mask);
        dmalocal_reset();
    } else {
        dma0_set_glitch_filter(DMA_SHARED_FILTER);
        dma0_enable_periph_clk();
        evtrtr0_enable_dma_req();
        if (is_secure) {
            dma0_set_boot_manager_secure();
        } else {
            dma0_set_boot_manager_nonsecure();
        }
        dma0_set_boot_irq_ns_mask(ns_mask);
        dma0_set_boot_periph_ns_mask(ns_mask);
        dma0_reset();
    }

    dma_descr->dma_inst = inst;
    dma_descr->is_local = is_local;
    dma_descr->is_secure = is_secure;
    dma_descr->irqn = (IRQn_Type) (is_local ? DMALOCAL_IRQ0_IRQn : DMA0_IRQ0_IRQn);
    dma_descr->initialized = true;

    // Configure DMA abort IRQ
    NVIC_ClearPendingIRQ(dma_descr->irqn + DMA_IRQ_ABORT_OFFSET);
    NVIC_SetPriority(dma_descr->irqn + DMA_IRQ_ABORT_OFFSET, IRQ_PRI_DMA);
    NVIC_EnableIRQ(dma_descr->irqn + DMA_IRQ_ABORT_OFFSET);
    return 0;
}

static int dma_deinit(dma_descr_t *dma_descr) {
    if (!dma_descr->initialized) {
        return 0;
    }

    // Deinitialize DMA
    if (dma_descr->is_local) {
        dmalocal_reset();
        evtrtrlocal_disable_dma_req();
        dmalocal_disable_periph_clk();
    } else {
        dma0_reset();
        evtrtr0_disable_dma_req();
        dma0_disable_periph_clk();
    }

    // Disable DMA abort IRQ
    NVIC_DisableIRQ(dma_descr->irqn + DMA_IRQ_ABORT_OFFSET);
    NVIC_ClearPendingIRQ(dma_descr->irqn + DMA_IRQ_ABORT_OFFSET);

    // Clear DMA descriptor.
    memset(dma_descr, 0, sizeof(dma_descr_t));
    return 0;
}

int dma_deinit_all() {
    dma_deinit(&dma_descr_all[0]);
    dma_deinit(&dma_descr_all[1]);
    return 0;
}

int dma_alloc(dma_channel_t *channel, dma_config_t *config) {
    dma_descr_t *dma_descr = dma_get_dma_descr(config->inst);
    chn_descr_t *chn_descr = dma_get_chn_descr(config->inst, channel->index);

    // Set DMA instance.
    channel->inst = config->inst;

    // Initialize DMA descriptor (if not initialized yet).
    if (!dma_descr->initialized) {
        dma_init(dma_descr, channel->inst);
    }

    // Check if DMA instance is configured in a different mode.
    if (dma_descr->dma_inst != channel->inst) {
        return -1;
    }

    // Allocate a DMA channel.
    if ((channel->index = dma_allocate_channel(&dma_descr->config)) < 0) {
        ;
        return -1;
    }

    // Allocate a DMA event.
    int8_t event;
    if ((event = dma_allocate_event(&dma_descr->config, channel->index)) < 0) {
        ;
        dma_release_channel(&dma_descr->config, channel->index);
        return -1;
    }

    #if CORE_M55_HP
    // Select DMA0 for LP peripherals on HP core.
    // The HE core uses local DMA to access these.
    if (!dma_descr->is_local) {
        switch (config->request & 0xFF) {
            case LPPDM_DMA_PERIPH_REQ:
                lppdm_select_dma0();
                break;
            case LPI2S_DMA_TX_PERIPH_REQ:
            case LPI2S_DMA_RX_PERIPH_REQ:
                lpi2s_select_dma0();
                break;
            case LPUART_DMA_TX_PERIPH_REQ:
            case LPUART_DMA_RX_PERIPH_REQ:
                lpuart_select_dma0();
                break;
            case LPSPI_DMA_TX_PERIPH_REQ:
            case LPSPI_DMA_RX_PERIPH_REQ:
                lpspi_select_dma0(RTE_LPSPI_SELECT_DMA0_GROUP);
                break;
        }
    }
    #endif

    // Configure event router.
    if (dma_descr->is_local) {
        evtrtrlocal_enable_dma_channel(config->request & 0xFF, DMA_ACK_COMPLETION_PERIPHERAL);
    } else {
        evtrtr0_enable_dma_channel(config->request & 0xFF, config->request >> 8, DMA_ACK_COMPLETION_PERIPHERAL);
        evtrtr0_enable_dma_handshake(config->request & 0xFF, config->request >> 8);
    }

    // Update active channels count.
    dma_descr->channels++;

    // Copy channel config to internal channel descriptor.
    chn_descr->flags |= config->flags;
    chn_descr->desc_info.direction = config->direction;
    chn_descr->desc_info.periph_num = config->request & 0xFF;
    chn_descr->desc_info.src_bsize = config->burst_size;
    chn_descr->desc_info.dst_bsize = config->burst_size;
    chn_descr->desc_info.src_blen = config->burst_blen;
    chn_descr->desc_info.dst_blen = config->burst_blen;
    chn_descr->desc_info.sec_state = dma_descr->is_secure == false;
    chn_descr->desc_info.dst_prot_ctrl = dma_descr->is_secure ? 0x00 : 0x02;
    chn_descr->desc_info.src_prot_ctrl = dma_descr->is_secure ? 0x00 : 0x02;
    chn_descr->desc_info.src_cache_ctrl = (config->direction == DMA_TRANSFER_DEV_TO_MEM) ? 0x0 : 0x02;
    chn_descr->desc_info.dst_cache_ctrl = (config->direction == DMA_TRANSFER_MEM_TO_DEV) ? 0x0 : 0x02;
    chn_descr->desc_info.endian_swap_size = config->byte_swap;

    // Configure the DMA channel's IRQ priority.
    NVIC_SetPriority(dma_descr->irqn + chn_descr->event_index, config->priority);
    return 0;
}

int dma_start(dma_channel_t *channel, void *src, void *dst0, void *dst1, uint32_t size, dma_callback_t callback) {
    dma_descr_t *dma_descr = dma_get_dma_descr(channel->inst);
    chn_descr_t *chn_descr = dma_get_chn_descr(channel->inst, channel->index);

    uint8_t go_opcode_buf[DMA_OP_6BYTE_LEN] = {0};
    dma_opcode_buf go_opcode = { .off = 0, .buf = go_opcode_buf, .buf_size = DMA_OP_6BYTE_LEN };

    if (dma_debug_is_busy(dma_descr->dma_inst)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("DMA debug is busy"));
    }

    if (!dma_thread_stopped(dma_descr->dma_inst, channel->index)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("DMA channel is busy"));
    }

    if (!dma_thread_allocated(dma_descr->dma_inst, channel->index)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("DMA channel not allocated"));
    }

    if (((uint32_t) src | (uint32_t) dst0 | size)
        & ((1 << (uint32_t) chn_descr->desc_info.dst_bsize) - 1)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Unaligned DMA transfer"));
    }

    // Set transfer source, destination and and total size.
    chn_descr->desc_info.src_addr = LocalToGlobal(src);
    chn_descr->desc_info.dst_addr0 = LocalToGlobal(dst0);
    chn_descr->desc_info.dst_addr1 = LocalToGlobal(dst1);
    chn_descr->desc_info.total_len = size;

    if (!dma_generate_opcode(&dma_descr->config, channel->index)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Generate OP code failed"));
    }

    // Flush opcode buffer.
    uint8_t *opcode_buf = dma_get_opcode_buf(&dma_descr->config, channel->index);
    RTSS_CleanDCache_by_Addr(opcode_buf, DMA_MICROCODE_SIZE);

    // Construct go microcode.
    dma_construct_go(!dma_descr->is_secure, channel->index, LocalToGlobal(opcode_buf), &go_opcode);
    dma_execute_opcode(dma_descr->dma_inst, &go_opcode, channel->index, DMA_THREAD_MANAGER);

    // Set channel callback
    dma_descr->callback[channel->index] = callback;


    // Configure and enable DMA channel IRQ
    IRQn_Type irqn = (IRQn_Type) (dma_descr->irqn + chn_descr->event_index);
    NVIC_DisableIRQ(irqn);
    NVIC_ClearPendingIRQ(irqn);
    NVIC_EnableIRQ(irqn);

    dma_clear_interrupt(dma_descr->dma_inst, chn_descr->event_index);
    dma_enable_interrupt(dma_descr->dma_inst, chn_descr->event_index);
    return 0;
}

int dma_abort(dma_channel_t *channel, bool dealloc) {
    dma_descr_t *dma_descr = dma_get_dma_descr(channel->inst);
    chn_descr_t *chn_descr = dma_get_chn_descr(channel->inst, channel->index);

    uint8_t opcode_buf[DMA_OP_1BYTE_LEN] = { 0 };
    dma_opcode_buf kill_opcode = { .off = 0, .buf = opcode_buf, .buf_size = DMA_OP_1BYTE_LEN };

    if (!dma_descr->initialized ||
        !dma_descr->channels ||
        dma_debug_is_busy(dma_descr->dma_inst) ||
        dma_thread_stopped(dma_descr->dma_inst, channel->index) ||
        !dma_thread_allocated(dma_descr->dma_inst, channel->index)) {
        return -1;
    }

    // Construct kill microcode.
    dma_construct_kill(&kill_opcode);
    dma_execute_opcode(dma_descr->dma_inst, &kill_opcode, channel->index, DMA_THREAD_CHANNEL);

    for (size_t i = 0, max_retry = 1000; i < max_retry; i++) {
        if (dma_thread_stopped(dma_descr->dma_inst, channel->index)) {
            break;
        }
        if ((i + 1) == max_retry) {
            return -1;
        }
    }

    // Disable and clear DMA channel IRQ
    IRQn_Type irqn = (IRQn_Type) (dma_descr->irqn + chn_descr->event_index);
    NVIC_DisableIRQ(irqn);
    NVIC_ClearPendingIRQ(irqn);

    dma_disable_interrupt(dma_descr->dma_inst, chn_descr->event_index);
    dma_clear_interrupt(dma_descr->dma_inst, chn_descr->event_index);

    printf("DMA channel: %d event: %d stopped\n", channel->index, chn_descr->event_index);

    // Deallocate channel
    if (dealloc) {
        dma_descr->channels--;
        dma_release_event(&dma_descr->config, chn_descr->event_index);
        dma_release_channel(&dma_descr->config, channel->index);
    }

    // Deinit DMA if last channel.
    if (dma_descr->channels == 0) {
        dma_deinit(dma_descr);
    }

    // Clear callback
    dma_descr->callback[channel->index] = NULL;
    return 0;
}

void *dma_target_address(dma_channel_t *channel) {
    chn_descr_t *chn_descr = dma_get_chn_descr(channel->inst, channel->index);
    uint32_t target = dma_get_channel_dest_addr(channel->inst, channel->index);
    if (chn_descr->desc_info.dst_addr1 == 0) {
        // Single buffer mode.
        return GlobalToLocal(target);
    } else if (target == chn_descr->desc_info.dst_addr0) {
        return GlobalToLocal(chn_descr->desc_info.dst_addr1);
    } else {
        return GlobalToLocal(chn_descr->desc_info.dst_addr0);
    }
}

// DMA0 Handlers
void DMA0_IRQ0Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 0);
}

void DMA0_IRQ1Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 1);
}

void DMA0_IRQ2Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 2);
}

void DMA0_IRQ3Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 3);
}

void DMA0_IRQ4Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 4);
}

void DMA0_IRQ5Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 5);
}

void DMA0_IRQ6Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 6);
}

void DMA0_IRQ7Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 7);
}

void DMA0_IRQ8Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 8);
}

void DMA0_IRQ9Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 9);
}

void DMA0_IRQ10Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 10);
}

void DMA0_IRQ11Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 11);
}

void DMA0_IRQ12Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 12);
}

void DMA0_IRQ13Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 13);
}

void DMA0_IRQ14Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 14);
}

void DMA0_IRQ15Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 15);
}

void DMA0_IRQ16Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 16);
}

void DMA0_IRQ17Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 17);
}

void DMA0_IRQ18Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 18);
}

void DMA0_IRQ19Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 19);
}

void DMA0_IRQ20Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 20);
}

void DMA0_IRQ21Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 21);
}

void DMA0_IRQ22Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 22);
}

void DMA0_IRQ23Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 23);
}

void DMA0_IRQ24Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 24);
}

void DMA0_IRQ25Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 25);
}

void DMA0_IRQ26Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 26);
}

void DMA0_IRQ27Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 27);
}

void DMA0_IRQ28Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 28);
}

void DMA0_IRQ29Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 29);
}

void DMA0_IRQ30Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 30);
}

void DMA0_IRQ31Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_SHARED_INDEX], 31);
}

void DMA0_IRQ_ABORT_Handler(void) {
    dma_fault_handler(&dma_descr_all[DMA_SHARED_INDEX]);
}

// DMALOCAL Handlers
void DMALOCAL_IRQ0Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 0);
}

void DMALOCAL_IRQ1Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 1);
}

void DMALOCAL_IRQ2Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 2);
}

void DMALOCAL_IRQ3Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 3);
}

void DMALOCAL_IRQ4Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 4);
}

void DMALOCAL_IRQ5Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 5);
}

void DMALOCAL_IRQ6Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 6);
}

void DMALOCAL_IRQ7Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 7);
}

void DMALOCAL_IRQ8Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 8);
}

void DMALOCAL_IRQ9Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 9);
}

void DMALOCAL_IRQ10Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 10);
}

void DMALOCAL_IRQ11Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 11);
}

void DMALOCAL_IRQ12Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 12);
}

void DMALOCAL_IRQ13Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 13);
}

void DMALOCAL_IRQ14Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 14);
}

void DMALOCAL_IRQ15Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 15);
}

void DMALOCAL_IRQ16Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 16);
}

void DMALOCAL_IRQ17Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 17);
}

void DMALOCAL_IRQ18Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 18);
}

void DMALOCAL_IRQ19Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 19);
}

void DMALOCAL_IRQ20Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 20);
}

void DMALOCAL_IRQ21Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 21);
}

void DMALOCAL_IRQ22Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 22);
}

void DMALOCAL_IRQ23Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 23);
}

void DMALOCAL_IRQ24Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 24);
}

void DMALOCAL_IRQ25Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 25);
}

void DMALOCAL_IRQ26Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 26);
}

void DMALOCAL_IRQ27Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 27);
}

void DMALOCAL_IRQ28Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 28);
}

void DMALOCAL_IRQ29Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 29);
}

void DMALOCAL_IRQ30Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 30);
}

void DMALOCAL_IRQ31Handler(void) {
    dma_event_handler(&dma_descr_all[DMA_LOCAL_INDEX], 31);
}

void DMALOCAL_IRQ_ABORT_Handler(void) {
    dma_fault_handler(&dma_descr_all[DMA_LOCAL_INDEX]);
}

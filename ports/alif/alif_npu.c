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
 * EthosU HAL layer.
 */
#include <stdio.h>
#include "irq.h"
#include CMSIS_MCU_H

#include "py/mphal.h"
#include "py/runtime.h"

#include "alif_hal.h"
#include "board_config.h"
#include "imlib.h"
#include "omv_memory.h"
#include "ethosu_driver.h"

#define ETHOSU_SEC_ENABLED      (1)
#define ETHOSU_PRIV_ENABLED     (1)

#define ETHOSU_CACHE_SIZE       (0)
#define ETHOSU_CACHE_ALIGN      (16)

#define ETHOSU_IRQ_NUMBER       NPULOCAL_IRQ_IRQn
#define ETHOSU_IRQ_HANDLER      NPULOCAL_IRQHandler
#define ETHOSU_BASE_ADDRESS     (void *) NPULOCAL_BASE

// The driver creates a semaphore per driver (NPU IRQ) and one per NPU variant
// (driver reservation), so two for the single NPU used by each core.
#define ETHOSU_SEMAPHORE_COUNT  (2)

// Give up on an NPU command stream after this long. The driver then resets the
// NPU and the inference fails, instead of the CPU waiting for an IRQ forever.
#define ETHOSU_INFERENCE_TIMEOUT_MS (5000)


#if (ETHOSU_CACHE_SIZE == 0)
static uint8_t * ETHOSU_CACHE_BUFFER = NULL;
#else
static uint8_t OMV_ATTR_SEC_ALIGN(ETHOSU_CACHE_BUFFER[ETHOSU_CACHE_SIZE], ".bss.sram0", ETHOSU_CACHE_ALIGN);
#endif

static struct ethosu_driver ethosu_driver;

// The command stream and all regions use AXI_LIMIT0 or AXI_LIMIT1 (the M0 port), with
// 128-byte bursts and normal (modifiable) non-cacheable transactions.
static struct u55_config_t ethosu_u55_config = {
    .qconfig.cmd_region0 = 0,
    .regioncfg = {
        .region0 = 0, .region1 = 0, .region2 = 1, .region3 = 1,
        .region4 = 1, .region5 = 1, .region6 = 1, .region7 = 1,
    },
    .axi_limit0 = { .max_beats = 1, .memtype = 2, .max_outstanding_read_m1 = 31, .max_outstanding_write_m1 = 15 },
    .axi_limit1 = { .max_beats = 1, .memtype = 2, .max_outstanding_read_m1 = 31, .max_outstanding_write_m1 = 15 },
    .axi_limit2 = { .max_beats = 1, .memtype = 2, .max_outstanding_read_m1 = 31, .max_outstanding_write_m1 = 15 },
    .axi_limit3 = { .max_beats = 1, .memtype = 2, .max_outstanding_read_m1 = 31, .max_outstanding_write_m1 = 15 },
};

static struct ethosu_device_config ethosu_u55_device_config = {
    .config = &ethosu_u55_config,
};

struct ethosu_semaphore_t {
    uint8_t count;
    bool used;
};

static struct ethosu_semaphore_t ethosu_semaphores[ETHOSU_SEMAPHORE_COUNT];

void *ethosu_semaphore_create(void) {
    // The default implementation allocates these dynamically.
    for (size_t i = 0; i < ETHOSU_SEMAPHORE_COUNT; i++) {
        if (!ethosu_semaphores[i].used) {
            ethosu_semaphores[i].used = true;
            ethosu_semaphores[i].count = 0;
            return &ethosu_semaphores[i];
        }
    }
    return NULL;
}

int ethosu_semaphore_take(void *sem, uint64_t timeout) {
    struct ethosu_semaphore_t *s = sem;
    mp_uint_t start = mp_hal_ticks_ms();
    while (s->count == 0) {
        if ((mp_hal_ticks_ms() - start) >= ETHOSU_INFERENCE_TIMEOUT_MS) {
            return -1;
        }
        __WFE();
    }
    s->count--;
    return 0;
}

int ethosu_semaphore_give(void *sem) {
    struct ethosu_semaphore_t *s = sem;
    s->count++;
    __SEV();
    return 0;
}

void ethosu_semaphore_destroy(void *sem) {
    struct ethosu_semaphore_t *s = sem;
    s->used = false;
}

static inline bool is_mram(const void *p, size_t bytes) {
    uintptr_t addr = (uintptr_t) p;
    return addr >= SOC_FEAT_MRAM_BASE && addr + bytes <= SOC_FEAT_MRAM_BASE + SOC_FEAT_MRAM_SIZE;
}

void ethosu_flush_dcache(const uint64_t *base_addr, const size_t *base_addr_size, int num_base_addr) {
    for (int i = 0; i < num_base_addr; i++) {
        void *p = (void *) (uintptr_t) base_addr[i];
        size_t bytes = base_addr_size[i];
        if (omv_memory_is_write_back(p, bytes)) {
            SCB_CleanDCache_by_Addr(p, bytes);
        }
    }
}

void ethosu_invalidate_dcache(const uint64_t *base_addr, const size_t *base_addr_size, int num_base_addr) {
    for (int i = 0; i < num_base_addr; i++) {
        void *p = (void *) (uintptr_t) base_addr[i];
        size_t bytes = base_addr_size[i];
        // MRAM is cached write-through, so it is cacheable, but it was never invalidated
        // before. It's excluded to keep that behavior, the NPU only reads it (e.g. weights).
        if (!is_mram(p, bytes) && omv_memory_is_cacheable(p, bytes)) {
            SCB_InvalidateDCache_by_Addr(p, bytes);
        }
    }
}

static uint64_t ethosu_address_remap(uint64_t address, int index) {
    return LocalToGlobal((void *) (uint32_t) address);
}

static struct ethosu_device_user_ops ethosu_user_ops = {
    .address_remap = ethosu_address_remap,
};

void ethosu_inference_begin(struct ethosu_driver *drv, void *user_arg) {
    // The driver doesn't flush the command stream, which may be in a model loaded to RAM.
    void *p = (void *) drv->job.custom_data_ptr;
    size_t bytes = drv->job.custom_data_size;
    if (omv_memory_is_write_back(p, bytes)) {
        SCB_CleanDCache_by_Addr(p, bytes);
    }
    imlib_poll_events_noexc();
}

void ethosu_inference_end(struct ethosu_driver *drv, void *user_arg) {
    imlib_poll_events_noexc();
}

void ETHOSU_IRQ_HANDLER(void) {
    ethosu_irq_handler(&ethosu_driver);
}

int alif_npu_init(void) {
    // Initialize Ethos-U.
    if (ethosu_init_ex(&ethosu_driver,              /* Ethos-U driver device pointer */
                       &ethosu_device_desc_u55,     /* Ethos-U device descriptor. */
                       &ethosu_u55_device_config,   /* Ethos-U device config. */
                       &ethosu_user_ops,            /* Ethos-U device user ops. */
                       ETHOSU_BASE_ADDRESS,         /* Ethos-U NPU's base address. */
                       ETHOSU_CACHE_BUFFER,         /* Pointer to fast mem area - NULL for U55. */
                       ETHOSU_CACHE_SIZE,           /* Fast mem region size. */
                       ETHOSU_SEC_ENABLED,          /* Security enable. */
                       ETHOSU_PRIV_ENABLED)) {
        /* Privilege enable. */
        return -1;
    }

    NVIC_SetPriority(ETHOSU_IRQ_NUMBER, IRQ_PRI_NPU);
    NVIC_ClearPendingIRQ(ETHOSU_IRQ_NUMBER);
    NVIC_EnableIRQ(ETHOSU_IRQ_NUMBER);
    return 0;
}

int alif_npu_deinit(void) {
    ethosu_soft_reset(&ethosu_driver);
    ethosu_deinit(&ethosu_driver);

    // Disable NPU interrupt
    NVIC_DisableIRQ(ETHOSU_IRQ_NUMBER);
    NVIC_ClearPendingIRQ(ETHOSU_IRQ_NUMBER);
    return 0;
}

int alif_npu_info() {
    struct ethosu_hw_info hw_info;
    ethosu_get_hw_info(&ethosu_driver, &hw_info);

    struct ethosu_driver_version driver_version;
    ethosu_get_driver_version(&driver_version);

    printf("Ethos-U version info:\n");
    printf("\tArch: %ld %ld %ld \n",
           hw_info.version.arch_major_rev,
           hw_info.version.arch_minor_rev,
           hw_info.version.arch_patch_rev);

    printf("\tDriver: %d %d %d\n",
           driver_version.major,
           driver_version.minor,
           driver_version.patch);

    printf("\tMACs/cc:    %ld \n", (uint32_t) (1 << hw_info.cfg.macs_per_cc));
    printf("\tCmd stream: %ld \n", hw_info.cfg.cmd_stream_version);
    return 0;
}

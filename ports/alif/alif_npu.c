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
#include "omv_boardconfig.h"
#include "ethosu_driver.h"

#define ETHOSU_SEC_ENABLED      (1)
#define ETHOSU_PRIV_ENABLED     (1)

#define ETHOSU_CACHE_ALIGN      (16)
//#define ETHOSU_CACHE_SIZE       (393216U)
#define ETHOSU_CACHE_SIZE       (0)

#define ETHOSU_CACHE_SECTION    section(".bss.ethosu_cache")
#define ETHOSU_CACHE_ATTRIBUTE  __attribute__((aligned(ETHOSU_CACHE_ALIGN), ETHOSU_CACHE_SECTION))

#define IRQ_PRI_NPU             NVIC_EncodePriority(NVIC_PRIORITYGROUP_7, 8, 0)

#if CORE_M55_HP
#define ETHOSU_IRQ_NUMBER       NPU_HP_IRQ_IRQn
#define ETHOSU_IRQ_HANDLER      NPU_HP_IRQHandler
#define ETHOSU_BASE_ADDRESS     (void *) NPU_HP_BASE
#else
#define ETHOSU_IRQ_NUMBER       NPU_HE_IRQ_IRQn
#define ETHOSU_IRQ_HANDLER      NPU_HE_IRQHandler
#define ETHOSU_BASE_ADDRESS     (void *) NPU_HE_BASE
#endif

#if (ETHOSU_CACHE_SIZE > 0)
static uint8_t ETHOSU_CACHE_BUFFER[ETHOSU_CACHE_SIZE] ETHOSU_CACHE_ATTRIBUTE;
#else
static uint8_t *ETHOSU_CACHE_BUFFER = NULL;
#endif

static struct ethosu_driver ethosu_driver;

typedef struct mmap {
    uintptr_t base;
    uintptr_t limit;
} mmap_t;

static const mmap_t mmap_nocache[] = {
    { .base = ITCM_BASE, .limit = ITCM_BASE + ITCM_SIZE },
    { .base = DTCM_BASE, .limit = DTCM_BASE + ITCM_SIZE },
    { .base = MRAM_BASE, .limit = MRAM_BASE + MRAM_SIZE },
    { .base = OSPI0_XIP_BASE, .limit = OSPI0_XIP_BASE + OSPI0_XIP_SIZE },
    { .base = OSPI1_XIP_BASE, .limit = OSPI1_XIP_BASE + OSPI1_XIP_SIZE }
};

static bool is_cacheable(const void *p, size_t bytes) {
    uintptr_t base = (uintptr_t) p;
    if (bytes == 0) {
        return false;
    }
    uintptr_t limit = base + bytes;
    for (unsigned int i = 0; i < sizeof(mmap_nocache) / sizeof(mmap_nocache[0]); i++) {
        if (base >= mmap_nocache[i].base && limit < mmap_nocache[i].limit) {
            return false;
        }
    }
    return true;
}

struct ethosu_semaphore_t {
    uint8_t count;
};

void *ethosu_semaphore_create(void) {
    // The default implementation allocates this dynamically,
    // but there's no need for more than one semaphore, so a
    // static one can be used.
    static struct ethosu_semaphore_t sem;
    sem.count = 0;
    return &sem;
}

int ethosu_semaphore_take(void *sem, uint64_t timeout) {
    struct ethosu_semaphore_t *s = sem;
    while (s->count == 0) {
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
    // The Semaphore is statically allocated.
}

void ethosu_flush_dcache(uint32_t *p, size_t bytes) {
    if (!p) {
        SCB_CleanDCache();
    } else if (is_cacheable(p, bytes)) {
        SCB_CleanDCache_by_Addr(p, bytes);
    }
}

void ethosu_invalidate_dcache(uint32_t *p, size_t bytes) {
    if (!p) {
        SCB_CleanInvalidateDCache();
    } else if (is_cacheable(p, bytes)) {
        SCB_InvalidateDCache_by_Addr(p, bytes);
    }
}

uint64_t ethosu_address_remap(uint64_t address, int index) {
    return LocalToGlobal((void *) (uint32_t) address);
}

void ethosu_inference_begin(struct ethosu_driver *drv, void *user_arg) {
    mp_handle_pending_internal(MP_HANDLE_PENDING_CALLBACKS_ONLY);
}

void ethosu_inference_end(struct ethosu_driver *drv, void *user_arg) {
    mp_handle_pending_internal(MP_HANDLE_PENDING_CALLBACKS_ONLY);
}

void ETHOSU_IRQ_HANDLER(void) {
    ethosu_irq_handler(&ethosu_driver);
}

int alif_npu_init(void) {
    // Initialize Ethos-U.
    if (ethosu_init(&ethosu_driver,         /* Ethos-U driver device pointer */
                    ETHOSU_BASE_ADDRESS,    /* Ethos-U NPU's base address. */
                    ETHOSU_CACHE_BUFFER,    /* Pointer to fast mem area - NULL for U55. */
                    ETHOSU_CACHE_SIZE,      /* Fast mem region size. */
                    ETHOSU_SEC_ENABLED,     /* Security enable. */
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

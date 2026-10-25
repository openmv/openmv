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
 * STM32 memory attributes.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include STM32_HAL_H
#include "omv_memory.h"

#if defined(STM32H7) || defined(STM32N6)
typedef struct {
    uint32_t base;
    uint64_t limit;
    uint32_t domain;
} omv_memory_region_t;
#endif

#if defined(STM32H7)
// H7 memory domains.
#define OMV_MEMORY_TCM      (1 << 0)
#define OMV_MEMORY_D1       (1 << 1)
#define OMV_MEMORY_D2       (1 << 2)
#define OMV_MEMORY_D3       (1 << 3)

static const omv_memory_region_t omv_memory_regions[] = {
    { D1_ITCMRAM_BASE, D1_ITCMRAM_BASE + 0x10000, OMV_MEMORY_TCM },
    { D1_DTCMRAM_BASE, D1_DTCMRAM_BASE + 0x20000, OMV_MEMORY_TCM },
    { FLASH_BANK1_BASE, FLASH_END + 1ULL, OMV_MEMORY_D1 },
    { D1_AXISRAM_BASE, D1_AXISRAM_BASE + 0x80000, OMV_MEMORY_D1 },
    { 0x60000000, 0xA0000000, OMV_MEMORY_D1 },  // FMC banks 1 to 3, QUADSPI.
    { 0xC0000000, 0xE0000000, OMV_MEMORY_D1 },  // FMC SDRAM banks 1 and 2.
    { D2_AHBSRAM_BASE, D2_AHBSRAM_BASE + 0x48000, OMV_MEMORY_D2 },  // SRAM1 to SRAM3.
    { D3_SRAM_BASE, D3_SRAM_BASE + 0x10000, OMV_MEMORY_D3 },        // SRAM4.
    { D3_BKPSRAM_BASE, D3_BKPSRAM_BASE + 0x1000, OMV_MEMORY_D3 },   // Backup SRAM.
};

// Returns the memory domains accessible by master (AN4891 Table 4, and
// AN5557 Table 1 for dual-core parts).
static uint32_t omv_memory_master_domains(uintptr_t master) {
    if (!master) {
        // GP-DMA (DMA1/DMA2).
        return OMV_MEMORY_D1 | OMV_MEMORY_D2 | OMV_MEMORY_D3;
    } else if (master - MDMA_BASE < 0x1000) {
        // MDMA reaches the TCMs through the M7 AHBS.
        return OMV_MEMORY_TCM | OMV_MEMORY_D1 | OMV_MEMORY_D2 | OMV_MEMORY_D3;
    } else if (master - BDMA_BASE < 0x400) {
        return OMV_MEMORY_D3;
    } else if (master == SDMMC1_BASE || master == LTDC_BASE) {
        return OMV_MEMORY_D1;
    } else if (master == DMA2D_BASE || master == ETH_BASE) {
        return OMV_MEMORY_D1 | OMV_MEMORY_D2;
    }
    // GP-DMA (DMA1/DMA2), SDMMC2, USB.
    return OMV_MEMORY_D1 | OMV_MEMORY_D2 | OMV_MEMORY_D3;
}
#elif defined(STM32N6)
// N6 memory interconnects.
#define OMV_MEMORY_TCM      (1 << 0)
#define OMV_MEMORY_AXI      (1 << 1)
#define OMV_MEMORY_AHB      (1 << 2)

// Both the nonsecure and secure aliases are listed.
static const omv_memory_region_t omv_memory_regions[] = {
    { ITCM_BASE_NS, ITCM_BASE_NS + 0x40000, OMV_MEMORY_TCM },
    { ITCM_BASE_S, ITCM_BASE_S + 0x40000, OMV_MEMORY_TCM },
    { DTCM_BASE_NS, DTCM_BASE_NS + 0x40000, OMV_MEMORY_TCM },
    { DTCM_BASE_S, DTCM_BASE_S + 0x40000, OMV_MEMORY_TCM },
    // FLEXRAM, AXISRAM1 to 6, CACHEAXI RAM and VENCRAM.
    { SRAM1_AXI_BASE_NS, SRAM1_AXI_BASE_NS + 0x420000, OMV_MEMORY_AXI },
    { SRAM1_AXI_BASE_S, SRAM1_AXI_BASE_S + 0x420000, OMV_MEMORY_AXI },
    { SRAM1_AHB_BASE_NS, SRAM1_AHB_BASE_NS + SRAM1_AHB_SIZE + SRAM2_AHB_SIZE, OMV_MEMORY_AHB },
    { SRAM1_AHB_BASE_S, SRAM1_AHB_BASE_S + SRAM1_AHB_SIZE + SRAM2_AHB_SIZE, OMV_MEMORY_AHB },
    { BKPSRAM_BASE_NS, BKPSRAM_BASE_NS + BKPSRAM_SIZE, OMV_MEMORY_AHB },
    { BKPSRAM_BASE_S, BKPSRAM_BASE_S + BKPSRAM_SIZE, OMV_MEMORY_AHB },
    { FMC_BASE, 0xA0000000, OMV_MEMORY_AXI },   // FMC NOR/SRAM, XSPI1 to 3.
    { 0xC0000000, 0xE0000000, OMV_MEMORY_AXI }, // FMC SDRAM 1 and 2.
};

#define OMV_MEMORY_IS_MASTER(master, base, size) \
    (((master) - base##_NS < (size)) || ((master) - base##_S < (size)))

// Returns the memory interconnects accessible by master (RM0486 Table 83).
static uint32_t omv_memory_master_domains(uintptr_t master) {
    if (!master || OMV_MEMORY_IS_MASTER(master, HPDMA1_BASE, 0x1000) ||
        OMV_MEMORY_IS_MASTER(master, GPDMA1_BASE, 0x1000)) {
        // HPDMA1 reaches the TCMs only through its AXI port, which is
        // selected per channel, so the TCMs are excluded.
        return OMV_MEMORY_AXI | OMV_MEMORY_AHB;
    }
    // DMA2D, LTDC, SDMMC1/2, ETH1, OTG1/2, DCMIPP, GPU, VENC.
    return OMV_MEMORY_AXI;
}
#endif

bool omv_memory_is_dmable(const void *master, const void *addr, size_t size) {
    // All STM32 DMA masters require word-aligned buffers.
    if (size == 0 || ((uintptr_t) addr & 3)) {
        return false;
    }

    #if defined(STM32F4) || defined(STM32H7) || defined(STM32N6)
    uint64_t start = (uintptr_t) addr;
    uint64_t end = start + size;
    #endif

    #if defined(STM32F4)
    // CCM is not accessible by DMA.
    return start > CCMDATARAM_END || end <= CCMDATARAM_BASE;
    #elif defined(STM32F7)
    // ITCM is not accessible by DMA, DTCM is through the M7 AHBS (RM0410 2.1.6).
    // The ITCM is 16KB at address 0 on all F7 parts.
    return (uintptr_t) addr >= RAMITCM_BASE + 0x4000;
    #elif defined(STM32H7) || defined(STM32N6)
    uint32_t domains = omv_memory_master_domains((uintptr_t) master);
    for (size_t i = 0; i < sizeof(omv_memory_regions) / sizeof(omv_memory_regions[0]); i++) {
        const omv_memory_region_t *r = &omv_memory_regions[i];
        if (start >= r->base && end <= r->limit) {
            return r->domain & domains;
        }
    }
    return false;
    #else
    return true;
    #endif
}

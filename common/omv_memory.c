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
 * Memory attributes abstraction layer.
 *
 * Cache attributes are derived from the core: the TCM control registers, the
 * MPU regions and, for addresses not covered by any region, the architectural
 * default memory map. The configuration is read once, on the first call. DMA
 * accessibility is port-specific.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include CMSIS_MCU_H
#include "omv_memory.h"

typedef enum {
    OMV_MEMORY_POLICY_NONE,
    OMV_MEMORY_POLICY_WT,
    OMV_MEMORY_POLICY_WB,
} omv_memory_policy_t;

// Unpredictable, reserved and implementation defined encodings are treated as
// write-back: maintenance on non-cacheable memory is harmless, skipping it on
// cacheable memory is not.
#define OMV_MEMORY_POLICY_UNKNOWN    OMV_MEMORY_POLICY_WB

#if __DCACHE_PRESENT
// Up to 16 MPU regions (M7 TRM 1.1.3, M55 TRM 8.3), followed by the ITCM, DTCM
// and their aliases.
#define OMV_MEMORY_REGIONS_MAX      (16 + 4)

typedef struct {
    uint64_t base;
    uint64_t limit;
    uint8_t srd;        // ARMv7-M disabled sub-regions.
    uint8_t policy;
} omv_memory_region_t;

typedef struct {
    bool init;
    bool force_wt;
    size_t count;
    omv_memory_region_t region[OMV_MEMORY_REGIONS_MAX];
} omv_memory_config_t;

static omv_memory_config_t omv_memory_config;

// Default memory map cache policy, per 512MB region (ARMv8-M ARM B8.1, PM0253
// Table 17). The 0x60000000 region is WBWA on ARMv8-M, PM0253 lists no policy
// for it on the M7, so it's treated as WBWA on both.
static const uint8_t omv_memory_default_policy[8] = {
    OMV_MEMORY_POLICY_WT,    // 0x00000000 Code
    OMV_MEMORY_POLICY_WB,    // 0x20000000 SRAM
    OMV_MEMORY_POLICY_NONE,  // 0x40000000 Peripheral
    OMV_MEMORY_POLICY_WB,    // 0x60000000 RAM
    OMV_MEMORY_POLICY_WT,    // 0x80000000 RAM
    OMV_MEMORY_POLICY_NONE,  // 0xA0000000 Device
    OMV_MEMORY_POLICY_NONE,  // 0xC0000000 Device
    OMV_MEMORY_POLICY_NONE,  // 0xE0000000 System
};

static void omv_memory_region_add(uint64_t base, uint64_t limit, uint8_t srd, omv_memory_policy_t policy) {
    if (omv_memory_config.count < OMV_MEMORY_REGIONS_MAX) {
        omv_memory_region_t *r = &omv_memory_config.region[omv_memory_config.count++];
        r->base = base;
        r->limit = limit;
        r->srd = srd;
        r->policy = policy;
    }
}

static uint64_t omv_memory_tcm_size(uint32_t tcmcr) {
    // ITCMCR/DTCMCR: EN [0], SZ [6:3]. SZ 0b0011 = 4KB .. 0b1111 = 16MB,
    // 0b0000 = no TCM (M7 TRM 3.3.6, M55 TRM 4.19).
    uint32_t sz = (tcmcr >> 3) & 0xF;
    if (!(tcmcr & 1) || sz < 3) {
        return 0;
    }
    return 1ULL << (sz + 9);
}

// Adds the TCM at base, and its alias if any. TCMs are never cached,
// regardless of the MPU attributes (M7 TRM 5.8.1, M55 TRM 9.8.2).
static void omv_memory_tcm_region_add(uint32_t base, uint32_t tcmcr, uint32_t alias) {
    uint64_t size = omv_memory_tcm_size(tcmcr);
    if (size) {
        omv_memory_region_add(base, base + size, 0, OMV_MEMORY_POLICY_NONE);
        if (alias) {
            omv_memory_region_add(base | alias, (base | alias) + size, 0, OMV_MEMORY_POLICY_NONE);
        }
    }
}

#if __MPU_PRESENT
#if (__ARM_ARCH == 7)
// ARMv7-M inner cache policy, indexed by C:B when TEX = 0b1BB (PM0253 Table 92).
static const uint8_t omv_memory_inner_policy[4] = {
    OMV_MEMORY_POLICY_NONE,
    OMV_MEMORY_POLICY_WB,
    OMV_MEMORY_POLICY_WT,
    OMV_MEMORY_POLICY_WB
};

// Adds ARMv7-M MPU region rnr if it's enabled.
static void omv_memory_mpu_region_add_v7(uint32_t rnr) {
    omv_memory_policy_t policy;
    MPU->RNR = rnr;
    uint32_t rbar = MPU->RBAR;
    uint32_t rasr = MPU->RASR;

    if (!(rasr & MPU_RASR_ENABLE_Msk)) {
        return;
    }

    uint64_t size = 1ULL << (((rasr & MPU_RASR_SIZE_Msk) >> MPU_RASR_SIZE_Pos) + 1);
    uint64_t base = rbar & MPU_RBAR_ADDR_Msk & ~(size - 1);
    // Regions of 256 bytes or more are split into 8 sub-regions.
    uint8_t srd = (size >= 256) ? (rasr & MPU_RASR_SRD_Msk) >> MPU_RASR_SRD_Pos : 0;

    // PM0253 Table 91 and Table 92.
    uint32_t tex = (rasr & MPU_RASR_TEX_Msk) >> MPU_RASR_TEX_Pos;
    uint32_t cb = ((rasr & MPU_RASR_C_Msk) ? 2 : 0) | ((rasr & MPU_RASR_B_Msk) ? 1 : 0);

    if (tex & 0x4) {
        // TEX = 0b1BB, C:B = AA is the inner cache policy.
        policy = omv_memory_inner_policy[cb];
    } else {
        switch ((tex << 2) | cb) {
            case 0x0:   // Strongly-ordered.
            case 0x1:   // Shareable device.
            case 0x4:   // Normal, non-cacheable.
            case 0x8:   // Non-shareable device.
                policy = OMV_MEMORY_POLICY_NONE;
                break;
            case 0x2:   // Normal, write-through, no write allocate.
                policy = OMV_MEMORY_POLICY_WT;
                break;
            case 0x3:   // Normal, write-back, no write allocate.
            case 0x7:   // Normal, write-back, write and read allocate.
                policy = OMV_MEMORY_POLICY_WB;
                break;
            default:
                policy = OMV_MEMORY_POLICY_UNKNOWN;
                break;
        }
    }

    omv_memory_region_add(base, base + size, srd, policy);
}
#elif (__ARM_ARCH == 8)
// Adds ARMv8-M MPU region rnr if it's enabled.
static void omv_memory_mpu_region_add_v8(uint32_t rnr) {
    omv_memory_policy_t policy;
    MPU->RNR = rnr;
    uint32_t rbar = MPU->RBAR;
    uint32_t rlar = MPU->RLAR;

    if (!(rlar & MPU_RLAR_EN_Msk)) {
        return;
    }

    // ARMv8-M ARM D1.2 MAIR_ATTR. Outer 0b0000 is Device memory,
    // otherwise the inner attributes define the L1 cache policy.
    uint32_t index = (rlar & MPU_RLAR_AttrIndx_Msk) >> MPU_RLAR_AttrIndx_Pos;
    uint32_t mair = (index < 4) ? MPU->MAIR0 : MPU->MAIR1;
    uint32_t attr = (mair >> ((index % 4) * 8)) & 0xFF;
    uint32_t outer = (attr >> 4) & 0xF;
    uint32_t inner = attr & 0xF;

    if (outer == 0) {
        policy = OMV_MEMORY_POLICY_NONE;
    } else {
        switch (inner >> 2) {
            case 0:     // 0b00RW, write-through transient, 0b0000 is unpredictable.
                policy = inner ? OMV_MEMORY_POLICY_WT : OMV_MEMORY_POLICY_UNKNOWN;
                break;
            case 1:     // 0b0100 non-cacheable, 0b01RW write-back transient.
                policy = (inner == 0x4) ? OMV_MEMORY_POLICY_NONE : OMV_MEMORY_POLICY_WB;
                break;
            case 2:     // 0b10RW, write-through non-transient.
                policy = OMV_MEMORY_POLICY_WT;
                break;
            default:    // 0b11RW, write-back non-transient.
                policy = OMV_MEMORY_POLICY_WB;
                break;
        }
    }

    omv_memory_region_add(rbar & MPU_RBAR_BASE_Msk, (uint64_t) (rlar & MPU_RLAR_LIMIT_Msk) + 0x20, 0, policy);
}
#else
#error "Unsupported architecture"
#endif
#endif // __MPU_PRESENT

static void omv_memory_init(void) {
    #if __MPU_PRESENT
    if (MPU->CTRL & MPU_CTRL_ENABLE_Msk) {
        uint32_t regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;
        for (uint32_t rnr = 0; rnr < regions; rnr++) {
            #if (__ARM_ARCH == 7)
            omv_memory_mpu_region_add_v7(rnr);
            #else
            omv_memory_mpu_region_add_v8(rnr);
            #endif
        }
    }
    #endif

    // The TCMs are always at 0x00000000 (ITCM) and 0x20000000 (DTCM).
    #if (__CORTEX_M == 7)
    omv_memory_tcm_region_add(0x00000000, SCB->ITCMCR, 0);
    omv_memory_tcm_region_add(0x20000000, SCB->DTCMCR, 0);
    omv_memory_config.force_wt = SCB->CACR & SCB_CACR_FORCEWT_Msk;
    #elif (__CORTEX_M == 55)
    // CFGMEMALIAS (CFGINFOSEL 0x4C) is one-hot: bit n selects alias bit 24 + n
    // (M55 TRM 4.17). Zero means aliasing is disabled.
    PRCCFGINF->CFGINFOSEL = 0x4C;
    uint32_t alias = PRCCFGINF->CFGINFORD & 0x1F;
    alias = alias ? (1UL << (24 + __builtin_ctz(alias))) : 0;
    omv_memory_tcm_region_add(0x00000000, MEMSYSCTL->ITCMCR, alias);
    omv_memory_tcm_region_add(0x20000000, MEMSYSCTL->DTCMCR, alias);
    omv_memory_config.force_wt = MEMSYSCTL->MSCR & MEMSYSCTL_MSCR_FORCEWT_Msk;
    #endif

    omv_memory_config.init = true;
}

// Lowers *next to boundary if it's above addr.
static inline void omv_memory_update_next(uint64_t addr, uint64_t boundary, uint64_t *next) {
    if (boundary > addr && boundary < *next) {
        *next = boundary;
    }
}

// Returns the cache policy at addr, and in *next the first address
// above addr where the policy may change.
static omv_memory_policy_t omv_memory_policy_at(uint64_t addr, uint64_t *next) {
    *next = ((addr >> 29) + 1) << 29;
    omv_memory_policy_t policy = omv_memory_default_policy[addr >> 29];

    // Higher numbered regions take priority on ARMv7-M, regions can't overlap
    // on ARMv8-M, and the TCMs come last, so the last match wins.
    for (size_t i = 0; i < omv_memory_config.count; i++) {
        const omv_memory_region_t *r = &omv_memory_config.region[i];
        omv_memory_update_next(addr, (addr < r->base) ? r->base : r->limit, next);

        if (addr < r->base || addr >= r->limit) {
            continue;
        }

        if (r->srd) {
            uint64_t sub_size = (r->limit - r->base) / 8;
            uint32_t sub = (addr - r->base) / sub_size;
            omv_memory_update_next(addr, r->base + (sub + 1) * sub_size, next);
            if (r->srd & (1 << sub)) {
                continue;
            }
        }

        policy = r->policy;
    }

    return policy;
}

// Returns the strongest cache policy (NONE < WT < WB) found in the range.
static omv_memory_policy_t omv_memory_range_policy(const void *addr, size_t size) {
    omv_memory_policy_t policy = OMV_MEMORY_POLICY_NONE;

    if (size == 0 || !(SCB->CCR & SCB_CCR_DC_Msk)) {
        return OMV_MEMORY_POLICY_NONE;
    }

    if (!omv_memory_config.init) {
        omv_memory_init();
    }

    uint64_t start = (uintptr_t) addr;
    uint64_t end = start + size;

    if (end > (1ULL << 32)) {
        end = 1ULL << 32;
    }

    // Walk the range one segment at a time, each segment has a single policy.
    while (start < end && policy != OMV_MEMORY_POLICY_WB) {
        uint64_t next;
        omv_memory_policy_t p = omv_memory_policy_at(start, &next);
        if (p > policy) {
            policy = p;
        }
        start = next;
    }

    // Force write-through treats write-back regions as write-through.
    if (policy == OMV_MEMORY_POLICY_WB && omv_memory_config.force_wt) {
        policy = OMV_MEMORY_POLICY_WT;
    }

    return policy;
}
#else
static omv_memory_policy_t omv_memory_range_policy(const void *addr, size_t size) {
    return OMV_MEMORY_POLICY_NONE;
}
#endif // __DCACHE_PRESENT

bool omv_memory_is_cacheable(const void *addr, size_t size) {
    return omv_memory_range_policy(addr, size) != OMV_MEMORY_POLICY_NONE;
}

bool omv_memory_is_write_back(const void *addr, size_t size) {
    return omv_memory_range_policy(addr, size) == OMV_MEMORY_POLICY_WB;
}

__attribute__((weak)) bool omv_memory_is_dmable(const void *master, const void *addr, size_t size) {
    return size != 0;
}

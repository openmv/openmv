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
 * Bootloader MPU driver.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "omv_bootconfig.h"

#ifndef __weak
#define __weak  __attribute__((weak))
#endif

#if (__ARM_ARCH >= 8)
static const uint8_t mpu_attr_table[MEMATTR_COUNT][2] = {
    /* Device Memory */
    [MEMATTR_DEVICE_nGnRE] = {
        ARM_MPU_ATTR_DEVICE,
        ARM_MPU_ATTR_DEVICE_nGnRE
    },
    /* NT=0, WB=0, RA=1, WA=0 */
    [MEMATTR_NORMAL_RA] = {
        ARM_MPU_ATTR_MEMORY_(0, 0, 1, 0),
        ARM_MPU_ATTR_MEMORY_(0, 0, 1, 0)
    },
    /* NT=1, WB=1, RA=1, WA=1 */
    [MEMATTR_NORMAL_WB_RA_WA] = {
        ARM_MPU_ATTR_MEMORY_(1, 1, 1, 1),
        ARM_MPU_ATTR_MEMORY_(1, 1, 1, 1)
    },
    /* NT=1, WB=0, RA=1, WA=0 */
    [MEMATTR_NORMAL_NT_RA] = {
        ARM_MPU_ATTR_MEMORY_(1, 0, 1, 0),
        ARM_MPU_ATTR_MEMORY_(1, 0, 1, 0)
    },
    /* NT=0, WB=1, RA=0, WA=0 */
    [MEMATTR_NORMAL_NCACHE] = {
        ARM_MPU_ATTR_MEMORY_(0, 1, 0, 0),
        ARM_MPU_ATTR_MEMORY_(0, 1, 0, 0)
    }
};
#endif

__weak void port_mpu_init(void) {
    #if (__ARM_ARCH >= 8)
    // Disable IRQs.
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    // Disable MPU
    ARM_MPU_Disable();

    // Clear all regions.
    for (size_t i = 0; i < (MPU->TYPE >> 8); i++) {
        ARM_MPU_ClrRegion(i);
    }

    // Load memory attributes.
    for (size_t i = 0; i < MEMATTR_COUNT; i++) {
        ARM_MPU_SetMemAttr(i, ARM_MPU_ATTR(mpu_attr_table[i][0], mpu_attr_table[i][1]));
    }

    // Load default regions.
    port_mpu_load_defaults();

    // Re-enable MPU
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk);

    // Re-enable IRQs.
    __set_PRIMASK(primask);
    #endif

    // Configure read-only MPU regions for boot partitions.
    for (size_t i = 0; i < OMV_BOOT_PARTITIONS_COUNT; i++) {
        port_mpu_config(&OMV_BOOT_PARTITIONS[i], 1, 1, 1);
    }
}

__weak void port_mpu_deinit() {
    #if (__ARM_ARCH >= 8)
    ARM_MPU_Disable();
    // Disable all regions.
    for (size_t i = 0; i < OMV_BOOT_PARTITIONS_COUNT; i++) {
        if (OMV_BOOT_PARTITIONS[i].region != -1) {
            ARM_MPU_ClrRegion(OMV_BOOT_PARTITIONS[i].region);
        }
    }

    // Configure XIP MPU regions (if any).
    for (size_t i = 0; i < OMV_BOOT_PARTITIONS_COUNT; i++) {
        if (OMV_BOOT_PARTITIONS[i].type == PTYPE_XIP_FLASH) {
            port_mpu_config(&OMV_BOOT_PARTITIONS[i], 1, 1, 0);
        }
    }
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk);
    #endif
}

__weak void port_mpu_load_defaults() {

}

__weak void port_mpu_config(const partition_t *p, bool ro, bool np, bool xn) {
    #if (__ARM_ARCH >= 8)
    if (p->region != -1) {
        ARM_MPU_Disable();
        MPU->RNR = p->region;
        MPU->RBAR = ARM_MPU_RBAR(p->start, ARM_MPU_SH_NON, ro, np, xn);
        MPU->RLAR = ARM_MPU_RLAR(p->limit - 1, p->attr);
        ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk);
    }
    #endif
}

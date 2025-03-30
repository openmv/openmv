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
 * TinyUSB config.
 */
#ifndef __ALIF_TUSB_CONFIG_H__
#define __ALIF_TUSB_CONFIG_H__
// Common configuration
#define CFG_TUSB_OS                 OPT_OS_NONE
#define CFG_TUSB_MCU                OPT_MCU_NONE
#define CFG_TUSB_RHPORT0_MODE       (OPT_MODE_DEVICE | OPT_MODE_HIGH_SPEED)
#define TUP_DCD_ENDPOINT_MAX        (8)

// Device stack configuration.
#define CFG_TUD_ENABLED             (1)
#define CFG_TUD_DFU                 (1)
#define CFG_TUD_ENDPOINT0_SIZE      (64)
#define CFG_TUD_DFU_XFER_BUFSIZE    (4096)

// Peripherals in expansion master 0 (such as USB, Ethernet, SD/MMC)
// are by default configured as non-secure, so they don't have access
// to DTCMs. These peripherals should place buffers in regular SRAM.
#define CFG_TUSB_MEM_SECTION    __attribute__((section(".bss.sram0")))
#define CFG_TUSB_MEM_ALIGN      __attribute__ ((aligned(256)))
#endif //__ALIF_TUSB_CONFIG_H__

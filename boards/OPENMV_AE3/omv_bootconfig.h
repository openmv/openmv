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
 * Bootloader configuration.
 */
// *INDENT-OFF*
#ifndef __OMV_BOOTCONFIG_H__
#define __OMV_BOOTCONFIG_H__
#include ALIF_CMSIS_H
#include "port.h"
#include "gpio.h"
#include "pinconf.h"

// Misc config.
#define OMV_BOOT_VID                (0x37C5)
#define OMV_BOOT_PID                (0x96E3)
#define OMV_BOOT_DFU_TIMEOUT        (1500)
#define OMV_BOOT_MAGIC_ADDR         (0x200FFFFCU)

// Flash config.
#define OMV_BOOT_AXI_FLASH_ENABLE   (1)
#define OMV_BOOT_SPI_FLASH_ENABLE   (1)
#define OMV_BOOT_REC_FLASH_ENABLE   (1)

// OSPI config.
#define OMV_BOOT_OSPI_SER           (1)
#define OMV_BOOT_OSPI_RX_DELAY      (4)
#define OMV_BOOT_OSPI_DDR_EDGE      (0)
#define OMV_BOOT_OSPI_RXDS_DELAY    (12)
#define OMV_BOOT_OSPI_CLOCK         (50000000)
#define OMV_BOOT_OSPI_SSI_BASE      (OSPI0_BASE)
#define OMV_BOOT_OSPI_AES_BASE      (AES0_BASE)
#define OMV_BOOT_OSPI_XIP_BASE      (OSPI0_XIP_BASE)

// IRQ priorities.
#define NVIC_PRIORITYGROUP_7        ((uint32_t)0x00000000U)
#define OMV_BOOT_TIM_IRQ_PRI        NVIC_EncodePriority(NVIC_PRIORITYGROUP_7, 0, 0)
#define OMV_BOOT_USB_IRQ_PRI        NVIC_EncodePriority(NVIC_PRIORITYGROUP_7, 5, 0)

// Board GPIO pins (NOTE: indices in the pins table)
#define OMV_BOOT_MUX_PIN            (0)
#define OMV_BOOT_LED_PIN            (1)
#define OMV_BOOT_OSPI_RST_PIN       (2)
#define OMV_BOOT_OSPI_D0_PIN        (3)
#define OMV_BOOT_OSPI_CS_PIN        (13)

#define OMV_BOOT_PAD_IO             (PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA)
#define OMV_BOOT_PAD_Dx             (PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST | PADCTRL_READ_ENABLE)
#define OMV_BOOT_PAD_D2             (OMV_BOOT_PAD_Dx | PADCTRL_DRIVER_DISABLED_PULL_UP)
#define OMV_BOOT_PAD_Cx             (PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST)
#define OMV_BOOT_ALT_GPIO           PINMUX_ALTERNATE_FUNCTION_0
#define OMV_BOOT_ALT_OSPI           PINMUX_ALTERNATE_FUNCTION_1
#define ALIF_GPIO(x)                ((GPIO_Type *) x##_BASE)

static const pin_t omv_boot_pins[] = {
  { .gpio = ALIF_GPIO(LPGPIO), .port = PORT_15, .pin = PIN_0, .alt = OMV_BOOT_ALT_GPIO, .pad = OMV_BOOT_PAD_IO }, /* MUX */
  { .gpio = ALIF_GPIO(GPIO6),  .port = PORT_6,  .pin = PIN_3, .alt = OMV_BOOT_ALT_GPIO, .pad = OMV_BOOT_PAD_IO }, /* LED */
  { .gpio = ALIF_GPIO(GPIO3),  .port = PORT_3,  .pin = PIN_3, .alt = OMV_BOOT_ALT_GPIO, .pad = OMV_BOOT_PAD_IO }, /* RST */
  { .gpio = ALIF_GPIO(GPIO2),  .port = PORT_2,  .pin = PIN_0, .alt = OMV_BOOT_ALT_OSPI, .pad = OMV_BOOT_PAD_Dx }, /* D0  */
  { .gpio = ALIF_GPIO(GPIO2),  .port = PORT_2,  .pin = PIN_1, .alt = OMV_BOOT_ALT_OSPI, .pad = OMV_BOOT_PAD_Dx }, /* D1  */
  { .gpio = ALIF_GPIO(GPIO2),  .port = PORT_2,  .pin = PIN_2, .alt = OMV_BOOT_ALT_OSPI, .pad = OMV_BOOT_PAD_D2 }, /* D2  */
  { .gpio = ALIF_GPIO(GPIO2),  .port = PORT_2,  .pin = PIN_3, .alt = OMV_BOOT_ALT_OSPI, .pad = OMV_BOOT_PAD_Dx }, /* D3  */
  { .gpio = ALIF_GPIO(GPIO2),  .port = PORT_2,  .pin = PIN_4, .alt = OMV_BOOT_ALT_OSPI, .pad = OMV_BOOT_PAD_Dx }, /* D4  */
  { .gpio = ALIF_GPIO(GPIO2),  .port = PORT_2,  .pin = PIN_5, .alt = OMV_BOOT_ALT_OSPI, .pad = OMV_BOOT_PAD_Dx }, /* D5  */
  { .gpio = ALIF_GPIO(GPIO2),  .port = PORT_2,  .pin = PIN_6, .alt = OMV_BOOT_ALT_OSPI, .pad = OMV_BOOT_PAD_Dx }, /* D6  */
  { .gpio = ALIF_GPIO(GPIO2),  .port = PORT_2,  .pin = PIN_7, .alt = OMV_BOOT_ALT_OSPI, .pad = OMV_BOOT_PAD_Dx }, /* D7  */
  { .gpio = ALIF_GPIO(GPIO1),  .port = PORT_1,  .pin = PIN_6, .alt = OMV_BOOT_ALT_OSPI, .pad = OMV_BOOT_PAD_Dx }, /* RXD */
  { .gpio = ALIF_GPIO(GPIO3),  .port = PORT_3,  .pin = PIN_0, .alt = OMV_BOOT_ALT_OSPI, .pad = OMV_BOOT_PAD_Cx }, /* CK  */
  { .gpio = ALIF_GPIO(GPIO3),  .port = PORT_3,  .pin = PIN_2, .alt = OMV_BOOT_ALT_OSPI, .pad = OMV_BOOT_PAD_Cx }, /* CS  */
};
#define OMV_BOOT_PINS_COUNT     (sizeof(omv_boot_pins) / sizeof(omv_boot_pins[0]))

// Note the first MPU regions are used by startup code.
static const partition_t OMV_BOOT_DFU_PARTITIONS[] = {
{ .type = PTYPE_AXI_FLASH, .region= 4, .rdonly = 1, .start = 0x80000000, .limit = 0x80020000, .attr = MEMATTR_NORMAL_RA },
{ .type = PTYPE_AXI_FLASH, .region= 5, .rdonly = 0, .start = 0x80020000, .limit = 0x80320000, .attr = MEMATTR_DEVICE_nGnRE },
{ .type = PTYPE_AXI_FLASH, .region= 6, .rdonly = 0, .start = 0x80320000, .limit = 0x8047E000, .attr = MEMATTR_DEVICE_nGnRE },
{ .type = PTYPE_AXI_FLASH, .region= 7, .rdonly = 0, .start = 0x8047E000, .limit = 0x8057E000, .attr = MEMATTR_DEVICE_nGnRE },
{ .type = PTYPE_AXI_FLASH, .region= 8, .rdonly = 0, .start = 0x8057E000, .limit = 0x80580000, .attr = MEMATTR_DEVICE_nGnRE },
{ .type = PTYPE_SPI_FLASH, .region=-1, .rdonly = 0, .start = 0x00000000, .limit = 0x00800000, .attr = 0 },
{ .type = PTYPE_SPI_FLASH, .region=-1, .rdonly = 0, .start = 0x00800000, .limit = 0x02000000, .attr = 0 },
{ .type = PTYPE_REC_FLASH, .region=-1, .rdonly = 0, .start = 0x00000000, .limit = 0x00001000, .attr = 0 },
};
#define OMV_BOOT_DFU_PARTITIONS_COUNT   8   // Must be a literal
#define OMV_BOOT_DFU_PARTITIONS_STR     "BOOT", "HP", "HE", "ROMFS1", "TOC", "RWFS", "ROMFS0", "RECOVERY"

// Protects MRAM before jump to main firmware
static const partition_t OMV_BOOT_XIP_PARTITIONS[] = {
{ .type = PTYPE_XIP_FLASH, .region= 4, .rdonly = 1, .start = 0x80000000, .limit = 0x80580000, .attr = MEMATTR_NORMAL_RA },
};
#define OMV_BOOT_XIP_PARTITIONS_COUNT   (sizeof(OMV_BOOT_XIP_PARTITIONS) / sizeof(OMV_BOOT_XIP_PARTITIONS[0]))

#endif  // __OMV_BOOTCONFIG_H__

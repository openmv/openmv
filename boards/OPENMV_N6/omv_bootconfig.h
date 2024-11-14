/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
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
 * Board configuration and pin definitions.
 */
// *INDENT-OFF*
#ifndef __OMV_BOOTCONFIG_H__
#define __OMV_BOOTCONFIG_H__
#include STM32_HAL_H
#include "Legacy/stm32_hal_legacy.h"
#include "port.h"

// Misc config.
#define OMV_BOOT_VID                (0x37C5)
#define OMV_BOOT_PID                (0x9206)
#define OMV_BOOT_DFU_TIMEOUT        (1500)
#define OMV_BOOT_MAGIC_ADDR         (0x3401FFFCU)
#define OMV_BOOT_LED_PIN            (0) // index in omv_boot_pins

// Flash config.
#define OMV_BOOT_AXI_FLASH_ENABLE   (0)
#define OMV_BOOT_SPI_FLASH_ENABLE   (1)
#define OMV_BOOT_SPI_FLASH_MMAP     (1) // Memory-map the flash on exit
#define OMV_BOOT_SPI_FLASH_MMAP_DTR (1)

// OSPI/XSPI config.
#define OMV_BOOT_XSPI_INSTANCE      (2)
#define OMV_BOOT_XSPI_FREQUENCY     (200000000)
#define OMV_BOOT_XSPI_FLASH_SIZE    (0x2000000) // Must be a power of 2.
#define OMV_BOOT_XSPI_FLASH_RST_PIN (13) // index in omv_boot_pins

#define GPIO_SPEED_LOW              GPIO_SPEED_FREQ_LOW
#define GPIO_SPEED_HIGH             GPIO_SPEED_FREQ_VERY_HIGH

// Boot I/O pins.
static const pin_t omv_boot_pins[] = {
    { .gpio = GPIOA, .pin = GPIO_PIN_7,  .speed = GPIO_SPEED_LOW,  .mode = GPIO_MODE_OUTPUT_PP, .pull = GPIO_PULLUP  },
    { .gpio = GPION, .pin = GPIO_PIN_0,  .speed = GPIO_SPEED_HIGH, .mode = GPIO_MODE_AF_PP, .alt = GPIO_AF9_XSPIM_P2 },
    { .gpio = GPION, .pin = GPIO_PIN_1,  .speed = GPIO_SPEED_HIGH, .mode = GPIO_MODE_AF_PP, .alt = GPIO_AF9_XSPIM_P2 },
    { .gpio = GPION, .pin = GPIO_PIN_2,  .speed = GPIO_SPEED_HIGH, .mode = GPIO_MODE_AF_PP, .alt = GPIO_AF9_XSPIM_P2 },
    { .gpio = GPION, .pin = GPIO_PIN_3,  .speed = GPIO_SPEED_HIGH, .mode = GPIO_MODE_AF_PP, .alt = GPIO_AF9_XSPIM_P2 },
    { .gpio = GPION, .pin = GPIO_PIN_4,  .speed = GPIO_SPEED_HIGH, .mode = GPIO_MODE_AF_PP, .alt = GPIO_AF9_XSPIM_P2 },
    { .gpio = GPION, .pin = GPIO_PIN_5,  .speed = GPIO_SPEED_HIGH, .mode = GPIO_MODE_AF_PP, .alt = GPIO_AF9_XSPIM_P2 },
    { .gpio = GPION, .pin = GPIO_PIN_6,  .speed = GPIO_SPEED_HIGH, .mode = GPIO_MODE_AF_PP, .alt = GPIO_AF9_XSPIM_P2 },
    { .gpio = GPION, .pin = GPIO_PIN_7,  .speed = GPIO_SPEED_HIGH, .mode = GPIO_MODE_AF_PP, .alt = GPIO_AF9_XSPIM_P2 },
    { .gpio = GPION, .pin = GPIO_PIN_8,  .speed = GPIO_SPEED_HIGH, .mode = GPIO_MODE_AF_PP, .alt = GPIO_AF9_XSPIM_P2 },
    { .gpio = GPION, .pin = GPIO_PIN_9,  .speed = GPIO_SPEED_HIGH, .mode = GPIO_MODE_AF_PP, .alt = GPIO_AF9_XSPIM_P2 },
    { .gpio = GPION, .pin = GPIO_PIN_10, .speed = GPIO_SPEED_HIGH, .mode = GPIO_MODE_AF_PP, .alt = GPIO_AF9_XSPIM_P2 },
    { .gpio = GPION, .pin = GPIO_PIN_11, .speed = GPIO_SPEED_HIGH, .mode = GPIO_MODE_AF_PP, .alt = GPIO_AF9_XSPIM_P2 },
    { .gpio = GPION, .pin = GPIO_PIN_12, .speed = GPIO_SPEED_LOW,  .mode = GPIO_MODE_OUTPUT_PP, .pull = GPIO_PULLUP  },
};
#define OMV_BOOT_PINS_COUNT         (sizeof(omv_boot_pins) / sizeof(omv_boot_pins[0]))

// Boot partitions.
static const partition_t OMV_BOOT_DFU_PARTITIONS[] = {
  { .type = PTYPE_SPI_FLASH, .region = -1, .rdonly = 1, .start = 0x00000000, .limit = 0x00080000, .attr = 0 }, // Boot
  { .type = PTYPE_SPI_FLASH, .region = -1, .rdonly = 0, .start = 0x00080000, .limit = 0x00400000, .attr = 0 }, // FIRMWARE
  { .type = PTYPE_SPI_FLASH, .region = -1, .rdonly = 0, .start = 0x00400000, .limit = 0x00800000, .attr = 0 }, // FILESYSTEM 
  { .type = PTYPE_SPI_FLASH, .region = -1, .rdonly = 0, .start = 0x00800000, .limit = 0x02000000, .attr = 0 }, // ROMFS0
};
#define OMV_BOOT_DFU_PARTITIONS_COUNT   4 // Must be a literal
#define OMV_BOOT_DFU_PARTITIONS_STR     "BOOTLOADER", "FIRMWARE", "FILESYSTEM", "ROMFS0"

// XIP flash, not used for DFU.  
static const partition_t OMV_BOOT_XIP_PARTITIONS[] = {
  { .type = PTYPE_XIP_FLASH, .region = 0, .rdonly = 1, .start = 0x70000000, .limit = 0x78000000, .attr = MEMATTR_NORMAL_WB_RA_WA },
};
#define OMV_BOOT_XIP_PARTITIONS_COUNT   (sizeof(OMV_BOOT_XIP_PARTITIONS) / sizeof(OMV_BOOT_XIP_PARTITIONS[0]))
#endif //__OMV_BOOTCONFIG_H__

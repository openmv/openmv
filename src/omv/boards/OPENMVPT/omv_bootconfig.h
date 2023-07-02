/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2022 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2022 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Board configuration and pin definitions.
 */
#ifndef __OMV_BOOTCONFIG_H__
#define __OMV_BOOTCONFIG_H__

// Bootloader version.
#define OMV_BOOT_VERSION                   (0xABCD0003)

// Bootloader LED GPIO config.
#define OMV_BOOT_LED_PIN                   (GPIO_PIN_1)
#define OMV_BOOT_LED_PORT                  (GPIOC)

// Flash layout for the bootloader.
// Flash FS sector, main FW sector, max sector.
#define OMV_BOOT_FLASH_LAYOUT              {1, 2, 15}

// Flash configuration.
#define OMV_BOOT_FLASH_ORIGIN              0x08000000
#define OMV_BOOT_FLASH_LENGTH              128K

// QSPI Flash layout for the bootloader.
// First block, maximum block, block size in bytes.
#define OMV_BOOT_QSPIF_LAYOUT              {0, 511, 64 * 1024}

// QSPI Flash configuration.
#define OMV_BOOT_QSPIF_SIZE_BITS           (25)     // 2**25 == 32MBytes.
#define OMV_BOOT_QSPIF_SR_WIP_MASK         (1 << 0)
#define OMV_BOOT_QSPIF_SR_WEL_MASK         (1 << 1)
#define OMV_BOOT_QSPIF_READ_QUADIO_DCYC    (6)

#define OMV_BOOT_QSPIF_PAGE_SIZE           (0x100)  // 256 bytes pages.
#define OMV_BOOT_QSPIF_NUM_PAGES           (0x20000) // 131072 pages of 256 bytes

#define OMV_BOOT_QSPIF_SECTOR_SIZE         (0x1000) // 4K bytes sectors.
#define OMV_BOOT_QSPIF_NUM_SECTORS         (0x2000) // 8192 sectors of 4K bytes

#define OMV_BOOT_QSPIF_BLOCK_SIZE          (0x10000) // 64K bytes blocks.
#define OMV_BOOT_QSPIF_NUM_BLOCKS          (0x200)  // 512 blocks of 64K bytes

#define OMV_BOOT_QSPIF_CLK_PIN             (GPIO_PIN_10)
#define OMV_BOOT_QSPIF_CLK_PORT            (GPIOF)
#define OMV_BOOT_QSPIF_CLK_ALT             (GPIO_AF9_QUADSPI)

#define OMV_BOOT_QSPIF_CS_PIN              (GPIO_PIN_6)
#define OMV_BOOT_QSPIF_CS_PORT             (GPIOG)
#define OMV_BOOT_QSPIF_CS_ALT              (GPIO_AF10_QUADSPI)

#define OMV_BOOT_QSPIF_D0_PIN              (GPIO_PIN_8)
#define OMV_BOOT_QSPIF_D0_PORT             (GPIOF)
#define OMV_BOOT_QSPIF_D0_ALT              (GPIO_AF10_QUADSPI)

#define OMV_BOOT_QSPIF_D1_PIN              (GPIO_PIN_9)
#define OMV_BOOT_QSPIF_D1_PORT             (GPIOF)
#define OMV_BOOT_QSPIF_D1_ALT              (GPIO_AF10_QUADSPI)

#define OMV_BOOT_QSPIF_D2_PIN              (GPIO_PIN_7)
#define OMV_BOOT_QSPIF_D2_PORT             (GPIOF)
#define OMV_BOOT_QSPIF_D2_ALT              (GPIO_AF9_QUADSPI)

#define OMV_BOOT_QSPIF_D3_PIN              (GPIO_PIN_6)
#define OMV_BOOT_QSPIF_D3_PORT             (GPIOF)
#define OMV_BOOT_QSPIF_D3_ALT              (GPIO_AF9_QUADSPI)

#define OMV_BOOT_QSPIF_CLK_ENABLE()        __HAL_RCC_QSPI_CLK_ENABLE()
#define OMV_BOOT_QSPIF_CLK_DISABLE()       __HAL_RCC_QSPI_CLK_DISABLE()
#define OMV_BOOT_QSPIF_FORCE_RESET()       __HAL_RCC_QSPI_FORCE_RESET()
#define OMV_BOOT_QSPIF_RELEASE_RESET()     __HAL_RCC_QSPI_RELEASE_RESET()

#endif //__OMV_BOOTCONFIG_H__

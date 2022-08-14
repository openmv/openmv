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
#define OMV_BOOT_VERSION            (0xABCD0003)

// Bootloader LED GPIO config.
#define OMV_BOOT_LED_PIN            (GPIO_PIN_1)
#define OMV_BOOT_LED_PORT           (GPIOC)

// Flash layout for the bootloader.
// Flash FS sector, main FW sector, max sector.
#define OMV_BOOT_FLASH_LAYOUT       {1, 2, 15}

// Flash configuration.
#define OMV_BOOT_FLASH_ORIGIN       0x08000000
#define OMV_BOOT_FLASH_LENGTH       128K

#endif //__OMV_BOOTCONFIG_H__

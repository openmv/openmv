/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Board configuration and pin definitions.
 */
#ifndef __OMV_BOARDCONFIG_H__
#define __OMV_BOARDCONFIG_H__

// Architecture info
#define OMV_ARCH_STR            "NANO33 M4" // 33 chars max
#define OMV_BOARD_TYPE          "NANO33"
#define OMV_UNIQUE_ID_ADDR      0x10000060  // Unique ID address.
#define OMV_UNIQUE_ID_SIZE      2           // Unique ID size in words.
#define OMV_UNIQUE_ID_OFFSET    4           // Bytes offset for multi-word UIDs.

#define OMV_XCLK_MCO            (0U)
#define OMV_XCLK_TIM            (1U)

// Sensor external clock source.
#define OMV_XCLK_SOURCE         (OMV_XCLK_TIM)

// Sensor external clock timer frequency.
#define OMV_XCLK_FREQUENCY      (12000000)

// Enable hardware JPEG
#define OMV_HARDWARE_JPEG       (0)

// Enable sensor drivers
#define OMV_ENABLE_OV2640       (0)
#define OMV_ENABLE_OV5640       (0)
#define OMV_ENABLE_OV7670       (1)
#define OMV_ENABLE_OV7690       (0)
#define OMV_ENABLE_OV7725       (0)
#define OMV_ENABLE_OV9650       (0)
#define OMV_ENABLE_MT9M114      (0)
#define OMV_ENABLE_MT9V0XX      (0)
#define OMV_ENABLE_LEPTON       (0)
#define OMV_ENABLE_HM01B0       (0)
#define OMV_ENABLE_PAJ6100      (0)
#define OMV_ENABLE_FROGEYE2020  (0)

// Set which OV767x sensor is used
#define OMV_OV7670_VERSION      (75)

// OV7670 clock divider
#define OMV_OV7670_CLKRC        (0x01)

// Enable sensor features
#define OMV_ENABLE_OV5640_AF    (0)

// Enable WiFi debug
#define OMV_ENABLE_WIFIDBG      (0)
#define OMV_ENABLE_TUSBDBG      (1)
#define OMV_TUSBDBG_PACKET      (64)

// Enable self-tests on first boot
#define OMV_ENABLE_SELFTEST     (0)

// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH     (320*240)

// Low and high JPEG QS.
#define JPEG_QUALITY_LOW        50
#define JPEG_QUALITY_HIGH       90

// FB Heap Block Size
#define OMV_UMM_BLOCK_SIZE      16

// Core VBAT for selftests
#define OMV_CORE_VBAT           "3.3"

// USB IRQn.
#define OMV_USB_IRQN            (USBD_IRQn)
#define OMV_USB1_IRQ_HANDLER    (USBD_IRQHandler)

// Linker script constants (see the linker script template port/x.ld.S).
#define OMV_FB_MEMORY           SRAM    // Framebuffer, fb_alloc
#define OMV_MAIN_MEMORY         SRAM    // data, bss and heap memory
#define OMV_STACK_MEMORY        SRAM    // stack memory

#define OMV_FB_SIZE             (128K)  // FB memory: header + QVGA/GS image
#define OMV_FB_ALLOC_SIZE       (16K)   // minimum fb alloc size
#define OMV_STACK_SIZE          (8K)
#define OMV_HEAP_SIZE           (64K)
#define OMV_JPEG_BUF_SIZE       (16 * 1024) // IDE JPEG buffer (header + data).

#define OMV_TEXT_ORIGIN         0x00026000
#define OMV_FFS_LENGTH          64K
#define OMV_TEXT_LENGTH         680K        // 0x00000 -> 0x26000  Soft device (152K)
                                            // 0x26000 -> 0xD0000  OpenMV firmware (680K).
                                            // 0xD0000 -> 0xE0000  Flash filesystem (64K).
                                            // 0xE0000 -> 0x100000 Arduino bootloader
#define OMV_SRAM_ORIGIN         0x20004000  // Reserve 16K for SD memory.
#define OMV_SRAM_LENGTH         240K        // RAM_SIZE - SD_RAM_SIZE

// FIR I2C
#define FIR_I2C_ID              (0)
#define FIR_I2C_SCL_PIN         (2)
#define FIR_I2C_SDA_PIN         (31)
#define FIR_I2C_SPEED           (CAMBUS_SPEED_FULL)

// ISC I2C
#define ISC_I2C_ID              (0)
#define ISC_I2C_SCL_PIN         (2)
#define ISC_I2C_SDA_PIN         (31)
#define ISC_I2C_SPEED           (CAMBUS_SPEED_STANDARD)

// I2C0
#define TWI0_ID                 (0)
#define TWI0_SCL_PIN            (2)
#define TWI0_SDA_PIN            (31)
#define TWI0_SPEED              (CAMBUS_SPEED_FULL)

// I2C1
#define TWI1_ID                 (1)
#define TWI1_SCL_PIN            (15)
#define TWI1_SDA_PIN            (14)
#define TWI1_SPEED              (CAMBUS_SPEED_FULL)

// PDM/MIC
#define PDM_DIN_PIN             (25)
#define PDM_CLK_PIN             (26)
#define PDM_PWR_PIN             (17)

// DCMI
#define DCMI_PWDN_PIN           (29)
#define DCMI_RESET_PIN          (30)

#define DCMI_D0_PIN             (32+2)
#define DCMI_D1_PIN             (32+3)
#define DCMI_D2_PIN             (32+10)
#define DCMI_D3_PIN             (32+11)
#define DCMI_D4_PIN             (32+12)
#define DCMI_D5_PIN             (32+13)
#define DCMI_D6_PIN             (32+14)
#define DCMI_D7_PIN             (32+15)

#define DCMI_VSYNC_PIN          (21)
#define DCMI_HSYNC_PIN          (5)
#define DCMI_PXCLK_PIN          (4)
#define DCMI_XCLK_PIN           (27)

#if defined(DCMI_RESET_PIN)
#define DCMI_RESET_LOW()        nrf_gpio_pin_clear(DCMI_RESET_PIN)
#define DCMI_RESET_HIGH()       nrf_gpio_pin_set(DCMI_RESET_PIN)
#else
#define DCMI_RESET_LOW()
#define DCMI_RESET_HIGH()
#endif

#if defined(DCMI_PWDN_PIN)
#define DCMI_PWDN_LOW()        nrf_gpio_pin_clear(DCMI_PWDN_PIN)
#define DCMI_PWDN_HIGH()       nrf_gpio_pin_set(DCMI_PWDN_PIN)
#else
#define DCMI_PWDN_LOW()
#define DCMI_PWDN_HIGH()
#endif

// FIR Module
#define OMV_ENABLE_FIR_MLX90621 (1)
#define OMV_ENABLE_FIR_MLX90640 (1)
#define OMV_ENABLE_FIR_MLX90641 (1)
#define OMV_ENABLE_FIR_AMG8833  (1)
#define OMV_ENABLE_FIR_LEPTON   (0)

#endif //__OMV_BOARDCONFIG_H__

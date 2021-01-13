/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
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
#define OMV_UNIQUE_ID_ADDR      0x10000060
#define OMV_UNIQUE_ID_SIZE      2 // 2 words

// Needed by the SWD JTAG testrig - located at the bottom of the frame buffer stack.
#define OMV_SELF_TEST_SWD_ADDR  MAIN_FB()->bpp

#define OMV_XCLK_MCO            (0U)
#define OMV_XCLK_TIM            (1U)

// Sensor external clock source.
#define OMV_XCLK_SOURCE         (OMV_XCLK_TIM)

// Sensor external clock timer frequency.
#define OMV_XCLK_FREQUENCY      (12000000)

// Sensor PLL register value.
#define OMV_OV7725_PLL_CONFIG   (0x41)  // x4

// Sensor Banding Filter Value
#define OMV_OV7725_BANDING      (0x7F)

// RAW buffer size
#define OMV_RAW_BUF_SIZE        (131072)

// Enable hardware JPEG
#define OMV_HARDWARE_JPEG       (0)

// Enable sensor drivers
#define OMV_ENABLE_OV2640       (0)
#define OMV_ENABLE_OV5640       (0)
#define OMV_ENABLE_OV7690       (0)
#define OMV_ENABLE_OV7725       (0)
#define OMV_ENABLE_OV9650       (0)
#define OMV_ENABLE_MT9V034      (0)
#define OMV_ENABLE_LEPTON       (0)
#define OMV_ENABLE_HM01B0       (0)

// Enable sensor features
#define OMV_ENABLE_OV5640_AF    (0)

// Enable WiFi debug
#define OMV_ENABLE_WIFIDBG      (1)

// Enable self-tests on first boot
#define OMV_ENABLE_SELFTEST     (0)

// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH     (320*240*2)

// Low and high JPEG QS.
#define JPEG_QUALITY_LOW        50
#define JPEG_QUALITY_HIGH       90

// Low and high JPEG QS.
#define JPEG_QUALITY_LOW        50
#define JPEG_QUALITY_HIGH       90

// FB Heap Block Size
#define OMV_UMM_BLOCK_SIZE      16

// Core VBAT for selftests
#define OMV_CORE_VBAT           "3.3"

// USB IRQn.
#define OMV_USB_IRQN        (USBD_IRQn)

// Linker script constants (see the linker script template port/x.ld.S).
#define OMV_FB_MEMORY       SRAM    // Framebuffer, fb_alloc
#define OMV_MAIN_MEMORY     SRAM    // data, bss and heap memory
#define OMV_STACK_MEMORY    SRAM    // stack memory

#define OMV_FB_SIZE         (128K)  // FB memory: header + QVGA/GS image
#define OMV_FB_ALLOC_SIZE   (16K)   // minimum fb alloc size
#define OMV_STACK_SIZE      (8K)
#define OMV_HEAP_SIZE       (64K)
#define OMV_JPEG_BUF_SIZE   (16 * 1024) // IDE JPEG buffer (header + data).

#define OMV_TEXT_ORIGIN     0x00026000
#define OMV_TEXT_LENGTH     808K        // FLASH_SIZE - SD_SIZE - FS_SIZE 1024-152-64
#define OMV_SRAM_ORIGIN     0x20004000  // Reserve 16K for SD memory.
#define OMV_SRAM_LENGTH     240K        // RAM_SIZE - SD_RAM_SIZE

// FIR I2C
#define FIR_I2C_ID              (0)
#define FIR_I2C_SCL_PIN         (2)
#define FIR_I2C_SDA_PIN         (31)
#define FIR_I2C_SPEED           (CAMBUS_SPEED_FULL)

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

// FIR Module
#define OMV_ENABLE_FIR_MLX90621 (1)
#define OMV_ENABLE_FIR_MLX90640 (1)
#define OMV_ENABLE_FIR_MLX90641 (1)
#define OMV_ENABLE_FIR_AMG8833  (1)
#define OMV_ENABLE_FIR_LEPTON   (0)

#endif //__OMV_BOARDCONFIG_H__

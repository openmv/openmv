/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Board configuration and pin definitions.
 */
#ifndef __OMV_BOARDCONFIG_H__
#define __OMV_BOARDCONFIG_H__

// Architecture info
#define OMV_BOARD_ARCH             "NANO33 M4"  // 33 chars max
#define OMV_BOARD_TYPE             "NANO33"
#define OMV_BOARD_UID_ADDR         0x10000060   // Unique ID address.
#define OMV_BOARD_UID_SIZE         2            // Unique ID size in words.
#define OMV_BOARD_UID_OFFSET       4            // Bytes offset for multi-word UIDs.

// JPEG configuration.
#define OMV_JPEG_CODEC_ENABLE      (0)
#define OMV_JPEG_QUALITY_LOW       50
#define OMV_JPEG_QUALITY_HIGH      90
#define OMV_JPEG_QUALITY_THRESHOLD (320 * 240)

// Image sensor drivers configuration.
#define OMV_OV7670_ENABLE          (1)
#define OMV_OV7670_VERSION         (75)
#define OMV_OV7670_CLKRC           (2)

// FIR sensor drivers configuration.
#define OMV_FIR_MLX90621_ENABLE    (1)
#define OMV_FIR_MLX90640_ENABLE    (1)
#define OMV_FIR_MLX90641_ENABLE    (1)
#define OMV_FIR_AMG8833_ENABLE     (1)

// Debugging configuration.
#define OMV_TUSBDBG_ENABLE         (1)
#define OMV_TUSBDBG_PACKET         (64)

// UMM heap block size
#define OMV_UMM_BLOCK_SIZE         16

// USB IRQn.
#define OMV_USB_IRQN               (USBD_IRQn)
#define OMV_USB1_IRQ_HANDLER       (USBD_IRQHandler)

// Linker script constants (see the linker script template port/x.ld.S).
#define OMV_MAIN_MEMORY            SRAM // data, bss and heap memory
#define OMV_HEAP_SIZE              (64K)
#define OMV_STACK_MEMORY           SRAM // stack memory
#define OMV_STACK_SIZE             (8K)
#define OMV_FB_MEMORY              SRAM // Framebuffer, fb_alloc
#define OMV_FB_SIZE                (128K) // FB memory: header + QVGA/GS image
#define OMV_FB_ALLOC_SIZE          (16K) // minimum fb alloc size
#define OMV_JPEG_BUF_SIZE          (16 * 1024) // IDE JPEG buffer (header + data).

#define OMV_TEXT_ORIGIN            0x00026000
#define OMV_FFS_LENGTH             64K
#define OMV_TEXT_LENGTH            680K     // 0x00000 -> 0x26000  Soft device (152K)
                                            // 0x26000 -> 0xD0000  OpenMV firmware (680K).
                                            // 0xD0000 -> 0xE0000  Flash filesystem (64K).
                                            // 0xE0000 -> 0x100000 Arduino bootloader
#define OMV_SRAM_ORIGIN            0x20004000 // Reserve 16K for SD memory.
#define OMV_SRAM_LENGTH            240K     // RAM_SIZE - SD_RAM_SIZE

// FIR I2C
#define OMV_FIR_I2C_ID             (0)
#define OMV_FIR_I2C_SPEED          (OMV_I2C_SPEED_FULL)

// ISC I2C
#define OMV_CSI_I2C_ID             (0)
#define OMV_CSI_I2C_SPEED          (OMV_I2C_SPEED_STANDARD)

// I2C0
#define OMV_I2C0_ID                (0)
#define OMV_I2C0_SCL_PIN           (2)
#define OMV_I2C0_SDA_PIN           (31)

// I2C1
#define OMV_I2C1_ID                (1)
#define OMV_I2C1_SCL_PIN           (15)
#define OMV_I2C1_SDA_PIN           (14)

// PDM/MIC
#define OMV_PDM_DIN_PIN            (25)
#define OMV_PDM_CLK_PIN            (26)
#define OMV_PDM_PWR_PIN            (17)

// Camera interface.
#define OMV_CSI_XCLK_SOURCE        (XCLK_SOURCE_TIM)
#define OMV_CSI_XCLK_FREQUENCY     (12000000)

#define OMV_CSI_D0_PIN             (32 + 2)
#define OMV_CSI_D1_PIN             (32 + 3)
#define OMV_CSI_D2_PIN             (32 + 10)
#define OMV_CSI_D3_PIN             (32 + 11)
#define OMV_CSI_D4_PIN             (32 + 12)
#define OMV_CSI_D5_PIN             (32 + 13)
#define OMV_CSI_D6_PIN             (32 + 14)
#define OMV_CSI_D7_PIN             (32 + 15)

#define OMV_CSI_VSYNC_PIN          (21)
#define OMV_CSI_HSYNC_PIN          (5)
#define OMV_CSI_PXCLK_PIN          (4)
#define OMV_CSI_MXCLK_PIN          (27)
#define OMV_CSI_POWER_PIN          (29)
#define OMV_CSI_RESET_PIN          (30)

#endif //__OMV_BOARDCONFIG_H__

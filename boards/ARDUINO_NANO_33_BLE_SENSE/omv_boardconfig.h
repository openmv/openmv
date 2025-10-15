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

// CSI drivers configuration.
#define OMV_OV7670_ENABLE          (1)
#define OMV_OV7670_VERSION         (75)
#define OMV_OV7670_CLKRC           (2)

// FIR drivers configuration.
#define OMV_FIR_MLX90621_ENABLE    (1)
#define OMV_FIR_MLX90640_ENABLE    (1)
#define OMV_FIR_MLX90641_ENABLE    (1)
#define OMV_FIR_AMG8833_ENABLE     (1)

// UMM heap block size
#define OMV_UMM_BLOCK_SIZE         16

// USB IRQn.
#define OMV_USB_IRQN               (USBD_IRQn)
#define OMV_USB1_IRQ_HANDLER       (USBD_IRQHandler)

// Linker script constants (see the linker script template port/x.ld.S).
#define OMV_MAIN_MEMORY            SRAM // Data, BSS memory
#define OMV_STACK_MEMORY           SRAM // stack memory
#define OMV_STACK_SIZE             (8K)
#define OMV_FB_MEMORY              SRAM // Framebuffer, fb_alloc
#define OMV_FB_SIZE                (128K) // FB memory: header + QVGA/GS image
#define OMV_FB_ALLOC_SIZE          (16K)  // minimum fb alloc size
#define OMV_GC_BLOCK0_MEMORY       SRAM   // Main GC block.
#define OMV_GC_BLOCK0_SIZE         (64K)
#define OMV_SB_SIZE                (16K) // Streaming buffer size.

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
#define OMV_CSI_CLK_SOURCE         (OMV_CSI_CLK_SOURCE_TIM)
#define OMV_CSI_CLK_FREQUENCY      (12000000)

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

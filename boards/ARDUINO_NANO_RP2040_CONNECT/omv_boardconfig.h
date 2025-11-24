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
#define OMV_BOARD_ARCH             "PICO M0" // 33 chars max
#define OMV_BOARD_TYPE             "PICO"
#ifndef LINKER_SCRIPT
extern unsigned char *OMV_BOARD_UID_ADDR;    // Unique address.
#endif
#define OMV_BOARD_UID_SIZE         2         // Unique ID size in words.
#define OMV_BOARD_UID_OFFSET       4         // Bytes offset for multi-word UIDs.

// JPEG configuration.
#define OMV_JPEG_CODEC_ENABLE      (0)
#define OMV_JPEG_QUALITY_LOW       (35)
#define OMV_JPEG_QUALITY_HIGH      (60)
#define OMV_JPEG_QUALITY_THRESHOLD (160 * 120)

// CSI drivers configuration.
#define OMV_OV7670_ENABLE          (1)
#define OMV_OV7670_VERSION         (70)
#define OMV_OV7670_CLKRC           (0x00)

// FIR drivers configuration.
#define OMV_FIR_AMG8833_ENABLE     (1)

// Debugging configuration.
#define OMV_TUSBDBG_ENABLE         (1)
#define OMV_TUSBDBG_BUFFER         (2048)

// UMM heap block size
#define OMV_UMM_BLOCK_SIZE         16

// USB IRQn.
#define OMV_USB_IRQN               (USBCTRL_IRQ_IRQn)

// Linker script constants (see the linker script template port/x.ld.S).
#define OMV_MAIN_MEMORY            RAM // Data/BSS memory
#define OMV_STACK_MEMORY           RAM // stack memory
#define OMV_STACK_SIZE             (16K)
#define OMV_FB_MEMORY              RAM // Framebuffer, fb_alloc
#define OMV_FB_SIZE                (100K) // FB memory
#define OMV_FB_ALLOC_SIZE          (16K) // minimum fb alloc size
#define OMV_GC_BLOCK0_MEMORY       RAM // Main GC block
#define OMV_GC_BLOCK0_SIZE         (60 * 1024) // MicroPython's heap
#define OMV_SB_SIZE                (20K) // Streaming buffer size.

// GP LED
#define OMV_LED_PIN                (6)

// FIR I2C
#define OMV_FIR_I2C_ID             (0)
#define OMV_FIR_I2C_SPEED          (OMV_I2C_SPEED_FULL)

// ISC I2C
#define OMV_CSI_I2C_ID             (0)
#define OMV_CSI_I2C_SPEED          (OMV_I2C_SPEED_STANDARD)

// I2C0
#define OMV_I2C0_ID                (0)
#define OMV_I2C0_SCL_PIN           (13)
#define OMV_I2C0_SDA_PIN           (12)

// I2C1
#define OMV_I2C1_ID                (1)
#define OMV_I2C1_SCL_PIN           (27)
#define OMV_I2C1_SDA_PIN           (26)

// LCD config.
#define OMV_LCD_SPI                (spi0)
#define OMV_LCD_CS_PIN             (5)
#define OMV_LCD_MOSI_PIN           (7)
#define OMV_LCD_SCLK_PIN           (6)
#define OMV_LCD_RST_PIN            (4)
#define OMV_LCD_RS_PIN             (0)

// AUDIO config.
#define OMV_PDM_PIO                (pio1)
#define OMV_PDM_SM                 (0)
#define OMV_PDM_DMA                (1)
#define OMV_PDM_DMA_IRQ            (DMA_IRQ_1)
#define OMV_PDM_DMA_CHANNEL        (0)
#define OMV_PDM_CLK_PIN            (23)
#define OMV_PDM_DIN_PIN            (22)

// Camera interface
#define OMV_CSI_PIO                (pio0)
#define OMV_CSI_SM                 (0)
#define OMV_CSI_DMA                (0)
#define OMV_CSI_DMA_IRQ            (DMA_IRQ_0)
#define OMV_CSI_DMA_CHANNEL        (0)
#define OMV_CSI_CLK_SOURCE         (OMV_CSI_CLK_SOURCE_TIM)
#define OMV_CSI_CLK_FREQUENCY      (12500000)

#define OMV_CSI_D0_PIN             (15)
#define OMV_CSI_D1_PIN             (16)
#define OMV_CSI_D2_PIN             (17)
#define OMV_CSI_D3_PIN             (18)
#define OMV_CSI_D4_PIN             (19)
#define OMV_CSI_D5_PIN             (20)
#define OMV_CSI_D6_PIN             (21)
#define OMV_CSI_D7_PIN             (25) // MSB is read separately.

#define OMV_CSI_HSYNC_PIN          (27)
#define OMV_CSI_VSYNC_PIN          (26)
#define OMV_CSI_PXCLK_PIN          (29)
#define OMV_CSI_MXCLK_PIN          (28)
#define OMV_CSI_POWER_PIN          (0)
#define OMV_CSI_RESET_PIN          (1)

#endif //__OMV_BOARDCONFIG_H__

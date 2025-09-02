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
#define OMV_BOARD_ARCH             "PICO M0" // 33 chars max
#define OMV_BOARD_TYPE             "PICO"
#ifndef LINKER_SCRIPT
extern unsigned char *OMV_BOARD_UID_ADDR;   // Unique ID address.
#endif
#define OMV_BOARD_UID_SIZE         2        // Unique ID size in words.
#define OMV_BOARD_UID_OFFSET       4        // Bytes offset for multi-word UIDs.

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
#define OMV_FIR_MLX90621_ENABLE    (0)
#define OMV_FIR_MLX90640_ENABLE    (0)
#define OMV_FIR_MLX90641_ENABLE    (0)
#define OMV_FIR_AMG8833_ENABLE     (1)

// UMM heap block size
#define OMV_UMM_BLOCK_SIZE         16

// USB IRQn.
#define OMV_USB_IRQN               (USBCTRL_IRQ_IRQn)
#define OMV_USB1_IRQ_HANDLER       (USBD_IRQHandler)

// Linker script constants (see the linker script template port/x.ld.S).
#define OMV_MAIN_MEMORY            RAM    // Data/BSS memory
#define OMV_GC_BLOCK0_MEMORY       RAM    // Main GC block.
#define OMV_GC_BLOCK0_SIZE         (32 * 1024)
#define OMV_STACK_MEMORY           RAM    // stack memory
#define OMV_STACK_SIZE             (8K)
#define OMV_FB_MEMORY              RAM    // Framebuffer, fb_alloc
#define OMV_FB_SIZE                (151K) // FB memory: header + QVGA/GS image
#define OMV_FB_ALLOC_SIZE          (12K)  // minimum fb alloc size
#define OMV_SB_SIZE                (20K)  // IDE JPEG buffer (header + data).

// GP LED
#define OMV_LED_PIN                (25)

// FIR I2C
#define OMV_FIR_I2C_ID             (0)
#define OMV_FIR_I2C_SPEED          (OMV_I2C_SPEED_FULL)

// ISC I2C
#define OMV_CSI_I2C_ID             (1)
#define OMV_CSI_I2C_SPEED          (OMV_I2C_SPEED_STANDARD)

// I2C0
#define OMV_I2C0_ID                (0)
#define OMV_I2C0_SCL_PIN           (21)
#define OMV_I2C0_SDA_PIN           (20)

// I2C1
#define OMV_I2C1_ID                (1)
#define OMV_I2C1_SCL_PIN           (15)
#define OMV_I2C1_SDA_PIN           (14)

#define OMV_LCD_SPI                (spi0)
#define OMV_LCD_CS_PIN             (17)
#define OMV_LCD_MOSI_PIN           (19)
#define OMV_LCD_SCLK_PIN           (18)
#define OMV_LCD_RST_PIN            (20)
#define OMV_LCD_RS_PIN             (21)

// Camera interface.
#define OMV_CSI_PIO                (pio0)
#define OMV_CSI_SM                 (0)
#define OMV_CSI_DMA                (0)
#define OMV_CSI_DMA_IRQ            (DMA_IRQ_0)
#define OMV_CSI_DMA_CHANNEL        (0)
#define OMV_CSI_CLK_SOURCE         (OMV_CSI_CLK_SOURCE_TIM)
#define OMV_CSI_CLK_FREQUENCY      (12500000)

#define OMV_CSI_D0_PIN             (2)
#define OMV_CSI_D1_PIN             (3)
#define OMV_CSI_D2_PIN             (4)
#define OMV_CSI_D3_PIN             (5)
#define OMV_CSI_D4_PIN             (6)
#define OMV_CSI_D5_PIN             (7)
#define OMV_CSI_D6_PIN             (8)
#define OMV_CSI_D7_PIN             (9)

// Must match the pins defined in dcmi.pio.
#define OMV_CSI_HSYNC_PIN          (12)
#define OMV_CSI_VSYNC_PIN          (13)
#define OMV_CSI_PXCLK_PIN          (11)
#define OMV_CSI_MXCLK_PIN          (10)
#define OMV_CSI_POWER_PIN          (0)
#define OMV_CSI_RESET_PIN          (1)

#endif //__OMV_BOARDCONFIG_H__

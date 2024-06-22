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
#define OMV_BOARD_ARCH                        "OMV2 F4 256 JPEG" // 33 chars max
#define OMV_BOARD_TYPE                        "M4"
#define OMV_BOARD_UID_ADDR                    0x1FFF7A10    // Unique ID address.
#define OMV_BOARD_UID_SIZE                    3             // Unique ID size in words.
#define OMV_BOARD_UID_OFFSET                  4             // Bytes offset for multi-word UIDs.

// JPEG configuration.
#define OMV_JPEG_CODEC_ENABLE                 (0)
#define OMV_JPEG_QUALITY_LOW                  (35)
#define OMV_JPEG_QUALITY_HIGH                 (60)
#define OMV_JPEG_QUALITY_THRESHOLD            (160 * 120 * 2)

// Image sensor drivers configuration.
#define OMV_OV2640_ENABLE                     (1)
#define OMV_OV7725_ENABLE                     (1)
#define OMV_OV7725_PLL_CONFIG                 (0x41) // x4
#define OMV_OV7725_BANDING                    (0x3F)

// FIR sensor drivers configuration.
#define OMV_FIR_MLX90621_ENABLE               (1)
#define OMV_FIR_MLX90640_ENABLE               (1)
#define OMV_FIR_MLX90641_ENABLE               (1)
#define OMV_FIR_AMG8833_ENABLE                (1)
#define OMV_FIR_LEPTON_ENABLE                 (1)

// UMM heap block size
#define OMV_UMM_BLOCK_SIZE                    16

// USB IRQn.
#define OMV_USB_IRQN                          (OTG_FS_IRQn)

//PLL1 192MHz/48MHz
#define OMV_OSC_PLL1M                         (12)
#define OMV_OSC_PLL1N                         (384)
#define OMV_OSC_PLL1P                         (2)
#define OMV_OSC_PLL1Q                         (8)

// HSE/HSI/CSI State
#define OMV_OSC_HSE_STATE                     (RCC_HSE_ON)

// Clock Sources
#define OMV_OSC_PLL_CLKSOURCE                 RCC_PLLSOURCE_HSE

// Flash Latency
#define OMV_FLASH_LATENCY                     (FLASH_LATENCY_7)

// Linker script constants (see the linker script template stm32fxxx.ld.S).
// Note: fb_alloc is a stack-based, dynamically allocated memory on FB.
// The maximum available fb_alloc memory = FB_ALLOC_SIZE + FB_SIZE - (w*h*bpp).
// Note: fb_alloc overwrites the line buffer which is only used during readout.
#define OMV_MAIN_MEMORY                       DTCM // data, bss and heap memory
#define OMV_DMA_MEMORY                        SRAM2 // Misc DMA buffers
#define OMV_HEAP_SIZE                         (47K)
#define OMV_STACK_MEMORY                      DTCM // stack memory
#define OMV_STACK_SIZE                        (8K)
#define OMV_FB_MEMORY                         SRAM1 // Framebuffer, fb_alloc
#define OMV_FB_SIZE                           (152K) // FB memory: header + QVGA/GS image
#define OMV_FB_ALLOC_SIZE                     (10K) // minimum fb alloc size
#define OMV_JPEG_BUF_SIZE                     (8 * 1024) // IDE JPEG buffer size (header + data).
#define OMV_MSC_BUF_SIZE                      (2K) // USB MSC bot data
#define OMV_VFS_BUF_SIZE                      (1K) // VFS struct + FATFS file buffer (624 bytes)
#define OMV_FFS_BUF_SIZE                      (16K) // Flash filesystem cache
#define OMV_LINE_BUF_SIZE                     (2 * 1024) // Image line buffer round(320 * 2BPP * 2 buffers).

// Memory map.
#define OMV_FLASH_ORIGIN                      0x08000000
#define OMV_FLASH_LENGTH                      1024K
#define OMV_DTCM_ORIGIN                       0x10000000
#define OMV_DTCM_LENGTH                       64K
#define OMV_SRAM1_ORIGIN                      0x20000000
#define OMV_SRAM1_LENGTH                      162K
#define OMV_SRAM2_ORIGIN                      0x20028800
#define OMV_SRAM2_LENGTH                      30K

// Flash configuration.
#define OMV_FLASH_FFS_ORIGIN                  0x08004000
#define OMV_FLASH_FFS_LENGTH                  48K
#define OMV_FLASH_TXT_ORIGIN                  0x08010000
#define OMV_FLASH_TXT_LENGTH                  960K

// Main image sensor I2C bus
#define OMV_CSI_I2C_ID                        (1)
#define OMV_CSI_I2C_SPEED                     (OMV_I2C_SPEED_STANDARD)

// Thermal image sensor I2C bus
#define OMV_FIR_I2C_ID                        (2)
#define OMV_FIR_I2C_SPEED                     (OMV_I2C_SPEED_FULL)

// Soft I2C bus
#define OMV_SOFT_I2C_SIOC_PIN                 (&omv_pin_B10_GPIO)
#define OMV_SOFT_I2C_SIOD_PIN                 (&omv_pin_B11_GPIO)
#define OMV_SOFT_I2C_SPIN_DELAY               16

// WINC SPI bus
#define OMV_WINC_SPI_ID                       (2)
#define OMV_WINC_SPI_BAUDRATE                 (27000000)
#define OMV_WINC_EN_PIN                       (&omv_pin_A5_GPIO)
#define OMV_WINC_RST_PIN                      (&omv_pin_D12_GPIO)
#define OMV_WINC_IRQ_PIN                      (&omv_pin_D13_GPIO)

// Camera Interface
#define OMV_CSI_XCLK_SOURCE                   (XCLK_SOURCE_TIM)
#define OMV_CSI_XCLK_FREQUENCY                (6000000)
#define OMV_CSI_TIM                           (TIM1)
#define OMV_CSI_TIM_PIN                       (&omv_pin_A8_TIM1)
#define OMV_CSI_TIM_CHANNEL                   (TIM_CHANNEL_1)
#define OMV_CSI_TIM_CLK_ENABLE()              __TIM1_CLK_ENABLE()
#define OMV_CSI_TIM_CLK_DISABLE()             __TIM1_CLK_DISABLE()
#define OMV_CSI_TIM_PCLK_FREQ()               HAL_RCC_GetPCLK2Freq()

#define OMV_CSI_D0_PIN                        (&omv_pin_C6_DCMI)
#define OMV_CSI_D1_PIN                        (&omv_pin_C7_DCMI)
#define OMV_CSI_D2_PIN                        (&omv_pin_E0_DCMI)
#define OMV_CSI_D3_PIN                        (&omv_pin_E1_DCMI)
#define OMV_CSI_D4_PIN                        (&omv_pin_E4_DCMI)
#define OMV_CSI_D5_PIN                        (&omv_pin_B6_DCMI)
#define OMV_CSI_D6_PIN                        (&omv_pin_E5_DCMI)
#define OMV_CSI_D7_PIN                        (&omv_pin_E6_DCMI)

#define OMV_CSI_HSYNC_PIN                     (&omv_pin_A4_DCMI)
#define OMV_CSI_VSYNC_PIN                     (&omv_pin_B7_DCMI)
#define OMV_CSI_PXCLK_PIN                     (&omv_pin_A6_DCMI)
#define OMV_CSI_RESET_PIN                     (&omv_pin_A10_GPIO)
#define OMV_CSI_POWER_PIN                     (&omv_pin_B5_GPIO)

// Physical I2C buses.

// I2C bus 1
#define OMV_I2C1_ID                           (1)
#define OMV_I2C1_SCL_PIN                      (&omv_pin_B8_I2C1)
#define OMV_I2C1_SDA_PIN                      (&omv_pin_B9_I2C1)

// I2C bus 2
#define OMV_I2C2_ID                           (2)
#define OMV_I2C2_SCL_PIN                      (&omv_pin_B10_I2C2)
#define OMV_I2C2_SDA_PIN                      (&omv_pin_B11_I2C2)

// Physical SPI buses.

// SPI bus 2
#define OMV_SPI2_ID                           (2)
#define OMV_SPI2_SCLK_PIN                     (&omv_pin_B13_SPI2)
#define OMV_SPI2_MISO_PIN                     (&omv_pin_B14_SPI2)
#define OMV_SPI2_MOSI_PIN                     (&omv_pin_B15_SPI2)
#define OMV_SPI2_SSEL_PIN                     (&omv_pin_B12_SPI2)
#define OMV_SPI2_DMA_TX_CHANNEL               (DMA1_Stream4)
#define OMV_SPI2_DMA_RX_CHANNEL               (DMA1_Stream3)
#define DMA_REQUEST_SPI2_TX                   (DMA_CHANNEL_0)
#define DMA_REQUEST_SPI2_RX                   (DMA_CHANNEL_0)

// SPI LCD Interface
#define OMV_SPI_DISPLAY_CONTROLLER            (OMV_SPI2_ID)
#define OMV_SPI_DISPLAY_MOSI_PIN              (&omv_pin_B15_SPI2)
#define OMV_SPI_DISPLAY_MISO_PIN              (&omv_pin_B14_SPI2)
#define OMV_SPI_DISPLAY_SCLK_PIN              (&omv_pin_B13_SPI2)
#define OMV_SPI_DISPLAY_SSEL_PIN              (&omv_pin_B12_GPIO)

#define OMV_SPI_DISPLAY_RS_PIN                (&omv_pin_D13_GPIO)
#define OMV_SPI_DISPLAY_RST_PIN               (&omv_pin_D12_GPIO)
#define OMV_SPI_DISPLAY_BL_PIN                (&omv_pin_A5_GPIO)

// FIR Lepton
#define OMV_FIR_LEPTON_I2C_BUS                (OMV_FIR_I2C_ID)
#define OMV_FIR_LEPTON_I2C_BUS_SPEED          (OMV_FIR_I2C_SPEED)

#define OMV_FIR_LEPTON_SPI_BUS                (OMV_SPI2_ID)
#define OMV_FIR_LEPTON_MOSI_PIN               (&omv_pin_B15_SPI2)
#define OMV_FIR_LEPTON_MISO_PIN               (&omv_pin_B14_SPI2)
#define OMV_FIR_LEPTON_SCLK_PIN               (&omv_pin_B13_SPI2)
#define OMV_FIR_LEPTON_SSEL_PIN               (&omv_pin_B12_GPIO)

#endif //__OMV_BOARDCONFIG_H__

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
#define OMV_ARCH_STR                          "OMV2 F4 256 JPEG" // 33 chars max
#define OMV_BOARD_TYPE                        "M4"
#define OMV_UNIQUE_ID_ADDR                    0x1FFF7A10 // Unique ID address.
#define OMV_UNIQUE_ID_SIZE                    3 // Unique ID size in words.
#define OMV_UNIQUE_ID_OFFSET                  4 // Bytes offset for multi-word UIDs.

// Needed by the SWD JTAG testrig - located at the bottom of the frame buffer stack.
#define OMV_SELF_TEST_SWD_ADDR                MAIN_FB()->pixfmt

#define OMV_XCLK_MCO                          (0U)
#define OMV_XCLK_TIM                          (1U)

// Sensor external clock source.
#define OMV_XCLK_SOURCE                       (OMV_XCLK_TIM)

// Sensor external clock timer frequency.
#define OMV_XCLK_FREQUENCY                    (6000000)

// Sensor PLL register value.
#define OMV_OV7725_PLL_CONFIG                 (0x41) // x4

// Sensor Banding Filter Value
#define OMV_OV7725_BANDING                    (0x3F)

// Enable sensor drivers
#define OMV_ENABLE_OV2640                     (1)
#define OMV_ENABLE_OV5640                     (0)
#define OMV_ENABLE_OV7690                     (0)
#define OMV_ENABLE_OV7725                     (1)
#define OMV_ENABLE_OV9650                     (0)
#define OMV_ENABLE_MT9M114                    (0)
#define OMV_ENABLE_MT9V0XX                    (0)
#define OMV_ENABLE_LEPTON                     (0)
#define OMV_ENABLE_HM01B0                     (0)
#define OMV_ENABLE_PAJ6100                    (0)
#define OMV_ENABLE_FROGEYE2020                (0)
#define OMV_ENABLE_FIR_MLX90621               (1)
#define OMV_ENABLE_FIR_MLX90640               (1)
#define OMV_ENABLE_FIR_MLX90641               (1)
#define OMV_ENABLE_FIR_AMG8833                (1)
#define OMV_ENABLE_FIR_LEPTON                 (1)

// Enable sensor features
#define OMV_ENABLE_OV5640_AF                  (0)

// Enable self-tests on first boot
#define OMV_ENABLE_SELFTEST                   (1)

// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH                   (160 * 120 * 2)

// Low and high JPEG QS.
#define JPEG_QUALITY_LOW                      35
#define JPEG_QUALITY_HIGH                     60

// FB Heap Block Size
#define OMV_UMM_BLOCK_SIZE                    16

// Core VBAT for selftests
#define OMV_CORE_VBAT                         "3.3"

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
#define OMV_FB_MEMORY                         SRAM1 // Framebuffer, fb_alloc
#define OMV_MAIN_MEMORY                       DTCM // data, bss and heap memory
#define OMV_STACK_MEMORY                      DTCM // stack memory
#define OMV_DMA_MEMORY                        SRAM2 // Misc DMA buffers

#define OMV_FB_SIZE                           (152K) // FB memory: header + QVGA/GS image
#define OMV_FB_ALLOC_SIZE                     (10K) // minimum fb alloc size
#define OMV_STACK_SIZE                        (8K)
#define OMV_HEAP_SIZE                         (47K)

#define OMV_LINE_BUF_SIZE                     (2 * 1024) // Image line buffer round(320 * 2BPP * 2 buffers).
#define OMV_MSC_BUF_SIZE                      (2K) // USB MSC bot data
#define OMV_VFS_BUF_SIZE                      (1K) // VFS struct + FATFS file buffer (624 bytes)
#define OMV_FIR_LEPTON_BUF_SIZE               (1K) // FIR Lepton Packet Double Buffer (328 bytes)
#define OMV_FFS_BUF_SIZE                      (16K) // Flash filesystem cache
#define OMV_JPEG_BUF_SIZE                     (8 * 1024) // IDE JPEG buffer size (header + data).

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
#define ISC_I2C_ID                            (1)
#define ISC_I2C_SPEED                         (OMV_I2C_SPEED_STANDARD)

// Thermal image sensor I2C bus
#define FIR_I2C_ID                            (2)
#define FIR_I2C_SPEED                         (OMV_I2C_SPEED_FULL)

// Soft I2C bus
#define SOFT_I2C_SIOC_PIN                     (&omv_pin_B10_GPIO)
#define SOFT_I2C_SIOD_PIN                     (&omv_pin_B11_GPIO)
#define SOFT_I2C_SPIN_DELAY                   16

// WINC SPI bus
#define WINC_SPI_ID                           (2)
#define WINC_SPI_BAUDRATE                     (27000000)
#define WINC_EN_PIN                           (&omv_pin_A5_GPIO)
#define WINC_RST_PIN                          (&omv_pin_D12_GPIO)
#define WINC_IRQ_PIN                          (&omv_pin_D13_GPIO)

// DCMI pins.
#define DCMI_TIM                              (TIM1)
#define DCMI_TIM_PIN                          (&omv_pin_A8_TIM1)
#define DCMI_TIM_CHANNEL                      (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()                 __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()                __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()                  HAL_RCC_GetPCLK2Freq()

// DCMI pins.
#define DCMI_RESET_PIN                        (&omv_pin_A10_GPIO)
#define DCMI_POWER_PIN                        (&omv_pin_B5_GPIO)

#define DCMI_D0_PIN                           (&omv_pin_C6_DCMI)
#define DCMI_D1_PIN                           (&omv_pin_C7_DCMI)
#define DCMI_D2_PIN                           (&omv_pin_E0_DCMI)
#define DCMI_D3_PIN                           (&omv_pin_E1_DCMI)
#define DCMI_D4_PIN                           (&omv_pin_E4_DCMI)
#define DCMI_D5_PIN                           (&omv_pin_B6_DCMI)
#define DCMI_D6_PIN                           (&omv_pin_E5_DCMI)
#define DCMI_D7_PIN                           (&omv_pin_E6_DCMI)

#define DCMI_HSYNC_PIN                        (&omv_pin_A4_DCMI)
#define DCMI_VSYNC_PIN                        (&omv_pin_B7_DCMI)
#define DCMI_PXCLK_PIN                        (&omv_pin_A6_DCMI)

// Physical I2C buses
// I2C bus 1
#define I2C1_ID                               (1)
#define I2C1_SCL_PIN                          (&omv_pin_B8_I2C1)
#define I2C1_SDA_PIN                          (&omv_pin_B9_I2C1)

// I2C bus 2
#define I2C2_ID                               (2)
#define I2C2_SCL_PIN                          (&omv_pin_B10_I2C2)
#define I2C2_SDA_PIN                          (&omv_pin_B11_I2C2)

// Physical SPI buses
// SPI bus 2
#define SPI2_ID                               (2)
#define SPI2_SCLK_PIN                         (&omv_pin_B13_SPI2)
#define SPI2_MISO_PIN                         (&omv_pin_B14_SPI2)
#define SPI2_MOSI_PIN                         (&omv_pin_B15_SPI2)
#define SPI2_SSEL_PIN                         (&omv_pin_B12_SPI2)
#define SPI2_DMA_TX_CHANNEL                   (DMA1_Stream4)
#define SPI2_DMA_RX_CHANNEL                   (DMA1_Stream3)
#define DMA_REQUEST_SPI2_TX                   (DMA_CHANNEL_0)
#define DMA_REQUEST_SPI2_RX                   (DMA_CHANNEL_0)

// SPI LCD Interface
#define OMV_SPI_LCD_CONTROLLER                (&spi_obj[1])
#define OMV_SPI_LCD_CONTROLLER_INSTANCE       (SPI2)

#define OMV_SPI_LCD_MOSI_PIN                  (&omv_pin_B15_SPI2)
#define OMV_SPI_LCD_MISO_PIN                  (&omv_pin_B14_SPI2)
#define OMV_SPI_LCD_SCLK_PIN                  (&omv_pin_B13_SPI2)
#define OMV_SPI_LCD_SSEL_PIN                  (&omv_pin_B12_GPIO)

#define OMV_SPI_LCD_RS_PIN                    (&omv_pin_D13_GPIO)
#define OMV_SPI_LCD_BL_PIN                    (&omv_pin_A5_GPIO)
#define OMV_SPI_LCD_RST_PIN                   (&omv_pin_D12_GPIO)

#define OMV_SPI_LCD_BL_DAC                    (DAC)
#define OMV_SPI_LCD_BL_DAC_CHANNEL            (DAC_CHANNEL_2)
#define OMV_SPI_LCD_BL_DAC_CLK_ENABLE()       __HAL_RCC_DAC_CLK_ENABLE()
#define OMV_SPI_LCD_BL_DAC_CLK_DISABLE()      __HAL_RCC_DAC_CLK_DISABLE()
#define OMV_SPI_LCD_BL_DAC_FORCE_RESET()      __HAL_RCC_DAC_FORCE_RESET()
#define OMV_SPI_LCD_BL_DAC_RELEASE_RESET()    __HAL_RCC_DAC_RELEASE_RESET()

// FIR Lepton
#define OMV_FIR_LEPTON_I2C_BUS                (FIR_I2C_ID)
#define OMV_FIR_LEPTON_I2C_BUS_SPEED          (FIR_I2C_SPEED)
#define OMV_FIR_LEPTON_CONTROLLER             (&spi_obj[1])
#define OMV_FIR_LEPTON_CONTROLLER_INSTANCE    (SPI2)

#define OMV_FIR_LEPTON_MOSI_PIN               (&omv_pin_B15_SPI2)
#define OMV_FIR_LEPTON_MISO_PIN               (&omv_pin_B14_SPI2)
#define OMV_FIR_LEPTON_SCLK_PIN               (&omv_pin_B13_SPI2)
#define OMV_FIR_LEPTON_SSEL_PIN               (&omv_pin_B12_GPIO)
#endif //__OMV_BOARDCONFIG_H__

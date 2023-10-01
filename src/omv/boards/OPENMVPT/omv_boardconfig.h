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
#define OMV_ARCH_STR                               "OPENMVPT 65536 SDRAM" // 33 chars max
#define OMV_BOARD_TYPE                             "H7"
#define OMV_UNIQUE_ID_ADDR                         0x1FF1E800 // Unique ID address.
#define OMV_UNIQUE_ID_SIZE                         3 // Unique ID size in words.
#define OMV_UNIQUE_ID_OFFSET                       4 // Bytes offset for multi-word UIDs.

#define OMV_XCLK_MCO                               (0U)
#define OMV_XCLK_TIM                               (1U)

// Sensor external clock source.
#define OMV_XCLK_SOURCE                            (OMV_XCLK_TIM)

// Sensor external clock timer frequency.
#define OMV_XCLK_FREQUENCY                         (12000000)

// Sensor PLL register value.
#define OMV_OV7725_PLL_CONFIG                      (0x41) // x4

// Sensor Banding Filter Value
#define OMV_OV7725_BANDING                         (0x7F)

// OV5640 Sensor Settings
#define OMV_OV5640_XCLK_FREQ                       (24000000)
#define OMV_OV5640_PLL_CTRL2                       (0x64)
#define OMV_OV5640_PLL_CTRL3                       (0x13)
#define OMV_OV5640_REV_Y_CHECK                     (1)
#define OMV_OV5640_REV_Y_FREQ                      (25000000)
#define OMV_OV5640_REV_Y_CTRL2                     (0x54)
#define OMV_OV5640_REV_Y_CTRL3                     (0x13)

// Enable hardware JPEG
#define OMV_HARDWARE_JPEG                          (1)

// Enable MDMA sensor offload.
#define OMV_ENABLE_SENSOR_MDMA                     (1)

// Enable sensor drivers
#define OMV_ENABLE_OV2640                          (0)
#define OMV_ENABLE_OV5640                          (1)
#define OMV_ENABLE_OV7690                          (0)
#define OMV_ENABLE_OV7725                          (0)
#define OMV_ENABLE_OV9650                          (0)
#define OMV_ENABLE_MT9M114                         (0)
#define OMV_ENABLE_MT9V0XX                         (0)
#define OMV_ENABLE_LEPTON                          (0)
#define OMV_ENABLE_HM01B0                          (0)
#define OMV_ENABLE_PAJ6100                         (0)
#define OMV_ENABLE_FROGEYE2020                     (0)
#define OMV_ENABLE_FIR_MLX90621                    (1)
#define OMV_ENABLE_FIR_MLX90640                    (1)
#define OMV_ENABLE_FIR_MLX90641                    (1)
#define OMV_ENABLE_FIR_AMG8833                     (1)
#define OMV_ENABLE_FIR_LEPTON                      (1)

// Enable additional GPIO banks.
#define OMV_ENABLE_GPIO_BANK_F                     (1)
#define OMV_ENABLE_GPIO_BANK_G                     (1)
#define OMV_ENABLE_GPIO_BANK_H                     (1)
#define OMV_ENABLE_GPIO_BANK_I                     (1)
#define OMV_ENABLE_GPIO_BANK_J                     (1)
#define OMV_ENABLE_GPIO_BANK_K                     (1)

// Enable sensor features
#define OMV_ENABLE_OV5640_AF                       (1)

// Enable WiFi debug
#define OMV_ENABLE_WIFIDBG                         (1)

// Enable self-tests on first boot
#define OMV_ENABLE_SELFTEST                        (0)

// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH                        (1920 * 1080 * 2)

// Low and high JPEG QS.
#define JPEG_QUALITY_LOW                           50
#define JPEG_QUALITY_HIGH                          90

// FB Heap Block Size
#define OMV_UMM_BLOCK_SIZE                         256

// Core VBAT for selftests
#define OMV_CORE_VBAT                              "3.3"

// USB IRQn.
#define OMV_USB_IRQN                               (OTG_FS_IRQn)

//PLL1 48MHz for USB, SDMMC and FDCAN
#define OMV_OSC_PLL1M                              (3)
#define OMV_OSC_PLL1N                              (240)
#define OMV_OSC_PLL1P                              (2)
#define OMV_OSC_PLL1Q                              (20)
#define OMV_OSC_PLL1R                              (2)
#define OMV_OSC_PLL1VCI                            (RCC_PLL1VCIRANGE_2)
#define OMV_OSC_PLL1VCO                            (RCC_PLL1VCOWIDE)
#define OMV_OSC_PLL1FRAC                           (0)

// PLL2 200MHz for FMC and QSPI.
#define OMV_OSC_PLL2M                              (3)
#define OMV_OSC_PLL2N                              (100)
#define OMV_OSC_PLL2P                              (2)
#define OMV_OSC_PLL2Q                              (2)
#define OMV_OSC_PLL2R                              (2)
#define OMV_OSC_PLL2VCI                            (RCC_PLL2VCIRANGE_2)
#define OMV_OSC_PLL2VCO                            (RCC_PLL2VCOWIDE)
#define OMV_OSC_PLL2FRAC                           (0)

// PLL3 160MHz for ADC and SPI123
#define OMV_OSC_PLL3M                              (3)
#define OMV_OSC_PLL3N                              (80)
#define OMV_OSC_PLL3P                              (2)
#define OMV_OSC_PLL3Q                              (2)
#define OMV_OSC_PLL3R                              (2)
#define OMV_OSC_PLL3VCI                            (RCC_PLL3VCIRANGE_2)
#define OMV_OSC_PLL3VCO                            (RCC_PLL3VCOWIDE)
#define OMV_OSC_PLL3FRAC                           (0)

// Clock Sources
#define OMV_OSC_PLL_CLKSOURCE                      RCC_PLLSOURCE_HSE
#define OMV_OSC_USB_CLKSOURCE                      RCC_USBCLKSOURCE_PLL
#define OMV_OSC_RNG_CLKSOURCE                      RCC_RNGCLKSOURCE_HSI48
#define OMV_OSC_ADC_CLKSOURCE                      RCC_ADCCLKSOURCE_PLL2
//#define OMV_OSC_RTC_CLKSOURCE           RCC_RTCCLKSOURCE_LSE
#define OMV_OSC_SPI123_CLKSOURCE                   RCC_SPI123CLKSOURCE_PLL2

// HSE/HSI/CSI State
// The LSE/LSI and RTC are managed by micropython's rtc.c.
// #define OMV_OSC_LSE_STATE               (RCC_LSE_ON)
// #define OMV_OSC_LSE_DRIVE               (RCC_LSEDRIVE_HIGH)
#define OMV_OSC_HSE_STATE                          (RCC_HSE_ON)
#define OMV_OSC_HSI48_STATE                        (RCC_HSI48_ON)
// Errata
#define OMV_OMVPT_ERRATA_RTC                       (1)

// Flash Latency
#define OMV_FLASH_LATENCY                          (FLASH_LATENCY_2)

// Power supply configuration
#define OMV_PWR_SUPPLY                             (PWR_LDO_SUPPLY)

// Linker script constants (see the linker script template stm32fxxx.ld.S).
// Note: fb_alloc is a stack-based, dynamically allocated memory on FB.
// The maximum available fb_alloc memory = FB_ALLOC_SIZE + FB_SIZE - (w*h*bpp).
#define OMV_MAIN_MEMORY                            SRAM1 // data, bss and heap
#define OMV_STACK_MEMORY                           ITCM // stack memory
#define OMV_DMA_MEMORY                             SRAM3 // DMA buffers memory.
#define OMV_FB_MEMORY                              DRAM // Framebuffer, fb_alloc
#define OMV_JPEG_MEMORY                            DRAM // JPEG buffer memory buffer.
#define OMV_JPEG_MEMORY_OFFSET                     (63M) // JPEG buffer is placed after FB/fballoc memory.
#define OMV_VOSPI_MEMORY                           SRAM4 // VoSPI buffer memory.
#define OMV_FB_OVERLAY_MEMORY                      AXI_SRAM // Fast fb_alloc memory.

#define OMV_FB_SIZE                                (32M) // FB memory: header + VGA/GS image
#define OMV_FB_ALLOC_SIZE                          (31M) // minimum fb alloc size
#define OMV_FB_OVERLAY_SIZE                        (496 * 1024) // Fast fb_alloc memory size.
#define OMV_STACK_SIZE                             (64K)
#define OMV_HEAP_SIZE                              (250K)
#define OMV_SDRAM_SIZE                             (64 * 1024 * 1024) // This needs to be here for UVC firmware.

#define OMV_LINE_BUF_SIZE                          (11 * 1024) // Image line buffer round(2592 * 2BPP * 2 buffers).
#define OMV_MSC_BUF_SIZE                           (2K) // USB MSC bot data
#define OMV_VFS_BUF_SIZE                           (1K) // VFS struct + FATFS file buffer (624 bytes)
#define OMV_JPEG_BUF_SIZE                          (1024 * 1024) // IDE JPEG buffer (header + data).

// Memory map.
#define OMV_FLASH_ORIGIN                           0x08000000
#define OMV_FLASH_LENGTH                           2048K
#define OMV_DTCM_ORIGIN                            0x20000000 // Note accessible by CPU and MDMA only.
#define OMV_DTCM_LENGTH                            128K
#define OMV_ITCM_ORIGIN                            0x00000000
#define OMV_ITCM_LENGTH                            64K
#define OMV_SRAM1_ORIGIN                           0x30000000
#define OMV_SRAM1_LENGTH                           272K // SRAM1 + SRAM2 + 1/2 SRAM3
#define OMV_SRAM3_ORIGIN                           0x30044000
#define OMV_SRAM3_LENGTH                           16K
#define OMV_SRAM4_ORIGIN                           0x38000000
#define OMV_SRAM4_LENGTH                           64K
#define OMV_AXI_SRAM_ORIGIN                        0x24000000
#define OMV_AXI_SRAM_LENGTH                        512K
#define OMV_DRAM_ORIGIN                            0xC0000000
#define OMV_DRAM_LENGTH                            64M

// Flash configuration.
#define OMV_FLASH_FFS_ORIGIN                       0x08020000
#define OMV_FLASH_FFS_LENGTH                       128K
#define OMV_FLASH_TXT_ORIGIN                       0x08040000
#define OMV_FLASH_TXT_LENGTH                       1792K

// Domain 1 DMA buffers region.
#define OMV_DMA_MEMORY_D1                          AXI_SRAM
#define OMV_DMA_MEMORY_D1_SIZE                     (16 * 1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D1_BASE                     (OMV_AXI_SRAM_ORIGIN + OMV_FB_OVERLAY_SIZE)
#define OMV_DMA_REGION_D1_SIZE                     MPU_REGION_SIZE_16KB

// Domain 2 DMA buffers region.
#define OMV_DMA_MEMORY_D2                          SRAM3
#define OMV_DMA_MEMORY_D2_SIZE                     (1 * 1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D2_BASE                     (OMV_SRAM3_ORIGIN + (0 * 1024))
#define OMV_DMA_REGION_D2_SIZE                     MPU_REGION_SIZE_16KB

// Domain 3 DMA buffers region.
#define OMV_DMA_MEMORY_D3                          SRAM4
#define OMV_DMA_MEMORY_D3_SIZE                     (64 * 1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D3_BASE                     (OMV_SRAM4_ORIGIN + (0 * 1024))
#define OMV_DMA_REGION_D3_SIZE                     MPU_REGION_SIZE_64KB

// AXI QoS - Low-High (0:15) - default 0
#define OMV_AXI_QOS_MDMA_R_PRI                     14 // Max pri to move data.
#define OMV_AXI_QOS_MDMA_W_PRI                     15 // Max pri to move data.
#define OMV_AXI_QOS_LTDC_R_PRI                     15 // Max pri to read out the frame buffer.

// Main image sensor I2C bus
#define ISC_I2C_ID                                 (1)
#define ISC_I2C_SPEED                              (OMV_I2C_SPEED_STANDARD)

// Thermal image sensor I2C bus
#define FIR_I2C_ID                                 (2)
#define FIR_I2C_SPEED                              (OMV_I2C_SPEED_FULL)

// Soft I2C bus.
#define SOFT_I2C_SIOC_PIN                          (&omv_pin_B10_GPIO)
#define SOFT_I2C_SIOD_PIN                          (&omv_pin_B11_GPIO)
#define SOFT_I2C_SPIN_DELAY                        64

// WINC1500 WiFi module SPI bus
#define WINC_SPI_ID                                (5)
#define WINC_SPI_BAUDRATE                          (50000000)
#define WINC_EN_PIN                                (&omv_pin_A0_GPIO)
#define WINC_RST_PIN                               (&omv_pin_C3_GPIO)
#define WINC_WAKE_PIN                              (&omv_pin_A1_GPIO)
#define WINC_CFG_PIN                               (&omv_pin_I15_GPIO)
#define WINC_IRQ_PIN                               (&omv_pin_H5_GPIO)

/* DCMI */
#define DCMI_TIM                                   (TIM1)
#define DCMI_TIM_PIN                               (&omv_pin_A8_TIM1)
#define DCMI_TIM_CHANNEL                           (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()                      __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()                     __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()                       HAL_RCC_GetPCLK2Freq()

#define DCMI_RESET_PIN                             (&omv_pin_A10_GPIO)
#define DCMI_POWER_PIN                             (&omv_pin_D7_GPIO)

#define DCMI_D0_PIN                                (&omv_pin_C6_DCMI)
#define DCMI_D1_PIN                                (&omv_pin_C7_DCMI)
#define DCMI_D2_PIN                                (&omv_pin_G10_DCMI)
#define DCMI_D3_PIN                                (&omv_pin_G11_DCMI)
#define DCMI_D4_PIN                                (&omv_pin_E4_DCMI)
#define DCMI_D5_PIN                                (&omv_pin_B6_DCMI)
#define DCMI_D6_PIN                                (&omv_pin_E5_DCMI)
#define DCMI_D7_PIN                                (&omv_pin_E6_DCMI)

#define DCMI_HSYNC_PIN                             (&omv_pin_A4_DCMI)
#define DCMI_VSYNC_PIN                             (&omv_pin_B7_DCMI)
#define DCMI_PXCLK_PIN                             (&omv_pin_A6_DCMI)

// Physical I2C buses.

// I2C bus 1
#define I2C1_ID                                    (1)
#define I2C1_SCL_PIN                               (&omv_pin_B8_I2C1)
#define I2C1_SDA_PIN                               (&omv_pin_B9_I2C1)

// I2C bus 2
#define I2C2_ID                                    (2)
#define I2C2_SCL_PIN                               (&omv_pin_B10_I2C2)
#define I2C2_SDA_PIN                               (&omv_pin_B11_I2C2)

// Physical SPI buses.

// SPI bus 2
#define SPI2_ID                                    (2)
#define SPI2_SCLK_PIN                              (&omv_pin_B13_SPI2)
#define SPI2_MISO_PIN                              (&omv_pin_B14_SPI2)
#define SPI2_MOSI_PIN                              (&omv_pin_B15_SPI2)
#define SPI2_SSEL_PIN                              (&omv_pin_B12_GPIO)
#define SPI2_DMA_TX_CHANNEL                        (DMA1_Stream4)
#define SPI2_DMA_RX_CHANNEL                        (DMA1_Stream3)

// SPI bus 3
#define SPI3_ID                                    (3)
#define SPI3_SCLK_PIN                              (&omv_pin_B3_SPI3)
#define SPI3_MISO_PIN                              (&omv_pin_B4_SPI3)
#define SPI3_MOSI_PIN                              (&omv_pin_B5_SPI3)
#define SPI3_SSEL_PIN                              (&omv_pin_A15_GPIO)
#define SPI3_DMA_TX_CHANNEL                        (DMA1_Stream7)
#define SPI3_DMA_RX_CHANNEL                        (DMA1_Stream2)

// SPI bus 5
#define SPI5_ID                                    (5)
#define SPI5_SCLK_PIN                              (&omv_pin_H6_SPI5)
#define SPI5_MISO_PIN                              (&omv_pin_H7_SPI5)
#define SPI5_MOSI_PIN                              (&omv_pin_J10_SPI5)
#define SPI5_SSEL_PIN                              (&omv_pin_K1_GPIO)
#define SPI5_DMA_TX_CHANNEL                        (DMA2_Stream4)
#define SPI5_DMA_RX_CHANNEL                        (DMA2_Stream3)

// LCD Interface
#define OMV_RGB_DISPLAY_CONTROLLER                 (LTDC)
#define OMV_RGB_DISPLAY_CLK_ENABLE()               __HAL_RCC_LTDC_CLK_ENABLE()
#define OMV_RGB_DISPLAY_CLK_DISABLE()              __HAL_RCC_LTDC_CLK_DISABLE()
#define OMV_RGB_DISPLAY_FORCE_RESET()              __HAL_RCC_LTDC_FORCE_RESET()
#define OMV_RGB_DISPLAY_RELEASE_RESET()            __HAL_RCC_LTDC_RELEASE_RESET()

#define OMV_RGB_DISPLAY_R0_PIN                     (&omv_pin_G13_LTDC)
#define OMV_RGB_DISPLAY_R1_PIN                     (&omv_pin_A2_LTDC)
#define OMV_RGB_DISPLAY_R2_PIN                     (&omv_pin_J1_LTDC)
#define OMV_RGB_DISPLAY_R3_PIN                     (&omv_pin_J2_LTDC)
#define OMV_RGB_DISPLAY_R4_PIN                     (&omv_pin_J3_LTDC)
#define OMV_RGB_DISPLAY_R5_PIN                     (&omv_pin_J4_LTDC)
#define OMV_RGB_DISPLAY_R6_PIN                     (&omv_pin_J5_LTDC)
#define OMV_RGB_DISPLAY_R7_PIN                     (&omv_pin_J0_LTDC)

#define OMV_RGB_DISPLAY_G0_PIN                     (&omv_pin_J7_LTDC)
#define OMV_RGB_DISPLAY_G1_PIN                     (&omv_pin_J8_LTDC)
#define OMV_RGB_DISPLAY_G2_PIN                     (&omv_pin_J9_LTDC)
#define OMV_RGB_DISPLAY_G3_PIN                     (&omv_pin_J12_LTDC)
#define OMV_RGB_DISPLAY_G4_PIN                     (&omv_pin_J11_LTDC)
#define OMV_RGB_DISPLAY_G5_PIN                     (&omv_pin_K0_LTDC)
#define OMV_RGB_DISPLAY_G6_PIN                     (&omv_pin_I11_LTDC)
#define OMV_RGB_DISPLAY_G7_PIN                     (&omv_pin_D3_LTDC)

#define OMV_RGB_DISPLAY_B0_PIN                     (&omv_pin_G14_LTDC)
#define OMV_RGB_DISPLAY_B1_PIN                     (&omv_pin_G12_LTDC)
#define OMV_RGB_DISPLAY_B2_PIN                     (&omv_pin_D6_LTDC)
#define OMV_RGB_DISPLAY_B3_PIN                     (&omv_pin_J15_LTDC)
#define OMV_RGB_DISPLAY_B4_PIN                     (&omv_pin_K3_LTDC)
#define OMV_RGB_DISPLAY_B5_PIN                     (&omv_pin_K4_LTDC)
#define OMV_RGB_DISPLAY_B6_PIN                     (&omv_pin_K5_LTDC)
#define OMV_RGB_DISPLAY_B7_PIN                     (&omv_pin_K6_LTDC)

#define OMV_RGB_DISPLAY_CLK_PIN                    (&omv_pin_I14_LTDC)
#define OMV_RGB_DISPLAY_DE_PIN                     (&omv_pin_K7_LTDC)
#define OMV_RGB_DISPLAY_HSYNC_PIN                  (&omv_pin_I12_LTDC)
#define OMV_RGB_DISPLAY_VSYNC_PIN                  (&omv_pin_I13_LTDC)
#define OMV_RGB_DISPLAY_DISP_PIN                   (&omv_pin_G9_GPIO)
#define OMV_RGB_DISPLAY_BL_PIN                     (&omv_pin_B0_TIM3)

#define OMV_DISPLAY_BL_TIM                         (TIM3)
#define OMV_DISPLAY_BL_TIM_FREQ                    (100000)
#define OMV_DISPLAY_BL_TIM_CHANNEL                 (TIM_CHANNEL_3)
#define OMV_DISPLAY_BL_TIM_CLK_ENABLE()            __HAL_RCC_TIM3_CLK_ENABLE()
#define OMV_DISPLAY_BL_TIM_CLK_DISABLE()           __HAL_RCC_TIM3_CLK_DISABLE()
#define OMV_DISPLAY_BL_TIM_FORCE_RESET()           __HAL_RCC_TIM3_FORCE_RESET()
#define OMV_DISPLAY_BL_TIM_RELEASE_RESET()         __HAL_RCC_TIM3_RELEASE_RESET()
#define OMV_DISPLAY_BL_TIM_PCLK_FREQ()             HAL_RCC_GetPCLK1Freq()

// SPI LCD Interface
#define OMV_SPI_DISPLAY_CONTROLLER                 (SPI2_ID)
#define OMV_SPI_DISPLAY_MOSI_PIN                   (&omv_pin_B15_SPI2)
#define OMV_SPI_DISPLAY_MISO_PIN                   (&omv_pin_B14_SPI2)
#define OMV_SPI_DISPLAY_SCLK_PIN                   (&omv_pin_B13_SPI2)
#define OMV_SPI_DISPLAY_SSEL_PIN                   (&omv_pin_B12_GPIO)

#define OMV_SPI_DISPLAY_RS_PIN                     (&omv_pin_D13_GPIO)
#define OMV_SPI_DISPLAY_RST_PIN                    (&omv_pin_D12_GPIO)
#define OMV_SPI_DISPLAY_BL_PIN                     (&omv_pin_A5_GPIO)
#define OMV_SPI_DISPLAY_TRIPLE_BUFFER              (1)

#define OMV_DISPLAY_BL_DAC                         (DAC1)
#define OMV_DISPLAY_BL_DAC_CHANNEL                 (DAC_CHANNEL_2)
#define OMV_DISPLAY_BL_DAC_CLK_ENABLE()            __HAL_RCC_DAC12_CLK_ENABLE()
#define OMV_DISPLAY_BL_DAC_CLK_DISABLE()           __HAL_RCC_DAC12_CLK_DISABLE()
#define OMV_DISPLAY_BL_DAC_FORCE_RESET()           __HAL_RCC_DAC12_FORCE_RESET()
#define OMV_DISPLAY_BL_DAC_RELEASE_RESET()         __HAL_RCC_DAC12_RELEASE_RESET()

// HDMI CEC/DDC I/O
#define OMV_DISPLAY_CEC_ENABLE                     (1)
#define OMV_DISPLAY_DDC_ENABLE                     (1)
#define OMV_CEC_PIN                                (&omv_pin_H2_GPIO)
#define OMV_DDC_SCL_PIN                            (pin_H3)
#define OMV_DDC_SDA_PIN                            (pin_H4)

// TFP410 DVI serializer
#define OMV_TFP410_ENABLE                          (1)
#define OMV_TFP410_RESET_PIN                       (&omv_pin_D11_GPIO)
#define OMV_TFP410_SCL_PIN                         (pin_B1)
#define OMV_TFP410_SDA_PIN                         (pin_B2)
#define OMV_TFP410_INT_PIN                         (&omv_pin_I8_GPIO)

// FT5X06 Touch Screen
#define OMV_FT5X06_ENABLE                          (1)
#define OMV_FT5X06_RESET_PIN                       (&omv_pin_K2_GPIO)
#define OMV_FT5X06_SCL_PIN                         (pin_J13)
#define OMV_FT5X06_SDA_PIN                         (pin_J14)
#define OMV_FT5X06_INT_PIN                         (&omv_pin_J6_GPIO)

// FIR Lepton
#define OMV_FIR_LEPTON_I2C_BUS                     (ISC_I2C_ID)
#define OMV_FIR_LEPTON_I2C_BUS_SPEED               (ISC_I2C_SPEED)

#define OMV_FIR_LEPTON_SPI_BUS                     (SPI3_ID)
#define OMV_FIR_LEPTON_MOSI_PIN                    (&omv_pin_B5_SPI3)
#define OMV_FIR_LEPTON_MISO_PIN                    (&omv_pin_B4_SPI3)
#define OMV_FIR_LEPTON_SCLK_PIN                    (&omv_pin_B3_SPI3)
#define OMV_FIR_LEPTON_SSEL_PIN                    (&omv_pin_A15_GPIO)

#define OMV_FIR_LEPTON_RESET_PIN                   (&omv_pin_D5_GPIO)
#define OMV_FIR_LEPTON_POWER_PIN                   (&omv_pin_D4_GPIO)
#define OMV_FIR_LEPTON_VSYNC_PIN                   (&omv_pin_E3_GPIO)

#define OMV_FIR_LEPTON_MCLK_PIN                    (&omv_pin_A3_TIM15)
#define OMV_FIR_LEPTON_MCLK_FREQ                   (24000000)

#define OMV_FIR_LEPTON_MCLK_TIM                    (TIM15)
#define OMV_FIR_LEPTON_MCLK_TIM_CHANNEL            (TIM_CHANNEL_2)
#define OMV_FIR_LEPTON_MCLK_TIM_CLK_ENABLE()       __HAL_RCC_TIM15_CLK_ENABLE()
#define OMV_FIR_LEPTON_MCLK_TIM_CLK_DISABLE()      __HAL_RCC_TIM15_CLK_DISABLE()
#define OMV_FIR_LEPTON_MCLK_TIM_FORCE_RESET()      __HAL_RCC_TIM15_FORCE_RESET()
#define OMV_FIR_LEPTON_MCLK_TIM_RELEASE_RESET()    __HAL_RCC_TIM15_RELEASE_RESET()
#define OMV_FIR_LEPTON_MCLK_TIM_PCLK_FREQ()        HAL_RCC_GetPCLK2Freq()

// Buzzer
#define OMV_BUZZER_PIN                             (&omv_pin_A1_TIM2)
#define OMV_BUZZER_FREQ                            (4000)

#define OMV_BUZZER_TIM                             (TIM2)
#define OMV_BUZZER_TIM_CHANNEL                     (TIM_CHANNEL_2)
#define OMV_BUZZER_TIM_CLK_ENABLE()                __HAL_RCC_TIM2_CLK_ENABLE()
#define OMV_BUZZER_TIM_CLK_DISABLE()               __HAL_RCC_TIM2_CLK_DISABLE()
#define OMV_BUZZER_TIM_FORCE_RESET()               __HAL_RCC_TIM2_FORCE_RESET()
#define OMV_BUZZER_TIM_RELEASE_RESET()             __HAL_RCC_TIM2_RELEASE_RESET()
#define OMV_BUZZER_TIM_PCLK_FREQ()                 HAL_RCC_GetPCLK1Freq()

#endif //__OMV_BOARDCONFIG_H__

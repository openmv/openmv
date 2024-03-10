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
#define OMV_ARCH_STR                          "OMV4 H7 PRO 32768 SDRAM" // 33 chars max
#define OMV_BOARD_TYPE                        "M4"
#define OMV_UNIQUE_ID_ADDR                    0x1FF1E800 // Unique ID address.
#define OMV_UNIQUE_ID_SIZE                    3 // Unique ID size in words.
#define OMV_UNIQUE_ID_OFFSET                  4 // Bytes offset for multi-word UIDs.

#define OMV_XCLK_MCO                          (0U)
#define OMV_XCLK_TIM                          (1U)

// Sensor external clock source.
#define OMV_XCLK_SOURCE                       (OMV_XCLK_TIM)

// Sensor external clock timer frequency.
#define OMV_XCLK_FREQUENCY                    (24000000)

// Enable hardware JPEG
#define OMV_HARDWARE_JPEG                     (1)

// MDMA configuration
#define OMV_MDMA_CHANNEL_DCMI_0               (0)
#define OMV_MDMA_CHANNEL_DCMI_1               (1)
#define OMV_MDMA_CHANNEL_JPEG_IN              (7) // in has a lower pri than out
#define OMV_MDMA_CHANNEL_JPEG_OUT             (6) // out has a higher pri than in

// Enable additional GPIO banks
#define OMV_ENABLE_GPIO_BANK_F                (1)
#define OMV_ENABLE_GPIO_BANK_G                (1)
#define OMV_ENABLE_GPIO_BANK_H                (1)
#define OMV_ENABLE_GPIO_BANK_I                (1)
#define OMV_ENABLE_GPIO_BANK_J                (1)
#define OMV_ENABLE_GPIO_BANK_K                (1)

// Configure image sensor drivers
#define OMV_ENABLE_OV2640                     (1)
#define OMV_ENABLE_OV5640                     (1)
#define OMV_ENABLE_OV7690                     (0)
#define OMV_ENABLE_OV7725                     (1)
#define OMV_ENABLE_OV9650                     (1)
#define OMV_ENABLE_MT9M114                    (1)
#define OMV_ENABLE_MT9V0XX                    (1)
#define OMV_ENABLE_LEPTON                     (1)
#define OMV_ENABLE_HM01B0                     (0)
#define OMV_ENABLE_PAJ6100                    (1)
#define OMV_ENABLE_FROGEYE2020                (1)

// Configure FIR sensors drivers
#define OMV_ENABLE_FIR_MLX90621               (1)
#define OMV_ENABLE_FIR_MLX90640               (1)
#define OMV_ENABLE_FIR_MLX90641               (1)
#define OMV_ENABLE_FIR_AMG8833                (1)
#define OMV_ENABLE_FIR_LEPTON                 (0)
#define OMV_ENABLE_TOF_VL53L5CX               (1)

// OV7725 sensor settings
#define OMV_OV7725_PLL_CONFIG                 (0x41) // x4
#define OMV_OV7725_BANDING                    (0x7F)

// OV5640 sensor settings
#define OMV_OV5640_XCLK_FREQ                  (24000000)
#define OMV_OV5640_PLL_CTRL2                  (0x64)
#define OMV_OV5640_PLL_CTRL3                  (0x13)
#define OMV_OV5640_REV_Y_CHECK                (1)
#define OMV_OV5640_REV_Y_FREQ                 (25000000)
#define OMV_OV5640_REV_Y_CTRL2                (0x54)
#define OMV_OV5640_REV_Y_CTRL3                (0x13)
#define OMV_ENABLE_OV5640_AF                  (0)

// Enable WiFi debug
#define OMV_ENABLE_WIFIDBG                    (0)

// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH                   (1920 * 1080 * 2)

// Low and high JPEG QS.
#define JPEG_QUALITY_LOW                      50
#define JPEG_QUALITY_HIGH                     90

// FB Heap Block Size
#define OMV_UMM_BLOCK_SIZE                    256

// USB IRQn.
#define OMV_USB_IRQN                          (OTG_HS_IRQn)
#define OMV_USB_ULPI                          (1)
#define OMV_USB_ULPI_DIR_PIN                  (&omv_pin_I11_OTG_HS)
#define OMV_USB_ULPI_STP_PIN                  (&omv_pin_C0_OTG_HS)
#define OMV_USB_ULPI_DIR_CLK_ENABLE()         __HAL_RCC_GPIOI_CLK_ENABLE()

//PLL1 48MHz for USB, SDMMC and FDCAN
#define OMV_OSC_PLL1M                         (6)
#define OMV_OSC_PLL1N                         (240)
#define OMV_OSC_PLL1P                         (2)
#define OMV_OSC_PLL1Q                         (20)
#define OMV_OSC_PLL1R                         (2)
#define OMV_OSC_PLL1VCI                       (RCC_PLL1VCIRANGE_2)
#define OMV_OSC_PLL1VCO                       (RCC_PLL1VCOWIDE)
#define OMV_OSC_PLL1FRAC                      (0)

// PLL2 200MHz for FMC and QSPI.
#define OMV_OSC_PLL2M                         (6)
#define OMV_OSC_PLL2N                         (100)
#define OMV_OSC_PLL2P                         (2)
#define OMV_OSC_PLL2Q                         (2)
#define OMV_OSC_PLL2R                         (2)
#define OMV_OSC_PLL2VCI                       (RCC_PLL2VCIRANGE_2)
#define OMV_OSC_PLL2VCO                       (RCC_PLL2VCOWIDE)
#define OMV_OSC_PLL2FRAC                      (0)

// PLL3P 160MHz for SPI123
// PLL3Q 80MHz for SPI6
// PLL3R 160MHz for ADC
#define OMV_OSC_PLL3M                         (6)
#define OMV_OSC_PLL3N                         (80)
#define OMV_OSC_PLL3P                         (2)
#define OMV_OSC_PLL3Q                         (4)
#define OMV_OSC_PLL3R                         (2)
#define OMV_OSC_PLL3VCI                       (RCC_PLL3VCIRANGE_2)
#define OMV_OSC_PLL3VCO                       (RCC_PLL3VCOWIDE)
#define OMV_OSC_PLL3FRAC                      (0)

// Clock Sources
#define OMV_OSC_PLL_CLKSOURCE                 RCC_PLLSOURCE_HSE
#define OMV_OSC_USB_CLKSOURCE                 RCC_USBCLKSOURCE_HSI48
#define OMV_OSC_RNG_CLKSOURCE                 RCC_RNGCLKSOURCE_HSI48
#define OMV_OSC_ADC_CLKSOURCE                 RCC_ADCCLKSOURCE_PLL3
#define OMV_OSC_SPI123_CLKSOURCE              RCC_SPI123CLKSOURCE_PLL3
#define OMV_OSC_SPI6_CLKSOURCE                RCC_SPI6CLKSOURCE_PLL3

// HSE/HSI/CSI State
#define OMV_OSC_LSE_STATE                     (RCC_LSE_BYPASS)
#define OMV_OSC_HSE_STATE                     (RCC_HSE_BYPASS)
#define OMV_OSC_HSI48_STATE                   (RCC_HSI48_ON)

// Flash Latency
#define OMV_FLASH_LATENCY                     (FLASH_LATENCY_2)

// Power supply configuration
#define OMV_PWR_SUPPLY                        (PWR_LDO_SUPPLY)

// Linker script constants (see the linker script template stm32fxxx.ld.S).
// Note: fb_alloc is a stack-based, dynamically allocated memory on FB.
// The maximum available fb_alloc memory = FB_ALLOC_SIZE + FB_SIZE - (w*h*bpp).
#define OMV_MAIN_MEMORY                       SRAM1 // data, bss and heap
#define OMV_STACK_MEMORY                      ITCM // stack memory
#define OMV_DMA_MEMORY                        SRAM3 // Misc DMA buffers memory.
#define OMV_FB_MEMORY                         DRAM // Framebuffer, fb_alloc
#define OMV_JPEG_MEMORY                       DRAM // JPEG buffer memory buffer.
#define OMV_JPEG_MEMORY_OFFSET                (31M) // JPEG buffer is placed after FB/fballoc memory.
#define OMV_VOSPI_MEMORY                      SRAM4 // VoSPI buffer memory.
#define OMV_VOSPI_MEMORY_OFFSET               (4K) // First 4K reserved for D3 DMA buffers.
#define OMV_FB_OVERLAY_MEMORY                 AXI_SRAM // Fast fb_alloc memory.
#define OMV_CYW43_MEMORY                      FLASH_EXT // CYW43 firmware in external flash mmap'd flash.
#define OMV_CYW43_MEMORY_OFFSET               (0x90F00000)// Last Mbyte.

#define OMV_FB_SIZE                           (20M) // FB memory: header + VGA/GS image
#define OMV_FB_ALLOC_SIZE                     (11M) // minimum fb alloc size
#define OMV_FB_OVERLAY_SIZE                   (496 * 1024) // Fast fb_alloc memory size.
#define OMV_STACK_SIZE                        (64K)
#define OMV_HEAP_SIZE                         (196K)
#define OMV_SDRAM_SIZE                        (32 * 1024 * 1024) // This needs to be here for UVC firmware.

#define OMV_LINE_BUF_SIZE                     (11 * 1024) // Image line buffer round(2592 * 2BPP * 2 buffers).
#define OMV_MSC_BUF_SIZE                      (2K) // USB MSC bot data
#define OMV_VFS_BUF_SIZE                      (1K) // VFS struct + FATFS file buffer (624 bytes)
#define OMV_FIR_LEPTON_BUF_SIZE               (1K) // FIR Lepton Packet Double Buffer (328 bytes)
#define OMV_JPEG_BUF_SIZE                     (1024 * 1024) // IDE JPEG buffer (header + data).

// Memory map.
#define OMV_FLASH_ORIGIN                      0x08000000
#define OMV_FLASH_LENGTH                      2048K
#define OMV_DTCM_ORIGIN                       0x20000000 // Note accessible by CPU and MDMA only.
#define OMV_DTCM_LENGTH                       128K
#define OMV_ITCM_ORIGIN                       0x00000000
#define OMV_ITCM_LENGTH                       64K
#define OMV_SRAM1_ORIGIN                      0x30000000
#define OMV_SRAM1_LENGTH                      256K // SRAM1 + SRAM2 + 1/2 SRAM3
#define OMV_SRAM3_ORIGIN                      0x30040000
#define OMV_SRAM3_LENGTH                      32K
#define OMV_SRAM4_ORIGIN                      0x38000000
#define OMV_SRAM4_LENGTH                      64K
#define OMV_AXI_SRAM_ORIGIN                   0x24000000
#define OMV_AXI_SRAM_LENGTH                   512K
#define OMV_DRAM_ORIGIN                       0xC0000000
#define OMV_DRAM_LENGTH                       32M

// Flash configuration.
#define OMV_FLASH_FFS_ORIGIN                  0x08020000
#define OMV_FLASH_FFS_LENGTH                  128K
#define OMV_FLASH_TXT_ORIGIN                  0x08040000
#define OMV_FLASH_TXT_LENGTH                  1792K
#define OMV_FLASH_EXT_ORIGIN                  0x90000000
#define OMV_FLASH_EXT_LENGTH                  32M

// Domain 1 DMA buffers region.
#define OMV_DMA_MEMORY_D1                     AXI_SRAM
#define OMV_DMA_MEMORY_D1_SIZE                (8 * 1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D1_BASE                (OMV_AXI_SRAM_ORIGIN + OMV_FB_OVERLAY_SIZE)
#define OMV_DMA_REGION_D1_SIZE                MPU_REGION_SIZE_8KB

// Domain 2 DMA buffers region.
#define OMV_DMA_MEMORY_D2                     SRAM3
#define OMV_DMA_MEMORY_D2_SIZE                (8 * 1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D2_BASE                (OMV_SRAM3_ORIGIN + (0 * 1024))
#define OMV_DMA_REGION_D2_SIZE                MPU_REGION_SIZE_32KB

// Domain 3 DMA buffers region.
#define OMV_DMA_MEMORY_D3                     SRAM4
#define OMV_DMA_MEMORY_D3_SIZE                (4 * 1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D3_BASE                (OMV_SRAM4_ORIGIN + (0 * 1024))
#define OMV_DMA_REGION_D3_SIZE                MPU_REGION_SIZE_4KB

// AXI QoS - Low-High (0:15) - default 0
#define OMV_AXI_QOS_MDMA_R_PRI                15 // Max pri to move data.
#define OMV_AXI_QOS_MDMA_W_PRI                15 // Max pri to move data.

// Image sensor I2C configuration
#define ISC_I2C_ID                            (2)
#define ISC_I2C_SPEED                         (OMV_I2C_SPEED_STANDARD)

// FIR sensor I2C configuration
#define FIR_I2C_ID                            (1)
#define FIR_I2C_SPEED                         (OMV_I2C_SPEED_STANDARD)

// TOF sensor I2C configuration
#define TOF_I2C_ID                            (4)
#define TOF_I2C_SPEED                         (OMV_I2C_SPEED_STANDARD)

// IMU I2C configuration
#define IMU_I2C_ID                            (4)
#define IMU_I2C_SPEED                         (0x4030040B)
#define IMU_CHIP_LSM6DS3                      (1)
#define OMV_IMU_X_Y_ROTATION_DEGREES          (0)
#define OMV_IMU_MOUNTING_Z_DIRECTION          (-1)

// DCMI timer configuration
#define DCMI_TIM                              (TIM1)
#define DCMI_TIM_PIN                          (&omv_pin_A8_TIM1)
#define DCMI_TIM_CHANNEL                      (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()                 __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()                __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()                  HAL_RCC_GetPCLK2Freq()

// DCMI pins configuration
#define DCMI_RESET_PIN                        (&omv_pin_C13_GPIO)
#define DCMI_POWER_PIN                        (&omv_pin_I8_GPIO)
#define DCMI_FSYNC_PIN                        (&omv_pin_D5_GPIO)

#define DCMI_D0_PIN                           (&omv_pin_A9_DCMI)
#define DCMI_D1_PIN                           (&omv_pin_A10_DCMI)
#define DCMI_D2_PIN                           (&omv_pin_G10_DCMI)
#define DCMI_D3_PIN                           (&omv_pin_G11_DCMI)
#define DCMI_D4_PIN                           (&omv_pin_E4_DCMI)
#define DCMI_D5_PIN                           (&omv_pin_D3_DCMI)
#define DCMI_D6_PIN                           (&omv_pin_E5_DCMI)
#define DCMI_D7_PIN                           (&omv_pin_E6_DCMI)

#define DCMI_VSYNC_PIN                        (&omv_pin_G9_DCMI)
#define DCMI_HSYNC_PIN                        (&omv_pin_A4_DCMI)
#define DCMI_PXCLK_PIN                        (&omv_pin_A6_DCMI)

// Physical I2C buses.
// I2C bus 1
#define I2C1_ID                               (1)
#define I2C1_SCL_PIN                          (&omv_pin_B8_I2C1)
#define I2C1_SDA_PIN                          (&omv_pin_B9_I2C1)
// I2C bus 2
#define I2C2_ID                               (2)
#define I2C2_SCL_PIN                          (&omv_pin_H4_I2C2)
#define I2C2_SDA_PIN                          (&omv_pin_H5_I2C2)
// I2C bus 4
#define I2C4_ID                               (4)
#define I2C4_SCL_PIN                          (&omv_pin_B6_I2C4)
#define I2C4_SDA_PIN                          (&omv_pin_B7_I2C4)

#define ISC_SPI                               (SPI6)
// SPI6 clock source is PLL3Q (80MHz/4 == 20MHz) - Minimum (164*240*8*27 = 8,501,760Hz)
#define ISC_SPI_PRESCALER                     (SPI_BAUDRATEPRESCALER_4)

#define ISC_SPI_IRQn                          (SPI6_IRQn)
#define ISC_SPI_IRQHandler                    (SPI6_IRQHandler)

#define ISC_SPI_DMA_IRQn                      (BDMA_Channel1_IRQn)
#define ISC_SPI_DMA_STREAM                    (BDMA_Channel1)

#define ISC_SPI_DMA_REQUEST                   (BDMA_REQUEST_SPI6_RX)
#define ISC_SPI_DMA_IRQHandler                (BDMA_Channel1_IRQHandler)

#define ISC_SPI_RESET()                       __HAL_RCC_SPI6_FORCE_RESET()
#define ISC_SPI_RELEASE()                     __HAL_RCC_SPI6_RELEASE_RESET()

#define ISC_SPI_CLK_ENABLE()                  __HAL_RCC_SPI6_CLK_ENABLE()
#define ISC_SPI_CLK_DISABLE()                 __HAL_RCC_SPI6_CLK_DISABLE()
#define ISC_SPI_DMA_CLK_ENABLE()              __HAL_RCC_BDMA_CLK_ENABLE()

#define ISC_SPI_SCLK_PIN                      (&omv_pin_G13_SPI6)
#define ISC_SPI_MISO_PIN                      (&omv_pin_G12_SPI6)
#define ISC_SPI_MOSI_PIN                      (&omv_pin_G14_SPI6)
#define ISC_SPI_SSEL_PIN                      (&omv_pin_A15_SPI6)

#if 0
// SPI LCD Interface
#define OMV_SPI_DISPLAY_CONTROLLER            (&spi_obj[1])
#define OMV_SPI_DISPLAY_MOSI_PIN              (&omv_pin_C1_SPI2)
#define OMV_SPI_DISPLAY_MISO_PIN              (&omv_pin_C2_SPI2)
#define OMV_SPI_DISPLAY_SCLK_PIN              (&omv_pin_A12_SPI2)
#define OMV_SPI_DISPLAY_SSEL_PIN              (&omv_pin_A11_GPIO)

#define OMV_SPI_DISPLAY_RST_PIN               (&omv_pin_C6_GPIO)
#define OMV_SPI_DISPLAY_RS_PIN                (&omv_pin_C7_GPIO)
#define OMV_SPI_DISPLAY_BL_PIN                (&omv_pin_C5_GPIO)

// FIR Lepton
#define OMV_FIR_LEPTON_I2C_BUS                (FIR_I2C_ID)
#define OMV_FIR_LEPTON_I2C_BUS_SPEED          (FIR_I2C_SPEED)
#define OMV_FIR_LEPTON_CONTROLLER             (&spi_obj[1])
#define OMV_FIR_LEPTON_CONTROLLER_INSTANCE    (SPI2)

#define OMV_FIR_LEPTON_MOSI_PORT              (&omv_pin_C1_SPI2)
#define OMV_FIR_LEPTON_MISO_PORT              (&omv_pin_C2_SPI2)
#define OMV_FIR_LEPTON_SCLK_PORT              (&omv_pin_A12_SPI2)
#define OMV_FIR_LEPTON_CS_PORT                (&omv_pin_A11_GPIO)
#endif
#endif //__OMV_BOARDCONFIG_H__

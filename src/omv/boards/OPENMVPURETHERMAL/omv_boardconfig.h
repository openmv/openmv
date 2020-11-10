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
#define OMV_ARCH_STR                    "OPENMVPURETHERMAL 65536 SDRAM" // 33 chars max
#define OMV_BOARD_TYPE                  "H7"
#define OMV_UNIQUE_ID_ADDR              0x1FF1E800

// Flash sectors for the bootloader.
// Flash FS sector, main FW sector, max sector.
#define OMV_FLASH_LAYOUT                {1, 2, 15}

// QSPI Flash layout for the bootloader.
// First block, maximum block, block size in bytes.
#define OMV_QSPIF_LAYOUT                {0, 511, 64*1024}

#define OMV_XCLK_MCO                    (0U)
#define OMV_XCLK_TIM                    (1U)

// Sensor external clock source.
#define OMV_XCLK_SOURCE                 (OMV_XCLK_TIM)

// Sensor external clock timer frequency.
#define OMV_XCLK_FREQUENCY              (12000000)

// Sensor PLL register value.
#define OMV_OV7725_PLL_CONFIG           (0x41)  // x4

// Sensor Banding Filter Value
#define OMV_OV7725_BANDING              (0x7F)

// Bootloader LED GPIO port/pin
#define OMV_BOOTLDR_LED_PIN             (GPIO_PIN_1)
#define OMV_BOOTLDR_LED_PORT            (GPIOC)

// RAW buffer size
#define OMV_RAW_BUF_SIZE                (33554432)

// Enable hardware JPEG
#define OMV_HARDWARE_JPEG               (1)

// Enable sensor drivers
#define OMV_ENABLE_OV2640               (0)
#define OMV_ENABLE_OV5640               (1)
#define OMV_ENABLE_OV7690               (0)
#define OMV_ENABLE_OV7725               (0)
#define OMV_ENABLE_OV9650               (0)
#define OMV_ENABLE_MT9V034              (0)
#define OMV_ENABLE_LEPTON               (1)
#define OMV_ENABLE_HM01B0               (0)

// Enable WiFi debug
#define OMV_ENABLE_WIFIDBG              (1)

// Enable self-tests on first boot
#define OMV_ENABLE_SELFTEST             (0)

// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH             (1920*1080*2)

// Low and high JPEG QS.
#define JPEG_QUALITY_LOW                50
#define JPEG_QUALITY_HIGH               90

// FB Heap Block Size
#define OMV_UMM_BLOCK_SIZE              256

// Core VBAT for selftests
#define OMV_CORE_VBAT                   "3.3"

// Clock Sources
#define OMV_OSC_USB_CLKSOURCE           RCC_USBCLKSOURCE_PLL
#define OMV_OSC_RNG_CLKSOURCE           RCC_RNGCLKSOURCE_HSI48
#define OMV_OSC_ADC_CLKSOURCE           RCC_ADCCLKSOURCE_PLL2
#define OMV_OSC_SPI123_CLKSOURCE        RCC_SPI123CLKSOURCE_PLL2

//PLL1 48MHz for USB, SDMMC and FDCAN
#define OMV_OSC_PLL1M                   (3)
#define OMV_OSC_PLL1N                   (240)
#define OMV_OSC_PLL1P                   (2)
#define OMV_OSC_PLL1Q                   (20)
#define OMV_OSC_PLL1R                   (2)
#define OMV_OSC_PLL1VCI                 (RCC_PLL1VCIRANGE_2)
#define OMV_OSC_PLL1VCO                 (RCC_PLL1VCOWIDE)
#define OMV_OSC_PLL1FRAC                (0)

// PLL2 200MHz for FMC and QSPI.
#define OMV_OSC_PLL2M                   (3)
#define OMV_OSC_PLL2N                   (100)
#define OMV_OSC_PLL2P                   (2)
#define OMV_OSC_PLL2Q                   (2)
#define OMV_OSC_PLL2R                   (2)
#define OMV_OSC_PLL2VCI                 (RCC_PLL2VCIRANGE_2)
#define OMV_OSC_PLL2VCO                 (RCC_PLL2VCOWIDE)
#define OMV_OSC_PLL2FRAC                (0)

// PLL3 160MHz for ADC and SPI123
#define OMV_OSC_PLL3M                   (3)
#define OMV_OSC_PLL3N                   (80)
#define OMV_OSC_PLL3P                   (2)
#define OMV_OSC_PLL3Q                   (2)
#define OMV_OSC_PLL3R                   (2)
#define OMV_OSC_PLL3VCI                 (RCC_PLL3VCIRANGE_2)
#define OMV_OSC_PLL3VCO                 (RCC_PLL3VCOWIDE)
#define OMV_OSC_PLL3FRAC                (0)

// HSE/HSI/CSI State
#define OMV_OSC_HSE_STATE               (RCC_HSE_ON)
#define OMV_OSC_HSI48_STATE             (RCC_HSI48_ON)

// Flash Latency
#define OMV_FLASH_LATENCY               (FLASH_LATENCY_2)

// Linker script constants (see the linker script template stm32fxxx.ld.S).
// Note: fb_alloc is a stack-based, dynamically allocated memory on FB.
// The maximum available fb_alloc memory = FB_ALLOC_SIZE + FB_SIZE - (w*h*bpp).
#define OMV_FFS_MEMORY                  DTCM        // Flash filesystem cache memory
#define OMV_MAIN_MEMORY                 SRAM1       // data, bss and heap
#define OMV_STACK_MEMORY                ITCM        // stack memory
#define OMV_DMA_MEMORY                  AXI_SRAM    // DMA buffers memory.
#define OMV_FB_MEMORY                   DRAM        // Framebuffer, fb_alloc
#define OMV_JPEG_MEMORY                 DRAM        // JPEG buffer memory buffer.
#define OMV_JPEG_MEMORY_OFFSET          (63M)       // JPEG buffer is placed after FB/fballoc memory.
#define OMV_VOSPI_MEMORY                SRAM4       // VoSPI buffer memory.
#define OMV_FB_OVERLAY_MEMORY           AXI_SRAM    // _fballoc_overlay memory.
#define OMV_FB_OVERLAY_MEMORY_OFFSET    (480*1024)  // _fballoc_overlay

#define OMV_FB_SIZE                     (32M)       // FB memory: header + VGA/GS image
#define OMV_FB_ALLOC_SIZE               (31M)       // minimum fb alloc size
#define OMV_STACK_SIZE                  (64K)
#define OMV_HEAP_SIZE                   (244K)
#define OMV_SDRAM_SIZE                  (64 * 1024 * 1024) // This needs to be here for UVC firmware.
#define OMV_SDRAM_TEST                  (0)

#define OMV_LINE_BUF_SIZE               (11 * 1024) // Image line buffer round(2592 * 2BPP * 2 buffers).
#define OMV_MSC_BUF_SIZE                (12K)       // USB MSC bot data
#define OMV_VFS_BUF_SIZE                (1K)        // VFS sturct + FATFS file buffer (624 bytes)
#define OMV_JPEG_BUF_SIZE               (1024*1024) // IDE JPEG buffer (header + data).

#define OMV_BOOT_ORIGIN                 0x08000000
#define OMV_BOOT_LENGTH                 128K
#define OMV_TEXT_ORIGIN                 0x08040000
#define OMV_TEXT_LENGTH                 1792K
#define OMV_DTCM_ORIGIN                 0x20000000  // Note accessible by CPU and MDMA only.
#define OMV_DTCM_LENGTH                 128K
#define OMV_ITCM_ORIGIN                 0x00000000
#define OMV_ITCM_LENGTH                 64K
#define OMV_SRAM1_ORIGIN                0x30000000
#define OMV_SRAM1_LENGTH                256K
#define OMV_SRAM3_ORIGIN                0x30040000
#define OMV_SRAM3_LENGTH                32K
#define OMV_SRAM4_ORIGIN                0x38000000
#define OMV_SRAM4_LENGTH                64K
#define OMV_AXI_SRAM_ORIGIN             0x24000000
#define OMV_AXI_SRAM_LENGTH             512K
#define OMV_DRAM_ORIGIN                 0xC0000000
#define OMV_DRAM_LENGTH                 64M
#define OMV_FB_OVERLAY_MEMORY_ORIGIN    OMV_AXI_SRAM_ORIGIN

// Use the MPU to set an uncacheable memory region.
#define OMV_DMA_REGION_BASE             (OMV_AXI_SRAM_ORIGIN+OMV_FB_OVERLAY_MEMORY_OFFSET)
#define OMV_DMA_REGION_SIZE             MPU_REGION_SIZE_32KB

/* SCCB/I2C */
#define SCCB_I2C                        (I2C1)
#define SCCB_AF                         (GPIO_AF4_I2C1)
#define SCCB_CLK_ENABLE()               __I2C1_CLK_ENABLE()
#define SCCB_CLK_DISABLE()              __I2C1_CLK_DISABLE()
#define SCCB_PORT                       (GPIOB)
#define SCCB_SCL_PIN                    (GPIO_PIN_8)
#define SCCB_SDA_PIN                    (GPIO_PIN_9)
#define SCCB_TIMING                     (I2C_TIMING_STANDARD)
#define SCCB_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define SCCB_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()

/* FIR I2C */
#define FIR_I2C                         (I2C2)
#define FIR_I2C_AF                      (GPIO_AF4_I2C2)
#define FIR_I2C_CLK_ENABLE()            __I2C2_CLK_ENABLE()
#define FIR_I2C_CLK_DISABLE()           __I2C2_CLK_DISABLE()
#define FIR_I2C_PORT                    (GPIOB)
#define FIR_I2C_SCL_PIN                 (GPIO_PIN_10)
#define FIR_I2C_SDA_PIN                 (GPIO_PIN_11)
#define FIR_I2C_TIMING                  (I2C_TIMING_FULL)
#define FIR_I2C_FORCE_RESET()           __HAL_RCC_I2C2_FORCE_RESET()
#define FIR_I2C_RELEASE_RESET()         __HAL_RCC_I2C2_RELEASE_RESET()

/* DCMI */
#define DCMI_TIM                        (TIM1)
#define DCMI_TIM_PIN                    (GPIO_PIN_8)
#define DCMI_TIM_PORT                   (GPIOA)
#define DCMI_TIM_AF                     (GPIO_AF1_TIM1)
#define DCMI_TIM_CHANNEL                (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()           __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()          __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()            HAL_RCC_GetPCLK2Freq()

#define DCMI_RESET_PIN                  (GPIO_PIN_10)
#define DCMI_RESET_PORT                 (GPIOA)

#define DCMI_PWDN_PIN                   (GPIO_PIN_7)
#define DCMI_PWDN_PORT                  (GPIOD)

#define DCMI_D0_PIN                     (GPIO_PIN_6)
#define DCMI_D1_PIN                     (GPIO_PIN_7)
#define DCMI_D2_PIN                     (GPIO_PIN_10)
#define DCMI_D3_PIN                     (GPIO_PIN_11)
#define DCMI_D4_PIN                     (GPIO_PIN_4)
#define DCMI_D5_PIN                     (GPIO_PIN_6)
#define DCMI_D6_PIN                     (GPIO_PIN_5)
#define DCMI_D7_PIN                     (GPIO_PIN_6)

#define DCMI_D0_PORT                    (GPIOC)
#define DCMI_D1_PORT                    (GPIOC)
#define DCMI_D2_PORT                    (GPIOG)
#define DCMI_D3_PORT                    (GPIOG)
#define DCMI_D4_PORT                    (GPIOE)
#define DCMI_D5_PORT                    (GPIOB)
#define DCMI_D6_PORT                    (GPIOE)
#define DCMI_D7_PORT                    (GPIOE)

#define DCMI_HSYNC_PIN                  (GPIO_PIN_4)
#define DCMI_VSYNC_PIN                  (GPIO_PIN_7)
#define DCMI_PXCLK_PIN                  (GPIO_PIN_6)

#define DCMI_HSYNC_PORT                 (GPIOA)
#define DCMI_VSYNC_PORT                 (GPIOB)
#define DCMI_PXCLK_PORT                 (GPIOA)

#define DCMI_RESET_LOW()                HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_RESET)
#define DCMI_RESET_HIGH()               HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_SET)

#define DCMI_PWDN_LOW()                 HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_RESET)
#define DCMI_PWDN_HIGH()                HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_SET)

#define DCMI_VSYNC_IRQN                 EXTI9_5_IRQn
#define DCMI_VSYNC_IRQ_LINE             (7)

#define WINC_SPI                        (SPI5)
#define WINC_SPI_AF                     (GPIO_AF5_SPI5)
#define WINC_SPI_TIMEOUT                (1000)
// SPI5 clock source is PLL2 (200MHz/4 == 50MHz) - Slightly over the maximum of 48 MHz.
#define WINC_SPI_PRESCALER              (SPI_BAUDRATEPRESCALER_4)
#define WINC_SPI_CLK_ENABLE()           __HAL_RCC_SPI5_CLK_ENABLE()
#define WINC_SPI_HANDLE                 SPIHandle5

#define WINC_SPI_SCLK_PIN               (GPIO_PIN_6)
#define WINC_SPI_MISO_PIN               (GPIO_PIN_7)
#define WINC_SPI_MOSI_PIN               (GPIO_PIN_10)

#define WINC_SPI_SCLK_PORT              (GPIOH)
#define WINC_SPI_MISO_PORT              (GPIOH)
#define WINC_SPI_MOSI_PORT              (GPIOJ)

#define WINC_EN_PIN                     (GPIO_PIN_0)
#define WINC_CS_PIN                     (GPIO_PIN_1)
#define WINC_RST_PIN                    (GPIO_PIN_3)
#define WINC_IRQ_PIN                    (pin_H5)
#define WINC_WAKE_PIN                   (GPIO_PIN_1)
#define WINC_CFG_PIN                    (GPIO_PIN_15)

#define WINC_EN_PORT                    (GPIOA)
#define WINC_CS_PORT                    (GPIOK)
#define WINC_RST_PORT                   (GPIOC)
#define WINC_WAKE_PORT                  (GPIOA)
#define WINC_CFG_PORT                   (GPIOI)

#define WINC_CS_LOW()                   HAL_GPIO_WritePin(WINC_CS_PORT, WINC_CS_PIN, GPIO_PIN_RESET)
#define WINC_CS_HIGH()                  HAL_GPIO_WritePin(WINC_CS_PORT, WINC_CS_PIN, GPIO_PIN_SET)

#define SOFT_I2C_PORT                   GPIOB
#define SOFT_I2C_SIOC_PIN               GPIO_PIN_10
#define SOFT_I2C_SIOD_PIN               GPIO_PIN_11

#define SOFT_I2C_SIOC_H()               HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOC_PIN, GPIO_PIN_SET)
#define SOFT_I2C_SIOC_L()               HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOC_PIN, GPIO_PIN_RESET)

#define SOFT_I2C_SIOD_H()               HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN, GPIO_PIN_SET)
#define SOFT_I2C_SIOD_L()               HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN, GPIO_PIN_RESET)

#define SOFT_I2C_SIOD_READ()            HAL_GPIO_ReadPin (SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN)
#define SOFT_I2C_SIOD_WRITE(bit)        HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN, bit)

#define SOFT_I2C_SPIN_DELAY             64

#define LEPTON_SPI                      (SPI3)
// SPI1/2/3 clock source is PLL2 (200MHz/8 == 12.5MHz) - Minimum (164*240*8*27 = 8,501,760Hz)
#define LEPTON_SPI_PRESCALER            (SPI_BAUDRATEPRESCALER_16)

#define LEPTON_SPI_IRQn                 (SPI3_IRQn)
#define LEPTON_SPI_IRQHandler           (SPI3_IRQHandler)

#define LEPTON_SPI_DMA_IRQn             (DMA1_Stream0_IRQn)
#define LEPTON_SPI_DMA_STREAM           (DMA1_Stream0)

#define LEPTON_SPI_DMA_REQUEST          (DMA_REQUEST_SPI3_RX)
#define LEPTON_SPI_DMA_IRQHandler       (DMA1_Stream0_IRQHandler)

#define LEPTON_SPI_RESET()              __HAL_RCC_SPI3_FORCE_RESET()
#define LEPTON_SPI_RELEASE()            __HAL_RCC_SPI3_RELEASE_RESET()

#define LEPTON_SPI_CLK_ENABLE()         __HAL_RCC_SPI3_CLK_ENABLE()
#define LEPTON_SPI_CLK_DISABLE()        __HAL_RCC_SPI3_CLK_DISABLE()

#define LEPTON_SPI_SCLK_AF              (GPIO_AF6_SPI3)
#define LEPTON_SPI_MISO_AF              (GPIO_AF6_SPI3)
#define LEPTON_SPI_MOSI_AF              (GPIO_AF7_SPI3)
#define LEPTON_SPI_SSEL_AF              (GPIO_AF6_SPI3)

#define LEPTON_SPI_SCLK_PIN             (GPIO_PIN_3)
#define LEPTON_SPI_MISO_PIN             (GPIO_PIN_4)
#define LEPTON_SPI_MOSI_PIN             (GPIO_PIN_5)
#define LEPTON_SPI_SSEL_PIN             (GPIO_PIN_15)

#define LEPTON_SPI_SCLK_PORT            (GPIOB)
#define LEPTON_SPI_MISO_PORT            (GPIOB)
#define LEPTON_SPI_MOSI_PORT            (GPIOB)
#define LEPTON_SPI_SSEL_PORT            (GPIOA)

#define LEPTON_RST_PIN                  (GPIO_PIN_5)
#define LEPTON_RST_PORT                 (GPIOD)

#define LEPTON_PWND_PIN                 (GPIO_PIN_4)
#define LEPTON_PWDN_PORT                (GPIOD)

#define LEPTON_RST_LOW()                HAL_GPIO_WritePin(LEPTON_RST_PORT, LEPTON_RST_PIN, GPIO_PIN_RESET)
#define LEPTON_RST_HIGH()               HAL_GPIO_WritePin(LEPTON_RST_PORT, LEPTON_RST_PIN, GPIO_PIN_SET)

#define LEPTON_PWDN_LOW()               HAL_GPIO_WritePin(LEPTON_PWDN_PORT, LEPTON_PWND_PIN, GPIO_PIN_RESET)
#define LEPTON_PWDN_HIGH()              HAL_GPIO_WritePin(LEPTON_PWDN_PORT, LEPTON_PWND_PIN, GPIO_PIN_SET)

#define LEPTON_CLK_PIN                  (GPIO_PIN_3)
#define LEPTON_CLK_PORT                 (GPIOA)
#define LEPTON_CLK_ALT                  (GPIO_AF4_TIM15)
#define LEPTON_CLK_FREQ                 (25000000)

// QSPI flash configuration for the bootloader.
#define QSPIF_SIZE_BITS                 (25)        // 2**25 == 32MBytes.
#define QSPIF_SR_WIP_MASK               (1 << 0)
#define QSPIF_SR_WEL_MASK               (1 << 1)
#define QSPIF_READ_QUADIO_DCYC          (6)

#define QSPIF_PAGE_SIZE                 (0x100)     // 256 bytes pages.
#define QSPIF_NUM_PAGES                 (0x20000)   // 131072 pages of 256 bytes

#define QSPIF_SECTOR_SIZE               (0x1000)    // 4K bytes sectors.
#define QSPIF_NUM_SECTORS               (0x2000)    // 8192 sectors of 4K bytes

#define QSPIF_BLOCK_SIZE                (0x10000)   // 64K bytes blocks.
#define QSPIF_NUM_BLOCKS                (0x200)     // 512 blocks of 64K bytes

#define QSPIF_CLK_PIN                   (GPIO_PIN_10)
#define QSPIF_CLK_PORT                  (GPIOF)
#define QSPIF_CLK_ALT                   (GPIO_AF9_QUADSPI)

#define QSPIF_CS_PIN                    (GPIO_PIN_6)
#define QSPIF_CS_PORT                   (GPIOG)
#define QSPIF_CS_ALT                    (GPIO_AF10_QUADSPI)

#define QSPIF_D0_PIN                    (GPIO_PIN_8)
#define QSPIF_D0_PORT                   (GPIOF)
#define QSPIF_D0_ALT                    (GPIO_AF10_QUADSPI)

#define QSPIF_D1_PIN                    (GPIO_PIN_9)
#define QSPIF_D1_PORT                   (GPIOF)
#define QSPIF_D1_ALT                    (GPIO_AF10_QUADSPI)

#define QSPIF_D2_PIN                    (GPIO_PIN_7)
#define QSPIF_D2_PORT                   (GPIOF)
#define QSPIF_D2_ALT                    (GPIO_AF9_QUADSPI)

#define QSPIF_D3_PIN                    (GPIO_PIN_6)
#define QSPIF_D3_PORT                   (GPIOF)
#define QSPIF_D3_ALT                    (GPIO_AF9_QUADSPI)

#define QSPIF_CLK_ENABLE()              __HAL_RCC_QSPI_CLK_ENABLE()
#define QSPIF_CLK_DISABLE()             __HAL_RCC_QSPI_CLK_DISABLE()
#define QSPIF_FORCE_RESET()             __HAL_RCC_QSPI_FORCE_RESET()
#define QSPIF_RELEASE_RESET()           __HAL_RCC_QSPI_RELEASE_RESET()

#define QSPIF_CLK_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPIF_CS_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOG_CLK_ENABLE()
#define QSPIF_D0_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPIF_D1_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPIF_D2_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPIF_D3_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOF_CLK_ENABLE()

// LCD Interface
#define OMV_LCD_CONTROLLER              (LTDC)
#define OMV_LCD_CLK_ENABLE()            __HAL_RCC_LTDC_CLK_ENABLE()
#define OMV_LCD_CLK_DISABLE()           __HAL_RCC_LTDC_CLK_DISABLE()
#define OMV_LCD_FORCE_RESET()           __HAL_RCC_LTDC_FORCE_RESET()
#define OMV_LCD_RELEASE_RESET()         __HAL_RCC_LTDC_RELEASE_RESET()

#define OMV_LCD_R0_PIN                  (GPIO_PIN_13)
#define OMV_LCD_R1_PIN                  (GPIO_PIN_2)
#define OMV_LCD_R2_PIN                  (GPIO_PIN_1)
#define OMV_LCD_R3_PIN                  (GPIO_PIN_2)
#define OMV_LCD_R4_PIN                  (GPIO_PIN_3)
#define OMV_LCD_R5_PIN                  (GPIO_PIN_4)
#define OMV_LCD_R6_PIN                  (GPIO_PIN_5)
#define OMV_LCD_R7_PIN                  (GPIO_PIN_0)

#define OMV_LCD_R0_PORT                 (GPIOG)
#define OMV_LCD_R1_PORT                 (GPIOA)
#define OMV_LCD_R2_PORT                 (GPIOJ)
#define OMV_LCD_R3_PORT                 (GPIOJ)
#define OMV_LCD_R4_PORT                 (GPIOJ)
#define OMV_LCD_R5_PORT                 (GPIOJ)
#define OMV_LCD_R6_PORT                 (GPIOJ)
#define OMV_LCD_R7_PORT                 (GPIOJ)

#define OMV_LCD_R0_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_R1_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_R2_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_R3_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_R4_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_R5_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_R6_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_R7_ALT                  (GPIO_AF9_LTDC)

#define OMV_LCD_G0_PIN                  (GPIO_PIN_7)
#define OMV_LCD_G1_PIN                  (GPIO_PIN_8)
#define OMV_LCD_G2_PIN                  (GPIO_PIN_9)
#define OMV_LCD_G3_PIN                  (GPIO_PIN_12)
#define OMV_LCD_G4_PIN                  (GPIO_PIN_11)
#define OMV_LCD_G5_PIN                  (GPIO_PIN_0)
#define OMV_LCD_G6_PIN                  (GPIO_PIN_11)
#define OMV_LCD_G7_PIN                  (GPIO_PIN_3)

#define OMV_LCD_G0_PORT                 (GPIOJ)
#define OMV_LCD_G1_PORT                 (GPIOJ)
#define OMV_LCD_G2_PORT                 (GPIOJ)
#define OMV_LCD_G3_PORT                 (GPIOJ)
#define OMV_LCD_G4_PORT                 (GPIOJ)
#define OMV_LCD_G5_PORT                 (GPIOK)
#define OMV_LCD_G6_PORT                 (GPIOI)
#define OMV_LCD_G7_PORT                 (GPIOD)

#define OMV_LCD_G0_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_G1_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_G2_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_G3_ALT                  (GPIO_AF9_LTDC)
#define OMV_LCD_G4_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_G5_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_G6_ALT                  (GPIO_AF9_LTDC)
#define OMV_LCD_G7_ALT                  (GPIO_AF14_LTDC)

#define OMV_LCD_B0_PIN                  (GPIO_PIN_14)
#define OMV_LCD_B1_PIN                  (GPIO_PIN_12)
#define OMV_LCD_B2_PIN                  (GPIO_PIN_6)
#define OMV_LCD_B3_PIN                  (GPIO_PIN_15)
#define OMV_LCD_B4_PIN                  (GPIO_PIN_3)
#define OMV_LCD_B5_PIN                  (GPIO_PIN_4)
#define OMV_LCD_B6_PIN                  (GPIO_PIN_5)
#define OMV_LCD_B7_PIN                  (GPIO_PIN_6)

#define OMV_LCD_B0_PORT                 (GPIOG)
#define OMV_LCD_B1_PORT                 (GPIOG)
#define OMV_LCD_B2_PORT                 (GPIOD)
#define OMV_LCD_B3_PORT                 (GPIOJ)
#define OMV_LCD_B4_PORT                 (GPIOK)
#define OMV_LCD_B5_PORT                 (GPIOK)
#define OMV_LCD_B6_PORT                 (GPIOK)
#define OMV_LCD_B7_PORT                 (GPIOK)

#define OMV_LCD_B0_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_B1_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_B2_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_B3_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_B4_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_B5_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_B6_ALT                  (GPIO_AF14_LTDC)
#define OMV_LCD_B7_ALT                  (GPIO_AF14_LTDC)

#define OMV_LCD_CLK_PIN                 (GPIO_PIN_14)
#define OMV_LCD_CLK_PORT                (GPIOI)
#define OMV_LCD_CLK_ALT                 (GPIO_AF14_LTDC)

#define OMV_LCD_DE_PIN                  (GPIO_PIN_7)
#define OMV_LCD_DE_PORT                 (GPIOK)
#define OMV_LCD_DE_ALT                  (GPIO_AF14_LTDC)

#define OMV_LCD_HSYNC_PIN               (GPIO_PIN_12)
#define OMV_LCD_HSYNC_PORT              (GPIOI)
#define OMV_LCD_HSYNC_ALT               (GPIO_AF14_LTDC)

#define OMV_LCD_VSYNC_PIN               (GPIO_PIN_13)
#define OMV_LCD_VSYNC_PORT              (GPIOI)
#define OMV_LCD_VSYNC_ALT               (GPIO_AF14_LTDC)

#define OMV_LCD_DISP_PIN                (GPIO_PIN_9)
#define OMV_LCD_DISP_PORT               (GPIOG)
#define OMV_LCD_DISP_ON()               HAL_GPIO_WritePin(OMV_LCD_DISP_PORT, OMV_LCD_DISP_PIN, GPIO_PIN_SET)
#define OMV_LCD_DISP_OFF()              HAL_GPIO_WritePin(OMV_LCD_DISP_PORT, OMV_LCD_DISP_PIN, GPIO_PIN_RESET)

#define OMV_LCD_BL_PIN                  (GPIO_PIN_0)
#define OMV_LCD_BL_PORT                 (GPIOB)
#define OMV_LCD_BL_ALT                  (GPIO_AF2_TIM3)
#define OMV_LCD_BL_FREQ                 (100000)
#define OMV_LCD_BL_ON()                 HAL_GPIO_WritePin(OMV_LCD_BL_PORT, OMV_LCD_BL_PIN, GPIO_PIN_SET)
#define OMV_LCD_BL_OFF()                HAL_GPIO_WritePin(OMV_LCD_BL_PORT, OMV_LCD_BL_PIN, GPIO_PIN_RESET)

#define OMV_LCD_BL_TIM                  (TIM3)
#define OMV_LCD_BL_TIM_CHANNEL          (TIM_CHANNEL_3)
#define OMV_LCD_BL_TIM_CLK_ENABLE()     __HAL_RCC_TIM3_CLK_ENABLE()
#define OMV_LCD_BL_TIM_CLK_DISABLE()    __HAL_RCC_TIM3_CLK_DISABLE()
#define OMV_LCD_BL_TIM_FORCE_RESET()    __HAL_RCC_TIM3_FORCE_RESET()
#define OMV_LCD_BL_TIM_RELEASE_RESET()  __HAL_RCC_TIM3_RELEASE_RESET()
#define OMV_LCD_BL_TIM_PCLK_FREQ()      HAL_RCC_GetPCLK1Freq()

// SPI LCD Interface
#define OMV_SPI_LCD_CONTROLLER              (&spi_obj[1])
#define OMV_SPI_LCD_CONTROLLER_INSTANCE     (SPI2)

#define OMV_SPI_LCD_MOSI_PIN                (GPIO_PIN_15)
#define OMV_SPI_LCD_MOSI_PORT               (GPIOB)
#define OMV_SPI_LCD_MOSI_ALT                (GPIO_AF5_SPI2)

#define OMV_SPI_LCD_SCLK_PIN                (GPIO_PIN_13)
#define OMV_SPI_LCD_SCLK_PORT               (GPIOB)
#define OMV_SPI_LCD_SCLK_ALT                (GPIO_AF5_SPI2)

#define OMV_SPI_LCD_RST_PIN                 (GPIO_PIN_12)
#define OMV_SPI_LCD_RST_PORT                (GPIOD)
#define OMV_SPI_LCD_RST_OFF()               HAL_GPIO_WritePin(OMV_SPI_LCD_RST_PORT, OMV_SPI_LCD_RST_PIN, GPIO_PIN_SET)
#define OMV_SPI_LCD_RST_ON()                HAL_GPIO_WritePin(OMV_SPI_LCD_RST_PORT, OMV_SPI_LCD_RST_PIN, GPIO_PIN_RESET)

#define OMV_SPI_LCD_RS_PIN                  (GPIO_PIN_13)
#define OMV_SPI_LCD_RS_PORT                 (GPIOD)
#define OMV_SPI_LCD_RS_OFF()                HAL_GPIO_WritePin(OMV_SPI_LCD_RS_PORT, OMV_SPI_LCD_RS_PIN, GPIO_PIN_SET)
#define OMV_SPI_LCD_RS_ON()                 HAL_GPIO_WritePin(OMV_SPI_LCD_RS_PORT, OMV_SPI_LCD_RS_PIN, GPIO_PIN_RESET)

#define OMV_SPI_LCD_CS_PIN                  (GPIO_PIN_12)
#define OMV_SPI_LCD_CS_PORT                 (GPIOB)
#define OMV_SPI_LCD_CS_HIGH()               HAL_GPIO_WritePin(OMV_SPI_LCD_CS_PORT, OMV_SPI_LCD_CS_PIN, GPIO_PIN_SET)
#define OMV_SPI_LCD_CS_LOW()                HAL_GPIO_WritePin(OMV_SPI_LCD_CS_PORT, OMV_SPI_LCD_CS_PIN, GPIO_PIN_RESET)

#define OMV_SPI_LCD_BL_PIN                  (GPIO_PIN_5)
#define OMV_SPI_LCD_BL_PORT                 (GPIOA)
#define OMV_SPI_LCD_BL_ON()                 HAL_GPIO_WritePin(OMV_SPI_LCD_BL_PORT, OMV_SPI_LCD_BL_PIN, GPIO_PIN_SET)
#define OMV_SPI_LCD_BL_OFF()                HAL_GPIO_WritePin(OMV_SPI_LCD_BL_PORT, OMV_SPI_LCD_BL_PIN, GPIO_PIN_RESET)

#define OMV_SPI_LCD_BL_DAC                  (DAC1)
#define OMV_SPI_LCD_BL_DAC_CHANNEL          (DAC_CHANNEL_2)
#define OMV_SPI_LCD_BL_DAC_CLK_ENABLE()     __HAL_RCC_DAC12_CLK_ENABLE()
#define OMV_SPI_LCD_BL_DAC_CLK_DISABLE()    __HAL_RCC_DAC12_CLK_DISABLE()
#define OMV_SPI_LCD_BL_DAC_FORCE_RESET()    __HAL_RCC_DAC12_FORCE_RESET()
#define OMV_SPI_LCD_BL_DAC_RELEASE_RESET()  __HAL_RCC_DAC12_RELEASE_RESET()

// Touch Screen I/O
#define OMV_TOUCH_RESET_PIN             (GPIO_PIN_9)
#define OMV_TOUCH_RESET_PORT            (GPIOG)

#define OMV_TOUCH_SCL_PIN               (GPIO_PIN_13)
#define OMV_TOUCH_SCL_PORT              (GPIOJ)

#define OMV_TOUCH_SDA_PIN               (GPIO_PIN_14)
#define OMV_TOUCH_SDA_PORT              (GPIOJ)

#define OMV_TOUCH_INT                   (pin_J6)

// DVI TFP410AP I/O
#define OMV_DVI_RESET_PIN               (GPIO_PIN_11)
#define OMV_DVI_RESET_PORT              (GPIOD)

#define OMV_DVI_SCL_PIN                 (GPIO_PIN_1)
#define OMV_DVI_SCL_PORT                (GPIOB)

#define OMV_DVI_SDA_PIN                 (GPIO_PIN_2)
#define OMV_DVI_SDA_PORT                (GPIOB)

#define OMV_DVI_INT                     (pin_I8)

// Enable additional GPIO banks for DRAM...
#define OMV_ENABLE_GPIO_BANK_F
#define OMV_ENABLE_GPIO_BANK_G
#define OMV_ENABLE_GPIO_BANK_H
#define OMV_ENABLE_GPIO_BANK_I

// Enable additional GPIO banks for LCD...
#define OMV_ENABLE_GPIO_BANK_J
#define OMV_ENABLE_GPIO_BANK_K

#endif //__OMV_BOARDCONFIG_H__

/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Board configuration and pin definitions.
 */
#ifndef __OMV_BOARDCONFIG_H__
#define __OMV_BOARDCONFIG_H__

// Architecture info
#define OMV_ARCH_STR                        "GIGA H7 8192 SDRAM"    // 33 chars max
#define OMV_BOARD_TYPE                      "H7"
#define OMV_UNIQUE_ID_ADDR                  0x1FF1E800    // Unique ID address.
#define OMV_UNIQUE_ID_SIZE                  3       // Unique ID size in words.
#define OMV_UNIQUE_ID_OFFSET                4       // Bytes offset for multi-word UIDs.

#define OMV_XCLK_MCO                        (0U)
#define OMV_XCLK_TIM                        (1U)
#define OMV_XCLK_OSC                        (2U)

// Sensor external clock source.
#define OMV_XCLK_SOURCE                     (OMV_XCLK_TIM)

// Sensor external clock timer frequency.
#define OMV_XCLK_FREQUENCY                  (12000000)

// Enable hardware JPEG
#define OMV_HARDWARE_JPEG                   (1)

// MDMA configuration
#define OMV_MDMA_CHANNEL_DCMI_0             (0)
#define OMV_MDMA_CHANNEL_DCMI_1             (1)
#define OMV_MDMA_CHANNEL_JPEG_IN            (7) // in has a lower pri than out
#define OMV_MDMA_CHANNEL_JPEG_OUT           (6) // out has a higher pri than in

// OV5640 sensor settings
#define OMV_OV5640_XCLK_FREQ                  (24000000)
#define OMV_OV5640_PLL_CTRL2                  (0x64)
#define OMV_OV5640_PLL_CTRL3                  (0x13)
#define OMV_OV5640_REV_Y_CHECK                (1)
#define OMV_OV5640_REV_Y_FREQ                 (25000000)
#define OMV_OV5640_REV_Y_CTRL2                (0x54)
#define OMV_OV5640_REV_Y_CTRL3                (0x13)

// Enable additional GPIO banks.
#define OMV_ENABLE_GPIO_BANK_F              (1)
#define OMV_ENABLE_GPIO_BANK_G              (1)
#define OMV_ENABLE_GPIO_BANK_H              (1)
#define OMV_ENABLE_GPIO_BANK_I              (1)
#define OMV_ENABLE_GPIO_BANK_J              (1)
#define OMV_ENABLE_GPIO_BANK_K              (1)

// Enable sensor drivers
#define OMV_ENABLE_OV2640                   (0)
#define OMV_ENABLE_OV5640                   (1)
#define OMV_ENABLE_OV7670                   (1)
#define OMV_ENABLE_OV7690                   (0)
#define OMV_ENABLE_OV7725                   (0)
#define OMV_ENABLE_OV9650                   (0)
#define OMV_ENABLE_MT9M114                  (0)
#define OMV_ENABLE_MT9V0XX                  (0)
#define OMV_ENABLE_LEPTON                   (0)
#define OMV_ENABLE_HM01B0                   (0)
#define OMV_ENABLE_PAJ6100                  (0)
#define OMV_ENABLE_FROGEYE2020              (0)
#define OMV_ENABLE_FIR_MLX90621             (1)
#define OMV_ENABLE_FIR_MLX90640             (1)
#define OMV_ENABLE_FIR_MLX90641             (1)
#define OMV_ENABLE_FIR_AMG8833              (1)
#define OMV_ENABLE_FIR_LEPTON               (0)

// Set which OV767x sensor is used
#define OMV_OV7670_VERSION                  (75)

// OV7670 clock divider
#define OMV_OV7670_CLKRC                    (0)

// Enable sensor features
#define OMV_ENABLE_OV5640_AF                (1)

// Enable WiFi debug
#define OMV_ENABLE_WIFIDBG                  (0)

// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH                 (320 * 240 * 2)

// Low and high JPEG QS.
#define JPEG_QUALITY_LOW                    50
#define JPEG_QUALITY_HIGH                   90

// FB Heap Block Size
#define OMV_UMM_BLOCK_SIZE                  256

// USB IRQn.
#define OMV_USB_IRQN                        (OTG_FS_IRQn)
#define OMV_USB_ULPI                        (0)

// PLL1 480MHz/48MHz SDMMC and FDCAN
// USB and RNG are clocked from the HSI48
#define OMV_OSC_PLL1M                       (4)
#define OMV_OSC_PLL1N                       (240)
#define OMV_OSC_PLL1P                       (2)
#define OMV_OSC_PLL1Q                       (20)
#define OMV_OSC_PLL1R                       (2)
#define OMV_OSC_PLL1VCI                     (RCC_PLL1VCIRANGE_2)
#define OMV_OSC_PLL1VCO                     (RCC_PLL1VCOWIDE)
#define OMV_OSC_PLL1FRAC                    (0)

// PLL2 200MHz for FMC and QSPI.
#define OMV_OSC_PLL2M                       (4)
#define OMV_OSC_PLL2N                       (100)
#define OMV_OSC_PLL2P                       (2)
#define OMV_OSC_PLL2Q                       (2)
#define OMV_OSC_PLL2R                       (2)
#define OMV_OSC_PLL2VCI                     (RCC_PLL2VCIRANGE_2)
#define OMV_OSC_PLL2VCO                     (RCC_PLL2VCOWIDE)
#define OMV_OSC_PLL2FRAC                    (0)

// PLL3 160MHz for ADC and SPI123
#define OMV_OSC_PLL3M                       (4)
#define OMV_OSC_PLL3N                       (80)
#define OMV_OSC_PLL3P                       (2)
#define OMV_OSC_PLL3Q                       (2)
#define OMV_OSC_PLL3R                       (2)
#define OMV_OSC_PLL3VCI                     (RCC_PLL3VCIRANGE_2)
#define OMV_OSC_PLL3VCO                     (RCC_PLL3VCOWIDE)
#define OMV_OSC_PLL3FRAC                    (0)

// Clock Sources
#define OMV_OSC_PLL_CLKSOURCE               RCC_PLLSOURCE_HSE
#define OMV_OSC_USB_CLKSOURCE               RCC_USBCLKSOURCE_HSI48
#define OMV_OSC_RNG_CLKSOURCE               RCC_RNGCLKSOURCE_HSI48
#define OMV_OSC_ADC_CLKSOURCE               RCC_ADCCLKSOURCE_PLL3
#define OMV_OSC_SPI123_CLKSOURCE            RCC_SPI123CLKSOURCE_PLL3
#define OMV_OSC_DFSDM1_CLKSOURCE            RCC_DFSDM1CLKSOURCE_D2PCLK1

// HSE/HSI/CSI State
#define OMV_OSC_HSE_STATE                   (RCC_HSE_ON)
#define OMV_OSC_HSI48_STATE                 (RCC_HSI48_ON)

// Flash Latency
#define OMV_FLASH_LATENCY                   (FLASH_LATENCY_2)

// Power supply configuration
#define OMV_PWR_SUPPLY                      (PWR_LDO_SUPPLY)

// Linker script constants (see the linker script template stm32fxxx.ld.S).
// Note: fb_alloc is a stack-based, dynamically allocated memory on FB.
// The maximum available fb_alloc memory = FB_ALLOC_SIZE + FB_SIZE - (w*h*bpp).
#define OMV_MAIN_MEMORY                     SRAM1    // data, bss and heap
#define OMV_STACK_MEMORY                    DTCM    // stack memory
#define OMV_DMA_MEMORY                      SRAM3    // DMA buffers memory.
#define OMV_FB_MEMORY                       DRAM    // Framebuffer, fb_alloc
#define OMV_JPEG_MEMORY                     DRAM    // JPEG buffer memory buffer.
#define OMV_JPEG_MEMORY_OFFSET              (7M)    // JPEG buffer is placed after FB/fballoc memory.
#define OMV_VOSPI_MEMORY                    SRAM4    // VoSPI buffer memory.
#define OMV_VOSPI_MEMORY_OFFSET             (20K) // SRAM4 reserves 16K for CM4 + 4K D3 DMA buffers.
#define OMV_FB_OVERLAY_MEMORY               AXI_SRAM    // Fast fb_alloc memory.

#define OMV_FB_SIZE                         (4M)    // FB memory: header + VGA/GS image
#define OMV_FB_ALLOC_SIZE                   (3M)    // minimum fb alloc size
#define OMV_FB_OVERLAY_SIZE                 (480 * 1024)    // Fast fb_alloc memory size.
#define OMV_STACK_SIZE                      (64K)
#define OMV_HEAP_SIZE                       (199K)
#define OMV_SDRAM_SIZE                      (8 * 1024 * 1024)    // This needs to be here for UVC firmware.

#define OMV_LINE_BUF_SIZE                   (11 * 1024)    // Image line buffer round(2592 * 2BPP * 2 buffers).
#define OMV_MSC_BUF_SIZE                    (2K)    // USB MSC bot data
#define OMV_VFS_BUF_SIZE                    (1K)    // VFS struct + FATFS file buffer (624 bytes)
#define OMV_JPEG_BUF_SIZE                   (1024 * 1024)    // IDE JPEG buffer (header + data).

// Memory map.
#define OMV_FLASH_ORIGIN                    0x08000000
#define OMV_FLASH_LENGTH                    2048K
#define OMV_DTCM_ORIGIN                     0x20000000    // Note accessible by CPU and MDMA only.
#define OMV_DTCM_LENGTH                     128K
#define OMV_SRAM1_ORIGIN                    0x30000000
#define OMV_SRAM1_LENGTH                    256K
#define OMV_SRAM3_ORIGIN                    0x30040000
#define OMV_SRAM3_LENGTH                    32K
#define OMV_SRAM4_ORIGIN                    0x38000000
#define OMV_SRAM4_LENGTH                    64K
#define OMV_AXI_SRAM_ORIGIN                 0x24000000
#define OMV_AXI_SRAM_LENGTH                 512K
#define OMV_DRAM_ORIGIN                     0xC0000000
#define OMV_DRAM_LENGTH                     8M
#define OMV_CM4_RAM_ORIGIN                  0x38000000 // Cortex-M4 memory @SRAM4.
#define OMV_CM4_RAM_LENGTH                  16K

// Flash configuration.
#define OMV_FLASH_FFS_ORIGIN                0x08020000
#define OMV_FLASH_FFS_LENGTH                128K
#define OMV_FLASH_TXT_ORIGIN                0x08040000
#define OMV_FLASH_TXT_LENGTH                1792K
#define OMV_FLASH_EXT_ORIGIN                0x90000000
#define OMV_FLASH_EXT_LENGTH                16M
#define OMV_CM4_FLASH_ORIGIN                0x08020000
#define OMV_CM4_FLASH_LENGTH                128K

// Domain 1 DMA buffers region.
#define OMV_DMA_MEMORY_D1                   AXI_SRAM
#define OMV_DMA_MEMORY_D1_SIZE              (8 * 1024)    // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D1_BASE              (OMV_AXI_SRAM_ORIGIN + OMV_FB_OVERLAY_SIZE)
#define OMV_DMA_REGION_D1_SIZE              MPU_REGION_SIZE_32KB

// Domain 2 DMA buffers region.
#define OMV_DMA_MEMORY_D2                   SRAM3
#define OMV_DMA_MEMORY_D2_SIZE              (4 * 1024)    // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D2_BASE              (OMV_SRAM3_ORIGIN + (0 * 1024))
#define OMV_DMA_REGION_D2_SIZE              MPU_REGION_SIZE_32KB

// Domain 3 DMA buffers region.
#define OMV_DMA_MEMORY_D3                   SRAM4
#define OMV_DMA_MEMORY_D3_SIZE              (4 * 1024)    // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D3_BASE              (OMV_SRAM4_ORIGIN + (16 * 1024))
#define OMV_DMA_REGION_D3_SIZE              MPU_REGION_SIZE_4KB

// AXI QoS - Low-High (0:15) - default 0
#define OMV_AXI_QOS_MDMA_R_PRI              15
#define OMV_AXI_QOS_MDMA_W_PRI              15
#define OMV_AXI_QOS_LTDC_R_PRI              14
//#define OMV_AXI_QOS_DMA2D_R_PRI             14
//#define OMV_AXI_QOS_DMA2D_W_PRI             14

// Image sensor I2C
#define ISC_I2C_ID                          (4)
#define ISC_I2C_SPEED                       (OMV_I2C_SPEED_STANDARD)

// FIR I2C
#define FIR_I2C_ID                          (1)
#define FIR_I2C_SPEED                       (OMV_I2C_SPEED_STANDARD)

// Soft I2C bus.
///#define SOFT_I2C_SIOC_PIN                   (&omv_pin_B10_GPIO)
///#define SOFT_I2C_SIOD_PIN                   (&omv_pin_B11_GPIO)
///#define SOFT_I2C_SPIN_DELAY                 64

// DCMI timer.
#define DCMI_TIM                            (TIM1)
#define DCMI_TIM_PIN                        (&omv_pin_J9_TIM1)
#define DCMI_TIM_CHANNEL                    (TIM_CHANNEL_3)
#define DCMI_TIM_CLK_ENABLE()               __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()              __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()                HAL_RCC_GetPCLK2Freq()

// DCMI pins.
//#define DCMI_RESET_PIN                      (&omv_pin_A1_GPIO)
#define DCMI_POWER_PIN                      (&omv_pin_D4_GPIO)

#define DCMI_D0_PIN                         (&omv_pin_H9_DCMI)
#define DCMI_D1_PIN                         (&omv_pin_H10_DCMI)
#define DCMI_D2_PIN                         (&omv_pin_H11_DCMI)
#define DCMI_D3_PIN                         (&omv_pin_G11_DCMI)
#define DCMI_D4_PIN                         (&omv_pin_H14_DCMI)
#define DCMI_D5_PIN                         (&omv_pin_I4_DCMI)
#define DCMI_D6_PIN                         (&omv_pin_I6_DCMI)
#define DCMI_D7_PIN                         (&omv_pin_I7_DCMI)

#define DCMI_HSYNC_PIN                      (&omv_pin_H8_DCMI)
#define DCMI_VSYNC_PIN                      (&omv_pin_I5_DCMI)
#define DCMI_PXCLK_PIN                      (&omv_pin_A6_DCMI)

// Physical I2C buses.
// I2C bus 1
#define I2C1_ID                             (1)
#define I2C1_SCL_PIN                        (&omv_pin_B8_I2C1)
#define I2C1_SDA_PIN                        (&omv_pin_B9_I2C1)

// I2C bus 4
#define I2C4_ID                             (4)
#define I2C4_SCL_PIN                        (&omv_pin_B6_I2C4)
#define I2C4_SDA_PIN                        (&omv_pin_H12_I2C4)

// DFSDM1
#define AUDIO_DFSDM                         (DFSDM1_Channel0)
#define AUDIO_DFSDM_CHANNEL                 (DFSDM_CHANNEL_0)
// DFSDM output clock is derived from the Aclk (set in SAI1SEL[2:0])
// for SAI1 and DFSDM1, which is clocked from PLL1Q by default (48MHz).
#define AUDIO_DFSDM_FREQMHZ                 (48)
#define AUDIO_MAX_CHANNELS                  (1) // Maximum number of channels.

#define AUDIO_DFSDM_CK_PIN                  (&omv_pin_D3_DFSDM1)
#define AUDIO_DFSDM_D1_PIN                  (&omv_pin_C1_DFSDM1)

#define AUDIO_DFSDM_FLT0                    DFSDM1_Filter0
#define AUDIO_DFSDM_FLT0_IRQ                DFSDM1_FLT0_IRQn
#define AUDIO_DFSDM_FLT0_IRQHandler         DFSDM1_FLT0_IRQHandler
#define AUDIO_DFSDM_FLT0_DMA_STREAM         DMA1_Stream1
#define AUDIO_DFSDM_FLT0_DMA_REQUEST        DMA_REQUEST_DFSDM1_FLT0
#define AUDIO_DFSDM_FLT0_DMA_IRQ            DMA1_Stream1_IRQn
#define AUDIO_DFSDM_FLT0_DMA_IRQHandler     DMA1_Stream1_IRQHandler

#define AUDIO_DFSDM_CLK_ENABLE()            __HAL_RCC_DFSDM1_CLK_ENABLE()
#define AUDIO_DFSDM_CLK_DISABLE()           __HAL_RCC_DFSDM1_CLK_DISABLE()
#define AUDIO_DFSDM_DMA_CLK_ENABLE()        __HAL_RCC_DMA1_CLK_ENABLE()

// LCD Interface
#define OMV_RGB_DISPLAY_CONTROLLER          (LTDC)
#define OMV_RGB_DISPLAY_BL_PIN              (&omv_pin_B12_GPIO)
#define OMV_RGB_DISPLAY_CLK_ENABLE()        __HAL_RCC_LTDC_CLK_ENABLE()
#define OMV_RGB_DISPLAY_CLK_DISABLE()       __HAL_RCC_LTDC_CLK_DISABLE()
#define OMV_RGB_DISPLAY_FORCE_RESET()       __HAL_RCC_LTDC_FORCE_RESET()
#define OMV_RGB_DISPLAY_RELEASE_RESET()     __HAL_RCC_LTDC_RELEASE_RESET()

// DSI Interface
#define OMV_DISPLAY_ST7701_ENABLE           (1)
#define OMV_DSI_DISPLAY_CONTROLLER          (DSI)
#define OMV_DSI_DISPLAY_CONTROLLER_INIT     st7701_init
#define OMV_DSI_DISPLAY_TE_ENABLE           (1)
#define OMV_DSI_DISPLAY_BL_PIN              (&omv_pin_B12_GPIO)
#define OMV_DSI_DISPLAY_CLK_ENABLE()        __HAL_RCC_DSI_CLK_ENABLE()
#define OMV_DSI_DISPLAY_CLK_DISABLE()       __HAL_RCC_DSI_CLK_DISABLE()
#define OMV_DSI_DISPLAY_FORCE_RESET()       __HAL_RCC_DSI_FORCE_RESET()
#define OMV_DSI_DISPLAY_RELEASE_RESET()     __HAL_RCC_DSI_RELEASE_RESET()

#endif //__OMV_BOARDCONFIG_H__

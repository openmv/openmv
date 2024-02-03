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
#define OMV_ARCH_STR                        "PORTENTA H7 8192 SDRAM"    // 33 chars max
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

// Enable fast line transfer with DMA.
#define OMA_ENABLE_DMA_MEMCPY               (1)

// MDMA configuration
#define OMV_MDMA_CHANNEL_DCMI_0             (0)
#define OMV_MDMA_CHANNEL_DCMI_1             (1)
#define OMV_MDMA_CHANNEL_JPEG_IN            (7) // in has a lower pri than out
#define OMV_MDMA_CHANNEL_JPEG_OUT           (6) // out has a higher pri than in

// Enable additional GPIO banks.
#define OMV_ENABLE_GPIO_BANK_F              (1)
#define OMV_ENABLE_GPIO_BANK_G              (1)
#define OMV_ENABLE_GPIO_BANK_H              (1)
#define OMV_ENABLE_GPIO_BANK_I              (1)
#define OMV_ENABLE_GPIO_BANK_J              (1)
#define OMV_ENABLE_GPIO_BANK_K              (1)

// Enable sensor drivers
#define OMV_ENABLE_OV2640                   (0)
#define OMV_ENABLE_OV5640                   (0)
#define OMV_ENABLE_OV7690                   (0)
#define OMV_ENABLE_OV7725                   (0)
#define OMV_ENABLE_OV9650                   (0)
#define OMV_ENABLE_MT9M114                  (0)
#define OMV_ENABLE_MT9V0XX                  (0)
#define OMV_ENABLE_LEPTON                   (0)
#define OMV_ENABLE_HM01B0                   (1)
#define OMV_ENABLE_HM0360                   (1)
#define OMV_ENABLE_GC2145                   (0)

// OV7725 sensor settings
#define OMV_OV7725_PLL_CONFIG               (0x41)    // x4
#define OMV_OV7725_BANDING                  (0x7F)

// MT9V0XX sensor settings
#define MT9V0XX_XCLK_FREQ                   (25000000)

// OV5640 sensor settings
#define OMV_ENABLE_OV5640_AF                (1)
#define OMV_OV5640_XCLK_FREQ                (12500000)
#define OMV_OV5640_PLL_CTRL2                (0x7E)
#define OMV_OV5640_PLL_CTRL3                (0x13)
#define OMV_OV5640_REV_Y_CHECK              (0)
#define OMV_OV5640_REV_Y_FREQ               (12500000)
#define OMV_OV5640_REV_Y_CTRL2              (0x7E)
#define OMV_OV5640_REV_Y_CTRL3              (0x13)

// FIR Module
#define OMV_ENABLE_FIR_MLX90621             (1)
#define OMV_ENABLE_FIR_MLX90640             (1)
#define OMV_ENABLE_FIR_MLX90641             (1)
#define OMV_ENABLE_FIR_AMG8833              (1)
#define OMV_ENABLE_FIR_LEPTON               (1)

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
#define OMV_USB_IRQN                        (OTG_HS_IRQn)
#define OMV_USB_ULPI                        (1)
#define OMV_USB_ULPI_STP_PIN                (&omv_pin_C0_OTG_HS)
#define OMV_USB_ULPI_DIR_PIN                (&omv_pin_I11_OTG_HS)

// Defined for cpu frequency scaling to override the revid.
#define OMV_MAX_CPU_FREQ                    (400)

// PLL1 400MHz/40MHz for SDMMC and FDCAN
// USB and RNG are clocked from the HSI48
#define OMV_OSC_PLL1M                       (5)
#define OMV_OSC_PLL1N                       (160)
#define OMV_OSC_PLL1P                       (2)
#define OMV_OSC_PLL1Q                       (16)
#define OMV_OSC_PLL1R                       (2)
#define OMV_OSC_PLL1VCI                     (RCC_PLL1VCIRANGE_2)
#define OMV_OSC_PLL1VCO                     (RCC_PLL1VCOWIDE)
#define OMV_OSC_PLL1FRAC                    (0)

// PLL2 200MHz for FMC and QSPI.
#define OMV_OSC_PLL2M                       (5)
#define OMV_OSC_PLL2N                       (80)
#define OMV_OSC_PLL2P                       (2)
#define OMV_OSC_PLL2Q                       (2)
#define OMV_OSC_PLL2R                       (2)
#define OMV_OSC_PLL2VCI                     (RCC_PLL2VCIRANGE_2)
#define OMV_OSC_PLL2VCO                     (RCC_PLL2VCOWIDE)
#define OMV_OSC_PLL2FRAC                    (0)

// PLL3 160MHz for ADC and SPI123
#define OMV_OSC_PLL3M                       (5)
#define OMV_OSC_PLL3N                       (160)
#define OMV_OSC_PLL3P                       (2)
#define OMV_OSC_PLL3Q                       (5)
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

// HSE/HSI/CSI State
#define OMV_OSC_HSE_STATE                   (RCC_HSE_BYPASS)
#define OMV_OSC_HSI48_STATE                 (RCC_HSI48_ON)

// Flash Latency
#define OMV_FLASH_LATENCY                   (FLASH_LATENCY_2)

// Power supply configuration
#define OMV_PWR_SUPPLY                      (PWR_SMPS_1V8_SUPPLIES_LDO)

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
#define OMV_FB_OVERLAY_MEMORY               AXI_SRAM    // Fast fb_alloc memory.

#define OMV_FB_SIZE                         (4M)    // FB memory: header + VGA/GS image
#define OMV_FB_ALLOC_SIZE                   (3M)    // minimum fb alloc size
#define OMV_FB_OVERLAY_SIZE                 (480 * 1024)    // Fast fb_alloc memory size.
#define OMV_STACK_SIZE                      (64K)
#define OMV_HEAP_SIZE                       (160K)
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
#define OMV_SRAM1_LENGTH                    256K    // SRAM1 + SRAM2
#define OMV_SRAM3_ORIGIN                    0x30040000    // Second half of SRAM3 reserved for M4.
#define OMV_SRAM3_LENGTH                    16K
#define OMV_SRAM4_ORIGIN                    0x38000000
#define OMV_SRAM4_LENGTH                    64K
#define OMV_AXI_SRAM_ORIGIN                 0x24000000
#define OMV_AXI_SRAM_LENGTH                 512K
#define OMV_DRAM_ORIGIN                     0xC0000000
#define OMV_DRAM_LENGTH                     8M
#define OMV_CM4_RAM_ORIGIN                  0x30044000    // Cortex-M4 memory.
#define OMV_CM4_RAM_LENGTH                  16K
#define OMV_CM4_FLASH_ORIGIN                0x08020000
#define OMV_CM4_FLASH_LENGTH                128K

// Flash configuration.
#define OMV_FLASH_FFS_ORIGIN                0x08020000
#define OMV_FLASH_FFS_LENGTH                128K
#define OMV_FLASH_TXT_ORIGIN                0x08040000
#define OMV_FLASH_TXT_LENGTH                1792K
#define OMV_FLASH_EXT_ORIGIN                0x90000000
#define OMV_FLASH_EXT_LENGTH                16M

// Domain 1 DMA buffers region.
#define OMV_DMA_MEMORY_D1                   AXI_SRAM
#define OMV_DMA_MEMORY_D1_SIZE              (16 * 1024)    // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D1_BASE              (OMV_AXI_SRAM_ORIGIN + OMV_FB_OVERLAY_SIZE)
#define OMV_DMA_REGION_D1_SIZE              MPU_REGION_SIZE_32KB

// Domain 2 DMA buffers region.
#define OMV_DMA_MEMORY_D2                   SRAM3
#define OMV_DMA_MEMORY_D2_SIZE              (1 * 1024)    // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D2_BASE              (OMV_SRAM3_ORIGIN + (0 * 1024))
#define OMV_DMA_REGION_D2_SIZE              MPU_REGION_SIZE_16KB

// Domain 3 DMA buffers region.
#define OMV_DMA_MEMORY_D3                   SRAM4
#define OMV_DMA_MEMORY_D3_SIZE              (32 * 1024)    // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D3_BASE              (OMV_SRAM4_ORIGIN + (0 * 1024))
#define OMV_DMA_REGION_D3_SIZE              MPU_REGION_SIZE_64KB

// AXI QoS - Low-High (0:15) - default 0
#define OMV_AXI_QOS_MDMA_R_PRI              14    // Max pri to move data.
#define OMV_AXI_QOS_MDMA_W_PRI              15    // Max pri to move data.
#define OMV_AXI_QOS_LTDC_R_PRI              15    // Max pri to read out the frame buffer.

// Image sensor I2C
#define ISC_I2C_ID                          (3)
#define ISC_I2C_SPEED                       (OMV_I2C_SPEED_STANDARD)

// Alternate I2C bus for the Portenta breakout
#define ISC_I2C_ALT_ID                      (4)
#define ISC_I2C_ALT_SPEED                   (OMV_I2C_SPEED_STANDARD)

// Thermal image sensor I2C bus
#define FIR_I2C_ID                          (3)
#define FIR_I2C_SPEED                       (OMV_I2C_SPEED_STANDARD)

// Soft I2C bus
//#define SOFT_I2C_SIOC_PIN                 (&omv_pin_PH7_GPIO)
//#define SOFT_I2C_SIOD_PIN                 (&omv_pin_PH8_GPIO)
//#define SOFT_I2C_SPIN_DELAY               64

// GPIO.0 is connected to the sensor module reset pin on the Portenta
// breakout board and to the LDO's LDO_ENABLE pin on the Himax shield.
// The sensor probing process will detect the right reset or powerdown
// polarity, so it should be fine to enable it for both boards.
#define DCMI_RESET_PIN                      (&omv_pin_C13_GPIO)

// GPIO.1 is connected to the sensor module frame sync pin (OUTPUT) on
// the Portenta breakout board and to the INT pin (OUTPUT) on the Himax
// shield, so it can't be enabled for the two boards at the same time.
//#define DCMI_FSYNC_PIN                    (&omv_pin_C15_GPIO)

// GPIO.3 is connected to the powerdown pin on the Portenta breakout board,
// and to the STROBE pin on the Himax shield, however it's not actually
// used on the Himax shield and can be safely enable for the two boards.
#define DCMI_POWER_PIN                      (&omv_pin_D5_GPIO)

/* DCMI */
#define DCMI_TIM                            (TIM1)
#define DCMI_TIM_PIN                        (&omv_pin_K1_TIM1)
// Enable TIM1-CH1 on PA8 too for Portenta breakout.
#define DCMI_TIM_EXT_PIN                    (&omv_pin_A8_TIM1)
#define DCMI_TIM_CHANNEL                    (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()               __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()              __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()                HAL_RCC_GetPCLK2Freq()

#define DCMI_D0_PIN                         (&omv_pin_H9_DCMI)
#define DCMI_D1_PIN                         (&omv_pin_H10_DCMI)
#define DCMI_D2_PIN                         (&omv_pin_H11_DCMI)
#define DCMI_D3_PIN                         (&omv_pin_H12_DCMI)
#define DCMI_D4_PIN                         (&omv_pin_H14_DCMI)
#define DCMI_D5_PIN                         (&omv_pin_I4_DCMI)
#define DCMI_D6_PIN                         (&omv_pin_I6_DCMI)
#define DCMI_D7_PIN                         (&omv_pin_I7_DCMI)

#define DCMI_HSYNC_PIN                      (&omv_pin_A4_DCMI)
#define DCMI_VSYNC_PIN                      (&omv_pin_I5_DCMI)
#define DCMI_PXCLK_PIN                      (&omv_pin_A6_DCMI)

// Physical I2C buses.

// I2C bus 3
#define I2C3_ID                             (3)
#define I2C3_SCL_PIN                        (&omv_pin_H7_I2C3)
#define I2C3_SDA_PIN                        (&omv_pin_H8_I2C3)

// I2C bus 4
#define I2C4_ID                             (4)
#define I2C4_SCL_PIN                        (&omv_pin_H11_I2C4)
#define I2C4_SDA_PIN                        (&omv_pin_H12_I2C4)

// Physical SPI buses.

// SPI bus 2
#define SPI2_ID                             (2)
#define SPI2_SCLK_PIN                       (&omv_pin_I1_SPI2)
#define SPI2_MISO_PIN                       (&omv_pin_C2_SPI2)
#define SPI2_MOSI_PIN                       (&omv_pin_C3_SPI2)
#define SPI2_SSEL_PIN                       (&omv_pin_I0_SPI2)
#define SPI2_DMA_TX_CHANNEL                 (DMA1_Stream4)
#define SPI2_DMA_RX_CHANNEL                 (DMA1_Stream3)

// SAI4
#define AUDIO_SAI                           (SAI4_Block_A)
// SCKx frequency = SAI_KER_CK / MCKDIV / 2
#define AUDIO_SAI_MCKDIV                    (12)
#define AUDIO_SAI_FREQKHZ                   (2048U) // 2048KHz
#define AUDIO_MAX_CHANNELS                  (2) // Maximum number of channels.

#define AUDIO_SAI_CK_PIN                    (&omv_pin_E2_SAI4)
#define AUDIO_SAI_D1_PIN                    (&omv_pin_B2_SAI4)

#define AUDIO_SAI_DMA_STREAM                BDMA_Channel1
#define AUDIO_SAI_DMA_REQUEST               BDMA_REQUEST_SAI4_A
#define AUDIO_SAI_DMA_IRQ                   BDMA_Channel1_IRQn
#define AUDIO_SAI_DMA_IRQHandler            BDMA_Channel1_IRQHandler

#define AUDIO_SAI_CLK_ENABLE()              __HAL_RCC_SAI4_CLK_ENABLE()
#define AUDIO_SAI_CLK_DISABLE()             __HAL_RCC_SAI4_CLK_DISABLE()
#define AUDIO_SAI_DMA_CLK_ENABLE()          __HAL_RCC_BDMA_CLK_ENABLE()

// SAI1
// Set SAI1 clock source in system ex: Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL;
//#define AUDIO_SAI                         (SAI1_Block_A)
//#define AUDIO_SAI_MCKDIV                  (12)
//#define AUDIO_SAI_FREQKHZ                 (2048U) // 2048KHz
//#define AUDIO_MAX_CHANNELS                (2) // Maximum number of channels.
//
//#define AUDIO_SAI_CK_PIN                  (&omv_pin_E2_SAI1)
//#define AUDIO_SAI_D1_PIN                  (&omv_pin_B2_SAI1)
//
//#define AUDIO_SAI_DMA_STREAM              DMA2_Stream6
//#define AUDIO_SAI_DMA_REQUEST             DMA_REQUEST_SAI1_A
//#define AUDIO_SAI_DMA_IRQ                 DMA2_Stream6_IRQn
//#define AUDIO_SAI_DMA_IRQHandler          DMA2_Stream6_IRQHandler
//
//#define AUDIO_SAI_CLK_ENABLE()            __HAL_RCC_SAI1_CLK_ENABLE()
//#define AUDIO_SAI_CLK_DISABLE()           __HAL_RCC_SAI1_CLK_DISABLE()
//#define AUDIO_SAI_DMA_CLK_ENABLE()        __HAL_RCC_DMA2_CLK_ENABLE()

// LCD Interface
#define OMV_RGB_DISPLAY_CONTROLLER          (LTDC)
#define OMV_RGB_DISPLAY_CLK_ENABLE()        __HAL_RCC_LTDC_CLK_ENABLE()
#define OMV_RGB_DISPLAY_CLK_DISABLE()       __HAL_RCC_LTDC_CLK_DISABLE()
#define OMV_RGB_DISPLAY_FORCE_RESET()       __HAL_RCC_LTDC_FORCE_RESET()
#define OMV_RGB_DISPLAY_RELEASE_RESET()     __HAL_RCC_LTDC_RELEASE_RESET()

// DSI Interface
//#define OMV_DSI_DISPLAY_CONTROLLER        (DSI)
//#define OMV_DSI_DISPLAY_CLK_ENABLE()      __HAL_RCC_DSI_CLK_ENABLE()
//#define OMV_DSI_DISPLAY_CLK_DISABLE()     __HAL_RCC_DSI_CLK_DISABLE()
//#define OMV_DSI_DISPLAY_FORCE_RESET()     __HAL_RCC_DSI_FORCE_RESET()
//#define OMV_DSI_DISPLAY_RELEASE_RESET()   __HAL_RCC_DSI_RELEASE_RESET()

// SPI LCD Interface
#define OMV_SPI_DISPLAY_CONTROLLER          (SPI2_ID)
#define OMV_SPI_DISPLAY_MOSI_PIN            (&omv_pin_C3_SPI2)
#define OMV_SPI_DISPLAY_MISO_PIN            (&omv_pin_C2_SPI2)
#define OMV_SPI_DISPLAY_SCLK_PIN            (&omv_pin_I1_SPI2)
#define OMV_SPI_DISPLAY_SSEL_PIN            (&omv_pin_I0_GPIO)

#define OMV_SPI_DISPLAY_RS_PIN              (&omv_pin_C6_GPIO)
#define OMV_SPI_DISPLAY_RST_PIN             (&omv_pin_C7_GPIO)
#define OMV_SPI_DISPLAY_TRIPLE_BUFFER       (1)

// FIR Lepton
#define OMV_FIR_LEPTON_I2C_BUS              (FIR_I2C_ID)
#define OMV_FIR_LEPTON_I2C_BUS_SPEED        (FIR_I2C_SPEED)

#define OMV_FIR_LEPTON_SPI_BUS              (SPI2_ID)
#define OMV_FIR_LEPTON_MOSI_PIN             (&omv_pin_C3_SPI2)
#define OMV_FIR_LEPTON_MISO_PIN             (&omv_pin_C2_SPI2)
#define OMV_FIR_LEPTON_SCLK_PIN             (&omv_pin_I1_SPI2)
#define OMV_FIR_LEPTON_SSEL_PIN             (&omv_pin_I0_GPIO)

#endif //__OMV_BOARDCONFIG_H__

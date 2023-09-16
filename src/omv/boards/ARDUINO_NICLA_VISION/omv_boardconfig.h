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
#define OMV_ARCH_STR                          "NICLAV H7 1024" // 33 chars max
#define OMV_BOARD_TYPE                        "NICLAV"
#define OMV_UNIQUE_ID_ADDR                    0x1FF1E800 // Unique ID address.
#define OMV_UNIQUE_ID_SIZE                    3 // Unique ID size in words.
#define OMV_UNIQUE_ID_OFFSET                  4 // Bytes offset for multi-word UIDs.


#define OMV_XCLK_MCO                          (0U)
#define OMV_XCLK_TIM                          (1U)
#define OMV_XCLK_OSC                          (2U)

// Sensor external clock source
#define OMV_XCLK_SOURCE                       (OMV_XCLK_TIM)

// Sensor external clock timer frequency
#define OMV_XCLK_FREQUENCY                    (12000000)

// GC4145 Sensor Settings
#define OMV_GC2145_ROTATE                     (1)

// Enable hardware JPEG
#define OMV_HARDWARE_JPEG                     (1)

// Enable MDMA sensor offload
#define OMV_ENABLE_SENSOR_MDMA                (1)

// Enable additional GPIO banks
#define OMV_ENABLE_GPIO_BANK_F                (1)
#define OMV_ENABLE_GPIO_BANK_G                (1)
#define OMV_ENABLE_GPIO_BANK_H                (1)
#define OMV_ENABLE_GPIO_BANK_I                (1)
#define OMV_ENABLE_GPIO_BANK_J                (1)
#define OMV_ENABLE_GPIO_BANK_K                (1)

// Configure image sensor drivers
#define OMV_ENABLE_OV2640                     (0)
#define OMV_ENABLE_OV5640                     (0)
#define OMV_ENABLE_OV7690                     (1)
#define OMV_ENABLE_OV7725                     (0)
#define OMV_ENABLE_OV9650                     (0)
#define OMV_ENABLE_MT9V0XX                    (0)
#define OMV_ENABLE_MT9M114                    (0)
#define OMV_ENABLE_LEPTON                     (0)
#define OMV_ENABLE_HM01B0                     (0)
#define OMV_ENABLE_GC2145                     (1)

// Configure FIR sensors drivers
#define OMV_ENABLE_FIR_MLX90621               (1)
#define OMV_ENABLE_FIR_MLX90640               (1)
#define OMV_ENABLE_FIR_MLX90641               (1)
#define OMV_ENABLE_FIR_AMG8833                (1)
#define OMV_ENABLE_FIR_LEPTON                 (1)

// Enable WiFi debug
#define OMV_ENABLE_WIFIDBG                    (0)

// Enable self-tests on first boot
#define OMV_ENABLE_SELFTEST                   (0)

// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH                   (320 * 240 * 2)

// Low and high JPEG QS.
#define JPEG_QUALITY_LOW                      50
#define JPEG_QUALITY_HIGH                     90

// FB Heap Block Size
#define OMV_UMM_BLOCK_SIZE                    16

// Core VBAT for selftests
#define OMV_CORE_VBAT                         "3.0"

// USB IRQn.
#define OMV_USB_IRQN                          (OTG_HS_IRQn)
#define OMV_USB_ULPI                          (1)
#define OMV_USB_ULPI_STP_PIN                  (&omv_pin_C0_OTG_HS)
#define OMV_USB_ULPI_DIR_PIN                  (&omv_pin_C2_OTG_HS)

// Defined for cpu frequency scaling to override the revid.
#define OMV_MAX_CPU_FREQ                      (400)

// PLL1 400MHz/40MHz for SDMMC and FDCAN
// USB and RNG are clocked from the HSI48
#define OMV_OSC_PLL1M                         (5)
#define OMV_OSC_PLL1N                         (160)
#define OMV_OSC_PLL1P                         (2)
#define OMV_OSC_PLL1Q                         (16)
#define OMV_OSC_PLL1R                         (2)
#define OMV_OSC_PLL1VCI                       (RCC_PLL1VCIRANGE_2)
#define OMV_OSC_PLL1VCO                       (RCC_PLL1VCOWIDE)
#define OMV_OSC_PLL1FRAC                      (0)

// PLL2 200MHz for FMC and QSPI.
#define OMV_OSC_PLL2M                         (5)
#define OMV_OSC_PLL2N                         (80)
#define OMV_OSC_PLL2P                         (2)
#define OMV_OSC_PLL2Q                         (2)
#define OMV_OSC_PLL2R                         (2)
#define OMV_OSC_PLL2VCI                       (RCC_PLL2VCIRANGE_2)
#define OMV_OSC_PLL2VCO                       (RCC_PLL2VCOWIDE)
#define OMV_OSC_PLL2FRAC                      (0)

// PLL3 160MHz for ADC and SPI123
#define OMV_OSC_PLL3M                         (5)
#define OMV_OSC_PLL3N                         (160)
#define OMV_OSC_PLL3P                         (2)
#define OMV_OSC_PLL3Q                         (5)
#define OMV_OSC_PLL3R                         (2)
#define OMV_OSC_PLL3VCI                       (RCC_PLL3VCIRANGE_2)
#define OMV_OSC_PLL3VCO                       (RCC_PLL3VCOWIDE)
#define OMV_OSC_PLL3FRAC                      (0)

// Clock Sources
#define OMV_OSC_PLL_CLKSOURCE                 RCC_PLLSOURCE_HSE
#define OMV_OSC_USB_CLKSOURCE                 RCC_USBCLKSOURCE_HSI48
#define OMV_OSC_RNG_CLKSOURCE                 RCC_RNGCLKSOURCE_HSI48
#define OMV_OSC_ADC_CLKSOURCE                 RCC_ADCCLKSOURCE_PLL3
#define OMV_OSC_SPI45_CLKSOURCE               RCC_SPI45CLKSOURCE_PLL3
#define OMV_OSC_SPI123_CLKSOURCE              RCC_SPI123CLKSOURCE_PLL2
#define OMV_OSC_DFSDM1_CLKSOURCE              RCC_DFSDM1CLKSOURCE_D2PCLK1

// HSE/HSI/CSI State
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
#define OMV_STACK_MEMORY                      DTCM // stack memory
#define OMV_DMA_MEMORY                        SRAM2 // DMA buffers memory.
#define OMV_FB_MEMORY                         AXI_SRAM // Framebuffer, fb_alloc
#define OMV_JPEG_MEMORY                       SRAM3 // JPEG buffer memory buffer.
#define OMV_VOSPI_MEMORY                      SRAM4 // VoSPI frame buffer memory.
#define OMV_VOSPI_MEMORY_OFFSET               (20K) // SRAM4 reserves 16K for CM4 + 4K D3 DMA buffers.

#define OMV_FB_SIZE                           (400K) // FB memory: header + VGA/GS image
#define OMV_FB_ALLOC_SIZE                     (100K) // minimum fb alloc size
#define OMV_STACK_SIZE                        (64K)
#define OMV_HEAP_SIZE                         (182K)

#define OMV_LINE_BUF_SIZE                     (3 * 1024) // Image line buffer round(640 * 2BPP * 2 buffers).
#define OMV_MSC_BUF_SIZE                      (2K) // USB MSC bot data
#define OMV_VFS_BUF_SIZE                      (1K) // VFS struct + FATFS file buffer (624 bytes)
#define OMV_FIR_LEPTON_BUF_SIZE               (1K) // FIR Lepton Packet Double Buffer (328 bytes)
#define OMV_JPEG_BUF_SIZE                     (32 * 1024) // IDE JPEG buffer (header + data).

// Memory map.
#define OMV_FLASH_ORIGIN                      0x08000000
#define OMV_FLASH_LENGTH                      2048K
#define OMV_DTCM_ORIGIN                       0x20000000 // Note accessible by CPU and MDMA only.
#define OMV_DTCM_LENGTH                       128K
#define OMV_ITCM_ORIGIN                       0x00000000
#define OMV_ITCM_LENGTH                       64K
#define OMV_SRAM1_ORIGIN                      0x30000000
#define OMV_SRAM1_LENGTH                      240K
#define OMV_SRAM2_ORIGIN                      0x3003C000 // 16KB of SRAM1
#define OMV_SRAM2_LENGTH                      16K
#define OMV_SRAM3_ORIGIN                      0x30040000
#define OMV_SRAM3_LENGTH                      32K
#define OMV_SRAM4_ORIGIN                      0x38000000
#define OMV_SRAM4_LENGTH                      64K
#define OMV_AXI_SRAM_ORIGIN                   0x24000000
#define OMV_AXI_SRAM_LENGTH                   512K
#define OMV_CM4_RAM_ORIGIN                    0x38000000 // Cortex-M4 memory @SRAM4.
#define OMV_CM4_RAM_LENGTH                    16K

// Flash configuration.
#define OMV_FLASH_FFS_ORIGIN                  0x08020000
#define OMV_FLASH_FFS_LENGTH                  128K
#define OMV_FLASH_TXT_ORIGIN                  0x08040000
#define OMV_FLASH_TXT_LENGTH                  1792K
#define OMV_FLASH_EXT_ORIGIN                  0x90000000
#define OMV_FLASH_EXT_LENGTH                  16M
#define OMV_CM4_FLASH_ORIGIN                  0x08020000
#define OMV_CM4_FLASH_LENGTH                  128K

// Domain 1 DMA buffers region.
#define OMV_DMA_MEMORY_D1                     AXI_SRAM
#define OMV_DMA_MEMORY_D1_SIZE                (8 * 1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D1_BASE                (OMV_AXI_SRAM_ORIGIN + (500 * 1024))
#define OMV_DMA_REGION_D1_SIZE                MPU_REGION_SIZE_8KB

// Domain 2 DMA buffers region.
#define OMV_DMA_MEMORY_D2                     SRAM2
#define OMV_DMA_MEMORY_D2_SIZE                (4 * 1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D2_BASE                (OMV_SRAM2_ORIGIN + (0 * 1024))
#define OMV_DMA_REGION_D2_SIZE                MPU_REGION_SIZE_16KB

// Domain 3 DMA buffers region.
#define OMV_DMA_MEMORY_D3                     SRAM4
#define OMV_DMA_MEMORY_D3_SIZE                (4 * 1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D3_BASE                (OMV_SRAM4_ORIGIN + (16 * 1024))
#define OMV_DMA_REGION_D3_SIZE                MPU_REGION_SIZE_4KB

// AXI QoS - Low-High (0:15) - default 0
#define OMV_AXI_QOS_MDMA_R_PRI                15 // Max pri to move data.
#define OMV_AXI_QOS_MDMA_W_PRI                15 // Max pri to move data.

// Main image sensor I2C bus
#define ISC_I2C_ID                            (3)
#define ISC_I2C_SPEED                         (OMV_I2C_SPEED_STANDARD)

// Thermal image sensor I2C bus
#define FIR_I2C_ID                            (1)
#define FIR_I2C_SPEED                         (OMV_I2C_SPEED_FULL)

// Soft I2C bus
#define SOFT_I2C_SIOC_PIN                     (&omv_pin_C10_GPIO)
#define SOFT_I2C_SIOD_PIN                     (&omv_pin_C11_GPIO)
#define SOFT_I2C_SPIN_DELAY                   64

// IMU SPI bus
#define IMU_SPI_ID                            (5)
#define IMU_SPI_BAUDRATE                      (500000)
#define IMU_CHIP_LSM6DSOX                     (1)
#define OMV_IMU_X_Y_ROTATION_DEGREES          90
#define OMV_IMU_MOUNTING_Z_DIRECTION          -1

// DCMI timer
#define DCMI_TIM                              (TIM3)
#define DCMI_TIM_PIN                          (&omv_pin_A7_TIM3)
#define DCMI_TIM_CHANNEL                      (TIM_CHANNEL_2)
#define DCMI_TIM_CLK_ENABLE()                 __TIM3_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()                __TIM3_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()                  HAL_RCC_GetPCLK1Freq()

// DCMI pins
//#define DCMI_RESET_PIN         (&omv_pin_A10_GPIO)
//#define DCMI_POWER_PIN         (&omv_pin_G3_GPIO)
//#define DCMI_FSYNC_PIN         (&omv_pin_B4_GPIO)

#define DCMI_D0_PIN                           (&omv_pin_C6_DCMI)
#define DCMI_D1_PIN                           (&omv_pin_C7_DCMI)
#define DCMI_D2_PIN                           (&omv_pin_E0_DCMI)
#define DCMI_D3_PIN                           (&omv_pin_E1_DCMI)
#define DCMI_D4_PIN                           (&omv_pin_E4_DCMI)
#define DCMI_D5_PIN                           (&omv_pin_D3_DCMI)
#define DCMI_D6_PIN                           (&omv_pin_E5_DCMI)
#define DCMI_D7_PIN                           (&omv_pin_E6_DCMI)

#define DCMI_HSYNC_PIN                        (&omv_pin_A4_DCMI)
#define DCMI_VSYNC_PIN                        (&omv_pin_G9_DCMI)
#define DCMI_PXCLK_PIN                        (&omv_pin_A6_DCMI)

// DFSDM1
#define AUDIO_DFSDM                           (DFSDM1_Channel2)
#define AUDIO_DFSDM_CHANNEL                   (DFSDM_CHANNEL_2)
// DFSDM output clock is derived from the Aclk (set in SAI1SEL[2:0])
// for SAI1 and DFSDM1, which is clocked from PLL1Q by default (50MHz).
#define AUDIO_DFSDM_FREQMHZ                   (50)
#define AUDIO_MAX_CHANNELS                    (1) // Maximum number of channels.

#define AUDIO_DFSDM_CK_PIN                    (&omv_pin_D10_DFSDM1)
#define AUDIO_DFSDM_D1_PIN                    (&omv_pin_E7_DFSDM1)

#define AUDIO_DFSDM_FLT0                      DFSDM1_Filter0
#define AUDIO_DFSDM_FLT0_IRQ                  DFSDM1_FLT0_IRQn
#define AUDIO_DFSDM_FLT0_IRQHandler           DFSDM1_FLT0_IRQHandler
#define AUDIO_DFSDM_FLT0_DMA_STREAM           DMA1_Stream1
#define AUDIO_DFSDM_FLT0_DMA_REQUEST          DMA_REQUEST_DFSDM1_FLT0
#define AUDIO_DFSDM_FLT0_DMA_IRQ              DMA1_Stream1_IRQn
#define AUDIO_DFSDM_FLT0_DMA_IRQHandler       DMA1_Stream1_IRQHandler

#define AUDIO_DFSDM_CLK_ENABLE()              __HAL_RCC_DFSDM1_CLK_ENABLE()
#define AUDIO_DFSDM_CLK_DISABLE()             __HAL_RCC_DFSDM1_CLK_DISABLE()
#define AUDIO_DFSDM_DMA_CLK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()

// Physical I2C buses
// I2C bus 1
#define I2C1_ID                               (1)
#define I2C1_SCL_PIN                          (&omv_pin_B8_I2C1)
#define I2C1_SDA_PIN                          (&omv_pin_B9_I2C1)

// I2C bus 3
#define I2C3_ID                               (3)
#define I2C3_SCL_PIN                          (&omv_pin_A8_I2C3)
#define I2C3_SDA_PIN                          (&omv_pin_C9_I2C3)

// Physical SPI buses.
// SPI bus 5
#define SPI5_ID                               (5)
#define SPI5_SCLK_PIN                         (&omv_pin_F7_SPI5)
#define SPI5_MISO_PIN                         (&omv_pin_F8_SPI5)
#define SPI5_MOSI_PIN                         (&omv_pin_F11_SPI5)
#define SPI5_SSEL_PIN                         (&omv_pin_F6_SPI5)
#define SPI5_DMA_TX_CHANNEL                   (DMA2_Stream4)
#define SPI5_DMA_RX_CHANNEL                   (DMA2_Stream3)

// SPI LCD Interface
#define OMV_SPI_LCD_CONTROLLER                (&spi_obj[3])
#define OMV_SPI_LCD_CONTROLLER_INSTANCE       (SPI4)

#define OMV_SPI_LCD_MOSI_PIN                  (&omv_pin_E14_SPI4)
#define OMV_SPI_LCD_MISO_PIN                  (&omv_pin_E13_SPI4)
#define OMV_SPI_LCD_SCLK_PIN                  (&omv_pin_E12_SPI4)
#define OMV_SPI_LCD_SSEL_PIN                  (&omv_pin_E11_GPIO)

#define OMV_SPI_LCD_RS_PIN                    (&omv_pin_C11_GPIO)
//#define OMV_SPI_LCD_BL_PIN                (&omv_pin_A4_GPIO)
#define OMV_SPI_LCD_RST_PIN                   (&omv_pin_C10_GPIO)

#ifdef OMV_SPI_LCD_BL_PIN
#define OMV_SPI_LCD_BL_DAC                    (DAC1)
#define OMV_SPI_LCD_BL_DAC_CHANNEL            (DAC_CHANNEL_1)
#define OMV_SPI_LCD_BL_DAC_CLK_ENABLE()       __HAL_RCC_DAC12_CLK_ENABLE()
#define OMV_SPI_LCD_BL_DAC_CLK_DISABLE()      __HAL_RCC_DAC12_CLK_DISABLE()
#define OMV_SPI_LCD_BL_DAC_FORCE_RESET()      __HAL_RCC_DAC12_FORCE_RESET()
#define OMV_SPI_LCD_BL_DAC_RELEASE_RESET()    __HAL_RCC_DAC12_RELEASE_RESET()
#endif

// FIR Lepton
#define OMV_FIR_LEPTON_I2C_BUS                (FIR_I2C_ID)
#define OMV_FIR_LEPTON_I2C_BUS_SPEED          (FIR_I2C_SPEED)
#define OMV_FIR_LEPTON_CONTROLLER             (&spi_obj[3])
#define OMV_FIR_LEPTON_CONTROLLER_INSTANCE    (SPI4)

#define OMV_FIR_LEPTON_MOSI_PIN               (&omv_pin_E14_SPI4)
#define OMV_FIR_LEPTON_MISO_PIN               (&omv_pin_E13_SPI4)
#define OMV_FIR_LEPTON_SCLK_PIN               (&omv_pin_E12_SPI4)
#define OMV_FIR_LEPTON_SSEL_PIN               (&omv_pin_E11_GPIO)

#endif //__OMV_BOARDCONFIG_H__

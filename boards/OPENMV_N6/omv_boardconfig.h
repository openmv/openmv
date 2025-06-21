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
#define OMV_BOARD_ARCH                      "OpenMV N6" // 33 chars max
#define OMV_BOARD_TYPE                      "N6"
#define OMV_BOARD_UID_ADDR                  0x46009014    // Unique ID address.
#define OMV_BOARD_UID_SIZE                  3             // Unique ID size in words.
#define OMV_BOARD_UID_OFFSET                4             // Bytes offset for multi-word UIDs.

// JPEG compression settings.
#define OMV_JPEG_CODEC_ENABLE               (1)
#define OMV_JPEG_QUALITY_LOW                (50)
#define OMV_JPEG_QUALITY_HIGH               (90)
#define OMV_JPEG_QUALITY_THRESHOLD          (800 * 600 * 2)

// Enable RAW preview.
#define OMV_RAW_PREVIEW_ENABLE              (1)
#define OMV_RAW_PREVIEW_WIDTH               (512)
#define OMV_RAW_PREVIEW_HEIGHT              (512)

// GPU Configuration
#define OMV_GPU_ENABLE                      (1)
#define OMV_GPU_NEMA                        (1)
#define OMV_GPU_NEMA_BUFFER_SIZE            (32 * 1024)

#define OMV_OV7725_ENABLE                   (1)
#define OMV_OV7725_PLL_CONFIG               (0x41) // x4
#define OMV_OV7725_BANDING                  (0x7F)

#define OMV_OV5640_ENABLE                   (1)
#define OMV_OV5640_AF_ENABLE                (1)
#define OMV_OV5640_CLK_FREQ                 (24000000)
#define OMV_OV5640_PLL_CTRL2                (0x64)
#define OMV_OV5640_PLL_CTRL3                (0x13)
#define OMV_OV5640_REV_Y_CHECK              (1)
#define OMV_OV5640_REV_Y_FREQ               (25000000)
#define OMV_OV5640_REV_Y_CTRL2              (0x54)
#define OMV_OV5640_REV_Y_CTRL3              (0x13)

#define OMV_MT9V0XX_ENABLE                  (1)
#define OMV_LEPTON_ENABLE                   (1)
#define OMV_PAG7936_ENABLE                  (1)
#define OMV_PAG7936_MIPI_CSI2               (1)
#define OMV_PS5520_ENABLE                   (1)
#define OMV_GENX320_EHC_ENABLE              (1)
#define OMV_GENX320_HSYNC_VALUE             (0x1)

// FIR drivers configuration.
#define OMV_FIR_MLX90621_ENABLE             (1)
#define OMV_FIR_MLX90640_ENABLE             (1)
#define OMV_FIR_MLX90641_ENABLE             (1)
#define OMV_FIR_AMG8833_ENABLE              (1)
#define OMV_FIR_LEPTON_ENABLE               (0)

// UMM heap block size
#define OMV_UMM_BLOCK_SIZE                  256

// USB IRQn.
#define OMV_USB_IRQN                        (USB1_OTG_HS_IRQn)

//PLL1 800MHz
#define OMV_OSC_PLL1M                       (3)
#define OMV_OSC_PLL1N                       (50)
#define OMV_OSC_PLL1P1                      (1)
#define OMV_OSC_PLL1P2                      (1)
#define OMV_OSC_PLL1FRAC                    (0)
#define OMV_OSC_PLL1SOURCE                  RCC_PLLSOURCE_HSE

//PLL2 1000MHz
#define OMV_OSC_PLL2M                       (6)
#define OMV_OSC_PLL2N                       (125)
#define OMV_OSC_PLL2P1                      (1)
#define OMV_OSC_PLL2P2                      (1)
#define OMV_OSC_PLL2FRAC                    (0)
#define OMV_OSC_PLL2SOURCE                  RCC_PLLSOURCE_HSE

//PLL3  1200MHz
#define OMV_OSC_PLL3M                       (1)
#define OMV_OSC_PLL3N                       (25)
#define OMV_OSC_PLL3P1                      (1)
#define OMV_OSC_PLL3P2                      (1)
#define OMV_OSC_PLL3FRAC                    (0)
#define OMV_OSC_PLL3SOURCE                  RCC_PLLSOURCE_HSE

//PLL4  1200MHz
#define OMV_OSC_PLL4M                       (1)
#define OMV_OSC_PLL4N                       (25)
#define OMV_OSC_PLL4P1                      (1)
#define OMV_OSC_PLL4P2                      (1)
#define OMV_OSC_PLL4FRAC                    (0)
#define OMV_OSC_PLL4SOURCE                  RCC_PLLSOURCE_HSE

// Clock Sources
#define OMV_RCC_IC8_SOURCE                  (RCC_ICCLKSOURCE_PLL3)
#define OMV_RCC_IC8_CLKDIV                  (25)

#define OMV_RCC_IC9_SOURCE                  (RCC_ICCLKSOURCE_PLL1)
#define OMV_RCC_IC9_CLKDIV                  (10)

#define OMV_RCC_IC10_SOURCE                 (RCC_ICCLKSOURCE_PLL1)
#define OMV_RCC_IC10_CLKDIV                 (8)

#define OMV_RCC_IC14_SOURCE                 (RCC_ICCLKSOURCE_PLL1)
#define OMV_RCC_IC14_CLKDIV                 (10)

#define OMV_RCC_IC15_SOURCE                 (RCC_ICCLKSOURCE_PLL1)
#define OMV_RCC_IC15_CLKDIV                 (16)

#define OMV_RCC_IC17_SOURCE                 (RCC_ICCLKSOURCE_PLL3)
#define OMV_RCC_IC17_CLKDIV                 (4)

#define OMV_RCC_IC18_SOURCE                 (RCC_ICCLKSOURCE_PLL3)
#define OMV_RCC_IC18_CLKDIV                 (60)

#define OMV_OSC_I2C3_SOURCE                 (RCC_I2C3CLKSOURCE_IC10)
#define OMV_OSC_SPI5_SOURCE                 (RCC_SPI5CLKSOURCE_IC9)
#define OMV_OSC_DCMIPP_SOURCE               (RCC_DCMIPPCLKSOURCE_IC17)
#define OMV_OSC_CSI_SOURCE                  (0) // has one clock source IC18
#define OMV_OSC_ADF1_SOURCE                 (RCC_ADF1CLKSOURCE_IC8)

// HSE/HSI/CSI State
#define OMV_OSC_HSE_STATE                   (RCC_HSE_ON)
#define OMV_OSC_HSI_STATE                   (RCC_HSI_ON)
#define OMV_OSC_HSI_DIV                     (RCC_HSI_DIV1)
#define OMV_OSC_HSI_CAL                     (RCC_HSICALIBRATION_DEFAULT)


// Power supply configuration
#define OMV_PWR_SUPPLY                      (PWR_SMPS_SUPPLY)

// Linker script constants (see the linker script template stm32fxxx.ld.S).
// Note: fb_alloc is a stack-based, dynamically allocated memory on FB.
// The maximum available fb_alloc memory = FB_ALLOC_SIZE + FB_SIZE - (w*h*bpp).
#define OMV_MAIN_MEMORY                     SRAM1  // Data/BSS memory
#define OMV_STACK_MEMORY                    SRAM1  // stack memory
#define OMV_RAMFUNC_MEMORY                  ITCM
#define OMV_STACK_SIZE                      (64K)
#define OMV_HEAP_MEMORY                     SRAM1  // libc/sbrk heap memory
#define OMV_HEAP_SIZE                       (256K)
#define OMV_FB_MEMORY                       DRAM   // Framebuffer, fb_alloc
#define OMV_FB_SIZE                         (20M)   // FB memory.
#define OMV_FB_ALLOC_SIZE                   (11M)   // minimum fb_alloc size
#define OMV_JPEG_MEMORY                     DRAM   // JPEG buffer memory buffer.
#define OMV_JPEG_SIZE                       (1M)   // IDE JPEG buffer (header + data).
#define OMV_DMA_MEMORY                      SRAM1  // Misc DMA buffers memory.
#define OMV_GC_BLOCK0_MEMORY                SRAM2  // Main GC block
#define OMV_GC_BLOCK0_SIZE                  (1M)
#define OMV_GC_BLOCK1_MEMORY                DRAM  // Main GC block
#define OMV_GC_BLOCK1_SIZE                  (24M)
#define OMV_MSC_BUF_SIZE                    (4K)   // USB MSC bot data

// Memory map.
#define OMV_DTCM_ORIGIN                     0x30000000
#define OMV_DTCM_LENGTH                     128K
#define OMV_ITCM_ORIGIN                     0x10000000
#define OMV_ITCM_LENGTH                     64K
#define OMV_SRAM1_ORIGIN                    0x34000000  // AXISRAM1
#define OMV_SRAM1_LENGTH                    1M          // 1MB
#define OMV_SRAM2_ORIGIN                    0x34100000  // AXISRAM2
#define OMV_SRAM2_LENGTH                    1M          // 1MB
#define OMV_SRAM3_ORIGIN                    0x34200000  // AXISRAM3
#define OMV_SRAM3_LENGTH                    448K        // 448KB
#define OMV_SRAM4_ORIGIN                    0x34270000  // AXISRAM4
#define OMV_SRAM4_LENGTH                    448K        // 448KB
#define OMV_SRAM5_ORIGIN                    0x342E0000  // AXISRAM5
#define OMV_SRAM5_LENGTH                    448K        // 448KB
#define OMV_SRAM6_ORIGIN                    0x34350000  // AXISRAM6
#define OMV_SRAM6_LENGTH                    448K        // 448KB
#define OMV_SRAM7_ORIGIN                    0x38000000  // AHBSRAM1 + AHBSRAM2 combined
#define OMV_SRAM7_LENGTH                    32K         // 16KB + 16KB = 32KB
#define OMV_DRAM_ORIGIN                     0x90000000  // XSPI1
#define OMV_DRAM_LENGTH                     64M         // 512 Mbits (64 MBytes)

// Flash configuration.
#define OMV_FLASH_BOOT_ORIGIN               0x34180400
#define OMV_FLASH_BOOT_LENGTH               512K
#define OMV_FLASH_TXT_ORIGIN                0x70080000
#define OMV_FLASH_TXT_LENGTH                3584K
#define OMV_ROMFS_PART0_ORIGIN              0x70800000
#define OMV_ROMFS_PART0_LENGTH              0x01800000

// Enable additional GPIO ports.
#define OMV_GPIO_PORT_F_ENABLE              (1)
#define OMV_GPIO_PORT_G_ENABLE              (1)
#define OMV_GPIO_PORT_H_ENABLE              (1)
#define OMV_GPIO_PORT_N_ENABLE              (1)
#define OMV_GPIO_PORT_O_ENABLE              (1)
#define OMV_GPIO_PORT_P_ENABLE              (1)
#define OMV_GPIO_PORT_Q_ENABLE              (1)

// Physical I2C buses.

// I2C bus 3
#define OMV_I2C2_ID                         (2)
#define OMV_I2C2_SCL_PIN                    (&omv_pin_B10_I2C2)
#define OMV_I2C2_SDA_PIN                    (&omv_pin_B11_I2C2)

// I2C bus 3
#define OMV_I2C3_ID                         (3)
#define OMV_I2C3_SCL_PIN                    (&omv_pin_A8_I2C3)
#define OMV_I2C3_SDA_PIN                    (&omv_pin_A9_I2C3)

// I2C bus 4
#define OMV_I2C4_ID                         (4)
#define OMV_I2C4_SCL_PIN                    (&omv_pin_E13_I2C4)
#define OMV_I2C4_SDA_PIN                    (&omv_pin_E14_I2C4)

// Physical SPI buses.

// SPI bus 5
#define OMV_SPI5_ID                         (5)
#define OMV_SPI5_SCLK_PIN                   (&omv_pin_E15_SPI5)
#define OMV_SPI5_MISO_PIN                   (&omv_pin_D4_SPI5)
#define OMV_SPI5_MOSI_PIN                   (&omv_pin_A4_SPI5)
#define OMV_SPI5_SSEL_PIN                   (&omv_pin_A3_SPI5)
#define OMV_SPI5_DMA_TX_CHANNEL             (GPDMA1_Channel8)
#define OMV_SPI5_DMA_TX_REQUEST             (GPDMA1_REQUEST_SPI5_TX)
#define OMV_SPI5_DMA_RX_CHANNEL             (GPDMA1_Channel9)
#define OMV_SPI5_DMA_RX_REQUEST             (GPDMA1_REQUEST_SPI5_RX)

// CSI SPI bus
#define OMV_CSI_SPI_ID                      (OMV_SPI5_ID)

// CSI I2C bus
#define OMV_CSI_I2C_ID                      (OMV_I2C3_ID)
#define OMV_CSI_I2C_SPEED                   (OMV_I2C_SPEED_STANDARD)

// FIR I2C bus
#define OMV_FIR_I2C_ID                      (OMV_I2C2_ID)
#define OMV_FIR_I2C_SPEED                   (OMV_I2C_SPEED_FULL)

// IMU SPI bus
#define OMV_IMU_I2C_ID                      (OMV_I2C4_ID)
#define OMV_IMU_I2C_SPEED                   (OMV_I2C_SPEED_FULL)
#define OMV_IMU_CHIP_LSM6DSM                (1)
#define OMV_IMU_X_Y_ROTATION_DEGREES        90
#define OMV_IMU_MOUNTING_Z_DIRECTION        -1

// MDF1
#define OMV_MDF                             (ADF1_Filter0)
#define OMV_MDF_PROC_CLKDIV                 (2)  // 48MHz / 2 = 24MHz
#define OMV_MDF_CCKY_CLKDIV                 (12) // 24MHz / 12 = 2 MHz
#define OMV_AUDIO_MAX_CHANNELS              (2)

#define OMV_MDF_CK_PIN                      (&omv_pin_E2_ADF1)
#define OMV_MDF_D1_PIN                      (&omv_pin_B2_ADF1)

#define OMV_MDF_FLT0_IRQ                    ADF1_FLT0_IRQn
#define OMV_MDF_FLT0_IRQHandler             ADF1_FLT0_IRQHandler
#define OMV_MDF_FLT0_DMA_STREAM             GPDMA1_Channel10
#define OMV_MDF_FLT0_DMA_REQUEST            GPDMA1_REQUEST_ADF1_FLT0
#define OMV_MDF_FLT0_DMA_IRQ                GPDMA1_Channel10_IRQn

// Camera Interface
#define OMV_CSI_CLK_SOURCE                  (OMV_CSI_CLK_SOURCE_TIM)
#define OMV_CSI_CLK_FREQUENCY               (12000000)
#define OMV_CSI_TIM                         (TIM1)
#define OMV_CSI_TIM_PIN                     (&omv_pin_E9_TIM1)
#define OMV_CSI_TIM_CHANNEL                 (TIM_CHANNEL_1)
#define OMV_CSI_TIM_CLK_ENABLE()            __TIM1_CLK_ENABLE()
#define OMV_CSI_TIM_CLK_DISABLE()           __TIM1_CLK_DISABLE()
#define OMV_CSI_TIM_CLK_SLEEP_ENABLE()      __TIM1_CLK_SLEEP_ENABLE()
#define OMV_CSI_TIM_CLK_SLEEP_DISABLE()     __TIM1_CLK_SLEEP_DISABLE()
#define OMV_CSI_TIM_PCLK_FREQ()             HAL_RCC_GetPCLK2Freq()
#define OMV_CSI_DMA_CHANNEL                 (HPDMA1_Channel12)
#define OMV_CSI_DMA_REQUEST                 (HPDMA1_REQUEST_DCMI_PSSI)
#define OMV_CSI_DMA_MEMCPY_ENABLE           (0)
#define OMV_CSI_HW_CROP_ENABLE              (1)

#define OMV_CSI_D0_PIN                      (&omv_pin_A1_DCMI)
#define OMV_CSI_D1_PIN                      (&omv_pin_A10_DCMI)
#define OMV_CSI_D2_PIN                      (&omv_pin_E0_DCMI)
#define OMV_CSI_D3_PIN                      (&omv_pin_E10_DCMI)
#define OMV_CSI_D4_PIN                      (&omv_pin_B0_DCMI)
#define OMV_CSI_D5_PIN                      (&omv_pin_E5_DCMI)
#define OMV_CSI_D6_PIN                      (&omv_pin_G2_DCMI)
#define OMV_CSI_D7_PIN                      (&omv_pin_F1_DCMI)

#define OMV_CSI_HSYNC_PIN                   (&omv_pin_D0_DCMI)
#define OMV_CSI_VSYNC_PIN                   (&omv_pin_E6_DCMI)
#define OMV_CSI_PXCLK_PIN                   (&omv_pin_G1_DCMI)
#define OMV_CSI_RESET_PIN                   (&omv_pin_E3_GPIO)
#define OMV_CSI_POWER_PIN                   (&omv_pin_E1_GPIO)
//#define OMV_CSI_FSYNC_PIN                   (&omv_pin_B4_GPIO)

#define OMV_XSPI1_IO00_PIN                  (&omv_pin_P0_XSPIM_P1)
#define OMV_XSPI1_IO01_PIN                  (&omv_pin_P1_XSPIM_P1)
#define OMV_XSPI1_IO02_PIN                  (&omv_pin_P2_XSPIM_P1)
#define OMV_XSPI1_IO03_PIN                  (&omv_pin_P3_XSPIM_P1)
#define OMV_XSPI1_IO04_PIN                  (&omv_pin_P4_XSPIM_P1)
#define OMV_XSPI1_IO05_PIN                  (&omv_pin_P5_XSPIM_P1)
#define OMV_XSPI1_IO06_PIN                  (&omv_pin_P6_XSPIM_P1)
#define OMV_XSPI1_IO07_PIN                  (&omv_pin_P7_XSPIM_P1)
#define OMV_XSPI1_IO08_PIN                  (&omv_pin_P8_XSPIM_P1)
#define OMV_XSPI1_IO09_PIN                  (&omv_pin_P9_XSPIM_P1)
#define OMV_XSPI1_IO10_PIN                  (&omv_pin_P10_XSPIM_P1)
#define OMV_XSPI1_IO11_PIN                  (&omv_pin_P11_XSPIM_P1)
#define OMV_XSPI1_IO12_PIN                  (&omv_pin_P12_XSPIM_P1)
#define OMV_XSPI1_IO13_PIN                  (&omv_pin_P13_XSPIM_P1)
#define OMV_XSPI1_IO14_PIN                  (&omv_pin_P14_XSPIM_P1)
#define OMV_XSPI1_IO15_PIN                  (&omv_pin_P15_XSPIM_P1)
#define OMV_XSPI1_NCS1_PIN                  (&omv_pin_O0_XSPIM_P1)
#define OMV_XSPI1_DQS0_PIN                  (&omv_pin_O2_XSPIM_P1)
#define OMV_XSPI1_DQS1_PIN                  (&omv_pin_O3_XSPIM_P1)
#define OMV_XSPI1_CLKP_PIN                  (&omv_pin_O4_XSPIM_P1)

#define OMV_XSPI_PSRAM_ID                   (1)
#define OMV_XSPI_PSRAM_SIZE                 (0x4000000)
#define OMV_XSPI_PSRAM_FREQUENCY            (200000000)
#endif //__OMV_BOARDCONFIG_H__

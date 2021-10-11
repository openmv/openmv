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
#define OMV_ARCH_STR            "BORMIO H7 1024" // 33 chars max
#define OMV_BOARD_TYPE          "BORMIO"
#define OMV_UNIQUE_ID_ADDR      0x1FF1E800
#define OMV_UNIQUE_ID_SIZE      3 // 3 words

// Flash sectors for the bootloader.
// Flash FS sector, main FW sector, max sector.
#define OMV_FLASH_LAYOUT        {1, 2, 15}

#define OMV_XCLK_MCO            (0U)
#define OMV_XCLK_TIM            (1U)
#define OMV_XCLK_OSC            (2U)

// Sensor external clock source.
#define OMV_XCLK_SOURCE         (OMV_XCLK_TIM)

// Sensor external clock timer frequency.
#define OMV_XCLK_FREQUENCY      (12000000)

// Bootloader LED GPIO port/pin
#define OMV_BOOTLDR_LED_PIN     (GPIO_PIN_12)
#define OMV_BOOTLDR_LED_PORT    (GPIOI)

// RAW buffer size
#define OMV_RAW_BUF_SIZE        (409600)

// Enable hardware JPEG
#define OMV_HARDWARE_JPEG       (1)

// Enable MDMA sensor offload.
#define OMV_ENABLE_SENSOR_MDMA  (1)

// Enable sensor drivers
#define OMV_ENABLE_OV2640       (0)
#define OMV_ENABLE_OV5640       (0)
#define OMV_ENABLE_OV7690       (1)
#define OMV_ENABLE_OV7725       (0)
#define OMV_ENABLE_OV9650       (0)
#define OMV_ENABLE_MT9V034      (0)
#define OMV_ENABLE_MT9M114      (1)
#define OMV_ENABLE_LEPTON       (0)
#define OMV_ENABLE_HM01B0       (0)
#define OMV_ENABLE_GC2145       (1)

// Enable sensor features
#define OMV_ENABLE_OV5640_AF    (0)

// Enable WiFi debug
#define OMV_ENABLE_WIFIDBG      (0)

// Enable self-tests on first boot
#define OMV_ENABLE_SELFTEST     (0)

// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH     (320*240*2)

// Low and high JPEG QS.
#define JPEG_QUALITY_LOW        50
#define JPEG_QUALITY_HIGH       90

// FB Heap Block Size
#define OMV_UMM_BLOCK_SIZE      16

// Core VBAT for selftests
#define OMV_CORE_VBAT           "3.0"

// USB IRQn.
#define OMV_USB_IRQN            (OTG_HS_IRQn)
#define OMV_USB_ULPI            (1)
#define OMV_USB_ULPI_DIR_PORT   (GPIOC)
#define OMV_USB_ULPI_DIR_PIN    (2)
#define OMV_USB_ULPI_DIR_CLK_ENABLE()   __HAL_RCC_GPIOC_CLK_ENABLE()

// Defined for cpu frequency scaling to override the revid.
#define OMV_MAX_CPU_FREQ        (400)

// PLL1 400MHz/40MHz for SDMMC and FDCAN
// USB and RNG are clocked from the HSI48
#define OMV_OSC_PLL1M           (5)
#define OMV_OSC_PLL1N           (160)
#define OMV_OSC_PLL1P           (2)
#define OMV_OSC_PLL1Q           (16)
#define OMV_OSC_PLL1R           (2)
#define OMV_OSC_PLL1VCI         (RCC_PLL1VCIRANGE_2)
#define OMV_OSC_PLL1VCO         (RCC_PLL1VCOWIDE)
#define OMV_OSC_PLL1FRAC        (0)

// PLL2 200MHz for FMC and QSPI.
#define OMV_OSC_PLL2M           (5)
#define OMV_OSC_PLL2N           (80)
#define OMV_OSC_PLL2P           (2)
#define OMV_OSC_PLL2Q           (2)
#define OMV_OSC_PLL2R           (2)
#define OMV_OSC_PLL2VCI         (RCC_PLL2VCIRANGE_2)
#define OMV_OSC_PLL2VCO         (RCC_PLL2VCOWIDE)
#define OMV_OSC_PLL2FRAC        (0)

// PLL3 160MHz for ADC and SPI123
#define OMV_OSC_PLL3M           (5)
#define OMV_OSC_PLL3N           (160)
#define OMV_OSC_PLL3P           (2)
#define OMV_OSC_PLL3Q           (5)
#define OMV_OSC_PLL3R           (2)
#define OMV_OSC_PLL3VCI         (RCC_PLL3VCIRANGE_2)
#define OMV_OSC_PLL3VCO         (RCC_PLL3VCOWIDE)
#define OMV_OSC_PLL3FRAC        (0)

// Clock Sources
#define OMV_OSC_PLL_CLKSOURCE       RCC_PLLSOURCE_HSE
#define OMV_OSC_USB_CLKSOURCE       RCC_USBCLKSOURCE_HSI48
#define OMV_OSC_RNG_CLKSOURCE       RCC_RNGCLKSOURCE_HSI48
#define OMV_OSC_ADC_CLKSOURCE       RCC_ADCCLKSOURCE_PLL3
#define OMV_OSC_SPI45_CLKSOURCE     RCC_SPI45CLKSOURCE_PLL3
#define OMV_OSC_SPI123_CLKSOURCE    RCC_SPI123CLKSOURCE_PLL2
#define OMV_OSC_DFSDM1_CLKSOURCE    RCC_DFSDM1CLKSOURCE_D2PCLK1

// HSE/HSI/CSI State
#define OMV_OSC_HSE_STATE       (RCC_HSE_BYPASS)
#define OMV_OSC_HSI48_STATE     (RCC_HSI48_ON)

// Flash Latency
#define OMV_FLASH_LATENCY       (FLASH_LATENCY_2)

// Power supply configuration
#define OMV_PWR_SUPPLY          (PWR_SMPS_1V8_SUPPLIES_LDO)

// Linker script constants (see the linker script template stm32fxxx.ld.S).
// Note: fb_alloc is a stack-based, dynamically allocated memory on FB.
// The maximum available fb_alloc memory = FB_ALLOC_SIZE + FB_SIZE - (w*h*bpp).
#define OMV_MAIN_MEMORY         SRAM1       // data, bss and heap
#define OMV_STACK_MEMORY        DTCM        // stack memory
#define OMV_DMA_MEMORY          SRAM2       // DMA buffers memory.
#define OMV_FB_MEMORY           AXI_SRAM    // Framebuffer, fb_alloc
#define OMV_JPEG_MEMORY         SRAM3       // JPEG buffer memory buffer.
#define OMV_VOSPI_MEMORY        SRAM4       // VoSPI buffer memory.
#define OMV_CYW43_MEMORY        FLASH_EXT   // CYW43 firmware in external flash mmap'd flash.
#define OMV_CYW43_MEMORY_OFFSET (0x90F00000)// Last Mbyte.

#define OMV_FB_SIZE             (400K)      // FB memory: header + VGA/GS image
#define OMV_FB_ALLOC_SIZE       (100K)      // minimum fb alloc size
#define OMV_STACK_SIZE          (64K)
#define OMV_HEAP_SIZE           (192K)

#define OMV_LINE_BUF_SIZE       (3 * 1024)  // Image line buffer round(640 * 2BPP * 2 buffers).
#define OMV_MSC_BUF_SIZE        (2K)        // USB MSC bot data
#define OMV_VFS_BUF_SIZE        (1K)        // VFS sturct + FATFS file buffer (624 bytes)
#define OMV_FIR_LEPTON_BUF_SIZE (1K)        // FIR Lepton Packet Double Buffer (328 bytes)
#define OMV_JPEG_BUF_SIZE       (32 * 1024) // IDE JPEG buffer (header + data).

#define OMV_BOOT_ORIGIN         0x08000000
#define OMV_BOOT_LENGTH         128K
#define OMV_TEXT_ORIGIN         0x08040000
#define OMV_TEXT_LENGTH         1792K
#define OMV_DTCM_ORIGIN         0x20000000  // Note accessible by CPU and MDMA only.
#define OMV_DTCM_LENGTH         128K
#define OMV_ITCM_ORIGIN         0x00000000
#define OMV_ITCM_LENGTH         64K
#define OMV_SRAM1_ORIGIN        0x30000000
#define OMV_SRAM1_LENGTH        240K
#define OMV_SRAM2_ORIGIN        0x3003C000  // 16KB of SRAM1
#define OMV_SRAM2_LENGTH        16K
#define OMV_SRAM3_ORIGIN        0x30040000
#define OMV_SRAM3_LENGTH        32K
#define OMV_SRAM4_ORIGIN        0x38000000
#define OMV_SRAM4_LENGTH        64K
#define OMV_AXI_SRAM_ORIGIN     0x24000000
#define OMV_AXI_SRAM_LENGTH     512K
#define OMV_FLASH_EXT_ORIGIN    0x90000000
#define OMV_FLASH_EXT_LENGTH    16M

// Domain 1 DMA buffers region.
#define OMV_DMA_MEMORY_D1       AXI_SRAM
#define OMV_DMA_MEMORY_D1_SIZE  (8*1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D1_BASE  (OMV_AXI_SRAM_ORIGIN+(500*1024))
#define OMV_DMA_REGION_D1_SIZE  MPU_REGION_SIZE_8KB

// Domain 2 DMA buffers region.
#define OMV_DMA_MEMORY_D2       SRAM2
#define OMV_DMA_MEMORY_D2_SIZE  (6*1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D2_BASE  (OMV_SRAM2_ORIGIN+(0*1024))
#define OMV_DMA_REGION_D2_SIZE  MPU_REGION_SIZE_16KB

// Domain 3 DMA buffers region.
//#define OMV_DMA_MEMORY_D3       SRAM4
//#define OMV_DMA_REGION_D3_BASE  (OMV_SRAM4_ORIGIN+(0*1024))
//#define OMV_DMA_REGION_D3_SIZE  MPU_REGION_SIZE_64KB

// AXI QoS - Low-High (0:15) - default 0
#define OMV_AXI_QOS_MDMA_R_PRI  15 // Max pri to move data.
#define OMV_AXI_QOS_MDMA_W_PRI  15 // Max pri to move data.

// Image sensor I2C
#define ISC_I2C                 (I2C3)
#define ISC_I2C_ID              (3)
#define ISC_I2C_AF              (GPIO_AF4_I2C3)
#define ISC_I2C_CLK_ENABLE()    __I2C3_CLK_ENABLE()
#define ISC_I2C_CLK_DISABLE()   __I2C3_CLK_DISABLE()
#define ISC_I2C_SCL_PORT        (GPIOA)
#define ISC_I2C_SCL_PIN         (GPIO_PIN_8)
#define ISC_I2C_SDA_PORT        (GPIOC)
#define ISC_I2C_SDA_PIN         (GPIO_PIN_9)
#define ISC_I2C_SPEED           (CAMBUS_SPEED_STANDARD)
#define ISC_I2C_FORCE_RESET()   __HAL_RCC_I2C3_FORCE_RESET()
#define ISC_I2C_RELEASE_RESET() __HAL_RCC_I2C3_RELEASE_RESET()

// FIR I2C
#define FIR_I2C                 (I2C1)
#define FIR_I2C_ID              (1)
#define FIR_I2C_AF              (GPIO_AF4_I2C1)
#define FIR_I2C_CLK_ENABLE()    __I2C1_CLK_ENABLE()
#define FIR_I2C_CLK_DISABLE()   __I2C1_CLK_DISABLE()
#define FIR_I2C_SCL_PORT        (GPIOB)
#define FIR_I2C_SCL_PIN         (GPIO_PIN_8)
#define FIR_I2C_SDA_PORT        (GPIOB)
#define FIR_I2C_SDA_PIN         (GPIO_PIN_9)
#define FIR_I2C_SPEED           (CAMBUS_SPEED_FULL)
#define FIR_I2C_FORCE_RESET()   __HAL_RCC_I2C1_FORCE_RESET()
#define FIR_I2C_RELEASE_RESET() __HAL_RCC_I2C1_RELEASE_RESET()

/* DCMI */
#define DCMI_TIM                (TIM3)
#define DCMI_TIM_PORT           (GPIOA)
#define DCMI_TIM_PIN            (GPIO_PIN_7)
#define DCMI_TIM_AF             (GPIO_AF2_TIM3)
#define DCMI_TIM_CHANNEL        (TIM_CHANNEL_2)
#define DCMI_TIM_CLK_ENABLE()   __TIM3_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()  __TIM3_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()    HAL_RCC_GetPCLK1Freq()

//#define DCMI_RESET_PORT         (GPIOA)
//#define DCMI_RESET_PIN          (GPIO_PIN_10)

//#define DCMI_PWDN_PORT          (GPIOG)
//#define DCMI_PWDN_PIN           (GPIO_PIN_3)

//#define DCMI_FSYNC_PORT         (GPIOB)
//#define DCMI_FSYNC_PIN          (GPIO_PIN_4)

#define DCMI_D0_PORT            (GPIOC)
#define DCMI_D0_PIN             (GPIO_PIN_6)

#define DCMI_D1_PORT            (GPIOC)
#define DCMI_D1_PIN             (GPIO_PIN_7)

#define DCMI_D2_PORT            (GPIOE)
#define DCMI_D2_PIN             (GPIO_PIN_0)

#define DCMI_D3_PORT            (GPIOE)
#define DCMI_D3_PIN             (GPIO_PIN_1)

#define DCMI_D4_PORT            (GPIOE)
#define DCMI_D4_PIN             (GPIO_PIN_4)

#define DCMI_D5_PORT            (GPIOD)
#define DCMI_D5_PIN             (GPIO_PIN_3)

#define DCMI_D6_PORT            (GPIOE)
#define DCMI_D6_PIN             (GPIO_PIN_5)

#define DCMI_D7_PORT            (GPIOE)
#define DCMI_D7_PIN             (GPIO_PIN_6)

#define DCMI_HSYNC_PORT         (GPIOA)
#define DCMI_HSYNC_PIN          (GPIO_PIN_4)

#define DCMI_VSYNC_PORT         (GPIOG)
#define DCMI_VSYNC_PIN          (GPIO_PIN_9)

#define DCMI_PXCLK_PORT         (GPIOA)
#define DCMI_PXCLK_PIN          (GPIO_PIN_6)

#if defined(DCMI_RESET_PIN)
#define DCMI_RESET_LOW()        HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_RESET)
#define DCMI_RESET_HIGH()       HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_SET)
#else
#define DCMI_RESET_LOW()
#define DCMI_RESET_HIGH()
#endif

#if defined(DCMI_PWDN_PIN)
#define DCMI_PWDN_LOW()         HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_SET)
#define DCMI_PWDN_HIGH()        HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_RESET)
#else
#define DCMI_PWDN_LOW()
#define DCMI_PWDN_HIGH()
#endif

#define DCMI_VSYNC_IRQN         EXTI9_5_IRQn
#define DCMI_VSYNC_IRQ_LINE     (9)

#define SOFT_I2C_PORT                GPIOC
#define SOFT_I2C_SIOC_PIN            GPIO_PIN_10
#define SOFT_I2C_SIOD_PIN            GPIO_PIN_11

#define SOFT_I2C_SIOC_H()            HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOC_PIN, GPIO_PIN_SET)
#define SOFT_I2C_SIOC_L()            HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOC_PIN, GPIO_PIN_RESET)

#define SOFT_I2C_SIOD_H()            HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN, GPIO_PIN_SET)
#define SOFT_I2C_SIOD_L()            HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN, GPIO_PIN_RESET)

#define SOFT_I2C_SIOD_READ()         HAL_GPIO_ReadPin (SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN)
#define SOFT_I2C_SIOD_WRITE(bit)     HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN, bit);

#define SOFT_I2C_SPIN_DELAY          64

// DFSDM1
#define AUDIO_DFSDM                     (DFSDM1_Channel2)
#define AUDIO_DFSDM_CHANNEL             (DFSDM_CHANNEL_2)
// DFSDM output clock is derived from the Aclk (set in SAI1SEL[2:0])
// for SAI1 and DFSDM1, which is clocked from PLL1Q by default (50MHz).
#define AUDIO_DFSDM_FREQMHZ             (50)
#define AUDIO_MAX_CHANNELS              (1) // Maximum number of channels.

#define AUDIO_DFSDM_CK_PORT             (GPIOD)
#define AUDIO_DFSDM_CK_PIN              (GPIO_PIN_10)
#define AUDIO_DFSDM_CK_AF               (GPIO_AF3_DFSDM1)

#define AUDIO_DFSDM_D1_PORT             (GPIOE)
#define AUDIO_DFSDM_D1_PIN              (GPIO_PIN_7)
#define AUDIO_DFSDM_D1_AF               (GPIO_AF3_DFSDM1)

#define AUDIO_DFSDM_FLT0                DFSDM1_Filter0
#define AUDIO_DFSDM_FLT0_IRQ            DFSDM1_FLT0_IRQn
#define AUDIO_DFSDM_FLT0_IRQHandler     DFSDM1_FLT0_IRQHandler
#define AUDIO_DFSDM_FLT0_DMA_STREAM     DMA1_Stream1
#define AUDIO_DFSDM_FLT0_DMA_REQUEST    DMA_REQUEST_DFSDM1_FLT0
#define AUDIO_DFSDM_FLT0_DMA_IRQ        DMA1_Stream1_IRQn
#define AUDIO_DFSDM_FLT0_DMA_IRQHandler DMA1_Stream1_IRQHandler

#define AUDIO_DFSDM_CLK_ENABLE()        __HAL_RCC_DFSDM1_CLK_ENABLE()
#define AUDIO_DFSDM_CLK_DISABLE()       __HAL_RCC_DFSDM1_CLK_DISABLE()
#define AUDIO_DFSDM_DMA_CLK_ENABLE()    __HAL_RCC_DMA1_CLK_ENABLE()

#define IMU_CHIP_LSM6DSOX           (1)
#define IMU_SPI                     (SPI5)
#define IMU_SPI_AF                  (GPIO_AF5_SPI5)
// SPI4/5 clock source is PLL3 (160MHz/16 == 10MHz).
#define IMU_SPI_PRESCALER           (SPI_BAUDRATEPRESCALER_16)

#define IMU_SPI_RESET()             __HAL_RCC_SPI5_FORCE_RESET()
#define IMU_SPI_RELEASE()           __HAL_RCC_SPI5_RELEASE_RESET()

#define IMU_SPI_CLK_ENABLE()        __HAL_RCC_SPI5_CLK_ENABLE()
#define IMU_SPI_CLK_DISABLE()       __HAL_RCC_SPI5_CLK_DISABLE()

#define IMU_SPI_SSEL_PORT           (GPIOF)
#define IMU_SPI_SSEL_PIN            (GPIO_PIN_6)

#define IMU_SPI_SCLK_PORT           (GPIOF)
#define IMU_SPI_SCLK_PIN            (GPIO_PIN_7)

#define IMU_SPI_MISO_PORT           (GPIOF)
#define IMU_SPI_MISO_PIN            (GPIO_PIN_8)

#define IMU_SPI_MOSI_PORT           (GPIOF)
#define IMU_SPI_MOSI_PIN            (GPIO_PIN_11)

// SPI LCD Interface
#define OMV_SPI_LCD_CONTROLLER              (&spi_obj[3])
#define OMV_SPI_LCD_CONTROLLER_INSTANCE     (SPI4)

#define OMV_SPI_LCD_MOSI_PIN                (GPIO_PIN_14)
#define OMV_SPI_LCD_MOSI_PORT               (GPIOE)
#define OMV_SPI_LCD_MOSI_ALT                (GPIO_AF5_SPI4)

#define OMV_SPI_LCD_MISO_PIN                (GPIO_PIN_13)
#define OMV_SPI_LCD_MISO_PORT               (GPIOE)
#define OMV_SPI_LCD_MISO_ALT                (GPIO_AF5_SPI4)

#define OMV_SPI_LCD_SCLK_PIN                (GPIO_PIN_12)
#define OMV_SPI_LCD_SCLK_PORT               (GPIOE)
#define OMV_SPI_LCD_SCLK_ALT                (GPIO_AF5_SPI4)

#define OMV_SPI_LCD_CS_PIN                  (GPIO_PIN_11)
#define OMV_SPI_LCD_CS_PORT                 (GPIOE)
#define OMV_SPI_LCD_CS_HIGH()               HAL_GPIO_WritePin(OMV_SPI_LCD_CS_PORT, OMV_SPI_LCD_CS_PIN, GPIO_PIN_SET)
#define OMV_SPI_LCD_CS_LOW()                HAL_GPIO_WritePin(OMV_SPI_LCD_CS_PORT, OMV_SPI_LCD_CS_PIN, GPIO_PIN_RESET)

#define OMV_SPI_LCD_RST_PIN                 (GPIO_PIN_10)
#define OMV_SPI_LCD_RST_PORT                (GPIOC)
#define OMV_SPI_LCD_RST_OFF()               HAL_GPIO_WritePin(OMV_SPI_LCD_RST_PORT, OMV_SPI_LCD_RST_PIN, GPIO_PIN_SET)
#define OMV_SPI_LCD_RST_ON()                HAL_GPIO_WritePin(OMV_SPI_LCD_RST_PORT, OMV_SPI_LCD_RST_PIN, GPIO_PIN_RESET)

#define OMV_SPI_LCD_RS_PIN                  (GPIO_PIN_11)
#define OMV_SPI_LCD_RS_PORT                 (GPIOC)
#define OMV_SPI_LCD_RS_OFF()                HAL_GPIO_WritePin(OMV_SPI_LCD_RS_PORT, OMV_SPI_LCD_RS_PIN, GPIO_PIN_SET)
#define OMV_SPI_LCD_RS_ON()                 HAL_GPIO_WritePin(OMV_SPI_LCD_RS_PORT, OMV_SPI_LCD_RS_PIN, GPIO_PIN_RESET)

//#define OMV_SPI_LCD_BL_PIN                  (GPIO_PIN_4)
//#define OMV_SPI_LCD_BL_PORT                 (GPIOA)
//#define OMV_SPI_LCD_BL_ON()                 HAL_GPIO_WritePin(OMV_SPI_LCD_BL_PORT, OMV_SPI_LCD_BL_PIN, GPIO_PIN_SET)
//#define OMV_SPI_LCD_BL_OFF()                HAL_GPIO_WritePin(OMV_SPI_LCD_BL_PORT, OMV_SPI_LCD_BL_PIN, GPIO_PIN_RESET)

//#define OMV_SPI_LCD_BL_DAC                  (DAC1)
//#define OMV_SPI_LCD_BL_DAC_CHANNEL          (DAC_CHANNEL_1)
//#define OMV_SPI_LCD_BL_DAC_CLK_ENABLE()     __HAL_RCC_DAC12_CLK_ENABLE()
//#define OMV_SPI_LCD_BL_DAC_CLK_DISABLE()    __HAL_RCC_DAC12_CLK_DISABLE()
//#define OMV_SPI_LCD_BL_DAC_FORCE_RESET()    __HAL_RCC_DAC12_FORCE_RESET()
//#define OMV_SPI_LCD_BL_DAC_RELEASE_RESET()  __HAL_RCC_DAC12_RELEASE_RESET()

// FIR Module
#define OMV_ENABLE_FIR_MLX90621             (1)
#define OMV_ENABLE_FIR_MLX90640             (1)
#define OMV_ENABLE_FIR_MLX90641             (1)
#define OMV_ENABLE_FIR_AMG8833              (1)
#define OMV_ENABLE_FIR_LEPTON               (1)

// FIR Lepton
#define OMV_FIR_LEPTON_I2C_BUS              (FIR_I2C_ID)
#define OMV_FIR_LEPTON_I2C_BUS_SPEED        (FIR_I2C_SPEED)
#define OMV_FIR_LEPTON_CONTROLLER           (&spi_obj[3])
#define OMV_FIR_LEPTON_CONTROLLER_INSTANCE  (SPI4)

#define OMV_FIR_LEPTON_MOSI_PIN             (GPIO_PIN_14)
#define OMV_FIR_LEPTON_MOSI_PORT            (GPIOE)
#define OMV_FIR_LEPTON_MOSI_ALT             (GPIO_AF5_SPI4)

#define OMV_FIR_LEPTON_MISO_PIN             (GPIO_PIN_13)
#define OMV_FIR_LEPTON_MISO_PORT            (GPIOE)
#define OMV_FIR_LEPTON_MISO_ALT             (GPIO_AF5_SPI4)

#define OMV_FIR_LEPTON_SCLK_PIN             (GPIO_PIN_12)
#define OMV_FIR_LEPTON_SCLK_PORT            (GPIOE)
#define OMV_FIR_LEPTON_SCLK_ALT             (GPIO_AF5_SPI4)

#define OMV_FIR_LEPTON_CS_PIN               (GPIO_PIN_11)
#define OMV_FIR_LEPTON_CS_PORT              (GPIOE)
#define OMV_FIR_LEPTON_CS_HIGH()            HAL_GPIO_WritePin(OMV_FIR_LEPTON_CS_PORT, OMV_FIR_LEPTON_CS_PIN, GPIO_PIN_SET)
#define OMV_FIR_LEPTON_CS_LOW()             HAL_GPIO_WritePin(OMV_FIR_LEPTON_CS_PORT, OMV_FIR_LEPTON_CS_PIN, GPIO_PIN_RESET)

// Enable additional GPIO banks for DRAM...
#define OMV_ENABLE_GPIO_BANK_F
#define OMV_ENABLE_GPIO_BANK_G
#define OMV_ENABLE_GPIO_BANK_H
#define OMV_ENABLE_GPIO_BANK_I
#define OMV_ENABLE_GPIO_BANK_J
#define OMV_ENABLE_GPIO_BANK_K

#endif //__OMV_BOARDCONFIG_H__

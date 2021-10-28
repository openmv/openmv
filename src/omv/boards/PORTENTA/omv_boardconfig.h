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
#define OMV_ARCH_STR            "PORTENTA H7 8192 SDRAM" // 33 chars max
#define OMV_BOARD_TYPE          "H7"
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

// Sensor PLL register value.
#define OMV_OV7725_PLL_CONFIG   (0x41)  // x4

// Sensor Banding Filter Value
#define OMV_OV7725_BANDING      (0x7F)

// Bootloader LED GPIO port/pin
#define OMV_BOOTLDR_LED_PIN     (GPIO_PIN_6)
#define OMV_BOOTLDR_LED_PORT    (GPIOK)

// RAW buffer size
#define OMV_RAW_BUF_SIZE        (4*1024*1024)

// Enable hardware JPEG
#define OMV_HARDWARE_JPEG       (1)

// Enable MDMA sensor offload.
#define OMV_ENABLE_SENSOR_MDMA  (1)

// Enable sensor drivers
#define OMV_ENABLE_OV2640       (0)
#define OMV_ENABLE_OV5640       (0)
#define OMV_ENABLE_OV7690       (0)
#define OMV_ENABLE_OV7725       (1)
#define OMV_ENABLE_OV9650       (0)
#define OMV_ENABLE_MT9M114      (0)
#define OMV_ENABLE_MT9V034      (1)
#define OMV_ENABLE_LEPTON       (0)
#define OMV_ENABLE_HM01B0       (1)
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
#define OMV_UMM_BLOCK_SIZE      256

// Core VBAT for selftests
#define OMV_CORE_VBAT           "3.0"

// USB IRQn.
#define OMV_USB_IRQN            (OTG_HS_IRQn)
#define OMV_USB_ULPI            (1)
#define OMV_USB_ULPI_DIR_PORT   (GPIOI)
#define OMV_USB_ULPI_DIR_PIN    (11)
#define OMV_USB_ULPI_DIR_CLK_ENABLE()   __HAL_RCC_GPIOI_CLK_ENABLE()

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
#define OMV_OSC_SPI123_CLKSOURCE    RCC_SPI123CLKSOURCE_PLL3

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
#define OMV_DMA_MEMORY          SRAM3       // DMA buffers memory.
#define OMV_FB_MEMORY           DRAM        // Framebuffer, fb_alloc
#define OMV_JPEG_MEMORY         DRAM        // JPEG buffer memory buffer.
#define OMV_JPEG_MEMORY_OFFSET  (7M)        // JPEG buffer is placed after FB/fballoc memory.
#define OMV_VOSPI_MEMORY        SRAM4       // VoSPI buffer memory.
#define OMV_FB_OVERLAY_MEMORY   AXI_SRAM    // Fast fb_alloc memory.
#define OMV_FB_OVERLAY_MEMORY_OFFSET    (480*1024)  // Fast fb_alloc memory size.
#define OMV_CYW43_MEMORY        FLASH_EXT   // CYW43 firmware in external flash mmap'd flash.
#define OMV_CYW43_MEMORY_OFFSET (0x90F00000)// Last Mbyte.

#define OMV_FB_SIZE             (4M)       // FB memory: header + VGA/GS image
#define OMV_FB_ALLOC_SIZE       (3M)       // minimum fb alloc size
#define OMV_STACK_SIZE          (64K)
#define OMV_HEAP_SIZE           (160K)
#define OMV_SDRAM_SIZE          (8 * 1024 * 1024) // This needs to be here for UVC firmware.

#define OMV_LINE_BUF_SIZE       (11 * 1024) // Image line buffer round(2592 * 2BPP * 2 buffers).
#define OMV_MSC_BUF_SIZE        (2K)        // USB MSC bot data
#define OMV_VFS_BUF_SIZE        (1K)        // VFS sturct + FATFS file buffer (624 bytes)
#define OMV_FIR_LEPTON_BUF_SIZE (1K)        // FIR Lepton Packet Double Buffer (328 bytes)
#define OMV_JPEG_BUF_SIZE       (1024*1024) // IDE JPEG buffer (header + data).

#define OMV_BOOT_ORIGIN         0x08000000
#define OMV_BOOT_LENGTH         128K
#define OMV_TEXT_ORIGIN         0x08040000
#define OMV_TEXT_LENGTH         1792K
#define OMV_DTCM_ORIGIN         0x20000000  // Note accessible by CPU and MDMA only.
#define OMV_DTCM_LENGTH         128K
#define OMV_SRAM1_ORIGIN        0x30000000
#define OMV_SRAM1_LENGTH        256K        // SRAM1 + SRAM2
#define OMV_SRAM3_ORIGIN        0x30040000  // Second half of SRAM3 reserved for M4.
#define OMV_SRAM3_LENGTH        16K
#define OMV_SRAM4_ORIGIN        0x38000000
#define OMV_SRAM4_LENGTH        64K
#define OMV_AXI_SRAM_ORIGIN     0x24000000
#define OMV_AXI_SRAM_LENGTH     512K
#define OMV_DRAM_ORIGIN         0xC0000000
#define OMV_DRAM_LENGTH         8M
#define OMV_FLASH_EXT_ORIGIN    0x90000000
#define OMV_FLASH_EXT_LENGTH    16M
#define OMV_CM4_RAM_ORIGIN      0x30044000  // Cortex-M4 memory.
#define OMV_CM4_RAM_LENGTH      16K
#define OMV_CM4_FLASH_ORIGIN    0x08020000
#define OMV_CM4_FLASH_LENGTH    128K

// Domain 1 DMA buffers region.
#define OMV_DMA_MEMORY_D1       AXI_SRAM
#define OMV_DMA_MEMORY_D1_SIZE  (16*1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D1_BASE  (OMV_AXI_SRAM_ORIGIN+OMV_FB_OVERLAY_MEMORY_OFFSET)
#define OMV_DMA_REGION_D1_SIZE  MPU_REGION_SIZE_32KB

// Domain 2 DMA buffers region.
#define OMV_DMA_MEMORY_D2       SRAM3
#define OMV_DMA_MEMORY_D2_SIZE  (1*1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D2_BASE  (OMV_SRAM3_ORIGIN+(0*1024))
#define OMV_DMA_REGION_D2_SIZE  MPU_REGION_SIZE_16KB

// Domain 3 DMA buffers region.
#define OMV_DMA_MEMORY_D3       SRAM4
#define OMV_DMA_MEMORY_D3_SIZE  (32*1024) // Reserved memory for DMA buffers
#define OMV_DMA_REGION_D3_BASE  (OMV_SRAM4_ORIGIN+(0*1024))
#define OMV_DMA_REGION_D3_SIZE  MPU_REGION_SIZE_64KB

// AXI QoS - Low-High (0:15) - default 0
#define OMV_AXI_QOS_MDMA_R_PRI  14 // Max pri to move data.
#define OMV_AXI_QOS_MDMA_W_PRI  15 // Max pri to move data.
#define OMV_AXI_QOS_LTDC_R_PRI  15 // Max pri to read out the frame buffer.

// Image sensor I2C
#define ISC_I2C                 (I2C3)
#define ISC_I2C_ID              (3)
#define ISC_I2C_AF              (GPIO_AF4_I2C3)
#define ISC_I2C_CLK_ENABLE()    __I2C3_CLK_ENABLE()
#define ISC_I2C_CLK_DISABLE()   __I2C3_CLK_DISABLE()
#define ISC_I2C_SCL_PORT        (GPIOH)
#define ISC_I2C_SCL_PIN         (GPIO_PIN_7)
#define ISC_I2C_SDA_PORT        (GPIOH)
#define ISC_I2C_SDA_PIN         (GPIO_PIN_8)
#define ISC_I2C_SPEED           (CAMBUS_SPEED_STANDARD)
#define ISC_I2C_FORCE_RESET()   __HAL_RCC_I2C3_FORCE_RESET()
#define ISC_I2C_RELEASE_RESET() __HAL_RCC_I2C3_RELEASE_RESET()

// Alternate I2C bus for the Portenta breakout
#define ISC_I2C_ALT                 (I2C4)
#define ISC_I2C_ALT_ID              (4)
#define ISC_I2C_ALT_AF              (GPIO_AF4_I2C4)
#define ISC_I2C_ALT_CLK_ENABLE()    __HAL_RCC_I2C4_CLK_ENABLE()
#define ISC_I2C_ALT_CLK_DISABLE()   __HAL_RCC_I2C4_CLK_DISABLE()
#define ISC_I2C_ALT_SCL_PORT        (GPIOH)
#define ISC_I2C_ALT_SCL_PIN         (GPIO_PIN_11)
#define ISC_I2C_ALT_SDA_PORT        (GPIOH)
#define ISC_I2C_ALT_SDA_PIN         (GPIO_PIN_12)
#define ISC_I2C_ALT_SPEED           (CAMBUS_SPEED_STANDARD)
#define ISC_I2C_ALT_FORCE_RESET()   __HAL_RCC_I2C4_FORCE_RESET()
#define ISC_I2C_ALT_RELEASE_RESET() __HAL_RCC_I2C4_RELEASE_RESET()

// FIR I2C
#define FIR_I2C                 (I2C3)
#define FIR_I2C_ID              (3)
#define FIR_I2C_AF              (GPIO_AF4_I2C3)
#define FIR_I2C_CLK_ENABLE()    __I2C3_CLK_ENABLE()
#define FIR_I2C_CLK_DISABLE()   __I2C3_CLK_DISABLE()
#define FIR_I2C_SCL_PORT        (GPIOH)
#define FIR_I2C_SCL_PIN         (GPIO_PIN_7)
#define FIR_I2C_SDA_PORT        (GPIOH)
#define FIR_I2C_SDA_PIN         (GPIO_PIN_8)
#define FIR_I2C_SPEED           (CAMBUS_SPEED_STANDARD)
#define FIR_I2C_FORCE_RESET()   __HAL_RCC_I2C3_FORCE_RESET()
#define FIR_I2C_RELEASE_RESET() __HAL_RCC_I2C3_RELEASE_RESET()

// GPIO.0 is connected to the sensor module reset pin on the Portenta
// breakout board and to the LDO's LDO_ENABLE pin on the Himax shield.
// The sensor probing process will detect the right reset or powerdown
// polarity, so it should be fine to enable it for both boards.
#define DCMI_RESET_PIN          (GPIO_PIN_13)
#define DCMI_RESET_PORT         (GPIOC)

// GPIO.1 is connected to the sensor module frame sync pin (OUTPUT) on
// the Portenta breakout board and to the INT pin (OUTPUT) on the Himax
// shield, so it can't be enabled for the two boards at the same time.
//#define DCMI_FSYNC_PIN          (GPIO_PIN_15)
//#define DCMI_FSYNC_PORT         (GPIOC)

// GPIO.3 is connected to the powerdown pin on the Portenta breakout board,
// and to the STROBE pin on the Himax shield, however it's not actually
// used on the Himax shield and can be safely enable for the two boards.
#define DCMI_PWDN_PIN           (GPIO_PIN_5)
#define DCMI_PWDN_PORT          (GPIOD)

/* DCMI */
#define DCMI_TIM                (TIM1)
#define DCMI_TIM_PIN            (GPIO_PIN_1)
#define DCMI_TIM_PORT           (GPIOK)
// Enable TIM1-CH1 on PA8 too for Portenta breakout.
#define DCMI_TIM_EXT_PIN        (GPIO_PIN_8)
#define DCMI_TIM_EXT_PORT       (GPIOA)
#define DCMI_TIM_AF             (GPIO_AF1_TIM1)
#define DCMI_TIM_CHANNEL        (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()   __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()  __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()    HAL_RCC_GetPCLK2Freq()

#define DCMI_D0_PIN             (GPIO_PIN_9)
#define DCMI_D1_PIN             (GPIO_PIN_10)
#define DCMI_D2_PIN             (GPIO_PIN_11)
#define DCMI_D3_PIN             (GPIO_PIN_12)
#define DCMI_D4_PIN             (GPIO_PIN_14)
#define DCMI_D5_PIN             (GPIO_PIN_4)
#define DCMI_D6_PIN             (GPIO_PIN_6)
#define DCMI_D7_PIN             (GPIO_PIN_7)

#define DCMI_D0_PORT            (GPIOH)
#define DCMI_D1_PORT            (GPIOH)
#define DCMI_D2_PORT            (GPIOH)
#define DCMI_D3_PORT            (GPIOH)
#define DCMI_D4_PORT            (GPIOH)
#define DCMI_D5_PORT            (GPIOI)
#define DCMI_D6_PORT            (GPIOI)
#define DCMI_D7_PORT            (GPIOI)

#define DCMI_HSYNC_PIN          (GPIO_PIN_4)
#define DCMI_VSYNC_PIN          (GPIO_PIN_5)
#define DCMI_PXCLK_PIN          (GPIO_PIN_6)

#define DCMI_HSYNC_PORT         (GPIOA)
#define DCMI_VSYNC_PORT         (GPIOI)
#define DCMI_PXCLK_PORT         (GPIOA)

#if defined(DCMI_RESET_PIN)
#define DCMI_RESET_LOW()        HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_RESET)
#define DCMI_RESET_HIGH()       HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_SET)
#else
#define DCMI_RESET_LOW()
#define DCMI_RESET_HIGH()
#endif

#if defined(DCMI_PWDN_PIN)
#define DCMI_PWDN_LOW()         HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_RESET)
#define DCMI_PWDN_HIGH()        HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_SET)
#else
#define DCMI_PWDN_LOW()
#define DCMI_PWDN_HIGH()
#endif

#if defined(DCMI_FSYNC_PIN)
#define DCMI_FSYNC_LOW()        HAL_GPIO_WritePin(DCMI_FSYNC_PORT, DCMI_FSYNC_PIN, GPIO_PIN_RESET)
#define DCMI_FSYNC_HIGH()       HAL_GPIO_WritePin(DCMI_FSYNC_PORT, DCMI_FSYNC_PIN, GPIO_PIN_SET)
#else
#define DCMI_FSYNC_LOW()
#define DCMI_FSYNC_HIGH()
#endif

#define DCMI_VSYNC_IRQN         EXTI9_5_IRQn
#define DCMI_VSYNC_IRQ_LINE     (7)

#define SOFT_I2C_PORT                GPIOB
#define SOFT_I2C_SIOC_PIN            GPIO_PIN_10
#define SOFT_I2C_SIOD_PIN            GPIO_PIN_11

#define SOFT_I2C_SIOC_H()            HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOC_PIN, GPIO_PIN_SET)
#define SOFT_I2C_SIOC_L()            HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOC_PIN, GPIO_PIN_RESET)

#define SOFT_I2C_SIOD_H()            HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN, GPIO_PIN_SET)
#define SOFT_I2C_SIOD_L()            HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN, GPIO_PIN_RESET)

#define SOFT_I2C_SIOD_READ()         HAL_GPIO_ReadPin (SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN)
#define SOFT_I2C_SIOD_WRITE(bit)     HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN, bit);

#define SOFT_I2C_SPIN_DELAY          64

// SAI4
#define AUDIO_SAI                   (SAI4_Block_A)
// SCKx frequency = SAI_KER_CK / MCKDIV / 2
#define AUDIO_SAI_MCKDIV            (12)
#define AUDIO_SAI_FREQKHZ           (2048U) // 2048KHz
#define AUDIO_MAX_CHANNELS          (2) // Maximum number of channels.

#define AUDIO_SAI_CK_PORT           (GPIOE)
#define AUDIO_SAI_CK_PIN            (GPIO_PIN_2)
#define AUDIO_SAI_CK_AF             (GPIO_AF10_SAI4)

#define AUDIO_SAI_D1_PORT           (GPIOB)
#define AUDIO_SAI_D1_PIN            (GPIO_PIN_2)
#define AUDIO_SAI_D1_AF             (GPIO_AF10_SAI4)

#define AUDIO_SAI_DMA_STREAM        BDMA_Channel1
#define AUDIO_SAI_DMA_REQUEST       BDMA_REQUEST_SAI4_A
#define AUDIO_SAI_DMA_IRQ           BDMA_Channel1_IRQn
#define AUDIO_SAI_DMA_IRQHandler    BDMA_Channel1_IRQHandler

#define AUDIO_SAI_CLK_ENABLE()      __HAL_RCC_SAI4_CLK_ENABLE()
#define AUDIO_SAI_CLK_DISABLE()     __HAL_RCC_SAI4_CLK_DISABLE()
#define AUDIO_SAI_DMA_CLK_ENABLE()  __HAL_RCC_BDMA_CLK_ENABLE()

// SAI1
// Set SAI1 clock source in system ex: Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL;
// #define AUDIO_SAI                   (SAI1_Block_A)
// #define AUDIO_SAI_MCKDIV            (12)
// #define AUDIO_SAI_FREQKHZ           (2048U) // 2048KHz
// #define AUDIO_MAX_CHANNELS          (2) // Maximum number of channels.
//
// #define AUDIO_SAI_CK_PORT           (GPIOE)
// #define AUDIO_SAI_CK_PIN            (GPIO_PIN_2)
// #define AUDIO_SAI_CK_AF             (GPIO_AF2_SAI1)
//
// #define AUDIO_SAI_D1_PORT           (GPIOB)
// #define AUDIO_SAI_D1_PIN            (GPIO_PIN_2)
// #define AUDIO_SAI_D1_AF             (GPIO_AF2_SAI1)
//
// #define AUDIO_SAI_DMA_STREAM        DMA2_Stream6
// #define AUDIO_SAI_DMA_REQUEST       DMA_REQUEST_SAI1_A
// #define AUDIO_SAI_DMA_IRQ           DMA2_Stream6_IRQn
// #define AUDIO_SAI_DMA_IRQHandler    DMA2_Stream6_IRQHandler
//
// #define AUDIO_SAI_CLK_ENABLE()      __HAL_RCC_SAI1_CLK_ENABLE()
// #define AUDIO_SAI_CLK_DISABLE()     __HAL_RCC_SAI1_CLK_DISABLE()
// #define AUDIO_SAI_DMA_CLK_ENABLE()  __HAL_RCC_DMA2_CLK_ENABLE()

// LCD Interface
#define OMV_LCD_CONTROLLER              (LTDC)
#define OMV_LCD_CLK_ENABLE()            __HAL_RCC_LTDC_CLK_ENABLE()
#define OMV_LCD_CLK_DISABLE()           __HAL_RCC_LTDC_CLK_DISABLE()
#define OMV_LCD_FORCE_RESET()           __HAL_RCC_LTDC_FORCE_RESET()
#define OMV_LCD_RELEASE_RESET()         __HAL_RCC_LTDC_RELEASE_RESET()

// DSI Interface
#define OMV_DSI_CONTROLLER              (DSI)
#define OMV_DSI_CLK_ENABLE()            __HAL_RCC_DSI_CLK_ENABLE()
#define OMV_DSI_CLK_DISABLE()           __HAL_RCC_DSI_CLK_DISABLE()
#define OMV_DSI_FORCE_RESET()           __HAL_RCC_DSI_FORCE_RESET()
#define OMV_DSI_RELEASE_RESET()         __HAL_RCC_DSI_RELEASE_RESET()

// SPI LCD Interface
#define OMV_SPI_LCD_CONTROLLER              (&spi_obj[1])
#define OMV_SPI_LCD_CONTROLLER_INSTANCE     (SPI2)

#define OMV_SPI_LCD_MOSI_PIN                (GPIO_PIN_3)
#define OMV_SPI_LCD_MOSI_PORT               (GPIOC)
#define OMV_SPI_LCD_MOSI_ALT                (GPIO_AF5_SPI2)

#define OMV_SPI_LCD_MISO_PIN                (GPIO_PIN_2)
#define OMV_SPI_LCD_MISO_PORT               (GPIOC)
#define OMV_SPI_LCD_MISO_ALT                (GPIO_AF5_SPI2)

#define OMV_SPI_LCD_SCLK_PIN                (GPIO_PIN_1)
#define OMV_SPI_LCD_SCLK_PORT               (GPIOI)
#define OMV_SPI_LCD_SCLK_ALT                (GPIO_AF5_SPI2)

#define OMV_SPI_LCD_RST_PIN                 (GPIO_PIN_15)
#define OMV_SPI_LCD_RST_PORT                (GPIOH)
#define OMV_SPI_LCD_RST_OFF()               HAL_GPIO_WritePin(OMV_SPI_LCD_RST_PORT, OMV_SPI_LCD_RST_PIN, GPIO_PIN_SET)
#define OMV_SPI_LCD_RST_ON()                HAL_GPIO_WritePin(OMV_SPI_LCD_RST_PORT, OMV_SPI_LCD_RST_PIN, GPIO_PIN_RESET)

#define OMV_SPI_LCD_RS_PIN                  (GPIO_PIN_1)
#define OMV_SPI_LCD_RS_PORT                 (GPIOK)
#define OMV_SPI_LCD_RS_OFF()                HAL_GPIO_WritePin(OMV_SPI_LCD_RS_PORT, OMV_SPI_LCD_RS_PIN, GPIO_PIN_SET)
#define OMV_SPI_LCD_RS_ON()                 HAL_GPIO_WritePin(OMV_SPI_LCD_RS_PORT, OMV_SPI_LCD_RS_PIN, GPIO_PIN_RESET)

#define OMV_SPI_LCD_CS_PIN                  (GPIO_PIN_0)
#define OMV_SPI_LCD_CS_PORT                 (GPIOI)
#define OMV_SPI_LCD_CS_HIGH()               HAL_GPIO_WritePin(OMV_SPI_LCD_CS_PORT, OMV_SPI_LCD_CS_PIN, GPIO_PIN_SET)
#define OMV_SPI_LCD_CS_LOW()                HAL_GPIO_WritePin(OMV_SPI_LCD_CS_PORT, OMV_SPI_LCD_CS_PIN, GPIO_PIN_RESET)

#define OMV_SPI_LCD_BL_PIN                  (GPIO_PIN_4)
#define OMV_SPI_LCD_BL_PORT                 (GPIOA)
#define OMV_SPI_LCD_BL_ON()                 HAL_GPIO_WritePin(OMV_SPI_LCD_BL_PORT, OMV_SPI_LCD_BL_PIN, GPIO_PIN_SET)
#define OMV_SPI_LCD_BL_OFF()                HAL_GPIO_WritePin(OMV_SPI_LCD_BL_PORT, OMV_SPI_LCD_BL_PIN, GPIO_PIN_RESET)

#define OMV_SPI_LCD_BL_DAC                  (DAC1)
#define OMV_SPI_LCD_BL_DAC_CHANNEL          (DAC_CHANNEL_1)
#define OMV_SPI_LCD_BL_DAC_CLK_ENABLE()     __HAL_RCC_DAC12_CLK_ENABLE()
#define OMV_SPI_LCD_BL_DAC_CLK_DISABLE()    __HAL_RCC_DAC12_CLK_DISABLE()
#define OMV_SPI_LCD_BL_DAC_FORCE_RESET()    __HAL_RCC_DAC12_FORCE_RESET()
#define OMV_SPI_LCD_BL_DAC_RELEASE_RESET()  __HAL_RCC_DAC12_RELEASE_RESET()

// FIR Module
#define OMV_ENABLE_FIR_MLX90621             (1)
#define OMV_ENABLE_FIR_MLX90640             (1)
#define OMV_ENABLE_FIR_MLX90641             (1)
#define OMV_ENABLE_FIR_AMG8833              (1)
#define OMV_ENABLE_FIR_LEPTON               (1)

// FIR Lepton
#define OMV_FIR_LEPTON_I2C_BUS              (FIR_I2C_ID)
#define OMV_FIR_LEPTON_I2C_BUS_SPEED        (FIR_I2C_SPEED)
#define OMV_FIR_LEPTON_CONTROLLER           (&spi_obj[1])
#define OMV_FIR_LEPTON_CONTROLLER_INSTANCE  (SPI2)

#define OMV_FIR_LEPTON_MOSI_PIN             (GPIO_PIN_3)
#define OMV_FIR_LEPTON_MOSI_PORT            (GPIOC)
#define OMV_FIR_LEPTON_MOSI_ALT             (GPIO_AF5_SPI2)

#define OMV_FIR_LEPTON_MISO_PIN             (GPIO_PIN_2)
#define OMV_FIR_LEPTON_MISO_PORT            (GPIOC)
#define OMV_FIR_LEPTON_MISO_ALT             (GPIO_AF5_SPI2)

#define OMV_FIR_LEPTON_SCLK_PIN             (GPIO_PIN_1)
#define OMV_FIR_LEPTON_SCLK_PORT            (GPIOI)
#define OMV_FIR_LEPTON_SCLK_ALT             (GPIO_AF5_SPI2)

#define OMV_FIR_LEPTON_CS_PIN               (GPIO_PIN_0)
#define OMV_FIR_LEPTON_CS_PORT              (GPIOI)
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

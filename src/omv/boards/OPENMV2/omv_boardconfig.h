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
#define OMV_ARCH_STR            "OMV2 F4 256 JPEG" // 33 chars max
#define OMV_BOARD_TYPE          "M4"
#define OMV_UNIQUE_ID_ADDR      0x1FFF7A10
#define OMV_UNIQUE_ID_SIZE      3 // 3 words

// Needed by the SWD JTAG testrig - located at the bottom of the frame buffer stack.
#define OMV_SELF_TEST_SWD_ADDR  MAIN_FB()->pixfmt

// Flash sectors for the bootloader.
// Flash FS sector, main FW sector, max sector.
#define OMV_FLASH_LAYOUT        {1, 4, 11}

#define OMV_XCLK_MCO            (0U)
#define OMV_XCLK_TIM            (1U)

// Sensor external clock source.
#define OMV_XCLK_SOURCE         (OMV_XCLK_TIM)

// Sensor external clock timer frequency.
#define OMV_XCLK_FREQUENCY      (6000000)

// Sensor PLL register value.
#define OMV_OV7725_PLL_CONFIG   (0x41)  // x4

// Sensor Banding Filter Value
#define OMV_OV7725_BANDING      (0x3F)

// Bootloader LED GPIO port/pin
#define OMV_BOOTLDR_LED_PIN     (GPIO_PIN_2)
#define OMV_BOOTLDR_LED_PORT    (GPIOC)

// RAW buffer size
#define OMV_RAW_BUF_SIZE        (153600)

// Enable sensor drivers
#define OMV_ENABLE_OV2640       (1)
#define OMV_ENABLE_OV5640       (0)
#define OMV_ENABLE_OV7690       (0)
#define OMV_ENABLE_OV7725       (1)
#define OMV_ENABLE_OV9650       (0)
#define OMV_ENABLE_MT9M114      (0)
#define OMV_ENABLE_MT9V034      (0)
#define OMV_ENABLE_LEPTON       (0)
#define OMV_ENABLE_HM01B0       (0)
#define OMV_ENABLE_PAJ6100      (0)

// Enable sensor features
#define OMV_ENABLE_OV5640_AF    (0)

// Enable self-tests on first boot
#define OMV_ENABLE_SELFTEST     (1)

// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH     (160*120*2)

// Low and high JPEG QS.
#define JPEG_QUALITY_LOW        35
#define JPEG_QUALITY_HIGH       60

// FB Heap Block Size
#define OMV_UMM_BLOCK_SIZE      16

// Core VBAT for selftests
#define OMV_CORE_VBAT           "3.3"

// USB IRQn.
#define OMV_USB_IRQN            (OTG_FS_IRQn)

//PLL1 192MHz/48MHz
#define OMV_OSC_PLL1M           (12)
#define OMV_OSC_PLL1N           (384)
#define OMV_OSC_PLL1P           (2)
#define OMV_OSC_PLL1Q           (8)

// HSE/HSI/CSI State
#define OMV_OSC_HSE_STATE       (RCC_HSE_ON)

// Clock Sources
#define OMV_OSC_PLL_CLKSOURCE   RCC_PLLSOURCE_HSE

// Flash Latency
#define OMV_FLASH_LATENCY       (FLASH_LATENCY_7)

// Linker script constants (see the linker script template stm32fxxx.ld.S).
// Note: fb_alloc is a stack-based, dynamically allocated memory on FB.
// The maximum available fb_alloc memory = FB_ALLOC_SIZE + FB_SIZE - (w*h*bpp).
// Note: fb_alloc overwrites the line buffer which is only used during readout.
#define OMV_FB_MEMORY       SRAM1   // Framebuffer, fb_alloc
#define OMV_MAIN_MEMORY     DTCM    // data, bss and heap memory
#define OMV_STACK_MEMORY    DTCM    // stack memory
#define OMV_DMA_MEMORY      SRAM2   // Misc DMA buffers

#define OMV_FB_SIZE         (150K)  // FB memory: header + QVGA/GS image
#define OMV_FB_ALLOC_SIZE   (12K)   // minimum fb alloc size
#define OMV_STACK_SIZE      (4K)
#define OMV_HEAP_SIZE       (51K)

#define OMV_LINE_BUF_SIZE   (2 * 1024)  // Image line buffer round(320 * 2BPP * 2 buffers).
#define OMV_MSC_BUF_SIZE    (2K)    // USB MSC bot data
#define OMV_VFS_BUF_SIZE    (1K)    // VFS sturct + FATFS file buffer (624 bytes)
#define OMV_FIR_LEPTON_BUF_SIZE (1K) // FIR Lepton Packet Double Buffer (328 bytes)
#define OMV_FFS_BUF_SIZE    (16K)   // Flash filesystem cache
#define OMV_JPEG_BUF_SIZE   (8 * 1024)  // IDE JPEG buffer size (header + data).

#define OMV_BOOT_ORIGIN     0x08000000
#define OMV_BOOT_LENGTH     16K
#define OMV_TEXT_ORIGIN     0x08010000
#define OMV_TEXT_LENGTH     960K
#define OMV_DTCM_ORIGIN     0x10000000
#define OMV_DTCM_LENGTH     64K
#define OMV_SRAM1_ORIGIN    0x20000000
#define OMV_SRAM1_LENGTH    162K
#define OMV_SRAM2_ORIGIN    0x20028800
#define OMV_SRAM2_LENGTH    30K

// Image sensor I2C
#define ISC_I2C                 (I2C1)
#define ISC_I2C_ID              (1)
#define ISC_I2C_AF              (GPIO_AF4_I2C1)
#define ISC_I2C_CLK_ENABLE()    __I2C1_CLK_ENABLE()
#define ISC_I2C_CLK_DISABLE()   __I2C1_CLK_DISABLE()
#define ISC_I2C_SCL_PORT        (GPIOB)
#define ISC_I2C_SCL_PIN         (GPIO_PIN_8)
#define ISC_I2C_SDA_PORT        (GPIOB)
#define ISC_I2C_SDA_PIN         (GPIO_PIN_9)
#define ISC_I2C_SPEED           (CAMBUS_SPEED_STANDARD)
#define ISC_I2C_FORCE_RESET()   __HAL_RCC_I2C1_FORCE_RESET()
#define ISC_I2C_RELEASE_RESET() __HAL_RCC_I2C1_RELEASE_RESET()

// FIR I2C
#define FIR_I2C                 (I2C2)
#define FIR_I2C_ID              (2)
#define FIR_I2C_AF              (GPIO_AF4_I2C2)
#define FIR_I2C_CLK_ENABLE()    __I2C2_CLK_ENABLE()
#define FIR_I2C_CLK_DISABLE()   __I2C2_CLK_DISABLE()
#define FIR_I2C_SCL_PORT        (GPIOB)
#define FIR_I2C_SCL_PIN         (GPIO_PIN_10)
#define FIR_I2C_SDA_PORT        (GPIOB)
#define FIR_I2C_SDA_PIN         (GPIO_PIN_11)
#define FIR_I2C_SPEED           (CAMBUS_SPEED_FULL)
#define FIR_I2C_FORCE_RESET()   __HAL_RCC_I2C2_FORCE_RESET()
#define FIR_I2C_RELEASE_RESET() __HAL_RCC_I2C2_RELEASE_RESET()

/* DCMI */
#define DCMI_TIM                (TIM1)
#define DCMI_TIM_PIN            (GPIO_PIN_8)
#define DCMI_TIM_PORT           (GPIOA)
#define DCMI_TIM_AF             (GPIO_AF1_TIM1)
#define DCMI_TIM_CHANNEL        (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()   __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()  __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()    HAL_RCC_GetPCLK2Freq()

#define DCMI_RESET_PIN          (GPIO_PIN_10)
#define DCMI_RESET_PORT         (GPIOA)

#define DCMI_PWDN_PIN           (GPIO_PIN_5)
#define DCMI_PWDN_PORT          (GPIOB)

#define DCMI_D0_PIN             (GPIO_PIN_6)
#define DCMI_D1_PIN             (GPIO_PIN_7)
#define DCMI_D2_PIN             (GPIO_PIN_0)
#define DCMI_D3_PIN             (GPIO_PIN_1)
#define DCMI_D4_PIN             (GPIO_PIN_4)
#define DCMI_D5_PIN             (GPIO_PIN_6)
#define DCMI_D6_PIN             (GPIO_PIN_5)
#define DCMI_D7_PIN             (GPIO_PIN_6)

#define DCMI_D0_PORT            (GPIOC)
#define DCMI_D1_PORT            (GPIOC)
#define DCMI_D2_PORT            (GPIOE)
#define DCMI_D3_PORT            (GPIOE)
#define DCMI_D4_PORT            (GPIOE)
#define DCMI_D5_PORT            (GPIOB)
#define DCMI_D6_PORT            (GPIOE)
#define DCMI_D7_PORT            (GPIOE)

#define DCMI_HSYNC_PIN          (GPIO_PIN_4)
#define DCMI_VSYNC_PIN          (GPIO_PIN_7)
#define DCMI_PXCLK_PIN          (GPIO_PIN_6)

#define DCMI_HSYNC_PORT         (GPIOA)
#define DCMI_VSYNC_PORT         (GPIOB)
#define DCMI_PXCLK_PORT         (GPIOA)

#define DCMI_RESET_LOW()        HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_RESET)
#define DCMI_RESET_HIGH()       HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_SET)

#define DCMI_PWDN_LOW()         HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_RESET)
#define DCMI_PWDN_HIGH()        HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_SET)

#define DCMI_VSYNC_IRQN         EXTI9_5_IRQn
#define DCMI_VSYNC_IRQ_LINE     (7)

#define WINC_SPI                (SPI2)
#define WINC_SPI_AF             (GPIO_AF5_SPI2)
#define WINC_SPI_TIMEOUT        (1000)
#define WINC_SPI_PRESCALER      (SPI_BAUDRATEPRESCALER_2)
#define WINC_SPI_CLK_ENABLE()   __HAL_RCC_SPI2_CLK_ENABLE()

#define WINC_SPI_SCLK_PIN       (GPIO_PIN_13)
#define WINC_SPI_MISO_PIN       (GPIO_PIN_14)
#define WINC_SPI_MOSI_PIN       (GPIO_PIN_15)

#define WINC_SPI_SCLK_PORT      (GPIOB)
#define WINC_SPI_MISO_PORT      (GPIOB)
#define WINC_SPI_MOSI_PORT      (GPIOB)

#define WINC_EN_PIN             (GPIO_PIN_5)
#define WINC_CS_PIN             (GPIO_PIN_12)
#define WINC_RST_PIN            (GPIO_PIN_12)
#define WINC_IRQ_PIN            (pin_D13)

#define WINC_EN_PORT            (GPIOA)
#define WINC_CS_PORT            (GPIOB)
#define WINC_RST_PORT           (GPIOD)

#define WINC_CS_LOW()           HAL_GPIO_WritePin(WINC_CS_PORT, WINC_CS_PIN, GPIO_PIN_RESET)
#define WINC_CS_HIGH()          HAL_GPIO_WritePin(WINC_CS_PORT, WINC_CS_PIN, GPIO_PIN_SET)

#define SOFT_I2C_PORT                GPIOB
#define SOFT_I2C_SIOC_PIN            GPIO_PIN_10
#define SOFT_I2C_SIOD_PIN            GPIO_PIN_11

#define SOFT_I2C_SIOC_H()            HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOC_PIN, GPIO_PIN_SET)
#define SOFT_I2C_SIOC_L()            HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOC_PIN, GPIO_PIN_RESET)

#define SOFT_I2C_SIOD_H()            HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN, GPIO_PIN_SET)
#define SOFT_I2C_SIOD_L()            HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN, GPIO_PIN_RESET)

#define SOFT_I2C_SIOD_READ()         HAL_GPIO_ReadPin (SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN)
#define SOFT_I2C_SIOD_WRITE(bit)     HAL_GPIO_WritePin(SOFT_I2C_PORT, SOFT_I2C_SIOD_PIN, bit);

#define SOFT_I2C_SPIN_DELAY          16

// SPI LCD Interface
#define OMV_SPI_LCD_CONTROLLER              (&spi_obj[1])
#define OMV_SPI_LCD_CONTROLLER_INSTANCE     (SPI2)

#define OMV_SPI_LCD_MOSI_PIN                (GPIO_PIN_15)
#define OMV_SPI_LCD_MOSI_PORT               (GPIOB)
#define OMV_SPI_LCD_MOSI_ALT                (GPIO_AF5_SPI2)

#define OMV_SPI_LCD_MISO_PIN                (GPIO_PIN_14)
#define OMV_SPI_LCD_MISO_PORT               (GPIOB)
#define OMV_SPI_LCD_MISO_ALT                (GPIO_AF5_SPI2)

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

#define OMV_SPI_LCD_BL_DAC                  (DAC)
#define OMV_SPI_LCD_BL_DAC_CHANNEL          (DAC_CHANNEL_2)
#define OMV_SPI_LCD_BL_DAC_CLK_ENABLE()     __HAL_RCC_DAC_CLK_ENABLE()
#define OMV_SPI_LCD_BL_DAC_CLK_DISABLE()    __HAL_RCC_DAC_CLK_DISABLE()
#define OMV_SPI_LCD_BL_DAC_FORCE_RESET()    __HAL_RCC_DAC_FORCE_RESET()
#define OMV_SPI_LCD_BL_DAC_RELEASE_RESET()  __HAL_RCC_DAC_RELEASE_RESET()

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

#define OMV_FIR_LEPTON_MOSI_PIN             (GPIO_PIN_15)
#define OMV_FIR_LEPTON_MOSI_PORT            (GPIOB)
#define OMV_FIR_LEPTON_MOSI_ALT             (GPIO_AF5_SPI2)

#define OMV_FIR_LEPTON_MISO_PIN             (GPIO_PIN_14)
#define OMV_FIR_LEPTON_MISO_PORT            (GPIOB)
#define OMV_FIR_LEPTON_MISO_ALT             (GPIO_AF5_SPI2)

#define OMV_FIR_LEPTON_SCLK_PIN             (GPIO_PIN_13)
#define OMV_FIR_LEPTON_SCLK_PORT            (GPIOB)
#define OMV_FIR_LEPTON_SCLK_ALT             (GPIO_AF5_SPI2)

#define OMV_FIR_LEPTON_CS_PIN               (GPIO_PIN_12)
#define OMV_FIR_LEPTON_CS_PORT              (GPIOB)
#define OMV_FIR_LEPTON_CS_HIGH()            HAL_GPIO_WritePin(OMV_FIR_LEPTON_CS_PORT, OMV_FIR_LEPTON_CS_PIN, GPIO_PIN_SET)
#define OMV_FIR_LEPTON_CS_LOW()             HAL_GPIO_WritePin(OMV_FIR_LEPTON_CS_PORT, OMV_FIR_LEPTON_CS_PIN, GPIO_PIN_RESET)

#endif //__OMV_BOARDCONFIG_H__

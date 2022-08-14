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

// Sensor external clock frequency.
#define OMV_XCLK_FREQUENCY      (12000000)
// Max integral image.
#define OMV_MAX_INT_FRAME       FRAMESIZE_QCIF
#define OMV_MAX_INT_FRAME_STR   "QCIF"
// Max GS/RGB565/YUV422/Binary image.
#define OMV_MAX_RAW_FRAME       FRAMESIZE_QCIF
#define OMV_MAX_RAW_FRAME_STR   "QCIF"
// Max raw (RGB565/YUV422) image for blob detection.
#define OMV_MAX_BLOB_FRAME      FRAMESIZE_QCIF
#define OMV_MAX_BLOB_FRAME_STR  "QCIF"

// Core VBAT for selftests
#define OMV_CORE_VBAT           "3.3"

// USB IRQn.
#define OMV_USB_IRQN            (OTG_FS_IRQn)

//PLL1 168MHz/48MHz
#define OMV_OSC_PLL1M           (12)
#define OMV_OSC_PLL1N           (336)
#define OMV_OSC_PLL1P           (2)
#define OMV_OSC_PLL1Q           (7)

// HSE/HSI/CSI State
#define OMV_OSC_HSE_STATE       (RCC_HSE_ON)

// Clock Sources
#define OMV_OSC_PLL_CLKSOURCE   RCC_PLLSOURCE_HSE

// Flash Latency
#define OMV_FLASH_LATENCY       (FLASH_LATENCY_5)

// Linker script constants (see the linker script template stm32fxxx.ld.S).
#define OMV_TEXT_ORIGIN     0x08010000
#define OMV_TEXT_LENGTH     448K
#define OMV_SRAM1_ORIGIN    0x20000000
#define OMV_SRAM1_LENGTH    128K
#define OMV_DTCM_ORIGIN      0x10000000
#define OMV_DTCM_LENGTH      64K

/* SCCB/I2C */
#define SCCB_I2C                (I2C1)
#define SCCB_AF                 (GPIO_AF4_I2C1)
#define SCCB_CLK_ENABLE()       __I2C1_CLK_ENABLE()
#define SCCB_CLK_DISABLE()      __I2C1_CLK_DISABLE()
#define SCCB_PORT               (GPIOB)
#define SCCB_SCL_PIN            (GPIO_PIN_8)
#define SCCB_SDA_PIN            (GPIO_PIN_9)

/* DCMI */
#define DCMI_TIM                (TIM1)
#define DCMI_TIM_PIN            (GPIO_PIN_9)
#define DCMI_TIM_PORT           (GPIOE)
#define DCMI_TIM_AF             (GPIO_AF1_TIM1)
#define DCMI_TIM_CHANNEL        (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()   __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()  __TIM1_CLK_DISABLE()

#define DCMI_RESET_PIN          (GPIO_PIN_10)
#define DCMI_RESET_PORT         (GPIOA)

#define DCMI_PWDN_PIN           (GPIO_PIN_5)
#define DCMI_PWDN_PORT          (GPIOB)

#define DCMI_D0_PIN             (GPIO_PIN_6)
#define DCMI_D1_PIN             (GPIO_PIN_7)
#define DCMI_D2_PIN             (GPIO_PIN_0)
#define DCMI_D3_PIN             (GPIO_PIN_1)
#define DCMI_D4_PIN             (GPIO_PIN_4)
#define DCMI_D5_PIN             (GPIO_PIN_5)
#define DCMI_D6_PIN             (GPIO_PIN_6)
#define DCMI_D7_PIN             (GPIO_PIN_6)

#define DCMI_D0_PORT            (GPIOC)
#define DCMI_D1_PORT            (GPIOC)
#define DCMI_D2_PORT            (GPIOE)
#define DCMI_D3_PORT            (GPIOE)
#define DCMI_D4_PORT            (GPIOE)
#define DCMI_D5_PORT            (GPIOE)
#define DCMI_D6_PORT            (GPIOE)
#define DCMI_D7_PORT            (GPIOB)

#define DCMI_HSYNC_PIN          (GPIO_PIN_7)
#define DCMI_VSYNC_PIN          (GPIO_PIN_4)
#define DCMI_PXCLK_PIN          (GPIO_PIN_6)

#define DCMI_HSYNC_PORT         (GPIOB)
#define DCMI_VSYNC_PORT         (GPIOA)
#define DCMI_PXCLK_PORT         (GPIOA)

#define DCMI_RESET_LOW()        HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_RESET)
#define DCMI_RESET_HIGH()       HAL_GPIO_WritePin(DCMI_RESET_PORT, DCMI_RESET_PIN, GPIO_PIN_SET)

#define DCMI_PWDN_LOW()         HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_RESET)
#define DCMI_PWDN_HIGH()        HAL_GPIO_WritePin(DCMI_PWDN_PORT, DCMI_PWDN_PIN, GPIO_PIN_SET)

/* uSD */
#define SD_SPI                  (SPI2)
#define SD_SPI_AF               (GPIO_AF5_SPI2)
#define SD_CD_PIN               (GPIO_PIN_0)
#define SD_CS_PIN               (GPIO_PIN_1)
#define SD_SCLK_PIN             (GPIO_PIN_13)
#define SD_MISO_PIN             (GPIO_PIN_2)
#define SD_MOSI_PIN             (GPIO_PIN_3)

#define SD_CD_PORT              (GPIOC)
#define SD_CS_PORT              (GPIOC)
#define SD_SCLK_PORT            (GPIOB)
#define SD_MISO_PORT            (GPIOC)
#define SD_MOSI_PORT            (GPIOC)

#define SD_SPI_CLK_ENABLE()     __SPI2_CLK_ENABLE()
#define SD_SPI_CLK_DISABLE()    __SPI2_CLK_DISABLE()

#define SD_SELECT()             HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_RESET)
#define SD_DESELECT()           HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_SET)

#endif //__OMV_BOARDCONFIG_H__

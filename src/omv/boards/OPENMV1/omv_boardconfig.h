/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Board configuration and pin definitions.
 */
#ifndef __OMV_BOARDCONFIG_H__
#define __OMV_BOARDCONFIG_H__

// Max integral image.
#define OMV_MAX_INT_FRAME         OMV_CSI_FRAMESIZE_QCIF
#define OMV_MAX_INT_FRAME_STR     "QCIF"
// Max GS/RGB565/YUV422/Binary image.
#define OMV_MAX_RAW_FRAME         OMV_CSI_FRAMESIZE_QCIF
#define OMV_MAX_RAW_FRAME_STR     "QCIF"
// Max raw (RGB565/YUV422) image for blob detection.
#define OMV_MAX_BLOB_FRAME        OMV_CSI_FRAMESIZE_QCIF
#define OMV_MAX_BLOB_FRAME_STR    "QCIF"

// USB IRQn.
#define OMV_USB_IRQN              (OTG_FS_IRQn)

//PLL1 168MHz/48MHz
#define OMV_OSC_PLL1M             (12)
#define OMV_OSC_PLL1N             (336)
#define OMV_OSC_PLL1P             (2)
#define OMV_OSC_PLL1Q             (7)

// HSE/HSI/CSI State
#define OMV_OSC_HSE_STATE         (RCC_HSE_ON)

// Clock Sources
#define OMV_OSC_PLL_CLKSOURCE     RCC_PLLSOURCE_HSE

// Flash Latency
#define OMV_FLASH_LATENCY         (FLASH_LATENCY_5)

// Memory map.
#define OMV_FLASH_ORIGIN          0x08000000
#define OMV_FLASH_LENGTH          512K
#define OMV_SRAM1_ORIGIN          0x20000000
#define OMV_SRAM1_LENGTH          128K
#define OMV_DTCM_ORIGIN           0x10000000
#define OMV_DTCM_LENGTH           64K

// Flash configuration.
#define OMV_FLASH_FFS_ORIGIN      0x08004000
#define OMV_FLASH_FFS_LENGTH      48K
#define OMV_FLASH_TXT_ORIGIN      0x08010000
#define OMV_FLASH_TXT_LENGTH      448K

/* SCCB/I2C */
#define SCCB_I2C                  (I2C1)
#define SCCB_AF                   (GPIO_AF4_I2C1)
#define SCCB_CLK_ENABLE()         __I2C1_CLK_ENABLE()
#define SCCB_CLK_DISABLE()        __I2C1_CLK_DISABLE()
#define SCCB_PORT                 (GPIOB)
#define SCCB_SCL_PIN              (GPIO_PIN_8)
#define SCCB_SDA_PIN              (GPIO_PIN_9)

// Camera interface
#define OMV_CSI_CLK_FREQUENCY     (12000000)
#define OMV_CSI_TIM               (TIM1)
#define OMV_CSI_TIM_PIN           (GPIO_PIN_9)
#define OMV_CSI_TIM_PORT          (GPIOE)
#define OMV_CSI_TIM_AF            (GPIO_AF1_TIM1)
#define OMV_CSI_TIM_CHANNEL       (TIM_CHANNEL_1)
#define OMV_CSI_TIM_CLK_ENABLE()  __TIM1_CLK_ENABLE()
#define OMV_CSI_TIM_CLK_DISABLE() __TIM1_CLK_DISABLE()
#define OMV_CSI_TIM_CLK_SLEEP_ENABLE() __TIM1_CLK_SLEEP_ENABLE()
#define OMV_CSI_HW_CROP_ENABLE    (1)

#define OMV_CSI_RESET_PIN         (GPIO_PIN_10)
#define OMV_CSI_RESET_PORT        (GPIOA)

#define OMV_CSI_PWDN_PIN          (GPIO_PIN_5)
#define OMV_CSI_PWDN_PORT         (GPIOB)

#define OMV_CSI_D0_PIN            (GPIO_PIN_6)
#define OMV_CSI_D1_PIN            (GPIO_PIN_7)
#define OMV_CSI_D2_PIN            (GPIO_PIN_0)
#define OMV_CSI_D3_PIN            (GPIO_PIN_1)
#define OMV_CSI_D4_PIN            (GPIO_PIN_4)
#define OMV_CSI_D5_PIN            (GPIO_PIN_5)
#define OMV_CSI_D6_PIN            (GPIO_PIN_6)
#define OMV_CSI_D7_PIN            (GPIO_PIN_6)

#define OMV_CSI_D0_PORT           (GPIOC)
#define OMV_CSI_D1_PORT           (GPIOC)
#define OMV_CSI_D2_PORT           (GPIOE)
#define OMV_CSI_D3_PORT           (GPIOE)
#define OMV_CSI_D4_PORT           (GPIOE)
#define OMV_CSI_D5_PORT           (GPIOE)
#define OMV_CSI_D6_PORT           (GPIOE)
#define OMV_CSI_D7_PORT           (GPIOB)

#define OMV_CSI_HSYNC_PIN         (GPIO_PIN_7)
#define OMV_CSI_VSYNC_PIN         (GPIO_PIN_4)
#define OMV_CSI_PXCLK_PIN         (GPIO_PIN_6)

#define OMV_CSI_HSYNC_PORT        (GPIOB)
#define OMV_CSI_VSYNC_PORT        (GPIOA)
#define OMV_CSI_PXCLK_PORT        (GPIOA)

#define OMV_CSI_RESET_LOW()       HAL_GPIO_WritePin(OMV_CSI_RESET_PORT, OMV_CSI_RESET_PIN, GPIO_PIN_RESET)
#define OMV_CSI_RESET_HIGH()      HAL_GPIO_WritePin(OMV_CSI_RESET_PORT, OMV_CSI_RESET_PIN, GPIO_PIN_SET)

#define OMV_CSI_PWDN_LOW()        HAL_GPIO_WritePin(OMV_CSI_PWDN_PORT, OMV_CSI_PWDN_PIN, GPIO_PIN_RESET)
#define OMV_CSI_PWDN_HIGH()       HAL_GPIO_WritePin(OMV_CSI_PWDN_PORT, OMV_CSI_PWDN_PIN, GPIO_PIN_SET)

/* uSD */
#define SD_SPI                    (SPI2)
#define SD_SPI_AF                 (GPIO_AF5_SPI2)
#define SD_CD_PIN                 (GPIO_PIN_0)
#define SD_CS_PIN                 (GPIO_PIN_1)
#define SD_SCLK_PIN               (GPIO_PIN_13)
#define SD_MISO_PIN               (GPIO_PIN_2)
#define SD_MOSI_PIN               (GPIO_PIN_3)

#define SD_CD_PORT                (GPIOC)
#define SD_CS_PORT                (GPIOC)
#define SD_SCLK_PORT              (GPIOB)
#define SD_MISO_PORT              (GPIOC)
#define SD_MOSI_PORT              (GPIOC)

#define SD_SPI_CLK_ENABLE()       __SPI2_CLK_ENABLE()
#define SD_SPI_CLK_DISABLE()      __SPI2_CLK_DISABLE()

#define SD_SELECT()               HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_RESET)
#define SD_DESELECT()             HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_SET)

#endif //__OMV_BOARDCONFIG_H__

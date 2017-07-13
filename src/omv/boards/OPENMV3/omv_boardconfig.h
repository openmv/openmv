/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Board configuration and pin definitions.
 *
 */
#ifndef __OMV_BOARDCONFIG_H__
#define __OMV_BOARDCONFIG_H__

// Architecture info
#define OMV_ARCH_STR            "OMV3 F7 512" // 33 chars max
#define OMV_BOARD_TYPE          "M7"
#define OMV_UNIQUE_ID_ADDR      0x1FF0F420

#define OMV_XCLK_MCO            (0U)
#define OMV_XCLK_TIM            (1U)

// Sensor external clock source.
#define OMV_XCLK_SOURCE         (OMV_XCLK_TIM)

// Sensor external clock timer frequency.
#define OMV_XCLK_FREQUENCY      (9000000)

// Have built-in RGB->LAB table.
#define OMV_HAVE_LAB_TABLE

// Enable Find_Circles
#define OMV_ENABLE_FIND_CIRCLES

// Enable Find_Rects
#define OMV_ENABLE_FIND_RECTS

// Enable AprilTags (64 KB).
#define OMV_ENABLE_APRILTAGS

// Enable DataMatrices (26 KB).
#define OMV_ENABLE_DATAMATRICES

// Enable Barcodes (42 KB).
#define OMV_ENABLE_BARCODES

// Enable LENET (200+ KB).
#define OMV_ENABLE_LENET

// Bootloader LED GPIO port/pin
#define OMV_BOOTLDR_LED_PIN     (GPIO_PIN_1)
#define OMV_BOOTLDR_LED_PORT    (GPIOC)

// RAW buffer size
#define OMV_RAW_BUF_SIZE        (307200)

// JPEG buffer size (header + data)
#define OMV_JPEG_BUF_SIZE       (23000)

// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH     (160*120*2)

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

#define DCMI_FREX_PIN           (GPIO_PIN_9)
#define DCMI_FREX_PORT          (GPIOD)

#define DCMI_EXPST_PIN          (GPIO_PIN_8)
#define DCMI_EXPST_PORT         (GPIOD)

#define DCMI_FSIN_PIN           (GPIO_PIN_3)
#define DCMI_FSIN_PORT          (GPIOD)

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

#define DCMI_FREX_LOW()         HAL_GPIO_WritePin(DCMI_FREX_PORT, DCMI_FREX_PIN, GPIO_PIN_RESET)
#define DCMI_FREX_HIGH()        HAL_GPIO_WritePin(DCMI_FREX_PORT, DCMI_FREX_PIN, GPIO_PIN_SET)

#define DCMI_EXPST_LOW()        HAL_GPIO_WritePin(DCMI_EXPST_PORT, DCMI_EXPST_PIN, GPIO_PIN_RESET)
#define DCMI_EXPST_HIGH()       HAL_GPIO_WritePin(DCMI_EXPST_PORT, DCMI_EXPST_PIN, GPIO_PIN_SET)

#define DCMI_FSIN_LOW()         HAL_GPIO_WritePin(DCMI_FSIN_PORT, DCMI_FSIN_PIN, GPIO_PIN_RESET)
#define DCMI_FSIN_HIGH()        HAL_GPIO_WritePin(DCMI_FSIN_PORT, DCMI_FSIN_PIN, GPIO_PIN_SET)

#define DCMI_VSYNC_IRQN         EXTI9_5_IRQn
#define DCMI_VSYNC_IRQ_LINE     (7)

#endif //__OMV_BOARDCONFIG_H__

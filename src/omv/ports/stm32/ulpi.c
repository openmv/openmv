/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2016 STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions, and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * USB3320 ULPI functions ported from stm32f7xx_lp_modes.c
 */
#include "omv_boardconfig.h"

#if (OMV_USB_ULPI == 1)
#include STM32_HAL_H
#include <stdint.h>
#include "ulpi.h"
#include "omv_gpio.h"

#if defined(STM32H7)
#define GPIO_AF10_OTG_HS                   (GPIO_AF10_OTG2_HS)
#endif

#define USBULPI_PHYCR                      ((uint32_t) (0x40040000 + 0x034))
#define USBULPI_D07                        ((uint32_t) 0x000000FF)
#define USBULPI_New                        ((uint32_t) 0x02000000)
#define USBULPI_RW                         ((uint32_t) 0x00400000)
#define USBULPI_S_BUSY                     ((uint32_t) 0x04000000)
#define USBULPI_S_DONE                     ((uint32_t) 0x08000000)
#define USBULPI_TIMEOUT_COUNT              (500)

#define USB_OTG_READ_REG32(reg)            (*(__IO uint32_t *) (reg))
#define USB_OTG_WRITE_REG32(reg, value)    (*(__IO uint32_t *) (reg) = (value))


extern void __fatal_error(const char *msg);

/**
 * @brief  Read CR value
 * @param  Addr the Address of the ULPI Register
 * @retval Returns value of PHY CR register
 */
static uint32_t USB_ULPI_Read(uint32_t Addr) {
    __IO uint32_t val = 0;
    __IO uint32_t timeout = USBULPI_TIMEOUT_COUNT;

    USB_OTG_WRITE_REG32(USBULPI_PHYCR, USBULPI_New | (Addr << 16));
    val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    while (((val & USBULPI_S_DONE) == 0) && (timeout--)) {
        val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    }
    val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    return val & 0x000000ff;
}

/**
 * @brief  Write CR value
 * @param  Addr the Address of the ULPI Register
 * @param  Data Data to write
 * @retval Returns value of PHY CR register
 */
static uint32_t USB_ULPI_Write(uint32_t Addr, uint32_t Data) {
    __IO uint32_t val;
    __IO uint32_t timeout = USBULPI_TIMEOUT_COUNT;

    USB_OTG_WRITE_REG32(USBULPI_PHYCR, USBULPI_New | USBULPI_RW | (Addr << 16) | (Data & 0x000000ff));
    val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    while (((val & USBULPI_S_DONE) == 0) && (timeout--)) {
        val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    }

    val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    return 0;
}

#ifdef OMV_USB_ULPI_RST_PIN
void ulpi_reset(void) {
    omv_gpio_clock_enable(OMV_USB_ULPI_RST_PIN, true);
    omv_gpio_config(OMV_USB_ULPI_RST_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_USB_ULPI_RST_PIN, 0);
    HAL_Delay(10);
    omv_gpio_write(OMV_USB_ULPI_RST_PIN, 1);
    HAL_Delay(100);
}
#endif

/**
 * @brief  This function configures the USB PHY to enter the low power mode
 * @param  None
 * @retval None
 */
void ulpi_enter_low_power(void) {
    __IO uint32_t regval = 0;

    // Disable ULPI_CLK by accessing ULPI_PHY
    // Read Vendor ID : (Low, High) 0x24,0x04 for USB3300
    regval = USB_ULPI_Read(0x00);
    if (regval != 0x24) {
        __fatal_error("ULPI Error 0x00 != 0x24");
    }

    regval = USB_ULPI_Read(0x01);
    if (regval != 0x04) {
        __fatal_error("ULPI Error 0x01 != 0x04");
    }

    // Read Product ID
    regval = USB_ULPI_Read(0x02);
    if (regval != 0x07) {
        __fatal_error("ULPI Error 0x02 != 0x07");
    }

    regval = USB_ULPI_Read(0x03);
    if (regval != 0x00) {
        __fatal_error("ULPI Error 0x03 != 0x00");
    }

    // Write to scratch register the pattern 0x55
    USB_ULPI_Write(0x16, 0x55);
    // Read to scratch Register and check-it again the written Pattern
    regval = USB_ULPI_Read(0x16);
    if (regval != 0x55) {
        __fatal_error("ULPI Error 0x16 != 0x55");
    }

    // Write to scratch register the pattern 0xAA
    USB_ULPI_Write(0x16, 0xAA);
    // Read to scratch Register and check-it again the written Pattern
    regval = USB_ULPI_Read(0x16);
    if (regval != 0xAA) {
        __fatal_error("ULPI Error 0x16 != 0xAA");
    }

    // Read InterfaceControl reg
    regval = USB_ULPI_Read(0x07);

    // Write InterfaceControl reg,to disable PullUp on stp,
    // to avoid USB_PHY wake up when MCU entering standby
    USB_ULPI_Write(0x07, regval | 0x80);

    //Read InterfaceControl reg
    regval = USB_ULPI_Read(0x07);
    if (regval != 0x80) {
        __fatal_error("ULPI Error 0x07 != 0x80");
    }

    // Read FunctionControl reg
    regval = USB_ULPI_Read(0x04);

    // Reg 0x40 has a different value if USB is disconnected.
    if (regval != 0x40 && regval != 0x45) {
        __fatal_error("ULPI Error 0x04 != 0x40 || 0x45");
    }

    // Write FunctionControl reg,to put PHY into LowPower mode
    USB_ULPI_Write(0x04, regval & (~0x40));

    // Read FunctionControl reg again
    regval = USB_ULPI_Read(0x04);
    if (regval != 0x00) {
        __fatal_error("ULPI Error 0x04 != 0x00");
    }

    HAL_Delay(10);
}

/**
 * @brief  This function wakeup the USB PHY from the Low power mode
 * @param  None
 * @retval None
 */
void ulpi_leave_low_power(void) {
    // Configure STP as an output pin
    omv_gpio_clock_enable(OMV_USB_ULPI_STP_PIN, true);
    omv_gpio_config(OMV_USB_ULPI_STP_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);

    // Configure DIR as an input pin
    omv_gpio_clock_enable(OMV_USB_ULPI_DIR_PIN, true);
    omv_gpio_config(OMV_USB_ULPI_DIR_PIN, OMV_GPIO_MODE_INPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);

    // Set STP pin high
    omv_gpio_write(OMV_USB_ULPI_STP_PIN, 1);

    // Wait for DIR to go low
    for (uint32_t i = 0; i < 1000; i++, HAL_Delay(1)) {
        if (!omv_gpio_read(OMV_USB_ULPI_DIR_PIN)) {
            break;
        }
    }

    #ifdef OMV_USB_ULPI_RST_PIN
    if (omv_gpio_read(OMV_USB_ULPI_DIR_PIN)) {
        // Failed to exit suspend mode, reset the ULPI.
        ulpi_reset();
    }
    #endif

    // Configure OTG STP and DIR pin as USB AF
    omv_gpio_config(OMV_USB_ULPI_STP_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MAX, -1);
    omv_gpio_config(OMV_USB_ULPI_DIR_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MAX, -1);
}
#endif // (OMV_USB_ULPI == 1)

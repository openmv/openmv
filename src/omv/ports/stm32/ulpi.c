/*
 * Copyright (c) 2016 STMicroelectronics. All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 * USB3320 ULPI functions ported from stm32f7xx_lp_modes.c 
 */
#include "omv_boardconfig.h"

#if (OMV_USB_ULPI == 1)
#include STM32_HAL_H
#include <stdint.h>
#include "ulpi.h"
#include "omv_gpio.h"

#define USBULPI_PHYCR       ((uint32_t)(0x40040000 + 0x034))
#define USBULPI_D07         ((uint32_t)0x000000FF)
#define USBULPI_New         ((uint32_t)0x02000000)
#define USBULPI_RW          ((uint32_t)0x00400000)
#define USBULPI_S_BUSY      ((uint32_t)0x04000000)
#define USBULPI_S_DONE      ((uint32_t)0x08000000)

#define USB_OTG_READ_REG32(reg)  (*(__IO uint32_t *)(reg))
#define USB_OTG_WRITE_REG32(reg,value) (*(__IO uint32_t *)(reg) = (value))

#if defined(STM32H7)
#define GPIO_AF10_OTG_HS    (GPIO_AF10_OTG2_HS)
#endif

extern void __fatal_error(const char *msg);

/**
  * @brief  Read CR value
  * @param  Addr the Address of the ULPI Register
  * @retval Returns value of PHY CR register
  */
static uint32_t USB_ULPI_Read(uint32_t Addr)
{
    __IO uint32_t val = 0;
    __IO uint32_t timeout = 100; /* Can be tuned based on the Clock or etc... */

    USB_OTG_WRITE_REG32(USBULPI_PHYCR, USBULPI_New | (Addr << 16));
    val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    while (((val & USBULPI_S_DONE) == 0) && (timeout--)) {
        val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    }
    val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    return  (val & 0x000000ff);
}

/**
  * @brief  Write CR value
  * @param  Addr the Address of the ULPI Register
  * @param  Data Data to write
  * @retval Returns value of PHY CR register
  */
static uint32_t USB_ULPI_Write(uint32_t Addr, uint32_t Data)
{
    __IO uint32_t val;
    __IO uint32_t timeout = 10;   /* Can be tuned based on the Clock or etc... */

    USB_OTG_WRITE_REG32(USBULPI_PHYCR, USBULPI_New | USBULPI_RW | (Addr << 16) | (Data & 0x000000ff));
    val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    while (((val & USBULPI_S_DONE) == 0) && (timeout--)) {
        val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    }

    val = USB_OTG_READ_REG32(USBULPI_PHYCR);
    return 0;
}

/**
  * @brief  This function configures the USB PHY to enter the low power mode
  * @param  None
  * @retval None
  */
void ulpi_enter_low_power(void)
{
    __IO uint32_t regval = 0;

    /* disable ULPI_CLK by accessing ULPI_PHY */
    /* read Vendor ID : (Low, High) 0x24,0x04 for USB3300 */
    regval = USB_ULPI_Read(0x00);
    if (regval != 0x24) {
        __fatal_error("ULPI Error 0x00 != 0x24");
    }

    regval = USB_ULPI_Read(0x01);
    if (regval != 0x04) {
        __fatal_error("ULPI Error 0x01 != 0x04");
    }

    /* read Product ID */
    regval = USB_ULPI_Read(0x02);
    if (regval != 0x07) {
        __fatal_error("ULPI Error 0x02 != 0x07");
    }

    regval = USB_ULPI_Read(0x03);
    if (regval != 0x00) {
        __fatal_error("ULPI Error 0x03 != 0x00");
    }

    /* Write to scratch register the pattern 0x55 */
    USB_ULPI_Write(0x16, 0x55);
    /* Read to scratch Register and check-it again the written Pattern */
    regval = USB_ULPI_Read(0x16);
    if (regval != 0x55) {
        __fatal_error("ULPI Error 0x16 != 0x55");
    }

    /* Write to scratch register the pattern 0xAA */
    USB_ULPI_Write(0x16, 0xAA);
    /* Read to scratch Register and check-it again the written Pattern */
    regval = USB_ULPI_Read(0x16);
    if (regval != 0xAA) {
        __fatal_error("ULPI Error 0x16 != 0xAA");
    }

    /* read InterfaceControl reg */
    regval = USB_ULPI_Read(0x07);
    /* write InterfaceControl reg,to disable PullUp on stp,
       to avoid USB_PHY wake up when MCU entering standby */
    USB_ULPI_Write(0x07, regval | 0x80) ;
    /* read InterfaceControl reg */
    regval = USB_ULPI_Read(0x07);
    if (regval != 0x80) {
        __fatal_error("ULPI Error 0x07 != 0x80");
    }

    /* read FunctionControl reg */
    regval = USB_ULPI_Read(0x04);

    // Reg 0x40 has a different value if USB is disconnected.
    if (regval != 0x40 && regval != 0x45) {
        __fatal_error("ULPI Error 0x04 != 0x40 || 0x45");
    }

    /* write FunctionControl reg,to put PHY into LowPower mode */
    USB_ULPI_Write(0x04, regval & (~0x40));

    /* read FunctionControl reg again */
    regval = USB_ULPI_Read(0x04);
    if (regval != 0x00) {
        __fatal_error("ULPI Error 0x04 != 0x00");
    }

    /* Delay 4 ms */
    HAL_Delay(10);
}

/**
 * @brief  This function wakeup the USB PHY from the Low power mode
 * @param  None
 * @retval None
 */
void ulpi_leave_low_power(void)
{
    /* Enable GPIO clock for OTG USB STP pin */
    omv_gpio_clock_enable(OMV_USB_ULPI_STP_PIN, true);
    omv_gpio_clock_enable(OMV_USB_ULPI_DIR_PIN, true);

    /* Configure OTG STP and DIR pin as GP Output  */
    omv_gpio_config(OMV_USB_ULPI_STP_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_config(OMV_USB_ULPI_DIR_PIN, OMV_GPIO_MODE_INPUT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);

    /* Set OTG STP pin to High */
    omv_gpio_write(OMV_USB_ULPI_STP_PIN, 1);

    /* Wait for DIR to go low */
    for (uint32_t ticks = HAL_GetTick();
            ((HAL_GetTick() - ticks) < 500)
            && omv_gpio_read(OMV_USB_ULPI_DIR_PIN);) {
        __WFI();
    }

    /* Configure OTG STP and DIR pin as USB AF */
    omv_gpio_config(OMV_USB_ULPI_STP_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MAX, -1);
    omv_gpio_config(OMV_USB_ULPI_DIR_PIN, OMV_GPIO_MODE_ALT, OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_MAX, -1);
}
#endif // (OMV_USB_ULPI == 1)

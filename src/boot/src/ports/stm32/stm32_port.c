/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * DFU bootloader STM32 port.
 */
#include <stdint.h>
#include <string.h>

#include STM32_HAL_H
#include "omv_boardconfig.h"
#include "omv_bootconfig.h"
#include "flash.h"

#ifdef USE_USB_FS
#define USB_IRQ_Handler OTG_FS_IRQHandler
#else
#define USB_IRQ_Handler OTG_HS_IRQHandler
#endif

static volatile uint32_t ticks_ms;

extern void SystemClock_Config(void);
static void enable_gpio_clocks(bool enable);

void SysTick_Handler(void) {
    ticks_ms += 1;
}

void OTG_FS_IRQHandler(void) {
    tud_int_handler(0);
}

void OTG_HS_IRQHandler(void) {
    tud_int_handler(1);
}

int port_init(void) {
    HAL_Init();

    // Set the system clock
    SystemClock_Config();

    // Config Systick
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

    // Enable GPIO clocks.
    enable_gpio_clocks(true);

    // Configure I/O pins.
    for (size_t i = 0; i < OMV_BOOT_PINS_COUNT; i++) {
        const pin_t *pin = &omv_boot_pins[i];
        GPIO_InitTypeDef pin_config = {
            .Pin = pin->pin,
            .Pull = pin->pull,
            .Speed = pin->speed,
            .Mode = pin->mode,
            .Alternate = pin->alt,
        };
        HAL_GPIO_Init(pin->gpio, &pin_config);
    }

    // Turn off LED.
    port_pin_write(OMV_BOOT_LED_PIN, false);

    // Enable USB clocks.
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
    __HAL_RCC_USB_OTG_FS_CLK_SLEEP_ENABLE();

    // Configure and enable USB IRQ.
    HAL_NVIC_SetPriority(OMV_USB_IRQN, 6, 0);
    NVIC_ClearPendingIRQ(OMV_USB_IRQN);
    HAL_NVIC_EnableIRQ(OMV_USB_IRQN);
    return 0;
}

int port_deinit(void) {
    // Disable USB IRQ.
    HAL_NVIC_DisableIRQ(OMV_USB_IRQN);
    NVIC_ClearPendingIRQ(OMV_USB_IRQN);

    // Deinitialize SPI Flash.
    #if OMV_BOOT_SPI_FLASH_ENABLE
    spi_flash_deinit();
    #endif

    // Deinit all I/O pins.
    for (size_t i = 0; i < OMV_BOOT_PINS_COUNT; i++) {
        const pin_t *pin = &omv_boot_pins[i];
        HAL_GPIO_DeInit(pin->gpio, pin->pin);
    }

    // Disable USB clocks.
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
    __HAL_RCC_USB_OTG_FS_CLK_SLEEP_DISABLE();

    // Disable GPIO clocks.
    enable_gpio_clocks(false);
    return 0;
}

int port_get_uid(uint8_t *buf) {
    ((uint32_t *) buf)[0] = __REV(*((uint32_t *) (OMV_BOARD_UID_ADDR + 8)));
    ((uint32_t *) buf)[1] = __REV(*((uint32_t *) (OMV_BOARD_UID_ADDR + 4)));
    ((uint32_t *) buf)[2] = __REV(*((uint32_t *) (OMV_BOARD_UID_ADDR + 0)));
    return 0;
}

void port_mpu_protect(const partition_t *p, bool enable) {

}

uint32_t port_ticks_ms() {
    return ticks_ms;
}

void port_delay_ms(uint32_t ms) {
    uint32_t start_ms = port_ticks_ms();
    while ((port_ticks_ms() - start_ms) < ms) {
        __WFI();
    }
}

void port_pin_mode(uint32_t pin, uint32_t mode) {
    const pin_t *p = &omv_boot_pins[pin];
    GPIO_InitTypeDef pin_config;
    pin_config.Pin = p->pin;
    pin_config.Pull = GPIO_PULLUP;
    pin_config.Speed = GPIO_SPEED_FREQ_LOW;
    if (mode == PIN_MODE_OUTPUT) {
        pin_config.Mode = GPIO_MODE_OUTPUT_PP;
    } else {
        pin_config.Mode = GPIO_MODE_INPUT;
    }
    HAL_GPIO_Init(p->gpio, &pin_config);
}

void port_pin_write(uint32_t pin, uint32_t state) {
    const pin_t *p = &omv_boot_pins[pin];
    if (state) {
        HAL_GPIO_WritePin(p->gpio, p->pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(p->gpio, p->pin, GPIO_PIN_SET);
    }
}

void port_led_blink(uint32_t interval_ms) {
    static uint32_t start_ms = 0;
    static uint32_t led_state = 0;

    if ((port_ticks_ms() - start_ms) < interval_ms) {
        return;
    }

    led_state ^= 1;
    port_pin_write(OMV_BOOT_LED_PIN, led_state);
    start_ms += interval_ms;
}

void __attribute__((noreturn)) __fatal_error() {
    while (1) {
        port_led_blink(50);
    }
}

static void enable_gpio_clocks(bool enable) {
    // Enable GPIO clocks
    if (enable) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_GPIOE_CLK_ENABLE();
        #ifdef OMV_GPIO_PORT_F_ENABLE
        __HAL_RCC_GPIOF_CLK_ENABLE();
        #endif
        #ifdef OMV_GPIO_PORT_G_ENABLE
        __HAL_RCC_GPIOG_CLK_ENABLE();
        #endif
        #ifdef OMV_GPIO_PORT_H_ENABLE
        __HAL_RCC_GPIOH_CLK_ENABLE();
        #endif
        #ifdef OMV_GPIO_PORT_I_ENABLE
        __HAL_RCC_GPIOI_CLK_ENABLE();
        #endif
        #ifdef OMV_GPIO_PORT_J_ENABLE
        __HAL_RCC_GPIOJ_CLK_ENABLE();
        #endif
        #ifdef OMV_GPIO_PORT_K_ENABLE
        __HAL_RCC_GPIOK_CLK_ENABLE();
        #endif
        #ifdef OMV_GPIO_PORT_H_ENABLE
        __HAL_RCC_GPIOH_CLK_ENABLE();
        #endif
        #ifdef OMV_GPIO_PORT_N_ENABLE
        __HAL_RCC_GPION_CLK_ENABLE();
        #endif
        #ifdef OMV_GPIO_PORT_O_ENABLE
        __HAL_RCC_GPIOO_CLK_ENABLE();
        #endif
        #ifdef OMV_GPIO_PORT_P_ENABLE
        __HAL_RCC_GPIOP_CLK_ENABLE();
        #endif
        #ifdef OMV_GPIO_PORT_Q_ENABLE
        __HAL_RCC_GPIOQ_CLK_ENABLE();
        #endif
    } else {
        __HAL_RCC_GPIOA_CLK_DISABLE();
        __HAL_RCC_GPIOB_CLK_DISABLE();
        __HAL_RCC_GPIOC_CLK_DISABLE();
        __HAL_RCC_GPIOD_CLK_DISABLE();
        __HAL_RCC_GPIOE_CLK_DISABLE();
        #ifdef OMV_GPIO_PORT_F_ENABLE
        __HAL_RCC_GPIOF_CLK_DISABLE();
        #endif
        #ifdef OMV_GPIO_PORT_G_ENABLE
        __HAL_RCC_GPIOG_CLK_DISABLE();
        #endif
        #ifdef OMV_GPIO_PORT_H_ENABLE
        __HAL_RCC_GPIOH_CLK_DISABLE();
        #endif
        #ifdef OMV_GPIO_PORT_I_ENABLE
        __HAL_RCC_GPIOI_CLK_DISABLE();
        #endif
        #ifdef OMV_GPIO_PORT_J_ENABLE
        __HAL_RCC_GPIOJ_CLK_DISABLE();
        #endif
        #ifdef OMV_GPIO_PORT_K_ENABLE
        __HAL_RCC_GPIOK_CLK_DISABLE();
        #endif
        #ifdef OMV_GPIO_PORT_H_ENABLE
        __HAL_RCC_GPIOH_CLK_DISABLE();
        #endif
        #ifdef OMV_GPIO_PORT_N_ENABLE
        __HAL_RCC_GPION_CLK_DISABLE();
        #endif
        #ifdef OMV_GPIO_PORT_O_ENABLE
        __HAL_RCC_GPIOO_CLK_DISABLE();
        #endif
        #ifdef OMV_GPIO_PORT_P_ENABLE
        __HAL_RCC_GPIOP_CLK_DISABLE();
        #endif
        #ifdef OMV_GPIO_PORT_Q_ENABLE
        __HAL_RCC_GPIOQ_CLK_DISABLE();
        #endif
    }
}

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
 * DFU bootloader Alif port.
 */
#include <stdint.h>
#include <string.h>

#include ALIF_CMSIS_H
#include "gpio.h"
#include "pinconf.h"

#include "omv_boardconfig.h"
#include "omv_bootconfig.h"
#include "flash.h"
#include "alif_services.h"

static volatile uint32_t ticks_ms;

void SysTick_Handler(void) {
    ticks_ms += 1;
}

int port_init(void) {
    // Initialize the MPU and configure
    // the default attributes and regions.
    port_mpu_init();

    // Initialize the services API.
    alif_services_init();

    // Initialize and configure SysTick.
    SysTick_Config(SystemCoreClock / 1000);

    // Configure pin mux.
    for (size_t i = 0; i < OMV_BOOT_PINS_COUNT; i++) {
        const pin_t *pin = &omv_boot_pins[i];
        pinconf_set(pin->port, pin->pin, pin->alt, pin->pad);
    }

    // Configure USB MUX GPIO.
    port_pin_mode(OMV_BOOT_MUX_PIN, PIN_MODE_OUTPUT);
    port_pin_write(OMV_BOOT_MUX_PIN, 1);

    // Configure LED GPIO.
    port_pin_mode(OMV_BOOT_LED_PIN, PIN_MODE_OUTPUT);
    port_pin_write(OMV_BOOT_LED_PIN, 1);

    // Configure OSPI reset GPIO.
    port_pin_mode(OMV_BOOT_OSPI_RST_PIN, PIN_MODE_OUTPUT);
    port_pin_write(OMV_BOOT_OSPI_RST_PIN, 0);

    // Configure and enable USB IRQs.
    NVIC_ClearPendingIRQ(USB_IRQ_IRQn);
    NVIC_SetPriority(USB_IRQ_IRQn, OMV_BOOT_USB_IRQ_PRI);
    return 0;
}

int port_deinit(void) {
    // Deinit DCD and disable USB IRQs.
    extern void dcd_uninit(void);
    dcd_uninit();

    // Deinit services API.
    alif_services_deinit();

    // Deinitialize SPI Flash.
    #if OMV_BOOT_SPI_FLASH_ENABLE
    spi_flash_deinit();
    #endif

    // Disable SysTick and its IRQ.
    NVIC_DisableIRQ(SysTick_IRQn);
    NVIC_ClearPendingIRQ(SysTick_IRQn);
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk);

    // Clean/invalidate any cached lines.
    SCB_CleanInvalidateDCache();

    // Clear default regions and configure XIP regions (if any).
    port_mpu_deinit();

    // Turn off LED.
    gpio_set_value_high(omv_boot_pins[OMV_BOOT_LED_PIN].gpio, omv_boot_pins[OMV_BOOT_LED_PIN].pin);
    return 0;
}

int port_get_uid(uint8_t *buf) {
    alif_services_get_unique_id(buf, 12);
    return 0;
}

void port_mpu_protect(const partition_t *p, bool enable) {
    if (p->region != -1) {
        ARM_MPU_Disable();
        MPU->RNR = p->region;
        MPU->RBAR = ARM_MPU_RBAR(p->start, ARM_MPU_SH_NON, enable & 1, 1, 0); // RO/NP/XN.
        MPU->RLAR = ARM_MPU_RLAR(p->limit - 1, p->attr);
        ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk);
    }
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
    if (mode == PIN_MODE_OUTPUT) {
        gpio_set_direction_output(omv_boot_pins[pin].gpio, omv_boot_pins[pin].pin);
    } else {
        gpio_set_direction_input(omv_boot_pins[pin].gpio, omv_boot_pins[pin].pin);
    }
}

void port_pin_write(uint32_t pin, uint32_t state) {
    if (state) {
        gpio_set_value_high(omv_boot_pins[pin].gpio, omv_boot_pins[pin].pin);
    } else {
        gpio_set_value_low(omv_boot_pins[pin].gpio, omv_boot_pins[pin].pin);
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

// Bypass startup MPU defaults.
void MPU_Setup(void) {

}

// Bypass startup MPU regions.
void MPU_Load_Regions(void) {

}

void port_mpu_load_defaults() {
    static const ARM_MPU_Region_t mpu_table[] __STARTUP_RO_DATA_ATTRIBUTE = {
        {   /* SRAM0 - 4MB : RO-0, NP-1, XN-0 */
            .RBAR = ARM_MPU_RBAR(0x02000000, ARM_MPU_SH_NON, 0, 1, 0),
            .RLAR = ARM_MPU_RLAR(0x023FFFFF, MEMATTR_NORMAL_RA)
        },
        {   /* SRAM1 - 2.5MB : RO-0, NP-1, XN-0 */
            .RBAR = ARM_MPU_RBAR(0x08000000, ARM_MPU_SH_NON, 0, 1, 0),
            .RLAR = ARM_MPU_RLAR(0x0827FFFF, MEMATTR_NORMAL_WB_RA_WA)
        },
        {   /* Host Peripherals - 16MB : RO-0, NP-1, XN-1 */
            .RBAR = ARM_MPU_RBAR(0x1A000000, ARM_MPU_SH_NON, 0, 1, 1),
            .RLAR = ARM_MPU_RLAR(0x1AFFFFFF, MEMATTR_DEVICE_nGnRE)
        },
        {   /* OSPI Regs - 16MB : RO-0, NP-1, XN-1  */
            .RBAR = ARM_MPU_RBAR(0x83000000, ARM_MPU_SH_NON, 0, 1, 1),
            .RLAR = ARM_MPU_RLAR(0x83FFFFFF, MEMATTR_DEVICE_nGnRE)
        },
    };

    /* Load the regions from the table */
    ARM_MPU_Load(0, mpu_table, sizeof(mpu_table) / sizeof(ARM_MPU_Region_t));
}

void __attribute__((noreturn)) __fatal_error(const char *msg) {
    while (1) {
        port_led_blink(50);
    }
}

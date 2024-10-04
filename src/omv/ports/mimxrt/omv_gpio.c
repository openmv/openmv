/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2023 OpenMV, LLC.
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
 * GPIO port for imxrt.
 */
#include <stdint.h>
#include <stdint.h>
#include <stdbool.h>

#include "py/mphal.h"

#include "fsl_gpio.h"
#include "fsl_iomuxc.h"

#include "mimxrt_hal.h"
#include "omv_boardconfig.h"
#include "omv_gpio.h"
#include "omv_common.h"

#define IOMUXC_SET_REG(base, reg, val)    *((volatile uint32_t *) ((((uint32_t) base) << 16) | reg)) = (val)

extern omv_gpio_irq_descr_t *const gpio_irq_descr_table[MIMXRT_PAD_COUNT];

static uint32_t omv_gpio_irq_get_gpio(omv_gpio_t pin) {
    const imxrt_pad_t *pad = pin->pad;
    GPIO_Type *gpio_all[] = GPIO_BASE_PTRS;

    for (size_t i = 1; i < OMV_ARRAY_SIZE(gpio_all); i++) {
        if (pad->port == gpio_all[i]) {
            return i;
        }
    }
    return 0;
}

static uint32_t omv_gpio_irq_get_irqn(omv_gpio_t pin) {
    const imxrt_pad_t *pad = pin->pad;
    uint32_t gpio = omv_gpio_irq_get_gpio(pin);
    uint8_t low_irqs[] = GPIO_COMBINED_LOW_IRQS;
    uint8_t high_irqs[] = GPIO_COMBINED_HIGH_IRQS;
    return (pad->pin < 16) ? low_irqs[gpio] : high_irqs[gpio];
}

static omv_gpio_irq_descr_t *omv_gpio_irq_get_descr(omv_gpio_t pin) {
    const imxrt_pad_t *pad = pin->pad;
    uint32_t gpio = omv_gpio_irq_get_gpio(pin);
    if (gpio && gpio_irq_descr_table[gpio]) {
        return &gpio_irq_descr_table[gpio][pad->pin];
    }
    return NULL;
}

static const imxrt_pad_af_t *omv_gpio_find_af(omv_gpio_t pin, uint32_t af) {
    static const imxrt_pad_af_t af_gpio = {5, 0, 0};
    if (af == 5) {
        return &af_gpio;
    }

    const imxrt_pad_t *pad = pin->pad;
    for (size_t i = 0; i < pad->af_count; i++) {
        if (af == pad->af_list[i].idx) {
            return &pad->af_list[i];
        }
    }
    return NULL;
}

void omv_gpio_config(omv_gpio_t pin, uint32_t mode, uint32_t pull, uint32_t speed, uint32_t af) {
    const imxrt_pad_t *pad = pin->pad;
    af = ((af == -1) ? pin->af : af);
    const imxrt_pad_af_t *pad_af = omv_gpio_find_af(pin, af);

    if (pad_af == NULL) {
        return;
    }

    uint16_t pad_config = (mode | pull | speed) & 0x1FFFF;
    uint16_t mux_config = IOMUXC_MUX_CTL(MUX_MODE, af) | IOMUXC_MUX_CTL(SION, pin->sion);

    IOMUXC_SET_REG(pad->iomux_base, pad->mux_register, mux_config);
    if (pad_af->input_register) {
        // Input register always uses IOMUXC_BASE
        uint32_t iomux_base = ((uint32_t) IOMUXC_BASE) >> 16;
        IOMUXC_SET_REG(iomux_base, pad_af->input_register, pad_af->input_daisy);
    }
    IOMUXC_SET_REG(pad->iomux_base, pad->cfg_register, pad_config);

    // If pad was configured for GPIO module, set the pin config.
    gpio_pin_config_t pin_config = { 0 };
    switch (mode) {
        case OMV_GPIO_MODE_ALT:
        case OMV_GPIO_MODE_ALT_OD: {
            // No GPIO config.
            return;
        }
        case OMV_GPIO_MODE_INPUT: {
            pin_config.direction = kGPIO_DigitalInput;
            GPIO_PinInit(pad->port, pad->pin, &pin_config);
            break;
        }
        case OMV_GPIO_MODE_OUTPUT:
        case OMV_GPIO_MODE_OUTPUT_OD: {
            pin_config.direction = kGPIO_DigitalOutput;
            pin_config.outputLogic = 1;
            break;
        }
        case OMV_GPIO_MODE_IT_FALL: {
            pin_config.interruptMode = kGPIO_IntFallingEdge;
            break;
        }
        case OMV_GPIO_MODE_IT_RISE: {
            pin_config.interruptMode = kGPIO_IntRisingEdge;
            break;
        }
        case OMV_GPIO_MODE_IT_BOTH: {
            pin_config.interruptMode = kGPIO_IntRisingOrFallingEdge;
            break;
        }
    }

    GPIO_PinInit(pad->port, pad->pin, &pin_config);
}

void omv_gpio_deinit(omv_gpio_t pin) {

}

bool omv_gpio_read(omv_gpio_t pin) {
    const imxrt_pad_t *pad = pin->pad;
    return GPIO_PinRead(pad->port, pad->pin);
}

void omv_gpio_write(omv_gpio_t pin, bool value) {
    const imxrt_pad_t *pad = pin->pad;
    GPIO_PinWrite(pad->port, pad->pin, value);
}

void omv_gpio_irq_register(omv_gpio_t pin, omv_gpio_callback_t callback, void *data) {
    omv_gpio_irq_descr_t *irq_descr = omv_gpio_irq_get_descr(pin);
    if (irq_descr) {
        irq_descr->enabled = true;
        irq_descr->data = data;
        irq_descr->callback = callback;
    }
}

void omv_gpio_irq_enable(omv_gpio_t pin, bool enable) {
    const imxrt_pad_t *pad = pin->pad;
    uint32_t irqn = omv_gpio_irq_get_irqn(pin);
    omv_gpio_irq_descr_t *irq_descr = omv_gpio_irq_get_descr(pin);

    if (irq_descr == NULL) {
        return;
    }

    if (!enable) {
        GPIO_PortDisableInterrupts(pad->port, 1U << pad->pin);
        GPIO_PortClearInterruptFlags(pad->port, ~(0U));
        DisableIRQ(irqn);
    } else {
        GPIO_PortClearInterruptFlags(pad->port, ~(0U));
        GPIO_PortEnableInterrupts(pad->port, 1U << pad->pin);
        NVIC_SetPriority(irqn, IRQ_PRI_EXTINT);
        EnableIRQ(irqn);
    }
    irq_descr->enabled = enable;
}

void omv_gpio_irq_handler(GPIO_Type *gpio, uint32_t gpio_nr, uint32_t pin_nr) {
    omv_gpio_irq_descr_t *irq_descr = NULL;
    if (gpio_irq_descr_table[gpio_nr]) {
        irq_descr = &gpio_irq_descr_table[gpio_nr][pin_nr];
        if (irq_descr->enabled && irq_descr->callback) {
            irq_descr->callback(irq_descr->data);
        }
    }
}

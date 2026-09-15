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
 * WINC1500 BSP.
 */
#include <stdint.h>
#include <string.h>

#include "py/mphal.h"

#include "board_config.h"
#include "omv_gpio.h"

#include "conf_winc.h"
#include "bsp/include/nm_bsp.h"
#include "common/include/nm_common.h"
#include "wdrv_winc_gpio.h"

static tpfNmBspIsr gpfIsr;

// The 19.7.x driver owns the reset sequence (nm_reset) and the delay (nm_sleep),
// and drives the pins through these.
void WDRV_WINC_GPIOChipEnableAssert(void) {
    omv_gpio_write(OMV_WINC_EN_PIN, 1);
}

void WDRV_WINC_GPIOChipEnableDeassert(void) {
    omv_gpio_write(OMV_WINC_EN_PIN, 0);
}

void WDRV_WINC_GPIOResetAssert(void) {
    omv_gpio_write(OMV_WINC_RST_PIN, 0);
}

void WDRV_WINC_GPIOResetDeassert(void) {
    omv_gpio_write(OMV_WINC_RST_PIN, 1);
}

void WDRV_MSDelay(uint32_t ms) {
    mp_hal_delay_ms(ms);
}

int8_t nm_bsp_init(void) {
    gpfIsr = NULL;

    // Configure GPIO pins
    omv_gpio_config(OMV_WINC_EN_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_WINC_EN_PIN, 1);

    omv_gpio_config(OMV_WINC_RST_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(OMV_WINC_RST_PIN, 1);

    // Perform chip reset.
    nm_reset();

    return M2M_SUCCESS;
}

int8_t nm_bsp_deinit(void) {
    return M2M_SUCCESS;
}

static void nm_bsp_extint_callback(void *data) {
    if (gpfIsr) {
        gpfIsr();
    }
}

void nm_bsp_register_isr(tpfNmBspIsr pfIsr) {
    gpfIsr = pfIsr;
    omv_gpio_config(OMV_WINC_IRQ_PIN, OMV_GPIO_MODE_IT_FALL, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_irq_register(OMV_WINC_IRQ_PIN, nm_bsp_extint_callback, NULL);
    omv_gpio_irq_enable(OMV_WINC_IRQ_PIN, true);
}

void nm_bsp_interrupt_ctrl(uint8_t enable) {
    omv_gpio_irq_enable(OMV_WINC_IRQ_PIN, enable);
}

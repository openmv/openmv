/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * WINC1500 BSP.
 */

#include <stdint.h>
#include <string.h>

#include "py/mphal.h"

#include "omv_boardconfig.h"
#include "omv_gpio.h"

#include "conf_winc.h"
#include "bsp/include/nm_bsp.h"
#include "common/include/nm_common.h"

static tpfNmBspIsr gpfIsr;

sint8 nm_bsp_init(void) {
    gpfIsr = NULL;

    // Configure GPIO pins
    omv_gpio_config(WINC_EN_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(WINC_EN_PIN, 1);

    omv_gpio_config(WINC_RST_PIN, OMV_GPIO_MODE_OUTPUT, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_write(WINC_RST_PIN, 1);

    // Perform chip reset.
    nm_bsp_reset();

    return M2M_SUCCESS;
}

void nm_bsp_reset(void) {
    omv_gpio_write(WINC_EN_PIN, 0);
    omv_gpio_write(WINC_RST_PIN, 0);
    nm_bsp_sleep(100);
    omv_gpio_write(WINC_EN_PIN, 1);
    nm_bsp_sleep(100);
    omv_gpio_write(WINC_RST_PIN, 1);
    nm_bsp_sleep(100);
}

void nm_bsp_sleep(uint32 u32TimeMsec) {
    mp_hal_delay_ms(u32TimeMsec);
}

static void nm_bsp_extint_callback(void *data) {
    if (gpfIsr) {
        gpfIsr();
    }
}

void nm_bsp_register_isr(tpfNmBspIsr pfIsr) {
    gpfIsr = pfIsr;
    omv_gpio_config(WINC_IRQ_PIN, OMV_GPIO_MODE_IT_FALL, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_irq_register(WINC_IRQ_PIN, nm_bsp_extint_callback, NULL);
    omv_gpio_irq_enable(WINC_IRQ_PIN, true);
}

void nm_bsp_interrupt_ctrl(uint8 enable) {
    omv_gpio_irq_enable(WINC_IRQ_PIN, enable);
}

/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2025 Damien P. George
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
 * NUCLEO-N657X0-Q board support for OpenMV firmware.
 *
 * Merges the OPENMV_N6 board (OpenMV bootloader entry, standby) with the IO supply
 * handling of the NUCLEO_N657X0 board.
 *
 * IMPORTANT: on the Nucleo, VDDIO2 (ports O/P, including the camera reset line PO5)
 * is powered at 3.3V. Its high-speed low-voltage (HSLV) OTP fuse must never be
 * programmed on this board, so only the VDDIO3 fuse (XSPI2 flash, 1.8V) is handled.
 */

#include "py/mphal.h"
#include "boardctrl.h"
#include "xspi.h"

// Values for OTP fuses for VDDIO3, to select low voltage mode (<2.5V).
// See RM0486, Section 5, Table 18.
#define BSEC_HW_CONFIG_ID       (124U)
#define BSEC_HWS_HSLV_VDDIO3    (1U << 15)

#define OMV_BOOT_MAGIC_ADDR     (0x3401FFFCU)
#define OMV_BOOT_MAGIC_VALUE    (0xB00710ADU)

static void board_config_vdd(void) {
    // Enable PWR, BSEC and SYSCFG clocks.
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_PWR);
    LL_APB4_GRP2_EnableClock(LL_APB4_GRP2_PERIPH_BSEC);
    LL_APB4_GRP2_EnableClock(LL_APB4_GRP2_PERIPH_SYSCFG);

    // Read (never program) the VDDIO3 high speed IO fuse.
    uint32_t fuse;
    BSEC_HandleTypeDef hbsec = { .Instance = BSEC };
    if (HAL_BSEC_OTP_Read(&hbsec, BSEC_HW_CONFIG_ID, &fuse) != HAL_OK) {
        fuse = 0;
    }

    // Enable Vdd ADC, needed for the ADC to work.
    LL_PWR_EnableVddADC();

    // Configure VDDIO2 (3.3V on the Nucleo).
    LL_PWR_EnableVddIO2();
    LL_PWR_SetVddIO2VoltageRange(LL_PWR_VDDIO_VOLTAGE_RANGE_3V3);
    SYSCFG->VDDIO2CCCR |= SYSCFG_VDDIO2CCCR_EN; // enable IO compensation

    // Configure VDDIO3 (XSPI2 flash). Only enable 1.8V mode if the fuse is set.
    LL_PWR_EnableVddIO3();
    if (fuse & BSEC_HWS_HSLV_VDDIO3) {
        LL_PWR_SetVddIO3VoltageRange(LL_PWR_VDDIO_VOLTAGE_RANGE_1V8);
    }
    SYSCFG->VDDIO3CCCR |= SYSCFG_VDDIO3CCCR_EN; // enable IO compensation

    // Configure VDDIO4.
    LL_PWR_EnableVddIO4();
    LL_PWR_SetVddIO4VoltageRange(LL_PWR_VDDIO_VOLTAGE_RANGE_3V3);
    SYSCFG->VDDIO4CCCR |= SYSCFG_VDDIO4CCCR_EN; // enable IO compensation

    // Enable VDD for ADC and USB.
    LL_PWR_EnableVddADC();
    LL_PWR_EnableVddUSB();
}

void mboot_board_early_init(void) {
    board_config_vdd();
    xspi_init();
}

void board_enter_bootloader(unsigned int n_args, const void *args) {
    // Support both OpenMV bootloader and mboot.
    *((uint32_t *)OMV_BOOT_MAGIC_ADDR) = OMV_BOOT_MAGIC_VALUE;
    SCB_CleanDCache();
    NVIC_SystemReset();
}

static char _boot_mem[128] __attribute__((aligned(1024), section(".ram_function_data")));

__attribute__((naked, noreturn, section(".ram_function"))) void ram_reset(void) {
    // NVIC_SystemReset doesn't get inlined here.
    SCB->AIRCR  = (uint32_t)((0x5FAUL << SCB_AIRCR_VECTKEY_Pos) |
                             (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                             SCB_AIRCR_SYSRESETREQ_Msk);
    __DSB();
    for (;;) {
        __NOP();
    }
}

void board_early_init(void) {
    // The OpenMV bootloader has already configured the IO supplies and XSPI flash.
    // Re-assert VDDIO2 at 3.3V in case an earlier firmware changed it.
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_PWR);
    LL_PWR_EnableVddIO2();
    LL_PWR_SetVddIO2VoltageRange(LL_PWR_VDDIO_VOLTAGE_RANGE_3V3);
}

void board_enter_standby(void) {
    HAL_PWREx_EnableTCMRetention();
    HAL_PWREx_DisableTCMFLXRetention();

    uint32_t *boot_mem = (uint32_t *)_boot_mem;
    boot_mem[0] = (uint32_t)(_boot_mem + sizeof(_boot_mem));
    boot_mem[1] = ((uint32_t)&ram_reset) | 1;
    SCB_CleanDCache_by_Addr((uint32_t *)_boot_mem, 32);

    SYSCFG->INITSVTORCR = (uint32_t) boot_mem;
    (void) SYSCFG->INITSVTORCR;
}

void board_leave_standby(void) {
    // Enable PWR, BSEC and SYSCFG clocks.
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_PWR);
    LL_APB4_GRP2_EnableClock(LL_APB4_GRP2_PERIPH_BSEC);
    LL_APB4_GRP2_EnableClock(LL_APB4_GRP2_PERIPH_SYSCFG);

    // Configure VDDIO2 (3.3V on the Nucleo).
    LL_PWR_EnableVddIO2();
    LL_PWR_SetVddIO2VoltageRange(LL_PWR_VDDIO_VOLTAGE_RANGE_3V3);
    SYSCFG->VDDIO2CCCR |= SYSCFG_VDDIO2CCCR_EN; // enable IO compensation

    // Configure VDDIO3 (1.8V mode selection is retained).
    LL_PWR_EnableVddIO3();
    SYSCFG->VDDIO3CCCR |= SYSCFG_VDDIO3CCCR_EN; // enable IO compensation

    // Configure VDDIO4.
    LL_PWR_EnableVddIO4();
    LL_PWR_SetVddIO4VoltageRange(LL_PWR_VDDIO_VOLTAGE_RANGE_3V3);
    SYSCFG->VDDIO4CCCR |= SYSCFG_VDDIO4CCCR_EN; // enable IO compensation

    // Enable VDD for ADC and USB.
    LL_PWR_EnableVddADC();
    LL_PWR_EnableVddUSB();
}

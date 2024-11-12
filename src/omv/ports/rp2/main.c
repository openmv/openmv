/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2021 Damien P. George
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
 */

#include <stdio.h>

#include "py/compile.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/stackctrl.h"
#include "extmod/modbluetooth.h"
#include "extmod/modnetwork.h"
#include "shared/readline/readline.h"
#include "shared/runtime/gchelper.h"
#include "shared/runtime/pyexec.h"
#include "shared/runtime/softtimer.h"
#include "tusb.h"
#include "uart.h"
#include "modmachine.h"
#include "modrp2.h"
#include "mpbthciport.h"
#include "mpnetworkport.h"
#include "genhdr/mpversion.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/unique_id.h"
#include "hardware/rtc.h"
#include "hardware/irq.h"
#include "hardware/regs/intctrl.h"
#include "hardware/structs/rosc.h"
#include "pico/bootrom.h"
#include "pico/aon_timer.h"
#include "shared/timeutils/timeutils.h"

#include "omv_boardconfig.h"
#include "framebuffer.h"
#include "omv_i2c.h"
#include "omv_csi.h"
#include "usbdbg.h"
#include "tinyusb_debug.h"
#include "py_fir.h"
#if MICROPY_PY_AUDIO
#include "py_audio.h"
#endif
#include "pendsv.h"

#if MICROPY_VFS_FAT && MICROPY_HW_USB_MSC
#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#endif
#include "mp_utils.h"

extern void pendsv_init(void);
extern uint8_t __StackTop, __StackBottom;
static char OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(gc_heap[OMV_GC_BLOCK0_SIZE], 4), ".heap");

uint8_t *OMV_BOARD_UID_ADDR;
static pico_unique_board_id_t OMV_ATTR_ALIGNED(pico_unique_id, 4);

// Embed version info in the binary in machine readable form
bi_decl(bi_program_version_string(MICROPY_GIT_TAG));

// Add a section to the picotool output similar to program features, but for frozen modules
// (it will aggregate BINARY_INFO_ID_MP_FROZEN binary info)
bi_decl(bi_program_feature_group_with_flags(BINARY_INFO_TAG_MICROPYTHON,
                                            BINARY_INFO_ID_MP_FROZEN, "frozen modules",
                                            BI_NAMED_GROUP_SEPARATE_COMMAS | BI_NAMED_GROUP_SORT_ALPHA));

void __fatal_error() {
    gpio_init(OMV_LED_PIN);
    gpio_set_dir(OMV_LED_PIN, GPIO_OUT);

    while (true) {
        gpio_put(OMV_LED_PIN, 1);
        sleep_ms(100);
        gpio_put(OMV_LED_PIN, 0);
        sleep_ms(100);
    }
}

void pico_reset_to_bootloader(size_t n_args, const void *args_in) {
    reset_usb_boot(0, 0);
}

int main(int argc, char **argv) {
    bool first_soft_reset = true;

    // This is a tickless port, interrupts should always trigger SEV.
    #if PICO_ARM
    SCB->SCR |= SCB_SCR_SEVONPEND_Msk;
    #endif

    soft_timer_init();

    // Set the MCU frequency and as a side effect the peripheral clock to 48 MHz.
    set_sys_clock_khz(SYS_CLK_KHZ, false);

    // Hook for setting up anything that needs to be super early in the bootup process.
    MICROPY_BOARD_STARTUP();

    #if MICROPY_HW_ENABLE_UART_REPL
    bi_decl(bi_program_feature("UART REPL"))
    setup_default_uart();
    mp_uart_init();
    #else
    #ifndef NDEBUG
    stdio_init_all();
    #endif
    #endif

    #if MICROPY_HW_ENABLE_USBDEV
    bi_decl(bi_program_feature("USB REPL"))
    #endif

    #if MICROPY_PY_THREAD
    bi_decl(bi_program_feature("thread support"))
    mp_thread_init();
    #endif

    // Start and initialise the RTC
    struct timespec ts = { 0, 0 };
    ts.tv_sec = timeutils_seconds_since_epoch(2021, 1, 1, 0, 0, 0);
    aon_timer_start(&ts);
    mp_hal_time_ns_set_from_rtc();

    // Set board unique ID from flash for USB debugging.
    OMV_BOARD_UID_ADDR = pico_unique_id.id;
    pico_get_unique_board_id(&pico_unique_id);

soft_reset:
    // Initialise stack extents and GC heap.
    mp_init_gc_stack(&__StackBottom, &__StackTop, &gc_heap[0], &gc_heap[MP_ARRAY_SIZE(gc_heap)], 256);

    // Initialise MicroPython runtime.
    mp_init();
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_lib));

    // Initialise sub-systems.
    readline_init0();
    machine_pin_init();
    rp2_pio_init();
    rp2_dma_init();
    machine_i2s_init0();

    #if MICROPY_PY_BLUETOOTH
    mp_bluetooth_hci_init();
    #endif
    #if MICROPY_PY_NETWORK
    mod_network_init();
    #endif

    pendsv_init();
    usbdbg_init();

    fb_alloc_init0();
    framebuffer_init0();

    py_fir_init0();

    #if MICROPY_PY_CSI
    if (omv_csi_init() != 0) {
        printf("csi init failed!\n");
    }
    #endif

    // Execute _boot.py to set up the filesystem.
    pyexec_frozen_module("_boot.py", false);

    // Initialize TinyUSB after the filesystem has been mounted.
    if (!tusb_inited()) {
        tusb_init();

        // Install Tinyusb CDC debugger IRQ handler.
        irq_set_enabled(USBCTRL_IRQ, false);
        irq_remove_handler(USBCTRL_IRQ, irq_get_vtable_handler(USBCTRL_IRQ));
        irq_set_exclusive_handler(USBCTRL_IRQ, OMV_USB1_IRQ_HANDLER);
    }

    // Run boot.py script.
    bool interrupted = mp_exec_bootscript("boot.py", true);

    // Run main.py script on first soft-reset.
    if (first_soft_reset && !interrupted && mp_vfs_import_stat("main.py")) {
        mp_exec_bootscript("main.py", true);
        goto soft_reset_exit;
    }

    // If there's no script ready, just re-exec REPL
    while (!usbdbg_script_ready()) {
        nlr_buf_t nlr;

        if (nlr_push(&nlr) == 0) {
            // enable IDE interrupt
            usbdbg_set_irq_enabled(true);

            // run REPL
            if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
                if (pyexec_raw_repl() != 0) {
                    break;
                }
            } else {
                if (pyexec_friendly_repl() != 0) {
                    break;
                }
            }

            nlr_pop();
        }
    }

    if (usbdbg_script_ready()) {
        nlr_buf_t nlr;
        if (nlr_push(&nlr) == 0) {
            // Enable IDE interrupt
            usbdbg_set_irq_enabled(true);
            // Execute the script.
            pyexec_str(usbdbg_get_script(), true);
            // Disable IDE interrupts
            usbdbg_set_irq_enabled(false);
            nlr_pop();
        } else {
            mp_obj_print_exception(&mp_plat_print, (mp_obj_t) nlr.ret_val);
        }
    }

soft_reset_exit:
    mp_printf(MP_PYTHON_PRINTER, "MPY: soft reboot\n");
    #if MICROPY_PY_AUDIO
    py_audio_deinit();
    #endif
    #if MICROPY_PY_BLUETOOTH
    mp_bluetooth_deinit();
    #endif
    #if MICROPY_PY_NETWORK
    mod_network_deinit();
    #endif
    rp2_pio_deinit();
    rp2_dma_deinit();
    machine_pwm_deinit_all();
    machine_pin_deinit();
    gc_sweep_all();
    mp_deinit();
    first_soft_reset = false;
    goto soft_reset;
    return 0;
}

void nlr_jump_fail(void *val) {
    printf("FATAL: uncaught exception %p\n", val);
    mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(val));
    for (;;) {
        __breakpoint();
    }
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    panic("Assertion failed");
}
#endif

#define POLY    (0xD5)

uint8_t rosc_random_u8(size_t cycles) {
    static uint8_t r;
    for (size_t i = 0; i < cycles; ++i) {
        r = ((r << 1) | rosc_hw->randombit) ^ (r & 0x80 ? POLY : 0);
        mp_hal_delay_us_fast(1);
    }
    return r;
}

uint32_t rosc_random_u32(void) {
    uint32_t value = 0;
    for (size_t i = 0; i < 4; ++i) {
        value = value << 8 | rosc_random_u8(32);
    }
    return value;
}

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
 */
#include "py/compile.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/stackctrl.h"
#include "shared/readline/readline.h"
#include "shared/runtime/gchelper.h"
#include "shared/runtime/pyexec.h"
#include "shared/runtime/softtimer.h"
#include "tusb.h"
#include "mpuart.h"
#include "ospi_flash.h"
#include "pendsv.h"
#include "se_services.h"
#include "system_tick.h"

#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"

#if MICROPY_PY_LWIP
#include "lwip/init.h"
#include "lwip/apps/mdns.h"
#endif

#if MICROPY_PY_NETWORK
#include "extmod/modnetwork.h"
#endif

#if MICROPY_PY_BLUETOOTH
#include "mpbthciport.h"
#include "extmod/modbluetooth.h"
#endif

#if MICROPY_PY_NETWORK_CYW43
#include "lib/cyw43-driver/src/cyw43.h"
#endif

#include "omv_boardconfig.h"
#include "framebuffer.h"
#include "usbdbg.h"
#include "tinyusb_debug.h"
#include "fb_alloc.h"
#include "file_utils.h"
#include "mp_utils.h"
#include "omv_csi.h"
#include "omv_gpu.h"
#include "alif_hal.h"
#include "alif_dma.h"
#include "py_audio.h"
#include "py_imu.h"

NORETURN void __fatal_error(const char *msg);

int main(void) {
    bool first_soft_reset = true; (void) first_soft_reset;

    system_tick_init();
    alif_hal_init();

    pendsv_init();

    #if MICROPY_HW_ENABLE_UART_REPL
    mp_uart_init_repl();
    #endif

    #if MICROPY_HW_ENABLE_OSPI
    ospi_flash_init();
    #endif

soft_reset:
    // Initialise stack extents and GC heap.
    extern uint8_t _estack, _sstack, _heap_start, _heap_end;
    mp_init_gc_stack(&_sstack, &_estack, &_heap_start, &_heap_end, 1024);

    // Initialise MicroPython runtime.
    mp_init();

    // Initialise sub-systems.
    readline_init0();
    fb_alloc_init0();
    framebuffer_init0();
    #if MICROPY_PY_CSI
    omv_csi_init0();
    #endif
    #ifdef IMLIB_ENABLE_IMAGE_FILE_IO
    file_buffer_init0();
    #endif
    #if CORE_M55_HP
    usbdbg_init();
    #endif
    #if MICROPY_PY_IMU
    py_imu_init();
    #endif
    imlib_init_all();
    #if MICROPY_PY_CSI
    if (first_soft_reset) {
        omv_csi_init();
    }
    #endif

    #if MICROPY_PY_LWIP
    // lwIP can only be initialized once, because the system timeout
    // list (next_timeout), is only ever reset by BSS clearing.
    if (first_soft_reset) {
        lwip_init();
        #if LWIP_MDNS_RESPONDER
        mdns_resp_init();
        #endif
    }
    mod_network_lwip_init();
    #endif

    #if MICROPY_PY_BLUETOOTH
    mp_bluetooth_hci_init();
    #endif

    #if MICROPY_PY_NETWORK_CYW43
    cyw43_init(&cyw43_state);
    uint8_t buf[8];
    memcpy(&buf[0], "ALIF", 4);
    mp_hal_get_mac_ascii(MP_HAL_MAC_WLAN0, 8, 4, (char *) &buf[4]);
    cyw43_wifi_ap_set_ssid(&cyw43_state, 8, buf);
    cyw43_wifi_ap_set_auth(&cyw43_state, CYW43_AUTH_WPA2_AES_PSK);
    cyw43_wifi_ap_set_password(&cyw43_state, 8, (const uint8_t *) "alif0123");
    #endif

    #if MICROPY_PY_NETWORK
    mod_network_init();
    #endif

    // TODO: _boot on HP uses OSPI which is disabled right now.
    #if MICROPY_HW_ENABLE_OSPI || CORE_M55_HE
    // Execute _boot.py.
    pyexec_frozen_module("_boot.py", false);
    #endif

    #if MICROPY_HW_ENABLE_OSPI
    // Mark the filesystem as an OpenMV storage.
    file_ll_touch(".openmv_disk");
    #endif

    // Initialize TinyUSB after the filesystem is mounted.
    #if MICROPY_HW_ENABLE_USBDEV
    if (!tusb_inited()) {
        tusb_init();
    }
    #endif

    // Run boot.py script.
    bool interrupted = mp_exec_bootscript("boot.py", true);

    // Run main.py script on first soft-reset.
    if (first_soft_reset && !interrupted && mp_vfs_import_stat("main.py")) {
        mp_exec_bootscript("main.py", true);
        goto soft_reset_exit;
    }

    #if CORE_M55_HE
    goto soft_reset_exit;
    #endif

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
    #if MICROPY_PY_NETWORK_CYW43
    cyw43_deinit(&cyw43_state);
    #endif
    imlib_deinit_all();
    soft_timer_deinit();
    dma_deinit_all();
    gc_sweep_all();
    mp_deinit();
    first_soft_reset = false;
    goto soft_reset;
}

void gc_collect(void) {
    gc_collect_start();
    gc_helper_collect_regs_and_stack();
    gc_collect_end();
}

void nlr_jump_fail(void *val) {
    mp_printf(&mp_plat_print, "FATAL: uncaught exception %p\n", val);
    mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(val));
    for (;;) {
        __WFE();
    }
}

NORETURN void __fatal_error(const char *msg) {
    mp_hal_stdout_tx_strn("\nFATAL ERROR:\n", 14);
    mp_hal_stdout_tx_strn(msg, strlen(msg));
    for (;;) {
        *(volatile uint32_t *) 0x4900C000 ^= 9;
        for (volatile uint delay = 0; delay < 10000000; delay++) {
        }
    }
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif

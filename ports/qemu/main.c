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

#include "omv_boardconfig.h"
#include "mp_utils.h"
#include "file_utils.h"
#include "fb_alloc.h"
#include "framebuffer.h"
#include "omv_csi.h"

int main(int argc, char **argv) {
    bool first_soft_reset = true;

soft_reset:
    // Initialise stack extents and GC heap.
    extern uint8_t _estack, _sstack, _heap_start, _heap_end;
    mp_init_gc_stack(&_sstack, &_estack, &_heap_start, &_heap_end, 1024);

    mp_init();

    fb_alloc_init0();
    framebuffer_init0();

    #ifdef IMLIB_ENABLE_IMAGE_FILE_IO
    file_buffer_init0();
    #endif

    imlib_init_all();

    #if MICROPY_PY_CSI
    omv_csi_init0();

    if (first_soft_reset) {
        omv_csi_init();
    }
    #endif

    for (;;) {
        if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
            if (pyexec_raw_repl() != 0) {
                break;
            }
        } else {
            if (pyexec_friendly_repl() != 0) {
                break;
            }
        }
    }

    mp_printf(&mp_plat_print, "MPY: soft reboot\n");
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
    exit(1);
}

void __fatal_error(const char *msg) {
    mp_hal_stdout_tx_strn("\nFATAL ERROR:\n", 14);
    mp_hal_stdout_tx_strn(msg, strlen(msg));
    for (;;) {
        *(volatile uint32_t *) 0x4900C000 ^= 9;
        for (volatile uint delay = 0; delay < 10000000; delay++) {
        }
    }
}

void usbdbg_set_irq_enabled(bool enabled) {

}

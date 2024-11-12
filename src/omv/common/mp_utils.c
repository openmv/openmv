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
 * Common MicroPython utility functions.
 */
#include <stdio.h>
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/objstr.h"
#include "py/mpthread.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "shared/runtime/gchelper.h"
#include "shared/runtime/softtimer.h"
#include "shared/runtime/pyexec.h"
#include "omv_boardconfig.h"
#include "usbdbg.h"
#if OMV_WIFIDBG_ENABLE
#include "wifidbg.h"
#endif
#include "mp_utils.h"

void __attribute__((weak)) gc_collect(void) {
    // start the GC
    gc_collect_start();

    // trace the stack and registers
    gc_helper_collect_regs_and_stack();

    // trace root pointers from any threads
    #if MICROPY_PY_THREAD
    mp_thread_gc_others();
    #endif

    // trace soft timer nodes
    #ifndef MP_PORT_NO_SOFTTIMER
    soft_timer_gc_mark_all();
    #endif

    // end the GC
    gc_collect_end();
}

bool mp_exec_bootscript(const char *path, bool interruptible, bool wifidbg_enabled) {
    nlr_buf_t nlr;
    bool interrupted = false;

    if (nlr_push(&nlr) == 0) {
        // Enable IDE interrupts if allowed.
        if (interruptible) {
            usbdbg_set_irq_enabled(true);
            usbdbg_set_script_running(true);
            #if OMV_WIFIDBG_ENABLE
            wifidbg_set_irq_enabled(wifidbg_enabled);
            #endif
        }

        // Parse, compile and execute the script.
        pyexec_file_if_exists(path, true);
        nlr_pop();
    } else {
        interrupted = true;
    }

    // Disable IDE interrupts
    usbdbg_set_irq_enabled(false);
    usbdbg_set_script_running(false);
    #if OMV_WIFIDBG_ENABLE
    wifidbg_set_irq_enabled(false);
    #endif

    if (interrupted) {
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t) nlr.ret_val);
    }

    return interrupted;
}

void mp_init_gc_stack(void *sstack, void *estack, void *heap_start, void *heap_end, size_t stack_limit) {
    // Initialize the stack.
    mp_stack_set_top(estack);

    // Set stack limit
    // Stack limit should be less than real stack size, so we have a
    // chance to recover from limit hit. (Limit is measured in bytes)
    mp_stack_set_limit(estack - sstack - stack_limit);

    #if MICROPY_ENABLE_PYSTACK
    static mp_obj_t pystack[384];
    mp_pystack_init(pystack, &pystack[384]);
    #endif

    // Initialize gc memory.
    gc_init(heap_start, heap_end);

    // Add GC blocks (if enabled).
    #ifdef OMV_GC_BLOCK1_MEMORY
    typedef struct {
        uint8_t *addr;
        uint32_t size;
    } gc_blocks_table_t;

    extern const gc_blocks_table_t _gc_blocks_table_start;
    extern const gc_blocks_table_t _gc_blocks_table_end;

    for (gc_blocks_table_t const *block = &_gc_blocks_table_start; block < &_gc_blocks_table_end; block++) {
        gc_add(block->addr, block->addr + block->size);
    }
    #endif
}

/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
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
#if MICROPY_HW_USB_MSC
#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
// Fresh filesystem templates.
#include "main_py.h"
#include "readme_txt.h"
#endif
#include "omv_boardconfig.h"
#include "usbdbg.h"
#if OMV_WIFIDBG_ENABLE
#include "wifidbg.h"
#endif
#include "file_utils.h"
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

#if MICROPY_VFS_FAT
extern void __fatal_error();

int mp_init_filesystem(fs_user_mount_t *vfs) {
    FIL fp; UINT n;
    uint8_t working_buf[FF_MAX_SS];
    if (f_mkfs(&vfs->fatfs, FM_FAT, 0, working_buf, sizeof(working_buf)) != FR_OK) {
        __fatal_error("Could not create LFS");
    }

    // Mark FS as OpenMV disk.
    if (f_stat(&vfs->fatfs, "/.openmv_disk", NULL) != FR_OK) {
        f_open(&vfs->fatfs, &fp, "/.openmv_disk", FA_WRITE | FA_CREATE_ALWAYS);
        f_close(&fp);
    }

    // Create default main.py
    f_open(&vfs->fatfs, &fp, "/main.py", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
    f_close(&fp);

    // Create readme file
    f_open(&vfs->fatfs, &fp, "/README.txt", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_readme_txt, sizeof(fresh_readme_txt) - 1 /* don't count null terminator */, &n);
    f_close(&fp);

    return 0;
}
#endif

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

    // Initialize gc memory.
    gc_init(heap_start, heap_end);

    // Add GC blocks (if enabled).
    #ifdef OMV_GC_BLOCK1_MEMORY
    typedef struct {
        uint32_t *addr;
        uint32_t size;
    } gc_blocks_table_t;

    extern const gc_blocks_table_t _gc_blocks_table_start;
    extern const gc_blocks_table_t _gc_blocks_table_end;

    for (gc_blocks_table_t const *block = &_gc_blocks_table_start; block < &_gc_blocks_table_end; block++) {
        gc_add(block->addr, block->addr + block->size);
    }
    #endif
}

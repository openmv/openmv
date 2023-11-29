/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Boot util functions.
 */
#include <stdio.h>
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/objstr.h"
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
#if OMV_ENABLE_WIFIDBG
#include "wifidbg.h"
#endif
#include "file_utils.h"
#include "boot_utils.h"

#if MICROPY_VFS_FAT
extern void __fatal_error();

int bootutils_init_filesystem(fs_user_mount_t *vfs) {
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

bool bootutils_exec_bootscript(const char *path, bool interruptible, bool wifidbg_enabled) {
    nlr_buf_t nlr;
    bool interrupted = false;

    if (nlr_push(&nlr) == 0) {
        // Enable IDE interrupts if allowed.
        if (interruptible) {
            usbdbg_set_irq_enabled(true);
            usbdbg_set_script_running(true);
            #if OMV_ENABLE_WIFIDBG
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
    #if OMV_ENABLE_WIFIDBG
    wifidbg_set_irq_enabled(false);
    #endif

    if (interrupted) {
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t) nlr.ret_val);
    }

    return interrupted;
}

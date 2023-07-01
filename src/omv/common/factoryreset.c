/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2022 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2022 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Factory reset util functions.
 */
#include <stdio.h>
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "omv_boardconfig.h"

#if MICROPY_HW_USB_MSC
#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#include "common/factoryreset.h"

// Fresh filesystem templates.
#include "main_py.h"
#include "readme_txt.h"
#if (OMV_ENABLE_SELFTEST == 1)
#include "selftest_py.h"
#endif

extern void __fatal_error();

int factoryreset_create_filesystem(fs_user_mount_t *vfs) {
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

    #if (OMV_ENABLE_SELFTEST == 1)
    // Create default selftest.py
    f_open(&vfs->fatfs, &fp, "/selftest.py", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&fp, fresh_selftest_py, sizeof(fresh_selftest_py) - 1 /* don't count null terminator */, &n);
    f_close(&fp);
    #endif

    return 0;
}
#endif

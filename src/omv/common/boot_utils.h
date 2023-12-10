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
#ifndef __BOOT_UTILS_H__
#define __BOOT_UTILS_H__
typedef struct _fs_user_mount_t fs_user_mount_t;
int bootutils_init_filesystem(fs_user_mount_t *vfs);
bool bootutils_exec_bootscript(const char *path, bool interruptible, bool wifidbg_enabled);
#endif // __BOOT_UTILS_H__

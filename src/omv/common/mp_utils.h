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
#ifndef __MP_UTILS_H__
#define __MP_UTILS_H__
typedef struct _fs_user_mount_t fs_user_mount_t;
void mp_init_gc_stack(void *stack_start, void *stack_end, void *heap_start, void *heap_end, size_t stack_limit);
int mp_init_filesystem(fs_user_mount_t *vfs);
bool mp_exec_bootscript(const char *path, bool interruptible, bool wifidbg_enabled);
#endif // __MP_UTILS_H__

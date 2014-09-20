/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * WLAN Python module.
 *
 */
#ifndef __PY_WLAN_H__
#define __PY_WLAN_H__
int wlan_get_fd_state(int fd);
void wlan_clear_fd_state(int fd);
const mp_obj_module_t *py_wlan_init();
#endif // __PY_WLAN_H__

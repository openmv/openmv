/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Socket Python module.
 *
 */
#ifndef __PY_SOCKET_H__
#define __PY_SOCKET_H__
typedef struct {
    mp_obj_base_t base;
    int fd;
} socket_t;
extern const mp_obj_type_t socket_type;
const mp_obj_module_t *py_socket_init();
#endif // __PY_SOCKET_H__

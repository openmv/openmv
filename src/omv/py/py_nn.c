/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * NN module.
 *
 */
#include "mp.h"
#include "nn.h"
#include "imlib.h"
#include "xalloc.h"
#include "py_image.h"
#include "py_helper.h"
#include "py_assert.h"
#include "omv_boardconfig.h"

static const mp_obj_type_t py_net_type;

typedef struct _py_net_obj_t {
    mp_obj_base_t base;
    nn_t _cobj;
} py_net_obj_t;

void *py_net_cobj(mp_obj_t net)
{
    PY_ASSERT_TYPE(net, &py_net_type);
    return &((py_net_obj_t *)net)->_cobj;
}

STATIC mp_obj_t py_net_forward(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    int8_t output_data[10];
    nn_t *net = py_net_cobj(args[0]);
    image_t *img = py_helper_arg_to_image_mutable(args[1]);
    nn_run_network(net, img, output_data);

    mp_obj_t output_list = mp_obj_new_list(0, NULL);
    for (int i=0; i<10; i++) {
        mp_obj_list_append(output_list, mp_obj_new_int(output_data[i]));
    }

    return output_list;
}

STATIC void py_net_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_net_obj_t *self = self_in;
    nn_dump_network(py_net_cobj(self));
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_net_forward_obj, 2, py_net_forward);


static const mp_map_elem_t locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_forward),  (mp_obj_t)&py_net_forward_obj},
    { NULL, NULL },
};

STATIC MP_DEFINE_CONST_DICT(locals_dict, locals_dict_table);

static const mp_obj_type_t py_net_type = {
    { &mp_type_type },
    .name  = MP_QSTR_Net,
    .print = py_net_print,
    .locals_dict = (mp_obj_t)&locals_dict,
};

static mp_obj_t py_nn_load(mp_obj_t path_obj)
{
    py_net_obj_t *net = NULL;
    const char *path = mp_obj_str_get_str(path_obj);
    net = m_new_obj(py_net_obj_t);
    net->base.type = &py_net_type;
    nn_load_network(py_net_cobj(net), path);
    return net;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_nn_load_obj, py_nn_load);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_nn) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_load),   (mp_obj_t)&py_nn_load_obj },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t nn_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t)&globals_dict,
};

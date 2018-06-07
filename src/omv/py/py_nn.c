/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2018 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */
#include <mp.h>
#include "nn.h"
#include "py_helper.h"
#include "py_image.h"
#include "omv_boardconfig.h"

#ifdef IMLIB_ENABLE_CNN

static const mp_obj_type_t py_net_type;

typedef struct _py_net_obj_t {
    mp_obj_base_t base;
    nn_t _cobj;
} py_net_obj_t;

void *py_net_cobj(mp_obj_t net_obj)
{
    PY_ASSERT_TYPE(net_obj, &py_net_type);
    return &((py_net_obj_t *)net_obj)->_cobj;
}

STATIC mp_obj_t py_net_forward(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    nn_t *net = py_net_cobj(args[0]);
    image_t *img = py_helper_arg_to_image_mutable(args[1]);
    bool softmax = py_helper_keyword_int(n_args, args, 2, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_softmax), false);
    bool dry_run = py_helper_keyword_int(n_args, args, 3, kw_args, MP_OBJ_NEW_QSTR(MP_QSTR_dry_run), false);

    mp_obj_t output_list = mp_obj_new_list(0, NULL);

    if (dry_run == false) {
        nn_run_network(net, img, softmax);
    } else {
        nn_dry_run_network(net, img, softmax);
    }

    for (int i=0; i<net->output_size; i++) {
        mp_obj_list_append(output_list, mp_obj_new_int(net->output_data[i]));
    }

    return output_list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_net_forward_obj, 2, py_net_forward);

STATIC void py_net_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_net_obj_t *self = self_in;
    nn_dump_network(py_net_cobj(self));
}

STATIC const mp_rom_map_elem_t locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_forward), MP_ROM_PTR(&py_net_forward_obj) }
};

STATIC MP_DEFINE_CONST_DICT(locals_dict, locals_dict_table);

static const mp_obj_type_t py_net_type = {
    { &mp_type_type },
    .name  = MP_QSTR_Net,
    .print = py_net_print,
    .locals_dict = (mp_obj_t) &locals_dict
};

static mp_obj_t py_nn_load(mp_obj_t path_obj)
{
    const char *path = mp_obj_str_get_str(path_obj);
    py_net_obj_t *net = m_new_obj(py_net_obj_t);
    net->base.type = &py_net_type;
    nn_load_network(py_net_cobj(net), path);
    return net;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_nn_load_obj, py_nn_load);

#endif // IMLIB_ENABLE_CNN

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_nn) },
#ifdef IMLIB_ENABLE_CNN
    { MP_ROM_QSTR(MP_QSTR_load),     MP_ROM_PTR(&py_nn_load_obj) },
#else
    { MP_ROM_QSTR(MP_QSTR_load),     MP_ROM_PTR(&py_func_unavailable_obj) }
#endif // IMLIB_ENABLE_CNN
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t nn_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t) &globals_dict
};

/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2019 STMicroelectronics
 *
 * This work is licensed under the MIT license, see the file LICENSE for
 * details.
 */

#include "py/runtime.h"
#include "py/objlist.h"

#include "omv_boardconfig.h"
#include "py_helper.h"
#include "py_assert.h"
#include "py_image.h"
#include "nn_st.h"

static const mp_obj_type_t py_st_net_type;

typedef struct _py_st_net_obj_t {
  mp_obj_base_t base;
  stnn_t _cobj;
} py_st_net_obj_t;

void *py_st_net_cobj(mp_obj_t net_obj) {
  PY_ASSERT_TYPE(net_obj, &py_st_net_type);
  return &((py_st_net_obj_t *)net_obj)->_cobj;
}

STATIC void py_net_print(const mp_print_t *print, mp_obj_t self_in,
                         mp_print_kind_t kind) {
  // py_st_net_obj_t *self = self_in;
  // nn_dump_network(py_st_net_cobj(self));
}

/*Function in charge of running a NN referenced to by args[0]. Raw input data is
 * pointed to by args[1]*/
STATIC mp_obj_t __attribute__((optimize("O0")))
py_net_predict(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
  stnn_t *net = py_st_net_cobj(args[0]);
  image_t *img = py_helper_arg_to_image_mutable(args[1]);

  rectangle_t roi;
  py_helper_keyword_rectangle_roi(img, n_args, args, 2, kw_args, &roi);

  mp_obj_t output_list = mp_obj_new_list(0, NULL);

  aiRun(net, img, &roi);

  float *out_data = (float *)(net->nn_exec_ctx_ptr->report.outputs->data);

  for (int i = 0; i < AI_NETWORK_OUT_1_SIZE; i++) {
    mp_obj_list_append(output_list, mp_obj_new_float(*(out_data + i)));
  }
  return output_list;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_net_predict_obj, 2, py_net_predict);

STATIC const mp_rom_map_elem_t locals_dict_table[] = {
    {MP_ROM_QSTR(MP_QSTR_predict), MP_ROM_PTR(&py_net_predict_obj)}};

STATIC MP_DEFINE_CONST_DICT(py_net_locals_dict, locals_dict_table);

STATIC MP_DEFINE_CONST_OBJ_TYPE(
    py_st_net_type,
    MP_QSTR_Net,
    MP_TYPE_FLAG_NONE,
    print, py_net_print,
    locals_dict, &py_net_locals_dict
);

/* Function in charge of creating an instance of "ST NN" class and initializing
 * the NN named nn_name */
static mp_obj_t py_nn_st_load(mp_obj_t nn_name) {
  const char *network_name = mp_obj_str_get_str(nn_name);
  py_st_net_obj_t *net = m_new_obj(py_st_net_obj_t);
  net->base.type = &py_st_net_type;
  aiDeInit();
  aiInit(network_name, py_st_net_cobj(net));
  return net;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_nn_st_load_obj, py_nn_st_load);

STATIC const mp_rom_map_elem_t globals_dict_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_nn_st)},
    {MP_ROM_QSTR(MP_QSTR_loadnnst), MP_ROM_PTR(&py_nn_st_load_obj)},
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

/* Create nn_st_module module + add ref to it in mpconfigport.h file */
const mp_obj_module_t nn_st_module = {.base = {&mp_type_module},
                                      .globals = (mp_obj_t)&globals_dict};

MP_REGISTER_MODULE(MP_QSTR_nn_st, nn_st_module);

#include <libmp.h>
#include "imlib.h"
#include "py_assert.h"
#include "py_image.h"
typedef struct _py_image_obj_t {
    mp_obj_base_t base;
    struct image _cobj;
} py_image_obj_t;

void py_image_load_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest)
{
//    py_image_obj_t *self = self_in;
//    dest[0] = mp_obj_new_int(self->width);
}

bool py_image_store_attr(mp_obj_t self_in, qstr attr, mp_obj_t value)
{
    return true;
}

void py_image_print(void (*print)(void *env, const char *fmt, ...), void *env, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_image_obj_t *self = self_in;
    print(env, "<image width:%d height:%d bpp:%d>", self->_cobj.w, self->_cobj.h, self->_cobj.bpp);
}

static const mp_method_t py_image_methods[] = {
    { NULL, NULL },
};

const mp_obj_type_t py_image_type = {
    { &mp_type_type },
    .name       = MP_QSTR_Image,
    .print      = py_image_print,
    .methods    = py_image_methods,
//    .load_attr  = py_image_load_attr,
//    .store_attr = py_image_store_attr,

};

mp_obj_t py_image(int w, int h, int bpp, void *pixels)
{
    py_image_obj_t *o = m_new_obj(py_image_obj_t);
    o->base.type = &py_image_type;

    o->_cobj.w =w;
    o->_cobj.h =w;
    o->_cobj.bpp =bpp;
    o->_cobj.pixels =pixels;
    return o;
}

void *py_image_cobj(mp_obj_t image)
{
    PY_ASSERT_TYPE(image, &py_image_type);
    return &((py_image_obj_t *)image)->_cobj;
}


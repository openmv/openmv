#include <libmp.h>
#include "spi.h"
#include "py_assert.h"
#include "py_spi.h"
#include "imlib.h"
#include "py_image.h"

static mp_obj_t py_spi_read()
{
    return mp_obj_new_int(spi_read());
}

static mp_obj_t py_spi_write(mp_obj_t c)
{
    spi_write(mp_obj_get_int(c));
    return mp_const_true;
}

static mp_obj_t py_spi_write_image(mp_obj_t image_obj)
{
    struct image *image;
    /* get image pointer */
    image = (struct image*) py_image_cobj(image_obj);

    uint16_t *pixels = (uint16_t*)image->data;
    for (int j=0;j<image->h;j++) {
        for (int i=0;i<image->w;i++) {
            uint16_t c = pixels[j*image->w+i];
            spi_write(c);
            spi_write(c>>8);
        }
    }
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_spi_read_obj,   py_spi_read);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_spi_write_obj,  py_spi_write);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_spi_write_image_obj, py_spi_write_image);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_spi) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read),      (mp_obj_t)&py_spi_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write),     (mp_obj_t)&py_spi_write_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_image), (mp_obj_t)&py_spi_write_image_obj },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

static const mp_obj_module_t py_spi_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_spi,
    .globals = (mp_obj_t)&globals_dict,
};

const mp_obj_module_t *py_spi_init()
{
    /* Init spi */
    spi_init();
    return &py_spi_module;
}

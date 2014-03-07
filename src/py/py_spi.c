#include <libmp.h>
#include "spi.h"
#include "py_assert.h"
#include "py_spi.h"
#include "imlib.h"
#include "py_image.h"

mp_obj_t py_spi_read()
{
    return mp_obj_new_int(spi_read());
}

mp_obj_t py_spi_write(mp_obj_t c)
{
    spi_write(mp_obj_get_int(c));
    return mp_const_true;
}

mp_obj_t py_spi_write_image(mp_obj_t image_obj)
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

mp_obj_t py_spi_init()
{
    spi_init();

    /* Create module */
    mp_obj_t m = mp_obj_new_module(qstr_from_str("spi"));

    /* Export functions */
    rt_store_attr(m, qstr_from_str("read"), rt_make_function_n(0, py_spi_read));
    rt_store_attr(m, qstr_from_str("write"), rt_make_function_n(1, py_spi_write));
    rt_store_attr(m, qstr_from_str("write_image"), rt_make_function_n(1, py_spi_write_image));
    return m;
}

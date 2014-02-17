#include <libmp.h>
#include "imlib.h"
#include "array.h"
#include "sensor.h"
#include "py_image.h"
#include "py_imlib.h"
#include "py_assert.h"
#include "py_file.h"

typedef struct _py_cascade_obj_t {
    mp_obj_base_t base;
    struct cascade _cobj;
} py_cascade_obj_t;

extern struct sensor_dev sensor;

static void py_cascade_print(void (*print)(void *env, const char *fmt, ...), void *env, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_cascade_obj_t *self = self_in;

    /* print some info */
    print(env, "width:%d height:%d n_stages:%d n_features:%d n_rectangles:%d\n",
            self->_cobj.window.w,
            self->_cobj.window.h,
            self->_cobj.n_stages,
            self->_cobj.n_features,
            self->_cobj.n_rectangles);
}

static const mp_obj_type_t py_cascade_type = {
    { &mp_const_type },
    "Cascade",
    .print = py_cascade_print,
};

void *py_cascade_cobj(mp_obj_t cascade)
{
    PY_ASSERT_TYPE(cascade, &py_cascade_type);
    return &((py_cascade_obj_t *)cascade)->_cobj;
}

mp_obj_t py_imlib_histeq(mp_obj_t image_obj)
{
    struct image *image;
    /* get image pointer */
    image = (struct image*) py_image_cobj(image_obj);

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_GRAYSCALE);

    imlib_histeq(image);
    return mp_const_none;
}

mp_obj_t py_imlib_draw_rectangle(mp_obj_t image_obj, mp_obj_t rectangle_obj)
{
    struct rectangle r;
    struct image *image;
    mp_obj_t *array;

    array = mp_obj_get_array_fixed_n(rectangle_obj, 4);
    r.x = mp_obj_get_int(array[0]);
    r.y = mp_obj_get_int(array[1]);
    r.w = mp_obj_get_int(array[2]);
    r.h = mp_obj_get_int(array[3]);

    /* get image pointer */
    image = py_image_cobj(image_obj);

    imlib_draw_rectangle(image, &r);
    return mp_const_none;
}

mp_obj_t py_imlib_detect_color(mp_obj_t image_obj, mp_obj_t color_obj, mp_obj_t threshold)
{
    /* C stuff */
    struct color color;
    struct rectangle rectangle;
    struct image *image;

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_RGB565);

    mp_obj_t *col_obj;
    col_obj = mp_obj_get_array_fixed_n(color_obj, 3);
    color.h = mp_obj_get_int(col_obj[0]);
    color.s = mp_obj_get_int(col_obj[1]);
    color.v = mp_obj_get_int(col_obj[2]);

     /* get image pointer */
    image = py_image_cobj(image_obj);

    imlib_detect_color(image, &color, &rectangle, mp_obj_get_int(threshold));

    mp_obj_t rec_obj[4];
    rec_obj[0] = mp_obj_new_int(rectangle.x);
    rec_obj[1] = mp_obj_new_int(rectangle.y);
    rec_obj[2] = mp_obj_new_int(rectangle.w);
    rec_obj[3] = mp_obj_new_int(rectangle.h);
    return rt_build_tuple(4, rec_obj);
}

mp_obj_t py_imlib_detect_objects(mp_obj_t image_obj, mp_obj_t cascade_obj)
{
    struct image *image = NULL;
    struct cascade *cascade = NULL;

    struct array *objects_array=NULL;
    mp_obj_t objects_list = mp_const_none;

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.framesize <= FRAMESIZE_QQVGA);
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_GRAYSCALE);

    /* get C image pointer */
    image = py_image_cobj(image_obj);
    /* get C cascade pointer */
    cascade = py_cascade_cobj(cascade_obj);

    /* detect objects */
    objects_array = imlib_detect_objects(image, cascade);
    int size = array_length(objects_array);
    if (size) {
        int i;
        objects_list = rt_build_list(0, NULL);
        for (i=0; i<size; i++) {
            struct rectangle *r = array_at(objects_array, 0);
            mp_obj_t rec_obj[4];
            rec_obj[0] = mp_obj_new_int(r->x);
            rec_obj[1] = mp_obj_new_int(r->y);
            rec_obj[2] = mp_obj_new_int(r->w);
            rec_obj[3] = mp_obj_new_int(r->h);
            rt_list_append(objects_list, rt_build_tuple(4, rec_obj));
        }
    }

    /* free objects array */
    array_free(objects_array);

    return objects_list;
}

mp_obj_t py_imlib_load_cascade(mp_obj_t path_obj)
{
    py_cascade_obj_t *o =NULL;

    /* detection parameters */
    struct cascade cascade = {
        .step = 2,
        .scale_factor = 1.25f,
    };

    const char *path = mp_obj_str_get_str(path_obj);
    int res = imlib_load_cascade(&cascade, path);
    if (res != FR_OK) {
        nlr_jump(mp_obj_new_exception_msg(qstr_from_str("Imlib"), ffs_strerror(res)));
    }

    o = m_new_obj(py_cascade_obj_t);
    o->base.type = &py_cascade_type;
    o->_cobj = cascade;
    return o;
}

mp_obj_t py_imlib_load_template(mp_obj_t path_obj)
{
    mp_obj_t image_obj =NULL;
    struct image *image;
    const char *path = mp_obj_str_get_str(path_obj);
    image_obj = py_image(0, 0, 0, 0);

    /* get image pointer */
    image = py_image_cobj(image_obj);

    int res = imlib_load_template(image, path);
    if (res != FR_OK) {
        nlr_jump(mp_obj_new_exception_msg(qstr_from_str("Imlib"), ffs_strerror(res)));
    }

    return image_obj;
}

mp_obj_t py_imlib_save_template(mp_obj_t image_obj, mp_obj_t rectangle_obj, mp_obj_t path_obj)
{
    struct image t;
    struct image *image = NULL;

    struct rectangle r;
    mp_obj_t *array;

    const char *path = mp_obj_str_get_str(path_obj);

    array = mp_obj_get_array_fixed_n(rectangle_obj, 4);
    r.x = mp_obj_get_int(array[0]);
    r.y = mp_obj_get_int(array[1]);
    r.w = mp_obj_get_int(array[2]);
    r.h = mp_obj_get_int(array[3]);


    /* get C image pointer */
    image = py_image_cobj(image_obj);
    t.w = r.w;
    t.h = r.h;
    t.data = malloc(sizeof(*t.data)*t.w*t.h);

    imlib_subimage(image, &t, r.x, r.y);

    int res = imlib_save_template(&t, path);
    free(t.data);

    if (res != FR_OK) {
        nlr_jump(mp_obj_new_exception_msg(qstr_from_str("Imlib"), ffs_strerror(res)));
    }

    return mp_const_true;
}

mp_obj_t py_imlib_template_match(mp_obj_t image_obj, mp_obj_t template_obj, mp_obj_t threshold)
{
    struct rectangle r;
    struct image *image = NULL;
    struct image *template = NULL;
    mp_obj_t rec_obj[4];
    mp_obj_t obj=mp_const_none;

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.framesize <= FRAMESIZE_QQVGA);
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_GRAYSCALE);

    /* get C image pointer */
    image = py_image_cobj(image_obj);
    template= py_image_cobj(template_obj);

    float t = mp_obj_get_float(threshold);

    /* look for object */
    if (imlib_template_match(image, template, &r)> t) {
        rec_obj[0] = mp_obj_new_int(r.x);
        rec_obj[1] = mp_obj_new_int(r.y);
        rec_obj[2] = mp_obj_new_int(r.w);
        rec_obj[3] = mp_obj_new_int(r.h);
        obj=rt_build_tuple(4, rec_obj);
    }
    return obj;
}

mp_obj_t py_imlib_blit(mp_obj_t image_obj, mp_obj_t template_obj)
{
    struct image *image = NULL;
    struct image *template = NULL;

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.framesize <= FRAMESIZE_QQVGA);
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_GRAYSCALE);

    /* get C image pointer */
    image = py_image_cobj(image_obj);
    template= py_image_cobj(template_obj);
    imlib_blit(image, template, 0, 0);

    return mp_const_true;
}

void py_imlib_print(void (*print)(void *env, const char *fmt, ...), void *env, mp_obj_t self_in, mp_print_kind_t kind)
{
//print(env, "<image width:%d height:%d bpp:%d>", self->width, self->height, self->bpp);
}

mp_obj_t py_imlib_init()
{
    /* Create module */
    mp_obj_t m = mp_obj_new_module(qstr_from_str("imlib"));

    /* Export functions */
    rt_store_attr(m, qstr_from_str("blit"), rt_make_function_n(2, py_imlib_blit));
    rt_store_attr(m, qstr_from_str("load_cascade"), rt_make_function_n(1, py_imlib_load_cascade));
    rt_store_attr(m, qstr_from_str("load_template"), rt_make_function_n(1, py_imlib_load_template));
    rt_store_attr(m, qstr_from_str("save_template"), rt_make_function_n(3, py_imlib_save_template));
    rt_store_attr(m, qstr_from_str("template_match"), rt_make_function_n(3, py_imlib_template_match));
    rt_store_attr(m, qstr_from_str("histeq"), rt_make_function_n(1, py_imlib_histeq));
    rt_store_attr(m, qstr_from_str("draw_rectangle"), rt_make_function_n(2, py_imlib_draw_rectangle));
    rt_store_attr(m, qstr_from_str("detect_color"), rt_make_function_n(3, py_imlib_detect_color));
    rt_store_attr(m, qstr_from_str("detect_objects"), rt_make_function_n(2, py_imlib_detect_objects));

    return m;
}

//MP_DEFINE_CONST_FUN_OBJ_0(image_obj, image);

#if 0
typedef struct _mp_obj_array_it_t {
    mp_obj_base_t base;
    mp_obj_array_t *array;
    machine_uint_t cur;
} mp_obj_array_it_t;

mp_obj_t array_it_iternext(mp_obj_t self_in) {
    mp_obj_array_it_t *self = self_in;
    if (self->cur < self->array->len) {
        machine_int_t val = array_get_el(self->array, self->cur++);
        return mp_obj_new_int(val);
    } else {
        return mp_const_stop_iteration;
    }
}

static const mp_obj_type_t array_it_type = {
    { &mp_const_type },
    "array_iterator",
    .iternext = array_it_iternext,
};

mp_obj_t array_iterator_new(mp_obj_t array_in) {
    mp_obj_array_t *array = array_in;
    mp_obj_array_it_t *o = m_new_obj(mp_obj_array_it_t);
    o->base.type = &array_it_type;
    o->array = array;
    o->cur = 0;
    return o;
}
#endif

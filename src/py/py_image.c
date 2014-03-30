#include <libmp.h>
#include "xalloc.h"
#include "imlib.h"
#include "array.h"
#include "sensor.h"
#include "py_image.h"
#include "py_assert.h"
#include "py_file.h"

extern struct sensor_dev sensor;
static const mp_obj_type_t py_cascade_type;
static const mp_obj_type_t py_surf_type;
static const mp_obj_type_t py_image_type;

/* Haar Cascade */
typedef struct _py_cascade_obj_t {
    mp_obj_base_t base;
    struct cascade _cobj;
} py_cascade_obj_t;

void *py_cascade_cobj(mp_obj_t cascade)
{
    PY_ASSERT_TYPE(cascade, &py_cascade_type);
    return &((py_cascade_obj_t *)cascade)->_cobj;
}

static void py_cascade_print(void (*print)(void *env, const char *fmt, ...), void *env, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_cascade_obj_t *self = self_in;
    /* print some info */
    print(env, "width:%d height:%d n_stages:%d n_features:%d n_rectangles:%d\n", self->_cobj.window.w,
          self->_cobj.window.h, self->_cobj.n_stages, self->_cobj.n_features, self->_cobj.n_rectangles);
}

static const mp_obj_type_t py_cascade_type = {
    { &mp_type_type },
    .name  = MP_QSTR_Cascade,
    .print = py_cascade_print,
};

/* SURF Detector */
typedef struct _py_surf_obj_t {
    mp_obj_base_t base;
    struct surf _cobj;
} py_surf_obj_t;

void *py_surf_cobj(mp_obj_t surf)
{
    PY_ASSERT_TYPE(surf, &py_surf_type);
    return &((py_surf_obj_t *)surf)->_cobj;
}

static void py_surf_print(void (*print)(void *env, const char *fmt, ...), void *env, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_surf_obj_t *self = self_in;
    /* print some info */
    print(env, "upright:%d octaves:%d intervals:%d init_sample:%d thresh:%f\n",
          self->_cobj.upright, self->_cobj.octaves, self->_cobj.intervals, (double) self->_cobj.thresh);
}

static const mp_obj_type_t py_surf_type = {
    { &mp_type_type },
    .name  = MP_QSTR_Surf,
    .print = py_surf_print,
    .print = NULL,
};

/* Image */
typedef struct _py_image_obj_t {
    mp_obj_base_t base;
    struct image _cobj;
} py_image_obj_t;

void *py_image_cobj(mp_obj_t image)
{
    PY_ASSERT_TYPE(image, &py_image_type);
    return &((py_image_obj_t *)image)->_cobj;
}

static void py_image_print(void (*print)(void *env, const char *fmt, ...), void *env, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_image_obj_t *self = self_in;
    print(env, "<image width:%d height:%d bpp:%d>", self->_cobj.w, self->_cobj.h, self->_cobj.bpp);
}

static mp_obj_t py_image_histeq(mp_obj_t image_obj)
{
    struct image *image;
    /* get image pointer */
    image = (struct image*) py_image_cobj(image_obj);

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_GRAYSCALE);

    imlib_histeq(image);
    return mp_const_none;
}

static mp_obj_t py_image_median(mp_obj_t image_obj, mp_obj_t ksize)
{
    struct image *image;
    /* get image pointer */
    image = (struct image*) py_image_cobj(image_obj);

    /* sanity checks */
    //PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_GRAYSCALE);

    imlib_median_filter(image, mp_obj_get_int(ksize));
    return mp_const_none;
}

static mp_obj_t py_image_draw_rectangle(mp_obj_t image_obj, mp_obj_t rectangle_obj)
{
    struct rectangle r;
    struct image *image;
    mp_obj_t *array;

    mp_obj_get_array_fixed_n(rectangle_obj, 4, &array);
    r.x = mp_obj_get_int(array[0]);
    r.y = mp_obj_get_int(array[1]);
    r.w = mp_obj_get_int(array[2]);
    r.h = mp_obj_get_int(array[3]);

    /* get image pointer */
    image = py_image_cobj(image_obj);

    imlib_draw_rectangle(image, &r);
    return mp_const_none;
}

static mp_obj_t py_image_draw_circle(mp_obj_t image_obj, mp_obj_t c_obj, mp_obj_t r_obj)
{
    int cx, cy, r;
    mp_obj_t *array;
    struct image *image;

    /* get image pointer */
    image = py_image_cobj(image_obj);

    /* center */
    mp_obj_get_array_fixed_n(c_obj, 2, &array);
    cx = mp_obj_get_int(array[0]);
    cy = mp_obj_get_int(array[1]);

    /* radius */
    r = mp_obj_get_int(r_obj);
    imlib_draw_circle(image, cx, cy, r);
    return mp_const_none;
}

static mp_obj_t py_image_threshold(mp_obj_t image_obj, mp_obj_t color_obj, mp_obj_t threshold)
{
    /* C stuff */
    struct color color;
    struct image *image;

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_RGB565);

    mp_obj_t *col_obj;
    mp_obj_get_array_fixed_n(color_obj, 3, &col_obj);
    color.r = mp_obj_get_int(col_obj[0]);
    color.g = mp_obj_get_int(col_obj[1]);
    color.b = mp_obj_get_int(col_obj[2]);

     /* get image pointer */
    image = py_image_cobj(image_obj);

    /* Threshold image using reference color */
    imlib_threshold(image, &color, mp_obj_get_int(threshold));

    return mp_const_none;
}

static mp_obj_t py_image_count_blobs(mp_obj_t image_obj)
{
    /* C stuff */
    array_t *blobs;
    struct image *image;

    /* MP List */
    mp_obj_t objects_list = mp_const_none;

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_RGB565);

     /* get image pointer */
    image = py_image_cobj(image_obj);

    /* run color dector */
    blobs = imlib_count_blobs(image);

    /* Create empty Python list */
    objects_list = rt_build_list(0, NULL);

    if (array_length(blobs)) {
        for (int j=0; j<array_length(blobs); j++) {
             rectangle_t *b = array_at(blobs, j);
             mp_obj_t r[4];
             r[0] = mp_obj_new_int(b->x);
             r[1] = mp_obj_new_int(b->y);
             r[2] = mp_obj_new_int(b->w);
             r[3] = mp_obj_new_int(b->h);
             rt_list_append(objects_list, rt_build_tuple(4, r));
        }
    }
    array_free(blobs);
    return objects_list;
}

static mp_obj_t py_image_detect_objects(mp_obj_t image_obj, mp_obj_t cascade_obj)
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

    /* Detect objects */
    objects_array = imlib_detect_objects(image, cascade);

    /* Create empty Python list */
    objects_list = rt_build_list(0, NULL);

    /* Add detected objects to the list */
    for (int i=0; i<array_length(objects_array); i++) {
        struct rectangle *r = array_at(objects_array, 0);
        mp_obj_t rec_obj[4];
        rec_obj[0] = mp_obj_new_int(r->x);
        rec_obj[1] = mp_obj_new_int(r->y);
        rec_obj[2] = mp_obj_new_int(r->w);
        rec_obj[3] = mp_obj_new_int(r->h);
        rt_list_append(objects_list, rt_build_tuple(4, rec_obj));
    }

    /* Free the objects array */
    array_free(objects_array);

    return objects_list;
}

static mp_obj_t py_image_load_cascade(mp_obj_t path_obj)
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
        nlr_jump(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    o = m_new_obj(py_cascade_obj_t);
    o->base.type = &py_cascade_type;
    o->_cobj = cascade;
    return o;
}

static mp_obj_t py_image_load_template(mp_obj_t path_obj)
{
    mp_obj_t image_obj =NULL;
    struct image *image;
    const char *path = mp_obj_str_get_str(path_obj);
    image_obj = py_image(0, 0, 0, 0);

    /* get image pointer */
    image = py_image_cobj(image_obj);

    int res = imlib_load_template(image, path);
    if (res != FR_OK) {
        nlr_jump(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    return image_obj;
}

static mp_obj_t py_image_save_template(mp_obj_t image_obj, mp_obj_t rectangle_obj, mp_obj_t path_obj)
{
    struct image t;
    struct image *image = NULL;

    struct rectangle r;
    mp_obj_t *array;

    const char *path = mp_obj_str_get_str(path_obj);

    mp_obj_get_array_fixed_n(rectangle_obj, 4, &array);
    r.x = mp_obj_get_int(array[0]);
    r.y = mp_obj_get_int(array[1]);
    r.w = mp_obj_get_int(array[2]);
    r.h = mp_obj_get_int(array[3]);


    /* get C image pointer */
    image = py_image_cobj(image_obj);
    t.w = r.w;
    t.h = r.h;
    t.data = xalloc(sizeof(*t.data)*t.w*t.h); /* TODO this is not really needed */

    imlib_subimage(image, &t, r.x, r.y);

    int res = imlib_save_template(&t, path);
    xfree(t.data);

    if (res != FR_OK) {
        nlr_jump(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    return mp_const_true;
}

static mp_obj_t py_image_template_match(mp_obj_t image_obj, mp_obj_t template_obj, mp_obj_t threshold)
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
    float corr = imlib_template_match(image, template, &r);
    if (corr> t) {
        rec_obj[0] = mp_obj_new_int(r.x);
        rec_obj[1] = mp_obj_new_int(r.y);
        rec_obj[2] = mp_obj_new_int(r.w);
        rec_obj[3] = mp_obj_new_int(r.h);
        obj=rt_build_tuple(4, rec_obj);
    }
    return obj;
}

static mp_obj_t py_image_blit(mp_obj_t image_obj, mp_obj_t template_obj)
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

static mp_obj_t py_image_surf_detector(mp_obj_t image_obj, mp_obj_t upright, mp_obj_t thresh)
{
    struct image *image;
    py_surf_obj_t *o =NULL;

    surf_t surf = {
        .upright=mp_obj_get_int(upright),
        .octaves=5,
        .init_sample=2,
        .thresh=mp_obj_get_float(thresh),
    };

    /* get image pointer */
    image = (struct image*) py_image_cobj(image_obj);

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_GRAYSCALE);

    /* run SURF detector */
    surf_detector(image, &surf);

    o = m_new_obj(py_surf_obj_t);
    o->base.type = &py_surf_type;
    o->_cobj = surf;
    return o;
}

static mp_obj_t py_image_surf_match(mp_obj_t image_obj, mp_obj_t surf1_obj, mp_obj_t surf2_obj)
{
    surf_t *surf1 = NULL;
    surf_t *surf2 = NULL;
    image_t *image = NULL;

    /* get C image pointer */
    image = py_image_cobj(image_obj);
    /* get C cascade pointer */
    surf1 = py_surf_cobj(surf1_obj);
    surf2 = py_surf_cobj(surf2_obj);

    /* Detect objects */
    array_t *match = surf_match(surf1, surf2);
    surf_draw_ipts(image, match);
    array_free(match); //TODO
    array_free(surf2->ipts); //TODO
    return mp_const_none;
}

static mp_obj_t py_image_surf_draw_ipts(mp_obj_t image_obj, mp_obj_t surf_obj)
{
    surf_t *surf = NULL;
    image_t *image = NULL;

    /* get C image pointer */
    image = py_image_cobj(image_obj);

    /* get C cascade pointer */
    surf = py_surf_cobj(surf_obj);

    surf_draw_ipts(image, surf->ipts);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_blit_obj,              py_image_blit);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_load_cascade_obj,      py_image_load_cascade);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_load_template_obj,     py_image_load_template);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_save_template_obj,     py_image_save_template);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_template_match_obj,    py_image_template_match);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_histeq_obj,            py_image_histeq);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_median_obj,            py_image_median);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_draw_rectangle_obj,    py_image_draw_rectangle);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_draw_circle_obj,       py_image_draw_circle);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_threshold_obj,         py_image_threshold);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_count_blobs_obj,       py_image_count_blobs);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_detect_objects_obj,    py_image_detect_objects);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_surf_detector_obj,     py_image_surf_detector);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_surf_match_obj,        py_image_surf_match);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_surf_draw_ipts_obj,    py_image_surf_draw_ipts);

static const mp_method_t py_image_methods[] = {
    {"blit",              &py_image_blit_obj},
    {"load_cascade",      &py_image_load_cascade_obj},
    {"load_template",     &py_image_load_template_obj},
    {"save_template",     &py_image_save_template_obj},
    {"template_match",    &py_image_template_match_obj},
    {"histeq",            &py_image_histeq_obj},
    {"median",            &py_image_median_obj},
    {"draw_rectangle",    &py_image_draw_rectangle_obj},
    {"draw_circle",       &py_image_draw_circle_obj},
    {"threshold",         &py_image_threshold_obj},
    {"count_blobs",       &py_image_count_blobs_obj},
    {"detect_objects",    &py_image_detect_objects_obj},
    {"surf_detector",     &py_image_surf_detector_obj},
    {"surf_match",        &py_image_surf_match_obj},
    {"surf_draw_ipts",    &py_image_surf_draw_ipts_obj},
    { NULL, NULL },
};

static const mp_obj_type_t py_image_type = {
    { &mp_type_type },
    .name       = MP_QSTR_Image,
    .print      = py_image_print,
    .methods    = py_image_methods,
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

#include "mp.h"
#include "imlib.h"
#include "array.h"
#include "sensor.h"
#include "py_assert.h"
#include "py_image.h"

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

static mp_obj_t py_image_save(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    rectangle_t r;
    image_t *image = py_image_cobj(args[0]);
    const char *path = mp_obj_str_get_str(args[1]);

    mp_map_elem_t *kw_subimage = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("subimage")), MP_MAP_LOOKUP);
    if (kw_subimage != NULL) {
        mp_obj_t *array;
        mp_obj_get_array_fixed_n(kw_subimage->value, 4, &array);
        r.x = mp_obj_get_int(array[0]);
        r.y = mp_obj_get_int(array[1]);
        r.w = mp_obj_get_int(array[2]);
        r.h = mp_obj_get_int(array[3]);
    }

    int res;
    if ((res=imlib_save_image(image, path, &r)) != FR_OK) {
        nlr_jump(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    return mp_const_true;
}

static mp_obj_t py_image_blit(mp_obj_t dst_image_obj, mp_obj_t offset_obj, mp_obj_t src_image_obj)
{
    int x,y;
    image_t *src_image = NULL;
    image_t *dst_image = NULL;

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.framesize <= FRAMESIZE_QQVGA);
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_GRAYSCALE);

    /* get C image pointer */
    src_image = py_image_cobj(src_image_obj);
    dst_image = py_image_cobj(dst_image_obj);

    /* get x,y */
    mp_obj_t *array;
    mp_obj_get_array_fixed_n(offset_obj, 2, &array);
    x = mp_obj_get_int(array[0]);
    y = mp_obj_get_int(array[1]);

    if ((src_image->w+x)>dst_image->w ||
        (src_image->h+y)>dst_image->h) {
        printf("src image > dst image\n");
        return mp_const_none;
    }

    imlib_blit(dst_image, src_image, x, y);
    return mp_const_true;
}

static mp_obj_t py_image_histeq(mp_obj_t image_obj)
{
    struct image *image;
    /* get image pointer */
    image = (struct image*) py_image_cobj(image_obj);

    /* sanity checks */
    PY_ASSERT_TRUE(image->bpp == 1);

    imlib_histeq(image);
    return mp_const_none;
}

static mp_obj_t py_image_median(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    int ksize = 1;
    /* get image pointer */
    image_t *image = py_image_cobj(args[0]);

    mp_map_elem_t *kw_ksize = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("size")), MP_MAP_LOOKUP);
    if (kw_ksize != NULL) {
        ksize = mp_obj_get_int(kw_ksize->value);
    }

    imlib_median_filter(image, ksize);
    return mp_const_none;
}

static mp_obj_t py_image_threshold(mp_obj_t image_obj, mp_obj_t color_obj, mp_obj_t threshold)
{
    /* C stuff */
    struct color color;
    struct image *image;
    mp_obj_t bimage_obj;

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_RGB565);
    PY_ASSERT_TRUE(sensor.framesize <= FRAMESIZE_QQVGA);

    mp_obj_t *col_obj;
    mp_obj_get_array_fixed_n(color_obj, 3, &col_obj);
    color.r = mp_obj_get_int(col_obj[0]);
    color.g = mp_obj_get_int(col_obj[1]);
    color.b = mp_obj_get_int(col_obj[2]);

     /* get image pointer */
    image = py_image_cobj(image_obj);

    /* returned image */
    bimage_obj = py_image(image->w, image->h, 1, image->data+(image->w*image->h*image->bpp));

    /* Threshold image using reference color */
    imlib_threshold(image, py_image_cobj(bimage_obj), &color, mp_obj_get_int(threshold));

    return bimage_obj;
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

static mp_obj_t py_image_draw_keypoints(mp_obj_t image_obj, mp_obj_t surf_obj)
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

static mp_obj_t py_image_find_blobs(mp_obj_t image_obj)
{
    /* C stuff */
    array_t *blobs;
    struct image *image;
    mp_obj_t rec_obj[4];

    /* MP List */
    mp_obj_t objects_list = mp_const_none;

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_RGB565);

     /* get image pointer */
    image = py_image_cobj(image_obj);

    /* run color dector */
    blobs = imlib_count_blobs(image);

    /* Create empty Python list */
    objects_list = mp_obj_new_list(0, NULL);

    if (array_length(blobs)) {
        for (int j=0; j<array_length(blobs); j++) {
             rectangle_t *r = array_at(blobs, j);
             rec_obj[0] = mp_obj_new_int(r->x);
             rec_obj[1] = mp_obj_new_int(r->y);
             rec_obj[2] = mp_obj_new_int(r->w);
             rec_obj[3] = mp_obj_new_int(r->h);
             mp_obj_list_append(objects_list, mp_obj_new_tuple(4, rec_obj));
        }
    }
    array_free(blobs);
    return objects_list;
}

static mp_obj_t py_image_find_features(mp_obj_t image_obj, mp_obj_t cascade_obj)
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
    objects_list = mp_obj_new_list(0, NULL);

    /* Add detected objects to the list */
    for (int i=0; i<array_length(objects_array); i++) {
        struct rectangle *r = array_at(objects_array, 0);
        mp_obj_t rec_obj[4];
        rec_obj[0] = mp_obj_new_int(r->x);
        rec_obj[1] = mp_obj_new_int(r->y);
        rec_obj[2] = mp_obj_new_int(r->w);
        rec_obj[3] = mp_obj_new_int(r->h);
        mp_obj_list_append(objects_list, mp_obj_new_tuple(4, rec_obj));
    }

    /* Free the objects array */
    array_free(objects_array);

    return objects_list;
}

static mp_obj_t py_image_find_template(mp_obj_t image_obj, mp_obj_t template_obj, mp_obj_t threshold)
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
        obj = mp_obj_new_tuple(4, rec_obj);
    }
    return obj;
}

static mp_obj_t py_image_find_keypoints(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    py_surf_obj_t *o =NULL;

    surf_t surf = {
        .upright=false,
        .octaves=2,
        .init_sample=2,
        .thresh=0.0004f,
    };

    /* get image pointer */
    image_t *image = py_image_cobj(args[0]);

    /* sanity checks */
    PY_ASSERT_TRUE(sensor.pixformat == PIXFORMAT_GRAYSCALE);

    /* read KW args */
    mp_map_elem_t *kw_upright = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("upright")), MP_MAP_LOOKUP);
    if (kw_upright != NULL) {
        surf.upright = mp_obj_get_int(kw_upright->value);
    }

    mp_map_elem_t *kw_octaves = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("octaves")), MP_MAP_LOOKUP);
    if (kw_octaves != NULL) {
        surf.octaves = mp_obj_get_int(kw_octaves->value);
    }

    mp_map_elem_t *kw_thresh = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("thresh")), MP_MAP_LOOKUP);
    if (kw_thresh != NULL) {
        surf.thresh = mp_obj_get_float(kw_thresh->value);
    }

    /* run SURF detector */
    surf_detector(image, &surf);

    o = m_new_obj(py_surf_obj_t);
    o->base.type = &py_surf_type;
    o->_cobj = surf;
    return o;
}

static mp_obj_t py_image_find_keypoints_match(mp_obj_t image_obj, mp_obj_t surf1_obj)
{
    surf_t *surf1 = NULL, surf2;
    image_t *image = NULL;

    /* get C image pointer */
    image = py_image_cobj(image_obj);

    /* get C cascade pointer */
    surf1 = py_surf_cobj(surf1_obj);
    surf2 = *surf1; //use same parameters

    /* run SURF detector */
    surf_detector(image, &surf2);

    /* Detect objects */
    array_t *match = surf_match(surf1, &surf2);

    // TODO return matching kpts
    surf_draw_ipts(image, match);
    array_free(match);
    array_free(surf2.ipts);

    return mp_const_none;
}

mp_obj_t py_image_load_image(mp_obj_t path_obj)
{
    mp_obj_t image_obj =NULL;
    struct image *image;
    const char *path = mp_obj_str_get_str(path_obj);
    image_obj = py_image(0, 0, 0, 0);

    /* get image pointer */
    image = py_image_cobj(image_obj);

    int res = imlib_load_image(image, path);
    if (res != FR_OK) {
        nlr_jump(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    return image_obj;
}

mp_obj_t py_image_load_cascade(mp_obj_t path_obj)
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

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_save_obj, 2, py_image_save);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_blit_obj, py_image_blit);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_histeq_obj, py_image_histeq);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_median_obj, 1, py_image_median);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_threshold_obj, py_image_threshold);

STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_draw_circle_obj, py_image_draw_circle);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_draw_rectangle_obj, py_image_draw_rectangle);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_draw_keypoints_obj, py_image_draw_keypoints);

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_find_blobs_obj, py_image_find_blobs);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_find_template_obj, py_image_find_template);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_find_features_obj, py_image_find_features);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_keypoints_obj, 1, py_image_find_keypoints);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_find_keypoints_match_obj, py_image_find_keypoints_match);

static const mp_map_elem_t locals_dict_table[] = {
    /* basic image functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_save),                (mp_obj_t)&py_image_save_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_blit),                (mp_obj_t)&py_image_blit_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_histeq),              (mp_obj_t)&py_image_histeq_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_median),              (mp_obj_t)&py_image_median_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_threshold),           (mp_obj_t)&py_image_threshold_obj},

    /* drawing functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_circle),         (mp_obj_t)&py_image_draw_circle_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_rectangle),      (mp_obj_t)&py_image_draw_rectangle_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_keypoints),      (mp_obj_t)&py_image_draw_keypoints_obj},

    /* objects/feature detection */
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_blobs),          (mp_obj_t)&py_image_find_blobs_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_template),       (mp_obj_t)&py_image_find_template_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_features),       (mp_obj_t)&py_image_find_features_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_keypoints),      (mp_obj_t)&py_image_find_keypoints_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_keypoints_match),(mp_obj_t)&py_image_find_keypoints_match_obj},

    { NULL, NULL },
};

STATIC MP_DEFINE_CONST_DICT(locals_dict, locals_dict_table);

static const mp_obj_type_t py_image_type = {
    { &mp_type_type },
    .name  = MP_QSTR_image,
    .print = py_image_print,
    .locals_dict = (mp_obj_t)&locals_dict,
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

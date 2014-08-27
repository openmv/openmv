#include "mp.h"
#include "imlib.h"
#include "array.h"
#include "sensor.h"
#include "xalloc.h"
#include "py_assert.h"
#include "py_image.h"
#include "py_file.h"

extern struct sensor_dev sensor;
static const mp_obj_type_t py_cascade_type;
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

/* Keypoints object */
typedef struct _py_kp_obj_t {
    mp_obj_base_t base;
    int size;
    kp_t *kpts;
    int threshold;
    bool normalized;
} py_kp_obj_t;

static void py_kp_print(void (*print)(void *env, const char *fmt, ...), void *env, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_kp_obj_t *self = self_in;
    print(env, "size:%d threshold:%d normalized:%d\n", self->size, self->threshold, self->normalized);
}

static const mp_obj_type_t py_kp_type = {
    { &mp_type_type },
    .name  = MP_QSTR_kp_desc,
    .print = py_kp_print,
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

static mp_int_t py_image_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, int flags) {
    image_t *image = py_image_cobj(self_in);

    if (flags == MP_BUFFER_READ) {
        bufinfo->buf = (void*)image->pixels;
        if (image->bpp > 2) { //JPEG
            bufinfo->len = image->bpp;
        } else {
            bufinfo->len = image->w*image->h*image->bpp;
        }
        bufinfo->typecode = 'b';
        return 0;
    } else {
        // disable write for now
        bufinfo->buf = NULL;
        bufinfo->len = 0;
        bufinfo->typecode = -1;
        return 1;
    }
}

static mp_obj_t py_image_size(mp_obj_t self_in)
{
    uint32_t len;
    image_t *image = py_image_cobj(self_in);
    if (image->bpp > 2) { //JPEG
        len = image->bpp;
    } else {
        len = image->w*image->h*image->bpp;
    }
    return mp_obj_new_int(len);
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
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    return mp_const_true;
}

static mp_obj_t py_image_scale(mp_obj_t image_obj, mp_obj_t size_obj)
{
    int w,h;
    mp_obj_t *array;
    image_t *src_image = NULL;

    /* get C image pointer */
    src_image = py_image_cobj(image_obj);

    /* get x,y */
    mp_obj_get_array_fixed_n(size_obj, 2, &array);
    w = mp_obj_get_int(array[0]);
    h = mp_obj_get_int(array[1]);

    image_t dst_image = {
        .w=w,
        .h=h,
        .bpp=src_image->bpp,
        .pixels=xalloc(w*h*src_image->bpp)
    };

    imlib_scale(src_image, &dst_image, INTERP_BILINEAR);

    *src_image = dst_image;
    return image_obj;
}

static mp_obj_t py_image_scaled(mp_obj_t image_obj, mp_obj_t size_obj)
{
    int w,h;
    mp_obj_t *array;
    image_t *src_image = NULL;

    /* get C image pointer */
    src_image = py_image_cobj(image_obj);

    /* get x,y */
    mp_obj_get_array_fixed_n(size_obj, 2, &array);
    w = mp_obj_get_int(array[0]);
    h = mp_obj_get_int(array[1]);

    image_t dst_image = {
        .w=w,
        .h=h,
        .bpp=src_image->bpp,
        .pixels=xalloc(w*h*src_image->bpp)
    };

    imlib_scale(src_image, &dst_image, INTERP_NEAREST);

    return py_image_from_struct(&dst_image);
}


static mp_obj_t py_image_blit(mp_obj_t dst_image_obj, mp_obj_t src_image_obj, mp_obj_t offset_obj)
{
    int x,y;
    image_t *src_image = NULL;
    image_t *dst_image = NULL;

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

    imlib_blit(src_image, dst_image, x, y);
    return mp_const_none;
}

static mp_obj_t py_image_blend(mp_obj_t dst_image_obj,
        mp_obj_t src_image_obj, mp_obj_t param_obj)
{
    int x,y;
    float alpha;
    image_t *src_image = NULL;
    image_t *dst_image = NULL;

    /* get C image pointer */
    src_image = py_image_cobj(src_image_obj);
    dst_image = py_image_cobj(dst_image_obj);

    /* get x,y,alpha */
    mp_obj_t *array;
    mp_obj_get_array_fixed_n(param_obj, 3, &array);
    x = mp_obj_get_int(array[0]);
    y = mp_obj_get_int(array[1]);
    alpha = mp_obj_get_float(array[2]);

    if ((src_image->w+x)>dst_image->w ||
        (src_image->h+y)>dst_image->h) {
        printf("src image > dst image\n");
        return mp_const_none;
    }

    imlib_blend(src_image, dst_image, x, y, (uint8_t)(alpha*256));
    return mp_const_none;
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

static mp_obj_t py_image_rainbow(mp_obj_t src_image_obj)
{
    image_t *src_image = NULL;

    /* get C image pointer */
    src_image = py_image_cobj(src_image_obj);
    /* sanity checks */
    PY_ASSERT_TRUE(src_image->bpp==1);

    image_t dst_image = {
        .w=src_image->w,
        .h=src_image->h,
        .bpp=2,
        .pixels=xalloc(src_image->w*src_image->h*2)
    };

    imlib_rainbow(src_image, &dst_image);
    *src_image = dst_image;
    return src_image_obj;
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

static mp_obj_t py_image_draw_keypoints(mp_obj_t image_obj, mp_obj_t kp_obj)
{
//    int r = 2;
//    for (int i=0; i<kpts_size; i++) {
//        kp_t *kp = &kpts[i];
//        if (kp->x&&kp->y) {
//            if (((kp->x+r) < image->w) && ((kp->y+r) < image->h)) {
//                imlib_draw_circle(image, kp->x, kp->y, r);
//            }
//        }
//    }

//    kp_t *kp = NULL;
//    image_t *image = NULL;
//
//    /* get C image pointer */
//    image = py_image_cobj(image_obj);
//
//    /* get C cascade pointer */
//    kp = py_kp_cobj(kp_obj);
//
//    kp_draw_ipts(image, kp->ipts);
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
    //printf("t:%f coor:%f\n", (double)t, (double)corr);
    if (corr > t) {
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
    int threshold = 20;
    bool normalized = false;
    py_kp_obj_t *kp_obj =NULL;

    /* make sure image is grayscale */
    image_t *image = py_image_cobj(args[0]);
    PY_ASSERT_TRUE(image->bpp == 1);

    /* read var args */ //TODO
    mp_map_elem_t *kw_thresh = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("threshold")), MP_MAP_LOOKUP);
    if (kw_thresh != NULL) {
        threshold = mp_obj_get_int(kw_thresh->value);
    }
    mp_map_elem_t *kw_norm = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("normalized")), MP_MAP_LOOKUP);
    if (kw_norm != NULL) {
        normalized = mp_obj_get_int(kw_norm->value);
    }

    /* run keypoint extractor */
    int kpts_size = 0;
    kp_t *kpts = fast_detect(image, threshold, &kpts_size);
    freak_find_keypoints(image, kpts, kpts_size, normalized, normalized);

    /* return keypoints MP object */
    kp_obj = m_new_obj(py_kp_obj_t);
    kp_obj->base.type = &py_kp_type;
    kp_obj->kpts = kpts;
    kp_obj->size = kpts_size;
    kp_obj->threshold = threshold;
    kp_obj->normalized = normalized;
    return kp_obj;
}

static mp_obj_t py_image_match_keypoints(mp_obj_t image_obj, mp_obj_t kpts_obj, mp_obj_t threshold_obj)
{
    kp_t *kpts1;
    kp_t *kpts2;

    int kpts1_size = 0;
    int kpts2_size = 0;

    int d_thresh; // detection threshold
    int m_thresh; // matching threshold
    bool normalized;

    // get image obj
    image_t *image = NULL;
    image = py_image_cobj(image_obj);

    // sanity checks
    PY_ASSERT_TRUE(image->bpp == 1);
    PY_ASSERT_TYPE(kpts_obj, &py_kp_type);

    // get kpts info
    kpts1       = ((py_kp_obj_t*)kpts_obj)->kpts;
    kpts1_size  = ((py_kp_obj_t*)kpts_obj)->size;
    normalized  = ((py_kp_obj_t*)kpts_obj)->normalized;
    d_thresh    = ((py_kp_obj_t*)kpts_obj)->threshold;
    m_thresh    = mp_obj_get_int(threshold_obj);

    // run keypoint extractor using the same params
    kpts2 = fast_detect(image, d_thresh, &kpts2_size);
    freak_find_keypoints(image, kpts2, kpts2_size, normalized, normalized);

    if (kpts1_size == 0 || kpts2_size == 0) {
        return mp_const_none;
    }

    // match the keypoint sets
    int16_t *kpts_match = freak_match_keypoints(kpts1, kpts1_size, kpts2, kpts2_size, m_thresh);

    // do something with the match
    for (int i=0; i<kpts1_size; i++) {
        if (kpts_match[i] != -1) {
            kp_t *kp1 = &kpts1[i];
            kp_t *kp2 = &kpts2[kpts_match[i]];
            imlib_draw_line(image, kp1->x, kp1->y, kp2->x, kp2->y);
            //int r = 4;
            //                if (((kp->x+r) < image->w) && ((kp->y+r) < image->h)) {
            //                    imlib_draw_circle(image, kp->x, kp->y, r);
            //                }
        }
    }

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
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
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
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    o = m_new_obj(py_cascade_obj_t);
    o->base.type = &py_cascade_type;
    o->_cobj = cascade;
    return o;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_size_obj, py_image_size);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_save_obj, 2, py_image_save);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_scale_obj, py_image_scale);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_scaled_obj, py_image_scaled);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_blit_obj, py_image_blit);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_blend_obj, py_image_blend);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_histeq_obj, py_image_histeq);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_median_obj, 1, py_image_median);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_threshold_obj, py_image_threshold);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_rainbow_obj, py_image_rainbow);

STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_draw_circle_obj, py_image_draw_circle);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_draw_rectangle_obj, py_image_draw_rectangle);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_draw_keypoints_obj, py_image_draw_keypoints);

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_find_blobs_obj, py_image_find_blobs);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_find_template_obj, py_image_find_template);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_find_features_obj, py_image_find_features);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_keypoints_obj, 1, py_image_find_keypoints);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_match_keypoints_obj, py_image_match_keypoints);

static const mp_map_elem_t locals_dict_table[] = {
    {MP_OBJ_NEW_QSTR(MP_QSTR_size),                (mp_obj_t)&py_image_size_obj},

    /* basic image functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_save),                (mp_obj_t)&py_image_save_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_scale),               (mp_obj_t)&py_image_scale_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_scaled),               (mp_obj_t)&py_image_scaled_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_blit),                (mp_obj_t)&py_image_blit_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_blend),               (mp_obj_t)&py_image_blend_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_histeq),              (mp_obj_t)&py_image_histeq_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_median),              (mp_obj_t)&py_image_median_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_threshold),           (mp_obj_t)&py_image_threshold_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_rainbow),             (mp_obj_t)&py_image_rainbow_obj},

    /* drawing functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_circle),         (mp_obj_t)&py_image_draw_circle_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_rectangle),      (mp_obj_t)&py_image_draw_rectangle_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_keypoints),      (mp_obj_t)&py_image_draw_keypoints_obj},

    /* objects/feature detection */
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_blobs),          (mp_obj_t)&py_image_find_blobs_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_template),       (mp_obj_t)&py_image_find_template_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_features),       (mp_obj_t)&py_image_find_features_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_keypoints),      (mp_obj_t)&py_image_find_keypoints_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_match_keypoints),     (mp_obj_t)&py_image_match_keypoints_obj},

    { NULL, NULL },
};

STATIC MP_DEFINE_CONST_DICT(locals_dict, locals_dict_table);

static const mp_obj_type_t py_image_type = {
    { &mp_type_type },
    .name  = MP_QSTR_image,
    .print = py_image_print,
    .buffer_p = { .get_buffer = py_image_get_buffer },
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

mp_obj_t py_image_from_struct(image_t *image)
{
    py_image_obj_t *o = m_new_obj(py_image_obj_t);
    o->base.type = &py_image_type;
    o->_cobj =*image;
    return o;
}

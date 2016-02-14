/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image Python module.
 *
 */
#include "mp.h"
#include "imlib.h"
#include "array.h"
#include "sensor.h"
#include "ff.h"
#include "xalloc.h"
#include "py_assert.h"
#include "py_image.h"
#include "arm_math.h"
#include "omv_boardconfig.h"

#define JPEG_INIT_BUF   (5*1024)

extern struct sensor_dev sensor;
static const mp_obj_type_t py_cascade_type;
static const mp_obj_type_t py_image_type;

extern const char *ffs_strerror(FRESULT res);

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

static void py_cascade_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_cascade_obj_t *self = self_in;
    /* print some info */
    mp_printf(print, "width:%d height:%d n_stages:%d n_features:%d n_rectangles:%d\n", self->_cobj.window.w,
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

static void py_kp_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_kp_obj_t *self = self_in;
    mp_printf(print, "size:%d threshold:%d normalized:%d\n", self->size, self->threshold, self->normalized);
}

static const mp_obj_type_t py_kp_type = {
    { &mp_type_type },
    .name  = MP_QSTR_kp_desc,
    .print = py_kp_print,
};

/* LBP descriptor */
typedef struct _py_lbp_obj_t {
    mp_obj_base_t base;
    uint8_t *hist;
} py_lbp_obj_t;

static void py_lbp_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    mp_printf(print, "<lbp descriptor>");
}

static const mp_obj_type_t py_lbp_type = {
    { &mp_type_type },
    .name  = MP_QSTR_lbp_desc,
    .print = py_lbp_print,
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

static void py_image_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    py_image_obj_t *self = self_in;
    mp_printf(print, "<image width:%d height:%d bpp:%d>", self->_cobj.w, self->_cobj.h, self->_cobj.bpp);
}

static mp_int_t py_image_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, mp_uint_t flags) {
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

static mp_obj_t py_image_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
    py_image_obj_t *o = self_in;
    image_t *image = py_image_cobj(self_in);

    if (value == MP_OBJ_NULL) {
        // delete
        return MP_OBJ_NULL; // op not supported
    } else if (value == MP_OBJ_SENTINEL) {
        // load
        mp_uint_t pixel;
        mp_uint_t index = mp_get_index(o->base.type, image->w*image->h, index_in, false);
        switch (image->bpp) {
            case 1:
                pixel = image->pixels[index];
                break;
            case 2:
                pixel = image->pixels[index*2]<<8 | image->pixels[index*2+1];
                break;
            default:
                return MP_OBJ_NULL; // op not supported
        }
        return mp_obj_new_int(pixel);
    } else {
        // store
        return mp_const_none;
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
    int res;
    image_t *image = py_image_cobj(args[0]);
    const char *path = mp_obj_str_get_str(args[1]);

    mp_map_elem_t *kw_subimage = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("subimage")), MP_MAP_LOOKUP);
    if (kw_subimage != NULL) {
        mp_obj_t *array;
        mp_obj_get_array_fixed_n(kw_subimage->value, 4, &array);

        rectangle_t r = {
            mp_obj_get_int(array[0]),
            mp_obj_get_int(array[1]),
            mp_obj_get_int(array[2]),
            mp_obj_get_int(array[3]),
        };

        res = imlib_save_image(image, path, &r);
    } else {
        res = imlib_save_image(image, path, NULL);
    }

    if (res != FR_OK) {
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

static mp_obj_t py_image_subimg(mp_obj_t image_obj, mp_obj_t subimg_obj)
{
    rectangle_t r;
    image_t *image;
    mp_obj_t *array;

    /* image pointer */
    image = py_image_cobj(image_obj);

    /* sub image */
    mp_obj_get_array_fixed_n(subimg_obj, 4, &array);
    r.x = mp_obj_get_int(array[0]);
    r.y = mp_obj_get_int(array[1]);
    r.w = mp_obj_get_int(array[2]);
    r.h = mp_obj_get_int(array[3]);

    image_t subimg = {
        .w=r.w,
        .h=r.h,
        .bpp=image->bpp,
        .pixels=xalloc(r.w*r.h*image->bpp)
    };

    imlib_subimage(image, &subimg, r.x, r.y);

    return py_image_from_struct(&subimg);
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
    PY_ASSERT_TRUE_MSG(image->bpp == 1,
            "This function is only supported on GRAYSCALE images");

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

static mp_obj_t py_image_invert(mp_obj_t image_obj)
{
    image_t *image;
    image = py_image_cobj(image_obj);

    /* Sanity checks */
    PY_ASSERT_TRUE_MSG(image->bpp == 1,
            "This function is only supported on GRAYSCALE images");

    /* Threshold image */
    imlib_invert(image);
    return mp_const_none;
}

static mp_obj_t py_image_binary(mp_obj_t image_obj, mp_obj_t threshold)
{
    image_t *image;

    /* sanity checks */
    PY_ASSERT_TRUE_MSG(sensor.pixformat == PIXFORMAT_GRAYSCALE,
            "This function is only supported on GRAYSCALE images");

    PY_ASSERT_TRUE_MSG(sensor.framesize <= OMV_MAX_RAW_FRAME,
            "This function is only supported on "OMV_MAX_RAW_FRAME_STR" and smaller frames");

    /* read arguments */
    image = py_image_cobj(image_obj);
    int thresh = mp_obj_get_int(threshold);

    /* Threshold image */
    imlib_binary(image, thresh);

    return mp_const_none;
}

static mp_obj_t py_image_threshold(mp_obj_t image_obj, mp_obj_t color_list_obj, mp_obj_t threshold)
{
    color_t *color;
    image_t *image;

    /* sanity checks */
    PY_ASSERT_TRUE_MSG(sensor.pixformat == PIXFORMAT_RGB565,
            "This function is only supported on RGB565 images");

    PY_ASSERT_TRUE_MSG(sensor.framesize <= OMV_MAX_BLOB_FRAME,
            "This function is only supported on "OMV_MAX_BLOB_FRAME_STR" and smaller frames");


    /* read arguments */
    image = py_image_cobj(image_obj);
    int thresh = mp_obj_get_int(threshold);

    /* returned image */
    image_t bimage = {
        .w=image->w,
        .h=image->h,
        .bpp=1,
        .pixels=image->data+(image->w*image->h*image->bpp)
    };

    /* copy color list */
    uint len;
    mp_obj_t *color_arr;
    mp_obj_get_array(color_list_obj, &len, &color_arr);

    color = xalloc(len*sizeof*color);

    for (int i=0; i<len; i++) {
        mp_obj_t *color_obj;
        mp_obj_get_array_fixed_n(color_arr[i], 3, &color_obj);
        color[i].r = mp_obj_get_int(color_obj[0]);
        color[i].g = mp_obj_get_int(color_obj[1]);
        color[i].b = mp_obj_get_int(color_obj[2]);
    }

    /* Threshold image using reference color */
    imlib_threshold(image, &bimage, color, len, thresh);

    return py_image_from_struct(&bimage);
}

static mp_obj_t py_image_rainbow(mp_obj_t src_image_obj)
{
    image_t *src_image = NULL;

    /* get C image pointer */
    src_image = py_image_cobj(src_image_obj);

    /* sanity checks */
    PY_ASSERT_TRUE_MSG(src_image->bpp == 1,
            "This function is only supported on GRAYSCALE images");

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

static mp_obj_t py_image_compress(mp_obj_t image_obj, mp_obj_t quality)
{
    image_t *image = py_image_cobj(image_obj);

    image_t cimage = {
        .w=image->w,
        .h=image->h,
        .bpp = JPEG_INIT_BUF,
        .pixels = xalloc(JPEG_INIT_BUF)
    };

    jpeg_compress(image, &cimage, mp_obj_get_int(quality));

    return py_image_from_struct(&cimage);
}

static mp_obj_t py_image_draw_line(mp_obj_t image_obj, mp_obj_t line_obj)
{
    /* get image pointer */
    struct image *image;
    image = py_image_cobj(image_obj);

    mp_obj_t *array;
    mp_obj_get_array_fixed_n(line_obj, 4, &array);
    int x0 = mp_obj_get_int(array[0]);
    int y0 = mp_obj_get_int(array[1]);
    int x1 = mp_obj_get_int(array[2]);
    int y1 = mp_obj_get_int(array[3]);

    imlib_draw_line(image, x0, y0, x1, y1);
    return mp_const_none;
}

static mp_obj_t py_image_draw_circle(mp_obj_t image_obj, mp_obj_t c_obj, mp_obj_t r_obj)
{
    int cx, cy, r;
    mp_obj_t *array;

    struct image *image;
    color_t c = {.r=0xFF, .g=0xFF, .b=0xFF};

    /* get image pointer */
    image = py_image_cobj(image_obj);

    /* center */
    mp_obj_get_array_fixed_n(c_obj, 2, &array);
    cx = mp_obj_get_int(array[0]);
    cy = mp_obj_get_int(array[1]);

    /* radius */
    r = mp_obj_get_int(r_obj);
    imlib_draw_circle(image, cx, cy, r, &c);
    return mp_const_none;
}

static mp_obj_t py_image_draw_string(uint n_args, const mp_obj_t *args)
{
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    image_t *image =py_image_cobj(args[0]);
    const char *str = mp_obj_str_get_str(args[3]);
    color_t c = {.r=0xFF, .g=0xFF, .b=0xFF};

    // check x, y
    PY_ASSERT_TRUE_MSG(x>=0 && x<image->w, "Image index out of range");
    PY_ASSERT_TRUE_MSG(y>=0 && y<image->h, "Image index out of range");
    PY_ASSERT_TRUE_MSG(image->bpp <= 2, "This function is not supported on JPEG images");

    if (n_args == 5) {
        // get color
        mp_obj_t *array;
        mp_obj_get_array_fixed_n(args[4], 3, &array);
        c.r = mp_obj_get_int(array[0]);
        c.g = mp_obj_get_int(array[1]);
        c.b = mp_obj_get_int(array[2]);
    }
    imlib_draw_string(image, x, y, str, &c);
    return mp_const_none;
}


static mp_obj_t py_image_erode(mp_obj_t image_obj, mp_obj_t ksize_obj)
{
    image_t *image = NULL;
    image = py_image_cobj(image_obj);

    /* sanity checks */
    PY_ASSERT_TRUE_MSG(image->bpp == 1,
            "This function is only supported on GRAYSCALE images");

    imlib_erode(image, mp_obj_get_int(ksize_obj));
    return mp_const_none;
}

static mp_obj_t py_image_dilate(mp_obj_t image_obj, mp_obj_t ksize_obj)
{
    image_t *image = NULL;
    image = py_image_cobj(image_obj);

    /* sanity checks */
    PY_ASSERT_TRUE_MSG(image->bpp == 1,
            "This function is only supported on GRAYSCALE images");

    imlib_dilate(image, mp_obj_get_int(ksize_obj));
    return mp_const_none;
}

static mp_obj_t py_image_morph(mp_obj_t image_obj, mp_obj_t ksize_obj)
{
    image_t *image = NULL;
    image = py_image_cobj(image_obj);

    /* sanity checks */
    PY_ASSERT_TRUE_MSG(image->bpp == 1,
            "This function is only supported on GRAYSCALE images");

    imlib_morph(image, NULL, mp_obj_get_int(ksize_obj));

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

static mp_obj_t py_image_draw_keypoints(mp_obj_t image_obj, mp_obj_t kpts_obj)
{
    image_t *image = NULL;
    py_kp_obj_t *kpts=NULL;

    /* get pointer */
    image = py_image_cobj(image_obj);
    kpts = (py_kp_obj_t*)kpts_obj;

    /* sanity checks */
    PY_ASSERT_TRUE_MSG(image->bpp == 1,
            "This function is only supported on GRAYSCALE images");
    PY_ASSERT_TYPE(kpts_obj, &py_kp_type);

    color_t cl = {.r=0xFF, .g=0xFF, .b=0xFF};
    for (int i=0; i<kpts->size; i++) {
        kp_t *kp = &kpts->kpts[i];
        float co = arm_cos_f32(kp->angle);
        float si = arm_sin_f32(kp->angle);
        imlib_draw_line(image, kp->x, kp->y, kp->x+(co*10), kp->y+(si*10));
        imlib_draw_circle(image, kp->x, kp->y, 4, &cl);
    }

    return mp_const_none;
}

static mp_obj_t py_image_find_blobs(mp_obj_t image_obj)
{
    /* C stuff */
    array_t *blobs;
    struct image *image;
    mp_obj_t blob_obj[6];

    /* MP List */
    mp_obj_t objects_list = mp_const_none;

     /* get image pointer */
    image = py_image_cobj(image_obj);

    /* run color dector */
    blobs = imlib_count_blobs(image);

    /* Create empty Python list */
    objects_list = mp_obj_new_list(0, NULL);

    if (array_length(blobs)) {
        for (int j=0; j<array_length(blobs); j++) {
             blob_t *r = array_at(blobs, j);
             blob_obj[0] = mp_obj_new_int(r->x);
             blob_obj[1] = mp_obj_new_int(r->y);
             blob_obj[2] = mp_obj_new_int(r->w);
             blob_obj[3] = mp_obj_new_int(r->h);
             blob_obj[4] = mp_obj_new_int(r->c);
             blob_obj[5] = mp_obj_new_int(r->id);
             mp_obj_list_append(objects_list, mp_obj_new_tuple(6, blob_obj));
        }
    }
    array_free(blobs);
    return objects_list;
}

static mp_obj_t py_image_find_features(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    struct image *image = NULL;
    struct cascade *cascade = NULL;

    struct array *objects_array=NULL;
    mp_obj_t objects_list = mp_const_none;

    /* sanity checks */
    PY_ASSERT_TRUE_MSG(sensor.pixformat == PIXFORMAT_GRAYSCALE,
            "This function is only supported on GRAYSCALE images");

    PY_ASSERT_TRUE_MSG(sensor.framesize <= OMV_MAX_INT_FRAME,
            "This function is only supported on "OMV_MAX_INT_FRAME_STR" and smaller frames");

    /* read arguments */
    image = py_image_cobj(args[0]);
    cascade = py_cascade_cobj(args[1]);

    /* set some defaults */
    cascade->threshold = 0.65f;
    cascade->scale_factor = 1.65f;

    /* read kw args */
    mp_map_elem_t *kw_thresh = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("threshold")), MP_MAP_LOOKUP);
    if (kw_thresh != NULL) {
        cascade->threshold = mp_obj_get_float(kw_thresh->value);
    }

    mp_map_elem_t *kw_scalef = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("scale")), MP_MAP_LOOKUP);
    if (kw_scalef != NULL) {
        cascade->scale_factor = mp_obj_get_float(kw_scalef->value);
    }

    /* Detect objects */
    objects_array = imlib_detect_objects(image, cascade);

    /* Create empty Python list */
    objects_list = mp_obj_new_list(0, NULL);

    /* Add detected objects to the list */
    for (int i=0; i<array_length(objects_array); i++) {
        struct rectangle *r = array_at(objects_array, i);
        mp_obj_t rec_obj[4] = {
            mp_obj_new_int(r->x),
            mp_obj_new_int(r->y),
            mp_obj_new_int(r->w),
            mp_obj_new_int(r->h),
        };
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
    PY_ASSERT_TRUE_MSG(sensor.pixformat == PIXFORMAT_GRAYSCALE,
            "This function is only supported on GRAYSCALE images");

    PY_ASSERT_TRUE_MSG(sensor.framesize <= OMV_MAX_INT_FRAME,
            "This function is only supported on "OMV_MAX_INT_FRAME_STR" and smaller frames");


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

    int kpts_size = 0;
    kp_t *kpts = NULL;
    py_kp_obj_t *kp_obj =NULL;

    image_t *image = py_image_cobj(args[0]);
    rectangle_t roi={0, 0, image->w, image->h};

    /* sanity checks */
    PY_ASSERT_TRUE_MSG(sensor.pixformat == PIXFORMAT_GRAYSCALE,
            "This function is only supported on GRAYSCALE images");

    PY_ASSERT_TRUE_MSG(sensor.framesize <= OMV_MAX_INT_FRAME,
            "This function is only supported on "OMV_MAX_INT_FRAME_STR" and smaller frames");

    /* read var args */
    mp_map_elem_t *kw_thresh = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("threshold")), MP_MAP_LOOKUP);
    if (kw_thresh != NULL) {
        threshold = mp_obj_get_int(kw_thresh->value);
    }

    mp_map_elem_t *kw_norm = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("normalized")), MP_MAP_LOOKUP);
    if (kw_norm != NULL) {
        normalized = mp_obj_get_int(kw_norm->value);
    }

    mp_map_elem_t *kw_roi = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("roi")), MP_MAP_LOOKUP);
    if (kw_roi != NULL) {
        mp_obj_t *array;
        mp_obj_get_array_fixed_n(kw_roi->value, 4, &array);
        roi.x=mp_obj_get_int(array[0]);
        roi.y=mp_obj_get_int(array[1]);
        roi.w=mp_obj_get_int(array[2]);
        roi.h=mp_obj_get_int(array[3]);
    }

    /* run keypoint extractor on ROI */
    kpts = fast_detect(image, threshold, &kpts_size, &roi);

    if (kpts_size) {
        /* run keypoint descriptor */
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
    return mp_const_none;
}

static mp_obj_t py_image_find_lbp(mp_obj_t image_obj, mp_obj_t roi_obj)
{
    image_t *image;
    py_lbp_obj_t *lbp_obj;

    image = py_image_cobj(image_obj);
    /* sanity checks */
    PY_ASSERT_TRUE_MSG(image->bpp == 1,
            "This function is only supported on GRAYSCALE images");

    mp_obj_t *array;
    mp_obj_get_array_fixed_n(roi_obj, 4, &array);

    rectangle_t roi = {
        mp_obj_get_int(array[0]),
        mp_obj_get_int(array[1]),
        mp_obj_get_int(array[2]),
        mp_obj_get_int(array[3]),
    };

    lbp_obj = m_new_obj(py_lbp_obj_t);
    lbp_obj->base.type = &py_lbp_type;
    lbp_obj->hist = imlib_lbp_cascade(image, &roi);
    return lbp_obj;
}

static mp_obj_t py_image_find_eyes(mp_obj_t image_obj, mp_obj_t roi_obj)
{
    image_t *image;

    image = py_image_cobj(image_obj);
    /* sanity checks */
    PY_ASSERT_TRUE_MSG(image->bpp == 1,
            "This function is only supported on GRAYSCALE images");

    mp_obj_t *array;
    mp_obj_get_array_fixed_n(roi_obj, 4, &array);

    rectangle_t roi = {
        mp_obj_get_int(array[0]),
        mp_obj_get_int(array[1]),
        mp_obj_get_int(array[2]),
        mp_obj_get_int(array[3]),
    };

    point_t l={0}, r={0};
    imlib_find_eyes(image, &l, &r, &roi);

    mp_obj_t eyes_obj[4] = {
        mp_obj_new_int(l.x),
        mp_obj_new_int(l.y),
        mp_obj_new_int(r.x),
        mp_obj_new_int(r.y),
    };

    return mp_obj_new_tuple(4, eyes_obj);
}

static mp_obj_t py_image_match_keypoints(uint n_args, const mp_obj_t *args)
{
    int16_t *kpts_match;
    int threshold = mp_obj_get_int(args[3]);
    py_kp_obj_t *kpts1 = ((py_kp_obj_t*)args[1]);
    py_kp_obj_t *kpts2 = ((py_kp_obj_t*)args[2]);

    // sanity checks
    PY_ASSERT_TYPE(kpts1, &py_kp_type);
    PY_ASSERT_TYPE(kpts2, &py_kp_type);

    if (kpts1->size == 0 || kpts2->size == 0) {
        return mp_const_none;
    }

    // match the keypoint sets
    kpts_match = freak_match_keypoints(kpts1->kpts, kpts1->size, kpts2->kpts, kpts2->size, threshold);

    int match=0, cx=0, cy=0;
    for (int i=0; i<kpts1->size; i++) {
        if (kpts_match[i] != -1) {
            kp_t *kp = &kpts2->kpts[kpts_match[i]];
            cx += kp->x; cy += kp->y;
            match++;
        }
    }

    if (match>=(kpts1->size/16)) {
        mp_obj_t rec_obj[2] = {
            mp_obj_new_int(cx/match),
            mp_obj_new_int(cy/match),
        };
        return mp_obj_new_tuple(2, rec_obj);
    }
    return mp_const_none;
}

static mp_obj_t py_image_match_lbp(mp_obj_t image_obj, mp_obj_t d0_obj, mp_obj_t d1_obj)
{
   // image_t *image;
   // image = py_image_cobj(image_obj);
    py_lbp_obj_t *d0 = ((py_lbp_obj_t*)d0_obj);
    py_lbp_obj_t *d1 = ((py_lbp_obj_t*)d1_obj);
    return mp_obj_new_int(imlib_lbp_desc_distance(d0->hist, d1->hist));
}

static mp_obj_t py_image_get_pixel(mp_obj_t image_obj, mp_obj_t x_obj, mp_obj_t y_obj)
{
    mp_obj_t ret_obj;
    mp_obj_t rgb_obj[3];

    // read args
    int x = mp_obj_get_int(x_obj);
    int y = mp_obj_get_int(y_obj);
    image_t *image = py_image_cobj(image_obj);

    // check x, y
    PY_ASSERT_TRUE_MSG(x>=0 && x<image->w, "image index out of range");
    PY_ASSERT_TRUE_MSG(y>=0 && y<image->h, "image index out of range");
    PY_ASSERT_TRUE_MSG(image->bpp <= 2, "This function not supported on JPEG images");

    switch (image->bpp) {
        case 1:
            ret_obj = mp_obj_new_int(image->pixels[y*image->w+x]);
            break;
        case 2: {
            uint32_t p = ((uint16_t*)image->pixels)[y*image->w+x];
            rgb_obj[0] = mp_obj_new_int(((p>>3)&0x1F)*255/31);
            rgb_obj[1] = mp_obj_new_int((((p&0x07)<<3)|(p>>13))*255/63);
            rgb_obj[2] = mp_obj_new_int(((p>>8)&0x1F)*255/31);
            ret_obj = mp_obj_new_tuple(3, rgb_obj);
            break;
        }
        default:
            // shouldn't happen
            ret_obj=mp_const_none;
            break;
    }

    return ret_obj;
}

#define RGB565(r, g, b)\
    (uint32_t)(((r&0x1F)<<3)|((g&0x3F)>>3)|(g<<13)|((b&0x1F)<<8))

static mp_obj_t py_image_set_pixel(uint n_args, const mp_obj_t *args)
{
    // read args
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    image_t *image = py_image_cobj(args[0]);

    // check x, y, format
    PY_ASSERT_TRUE_MSG(x>=0 && x<image->w, "image index out of range");
    PY_ASSERT_TRUE_MSG(y>=0 && y<image->h, "image index out of range");
    PY_ASSERT_TRUE_MSG(image->bpp <= 2, "This function is not supported on JPEG images");

    switch (image->bpp) {
        case 1:
            image->pixels[y*image->w+x] = mp_obj_get_int(args[3]);
            break;
        case 2: {
            mp_obj_t *color_obj;
            uint16_t *pixels = (uint16_t*)image->pixels;
            mp_obj_get_array_fixed_n(args[3], 3, &color_obj);
            pixels[y*image->w+x] = RGB565(mp_obj_get_int(color_obj[0]),
                                          mp_obj_get_int(color_obj[1]),
                                          mp_obj_get_int(color_obj[2]));
            break;
        }
        default:
            // shouldn't happen
            break;
    }
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_size_obj, py_image_size);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_save_obj, 2, py_image_save);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_scale_obj, py_image_scale);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_scaled_obj, py_image_scaled);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_subimg_obj, py_image_subimg);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_blit_obj, py_image_blit);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_blend_obj, py_image_blend);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_histeq_obj, py_image_histeq);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_median_obj, 1, py_image_median);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_invert_obj, py_image_invert);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_binary_obj, py_image_binary);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_threshold_obj, py_image_threshold);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_rainbow_obj, py_image_rainbow);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_erode_obj, py_image_erode);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_dilate_obj, py_image_dilate);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_morph_obj, py_image_morph);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_compress_obj, py_image_compress);

STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_draw_line_obj, py_image_draw_line);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_draw_circle_obj, py_image_draw_circle);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_draw_rectangle_obj, py_image_draw_rectangle);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_draw_keypoints_obj, py_image_draw_keypoints);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_image_draw_string_obj, 4, 5, py_image_draw_string);

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_find_blobs_obj, py_image_find_blobs);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_find_template_obj, py_image_find_template);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_features_obj, 2, py_image_find_features);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_find_keypoints_obj, 1, py_image_find_keypoints);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_find_lbp_obj, py_image_find_lbp);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_find_eyes_obj, py_image_find_eyes);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_image_match_keypoints_obj, 4, 4, py_image_match_keypoints);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_match_lbp_obj, py_image_match_lbp);

STATIC MP_DEFINE_CONST_FUN_OBJ_3(py_image_get_pixel_obj, py_image_get_pixel);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_image_set_pixel_obj, 4, 4, py_image_set_pixel);

static const mp_map_elem_t locals_dict_table[] = {
    {MP_OBJ_NEW_QSTR(MP_QSTR_size),                (mp_obj_t)&py_image_size_obj},

    /* basic image functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_save),                (mp_obj_t)&py_image_save_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_scale),               (mp_obj_t)&py_image_scale_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_scaled),              (mp_obj_t)&py_image_scaled_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_subimg),              (mp_obj_t)&py_image_subimg_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_blit),                (mp_obj_t)&py_image_blit_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_blend),               (mp_obj_t)&py_image_blend_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_histeq),              (mp_obj_t)&py_image_histeq_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_median),              (mp_obj_t)&py_image_median_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_invert),              (mp_obj_t)&py_image_invert_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_binary),              (mp_obj_t)&py_image_binary_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_threshold),           (mp_obj_t)&py_image_threshold_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_rainbow),             (mp_obj_t)&py_image_rainbow_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_erode),               (mp_obj_t)&py_image_erode_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_dilate),              (mp_obj_t)&py_image_dilate_obj},
//    {MP_OBJ_NEW_QSTR(MP_QSTR_morph),               (mp_obj_t)&py_image_morph_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_compress),            (mp_obj_t)&py_image_compress_obj},

    /* drawing functions */
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_line),           (mp_obj_t)&py_image_draw_line_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_circle),         (mp_obj_t)&py_image_draw_circle_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_rectangle),      (mp_obj_t)&py_image_draw_rectangle_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_keypoints),      (mp_obj_t)&py_image_draw_keypoints_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_draw_string),         (mp_obj_t)&py_image_draw_string_obj},

    /* objects/feature detection */
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_blobs),          (mp_obj_t)&py_image_find_blobs_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_template),       (mp_obj_t)&py_image_find_template_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_features),       (mp_obj_t)&py_image_find_features_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_keypoints),      (mp_obj_t)&py_image_find_keypoints_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_lbp),            (mp_obj_t)&py_image_find_lbp_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_find_eyes),           (mp_obj_t)&py_image_find_eyes_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_match_keypoints),     (mp_obj_t)&py_image_match_keypoints_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_match_lbp),           (mp_obj_t)&py_image_match_lbp_obj},

    {MP_OBJ_NEW_QSTR(MP_QSTR_get_pixel),           (mp_obj_t)&py_image_get_pixel_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_set_pixel),           (mp_obj_t)&py_image_set_pixel_obj},

    { NULL, NULL },
};

STATIC MP_DEFINE_CONST_DICT(locals_dict, locals_dict_table);

static const mp_obj_type_t py_image_type = {
    { &mp_type_type },
    .name  = MP_QSTR_image,
    .print = py_image_print,
    .buffer_p = { .get_buffer = py_image_get_buffer },
    .subscr = py_image_subscr,
    .locals_dict = (mp_obj_t)&locals_dict,
};

mp_obj_t py_image(int w, int h, int bpp, void *pixels)
{
    py_image_obj_t *o = m_new_obj(py_image_obj_t);
    o->base.type = &py_image_type;

    o->_cobj.w =w;
    o->_cobj.h =h;
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

int py_image_descriptor_from_roi(image_t *image, const char *path, rectangle_t *roi)
{
    int kpts_size = 0;
    kp_t *kpts = NULL;

    int threshold = 10;
    bool normalized = false;

    kpts = fast_detect(image, threshold, &kpts_size, roi);
    freak_find_keypoints(image, kpts, kpts_size, normalized, normalized);

    printf("Save Descriptor: KPTS(%d)\n", kpts_size);
    printf("Save Descriptor: ROI(%d %d %d %d)\n", roi->x, roi->y, roi->w, roi->h);

    if (kpts_size ==0) {
        return 0;
    }

    int res = freak_save_descriptor(kpts, kpts_size, path);
    if (res != FR_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }
    return 0;
}


// image module
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

mp_obj_t py_image_load_cascade(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    cascade_t cascade;

    py_cascade_obj_t *o = NULL;
    mp_map_elem_t *kw_stages = NULL;

    // Load cascade from file or flash
    const char *path = mp_obj_str_get_str(args[0]);
    int res = imlib_load_cascade(&cascade, path);
    if (res != FR_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }

    // Limit the number of stages if specified
    kw_stages = mp_map_lookup(kw_args, MP_OBJ_NEW_QSTR(qstr_from_str("stages")), MP_MAP_LOOKUP);
    if (kw_stages != NULL) {
        int stages = mp_obj_get_int(kw_stages->value);
        if (stages > 0 && stages <= cascade.n_stages) {
            cascade.n_stages = stages;
        }
    }

    // Return micropython cascade object
    o = m_new_obj(py_cascade_obj_t);
    o->base.type = &py_cascade_type;
    o->_cobj = cascade;
    return o;
}

mp_obj_t py_image_load_descriptor(mp_obj_t path_obj)
{
    kp_t *kpts=NULL;
    int kpts_size =0;

    py_kp_obj_t *kp_obj =NULL;
    const char *path = mp_obj_str_get_str(path_obj);

    int res = freak_load_descriptor(&kpts, &kpts_size, path);
    if (res != FR_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }
    /* return keypoints MP object */
    kp_obj = m_new_obj(py_kp_obj_t);
    kp_obj->base.type = &py_kp_type;
    kp_obj->kpts = kpts;
    kp_obj->size = kpts_size;
    kp_obj->threshold = 10;
    kp_obj->normalized = false;
    return kp_obj;
}

mp_obj_t py_image_load_lbp(mp_obj_t path_obj)
{
    py_lbp_obj_t *lbp = m_new_obj(py_lbp_obj_t);
    lbp->base.type = &py_lbp_type;

    int res = imlib_lbp_desc_load(mp_obj_str_get_str(path_obj), &lbp->hist);
    if (res != FR_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }
    return lbp;
}

mp_obj_t py_image_save_descriptor(mp_obj_t path_obj, mp_obj_t kpts_obj)
{
    py_kp_obj_t *kpts = ((py_kp_obj_t*)kpts_obj);
    const char *path = mp_obj_str_get_str(path_obj);

    int res = freak_save_descriptor(kpts->kpts, kpts->size, path);
    if (res != FR_OK) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
    }
    return mp_const_true;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_load_image_obj, py_image_load_image);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_image_load_cascade_obj, 1, py_image_load_cascade);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_load_descriptor_obj, py_image_load_descriptor);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_image_load_lbp_obj, py_image_load_lbp);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_image_save_descriptor_obj, py_image_save_descriptor);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_image) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Image),           (mp_obj_t)&py_image_load_image_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_HaarCascade),     (mp_obj_t)&py_image_load_cascade_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FreakDesc),       (mp_obj_t)&py_image_load_descriptor_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FreakDescSave),   (mp_obj_t)&py_image_save_descriptor_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_LBPDesc),         (mp_obj_t)&py_image_load_lbp_obj},
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t image_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_image,
    .globals = (mp_obj_t)&globals_dict,
};

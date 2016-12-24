/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __IMLIB_IMAGE_H__
#define __IMLIB_IMAGE_H__
#include <arm_math.h>
#include "other_log2.h"
#include "utils_rectangle.h"

typedef enum imlib_image_type
{
    IMLIB_IMAGE_TYPE_BINARY,
    IMLIB_IMAGE_TYPE_GRAYSCALE,
    IMLIB_IMAGE_TYPE_RGB565,
    IMLIB_IMAGE_TYPE_JPG
}
imlib_image_type_t;

typedef struct imlib_image
{
    imlib_image_type_t type;
    utils_size_t geometry;
    size_t size;
    void *data;
}
imlib_image_t;

void imlib_image_init(imlib_image_t *ptr, imlib_image_type_t type, utils_size_t *geometry);
void imlib_image_copy(imlib_image_t *dst, imlib_image_t *src);
void imlib_image_check_overlap(imlib_image_t *ptr, utils_rectangle_t *rect);
void imlib_image_intersected(imlib_image_t *ptr, utils_rectangle_t *rect);

#define IMLIB_IMAGE_GET_IMAGE_TYPE(image) \
({ \
    typeof (image) _image = (image); \
    _image->type; \
})

#define IMLIB_IMAGE_GET_IMAGE_IS_BINARY(image) \
({ \
    typeof (image) _image = (image); \
    _image->type == IMLIB_IMAGE_TYPE_BINARY; \
})

#define IMLIB_IMAGE_GET_IMAGE_IS_GRAYSCALE(image) \
({ \
    typeof (image) _image = (image); \
    _image->type == IMLIB_IMAGE_TYPE_GRAYSCALE; \
})

#define IMLIB_IMAGE_GET_IMAGE_IS_RGB565(image) \
({ \
    typeof (image) _image = (image); \
    _image->type == IMLIB_IMAGE_TYPE_RGB565; \
})

#define IMLIB_IMAGE_GET_IMAGE_IS_JPG(image) \
({ \
    typeof (image) _image = (image); \
    _image->type == IMLIB_IMAGE_TYPE_JPG; \
})

#define IMLIB_IMAGE_GET_IMAGE_GEOMETRY(image) \
({ \
    typeof (image) _image = (image); \
    &(_image->geometry); \
})

#define IMLIB_IMAGE_GET_IMAGE_W(image) \
({ \
    typeof (image) _image = (image); \
    _image->geometry.w; \
})

#define IMLIB_IMAGE_GET_X_IN_IMAGE_W(image, x) \
({ \
    typeof (image) _image = (image); \
    typeof (x) _x = (x); \
    (0 <= _x) && (_x < _image->geometry.w); \
})

#define IMLIB_IMAGE_GET_IMAGE_H(image) \
({ \
    typeof (image) _image = (image); \
    _image->geometry.h; \
})

#define IMLIB_IMAGE_GET_Y_IN_IMAGE_H(image, y) \
({ \
    typeof (image) _image = (image); \
    typeof (y) _y = (y); \
    (0 <= _y) && (_y < _image->geometry.h); \
})

#define IMLIB_IMAGE_GET_IMAGE_SIZE(image) \
({ \
    typeof (image) _image = (image); \
    _image->size; \
})

#define IMLIB_IMAGE_SET_IMAGE_SIZE(image, s) \
({ \
    typeof (image) _image = (image); \
    typeof (s) _s = (s); \
    _image->size = _s; \
})

#define IMLIB_IMAGE_GET_IMAGE_DATA(image) \
({ \
    typeof (image) _image = (image); \
    _image->data; \
})

#define IMLIB_IMAGE_SET_IMAGE_DATA(image, d) \
({ \
    typeof (image) _image = (image); \
    typeof (d) _d = (d); \
    _image->data = _d; \
})

#define IMLIB_IMAGE_IMAGE_TYPE_EQUAL(image0, image1) \
({ \
    typeof (image0) _image0 = (image0); \
    typeof (image1) _image1 = (image1); \
    (_image0->type == _image1->type); \
})

#define IMLIB_IMAGE_IMAGE_GEOMETRY_EQUAL(image0, image1) \
({ \
    typeof (image0) _image0 = (image0); \
    typeof (image1) _image1 = (image1); \
    (_image0->geometry.w == _image1->geometry.w) && (_image0->geometry.h == _image1->geometry.h); \
})

#define UINT8_T_BITS (sizeof(uint8_t) * 8)
#define UINT8_T_MASK (UINT8_T_BITS - 1)
#define UINT8_T_SHIFT OTHER_LOG2(UINT8_T_MASK)

#define UINT16_T_BITS (sizeof(uint16_t) * 8)
#define UINT16_T_MASK (UINT16_T_BITS - 1)
#define UINT16_T_SHIFT OTHER_LOG2(UINT16_T_MASK)

#define UINT32_T_BITS (sizeof(uint32_t) * 8)
#define UINT32_T_MASK (UINT32_T_BITS - 1)
#define UINT32_T_SHIFT OTHER_LOG2(UINT32_T_MASK)

#define IMLIB_IMAGE_GET_BINARY_PIXEL(image, x, y) \
({ \
    typeof (image) _image = (image); \
    typeof (x) _x = (x); \
    typeof (y) _y = (y); \
    (((uint32_t *) _image->data)[(((_image->geometry.w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] >> (_x & UINT32_T_MASK)) & 1; \
})

#define IMLIB_IMAGE_PUT_BINARY_PIXEL(image, x, y, v) \
({ \
    typeof (image) _image = (image); \
    typeof (x) _x = (x); \
    typeof (y) _y = (y); \
    typeof (v) _v = (v); \
    size_t _i = (((_image->geometry.w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT); \
    size_t _j = _x & UINT32_T_MASK; \
    ((uint32_t *) _image->data)[_i] = (((uint32_t *) _image->data)[_i] & (~(1 << _j))) | ((_v & 1) << _j); \
})

#define IMLIB_IMAGE_CLEAR_BINARY_PIXEL(image, x, y) \
({ \
    typeof (image) _image = (image); \
    typeof (x) _x = (x); \
    typeof (y) _y = (y); \
    ((uint32_t *) _image->data)[(((_image->geometry.w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] &= ~(1 << (_x & UINT32_T_MASK)); \
})

#define IMLIB_IMAGE_SET_BINARY_PIXEL(image, x, y) \
({ \
    typeof (image) _image = (image); \
    typeof (x) _x = (x); \
    typeof (y) _y = (y); \
    ((uint32_t *) _image->data)[(((_image->geometry.w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] |= 1 << (_x & UINT32_T_MASK); \
})

#define IMLIB_IMAGE_GET_GRAYSCALE_PIXEL(image, x, y) \
({ \
    typeof (image) _image = (image); \
    typeof (x) _x = (x); \
    typeof (y) _y = (y); \
    ((uint8_t *) _image->data)[(_image->geometry.w * _y) + _x]; \
})

#define IMLIB_IMAGE_PUT_GRAYSCALE_PIXEL(image, x, y, v) \
({ \
    typeof (image) _image = (image); \
    typeof (x) _x = (x); \
    typeof (y) _y = (y); \
    typeof (v) _v = (v); \
    ((uint8_t *) _image->data)[(_image->geometry.w * _y) + _x] = _v; \
})

#define IMLIB_IMAGE_GET_RGB565_PIXEL(image, x, y) \
({ \
    typeof (image) _image = (image); \
    typeof (x) _x = (x); \
    typeof (y) _y = (y); \
    ((uint16_t *) _image->data)[(_image->geometry.w * _y) + _x]; \
})

#define IMLIB_IMAGE_PUT_RGB565_PIXEL(image, x, y) \
({ \
    typeof (image) _image = (image); \
    typeof (x) _x = (x); \
    typeof (y) _y = (y); \
    typeof (v) _v = (v); \
    ((uint16_t *) _image->data)[(_image->geometry.w * _y) + _x] = _v; \
})

#ifdef __arm__
    #define IMLIB_IMAGE_REV_RGB565_PIXEL(pixel) \
    ({ \
        typeof (pixel) _pixel = (pixel); \
        __REV16(_pixel); \
    })
#else
    #define IMLIB_IMAGE_REV_RGB565_PIXEL(pixel) \
    ({ \
        typeof (pixel) _pixel = (pixel); \
        ((_pixel >> 8) | (_pixel << 8)) & 0xFFFF; \
    })
#endif

#define IMLIB_IMAGE_COMPUTE_TARGET_SIZE_SCALE_FACTOR(target_size, source_rect) \
typeof (target_size) _target_size = (target_size); \
typeof (source_rect) _source_rect = (source_rect); \
size_t IMLIB_IMAGE_X_SOURCE_OFFSET = _source_rect->p.x; \
size_t IMLIB_IMAGE_Y_SOURCE_OFFSET = _source_rect->p.y; \
size_t IMLIB_IMAGE_X_TARGET_OFFSET = 0; \
size_t IMLIB_IMAGE_Y_TARGET_OFFSET = 0; \
float IMLIB_IMAGE_X_RATIO = ((float) _source_rect->s.w) / ((float) _target_size->w); \
float IMLIB_IMAGE_Y_RATIO = ((float) _source_rect->s.h) / ((float) _target_size->h); \
({ 0; })

#define IMLIB_IMAGE_COMPUTE_TARGET_RECT_SCALE_FACTOR(target_rect, source_rect) \
typeof (target_rect) _target_rect = (target_rect); \
typeof (source_rect) _source_rect = (source_rect); \
size_t IMLIB_IMAGE_X_SOURCE_OFFSET = _source_rect->p.x; \
size_t IMLIB_IMAGE_Y_SOURCE_OFFSET = _source_rect->p.y; \
size_t IMLIB_IMAGE_X_TARGET_OFFSET = _target_rect->p.x; \
size_t IMLIB_IMAGE_Y_TARGET_OFFSET = _target_rect->p.y; \
float IMLIB_IMAGE_X_RATIO = ((float) _source_rect->s.w) / ((float) _target_rect->s.w); \
float IMLIB_IMAGE_Y_RATIO = ((float) _source_rect->s.h) / ((float) _target_rect->s.h); \
({ 0; })

#define IMLIB_IMAGE_GET_SCALED_BINARY_PIXEL(image, x, y) \
({ \
    typeof (image) _image = (image); \
    typeof (x) _x = (x); \
    typeof (y) _y = (y); \
    IMLIB_IMAGE_GET_BINARY_PIXEL(_image, ((size_t) ((IMLIB_IMAGE_X_RATIO * (_x - IMLIB_IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMLIB_IMAGE_X_SOURCE_OFFSET, ((size_t) ((IMLIB_IMAGE_Y_RATIO * (_y - IMLIB_IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMLIB_IMAGE_Y_SOURCE_OFFSET); \
})

#define IMLIB_IMAGE_GET_SCALED_GRAYSCALE_PIXEL(image, x, y) \
({ \
    typeof (image) _image = (image); \
    typeof (x) _x = (x); \
    typeof (y) _y = (y); \
    IMLIB_IMAGE_GET_GRAYSCALE_PIXEL(_image, ((size_t) ((IMLIB_IMAGE_X_RATIO * (_x - IMLIB_IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMLIB_IMAGE_X_SOURCE_OFFSET, ((size_t) ((IMLIB_IMAGE_Y_RATIO * (_y - IMLIB_IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMLIB_IMAGE_Y_SOURCE_OFFSET); \
})

#define IMLIB_IMAGE_GET_SCALED_RGB565_PIXEL(image, x, y) \
({ \
    typeof (image) _image = (image); \
    typeof (x) _x = (x); \
    typeof (y) _y = (y); \
    IMLIB_IMAGE_GET_RGB565_PIXEL(_image, ((size_t) ((IMLIB_IMAGE_X_RATIO * (_x - IMLIB_IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMLIB_IMAGE_X_SOURCE_OFFSET, ((size_t) ((IMLIB_IMAGE_Y_RATIO * (_y - IMLIB_IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMLIB_IMAGE_Y_SOURCE_OFFSET); \
})

// Fast Stuff //

#define IMLIB_IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(image, y) \
({ \
    typeof (image) _image = (image); \
    typeof (y) _y = (y); \
    ((uint32_t *) _image->data) + (((_image->geometry.w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y); \
})

#define IMLIB_IMAGE_INC_BINARY_PIXEL_ROW_PTR(row_ptr, image) \
({ \
    typeof (row_ptr) _row_ptr = (row_ptr); \
    typeof (image) _image = (image); \
    _row_ptr + ((_image->geometry.w + UINT32_T_MASK) >> UINT32_T_SHIFT); \
})

#define IMLIB_IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x) \
({ \
    typeof (row_ptr) _row_ptr = (row_ptr); \
    typeof (x) _x = (x); \
    (_row_ptr[_x >> UINT32_T_SHIFT] >> (_x & UINT32_T_MASK)) & 1; \
})

#define IMLIB_IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x, v) \
({ \
    typeof (row_ptr) _row_ptr = (row_ptr); \
    typeof (x) _x = (x); \
    typeof (v) _v = (v); \
    size_t _i = _x >> UINT32_T_SHIFT \
    size_t _j = _x & UINT32_T_MASK; \
    _row_ptr[_i] = (_row_ptr[_i] & (~(1 << _j))) | ((_v & 1) << _j); \
})

#define IMLIB_IMAGE_CLEAR_BINARY_PIXEL_FAST(row_ptr, x) \
({ \
    typeof (row_ptr) _row_ptr = (row_ptr); \
    typeof (x) _x = (x); \
    _row_ptr[_x >> UINT32_T_SHIFT] &= ~(1 << (_x & UINT32_T_MASK)); \
})

#define IMLIB_IMAGE_SET_BINARY_PIXEL_FAST(row_ptr, x) \
({ \
    typeof (row_ptr) _row_ptr = (row_ptr); \
    typeof (x) _x = (x); \
    _row_ptr[_x >> UINT32_T_SHIFT] |= 1 << (_x & UINT32_T_MASK); \
})

#define IMLIB_IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(image, y) \
({ \
    typeof (image) _image = (image); \
    typeof (y) _y = (y); \
    ((uint8_t *) _image->data) + (_image->geometry.w * _y); \
})

#define IMLIB_IMAGE_INC_GRAYSCALE_PIXEL_ROW_PTR(row_ptr, image) \
({ \
    typeof (row_ptr) _row_ptr = (row_ptr); \
    typeof (image) _image = (image); \
    row_ptr + _image->geometry.w; \
})

#define IMLIB_IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x) \
({ \
    typeof (row_ptr) _row_ptr = (row_ptr); \
    typeof (x) _x = (x); \
    _row_ptr[_x]; \
})

#define IMLIB_IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x, v) \
({ \
    typeof (row_ptr) _row_ptr = (row_ptr); \
    typeof (x) _x = (x); \
    typeof (v) _v = (v); \
    _row_ptr[_x] = _v; \
})

#define IMLIB_IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(image, y) \
({ \
    typeof (image) _image = (image); \
    typeof (y) _y = (y); \
    ((uint16_t *) _image->data) + (_image->geometry.w * _y); \
})

#define IMLIB_IMAGE_INC_RGB565_PIXEL_ROW_PTR(row_ptr, image) \
({ \
    typeof (row_ptr) _row_ptr = (row_ptr); \
    typeof (image) _image = (image); \
    row_ptr + _image->geometry.w; \
})

#define IMLIB_IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x) \
({ \
    typeof (row_ptr) _row_ptr = (row_ptr); \
    typeof (x) _x = (x); \
    _row_ptr[_x]; \
})

#define IMLIB_IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, v) \
({ \
    typeof (row_ptr) _row_ptr = (row_ptr); \
    typeof (x) _x = (x); \
    typeof (v) _v = (v); \
    _row_ptr[_x] = _v; \
})

#define IMLIB_IMAGE_COMPUTE_SCALED_BINARY_PIXEL_ROW_PTR(image, y) \
({ \
    typeof (image) _image = (image); \
    typeof (y) _y = (y); \
    ((uint32_t *) _image->data) + (((_image->geometry.w + UINT32_T_MASK) >> UINT32_T_SHIFT) * (((size_t) ((IMLIB_IMAGE_Y_RATIO * (_y - IMLIB_IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMLIB_IMAGE_Y_SOURCE_OFFSET)); \
})

#define IMLIB_IMAGE_GET_SCALED_BINARY_PIXEL_FAST(row_ptr, x) IMLIB_IMAGE_GET_BINARY_PIXEL_FAST((row_ptr), ((size_t) ((IMLIB_IMAGE_X_RATIO * ((x) - IMLIB_IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMLIB_IMAGE_X_SOURCE_OFFSET)

#define IMLIB_IMAGE_COMPUTE_SCALED_GRAYSCALE_PIXEL_ROW_PTR(image, y) \
({ \
    typeof (image) _image = (image); \
    typeof (y) _y = (y); \
    ((uint8_t *) _image->data) + (_image->geometry.w * (((size_t) ((IMLIB_IMAGE_Y_RATIO * (_y - IMLIB_IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMLIB_IMAGE_Y_SOURCE_OFFSET)); \
})

#define IMLIB_IMAGE_GET_SCALED_GRAYSCALE_PIXEL_FAST(row_ptr, x) IMLIB_IMAGE_GET_GRAYSCALE_PIXEL_FAST((row_ptr), ((size_t) ((IMLIB_IMAGE_X_RATIO * ((x) - IMLIB_IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMLIB_IMAGE_X_SOURCE_OFFSET)

#define IMLIB_IMAGE_COMPUTE_SCALED_RGB565_PIXEL_ROW_PTR(image, y) \
({ \
    typeof (image) _image = (image); \
    typeof (y) _y = (y); \
    ((uint16_t *) _image->data) + (_image->geometry.w * (((size_t) ((IMLIB_IMAGE_Y_RATIO * (_y - IMLIB_IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMLIB_IMAGE_Y_SOURCE_OFFSET)); \
})

#define IMLIB_IMAGE_GET_SCALED_RGB565_PIXEL_FAST(row_ptr, x) IMLIB_IMAGE_GET_RGB565_PIXEL_FAST((row_ptr), ((size_t) ((IMLIB_IMAGE_X_RATIO * ((x) - IMLIB_IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMLIB_IMAGE_X_SOURCE_OFFSET)

#endif /* __IMLIB_IMAGE_H__ */

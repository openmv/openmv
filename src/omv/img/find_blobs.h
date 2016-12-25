/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __FIND_BLOBS_H__
#define __FIND_BLOBS_H__
#include "imlib_image.h"
#include "utils_linkedlist.h"

typedef struct find_blobs_linkedlist_lnk_data
{
    utils_rectangle_t rect;
    uint32_t pixels;
    utils_point_t centroid;
    float rotation;
    uint16_t code, count;
}
find_blobs_linkedlist_lnk_data_t;

void find_blobs(utils_linkedlist_t *list, imlib_image_t *ptr, utils_rectangle_t *roi,
                utils_linkedlist_t *thresholds, bool invert, unsigned int area_threshold, unsigned int pixels_threshold,
                bool merge, int margin);

#endif /* __FIND_BLOBS_H__ */

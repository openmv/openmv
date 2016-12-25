/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __FIND_QRCODES_H__
#define __FIND_QRCODES_H__
#include "imlib_image.h"
#include "utils_linkedlist.h"

typedef struct find_qrcodes_linkedlist_lnk_data
{
    utils_rectangle_t rect;
    size_t payload_len;
    char *payload;
    uint8_t version, ecc_level, mask, data_type;
    uint32_t eci;
}
find_qrcodes_linkedlist_lnk_data_t;

void find_qrcodes(utils_linkedlist_t *list, imlib_image_t *ptr, utils_rectangle_t *roi);

#endif /* __FIND_QRCODES_H__ */

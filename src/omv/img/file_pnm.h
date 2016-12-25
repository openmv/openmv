/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __FILE_PNM_H__
#define __FILE_PNM_H__
#include "imlib_image.h"

typedef enum file_pnm_save_type
{
    FILE_PNM_BINARY,
    FILE_PNM_GRAYSCALE,
    FILE_PNM_RGB565,
    FILE_PNM_BEST
}
file_pnm_save_type_t;

void file_pnm_save(imlib_image_t *ptr, const char *path, file_pnm_save_type_t type, utils_rectangle_t *roi, utils_size_t *res);

#endif /* __FILE_PNM_H__ */

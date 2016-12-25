/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __FILE_BMP_H__
#define __FILE_BMP_H__
#include "imlib_image.h"

typedef enum file_bmp_save_type
{
    FILE_BMP_BINARY,
    FILE_BMP_GRAYSCALE,
    FILE_BMP_RGB565,
    FILE_BMP_BEST
}
file_bmp_save_type_t;

void file_bmp_save(imlib_image_t *ptr, const char *path, file_bmp_save_type_t type, utils_rectangle_t *roi, utils_size_t *res);

#endif /* __FILE_BMP_H__ */

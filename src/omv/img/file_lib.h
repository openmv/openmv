/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __FILE_LIB_H__
#define __FILE_LIB_H__
#include "imlib_image.h"
#include "file_fatfs.h"

typedef enum file_lib_file_type
{
    BMP_FORMAT,
    PNM_FORMAT,
    JPG_FORMAT
}
file_lib_file_type_t;

typedef struct file_lib_file
{
    file_fatfs_file_t fatfs_file;
    file_lib_file_type_t type;
    bool h_mirror;
    bool v_flip;
}
file_lib_file_t;

void file_lib_open(file_lib_file_t *ptr, const char *path);
void file_lib_read_pixel(file_lib_file_t *ptr, void *dst, imlib_image_type_t format);
void file_lib_close(file_lib_file_t *ptr);
void file_lib_save(imlib_image_t *ptr, const char *path, utils_rectangle_t *roi, utils_size_t *res, size_t quality);

#endif /* __FILE_LIB_H__ */

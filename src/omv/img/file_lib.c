/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include <string.h>
#include <mp.h>
#include "fb_alloc.h"
#include "file_bmp.h"
#include "file_pnm.h"
#include "file_lib.h"

static bool is_bmp(const char *path_null_terminator, size_t length)
{
    return
    (
        (length >= 4)
        && ((path_null_terminator[-1] == 'p') || (path_null_terminator[-1] == 'P'))
        && ((path_null_terminator[-2] == 'm') || (path_null_terminator[-2] == 'M'))
        && ((path_null_terminator[-3] == 'b') || (path_null_terminator[-3] == 'B'))
        && ((path_null_terminator[-4] == '.') || (path_null_terminator[-4] == '.'))
    )
    ||
    (
        (length >= 4)
        && ((path_null_terminator[-1] == 'b') || (path_null_terminator[-1] == 'B'))
        && ((path_null_terminator[-2] == 'i') || (path_null_terminator[-2] == 'I'))
        && ((path_null_terminator[-3] == 'd') || (path_null_terminator[-3] == 'D'))
        && ((path_null_terminator[-4] == '.') || (path_null_terminator[-4] == '.'))
    );
}

static bool is_pnm(const char *path_null_terminator, size_t length)
{
    return
    (
        (length >= 4)
        && ((path_null_terminator[-1] == 'm') || (path_null_terminator[-1] == 'M'))
        && ((path_null_terminator[-2] == 'b') || (path_null_terminator[-2] == 'B'))
        && ((path_null_terminator[-3] == 'p') || (path_null_terminator[-3] == 'P'))
        && ((path_null_terminator[-4] == '.') || (path_null_terminator[-4] == '.'))
    )
    ||
    (
        (length >= 4)
        && ((path_null_terminator[-1] == 'm') || (path_null_terminator[-1] == 'M'))
        && ((path_null_terminator[-2] == 'g') || (path_null_terminator[-2] == 'G'))
        && ((path_null_terminator[-3] == 'p') || (path_null_terminator[-3] == 'P'))
        && ((path_null_terminator[-4] == '.') || (path_null_terminator[-4] == '.'))
    )
    ||
    (
        (length >= 4)
        && ((path_null_terminator[-1] == 'm') || (path_null_terminator[-1] == 'M'))
        && ((path_null_terminator[-2] == 'p') || (path_null_terminator[-2] == 'P'))
        && ((path_null_terminator[-3] == 'p') || (path_null_terminator[-3] == 'P'))
        && ((path_null_terminator[-4] == '.') || (path_null_terminator[-4] == '.'))
    )
    ||
    (
        (length >= 4)
        && ((path_null_terminator[-1] == 'm') || (path_null_terminator[-1] == 'M'))
        && ((path_null_terminator[-2] == 'n') || (path_null_terminator[-2] == 'N'))
        && ((path_null_terminator[-3] == 'p') || (path_null_terminator[-3] == 'P'))
        && ((path_null_terminator[-4] == '.') || (path_null_terminator[-4] == '.'))
    );
}

static bool is_jpg(const char *path_null_terminator, size_t length)
{
    return
    (
        (length >= 4)
        && ((path_null_terminator[-1] == 'g') || (path_null_terminator[-1] == 'G'))
        && ((path_null_terminator[-2] == 'p') || (path_null_terminator[-2] == 'P'))
        && ((path_null_terminator[-3] == 'j') || (path_null_terminator[-3] == 'J'))
        && ((path_null_terminator[-4] == '.') || (path_null_terminator[-4] == '.'))
    )
    ||
    (
        (length >= 5)
        && ((path_null_terminator[-1] == 'g') || (path_null_terminator[-1] == 'G'))
        && ((path_null_terminator[-2] == 'e') || (path_null_terminator[-2] == 'E'))
        && ((path_null_terminator[-3] == 'p') || (path_null_terminator[-3] == 'P'))
        && ((path_null_terminator[-4] == 'j') || (path_null_terminator[-4] == 'J'))
        && ((path_null_terminator[-5] == '.') || (path_null_terminator[-5] == '.'))
    )
    ||
    (
        (length >= 4)
        && ((path_null_terminator[-1] == 'e') || (path_null_terminator[-1] == 'E'))
        && ((path_null_terminator[-2] == 'p') || (path_null_terminator[-2] == 'P'))
        && ((path_null_terminator[-3] == 'j') || (path_null_terminator[-3] == 'J'))
        && ((path_null_terminator[-4] == '.') || (path_null_terminator[-4] == '.'))
    )
    ||
    (
        (length >= 4)
        && ((path_null_terminator[-1] == 'f') || (path_null_terminator[-1] == 'F'))
        && ((path_null_terminator[-2] == 'i') || (path_null_terminator[-2] == 'I'))
        && ((path_null_terminator[-3] == 'j') || (path_null_terminator[-3] == 'J'))
        && ((path_null_terminator[-4] == '.') || (path_null_terminator[-4] == '.'))
    )
    ||
    (
        (length >= 5)
        && ((path_null_terminator[-1] == 'f') || (path_null_terminator[-1] == 'F'))
        && ((path_null_terminator[-2] == 'i') || (path_null_terminator[-2] == 'I'))
        && ((path_null_terminator[-3] == 'f') || (path_null_terminator[-3] == 'F'))
        && ((path_null_terminator[-4] == 'j') || (path_null_terminator[-4] == 'J'))
        && ((path_null_terminator[-5] == '.') || (path_null_terminator[-5] == '.'))
    )
    ||
    (
        (length >= 4)
        && ((path_null_terminator[-1] == 'i') || (path_null_terminator[-1] == 'I'))
        && ((path_null_terminator[-2] == 'f') || (path_null_terminator[-2] == 'F'))
        && ((path_null_terminator[-3] == 'j') || (path_null_terminator[-3] == 'J'))
        && ((path_null_terminator[-4] == '.') || (path_null_terminator[-4] == '.'))
    );
}

void file_lib_save(imlib_image_t *ptr, const char *path, utils_rectangle_t *roi, utils_size_t *res, size_t quality)
{
    size_t length = strlen(path);
    const char *path_null_terminator = path + length;

    if (IMLIB_IMAGE_GET_IMAGE_IS_JPG(ptr)) { // JPG Default
        if (is_jpg(path_null_terminator, length)) {
            // file_jpg_save(ptr, new_path, quality);
        } else {
            // const char *new_path = strcat(strcpy(fb_alloc(length + 5), path), ".jpg");
            // file_jpg_save(ptr, new_path, quality);
            // fb_free();
        }
    } else if (is_bmp(path_null_terminator, length)) {
        file_bmp_save(ptr, path, roi, res);
    } else if (is_pnm(path_null_terminator, length)) {
        file_pnm_save(ptr, path, roi, res);
    } else if (is_jpg(path_null_terminator, length)) {
        // file_jpg_save(ptr, new_path, quality);
    } else { // BMP Default
        const char *new_path = strcat(strcpy(fb_alloc(length + 5), path), ".bmp");
        file_bmp_save(ptr, new_path, roi, res);
        fb_free();
    }
}

/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include <stdio.h>
#include "imlib_color.h"
#include "file_fatfs.h"
#include "file_pnm.h"

void file_pnm_save(imlib_image_t *ptr, const char *path, utils_rectangle_t *roi, utils_size_t *res)
{
    utils_size_check(&(roi->s));
    utils_size_check(res);
    imlib_image_check_overlap(ptr, roi);

    utils_rectangle_t rect;
    utils_rectangle_copy(&rect, roi);
    imlib_image_intersected(ptr, &rect);
    utils_size_check(&(rect.s));

    file_fatfs_file_t file;
    file_fatfs_write_open(&file, path, true);

    switch(ptr->type) {
        case IMLIB_IMAGE_TYPE_BINARY: {
            // File Header
            char buffer[16]; // exactly big enough for 5-digit w/h
            file_fatfs_write_data(&file, buffer, snprintf(buffer, 16, "P4\n%d %d\n", res->w, res->h));
            // File Data
            IMLIB_IMAGE_COMPUTE_TARGET_SIZE_SCALE_FACTOR(res, &rect);
            for (size_t i = 0; i < res->h; i++) {
                uint32_t *row_ptr = IMLIB_IMAGE_COMPUTE_SCALED_BINARY_PIXEL_ROW_PTR(ptr, i);
                uint32_t pixels = 0;
                size_t number = 0;
                for (size_t j = 0; j < res->w; j++) {
                    pixels |= IMLIB_IMAGE_GET_SCALED_BINARY_PIXEL_FAST(row_ptr, j) << (UINT8_T_MASK - (j & UINT8_T_MASK));
                    number += 1;
                    if (number == UINT8_T_BITS) {
                        file_fatfs_write_byte(&file, pixels);
                        pixels = 0;
                        number = 0;
                    }
                }
                if (number) {
                    file_fatfs_write_byte(&file, pixels);
                    pixels = 0;
                    number = 0;
                }
            }
            break;
        }
        case IMLIB_IMAGE_TYPE_GRAYSCALE: {
            // File Header
            char buffer[20]; // exactly big enough for 5-digit w/h
            file_fatfs_write_data(&file, buffer, snprintf(buffer, 20, "P5\n%d %d\n255\n", res->w, res->h));
            // File Data
            IMLIB_IMAGE_COMPUTE_TARGET_SIZE_SCALE_FACTOR(res, &rect);
            for (size_t i = 0; i < res->h; i++) {
                uint8_t *row_ptr = IMLIB_IMAGE_COMPUTE_SCALED_GRAYSCALE_PIXEL_ROW_PTR(ptr, i);
                for (size_t j = 0; j < res->w; j++) {
                    file_fatfs_write_byte(&file, IMLIB_IMAGE_GET_SCALED_GRAYSCALE_PIXEL_FAST(row_ptr, j));
                }
            }
            break;
        }
        case IMLIB_IMAGE_TYPE_RGB565: {
            // File Header
            char buffer[20]; // exactly big enough for 5-digit w/h
            file_fatfs_write_data(&file, buffer, snprintf(buffer, 20, "P6\n%d %d\n255\n", res->w, res->h));
            // File Data
            IMLIB_IMAGE_COMPUTE_TARGET_SIZE_SCALE_FACTOR(res, &rect);
            for (size_t i = 0; i < res->h; i++) {
                uint16_t *row_ptr = IMLIB_IMAGE_COMPUTE_SCALED_RGB565_PIXEL_ROW_PTR(ptr, i);
                for (size_t j = 0; j < res->w; j++) {
                    uint16_t pixel = IMLIB_IMAGE_GET_SCALED_RGB565_PIXEL_FAST(row_ptr, j);
                    char buff[3];
                    buff[0] = IMLIB_COLOR_RGB565_TO_R8(pixel);
                    buff[1] = IMLIB_COLOR_RGB565_TO_G8(pixel);
                    buff[2] = IMLIB_COLOR_RGB565_TO_B8(pixel);
                    file_fatfs_write_data(&file, buff, 3);
                }
            }
            break;
        }
        default: {
            // Empty File.
            break;
        }
    }

    file_fatfs_close(&file);
}

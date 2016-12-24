/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include "file_fatfs.h"
#include "file_bmp.h"

void file_bmp_save(imlib_image_t *ptr, const char *path, utils_rectangle_t *roi, utils_size_t *res)
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
            size_t row_bytes = ((res->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * sizeof(uint32_t);
            size_t data_size = row_bytes * res->h;
            // File Header (14 bytes)
            file_fatfs_write_byte(&file, 'B');
            file_fatfs_write_byte(&file, 'M');
            file_fatfs_write_long(&file, 14 + 40 + 8 + data_size);
            file_fatfs_write_word(&file, 0);
            file_fatfs_write_word(&file, 0);
            file_fatfs_write_long(&file, 14 + 40 + 8);
            // Info Header (40 bytes)
            file_fatfs_write_long(&file, 40);
            file_fatfs_write_long(&file, res->w);
            file_fatfs_write_long(&file, -res->h); // v-flip
            file_fatfs_write_word(&file, 1);
            file_fatfs_write_word(&file, 1); // bits per pixel
            file_fatfs_write_long(&file, 0);
            file_fatfs_write_long(&file, data_size);
            file_fatfs_write_long(&file, 0);
            file_fatfs_write_long(&file, 0);
            file_fatfs_write_long(&file, 2);
            file_fatfs_write_long(&file, 2);
            // Color Table (8 bytes)
            file_fatfs_write_long(&file, 0x000000);
            file_fatfs_write_long(&file, 0xFFFFFF);
            // File Data
            IMLIB_IMAGE_COMPUTE_TARGET_SIZE_SCALE_FACTOR(res, &rect);
            for (size_t i = 0; i < res->h; i++) {
                uint32_t *row_ptr = IMLIB_IMAGE_COMPUTE_SCALED_BINARY_PIXEL_ROW_PTR(ptr, i);
                uint32_t pixels = 0;
                size_t number = 0;
                for (size_t j = 0; j < res->w; j++) {
                    pixels |= IMLIB_IMAGE_GET_SCALED_BINARY_PIXEL_FAST(row_ptr, j) << (j & UINT32_T_MASK);
                    number += 1;
                    if (number == UINT32_T_BITS) {
                        file_fatfs_write_long(&file, pixels);
                        pixels = 0;
                        number = 0;
                    }
                }
                if (number) {
                    file_fatfs_write_long(&file, pixels);
                    pixels = 0;
                    number = 0;
                }
            }
            break;
        }
        case IMLIB_IMAGE_TYPE_GRAYSCALE: {
            size_t row_bytes = (((res->w * UINT8_T_BITS) + UINT32_T_MASK) >> UINT32_T_SHIFT) * sizeof(uint32_t);
            size_t data_size = row_bytes * res->h;
            size_t row_waste = (row_bytes >> (OTHER_LOG2(sizeof(uint8_t)) - 1)) - res->w;
            // File Header (14 bytes)
            file_fatfs_write_byte(&file, 'B');
            file_fatfs_write_byte(&file, 'M');
            file_fatfs_write_long(&file, 14 + 40 + 1024 + data_size);
            file_fatfs_write_word(&file, 0);
            file_fatfs_write_word(&file, 0);
            file_fatfs_write_long(&file, 14 + 40 + 1024);
            // Info Header (40 bytes)
            file_fatfs_write_long(&file, 40);
            file_fatfs_write_long(&file, res->w);
            file_fatfs_write_long(&file, -res->h); // v-flip
            file_fatfs_write_word(&file, 1);
            file_fatfs_write_word(&file, UINT8_T_BITS); // bits per pixel
            file_fatfs_write_long(&file, 0);
            file_fatfs_write_long(&file, data_size);
            file_fatfs_write_long(&file, 0);
            file_fatfs_write_long(&file, 0);
            file_fatfs_write_long(&file, 256);
            file_fatfs_write_long(&file, 256);
            // Color Table (1024 bytes)
            for (size_t i = 0; i < 256; i++) {
                file_fatfs_write_long(&file, (i << 16) | (i << 8) | i);
            }
            // File Data
            IMLIB_IMAGE_COMPUTE_TARGET_SIZE_SCALE_FACTOR(res, &rect);
            for (size_t i = 0; i < res->h; i++) {
                uint8_t *row_ptr = IMLIB_IMAGE_COMPUTE_SCALED_GRAYSCALE_PIXEL_ROW_PTR(ptr, i);
                for (size_t j = 0; j < res->w; j++) {
                    file_fatfs_write_byte(&file, IMLIB_IMAGE_GET_SCALED_GRAYSCALE_PIXEL_FAST(row_ptr, j));
                }
                for (size_t j = 0; j < row_waste; j++) {
                    file_fatfs_write_byte(&file, 0);
                }
            }
            break;
        }
        case IMLIB_IMAGE_TYPE_RGB565: {
            size_t row_bytes = (((res->w * UINT16_T_BITS) + UINT32_T_MASK) >> UINT32_T_SHIFT) * sizeof(uint32_t);
            size_t data_size = row_bytes * res->h;
            size_t row_waste = (row_bytes >> (OTHER_LOG2(sizeof(uint16_t)) - 1)) - res->w;
            // File Header (14 bytes)
            file_fatfs_write_byte(&file, 'B');
            file_fatfs_write_byte(&file, 'M');
            file_fatfs_write_long(&file, 14 + 40 + 12 + data_size);
            file_fatfs_write_word(&file, 0);
            file_fatfs_write_word(&file, 0);
            file_fatfs_write_long(&file, 14 + 40 + 12);
            // Info Header (40 bytes)
            file_fatfs_write_long(&file, 40);
            file_fatfs_write_long(&file, res->w);
            file_fatfs_write_long(&file, -res->h); // v-flip
            file_fatfs_write_word(&file, 1);
            file_fatfs_write_word(&file, UINT16_T_BITS); // bits per pixel
            file_fatfs_write_long(&file, 3);
            file_fatfs_write_long(&file, data_size);
            file_fatfs_write_long(&file, 0);
            file_fatfs_write_long(&file, 0);
            file_fatfs_write_long(&file, 0);
            file_fatfs_write_long(&file, 0);
            // Bit Masks (12 bytes)
            file_fatfs_write_long(&file, 0xF800); // R5
            file_fatfs_write_long(&file, 0x07E0); // G6
            file_fatfs_write_long(&file, 0x001F); // B5
            // File Data
            IMLIB_IMAGE_COMPUTE_TARGET_SIZE_SCALE_FACTOR(res, &rect);
            for (size_t i = 0; i < res->h; i++) {
                uint16_t *row_ptr = IMLIB_IMAGE_COMPUTE_SCALED_RGB565_PIXEL_ROW_PTR(ptr, i);
                for (size_t j = 0; j < res->w; j++) {
                    file_fatfs_write_word(&file, IMLIB_IMAGE_REV_RGB565_PIXEL(IMLIB_IMAGE_GET_SCALED_RGB565_PIXEL_FAST(row_ptr, j)));
                }
                for (size_t j = 0; j < row_waste; j++) {
                    file_fatfs_write_word(&file, 0);
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

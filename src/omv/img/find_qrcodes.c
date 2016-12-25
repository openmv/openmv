/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include "xalloc.h"
#include "fb_alloc.h"
#include "quirc_quirc.h"
#include "imlib_color.h"
#include "find_qrcodes.h"

void find_qrcodes(utils_linkedlist_t *list, imlib_image_t *ptr, utils_rectangle_t *roi)
{
    utils_size_check(&(roi->s));
    imlib_image_check_overlap(ptr, roi);

    utils_rectangle_t rect;
    utils_rectangle_copy(&rect, roi);
    imlib_image_intersected(ptr, &rect);
    utils_size_check(&(rect.s));

    struct quirc *controller = quirc_new();
    quirc_resize(controller, rect.s.w, rect.s.h);

    int w, h;
    uint8_t *grayscale_image = quirc_begin(controller, &w, &h);

    switch(ptr->type) {
        case IMLIB_IMAGE_TYPE_BINARY: {
            for (int y = rect.p.y, yy = rect.p.y + h; y < yy; y++) {
                uint32_t *row_ptr = IMLIB_IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y);
                for (int x = rect.p.x, xx = rect.p.x + w; x < xx; x++) {
                    *(grayscale_image++) = IMLIB_COLOR_BINARY_TO_GRAYSCALE(IMLIB_IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                }
            }
            break;
        }
        case IMLIB_IMAGE_TYPE_GRAYSCALE: {
            for (int y = rect.p.y, yy = rect.p.y + h; y < yy; y++) {
                uint8_t *row_ptr = IMLIB_IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y);
                for (int x = rect.p.x, xx = rect.p.x + w; x < xx; x++) {
                    *(grayscale_image++) = IMLIB_IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x);
                }
            }
            break;
        }
        case IMLIB_IMAGE_TYPE_RGB565: {
            for (int y = rect.p.y, yy = rect.p.y + h; y < yy; y++) {
                uint16_t *row_ptr = IMLIB_IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y);
                for (int x = rect.p.x, xx = rect.p.x + w; x < xx; x++) {
                    *(grayscale_image++) = IMLIB_COLOR_RGB565_TO_GRAYSCALE(IMLIB_IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                }
            }
            break;
        }
        default: {
            memset(grayscale_image, 0, w * h);
            break;
        }
    }

    quirc_end(controller);
    utils_linkedlist_alloc(list, sizeof(find_qrcodes_linkedlist_lnk_data_t));

    for (int i = 0, j = quirc_count(controller); i < j; i++) {
        struct quirc_code *code = fb_alloc(sizeof(struct quirc_code));
        struct quirc_data *data = fb_alloc(sizeof(struct quirc_data));
        quirc_extract(controller, i, code);

        if(quirc_decode(code, data) == QUIRC_SUCCESS) {
            find_qrcodes_linkedlist_lnk_data_t lnk_data;
            utils_rectangle_init(&(lnk_data.rect), code->corners[0].x, code->corners[0].y, 0, 0);

            for (size_t k = 1; k < (sizeof(code->corners) / sizeof(code->corners[0])); k++) {
                utils_rectangle_t temp;
                utils_rectangle_init(&temp, code->corners[k].x, code->corners[k].y, 0, 0);
                utils_rectangle_united(&(lnk_data.rect), &temp);
            }

            // Payload is already null terminated.
            lnk_data.payload_len = data->payload_len;
            lnk_data.payload = xalloc(data->payload_len);
            memcpy(lnk_data.payload, data->payload, data->payload_len);

            lnk_data.version = data->version;
            lnk_data.ecc_level = data->ecc_level;
            lnk_data.mask = data->mask;
            lnk_data.data_type = data->data_type;
            lnk_data.eci = data->eci;

            utils_linkedlist_push_back(list, &lnk_data);
        }

        fb_free();
        fb_free();
    }

    quirc_destroy(controller);
}

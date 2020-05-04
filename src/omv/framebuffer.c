/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Framebuffer functions.
 */
#include "mpprint.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"

extern char _fb_base;
framebuffer_t *fb_framebuffer = (framebuffer_t *) &_fb_base;

extern char _jpeg_buf;
jpegbuffer_t *jpeg_fb_framebuffer = (jpegbuffer_t *) &_jpeg_buf;

void fb_set_streaming_enabled(bool enable)
{
    MAIN_FB()->streaming_enabled = enable;
}

bool fb_get_streaming_enabled()
{
    return MAIN_FB()->streaming_enabled;
}

int fb_encode_for_ide_new_size(image_t *img)
{
    return (((img->bpp * 8) + 5) / 6) + 2;
}

void fb_encode_for_ide(uint8_t *ptr, image_t *img)
{
    *ptr++ = 0xFE;

    for(int i = 0, j = (img->bpp / 3) * 3; i < j; i += 3) {
        int x = 0;
        x |= img->data[i + 0] << 0;
        x |= img->data[i + 1] << 8;
        x |= img->data[i + 2] << 16;
        *ptr++ = 0x80 | ((x >> 0) & 0x3F);
        *ptr++ = 0x80 | ((x >> 6) & 0x3F);
        *ptr++ = 0x80 | ((x >> 12) & 0x3F);
        *ptr++ = 0x80 | ((x >> 18) & 0x3F);
    }

    if((img->bpp % 3) == 2) { // 2 bytes -> 16-bits -> 24-bits sent
        int x = 0;
        x |= img->data[img->bpp - 2] << 0;
        x |= img->data[img->bpp - 1] << 8;
        *ptr++ = 0x80 | ((x >> 0) & 0x3F);
        *ptr++ = 0x80 | ((x >> 6) & 0x3F);
        *ptr++ = 0x80 | ((x >> 12) & 0x3F);
    }

    if((img->bpp % 3) == 1) { // 1 byte -> 8-bits -> 16-bits sent
        int x = 0;
        x |= img->data[img->bpp - 1] << 0;
        *ptr++ = 0x80 | ((x >> 0) & 0x3F);
        *ptr++ = 0x80 | ((x >> 6) & 0x3F);
    }

    *ptr++ = 0xFE;
}

uint32_t fb_buffer_size()
{
    switch (MAIN_FB()->bpp) {
        case IMAGE_BPP_BINARY: {
            return ((MAIN_FB()->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * MAIN_FB()->h;
        }
        case IMAGE_BPP_GRAYSCALE: {
            return (MAIN_FB()->w * MAIN_FB()->h) * sizeof(uint8_t);
        }
        case IMAGE_BPP_RGB565: {
            return (MAIN_FB()->w * MAIN_FB()->h) * sizeof(uint16_t);
        }
        case IMAGE_BPP_BAYER: {
            return MAIN_FB()->w * MAIN_FB()->h;
        }
        default: { // JPEG
            return MAIN_FB()->bpp;
        }
    }
}

void fb_update_jpeg_buffer()
{
    static int overflow_count = 0;

    if (MAIN_FB()->streaming_enabled && JPEG_FB()->enabled) {
        if (MAIN_FB()->bpp > 3) {
            bool does_not_fit = false;
            // Lock FB
            if (mutex_try_lock(&JPEG_FB()->lock, MUTEX_TID_OMV)) {
                if((OMV_JPEG_BUF_SIZE-64) < MAIN_FB()->bpp) {
                    // image won't fit. so don't copy.
                    JPEG_FB()->w = 0; JPEG_FB()->h = 0; JPEG_FB()->size = 0;
                    does_not_fit = true;
                } else {
                    memcpy(JPEG_FB()->pixels, MAIN_FB()->pixels, MAIN_FB()->bpp);
                    JPEG_FB()->w = MAIN_FB()->w; JPEG_FB()->h = MAIN_FB()->h; JPEG_FB()->size = MAIN_FB()->bpp;
                }

                // Unlock the framebuffer mutex
                mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_OMV);
            }
            if (does_not_fit) {
                image_t out = { .w=MAIN_FB()->w, .h=MAIN_FB()->h, .bpp=MAIN_FB()->bpp, .data=MAIN_FB()->pixels };
                int new_size = fb_encode_for_ide_new_size(&out);
                fb_alloc_mark();
                uint8_t *temp = fb_alloc(new_size, FB_ALLOC_NO_HINT);
                fb_encode_for_ide(temp, &out);
                (MP_PYTHON_PRINTER)->print_strn((MP_PYTHON_PRINTER)->data, (const char *) temp, new_size);
                fb_alloc_free_till_mark();
            }
        } else if (MAIN_FB()->bpp >= 0) {
            // Lock FB
            if (mutex_try_lock(&JPEG_FB()->lock, MUTEX_TID_OMV)) {
                // Set JPEG src and dst images.
                image_t src = {.w=MAIN_FB()->w, .h=MAIN_FB()->h, .bpp=MAIN_FB()->bpp,     .pixels=MAIN_FB()->pixels};
                image_t dst = {.w=MAIN_FB()->w, .h=MAIN_FB()->h, .bpp=(OMV_JPEG_BUF_SIZE-64),  .pixels=JPEG_FB()->pixels};

                // Note: lower quality saves USB bandwidth and results in a faster IDE FPS.
                bool overflow = jpeg_compress(&src, &dst, JPEG_FB()->quality, false);
                if (overflow == true) {
                    // JPEG buffer overflowed, reduce JPEG quality for the next frame
                    // and skip the current frame. The IDE doesn't receive this frame.
                    if (JPEG_FB()->quality > 1) {
                        // Keep this quality for the next n frames
                        overflow_count = 60;
                        JPEG_FB()->quality = IM_MAX(1, (JPEG_FB()->quality/2));
                    }
                    JPEG_FB()->w = 0; JPEG_FB()->h = 0; JPEG_FB()->size = 0;
                } else {
                    if (overflow_count) {
                        overflow_count--;
                    }
                    // No buffer overflow, increase quality up to max quality based on frame size
                    if (overflow_count == 0 && JPEG_FB()->quality
                           < ((fb_buffer_size() > JPEG_QUALITY_THRESH) ? JPEG_QUALITY_LOW:JPEG_QUALITY_HIGH)) {
                        JPEG_FB()->quality++;
                    }
                    // Set FB from JPEG image
                    JPEG_FB()->w = dst.w; JPEG_FB()->h = dst.h; JPEG_FB()->size = dst.bpp;
                }

                // Unlock the framebuffer mutex
                mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_OMV);
            }
        }
    }
}

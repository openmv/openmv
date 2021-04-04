/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Framebuffer functions.
 */
#include <stdio.h>
#include "mpprint.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"

#define CONSERVATIVE_JPEG_BUF_SIZE  (OMV_JPEG_BUF_SIZE-64)
#define N_BUFFERS       (2)
#define FB_ALIGNMENT    (32)
#define FB_ALIGN_SIZE(x) (((x + FB_ALIGNMENT - 1) / FB_ALIGNMENT) * FB_ALIGNMENT)

extern uint8_t _fb_base;            // Pointer to the raw buffer.
static bool streaming_enabled = false;
static framebuffer_t buffers[N_BUFFERS];
framebuffer_t *framebuffer = NULL;  // Current framebuffer pointer.

extern char _jpeg_buf;
jpegbuffer_t *jpeg_framebuffer = (jpegbuffer_t *) &_jpeg_buf;

void fb_set_streaming_enabled(bool enable)
{
    streaming_enabled = enable;
}

bool fb_get_streaming_enabled()
{
    return streaming_enabled;
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

void framebuffer_init0()
{
    // Save last JPEG FB enabled flag state
    int fb_enabled = JPEG_FB()->enabled;

    // Init JPEG framebuffer.
    memset(JPEG_FB(), 0, sizeof(*JPEG_FB()));

    // Set default quality
    JPEG_FB()->quality = ((JPEG_QUALITY_HIGH - JPEG_QUALITY_LOW) / 2) + JPEG_QUALITY_LOW;

    // Restore the JPEG buffer enabled state. Note this flag is controlled by the IDE.
    JPEG_FB()->enabled = fb_enabled;

    // Init camera framebuffers
    memset(buffers, 0, sizeof(framebuffer_t) * N_BUFFERS);

    for (int i=0; i<N_BUFFERS; i++) {
        // Skip the first frame.
        buffers[i].bpp = -1;
        // Set all buffers to point to the start of FB initially.
        buffers[i].pixels = &_fb_base;
    }

    // Set framebuffer pointer to first buffer by default.
    framebuffer = &buffers[0];

    // Enable streaming by default. Note this flag is controlled by the OpenMV Cam.
    streaming_enabled = true;
}

void framebuffer_initialize_image(image_t *img)
{
    img->w      = framebuffer->w;
    img->h      = framebuffer->h;
    img->bpp    = framebuffer->bpp;
    img->data   = framebuffer->pixels;
}

void framebuffer_initialize_image_from_fb(image_t *img, framebuffer_t *fb)
{
    img->w      = fb->w;
    img->h      = fb->h;
    img->bpp    = fb->bpp;
    img->data   = fb->pixels;
}

static void initialize_jpeg_buf_from_image(image_t *img)
{
    if (!img) {
        jpeg_framebuffer->w = 0;
        jpeg_framebuffer->h = 0;
        jpeg_framebuffer->size = 0;
    } else {
        jpeg_framebuffer->w = img->w;
        jpeg_framebuffer->h = img->h;
        jpeg_framebuffer->size = img->bpp;
    }
}

void framebuffer_update_jpeg_buffer()
{
    static int overflow_count = 0;

    // Find the last used framebuffer and compress it, if no frame
    // is set to dirty, use the first/main frame buffer by default.
    framebuffer_t *fb = framebuffer;
    for (int i=0; i<N_BUFFERS; i++) {
        if (FB_GET_FLAG(&buffers[i], FB_DIRTY)) {
            fb = &buffers[i];
            FB_CLR_FLAG(fb, FB_DIRTY);
            break;
        }
    }
    image_t src;
    framebuffer_initialize_image_from_fb(&src, fb);
    debug_printf("compress fb:%d buf:%p bpp:%d \n", fb == &buffers[0] ? 0 : 1, fb->pixels, src.bpp);

    if (streaming_enabled && jpeg_framebuffer->enabled) {
        if (src.bpp > 3) {
            bool does_not_fit = false;

            if (mutex_try_lock(&jpeg_framebuffer->lock, MUTEX_TID_OMV)) {
                if(CONSERVATIVE_JPEG_BUF_SIZE < src.bpp) {
                    initialize_jpeg_buf_from_image(NULL);
                    does_not_fit = true;
                } else {
                    initialize_jpeg_buf_from_image(&src);
                    memcpy(jpeg_framebuffer->pixels, src.pixels, src.bpp);
                }

                mutex_unlock(&jpeg_framebuffer->lock, MUTEX_TID_OMV);
            }

            if (does_not_fit) {
                printf("Warning: JPEG too big! Trying framebuffer transfer using fallback method!\n");
                int new_size = fb_encode_for_ide_new_size(&src);
                fb_alloc_mark();
                uint8_t *temp = fb_alloc(new_size, FB_ALLOC_NO_HINT);
                fb_encode_for_ide(temp, &src);
                (MP_PYTHON_PRINTER)->print_strn((MP_PYTHON_PRINTER)->data, (const char *) temp, new_size);
                fb_alloc_free_till_mark();
            }
        } else if (src.bpp >= 0) {
            if (mutex_try_lock(&jpeg_framebuffer->lock, MUTEX_TID_OMV)) {
                image_t dst = {.w=src.w, .h=src.h, .bpp=CONSERVATIVE_JPEG_BUF_SIZE, .pixels=jpeg_framebuffer->pixels};
                // Note: lower quality saves USB bandwidth and results in a faster IDE FPS.
                bool overflow = jpeg_compress(&src, &dst, jpeg_framebuffer->quality, false);

                if (overflow) {
                    // JPEG buffer overflowed, reduce JPEG quality for the next frame
                    // and skip the current frame. The IDE doesn't receive this frame.
                    if (jpeg_framebuffer->quality > 1) {
                        // Keep this quality for the next n frames
                        overflow_count = 60;
                        jpeg_framebuffer->quality = IM_MAX(1, (jpeg_framebuffer->quality/2));
                    }

                    initialize_jpeg_buf_from_image(NULL);
                } else {
                    if (overflow_count) {
                        overflow_count--;
                    }

                    // Dynamically adjust our quality if the image is huge.
                    bool big_frame_buffer = image_size(&src) > JPEG_QUALITY_THRESH;
                    int jpeg_quality_max = big_frame_buffer ? JPEG_QUALITY_LOW : JPEG_QUALITY_HIGH;

                    // No buffer overflow, increase quality up to max quality based on frame size...
                    if ((!overflow_count) && (jpeg_framebuffer->quality < jpeg_quality_max)) {
                        jpeg_framebuffer->quality++;
                    }

                    initialize_jpeg_buf_from_image(&dst);
                }

                mutex_unlock(&jpeg_framebuffer->lock, MUTEX_TID_OMV);
            }
        }
    }
}

int32_t framebuffer_get_x()
{
    return framebuffer->x;
}

int32_t framebuffer_get_y()
{
    return framebuffer->y;
}

int32_t framebuffer_get_u()
{
    return framebuffer->u;
}

int32_t framebuffer_get_v()
{
    return framebuffer->v;
}

int32_t framebuffer_get_width()
{
    return framebuffer->w;
}

int32_t framebuffer_get_height()
{
    return framebuffer->h;
}

int32_t framebuffer_get_depth()
{
    return framebuffer->bpp;
}

void framebuffer_set(int32_t w, int32_t h, int32_t bpp)
{
    framebuffer->w = w;
    framebuffer->h = h;
    framebuffer->bpp = bpp;
}

uint8_t *framebuffer_get_buffer()
{
    return framebuffer->pixels;
}

uint8_t *framebuffer_get_buffer_end()
{
    image_t img;
    framebuffer_t *fb = &buffers[N_BUFFERS-1];
    framebuffer_initialize_image_from_fb(&img, fb);
    return fb->pixels + FB_ALIGN_SIZE(image_size(&img));
}

uint32_t framebuffer_get_buffer_size()
{
    uint32_t size = (uint32_t) (fb_alloc_stack_pointer() - ((char *) framebuffer->pixels));
    // We don't want to give all of the frame buffer RAM to the frame buffer. So, we will limit the
    // maximum amount of RAM we return.
    size = IM_MIN(size, OMV_RAW_BUF_SIZE);
    // Needs to be a multiple of 32 for DMA transfers...
    return (size / 32) * 32;
}

framebuffer_t *framebuffer_get_framebuffer()
{
   return framebuffer;
}

void framebuffer_swap_buffers(uint32_t offset)
{
    // Save the current frame buffer before swapping.
    framebuffer_t *old_fb = framebuffer;

    // Set/Clear frame buffer flags.
    FB_CLR_FLAG(old_fb, FB_USED);
    FB_SET_FLAG(old_fb, FB_DIRTY);

    // Swap the main frame buffer pointer.
    for (int i=0; i<N_BUFFERS; i++) {
        if (framebuffer != &buffers[i]) {
            framebuffer = &buffers[i];
            break;
        }
    }

    // Initialize from the first buffer, copying x,y,w,h,u,v, etc...
    framebuffer->x = old_fb->x;
    framebuffer->y = old_fb->y;
    framebuffer->w = old_fb->w;
    framebuffer->h = old_fb->h;
    framebuffer->u = old_fb->u;
    framebuffer->v = old_fb->v;
    framebuffer->x = old_fb->x;
    framebuffer->bpp  = -1;

    // The second buffer needs to be initialized from fb0
    if (framebuffer != &buffers[0] && framebuffer->pixels == &_fb_base) {
        // If this is first time this buffer is used, adjust its pixels pointer
        // offset within the raw buffer to point just after the first frame buffer.
        framebuffer->pixels = &_fb_base + FB_ALIGN_SIZE(offset);
    }

    // Set/Clear frame buffer flags.
    FB_CLR_FLAG(framebuffer, FB_DIRTY);
    FB_SET_FLAG(framebuffer, FB_USED);
}

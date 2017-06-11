/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Framebuffer stuff.
 *
 */
#include "imlib.h"
#include "omv_boardconfig.h"
#include "framebuffer.h"
// If buffer size is bigger than this threshold, the quality is reduced.
// This is only used for JPEG images sent to the IDE not normal compression.
#define JPEG_QUALITY_THRESH     (160*120*2)

extern char _fb_base;
framebuffer_t *fb = (framebuffer_t *) &_fb_base;

extern char _jpeg_buf;
jpegbuffer_t *jpeg_fb = (jpegbuffer_t *) &_jpeg_buf;

void copy_fb_to_jpeg_fb()
{
    static int overflow_count = 0;

    if ((fb->bpp > 3) && JPEG_FB()->enabled) {
        // Lock FB
        if (mutex_try_lock(&JPEG_FB()->lock, MUTEX_TID_OMV)) {
            if(OMV_JPEG_BUF_SIZE < fb->bpp) {
                // image won't fit. so don't copy.
                JPEG_FB()->w = 0; JPEG_FB()->h = 0; JPEG_FB()->size = 0;
            } else {
                memcpy(JPEG_FB()->pixels, fb->pixels, fb->bpp);
                JPEG_FB()->w = fb->w; JPEG_FB()->h = fb->h; JPEG_FB()->size = fb->bpp;
            }

            // Unlock the framebuffer mutex
            mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_OMV);
        }
    } else if ((fb->bpp > 0) && JPEG_FB()->enabled) {
        // Lock FB
        if (mutex_try_lock(&JPEG_FB()->lock, MUTEX_TID_OMV)) {
            // Set JPEG src and dst images.
            image_t src = {.w=fb->w, .h=fb->h, .bpp=fb->bpp,            .pixels=fb->pixels};
            image_t dst = {.w=fb->w, .h=fb->h, .bpp=OMV_JPEG_BUF_SIZE,  .pixels=JPEG_FB()->pixels};

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
                if (overflow_count == 0 &&
                        JPEG_FB()->quality < ((MAIN_FB_SIZE() > JPEG_QUALITY_THRESH) ? 35:60)) {
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

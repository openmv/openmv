/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Framebuffer functions.
 */
#include <stdio.h>
#include "py/mphal.h"
#include "mpprint.h"
#include "fmath.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"

extern char _fb_memory_start;
extern char _fb_memory_end;
static framebuffer_t framebuffer;

extern char _jpeg_memory_start;
extern char _jpeg_memory_end;
jpegbuffer_t jpegbuffer;

void framebuffer_init0() {
    // Save enable flag before resetting the state.
    int fb_enabled = jpegbuffer.enabled;

    // Initialize the static frame buffer. 
    framebuffer_init(&framebuffer, &_fb_memory_start, &_fb_memory_end - &_fb_memory_start, false);

    // Initialize jpeg buffer. 
    memset(&jpegbuffer, 0, sizeof(jpegbuffer_t));
    mutex_init0(&jpegbuffer.lock);
    jpegbuffer.enabled = fb_enabled;
    jpegbuffer.pixels = (uint8_t *) &_jpeg_memory_start;
    jpegbuffer.quality = ((OMV_JPEG_QUALITY_HIGH - OMV_JPEG_QUALITY_LOW) / 2) + OMV_JPEG_QUALITY_LOW;
}

void framebuffer_init(framebuffer_t *fb, void *buff, size_t size, bool dynamic) {
    // Clear framebuffers
    memset(fb, 0, sizeof(framebuffer_t));

    fb->raw_size = size;
    fb->raw_base = buff;
    fb->dynamic = dynamic;
}

void framebuffer_init_image(framebuffer_t *fb, image_t *img) {
    if (img != NULL) {
        vbuffer_t *buffer = framebuffer_acquire(fb, FB_FLAG_USED | FB_FLAG_PEEK);

        img->w = fb->w;
        img->h = fb->h;
        img->size = fb->size;
        img->pixfmt = fb->pixfmt;
        img->pixels = (buffer == NULL) ? NULL : buffer->data;
    }
}

void framebuffer_init_from_image(framebuffer_t *fb, image_t *img) {
    fb->w = img->w;
    fb->h = img->h;
    fb->size = img->size;
    fb->pixfmt = img->pixfmt;
}

static void jpegbuffer_init_from_image(image_t *img) {
    if (img == NULL) {
        jpegbuffer.w = 0;
        jpegbuffer.h = 0;
        jpegbuffer.size = 0;
    } else {
        jpegbuffer.w = img->w;
        jpegbuffer.h = img->h;
        jpegbuffer.size = img->size;
    }
}

framebuffer_t *framebuffer_get(size_t id) {
    return &framebuffer;
}

char *framebuffer_pool_start(framebuffer_t *fb, size_t buf_count) {
    size_t qsize = (buf_count <= 3) ? 0 : queue_calc_size(buf_count);
    return fb->raw_base + OMV_ALIGN_TO(qsize * 2, FRAMEBUFFER_ALIGNMENT);
}

char *framebuffer_pool_end(framebuffer_t *fb) {
    char *pool_start = framebuffer_pool_start(fb, fb->buf_count);
    return pool_start + ((fb->buf_size + sizeof(vbuffer_t)) * fb->buf_count);
}

void *framebuffer_pool_get(framebuffer_t *fb, int32_t index) {
    char *pool_start = framebuffer_pool_start(fb, fb->buf_count);
    return pool_start + ((fb->buf_size + sizeof(vbuffer_t)) * index);
}

void framebuffer_flush(framebuffer_t *fb) {
    // Invalidate the frame buffer.
    fb->pixfmt = PIXFORMAT_INVALID;

    // Drop all frame buffers.
    if (fb->buf_count) {
        queue_flush(fb->free_queue);
        queue_flush(fb->used_queue);
    }

    for (size_t i=0; i<fb->buf_count; i++) {
        vbuffer_t *buffer = framebuffer_pool_get(fb, i);

        // Reset the buffer's state.
        framebuffer_reset(buffer);

        // Discard any cached CPU writes.
        #ifdef __DCACHE_PRESENT
        SCB_InvalidateDCache_by_Addr(buffer->data, fb->buf_size);
        #endif

        // Push it back the free queue.
        queue_push(fb->free_queue, buffer);
    }
}

int framebuffer_resize(framebuffer_t *fb, size_t count, bool expand) {
    size_t buf_size = 0;
    // Queue size given the requested buffer count.
    size_t queue_size = queue_calc_size(count);

    // Maximum usable memory size without queues.
    size_t max_size = fb->raw_size - queue_size * 2;
    size_t min_size = fb->frame_size + sizeof(vbuffer_t);

    // Use the frame buffer memory for big queues.
    char *queue_memory = (count > 3) ? fb->raw_base : fb->raw_static;

    // Calculate a single buffer size (including vbuffer header).
    if (!expand) {
        // No expansion: buffer size equals frame size plus header.
        buf_size = OMV_ALIGN_TO(min_size, FRAMEBUFFER_ALIGNMENT);
    } else if (fb->dynamic) {
        // Expanding a dynamic FB: divide the raw buffer size evenly.
        buf_size = OMV_ALIGN_DOWN(max_size / count, FRAMEBUFFER_ALIGNMENT);
    } else {
        // Expanding a static FB: calculate the free FB memory size.
        size_t fb_size = fb_alloc_sp() - framebuffer_pool_start(fb, count);
        max_size = IM_MIN(max_size, fb_size);
        buf_size = OMV_ALIGN_DOWN(max_size / count, FRAMEBUFFER_ALIGNMENT);
    }

    // Ensure that the buffer size is reasonable.
    if (buf_size < min_size || buf_size * count > max_size) {
        return -1;
    }

    // Initialize the frame buffer.
    fb->expanded = expand;
    fb->buf_count = count;
    fb->buf_size = buf_size - sizeof(vbuffer_t);

    // Initialize the buffer queues.
    queue_init(&fb->free_queue, count, &queue_memory[queue_size * 0]);
    queue_init(&fb->used_queue, count, &queue_memory[queue_size * 1]);

    // Flush and reset the queues.
    framebuffer_flush(fb);
    return 0;
}

bool framebuffer_writable(framebuffer_t *fb) {
    return !queue_is_empty(fb->free_queue);
}

bool framebuffer_readable(framebuffer_t *fb) {
    return !queue_is_empty(fb->used_queue);
}

vbuffer_t *framebuffer_acquire(framebuffer_t *fb, uint32_t flags) {
    queue_t *queue = (flags & FB_FLAG_USED) ? fb->used_queue : fb->free_queue;
    vbuffer_t *buffer = queue_pop(queue, (flags & FB_FLAG_PEEK));

    #ifdef __DCACHE_PRESENT
    // Discard any cached CPU writes.
    if (buffer && (flags & FB_FLAG_INVALIDATE)) {
        SCB_InvalidateDCache_by_Addr(buffer->data, fb->buf_size);
    }
    #endif

    return buffer;
}

vbuffer_t *framebuffer_release(framebuffer_t *fb, uint32_t flags) {
    vbuffer_t *buffer = NULL;

    if ((flags & FB_FLAG_CHECK_LAST) && queue_size(fb->free_queue) == 1) {
        if (fb->buf_count == 2) {
            // Double buffer: Reset but do Not release the buffer.
            vbuffer_t *buffer = queue_pop(fb->free_queue, true);
            framebuffer_reset(buffer);
            return NULL;
        } else if (fb->buf_count == 3) {
            // Triple buffer: Swap the old buffer with the latest.
            vbuffer_t *buffer = queue_swap(fb->used_queue, fb->free_queue);
            framebuffer_reset(buffer);
            return NULL;
        }
    }

    if ((buffer = framebuffer_acquire(fb, flags))) {
        if (flags & FB_FLAG_USED) {
            // Invalidate the frame buffer.
            fb->pixfmt = PIXFORMAT_INVALID;
            // Move the buffer back to the free queue.
            framebuffer_reset(buffer);
            queue_push(fb->free_queue, buffer);
        } else {
            // Move the buffer back to the used queue.
            queue_push(fb->used_queue, buffer);
        }
    }

    return buffer;
}

void framebuffer_update_jpeg_buffer(image_t *src) {
    static int overflow_count = 0;
    const size_t max_size = (&_jpeg_memory_end - &_jpeg_memory_start) - sizeof(jpegbuffer_t);

    // Check if JPEG buffer is disabled, image is NULL or format is not set.
    if (!jpegbuffer.enabled || !src->data || src->pixfmt == PIXFORMAT_INVALID) {
        return;
    }

    // Lock the JPEG buffer.
    if (!mutex_try_lock_fair(&jpegbuffer.lock, MUTEX_TID_OMV)) {
        return;
    }

    if (src->is_compressed) {
        if (max_size < src->size) {
            jpegbuffer_init_from_image(NULL);
            mp_printf(MP_PYTHON_PRINTER, "\x1b[40O\n");
        } else {
            jpegbuffer_init_from_image(src);
            memcpy(jpegbuffer.pixels, src->pixels, src->size);
        }
    } else {
        image_t dst = {
            .w = src->w,
            .h = src->h,
            .pixfmt = PIXFORMAT_JPEG,
            .size = max_size,
            .pixels = jpegbuffer.pixels
        };

        bool compress = true;
        bool overflow = false;

        #if OMV_RAW_PREVIEW_ENABLE
        if (src->is_mutable) {
            // Down-scale the frame (if necessary) and send the raw frame.
            dst.size = src->bpp;
            dst.pixfmt = src->pixfmt;
            if (src->w <= OMV_RAW_PREVIEW_WIDTH && src->h <= OMV_RAW_PREVIEW_HEIGHT) {
                if (image_size(&dst) <= max_size) {
                    memcpy(dst.pixels, src->pixels, image_size(src));
                    compress = false;
                }
            } else {
                float x_scale = OMV_RAW_PREVIEW_WIDTH / (float) src->w;
                float y_scale = OMV_RAW_PREVIEW_HEIGHT / (float) src->h;
                float scale = IM_MIN(x_scale, y_scale);
                dst.w = fast_floorf(src->w * scale);
                dst.h = fast_floorf(src->h * scale);
                if (image_size(&dst) <= max_size) {
                    imlib_draw_image(&dst, src, 0, 0, scale, scale, NULL, -1, 255, NULL, NULL,
                                     IMAGE_HINT_BILINEAR | IMAGE_HINT_BLACK_BACKGROUND,
                                     NULL, NULL, NULL, NULL);
                    compress = false;
                }
            }
        }
        #endif

        if (compress) {
            // For all other formats, send a compressed frame.
            overflow = jpeg_compress(src, &dst, jpegbuffer.quality, false, JPEG_SUBSAMPLING_AUTO);
        }

        if (overflow) {
            // JPEG buffer overflowed, reduce JPEG quality for the next frame
            // and skip the current frame. The IDE doesn't receive this frame.
            if (jpegbuffer.quality > 1) {
                // Keep this quality for the next n frames
                overflow_count = 60;
                jpegbuffer.quality = IM_MAX(1, (jpegbuffer.quality / 2));
            }

            jpegbuffer_init_from_image(NULL);
        } else {
            if (overflow_count) {
                overflow_count--;
            }

            // Dynamically adjust our quality if the image is huge.
            bool big_frame = image_size(src) > OMV_JPEG_QUALITY_THRESHOLD;
            int quality_max = big_frame ? OMV_JPEG_QUALITY_LOW : OMV_JPEG_QUALITY_HIGH;

            // No buffer overflow, increase quality up to max quality based on frame size...
            if ((!overflow_count) && (jpegbuffer.quality < quality_max)) {
                jpegbuffer.quality++;
            }

            jpegbuffer_init_from_image(&dst);
        }
    }
        
    // Unlock the JPEG buffer.
    mutex_unlock(&jpegbuffer.lock, MUTEX_TID_OMV);
}

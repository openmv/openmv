/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2022-2024 OpenMV, LLC.
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
 * OpenMV Protocol Channels.
 */

#include "omv_common.h"
#include "omv_protocol.h"
#include "omv_boardconfig.h"
#include "framebuffer.h"


static int stream_channel_init(const omv_protocol_channel_t *channel) {
    return channel->unlock(channel);
}

static bool stream_channel_poll(const omv_protocol_channel_t *channel) {
    return channel->size(channel) != 0;
}

static int stream_channel_lock(const omv_protocol_channel_t *channel) {
    framebuffer_t *fb = framebuffer_get(FB_STREAM_ID);
    size_t size = channel->size(channel);
    // Attempt locking only if the stream is ready.
    return size && mutex_try_lock(&fb->lock, MUTEX_TID_IDE) ? 0 : -1;
}

static int stream_channel_unlock(const omv_protocol_channel_t *channel) {
    framebuffer_t *fb = framebuffer_get(FB_STREAM_ID);
    if (mutex_unlock(&fb->lock, MUTEX_TID_IDE)) {
        // Reset header even if we don't hold the lock
        memset(fb->raw_base, 0, sizeof(framebuffer_header_t));
    }
    return 0;
}

static size_t stream_channel_size(const omv_protocol_channel_t *channel) {
    framebuffer_t *fb = framebuffer_get(FB_STREAM_ID);
    framebuffer_header_t *hdr = (framebuffer_header_t *) fb->raw_base;
    size_t size = hdr->is_compressed ? hdr->size : (hdr->width * hdr->height * hdr->bpp);
    // Return header size + stream data size
    return !size ? 0 : (size + sizeof(framebuffer_header_t));
}

static size_t stream_channel_shape(const omv_protocol_channel_t *channel, size_t shape[4]) {
    framebuffer_t *fb = framebuffer_get(FB_STREAM_ID);

    if (fb->is_compressed > 0) {
        // Compressed: shape is (size,)
        shape[0] = fb->size;
        return 1;
    } else {
        // Uncompressed: shape is (w, h, bpp)
        shape[0] = fb->w;
        shape[1] = fb->h;
        shape[2] = fb->bpp;
        return 3;
    }
}

static int stream_channel_ioctl(const omv_protocol_channel_t *channel, uint32_t cmd, size_t len, void *arg) {
    union {
        uint8_t bytes[16];
        uint32_t args[4];
    } u;

    memcpy(u.bytes, arg, len);
    framebuffer_t *fb = framebuffer_get(FB_STREAM_ID);

    switch (cmd) {
        case OMV_CHANNEL_IOCTL_STREAM_CTRL:
            fb->enabled = u.args[0];
            // Reset stream buffer state
            mutex_init0(&fb->lock);
            memset(fb->raw_base, 0, sizeof(framebuffer_header_t));
            return 0;
        case OMV_CHANNEL_IOCTL_STREAM_RAW_CFG:
            fb->raw_w = u.args[0];
            fb->raw_h = u.args[1];
            return 0;
        case OMV_CHANNEL_IOCTL_STREAM_RAW_CTRL:
            fb->raw_enabled = u.args[0];
            return 0;
        default:
            return -1;
    }
}

static const void *stream_channel_readp(const omv_protocol_channel_t *channel, uint32_t offset, size_t size) {
    framebuffer_t *fb = framebuffer_get(FB_STREAM_ID);
    size_t available = channel->size(channel);
    if (offset + size > available) {
        return NULL;
    }
    // Return pointer to framebuffer data starting from header
    return (uint8_t *) fb->raw_base + offset;
}

const omv_protocol_channel_t omv_stream_channel = {
    .priv = NULL,
    .id = OMV_PROTOCOL_CHANNEL_ID_STREAM,
    .name = "stream",
    .flags = OMV_PROTOCOL_CHANNEL_FLAG_READ |
             OMV_PROTOCOL_CHANNEL_FLAG_LOCK |
             OMV_PROTOCOL_CHANNEL_FLAG_STREAM,
    .init = stream_channel_init,
    .poll = stream_channel_poll,
    .lock = stream_channel_lock,
    .unlock = stream_channel_unlock,
    .size = stream_channel_size,
    .shape = stream_channel_shape,
    .readp = stream_channel_readp,
    .ioctl = stream_channel_ioctl
};

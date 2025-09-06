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
#include "omv_profiler.h"
#include "omv_protocol.h"
#include "omv_boardconfig.h"

#if OMV_PROFILER_ENABLE
static size_t profile_channel_size(const omv_protocol_channel_t *channel) {
    return omv_profiler_get_size();
}

static size_t profile_channel_shape(const omv_protocol_channel_t *channel, size_t shape[4]) {
    shape[0] = omv_profiler_get_size() / sizeof(omv_profiler_data_t);
    shape[1] = sizeof(omv_profiler_data_t);
    return 2;
}

static int profile_channel_lock(const omv_protocol_channel_t *channel) {
    size_t size = channel->size(channel);
    return size && mutex_try_lock(omv_profiler_lock(), MUTEX_TID_IDE) ? 0 : -1;
}

static int profile_channel_unlock(const omv_protocol_channel_t *channel) {
    mutex_unlock(omv_profiler_lock(), MUTEX_TID_IDE);
    return 0;
}

static const void *profile_channel_readp(const omv_protocol_channel_t *channel, uint32_t offset, size_t size) {
    const uint8_t *data = omv_profiler_get_data();
    if (offset + size > channel->size(channel)) {
        return NULL;
    }
    return data + offset;
}

const omv_protocol_channel_t omv_profile_channel = {
    .id = OMV_PROTOCOL_CHANNEL_ID_PROFILE,
    .name = "profile",
    .flags = OMV_PROTOCOL_CHANNEL_FLAG_READ,
    .priv = NULL,
    .init = NULL,
    .deinit = NULL,
    .lock = profile_channel_lock,
    .unlock = profile_channel_unlock,
    .size = profile_channel_size,
    .shape = profile_channel_shape,
    .read = NULL,
    .write = NULL,
    .readp = profile_channel_readp,
    .flush = NULL,
    .ioctl = NULL
};
#endif // OMV_PROFILER_ENABLE

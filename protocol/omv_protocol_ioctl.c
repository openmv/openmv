/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
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
 * OpenMV Protocol Ioctl functions.
 */
#include "omv_protocol_channel.h"

static const struct { uint8_t ch; uint32_t cmd; size_t size; } ioctl_sizes[] = {
    {OMV_PROTOCOL_CHANNEL_ID_STDIN, OMV_CHANNEL_IOCTL_STDIN_STOP, 0},
    {OMV_PROTOCOL_CHANNEL_ID_STDIN, OMV_CHANNEL_IOCTL_STDIN_EXEC, 0},
    {OMV_PROTOCOL_CHANNEL_ID_STREAM, OMV_CHANNEL_IOCTL_STREAM_CTRL, 4},
    {OMV_PROTOCOL_CHANNEL_ID_STREAM, OMV_CHANNEL_IOCTL_STREAM_RAW_CTRL, 4},
    {OMV_PROTOCOL_CHANNEL_ID_STREAM, OMV_CHANNEL_IOCTL_STREAM_RAW_CFG, 8},
    {OMV_PROTOCOL_CHANNEL_ID_PROFILE, OMV_CHANNEL_IOCTL_PROFILE_MODE, 4},
    {OMV_PROTOCOL_CHANNEL_ID_PROFILE, OMV_CHANNEL_IOCTL_PROFILE_SET_EVENT, 8},
    {OMV_PROTOCOL_CHANNEL_ID_PROFILE, OMV_CHANNEL_IOCTL_PROFILE_RESET, 0},
    {OMV_PROTOCOL_CHANNEL_ID_PROFILE, OMV_CHANNEL_IOCTL_PROFILE_STATS, 0},
};

bool omv_protocol_ioctl_check(uint8_t channel_id, uint32_t cmd, size_t len) {
    for (int i = 0; i < sizeof(ioctl_sizes)/sizeof(ioctl_sizes[0]); i++) {
        if (ioctl_sizes[i].ch == channel_id && ioctl_sizes[i].cmd == cmd) {
            return len == ioctl_sizes[i].size;
        }
    }
    return false; // Unknown ioctl on static channel
}

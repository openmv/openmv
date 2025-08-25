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

#include "py/mphal.h"
#include "py/nlr.h"
#include "py/gc.h"
#include "py/ringbuf.h"
#include "py/runtime.h"
#include "shared/runtime/interrupt_char.h"

#include "omv_common.h"
#include "omv_protocol.h"
#include "omv_boardconfig.h"

#ifndef OMV_PROTOCOL_STDIO_BUFFER_SIZE
#define OMV_PROTOCOL_STDIO_BUFFER_SIZE  (512)
#endif

// stdio ioctl commands
typedef enum {
    OMV_STDIO_IOCTL_INT_SCRIPT  = 0x01,
    OMV_STDIO_IOCTL_RUN_SCRIPT  = 0x02,
    OMV_STDIO_IOCTL_CLR_SCRIPT  = 0x03,
    OMV_STDIO_IOCTL_GET_SCRIPT  = 0x04,
} omv_stdio_ioctl_t;

typedef struct {
    vstr_t vstrbuf;
    bool script_running;
    ringbuf_t ringbuf;
    uint8_t rawbuf[OMV_PROTOCOL_STDIO_BUFFER_SIZE];
} stdio_channel_context_t;

static stdio_channel_context_t stdio_channel_ctx;

static int stdio_channel_init(const omv_protocol_channel_t *channel) {
    stdio_channel_context_t *ctx = channel->priv;

    if (channel->id == OMV_PROTOCOL_CHANNEL_ID_STDIN) {
        // Initialize stdin stuff
        ctx->script_running = false;
        vstr_init(&ctx->vstrbuf, 2048);
    } else if (channel->id == OMV_PROTOCOL_CHANNEL_ID_STDOUT) {
        // Initialize stdout stuff
        ctx->ringbuf = (ringbuf_t) { ctx->rawbuf, sizeof(ctx->rawbuf), 0, 0 };
    }
    return 0;
}

void stdio_channel_pyexec_hook(bool running) {
    stdio_channel_ctx.script_running = running;
}

static bool stdio_channel_poll(const omv_protocol_channel_t *channel) {
    stdio_channel_context_t *ctx = channel->priv;
    
    if (channel->id == OMV_PROTOCOL_CHANNEL_ID_STDIN) {
        return ctx->script_running;
    } else if (channel->id == OMV_PROTOCOL_CHANNEL_ID_STDOUT) {
        return ringbuf_avail(&ctx->ringbuf);
    }
    return false;
}

static size_t stdio_channel_size(const omv_protocol_channel_t *channel) {
    stdio_channel_context_t *ctx = channel->priv;
    return ringbuf_avail(&ctx->ringbuf);
}

static int stdio_channel_read(const omv_protocol_channel_t *channel,
                            uint32_t offset, size_t size, void *data) {
    stdio_channel_context_t *ctx = channel->priv;

    size = OMV_MIN(size, ringbuf_avail(&ctx->ringbuf));
    return !ringbuf_get_bytes(&ctx->ringbuf, data, size) ? size : -1;
}

static int stdio_channel_write(const omv_protocol_channel_t *channel,
                               uint32_t offset, size_t size, const void *data) {
    stdio_channel_context_t *ctx = channel->priv;
    
    if (offset == 0) {
        vstr_reset(&ctx->vstrbuf);
    }

    nlr_buf_t nlr;
    if (!gc_is_locked() && nlr_push(&nlr) == 0) {
        char *buf = vstr_add_len(&ctx->vstrbuf, size);
        memcpy(buf, data, size);
        nlr_pop();
        return size;
    }

    return -1;
}

static int stdio_channel_ioctl(const omv_protocol_channel_t *channel, uint32_t cmd, void *arg) {
    stdio_channel_context_t *ctx = channel->priv;
    
    switch (cmd) {
        case OMV_STDIO_IOCTL_INT_SCRIPT:
            if (mp_interrupt_char != -1) {
                mp_sched_vm_abort();
                mp_sched_keyboard_interrupt();
            }
            break;
        case OMV_STDIO_IOCTL_RUN_SCRIPT:
            if (mp_interrupt_char != -1) {
                mp_sched_vm_abort();
                mp_sched_keyboard_interrupt();
            }
            break;
        case OMV_STDIO_IOCTL_CLR_SCRIPT:
            vstr_reset(&ctx->vstrbuf);
            break;
        case OMV_STDIO_IOCTL_GET_SCRIPT:
            if (!vstr_len(&ctx->vstrbuf)) {
                *(vstr_t**)arg = NULL;
            } else {
                *(vstr_t**)arg = &ctx->vstrbuf;
            }
            break;
        default:
            return -1;
    }

    return 0;
}

// Wrap MicroPython stdio functions to intercept REPL.
extern uintptr_t __real_mp_hal_stdio_poll(uintptr_t poll_flags);
uintptr_t __wrap_mp_hal_stdio_poll(uintptr_t poll_flags) {
    if (!omv_protocol_active()) {
        return __real_mp_hal_stdio_poll(poll_flags);
    }
    return 0;
}

extern mp_uint_t __real_mp_hal_stdout_tx_strn(const char *str, mp_uint_t len);
mp_uint_t __wrap_mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    if (!omv_protocol_active()) {
        return __real_mp_hal_stdout_tx_strn(str, len);
    }

    const omv_protocol_channel_t *channel = omv_protocol_get_channel(OMV_PROTOCOL_CHANNEL_ID_STDOUT);
    stdio_channel_context_t *ctx = channel->priv;

    // On overflow, reset the ring buffer, if this string fits
    // entirely in the buffer, to recover from broken strings.
    for (int i = 0; i < len; i++) {
        if (ringbuf_put(&ctx->ringbuf, str[i]) == -1 && len <= ctx->ringbuf.size) {
            ctx->ringbuf.iget = 0;
            ctx->ringbuf.iput = 0;
            ringbuf_put(&ctx->ringbuf, str[i]);
        }
    }
    return len;
}

const omv_protocol_channel_t omv_stdin_channel = {
    .id = OMV_PROTOCOL_CHANNEL_ID_STDIN,
    .name = "stdin",
    .flags = OMV_PROTOCOL_CHANNEL_FLAG_WRITE,
    .priv = &stdio_channel_ctx,
    .init = stdio_channel_init,
    .deinit = NULL,
    .poll = stdio_channel_poll,
    .lock = NULL,
    .unlock = NULL,
    .size = NULL,
    .shape = NULL,
    .read = NULL,
    .write = stdio_channel_write,
    .readp = NULL,
    .flush = NULL,
    .ioctl = stdio_channel_ioctl
};

const omv_protocol_channel_t omv_stdout_channel = {
    .id = OMV_PROTOCOL_CHANNEL_ID_STDOUT,
    .name = "stdout",
    .flags = OMV_PROTOCOL_CHANNEL_FLAG_READ,
    .priv = &stdio_channel_ctx,
    .init = stdio_channel_init,
    .deinit = NULL,
    .poll = stdio_channel_poll,
    .lock = NULL,
    .unlock = NULL,
    .size = stdio_channel_size,
    .shape = NULL,
    .read = stdio_channel_read,
    .write = NULL,
    .readp = NULL,
    .flush = NULL,
    .ioctl = NULL
};

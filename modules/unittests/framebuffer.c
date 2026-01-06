/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025-2026 OpenMV, LLC.
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
 * Frame buffer allocation and framebuffer unit tests.
 */
#include "imlib_config.h"

#if MICROPY_PY_UNITTEST

#include <string.h>
#include "py/runtime.h"
#include "py/obj.h"

#include "imlib.h"
#include "fb_alloc.h"
#include "framebuffer.h"

// ============================================================================
// Frame Buffer Allocation Tests (fb_alloc.h)
// ============================================================================

// Test fb_alloc basic allocation and free
static mp_obj_t test_fb_alloc_basic(void) {
    uint32_t avail_before = fb_avail();

    // Mark the current position
    fb_alloc_mark();

    // Allocate some memory
    void *ptr1 = fb_alloc(256, FB_ALLOC_NO_HINT);
    if (ptr1 == NULL) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Available memory should have decreased
    uint32_t avail_after_alloc = fb_avail();
    if (avail_after_alloc >= avail_before) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Write to the allocated memory to verify it's usable
    memset(ptr1, 0xAA, 256);

    // Free back to mark
    fb_alloc_free_till_mark();

    // Available memory should be restored
    uint32_t avail_after_free = fb_avail();
    if (avail_after_free != avail_before) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fb_alloc_basic_obj, test_fb_alloc_basic);

// Test fb_alloc0 (zero-initialized allocation)
static mp_obj_t test_fb_alloc0(void) {
    fb_alloc_mark();

    // Allocate zero-initialized memory
    uint8_t *ptr = (uint8_t *) fb_alloc0(128, FB_ALLOC_NO_HINT);
    if (ptr == NULL) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Verify memory is zeroed
    for (int i = 0; i < 128; i++) {
        if (ptr[i] != 0) {
            fb_alloc_free_till_mark();
            return mp_const_false;
        }
    }

    fb_alloc_free_till_mark();
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fb_alloc0_obj, test_fb_alloc0);

// Test multiple allocations and stack behavior
static mp_obj_t test_fb_alloc_stack(void) {
    fb_alloc_mark();

    // Allocate multiple blocks
    void *ptr1 = fb_alloc(64, FB_ALLOC_NO_HINT);
    void *ptr2 = fb_alloc(128, FB_ALLOC_NO_HINT);
    void *ptr3 = fb_alloc(256, FB_ALLOC_NO_HINT);

    if (ptr1 == NULL || ptr2 == NULL || ptr3 == NULL) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Pointers should be different and in stack order (decreasing addresses)
    if (ptr1 == ptr2 || ptr2 == ptr3 || ptr1 == ptr3) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Stack grows downward, so ptr3 < ptr2 < ptr1
    if (!((uintptr_t) ptr3 < (uintptr_t) ptr2 && (uintptr_t) ptr2 < (uintptr_t) ptr1)) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Write to all blocks
    memset(ptr1, 0x11, 64);
    memset(ptr2, 0x22, 128);
    memset(ptr3, 0x33, 256);

    // Free individually (LIFO order)
    fb_free();  // Frees ptr3
    fb_free();  // Frees ptr2
    fb_free();  // Frees ptr1

    fb_alloc_free_till_mark();
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fb_alloc_stack_obj, test_fb_alloc_stack);

// Test fb_alloc with size 0 (should return NULL)
static mp_obj_t test_fb_alloc_zero_size(void) {
    fb_alloc_mark();

    // Allocating 0 bytes should return NULL
    void *ptr = fb_alloc(0, FB_ALLOC_NO_HINT);
    if (ptr != NULL) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    fb_alloc_free_till_mark();
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fb_alloc_zero_size_obj, test_fb_alloc_zero_size);

// Test fb_avail reports available memory
static mp_obj_t test_fb_avail(void) {
    uint32_t avail = fb_avail();

    // Should have some available memory
    if (avail == 0) {
        return mp_const_false;
    }

    fb_alloc_mark();

    // Allocate half the available memory (or 1KB, whichever is smaller)
    uint32_t alloc_size = (avail > 2048) ? 1024 : avail / 2;
    void *ptr = fb_alloc(alloc_size, FB_ALLOC_NO_HINT);
    if (ptr == NULL) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    uint32_t avail_after = fb_avail();

    // Available should have decreased by at least alloc_size
    if (avail_after >= avail) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    fb_alloc_free_till_mark();
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fb_avail_obj, test_fb_avail);

// Test nested marks
static mp_obj_t test_fb_alloc_nested_marks(void) {
    uint32_t avail_start = fb_avail();

    // First mark
    fb_alloc_mark();
    void *ptr1 = fb_alloc(64, FB_ALLOC_NO_HINT);

    // Second (nested) mark
    fb_alloc_mark();
    void *ptr2 = fb_alloc(128, FB_ALLOC_NO_HINT);

    if (ptr1 == NULL || ptr2 == NULL) {
        fb_alloc_free_till_mark();
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Free to inner mark (should free ptr2)
    fb_alloc_free_till_mark();

    // Should still have ptr1's allocation
    uint32_t avail_after_inner = fb_avail();
    if (avail_after_inner >= avail_start) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Free to outer mark (should free ptr1)
    fb_alloc_free_till_mark();

    // Should be back to start
    uint32_t avail_end = fb_avail();
    if (avail_end != avail_start) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fb_alloc_nested_marks_obj, test_fb_alloc_nested_marks);

// ============================================================================
// Framebuffer Tests (framebuffer.h)
// ============================================================================

// Test framebuffer_get returns valid framebuffers
static mp_obj_t test_framebuffer_get(void) {
    // Get main framebuffer
    framebuffer_t *fb_main = framebuffer_get(FB_MAINFB_ID);
    if (fb_main == NULL) {
        return mp_const_false;
    }

    // Get stream framebuffer
    framebuffer_t *fb_stream = framebuffer_get(FB_STREAM_ID);
    if (fb_stream == NULL) {
        return mp_const_false;
    }

    // They should be different
    if (fb_main == fb_stream) {
        return mp_const_false;
    }

    // Invalid ID should return NULL
    framebuffer_t *fb_invalid = framebuffer_get(FB_MAX_ID);
    if (fb_invalid != NULL) {
        return mp_const_false;
    }

    framebuffer_t *fb_invalid2 = framebuffer_get(100);
    if (fb_invalid2 != NULL) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_framebuffer_get_obj, test_framebuffer_get);

// Test framebuffer_from_image and framebuffer_to_image
static mp_obj_t test_framebuffer_image(void) {
    framebuffer_t *fb = framebuffer_get(FB_MAINFB_ID);
    if (fb == NULL) {
        return mp_const_false;
    }

    // Save original state
    int32_t orig_w = fb->w;
    int32_t orig_h = fb->h;
    uint32_t orig_size = fb->size;
    pixformat_t orig_pixfmt = fb->pixfmt;

    // Create a test image structure
    image_t test_img = {
        .w = 320,
        .h = 240,
        .size = 320 * 240 * 2,
        .pixfmt = PIXFORMAT_RGB565,
        .data = NULL
    };

    // Set framebuffer from image
    framebuffer_from_image(fb, &test_img);

    // Verify framebuffer was updated
    if (fb->w != 320 || fb->h != 240 || fb->pixfmt != PIXFORMAT_RGB565) {
        // Restore original state
        fb->w = orig_w;
        fb->h = orig_h;
        fb->size = orig_size;
        fb->pixfmt = orig_pixfmt;
        return mp_const_false;
    }

    // Test with NULL image (should reset)
    framebuffer_from_image(fb, NULL);
    if (fb->w != 0 || fb->h != 0 || fb->pixfmt != PIXFORMAT_INVALID) {
        fb->w = orig_w;
        fb->h = orig_h;
        fb->size = orig_size;
        fb->pixfmt = orig_pixfmt;
        return mp_const_false;
    }

    // Restore original state
    fb->w = orig_w;
    fb->h = orig_h;
    fb->size = orig_size;
    fb->pixfmt = orig_pixfmt;

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_framebuffer_image_obj, test_framebuffer_image);

// Test framebuffer_get_buffer_size
static mp_obj_t test_framebuffer_buffer_size(void) {
    framebuffer_t *fb = framebuffer_get(FB_MAINFB_ID);
    if (fb == NULL) {
        return mp_const_false;
    }

    // Get buffer size - should match fb->buf_size
    size_t size = framebuffer_get_buffer_size(fb);
    if (size != fb->buf_size) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_framebuffer_buffer_size_obj, test_framebuffer_buffer_size);

// Test framebuffer resize with single buffer
static mp_obj_t test_framebuffer_resize(void) {
    framebuffer_t *fb = framebuffer_get(FB_MAINFB_ID);
    if (fb == NULL) {
        return mp_const_false;
    }

    // Save original state
    size_t orig_buf_count = fb->buf_count;
    size_t orig_buf_size = fb->buf_size;

    // Resize to 1 buffer with small frame size
    int result = framebuffer_resize(fb, 1, 1024, false);
    if (result != 0) {
        return mp_const_false;
    }

    // Verify resize worked
    if (fb->buf_count != 1) {
        // Try to restore
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Test writable/readable status
    // After resize and flush, free queue should have buffer, used should be empty
    // Should be writable (free queue has buffer) but not readable (used queue empty)
    if (!framebuffer_writable(fb)) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Acquire a buffer from free queue
    vbuffer_t *buffer = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK);
    if (buffer == NULL) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Restore original configuration
    framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_framebuffer_resize_obj, test_framebuffer_resize);

// Test framebuffer_flush clears queues
static mp_obj_t test_framebuffer_flush(void) {
    framebuffer_t *fb = framebuffer_get(FB_MAINFB_ID);
    if (fb == NULL) {
        return mp_const_false;
    }

    // Resize to ensure we have buffers
    size_t orig_buf_count = fb->buf_count;
    size_t orig_buf_size = fb->buf_size;

    int result = framebuffer_resize(fb, 2, 1024, false);
    if (result != 0) {
        return mp_const_false;
    }

    // Flush the framebuffer
    framebuffer_flush(fb);

    // After flush, pixfmt should be invalid
    if (fb->pixfmt != PIXFORMAT_INVALID) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Free queue should have all buffers
    if (!framebuffer_writable(fb)) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Used queue should be empty
    if (framebuffer_readable(fb)) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Restore
    framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_framebuffer_flush_obj, test_framebuffer_flush);

// Test framebuffer acquire and release cycle
static mp_obj_t test_framebuffer_acquire_release(void) {
    framebuffer_t *fb = framebuffer_get(FB_MAINFB_ID);
    if (fb == NULL) {
        return mp_const_false;
    }

    // Resize to 2 buffers
    size_t orig_buf_count = fb->buf_count;
    size_t orig_buf_size = fb->buf_size;

    int result = framebuffer_resize(fb, 2, 1024, false);
    if (result != 0) {
        return mp_const_false;
    }

    // Initially: free queue has 2 buffers, used queue is empty
    if (!framebuffer_writable(fb) || framebuffer_readable(fb)) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Acquire from free queue (peek - keeps in queue)
    vbuffer_t *buf1 = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK);
    if (buf1 == NULL) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Release to used queue (moves from free to used)
    vbuffer_t *released = framebuffer_release(fb, FB_FLAG_FREE);
    if (released == NULL) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Now used queue should have 1 buffer
    if (!framebuffer_readable(fb)) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Restore
    framebuffer_flush(fb);
    framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_framebuffer_acquire_release_obj, test_framebuffer_acquire_release);

// Module definition
static const mp_rom_map_elem_t unittest_fb_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_unittest_fb) },
    // Frame buffer allocation tests
    { MP_ROM_QSTR(MP_QSTR_test_fb_alloc_basic), MP_ROM_PTR(&test_fb_alloc_basic_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fb_alloc0), MP_ROM_PTR(&test_fb_alloc0_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fb_alloc_stack), MP_ROM_PTR(&test_fb_alloc_stack_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fb_alloc_zero_size), MP_ROM_PTR(&test_fb_alloc_zero_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fb_avail), MP_ROM_PTR(&test_fb_avail_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fb_alloc_nested_marks), MP_ROM_PTR(&test_fb_alloc_nested_marks_obj) },
    // Framebuffer tests
    { MP_ROM_QSTR(MP_QSTR_test_framebuffer_get), MP_ROM_PTR(&test_framebuffer_get_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_framebuffer_image), MP_ROM_PTR(&test_framebuffer_image_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_framebuffer_buffer_size), MP_ROM_PTR(&test_framebuffer_buffer_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_framebuffer_resize), MP_ROM_PTR(&test_framebuffer_resize_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_framebuffer_flush), MP_ROM_PTR(&test_framebuffer_flush_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_framebuffer_acquire_release), MP_ROM_PTR(&test_framebuffer_acquire_release_obj) },
};

static MP_DEFINE_CONST_DICT(unittest_fb_module_globals, unittest_fb_module_globals_table);

const mp_obj_module_t unittest_fb_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &unittest_fb_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_unittest_fb, unittest_fb_module);

#endif // MICROPY_PY_UNITTEST

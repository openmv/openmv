/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Unix Port: GC-based framebuffer allocation
 *
 */
#include <string.h>
#include "py/runtime.h"
#include "py/gc.h"
#include "omv_common.h"
#include "fb_alloc.h"

// Track allocations for proper cleanup
typedef struct fb_alloc_block {
    void *ptr;
    size_t size;
    bool is_mark;       // Mark point in allocation stack
    bool is_permanent;  // Permanent mark - fb_alloc_free_till_mark won't free past this
    struct fb_alloc_block *next;
} fb_alloc_block_t;

static fb_alloc_block_t *fb_alloc_list = NULL;
static fb_alloc_block_t *fb_alloc_mark_head = NULL;  // Current mark point
static size_t fb_alloc_total = 0;

__weak NORETURN void fb_alloc_fail(void)
{
    nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError, MP_ERROR_TEXT(
        "Out of Frame Buffer Memory!"
        " Try increasing heap size with MICROPYTHON_HEAP_SIZE environment variable.")));
}

static void fb_alloc_gc_info(size_t *free, size_t *total) {
    gc_info_t info;
    gc_info(&info);
    *free = info.free;
    *total = info.total;
}

void fb_alloc_init0(void)
{
    // For GC-based allocation, just clear the list
    fb_free_all();
}

uint32_t fb_avail(void)
{
    size_t gc_free_bytes, gc_total_bytes;
    fb_alloc_gc_info(&gc_free_bytes, &gc_total_bytes);
    // Leave some headroom for Python operations
    return (uint32_t)(gc_free_bytes > 65536 ? gc_free_bytes - 65536 : gc_free_bytes / 2);
}

void fb_alloc_mark(void)
{
    // Allocate a mark block
    fb_alloc_block_t *mark = gc_alloc(sizeof(fb_alloc_block_t), false);
    if (!mark) {
        fb_alloc_fail();
    }

    mark->ptr = NULL;
    mark->size = 0;
    mark->is_mark = true;
    mark->is_permanent = false;
    mark->next = fb_alloc_list;
    fb_alloc_list = mark;
    fb_alloc_mark_head = mark;
}

static void int_fb_alloc_free_till_mark(bool free_permanent)
{
    // Free all allocations until we hit a mark
    while (fb_alloc_list != NULL) {
        fb_alloc_block_t *block = fb_alloc_list;

        // Check if this is a mark
        if (block->is_mark) {
            // Stop if it's permanent and we're not freeing permanent
            if (block->is_permanent && !free_permanent) {
                return;
            }

            // Found mark, free the mark itself and stop
            fb_alloc_list = block->next;
            gc_free(block);
            fb_alloc_mark_head = NULL;  // No mark now
            // Find previous mark if any
            fb_alloc_block_t *scan = fb_alloc_list;
            while (scan) {
                if (scan->is_mark) {
                    fb_alloc_mark_head = scan;
                    break;
                }
                scan = scan->next;
            }
            break;
        }

        // Free the data and the block
        fb_alloc_list = block->next;
        if (block->ptr) {
            fb_alloc_total -= block->size;
            gc_free(block->ptr);
        }
        gc_free(block);
    }
}

void fb_alloc_free_till_mark(void)
{
    int_fb_alloc_free_till_mark(false);
}

void fb_alloc_mark_permanent(void)
{
    // Mark the most recent mark as permanent
    if (fb_alloc_mark_head != NULL) {
        fb_alloc_mark_head->is_permanent = true;
    }
}

void fb_alloc_free_till_mark_past_mark_permanent(void)
{
    int_fb_alloc_free_till_mark(true);
}

void *fb_alloc(uint32_t size, int hints)
{
    (void)hints;

    if (size == 0) {
        return NULL;
    }

    // Align size to 4-byte boundary
    size = ((size + sizeof(uint32_t) - 1) / sizeof(uint32_t)) * sizeof(uint32_t);

    // Check if GC has enough free memory
    size_t gc_free_bytes, gc_total_bytes;
    fb_alloc_gc_info(&gc_free_bytes, &gc_total_bytes);

    if (size > gc_free_bytes) {
        // Try collecting garbage first
        gc_collect();
        fb_alloc_gc_info(&gc_free_bytes, &gc_total_bytes);

        if (size > gc_free_bytes) {
            fb_alloc_fail();
        }
    }

    // Allocate from GC
    void *ptr = gc_alloc(size, false);
    if (!ptr) {
        fb_alloc_fail();
    }

    // Track allocation
    fb_alloc_block_t *block = gc_alloc(sizeof(fb_alloc_block_t), false);
    if (!block) {
        gc_free(ptr);
        fb_alloc_fail();
    }

    block->ptr = ptr;
    block->size = size;
    block->is_mark = false;
    block->is_permanent = false;
    block->next = fb_alloc_list;
    fb_alloc_list = block;
    fb_alloc_total += size;

    return ptr;
}

void *fb_alloc0(uint32_t size, int hints)
{
    void *ptr = fb_alloc(size, hints);
    if (ptr && size > 0) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void *fb_alloc_all(uint32_t *size, int hints)
{
    (void)hints;

    // Get available memory
    size_t gc_free_bytes, gc_total_bytes;
    fb_alloc_gc_info(&gc_free_bytes, &gc_total_bytes);

    // Leave some headroom for Python operations and tracking
    size_t available = gc_free_bytes > 65536 ? gc_free_bytes - 65536 : gc_free_bytes / 2;
    available = (available / sizeof(uint32_t)) * sizeof(uint32_t);  // Round down

    if (available < sizeof(uint32_t)) {
        *size = 0;
        return NULL;
    }

    *size = (uint32_t)available;
    return fb_alloc(*size, hints);
}

void *fb_alloc0_all(uint32_t *size, int hints)
{
    void *ptr = fb_alloc_all(size, hints);
    if (ptr && *size > 0) {
        memset(ptr, 0, *size);
    }
    return ptr;
}

void fb_free(void)
{
    // Free the most recent allocation
    if (fb_alloc_list != NULL && !fb_alloc_list->is_mark) {
        fb_alloc_block_t *block = fb_alloc_list;
        fb_alloc_list = block->next;

        if (block->ptr) {
            fb_alloc_total -= block->size;
            gc_free(block->ptr);
        }
        gc_free(block);
    }
}

void fb_free_all(void)
{
    // Free all allocations
    while (fb_alloc_list != NULL) {
        fb_alloc_block_t *block = fb_alloc_list;
        fb_alloc_list = block->next;

        if (block->ptr) {
            fb_alloc_total -= block->size;
            gc_free(block->ptr);
        }
        gc_free(block);
    }
    fb_alloc_mark_head = NULL;
}

/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
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
 * UMA (Unified Memory Allocator) unit tests.
 */
#include "imlib_config.h"

#if MICROPY_PY_UNITTEST

#include <string.h>
#include "py/runtime.h"
#include "py/obj.h"

#include "tlsf.h"
#include "omv_common.h"
#include "umalloc.h"

// Helper: find which pool a pointer belongs to.
static uma_pool_t *find_pool_for_ptr(void *ptr) {
    uintptr_t addr = (uintptr_t) ptr;
    for (int i = 0; i < uma_pool_count(); i++) {
        uma_pool_t *pool = uma_pool_get(i);
        if (addr >= pool->base && addr < pool->end) {
            return pool;
        }
    }
    return NULL;
}

// Helper: find the first pool matching exact flags.
static uma_pool_t *find_pool_by_flags(uint32_t flags) {
    for (int i = 0; i < uma_pool_count(); i++) {
        uma_pool_t *p = uma_pool_get(i);
        if (p->flags == flags) {
            return p;
        }
    }
    return NULL;
}

// Helper: find the first pool with no flags (default pool).
static uma_pool_t *find_default_pool(void) {
    for (int i = 0; i < uma_pool_count(); i++) {
        uma_pool_t *p = uma_pool_get(i);
        if (p->flags == 0) {
            return p;
        }
    }
    return NULL;
}

// Test basic malloc/free and pool accounting.
static mp_obj_t test_uma_malloc_free(void) {
    uma_pool_t *pool = find_default_pool();
    if (!pool) {
        return mp_const_false;
    }

    size_t free_before = pool->free;

    void *ptr = uma_malloc(256, 0);
    if (!ptr) {
        return mp_const_false;
    }

    // Free should have decreased.
    if (pool->free >= free_before) {
        uma_free(ptr);
        return mp_const_false;
    }

    uma_free(ptr);

    // Free should be restored.
    if (pool->free != free_before) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_malloc_free_obj, test_uma_malloc_free);

// Test calloc zeros memory.
static mp_obj_t test_uma_calloc(void) {
    uint8_t *ptr = uma_calloc(128, 0);
    if (!ptr) {
        return mp_const_false;
    }

    for (int i = 0; i < 128; i++) {
        if (ptr[i] != 0) {
            uma_free(ptr);
            return mp_const_false;
        }
    }

    uma_free(ptr);
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_calloc_obj, test_uma_calloc);

// Test realloc grows and preserves data.
static mp_obj_t test_uma_realloc(void) {
    uint8_t *ptr = uma_malloc(64, 0);
    if (!ptr) {
        return mp_const_false;
    }

    memset(ptr, 0xAB, 64);

    ptr = uma_realloc(ptr, 256, 0);
    if (!ptr) {
        return mp_const_false;
    }

    // First 64 bytes should be preserved.
    for (int i = 0; i < 64; i++) {
        if (ptr[i] != 0xAB) {
            uma_free(ptr);
            return mp_const_false;
        }
    }

    uma_free(ptr);
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_realloc_obj, test_uma_realloc);

// Test multiple allocs and frees restore pool free to original.
static mp_obj_t test_uma_multi_alloc_free(void) {
    uma_pool_t *pool = find_default_pool();
    if (!pool) {
        return mp_const_false;
    }

    size_t free_before = pool->free;

    void *ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = uma_malloc(100 + i * 50, 0);
        if (!ptrs[i]) {
            // Free what we allocated so far.
            for (int j = 0; j < i; j++) {
                uma_free(ptrs[j]);
            }
            return mp_const_false;
        }
    }

    // Free in reverse order.
    for (int i = 9; i >= 0; i--) {
        uma_free(ptrs[i]);
    }

    if (pool->free != free_before) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_multi_alloc_free_obj, test_uma_multi_alloc_free);

// Test that persist flag survives uma_collect.
static mp_obj_t test_uma_persist_survives_collect(void) {
    uma_pool_t *pool = find_default_pool();
    if (!pool) {
        return mp_const_false;
    }

    size_t free_before = pool->free;

    // Allocate a persistent block.
    uint8_t *persist = uma_malloc(512, UMA_PERSIST);
    if (!persist) {
        return mp_const_false;
    }
    memset(persist, 0xCD, 512);

    // Allocate a scratch block.
    void *scratch = uma_malloc(256, 0);
    if (!scratch) {
        uma_free(persist);
        return mp_const_false;
    }

    size_t free_after_allocs = pool->free;

    // Collect should free scratch but not persist.
    uma_collect();

    // Free should have increased (scratch freed) but not fully restored
    // (persist still allocated).
    if (pool->free <= free_after_allocs) {
        uma_free(persist);
        return mp_const_false;
    }

    // Persistent data should still be intact.
    for (int i = 0; i < 512; i++) {
        if (persist[i] != 0xCD) {
            uma_free(persist);
            return mp_const_false;
        }
    }

    uma_free(persist);

    // Now everything is freed, pool should be restored.
    if (pool->free != free_before) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_persist_survives_collect_obj, test_uma_persist_survives_collect);

// Test that collect frees all non-persistent blocks.
static mp_obj_t test_uma_collect_frees_all(void) {
    uma_pool_t *pool = find_default_pool();
    if (!pool) {
        return mp_const_false;
    }

    size_t free_before = pool->free;

    // Allocate several scratch blocks.
    for (int i = 0; i < 5; i++) {
        uma_malloc(200, 0);
    }

    // Free should have decreased.
    if (pool->free >= free_before) {
        return mp_const_false;
    }

    // Collect should free them all.
    uma_collect();

    if (pool->free != free_before) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_collect_frees_all_obj, test_uma_collect_frees_all);

// Test collect with mixed persist and scratch blocks.
// Verifies that collect frees scratch but persistent blocks remain allocated.
static mp_obj_t test_uma_collect_mixed(void) {
    uma_pool_t *pool = find_default_pool();
    if (!pool) {
        return mp_const_false;
    }

    size_t free_before = pool->free;

    // Interleave persistent and scratch allocations.
    void *p1 = uma_malloc(128, UMA_PERSIST);
    void *s1 = uma_malloc(256, 0);
    void *p2 = uma_malloc(128, UMA_PERSIST);
    void *s2 = uma_malloc(256, 0);
    void *s3 = uma_malloc(256, 0);

    (void) s1;
    (void) s2;
    (void) s3;

    size_t free_after_allocs = pool->free;

    uma_collect();

    // Free should have increased (scratch freed) but not fully restored
    // (persist blocks still allocated).
    if (pool->free <= free_after_allocs) {
        uma_free(p1);
        uma_free(p2);
        return mp_const_false;
    }
    if (pool->free >= free_before) {
        uma_free(p1);
        uma_free(p2);
        return mp_const_false;
    }

    // Allocating over the persist blocks should fail to reclaim that space.
    // Instead, verify free restores fully after explicit free of persist blocks.
    uma_free(p1);
    uma_free(p2);

    if (pool->free != free_before) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_collect_mixed_obj, test_uma_collect_mixed);

// Test that stats reflect allocations.
static mp_obj_t test_uma_stats(void) {
    uma_stats_t s;
    uma_get_stats(&s);
    if (s.used_count != 0) {
        return mp_const_false;
    }

    void *a = uma_malloc(100, 0);
    void *b = uma_malloc(200, 0);
    void *c = uma_malloc(300, 0);

    uma_get_stats(&s);
    if (s.used_count != 3) {
        uma_free(a);
        uma_free(b);
        uma_free(c);
        return mp_const_false;
    }

    uma_free(a);
    uma_free(b);
    uma_free(c);

    uma_get_stats(&s);
    if (s.used_count != 0) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_stats_obj, test_uma_stats);

// Test that collect is reflected in stats.
static mp_obj_t test_uma_collect_stats(void) {
    uma_malloc(100, 0);
    uma_malloc(200, 0);

    uma_stats_t s;
    uma_get_stats(&s);
    if (s.used_count != 2) {
        return mp_const_false;
    }

    uma_collect();

    uma_get_stats(&s);
    if (s.used_count != 0) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_collect_stats_obj, test_uma_collect_stats);

// Test that UMA_FAST allocations go to the fast pool.
static mp_obj_t test_uma_fast_pool(void) {
    // Need at least 2 pools (slow + fast).
    if (uma_pool_count() < 2) {
        return mp_const_true;
    }

    // Find the fast pool.
    uma_pool_t *fast_pool = NULL;
    for (int i = 0; i < uma_pool_count(); i++) {
        uma_pool_t *p = uma_pool_get(i);
        if (p->flags & UMA_FAST) {
            fast_pool = p;
            break;
        }
    }
    if (!fast_pool) {
        return mp_const_false;
    }

    void *ptr = uma_malloc(64, UMA_FAST);
    if (!ptr) {
        return mp_const_false;
    }

    // Pointer should be in the fast pool.
    uma_pool_t *actual = find_pool_for_ptr(ptr);
    uma_free(ptr);

    if (actual != fast_pool) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_fast_pool_obj, test_uma_fast_pool);

// Test that UMA_FAST falls back when request exceeds fast pool.
static mp_obj_t test_uma_fast_fallback(void) {
    if (uma_pool_count() < 2) {
        return mp_const_true;
    }

    uma_pool_t *fast_pool = find_pool_by_flags(UMA_FAST);
    if (!fast_pool) {
        return mp_const_false;
    }

    // Request more than the fast pool can hold.
    void *ptr = uma_malloc(fast_pool->size + 1, UMA_FAST | UMA_MAYBE);
    if (!ptr) {
        return mp_const_false;
    }

    // Should have fallen back to a non-fast pool.
    uma_pool_t *actual = find_pool_for_ptr(ptr);
    uma_free(ptr);
    return (actual != fast_pool) ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_fast_fallback_obj, test_uma_fast_fallback);

// Test that UMA_CACHE allocations are cache-line aligned.
static mp_obj_t test_uma_cache_align(void) {
    void *ptr = uma_malloc(100, UMA_CACHE);
    if (!ptr) {
        return mp_const_false;
    }

    // Pointer should be aligned to OMV_CACHE_LINE_SIZE.
    if ((uintptr_t) ptr % OMV_CACHE_LINE_SIZE != 0) {
        uma_free(ptr);
        return mp_const_false;
    }

    uma_free(ptr);
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_cache_align_obj, test_uma_cache_align);

// Test uma_avail returns an allocatable size.
static mp_obj_t test_uma_avail(void) {
    size_t avail = uma_avail(0);
    if (avail == 0) {
        return mp_const_false;
    }

    // Should be able to allocate exactly avail bytes.
    void *ptr = uma_malloc(avail, 0);
    if (!ptr) {
        return mp_const_false;
    }

    // After allocating the largest block, avail should be much smaller.
    size_t avail_after = uma_avail(0);
    if (avail_after >= avail) {
        uma_free(ptr);
        return mp_const_false;
    }

    uma_free(ptr);

    // After free, avail should be restored.
    size_t avail_restored = uma_avail(0);
    if (avail_restored != avail) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_avail_obj, test_uma_avail);

// Test uma_avail accounts for fragmentation.
static mp_obj_t test_uma_avail_fragmented(void) {
    size_t avail_initial = uma_avail(0);

    // Use proportional sizes so the test works with any heap size.
    size_t chunk = avail_initial / 8;

    // Interleave alloc/free to fragment the heap.
    void *a = uma_malloc(chunk, 0);
    void *b = uma_malloc(chunk * 2, 0);
    void *c = uma_malloc(chunk, 0);
    void *d = uma_malloc(chunk * 2, 0);
    void *e = uma_malloc(chunk, 0);
    if (!a || !b || !c || !d || !e) {
        uma_free(a);
        uma_free(b);
        uma_free(c);
        uma_free(d);
        uma_free(e);
        return mp_const_false;
    }

    // Free alternating blocks to create holes.
    uma_free(b);
    uma_free(d);

    // avail should reflect the largest contiguous block, not total free.
    size_t avail_frag = uma_avail(0);

    // The largest block should be less than initial (a, c, e still allocated).
    if (avail_frag >= avail_initial) {
        uma_free(a);
        uma_free(c);
        uma_free(e);
        return mp_const_false;
    }

    // Should be able to allocate exactly avail_frag bytes.
    void *f = uma_malloc(avail_frag, 0);
    if (!f) {
        uma_free(a);
        uma_free(c);
        uma_free(e);
        return mp_const_false;
    }

    uma_free(f);
    uma_free(a);
    uma_free(c);
    uma_free(e);
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_avail_fragmented_obj, test_uma_avail_fragmented);

// Test free(NULL) is safe.
static mp_obj_t test_uma_free_null(void) {
    uma_free(NULL);
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_free_null_obj, test_uma_free_null);

// Test malloc(0) returns NULL.
static mp_obj_t test_uma_malloc_zero(void) {
    void *ptr = uma_malloc(0, 0);
    if (ptr != NULL) {
        uma_free(ptr);
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_malloc_zero_obj, test_uma_malloc_zero);

// Test that flags=0 goes to generic pool, not fast.
static mp_obj_t test_uma_pool_generic_alloc(void) {
    void *ptr = uma_malloc(64, 0);
    if (!ptr) {
        return mp_const_false;
    }

    uma_pool_t *actual = find_pool_for_ptr(ptr);
    uma_free(ptr);
    return (actual && actual->flags == 0) ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_pool_generic_alloc_obj, test_uma_pool_generic_alloc);

// Test that FAST alloc goes to FAST pool (exact match).
static mp_obj_t test_uma_pool_fast_exact(void) {
    void *ptr = uma_malloc(64, UMA_FAST);
    if (!ptr) {
        return mp_const_false;
    }

    uma_pool_t *actual = find_pool_for_ptr(ptr);
    uma_free(ptr);
    return (actual && actual->flags == UMA_FAST) ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_pool_fast_exact_obj, test_uma_pool_fast_exact);

// Test that DTCM alloc goes to FAST|DTCM pool (partial match).
static mp_obj_t test_uma_pool_dtcm_partial(void) {
    void *ptr = uma_malloc(64, UMA_DTCM);
    if (!ptr) {
        return mp_const_false;
    }

    uma_pool_t *actual = find_pool_for_ptr(ptr);
    uma_free(ptr);
    return (actual && actual->flags == (UMA_FAST | UMA_DTCM)) ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_pool_dtcm_partial_obj, test_uma_pool_dtcm_partial);

// Test that FAST|DTCM alloc goes to FAST|DTCM pool (exact), not bare FAST.
static mp_obj_t test_uma_pool_fast_dtcm_exact(void) {
    void *ptr = uma_malloc(64, UMA_FAST | UMA_DTCM);
    if (!ptr) {
        return mp_const_false;
    }

    uma_pool_t *actual = find_pool_for_ptr(ptr);
    uma_free(ptr);
    return (actual && actual->flags == (UMA_FAST | UMA_DTCM)) ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_pool_fast_dtcm_exact_obj, test_uma_pool_fast_dtcm_exact);

// Test DTCM fallback to generic: request exceeds FAST|DTCM pool.
static mp_obj_t test_uma_pool_dtcm_fallback_generic(void) {
    uma_pool_t *dtcm = find_pool_by_flags(UMA_FAST | UMA_DTCM);
    if (!dtcm) {
        return mp_const_false;
    }

    // Request more than FAST|DTCM but within the FAST pool.
    // No exact match for UMA_DTCM. FAST|DTCM (partial) too small. FAST partial matches.
    void *ptr = uma_malloc(dtcm->size + 1, UMA_DTCM | UMA_MAYBE);
    if (!ptr) {
        return mp_const_false;
    }

    uma_pool_t *actual = find_pool_for_ptr(ptr);
    uma_free(ptr);
    return (actual && actual->flags == 0) ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_pool_dtcm_fallback_generic_obj, test_uma_pool_dtcm_fallback_generic);

// Test FAST fallback to generic: request exceeds all special pools.
static mp_obj_t test_uma_pool_fast_fallback_generic(void) {
    // Find the largest special pool.
    size_t max_special = 0;
    for (int i = 0; i < uma_pool_count(); i++) {
        uma_pool_t *p = uma_pool_get(i);
        if (p->flags != 0 && p->size > max_special) {
            max_special = p->size;
        }
    }

    // Request more than any special pool can hold.
    void *ptr = uma_malloc(max_special + 1, UMA_FAST | UMA_MAYBE);
    if (!ptr) {
        return mp_const_false;
    }

    uma_pool_t *actual = find_pool_for_ptr(ptr);
    uma_free(ptr);
    return (actual && actual->flags == 0) ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_pool_fast_fallback_generic_obj, test_uma_pool_fast_fallback_generic);

// Test DTCM|STRICT goes to FAST|DTCM (superset match).
static mp_obj_t test_uma_pool_dtcm_strict(void) {
    void *ptr = uma_malloc(64, UMA_DTCM | UMA_STRICT);
    if (!ptr) {
        return mp_const_false;
    }

    uma_pool_t *actual = find_pool_for_ptr(ptr);
    uma_free(ptr);
    return (actual && actual->flags == (UMA_FAST | UMA_DTCM)) ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_pool_dtcm_strict_obj, test_uma_pool_dtcm_strict);

// Test DTCM|STRICT returns NULL when request exceeds FAST|DTCM pool.
static mp_obj_t test_uma_pool_dtcm_strict_fail(void) {
    uma_pool_t *dtcm = find_pool_by_flags(UMA_FAST | UMA_DTCM);
    if (!dtcm) {
        return mp_const_false;
    }

    // Request more than the FAST|DTCM pool can hold.
    // DTCM|STRICT should fail — no fallback to generic or bare FAST.
    void *ptr = uma_malloc(dtcm->size + 1, UMA_DTCM | UMA_STRICT | UMA_MAYBE);
    return (ptr == NULL) ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_pool_dtcm_strict_fail_obj, test_uma_pool_dtcm_strict_fail);

// Test that realloc with UMA_CACHE preserves alignment when the block relocates.
static mp_obj_t test_uma_realloc_aligned(void) {
    // Allocate a cache-aligned block.
    void *aligned = uma_malloc(128, UMA_CACHE | UMA_PERSIST);
    if (!aligned || (uintptr_t) aligned % OMV_CACHE_LINE_SIZE != 0) {
        uma_free(aligned);
        return mp_const_false;
    }

    // Allocate a blocker to prevent in-place growth.
    void *blocker = uma_malloc(64, 0);
    if (!blocker) {
        uma_free(aligned);
        return mp_const_false;
    }

    // Realloc to a larger size WITH UMA_CACHE. The blocker prevents
    // in-place resize, forcing relocation — alignment must be preserved.
    void *moved = uma_realloc(aligned, 256, UMA_CACHE | UMA_PERSIST);
    if (!moved) {
        uma_free(blocker);
        return mp_const_false;
    }

    bool pass = (moved != aligned
                 && (uintptr_t) moved % OMV_CACHE_LINE_SIZE == 0
                 && tlsf_block_is_persist(moved));

    uma_free(moved);
    uma_free(blocker);
    return pass ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_uma_realloc_aligned_obj, test_uma_realloc_aligned);

// Module definition
static const mp_rom_map_elem_t unittest_umalloc_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_unittest_umalloc) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_malloc_free), MP_ROM_PTR(&test_uma_malloc_free_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_calloc), MP_ROM_PTR(&test_uma_calloc_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_realloc), MP_ROM_PTR(&test_uma_realloc_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_multi_alloc_free), MP_ROM_PTR(&test_uma_multi_alloc_free_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_persist_survives_collect), MP_ROM_PTR(&test_uma_persist_survives_collect_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_collect_frees_all), MP_ROM_PTR(&test_uma_collect_frees_all_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_collect_mixed), MP_ROM_PTR(&test_uma_collect_mixed_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_stats), MP_ROM_PTR(&test_uma_stats_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_collect_stats), MP_ROM_PTR(&test_uma_collect_stats_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_fast_pool), MP_ROM_PTR(&test_uma_fast_pool_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_fast_fallback), MP_ROM_PTR(&test_uma_fast_fallback_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_cache_align), MP_ROM_PTR(&test_uma_cache_align_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_avail), MP_ROM_PTR(&test_uma_avail_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_avail_fragmented), MP_ROM_PTR(&test_uma_avail_fragmented_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_free_null), MP_ROM_PTR(&test_uma_free_null_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_malloc_zero), MP_ROM_PTR(&test_uma_malloc_zero_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_pool_generic_alloc), MP_ROM_PTR(&test_uma_pool_generic_alloc_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_pool_fast_exact), MP_ROM_PTR(&test_uma_pool_fast_exact_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_pool_dtcm_partial), MP_ROM_PTR(&test_uma_pool_dtcm_partial_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_pool_fast_dtcm_exact), MP_ROM_PTR(&test_uma_pool_fast_dtcm_exact_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_pool_dtcm_fallback_generic), MP_ROM_PTR(&test_uma_pool_dtcm_fallback_generic_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_pool_fast_fallback_generic), MP_ROM_PTR(&test_uma_pool_fast_fallback_generic_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_pool_dtcm_strict), MP_ROM_PTR(&test_uma_pool_dtcm_strict_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_pool_dtcm_strict_fail), MP_ROM_PTR(&test_uma_pool_dtcm_strict_fail_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_uma_realloc_aligned), MP_ROM_PTR(&test_uma_realloc_aligned_obj) },
};

static MP_DEFINE_CONST_DICT(unittest_umalloc_module_globals, unittest_umalloc_module_globals_table);

const mp_obj_module_t unittest_umalloc_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &unittest_umalloc_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_unittest_umalloc, unittest_umalloc_module);

#endif // MICROPY_PY_UNITTEST

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
 * Unified memory allocator backed by TLSF with multi-pool support.
 */
#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"

#include "tlsf.h"
#include "umalloc.h"
#include "board_config.h"
#include "omv_common.h"

static int uma_num_pools;
static uma_pool_t uma_pools[UMA_MAX_POOLS];
static volatile int uma_collect_locked;


void uma_init(void) {
    uma_num_pools = 0;

    #if defined(OMV_UMA_BLOCK0_MEMORY)
    typedef struct {
        uint8_t *addr;
        uint32_t size;
        uint32_t flags;
    } uma_blocks_table_t;

    extern const uma_blocks_table_t _uma_blocks_table_start;
    extern const uma_blocks_table_t _uma_blocks_table_end;

    for (uma_blocks_table_t const *block = &_uma_blocks_table_start;
         block < &_uma_blocks_table_end; block++) {
        uma_pool_add(block->addr, block->size, block->flags);
    }
    #endif
}

NORETURN void uma_fail(void) {
    mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Out of memory"));
}

int uma_pool_count(void) {
    return uma_num_pools;
}

uma_pool_t *uma_pool_get(int index) {
    if (index < 0 || index >= uma_num_pools) {
        return NULL;
    }
    return &uma_pools[index];
}

void uma_pool_add(void *mem, size_t size, uint32_t flags) {
    if (uma_num_pools >= UMA_MAX_POOLS) {
        uma_fail();
    }

    tlsf_t tlsf = tlsf_create_with_pool(mem, size);
    if (!tlsf) {
        return;
    }

    size_t overhead = tlsf_size() + tlsf_pool_overhead();
    size_t usable = (size > overhead) ? size - overhead : 0;

    uma_pool_t *p = &uma_pools[uma_num_pools++];
    p->tlsf = tlsf;
    p->base = (uintptr_t) mem;
    p->end = (uintptr_t) mem + size;
    p->flags = flags;
    p->size = usable;
    p->free = usable;
    p->persist = 0;
    p->peak = 0;
}

uma_pool_t *uma_pool_find(const void *ptr, size_t size, uint32_t flags) {
    // Lookup by pointer.
    if (ptr != NULL) {
        uintptr_t addr = (uintptr_t) ptr;
        for (int i = 0; i < uma_num_pools; i++) {
            if (addr >= uma_pools[i].base && addr < uma_pools[i].end) {
                return &uma_pools[i];
            }
        }
        // realloc/free invalid pointer
        uma_fail();
    }

    uma_pool_t *partial = NULL;
    uma_pool_t *fallback = NULL;
    bool strict = flags & UMA_STRICT;
    bool persist = flags & UMA_PERSIST;
    flags &= UMA_MEM_ATTR_MASK;

    for (int i = 0; i < uma_num_pools; i++) {
        if (tlsf_alloc_size_max(uma_pools[i].tlsf) < size) {
            continue;
        }
        // Never place persistent allocations in transient pools.
        if (persist && (uma_pools[i].flags & UMA_TRANSIENT)) {
            continue;
        }
        // Exact attribute match (also handles flags==0 -> generic pool).
        if (flags == uma_pools[i].flags) {
            return &uma_pools[i];
        }
        // Allow partial match unless strict matching was requested.
        if (!strict && !partial && (uma_pools[i].flags & flags)) {
            partial = &uma_pools[i];
        }
        // Allow generic fallback unless strict matching was requested.
        if (!strict && !fallback && !uma_pools[i].flags) {
            fallback = &uma_pools[i];
        }
    }

    return partial ? partial : fallback;
}

void *uma_malloc(size_t size, uint32_t flags) {
    void *ptr = NULL;

    if (size == 0) {
        return NULL;
    }

    if (flags & UMA_CACHE) {
        size = OMV_ALIGN_TO(size, OMV_CACHE_LINE_SIZE);
    }

    uma_pool_t *pool = uma_pool_find(NULL, size, flags);
    if (!pool) {
        if (flags & UMA_MAYBE) {
            return NULL;
        }
        uma_fail();
    }

    if (flags & UMA_CACHE) {
        ptr = tlsf_memalign(pool->tlsf, OMV_CACHE_LINE_SIZE, size);
    } else {
        ptr = tlsf_malloc(pool->tlsf, size);
    }
    if (!ptr) {
        if (flags & UMA_MAYBE) {
            return NULL;
        }
        uma_fail();
    }

    pool->free -= tlsf_block_size(ptr);
    if (flags & UMA_PERSIST) {
        tlsf_block_set_persist(ptr);
        pool->persist += tlsf_block_size(ptr);
    }
    if ((pool->size - pool->free) > pool->peak) {
        pool->peak = pool->size - pool->free;
    }

    return ptr;
}

void *uma_malign(size_t size, size_t align, uint32_t flags) {
    void *ptr = NULL;

    if (size == 0) {
        return NULL;
    }

    // Round the size up to the alignment for convenience. tlsf only rounds it up
    // to its own 4 byte granularity, regardless of the requested alignment.
    size = OMV_ALIGN_TO(size, OMV_MAX(align, 4));

    uma_pool_t *pool = uma_pool_find(NULL, size, flags);
    if (!pool) {
        if (flags & UMA_MAYBE) {
            return NULL;
        }
        uma_fail();
    }

    ptr = tlsf_memalign(pool->tlsf, align, size);
    if (!ptr) {
        if (flags & UMA_MAYBE) {
            return NULL;
        }
        uma_fail();
    }

    pool->free -= tlsf_block_size(ptr);
    if (flags & UMA_PERSIST) {
        tlsf_block_set_persist(ptr);
        pool->persist += tlsf_block_size(ptr);
    }
    if ((pool->size - pool->free) > pool->peak) {
        pool->peak = pool->size - pool->free;
    }

    return ptr;
}

void *uma_calloc(size_t size, uint32_t flags) {
    void *ptr = uma_malloc(size, flags);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void *uma_realloc(void *ptr, size_t size, uint32_t flags) {
    if (!ptr) {
        return uma_malloc(size, flags);
    }

    if ((flags & UMA_PERSIST) || tlsf_block_is_persist(ptr)) {
        // Well, we can, but there's no use use and it's much simple this way.
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("can't realloc a persistent block"));
    }

    if (size == 0) {
        uma_free(ptr);
        return NULL;
    }

    void *p = NULL;
    uma_pool_t *pool = NULL;
    size_t old_size = tlsf_block_size(ptr);

    if (flags & UMA_CACHE) {
        size = OMV_ALIGN_TO(size, OMV_CACHE_LINE_SIZE);
    }

    if (tlsf_block_can_resize(ptr, size)) {
        // If the block can grow or shrink in place, its base pointer
        // won't change and tlsf_realloc will perserve the alignment.
        // Find the pointer's pool
        pool = uma_pool_find(ptr, 0, 0);
        p = tlsf_realloc(pool->tlsf, ptr, size);
        if (!p) {
            if (flags & UMA_MAYBE) {
                return NULL;
            }
            uma_fail();
        }
    } else {
        // If the block can't grow in place, we handle the alloc and
        // memcpy here to respect the requested alignment (if any).
        // Find a new pool for the new alloc.
        pool = uma_pool_find(0, size, flags);
        if (!pool) {
            if (flags & UMA_MAYBE) {
                return NULL;
            }
            uma_fail();
        }

        if (flags & UMA_CACHE) {
            p = tlsf_memalign(pool->tlsf, OMV_CACHE_LINE_SIZE, size);
        } else {
            p = tlsf_malloc(pool->tlsf, size);
        }

        if (!p) {
            if (flags & UMA_MAYBE) {
                return NULL;
            }
            uma_fail();
        }

        memcpy(p, ptr, OMV_MIN(size, old_size));
        uma_free(ptr);
        old_size = 0;   // for accounting
    }

    size_t new_size = tlsf_block_size(p);
    pool->free += (int) old_size - (int) new_size;
    if ((pool->size - pool->free) > pool->peak) {
        pool->peak = pool->size - pool->free;
    }

    return p;
}

void uma_free(void *ptr) {
    if (!ptr) {
        return;
    }

    uma_pool_t *pool = uma_pool_find(ptr, 0, 0);
    if (tlsf_block_is_persist(ptr)) {
        pool->persist -= tlsf_block_size(ptr);
    }
    pool->free += tlsf_block_size(ptr);
    tlsf_free(pool->tlsf, ptr);
}

void uma_transient_acquire(void) {
    for (int i = 0; i < uma_num_pools; i++) {
        uma_pool_t *p = &uma_pools[i];
        if (!(p->flags & UMA_TRANSIENT)) {
            continue;
        }
        if (p->free != p->size) {
            mp_raise_msg(&mp_type_MemoryError,
                         MP_ERROR_TEXT("Transient pool in use"));
        }
        #ifdef __DCACHE_PRESENT
        if (p->peak == 0) {
            SCB_InvalidateDCache_by_Addr((void *) p->base, tlsf_size() + tlsf_pool_overhead());
        } else {
            SCB_InvalidateDCache_by_Addr((void *) p->base, p->end - p->base);
        }
        #endif
    }
}

void uma_transient_release(void) {
    for (int i = 0; i < uma_num_pools; i++) {
        uma_pool_t *p = &uma_pools[i];
        if (!(p->flags & UMA_TRANSIENT)) {
            continue;
        }
        #ifdef __DCACHE_PRESENT
        SCB_InvalidateDCache_by_Addr((void *) p->base, p->end - p->base);
        #endif

        void *mem = (void *) p->base;
        size_t size = p->end - p->base;

        p->tlsf = tlsf_create_with_pool(mem, size);
        if (!p->tlsf) {
            uma_fail();
        }
        p->free = p->size;
        p->persist = 0;
        p->peak = 0;
    }
}

void uma_collect_lock(void) {
    uma_collect_locked++;
}

void uma_collect_unlock(void) {
    uma_collect_locked--;
}

void uma_collect(void) {
    if (uma_collect_locked) {
        return;
    }
    for (int i = 0; i < uma_num_pools; i++) {
        size_t nblocks = 0;
        size_t freed = tlsf_collect(uma_pools[i].tlsf, &nblocks);
        uma_pools[i].free += freed;
    }
}

size_t uma_avail(uint32_t flags) {
    uma_pool_t *pool = uma_pool_find(NULL, 0, flags);
    if (!pool) {
        return 0;
    }
    size_t size = tlsf_alloc_size_max(pool->tlsf);
    return OMV_ALIGN_DOWN(size, OMV_CACHE_LINE_SIZE);
}

static void uma_block_walker(void *ptr, void *user) {
    uma_stats_t *s = (uma_stats_t *) user;
    if (tlsf_block_is_free(ptr)) {
        s->free_count++;
    } else if (tlsf_block_is_persist(ptr)) {
        s->persist_count++;
    } else {
        s->used_count++;
    }
}

void uma_get_stats(int index, bool full, uma_stats_t *stats) {
    int start = 0, end = uma_num_pools;

    if (index >= 0 && index < uma_num_pools) {
        start = index;
        end = index + 1;
    }

    memset(stats, 0, sizeof(*stats));

    for (int i = start; i < end; i++) {
        uma_pool_t *p = &uma_pools[i];
        stats->free_bytes += p->free;
        stats->used_bytes += p->size - p->free;
        stats->persist_bytes += p->persist;
    }

    if (full) {
        for (int i = start; i < end; i++) {
            uma_pool_t *p = &uma_pools[i];
            // It's not safe to walk transient pools
            if (!(p->flags & UMA_TRANSIENT)) {
                tlsf_walk(p->tlsf, uma_block_walker, stats);
            }
        }
    }

    if (end - start == 1) {
        stats->peak_bytes = uma_pools[start].peak;
    }
}

void uma_print_stats(int index) {
    int start = 0, end = uma_num_pools;

    if (index >= 0 && index < uma_num_pools) {
        start = index;
        end = index + 1;
    }

    for (int i = start; i < end; i++) {
        uma_pool_t *p = &uma_pools[i];
        uma_stats_t stats;
        uma_get_stats(i, true, &stats);

        printf("pool %d: base=0x%08lx size=%lu "
               "used=%lu(%lu) free=%lu(%lu) persist=%lu(%lu) "
               "peak=%lu flags=%s%s%s\n",
               i, (unsigned long) p->base, (unsigned long) p->size,
               (unsigned long) stats.used_bytes, (unsigned long) stats.used_count,
               (unsigned long) stats.free_bytes, (unsigned long) stats.free_count,
               (unsigned long) stats.persist_bytes, (unsigned long) stats.persist_count,
               (unsigned long) stats.peak_bytes,
               (p->flags & UMA_FAST) ? "FAST|" : "",
               (p->flags & UMA_DTCM) ? "DTCM|" : "",
               (p->flags & UMA_TRANSIENT) ? "TRANSIENT|" : "");
    }
}

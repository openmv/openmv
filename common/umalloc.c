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
        uma_add_pool(block->addr, block->size, block->flags);
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

void uma_add_pool(void *mem, size_t size, uint32_t flags) {
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
    p->peak = 0;
}

static uma_pool_t *uma_pool_find(const void *ptr, size_t size, uint32_t flags) {
    static int mp_poll;

    // Lookup by pointer.
    if (ptr != NULL) {
        uintptr_t addr = (uintptr_t) ptr;
        for (int i = 0; i < uma_num_pools; i++) {
            if (addr >= uma_pools[i].base && addr < uma_pools[i].end) {
                return &uma_pools[i];
            }
        }
        return NULL;
    }

    uma_pool_t *partial = NULL;
    uma_pool_t *fallback = NULL;
    bool strict = flags & UMA_STRICT;
    flags &= UMA_MEM_ATTR_MASK;

    for (int i = 0; i < uma_num_pools; i++) {
        if (uma_pools[i].free < size) {
            continue;
        }
        // Exact attribute match (also handles flags==0 -> generic pool).
        if (flags == uma_pools[i].flags) {
            return &uma_pools[i];
        }
        // Save first partial attribute match.
        // With strict, only allow superset matches (pool has all requested attrs).
        if (!partial && (flags & uma_pools[i].flags)) {
            if (!strict || (flags & uma_pools[i].flags) == flags) {
                partial = &uma_pools[i];
            }
        }
        // Allow generic fallback unless strict matching was requested.
        if (!strict && !fallback && !uma_pools[i].flags) {
            fallback = &uma_pools[i];
        }
    }

    // Poll MicroPython events in long running tasks.
    if (MP_STATE_THREAD(stack_top) && !(++mp_poll & 0xFF)) {
        mp_event_handle_nowait();
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

    if (flags & UMA_PERSIST) {
        tlsf_block_set_persist(ptr);
    }

    pool->free -= tlsf_block_size(ptr);
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

    size = OMV_ALIGN_TO(size, OMV_MIN(align, 4));

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

    if (flags & UMA_PERSIST) {
        tlsf_block_set_persist(ptr);
    }

    pool->free -= tlsf_block_size(ptr);
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

    if (size == 0) {
        uma_free(ptr);
        return NULL;
    }

    uma_pool_t *pool = uma_pool_find(ptr, 0, 0);
    if (!pool) {
        uma_fail();
    }

    void *p = NULL;
    size_t old_size = tlsf_block_size(ptr);

    if (flags & UMA_CACHE) {
        size = OMV_ALIGN_TO(size, OMV_CACHE_LINE_SIZE);
    }

    if (tlsf_block_can_resize(ptr, size)) {
        // If the block can grow or shrink in place, its base pointer
        // won't change and tlsf_realloc will perserve the alignment.
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

    if (flags & UMA_PERSIST) {
        tlsf_block_set_persist(p);
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
    if (!pool) {
        return;
    }

    pool->free += tlsf_block_size(ptr);
    tlsf_free(pool->tlsf, ptr);
}

void uma_collect(void) {
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
    size_t size = tlsf_block_size(ptr);
    if (tlsf_block_is_free(ptr)) {
        s->free_count++;
        s->free_bytes += size;
    } else if (tlsf_block_is_persist(ptr)) {
        s->persist_count++;
        s->persist_bytes += size;
    } else {
        s->used_count++;
        s->used_bytes += size;
    }
}

void uma_get_stats(uma_stats_t *stats) {
    memset(stats, 0, sizeof(*stats));
    for (int i = 0; i < uma_num_pools; i++) {
        tlsf_walk(uma_pools[i].tlsf, uma_block_walker, stats);
    }
}

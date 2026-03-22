/*
 * SPDX-License-Identifier: MIT AND BSD-3-Clause
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
 * TLSF (Two Level Segregated Fit) algorithm by Matthew Conte.
 * Copyright (c) 2006-2016, Matthew Conte. All rights reserved.
 * Used under BSD 3-Clause License. See below.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL MATTHEW CONTE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Unified memory allocator backed by TLSF with multi-pool support.
 */
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "omv_umalloc.h"
#include "omv_common.h"

// ---------------------------------------------------------------------------
// TLSF engine (inlined from mattconte/tlsf, stripped to GCC/ARM 32-bit)
// ---------------------------------------------------------------------------

#define tlsf_cast(t, exp)   ((t) (exp))
#define tlsf_min(a, b)      ((a) < (b) ? (a) : (b))
#define tlsf_max(a, b)      ((a) > (b) ? (a) : (b))
#define tlsf_assert assert

enum tlsf_public {
    SL_INDEX_COUNT_LOG2 = 5,
};

enum tlsf_private {
    ALIGN_SIZE_LOG2  = 2,
    ALIGN_SIZE       = (1 << ALIGN_SIZE_LOG2),
    FL_INDEX_MAX     = 30,
    SL_INDEX_COUNT   = (1 << SL_INDEX_COUNT_LOG2),
    FL_INDEX_SHIFT   = (SL_INDEX_COUNT_LOG2 + ALIGN_SIZE_LOG2),
    FL_INDEX_COUNT   = (FL_INDEX_MAX - FL_INDEX_SHIFT + 1),
    SMALL_BLOCK_SIZE = (1 << FL_INDEX_SHIFT),
};

static int tlsf_ffs(unsigned int word) {
    return __builtin_ffs(word) - 1;
}

static int tlsf_fls(unsigned int word) {
    return word ? 32 - __builtin_clz(word) - 1 : -1;
}


typedef struct block_header_t {
    struct block_header_t *prev_phys_block;
    size_t size;
    struct block_header_t *next_free;
    struct block_header_t *prev_free;
} block_header_t;

#define block_header_free_bit      ((size_t) 1 << 0)
#define block_header_prev_free_bit ((size_t) 1 << 1)
#define block_header_overhead      (sizeof(size_t))
#define block_start_offset         (offsetof(block_header_t, size) + sizeof(size_t))
#define block_size_min             (sizeof(block_header_t) - sizeof(block_header_t *))
#define block_size_max             ((size_t) 1 << FL_INDEX_MAX)

typedef struct control_t {
    block_header_t block_null;
    unsigned int fl_bitmap;
    unsigned int sl_bitmap[FL_INDEX_COUNT];
    block_header_t *blocks[FL_INDEX_COUNT][SL_INDEX_COUNT];
} control_t;

typedef ptrdiff_t tlsfptr_t;

static size_t block_size(const block_header_t *block) {
    return block->size & ~(block_header_free_bit | block_header_prev_free_bit);
}

static void block_set_size(block_header_t *block, size_t size) {
    const size_t oldsize = block->size;
    block->size = size | (oldsize & (block_header_free_bit | block_header_prev_free_bit));
}

#if !defined(NDEBUG)
static int block_is_last(const block_header_t *block) {
    return block_size(block) == 0;
}
#endif

static int block_is_free(const block_header_t *block) {
    return tlsf_cast(int, block->size & block_header_free_bit);
}

static void block_set_free(block_header_t *block) {
    block->size |= block_header_free_bit;
}

static void block_set_used(block_header_t *block) {
    block->size &= ~block_header_free_bit;
}

static int block_is_prev_free(const block_header_t *block) {
    return tlsf_cast(int, block->size & block_header_prev_free_bit);
}

static void block_set_prev_free(block_header_t *block) {
    block->size |= block_header_prev_free_bit;
}

static void block_set_prev_used(block_header_t *block) {
    block->size &= ~block_header_prev_free_bit;
}

static block_header_t *block_from_ptr(const void *ptr) {
    return tlsf_cast(block_header_t *,
                     tlsf_cast(unsigned char *, ptr) - block_start_offset);
}

static void *block_to_ptr(const block_header_t *block) {
    return tlsf_cast(void *,
                     tlsf_cast(unsigned char *, block) + block_start_offset);
}

static block_header_t *offset_to_block(const void *ptr, size_t size) {
    return tlsf_cast(block_header_t *, tlsf_cast(tlsfptr_t, ptr) + size);
}

static block_header_t *block_prev(const block_header_t *block) {
    tlsf_assert(block_is_prev_free(block) && "previous block must be free");
    return block->prev_phys_block;
}

static block_header_t *block_next(const block_header_t *block) {
    block_header_t *next = offset_to_block(block_to_ptr(block),
                                           block_size(block) - block_header_overhead);
    tlsf_assert(!block_is_last(block));
    return next;
}

static block_header_t *block_link_next(block_header_t *block) {
    block_header_t *next = block_next(block);
    next->prev_phys_block = block;
    return next;
}

static void block_mark_as_free(block_header_t *block) {
    block_header_t *next = block_link_next(block);
    block_set_prev_free(next);
    block_set_free(block);
}

static void block_mark_as_used(block_header_t *block) {
    block_header_t *next = block_next(block);
    block_set_prev_used(next);
    block_set_used(block);
}

static size_t align_up(size_t x, size_t align) {
    tlsf_assert(0 == (align & (align - 1)) && "must align to a power of two");
    return (x + (align - 1)) & ~(align - 1);
}

static size_t align_down(size_t x, size_t align) {
    tlsf_assert(0 == (align & (align - 1)) && "must align to a power of two");
    return x - (x & (align - 1));
}

static void *align_ptr(const void *ptr, size_t align) {
    const tlsfptr_t aligned =
        (tlsf_cast(tlsfptr_t, ptr) + (align - 1)) & ~(align - 1);
    tlsf_assert(0 == (align & (align - 1)) && "must align to a power of two");
    return tlsf_cast(void *, aligned);
}

static size_t adjust_request_size(size_t size, size_t align) {
    size_t adjust = 0;
    if (size) {
        const size_t aligned = align_up(size, align);
        if (aligned < block_size_max) {
            adjust = tlsf_max(aligned, block_size_min);
        }
    }
    return adjust;
}

static void mapping_insert(size_t size, int *fli, int *sli) {
    int fl, sl;
    if (size < SMALL_BLOCK_SIZE) {
        fl = 0;
        sl = tlsf_cast(int, size) / (SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
    } else {
        fl = tlsf_fls(size);
        sl = tlsf_cast(int, size >> (fl - SL_INDEX_COUNT_LOG2)) ^ (1 << SL_INDEX_COUNT_LOG2);
        fl -= (FL_INDEX_SHIFT - 1);
    }
    *fli = fl;
    *sli = sl;
}

static void mapping_search(size_t size, int *fli, int *sli) {
    if (size >= SMALL_BLOCK_SIZE) {
        const size_t round = (1 << (tlsf_fls(size) - SL_INDEX_COUNT_LOG2)) - 1;
        size += round;
    }
    mapping_insert(size, fli, sli);
}

static block_header_t *search_suitable_block(control_t *control, int *fli, int *sli) {
    int fl = *fli;
    int sl = *sli;

    unsigned int sl_map = control->sl_bitmap[fl] & (~0U << sl);
    if (!sl_map) {
        const unsigned int fl_map = control->fl_bitmap & (~0U << (fl + 1));
        if (!fl_map) {
            return 0;
        }
        fl = tlsf_ffs(fl_map);
        *fli = fl;
        sl_map = control->sl_bitmap[fl];
    }
    tlsf_assert(sl_map && "internal error - second level bitmap is null");
    sl = tlsf_ffs(sl_map);
    *sli = sl;

    return control->blocks[fl][sl];
}

static void remove_free_block(control_t *control, block_header_t *block, int fl, int sl) {
    block_header_t *prev = block->prev_free;
    block_header_t *next = block->next_free;
    tlsf_assert(prev && "prev_free field can not be null");
    tlsf_assert(next && "next_free field can not be null");
    next->prev_free = prev;
    prev->next_free = next;

    if (control->blocks[fl][sl] == block) {
        control->blocks[fl][sl] = next;
        if (next == &control->block_null) {
            control->sl_bitmap[fl] &= ~(1U << sl);
            if (!control->sl_bitmap[fl]) {
                control->fl_bitmap &= ~(1U << fl);
            }
        }
    }
}

static void insert_free_block(control_t *control, block_header_t *block, int fl, int sl) {
    block_header_t *current = control->blocks[fl][sl];
    tlsf_assert(current && "free list cannot have a null entry");
    tlsf_assert(block && "cannot insert a null entry into the free list");
    block->next_free = current;
    block->prev_free = &control->block_null;
    current->prev_free = block;

    tlsf_assert(block_to_ptr(block) == align_ptr(block_to_ptr(block), ALIGN_SIZE)
                && "block not aligned properly");

    control->blocks[fl][sl] = block;
    control->fl_bitmap |= (1U << fl);
    control->sl_bitmap[fl] |= (1U << sl);
}

static void block_remove(control_t *control, block_header_t *block) {
    int fl, sl;
    mapping_insert(block_size(block), &fl, &sl);
    remove_free_block(control, block, fl, sl);
}

static void block_insert(control_t *control, block_header_t *block) {
    int fl, sl;
    mapping_insert(block_size(block), &fl, &sl);
    insert_free_block(control, block, fl, sl);
}

static int block_can_split(block_header_t *block, size_t size) {
    return block_size(block) >= sizeof(block_header_t) + size;
}

static block_header_t *block_split(block_header_t *block, size_t size) {
    block_header_t *remaining =
        offset_to_block(block_to_ptr(block), size - block_header_overhead);
    const size_t remain_size = block_size(block) - (size + block_header_overhead);

    tlsf_assert(block_to_ptr(remaining) == align_ptr(block_to_ptr(remaining), ALIGN_SIZE)
                && "remaining block not aligned properly");

    tlsf_assert(block_size(block) == remain_size + size + block_header_overhead);
    block_set_size(remaining, remain_size);
    tlsf_assert(block_size(remaining) >= block_size_min && "block split with invalid size");

    block_set_size(block, size);
    block_mark_as_free(remaining);
    return remaining;
}

static block_header_t *block_absorb(block_header_t *prev, block_header_t *block) {
    tlsf_assert(!block_is_last(prev) && "previous block can't be last");
    prev->size += block_size(block) + block_header_overhead;
    block_link_next(prev);
    return prev;
}

static block_header_t *block_merge_prev(control_t *control, block_header_t *block) {
    if (block_is_prev_free(block)) {
        block_header_t *prev = block_prev(block);
        tlsf_assert(prev && "prev physical block can't be null");
        tlsf_assert(block_is_free(prev) && "prev block is not free though marked as such");
        block_remove(control, prev);
        block = block_absorb(prev, block);
    }
    return block;
}

static block_header_t *block_merge_next(control_t *control, block_header_t *block) {
    block_header_t *next = block_next(block);
    tlsf_assert(next && "next physical block can't be null");
    if (block_is_free(next)) {
        tlsf_assert(!block_is_last(block) && "previous block can't be last");
        block_remove(control, next);
        block = block_absorb(block, next);
    }
    return block;
}

static void block_trim_free(control_t *control, block_header_t *block, size_t size) {
    tlsf_assert(block_is_free(block) && "block must be free");
    if (block_can_split(block, size)) {
        block_header_t *remaining_block = block_split(block, size);
        block_link_next(block);
        block_set_prev_free(remaining_block);
        block_insert(control, remaining_block);
    }
}

static void block_trim_used(control_t *control, block_header_t *block, size_t size) {
    tlsf_assert(!block_is_free(block) && "block must be used");
    if (block_can_split(block, size)) {
        block_header_t *remaining_block = block_split(block, size);
        block_set_prev_used(remaining_block);
        remaining_block = block_merge_next(control, remaining_block);
        block_insert(control, remaining_block);
    }
}

static block_header_t *block_trim_free_leading(control_t *control, block_header_t *block, size_t size) {
    block_header_t *remaining_block = block;
    if (block_can_split(block, size)) {
        remaining_block = block_split(block, size - block_header_overhead);
        block_set_prev_free(remaining_block);
        block_link_next(block);
        block_insert(control, block);
    }
    return remaining_block;
}

static block_header_t *block_locate_free(control_t *control, size_t size) {
    int fl = 0, sl = 0;
    block_header_t *block = 0;

    if (size) {
        mapping_search(size, &fl, &sl);
        if (fl < FL_INDEX_COUNT) {
            block = search_suitable_block(control, &fl, &sl);
        }
    }

    if (block) {
        tlsf_assert(block_size(block) >= size);
        remove_free_block(control, block, fl, sl);
    }

    return block;
}

static void *block_prepare_used(control_t *control, block_header_t *block, size_t size) {
    void *p = 0;
    if (block) {
        tlsf_assert(size && "size must be non-zero");
        block_trim_free(control, block, size);
        block_mark_as_used(block);
        p = block_to_ptr(block);
    }
    return p;
}

static void control_construct(control_t *control) {
    int i, j;
    control->block_null.next_free = &control->block_null;
    control->block_null.prev_free = &control->block_null;
    control->fl_bitmap = 0;
    for (i = 0; i < FL_INDEX_COUNT; ++i) {
        control->sl_bitmap[i] = 0;
        for (j = 0; j < SL_INDEX_COUNT; ++j) {
            control->blocks[i][j] = &control->block_null;
        }
    }
}

static void tlsf_pool_add(control_t *control, void *mem, size_t bytes) {
    const size_t pool_overhead = 2 * block_header_overhead;
    const size_t pool_bytes = align_down(bytes - pool_overhead, ALIGN_SIZE);

    block_header_t *block = offset_to_block(mem, -(tlsfptr_t) block_header_overhead);
    block_set_size(block, pool_bytes);
    block_set_free(block);
    block_set_prev_used(block);
    block_insert(control, block);

    block_header_t *next = block_link_next(block);
    block_set_size(next, 0);
    block_set_used(next);
    block_set_prev_free(next);
}

static void *tlsf_malloc_internal(control_t *control, size_t size) {
    const size_t adjust = adjust_request_size(size, ALIGN_SIZE);
    block_header_t *block = block_locate_free(control, adjust);
    return block_prepare_used(control, block, adjust);
}

static void *tlsf_memalign_internal(control_t *control, size_t align, size_t size) {
    const size_t adjust = adjust_request_size(size, ALIGN_SIZE);
    const size_t gap_minimum = sizeof(block_header_t);
    const size_t size_with_gap = adjust_request_size(adjust + align + gap_minimum, align);
    const size_t aligned_size = (adjust && align > ALIGN_SIZE) ? size_with_gap : adjust;

    block_header_t *block = block_locate_free(control, aligned_size);

    tlsf_assert(sizeof(block_header_t) == block_size_min + block_header_overhead);

    if (block) {
        void *ptr = block_to_ptr(block);
        void *aligned = align_ptr(ptr, align);
        size_t gap = tlsf_cast(size_t,
                               tlsf_cast(tlsfptr_t, aligned) - tlsf_cast(tlsfptr_t, ptr));

        if (gap && gap < gap_minimum) {
            const size_t gap_remain = gap_minimum - gap;
            const size_t offset = tlsf_max(gap_remain, align);
            const void *next_aligned = tlsf_cast(void *,
                                                 tlsf_cast(tlsfptr_t, aligned) + offset);
            aligned = align_ptr(next_aligned, align);
            gap = tlsf_cast(size_t,
                            tlsf_cast(tlsfptr_t, aligned) - tlsf_cast(tlsfptr_t, ptr));
        }

        if (gap) {
            tlsf_assert(gap >= gap_minimum && "gap size too small");
            block = block_trim_free_leading(control, block, gap);
        }
    }

    return block_prepare_used(control, block, adjust);
}

static void tlsf_free_internal(control_t *control, void *ptr) {
    if (ptr) {
        block_header_t *block = block_from_ptr(ptr);
        tlsf_assert(!block_is_free(block) && "block already marked as free");
        block_mark_as_free(block);
        block = block_merge_prev(control, block);
        block = block_merge_next(control, block);
        block_insert(control, block);
    }
}

static void *tlsf_realloc_internal(control_t *control, void *ptr, size_t size) {
    void *p = 0;

    if (ptr && size == 0) {
        tlsf_free_internal(control, ptr);
    } else if (!ptr) {
        p = tlsf_malloc_internal(control, size);
    } else {
        block_header_t *block = block_from_ptr(ptr);
        block_header_t *next = block_next(block);

        const size_t cursize = block_size(block);
        const size_t combined = cursize + block_size(next) + block_header_overhead;
        const size_t adjust = adjust_request_size(size, ALIGN_SIZE);

        tlsf_assert(!block_is_free(block) && "block already marked as free");

        if (adjust > cursize && (!block_is_free(next) || adjust > combined)) {
            p = tlsf_malloc_internal(control, size);
            if (p) {
                const size_t minsize = tlsf_min(cursize, size);
                memcpy(p, ptr, minsize);
                tlsf_free_internal(control, ptr);
            }
        } else {
            if (adjust > cursize) {
                block_merge_next(control, block);
                block_mark_as_used(block);
            }
            block_trim_used(control, block, adjust);
            p = ptr;
        }
    }

    return p;
}

// ---------------------------------------------------------------------------
// Multi-pool layer
// ---------------------------------------------------------------------------

#define UMA_MAX_POOLS       4

typedef struct {
    uintptr_t base;
    uintptr_t end;
    uint32_t flags;
} uma_pool_t;

static int uma_pool_count;
static uma_pool_t uma_pools[UMA_MAX_POOLS];

void uma_init(void) {
    uma_pool_count = 0;
}

NORETURN void uma_fail(void) {
    mp_raise_msg(&mp_type_MemoryError,
                 MP_ERROR_TEXT("Out of fast frame buffer stack memory"));
}

static control_t *uma_pool_ctrl(const uma_pool_t *pool) {
    return (control_t *) pool->base;
}

void uma_add_pool(void *mem, size_t size, uint32_t flags) {
    if (uma_pool_count >= UMA_MAX_POOLS) {
        uma_fail();
    }

    size_t ctrl_size = align_up(sizeof(control_t), ALIGN_SIZE);
    if (size <= ctrl_size + 2 * block_header_overhead + block_size_min) {
        uma_fail();
    }

    control_t *ctrl = (control_t *) mem;
    control_construct(ctrl);

    void *pool_mem = (char *) mem + ctrl_size;
    size_t pool_size = size - ctrl_size;
    tlsf_pool_add(ctrl, pool_mem, pool_size);

    uma_pool_t *p = &uma_pools[uma_pool_count++];
    p->base = (uintptr_t) mem;
    p->end = (uintptr_t) mem + size;
    p->flags = flags;
}

static control_t *uma_pool_find(const void *ptr, uint32_t flags) {
    if (ptr) {
        uintptr_t addr = (uintptr_t) ptr;
        for (int i = 0; i < uma_pool_count; i++) {
            if (addr >= uma_pools[i].base && addr < uma_pools[i].end) {
                return uma_pool_ctrl(&uma_pools[i]);
            }
        }
        return NULL;
    }

    // Try pools matching requested flags.
    if (flags) {
        for (int i = 0; i < uma_pool_count; i++) {
            if (uma_pools[i].flags & flags) {
                return uma_pool_ctrl(&uma_pools[i]);
            }
        }
    }

    // Fall back to any pool.
    for (int i = 0; i < uma_pool_count; i++) {
        return uma_pool_ctrl(&uma_pools[i]);
    }

    return NULL;
}

void *uma_malloc(size_t size, uint32_t flags) {
    if (size == 0) {
        return NULL;
    }

    control_t *ctrl = uma_pool_find(NULL, flags);
    if (ctrl) {
        return tlsf_malloc_internal(ctrl, size);
    }

    return NULL;
}

void *uma_malign(size_t size, size_t align, uint32_t flags) {
    if (size == 0) {
        return NULL;
    }

    control_t *ctrl = uma_pool_find(NULL, flags);
    if (ctrl) {
        return tlsf_memalign_internal(ctrl, align, size);
    }

    return NULL;
}

void *uma_calloc(size_t num, size_t size, uint32_t flags) {
    size_t total = num * size;
    void *ptr = uma_malloc(total, flags);

    if (ptr) {
        memset(ptr, 0, total);
    }

    return ptr;
}

void *uma_realloc(void *ptr, size_t size) {
    if (!ptr) {
        return uma_malloc(size, 0);
    }

    if (size == 0) {
        uma_free(ptr);
        return NULL;
    }

    control_t *ctrl = uma_pool_find(ptr, 0);
    if (!ctrl) {
        uma_fail();
    }

    void *p = tlsf_realloc_internal(ctrl, ptr, size);
    if (!p) {
        // Standard realloc returns NULL. However, most callers don't
        // check the return of value.
        uma_fail();
    }

    return p;
}

void uma_free(void *ptr) {
    if (!ptr) {
        return;
    }

    control_t *ctrl = uma_pool_find(ptr, 0);
    if (!ctrl) {
        return;
    }

    tlsf_free_internal(ctrl, ptr);
}

size_t uma_avail(uint32_t flags) {
    // Walk TLSF free lists and sum available bytes in matching pools.
    size_t total = 0;
    for (int p = 0; p < uma_pool_count; p++) {
        if (flags && !(uma_pools[p].flags & flags)) {
            continue;
        }
        control_t *ctrl = uma_pool_ctrl(&uma_pools[p]);
        for (int fl = 0; fl < FL_INDEX_COUNT; fl++) {
            if (!ctrl->sl_bitmap[fl]) {
                continue;
            }
            for (int sl = 0; sl < SL_INDEX_COUNT; sl++) {
                block_header_t *block = ctrl->blocks[fl][sl];
                while (block != &ctrl->block_null) {
                    total += block_size(block);
                    block = block->next_free;
                }
            }
        }
    }
    return total;
}

void uma_stats(void) {
}

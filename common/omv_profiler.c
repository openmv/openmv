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
 * OpenMV code profiler.
 */
#if OMV_PROFILER_ENABLE
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mphal.h"
#include "omv_profiler.h"

#if __PMU_PRESENT
#define OMV_GET_CYCLE_COUNT()   ARM_PMU_Get_CCNTR()
#define OMV_GET_EVENT_COUNT(i)  ARM_PMU_Get_EVCNTR(i)
#else
#define OMV_GET_CYCLE_COUNT()   0
#define OMV_GET_EVENT_COUNT(i)  0
#endif  // __PMU_PRESENT
#define OMV_GET_TICKS_COUNT()   ticks_us_monotonic()

// Call stack entry for tracking nested calls
typedef struct {
    void *func_addr;            // Function address
    void *call_addr;            // Call site address

    uint32_t enter_ticks;       // Timestamp on entry
    uint32_t enter_cycles;      // Cycle count on entry

    uint64_t child_ticks;       // Total child execution time
    uint64_t child_cycles;      // Total child cycle count

    #if __PMU_PRESENT
    uint16_t enter_events[__PMU_NUM_EVENTCNT];  // PMU events on entry
    uint64_t child_events[__PMU_NUM_EVENTCNT];  // Total child PMU events
    #endif
} omv_stack_entry_t;

typedef struct {
    bool initialized;               // Profiler state.
    mutex_t mutex;                  // Protects profile data
    bool reset_pending;             // Reset requested flag
    omv_profiler_mode_t mode;       // Inclusive/exclusive mode

    uint32_t collisions;            // Hash table collision count
    omv_profiler_data_t *hash[OMV_PROFILER_HASH_SIZE];  // Hash table buckets

    uint32_t pool_index;            // Next free pool entry
    omv_profiler_data_t pool[OMV_PROFILER_HASH_SIZE];   // Entry pool

    int32_t stack_top;              // Current stack position
    uint32_t stack_depth;           // Max stack depth reached
    omv_stack_entry_t stack[OMV_PROFILER_STACK_DEPTH];  // Call stack
} omv_profiler_state_t;

static omv_profiler_state_t profiler;

#define profiler_stack_top(x) profiler.stack[profiler.stack_top].x
#define profiler_stack_prv(x) profiler.stack[profiler.stack_top - 1].x

static OMV_ATTR_NO_INSTRUMENT mp_uint_t ticks_us_monotonic(void) {
    static mp_uint_t last_timestamp = 0;
    mp_uint_t current = mp_hal_ticks_us();

    // Ensure monotonic behavior
    if (current > last_timestamp) {
        last_timestamp = current;
    }

    return last_timestamp;
}

static inline uint32_t OMV_ATTR_NO_INSTRUMENT hash_address(void *addr) {
    uint32_t x = (uint32_t) addr >> 2; // drop thumb bit + alignment bits
    // mix (Murmur finalizer style)
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x & (OMV_PROFILER_HASH_SIZE - 1);
}

// Initialize profiling system
void omv_profiler_init(void) {
    if (!profiler.initialized) {
        // Reset state
        memset(&profiler, 0, sizeof(profiler));
        #if __PMU_PRESENT
        // Enable PMU
        ARM_PMU_Enable();
        // Disable all event counters
        ARM_PMU_CNTR_Disable(((1U << __PMU_NUM_EVENTCNT) - 1));
        // Enable cycles counter.
        ARM_PMU_CNTR_Enable(PMU_CNTENSET_CCNTR_ENABLE_Msk);
        // Enable Trace
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        #endif
    }

    // Initialize mutex
    mutex_init0(&profiler.mutex);
    profiler.initialized = true;
}

void omv_profiler_reset(void) {
    if (!mutex_try_lock(&profiler.mutex, MUTEX_TID_OMV)) {
        // Set a pending reset if the data is locked.
        profiler.reset_pending = true;
    } else {
        // Reset state
        profiler.pool_index = 0;
        profiler.stack_top = -1;
        profiler.stack_depth = 0;
        profiler.collisions = 0;
        profiler.reset_pending = false;

        // Clear hash table
        memset(profiler.hash, 0, sizeof(profiler.hash));
        memset(profiler.pool, 0, sizeof(profiler.pool));
        memset(profiler.stack, 0, sizeof(profiler.stack));

        mutex_unlock(&profiler.mutex, MUTEX_TID_OMV);
    }
}

mutex_t *omv_profiler_lock(void) {
    return &profiler.mutex;
}

size_t omv_profiler_get_size(void) {
    return profiler.pool_index * sizeof(omv_profiler_data_t);
}

const void *omv_profiler_get_data(void) {
    return profiler.pool;
}

void omv_profiler_set_mode(uint32_t mode) {
    profiler.mode = mode;
    profiler.reset_pending = true;
}

void omv_profiler_set_event(uint32_t num, uint32_t type) {
    #if __PMU_PRESENT
    if (num < __PMU_NUM_EVENTCNT) {
        ARM_PMU_Disable();

        // Configure and enable event counter.
        ARM_PMU_Set_EVTYPER(num, type);
        ARM_PMU_CNTR_Enable(1 << num);

        // Reset all event counters
        ARM_PMU_EVCNTR_ALL_Reset();

        // Enable PMU
        ARM_PMU_Enable();
    }
    #endif
    profiler.reset_pending = true;
}

static omv_profiler_data_t *omv_profiler_get_entry(void *func_addr) {
    uint32_t hash = hash_address(func_addr);
    omv_profiler_data_t *entry = profiler.hash[hash];

    // Search existing entries in collision chain
    while (entry != NULL) {
        if (entry->func_addr == func_addr) {
            return entry;
        }
        entry = entry->next;
    }

    // Create new entry if not found
    if (profiler.pool_index >= OMV_PROFILER_HASH_SIZE) {
        // Pool exhausted
        return NULL;
    }

    entry = &profiler.pool[profiler.pool_index++];

    memset(entry, 0, sizeof(omv_profiler_data_t));
    entry->min_ticks = UINT32_MAX;

    // Insert at head of collision chain
    if (profiler.hash[hash] != NULL) {
        profiler.collisions++;
    }

    entry->next = profiler.hash[hash];
    profiler.hash[hash] = entry;

    return entry;
}

void OMV_ATTR_NO_INSTRUMENT __cyg_profile_func_enter(void *func_addr, void *call_addr) {
    uint32_t enter_ticks = OMV_GET_TICKS_COUNT();
    uint32_t enter_cycles = OMV_GET_CYCLE_COUNT();

    if (func_addr == NULL || call_addr == NULL) {
        return;
    }

    // Skip profiling if called from IRQ context
    #if !OMV_PROFILER_IRQ_ENABLE
    if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)) {
        return;
    }
    #endif

    // Restart profiler if needed
    if (profiler.reset_pending) {
        return omv_profiler_reset();
    }

    // Update call stack
    profiler.stack_top++;
    if (profiler.stack_top >= OMV_PROFILER_STACK_DEPTH) {
        profiler.stack_top = OMV_PROFILER_STACK_DEPTH - 1;
        return;
    }

    if (profiler.stack_top > (int32_t) profiler.stack_depth) {
        profiler.stack_depth = profiler.stack_top;
    }

    profiler_stack_top(func_addr) = func_addr;
    profiler_stack_top(call_addr) = call_addr;

    profiler_stack_top(child_ticks) = 0;
    profiler_stack_top(enter_ticks) = enter_ticks;

    profiler_stack_top(child_cycles) = 0;
    profiler_stack_top(enter_cycles) = enter_cycles;

    #if __PMU_PRESENT
    for (size_t i = 0; i < __PMU_NUM_EVENTCNT; i++) {
        profiler_stack_top(child_events[i]) = 0;
        profiler_stack_top(enter_events[i]) = OMV_GET_EVENT_COUNT(i);
    }
    #endif
}

void OMV_ATTR_NO_INSTRUMENT __cyg_profile_func_exit(void *func_addr, void *call_addr) {
    uint32_t exit_ticks = OMV_GET_TICKS_COUNT();
    uint32_t exit_cycles = OMV_GET_CYCLE_COUNT();

    if (func_addr == NULL || call_addr == NULL) {
        return;
    }

    // Skip profiling if called from IRQ context
    #if !OMV_PROFILER_IRQ_ENABLE
    if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)) {
        return;
    }
    #endif

    // Restart profiler if needed
    if (profiler.reset_pending) {
        return omv_profiler_reset();
    }

    // Update events
    #if __PMU_PRESENT
    uint16_t exit_events[__PMU_NUM_EVENTCNT];
    for (size_t i = 0; i < __PMU_NUM_EVENTCNT; i++) {
        exit_events[i] = OMV_GET_EVENT_COUNT(i);
    }
    #endif

    if (profiler.stack_top < 0 || profiler.stack_top >= OMV_PROFILER_STACK_DEPTH) {
        return;
    }

    // Function address should match the stack's top.
    if (func_addr != profiler_stack_top(func_addr) ||
        call_addr != profiler_stack_top(call_addr)) {
        goto exit_cleanup;
    }

    // Protect profile data update via mutex.
    if (!mutex_try_lock(&profiler.mutex, MUTEX_TID_OMV)) {
        goto exit_cleanup;
    }

    // Calculate inclusive ticks and cycles
    uint32_t incl_ticks = exit_ticks - profiler_stack_top(enter_ticks);
    uint32_t incl_cycles = exit_cycles - profiler_stack_top(enter_cycles);

    // Calculate exclusive ticks and cycles
    uint32_t excl_ticks = incl_ticks - profiler_stack_top(child_ticks);
    uint32_t excl_cycles = incl_cycles - profiler_stack_top(child_cycles);

    // Check if this entry makes sense.
    if (incl_ticks < excl_ticks || incl_cycles < excl_cycles) {
        goto mutex_unlock;
    }

    // Get or create a new entry
    omv_profiler_data_t *entry = omv_profiler_get_entry(func_addr);
    if (entry == NULL) {
        goto mutex_unlock;
    }

    entry->call_count++;
    entry->func_addr = func_addr;
    entry->call_addr = call_addr;

    entry->total_ticks += excl_ticks;
    entry->total_cycles += excl_cycles;

    if (excl_ticks > entry->max_ticks) {
        entry->max_ticks = excl_ticks;
    }

    if (excl_ticks < entry->min_ticks) {
        entry->min_ticks = excl_ticks;
    }

    // In exclusive mode, updated the parent's child counters.
    if (profiler.stack_top && profiler.mode == OMV_PROFILER_EXCLUSIVE) {
        profiler_stack_prv(child_ticks) += incl_ticks;
        profiler_stack_prv(child_cycles) += incl_cycles;
    }

    #if __PMU_PRESENT
    for (size_t i = 0; i < __PMU_NUM_EVENTCNT; i++) {
        uint16_t incl_events = exit_events[i] - profiler_stack_top(enter_events[i]);
        uint16_t excl_events = incl_events - profiler_stack_top(child_events[i]);

        entry->total_events[i] += excl_events;

        // In exclusive mode, updated the parent's child counters.
        if (profiler.stack_top && profiler.mode == OMV_PROFILER_EXCLUSIVE) {
            profiler_stack_prv(child_events[i]) += incl_events;
        }
    }
    #endif

mutex_unlock:
    mutex_unlock(&profiler.mutex, MUTEX_TID_OMV);
exit_cleanup:
    profiler.stack_top--;
}
#endif // OMV_PROFILER_ENABLE

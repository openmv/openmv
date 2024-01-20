/*
 * Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include <string.h>
#include CMSIS_MCU_H
#include "global_map.h"
#include "dave_base.h"
#include "dave_d0lib.h"
#include "dave_registermap.h"
#include "dave_driver.h"
#include "irq.h"

#define D1_REG(index)       ((long *) GPU2D_BASE)[index]
#define D1_DEV(handle)      ((d1_device_intern *) handle)
#define D1_IRQCTL_ENABLE    (D2IRQCTL_CLR_FINISH_DLIST | D2IRQCTL_CLR_FINISH_ENUM \
                             | D2IRQCTL_ENABLE_FINISH_DLIST)
#define D1_IRQCTL_CLEAR     D1_IRQCTL_ENABLE
#define D1_IRQCTL_DISABLE   (D2IRQCTL_CLR_FINISH_DLIST | D2IRQCTL_CLR_FINISH_ENUM)

#define STRINGIFY(x)        #x
#define TOSTRING(x)         STRINGIFY(x)

#define D1_VERSION          ((D1_VERSION_MAJOR << 16) | D1_VERSION_MINOR)
#define D1_VERSION_STRING   "v" TOSTRING(D1_VERSION_MAJOR) "." TOSTRING(D1_VERSION_MINOR)

typedef struct _d1_device_intern {
    long flags;
    int dlist_indirect;
    volatile long *dlist_start;
} d1_device_intern;

static volatile d1_device_intern *s_isr_context;
static volatile unsigned int s_dlists_done = 0;

int d1_getversion() {
    return D1_VERSION;
}

const char *d1_getversionstring() {
    return D1_VERSION_STRING;
}

void d1_irq_enable() {
    // Clear all interrupts and enable DLIST IRQ
    D1_REG(D2_IRQCTL) = D1_IRQCTL_ENABLE;

    // Configure and enable GPU interrupt
    NVIC_SetPriority(GPU2D_IRQ_IRQn, IRQ_PRI_GPU);
    NVIC_ClearPendingIRQ(GPU2D_IRQ_IRQn);
    NVIC_EnableIRQ(GPU2D_IRQ_IRQn);
}

void d1_irq_disable() {
    // Clear all interrupts and disable DLIST IRQ
    D1_REG(D2_IRQCTL) = D1_IRQCTL_DISABLE;

    // Disable GPU interrupt
    NVIC_DisableIRQ(GPU2D_IRQ_IRQn);
    NVIC_ClearPendingIRQ(GPU2D_IRQ_IRQn);
}

int d1_queryirq(d1_device *handle, int irqmask, int timeout) {
    if (timeout == d1_to_no_wait) {
        return 1;
    }

    for (; timeout > 0; --timeout) {
        if (s_dlists_done) {
            --s_dlists_done;
            return GPU2D_IRQ_IRQn;
        }
    }
    return 0;
}

d1_device *d1_opendevice(long flags) {
    d1_device *handle = d1_allocmem(sizeof(d1_device_intern));

    D1_DEV(handle)->flags = flags;

    #if (D1_DLIST_INDIRECT == 1)
    D1_DEV(handle)->dlist_indirect = 1;
    #endif

    // Enable D/AVE2D clock
    REG32_SET_ONE_BIT(CLKCTL_PER_MST->PERIPH_CLK_ENA, 8);

    // Enable interrupts
    d1_irq_enable();
    s_isr_context = handle;

    return handle;
}

int d1_closedevice(d1_device *handle) {
    // Disable interrupts
    d1_irq_disable();
    s_isr_context = NULL;

    // Disable clock
    REG32_CLR_ONE_BIT(CLKCTL_PER_MST->PERIPH_CLK_ENA, 8);

    d1_freemem(handle);
    return 1;
}

int d1_devicesupported(d1_device *handle, int deviceid) {
    switch (deviceid) {
        case D1_DAVE2D:
        #if (D1_DLIST_INDIRECT == 1)
        case D1_DLISTINDIRECT:
        #endif
            return 1;
        default:
            return 0;
    }
}

long d1_getregister(d1_device *handle, int deviceid, int index) {
    switch (deviceid) {
        case D1_DAVE2D:
            return D1_REG(index);
        #if (D1_DLIST_INDIRECT == 1)
        case D1_DLISTINDIRECT:
            return D1_DEV(handle)->dlist_indirect;
        #endif
        default:
            return 0;
    }
}

void d1_setregister(d1_device *handle, int deviceid, int index, long value) {
    switch (deviceid) {
        case D1_DAVE2D:
            #if (D1_DLIST_INDIRECT == 1)
            if (index == D2_DLISTSTART && D1_DEV(handle)->dlist_indirect) {
                long *dlist_ptr = (long *) value;
                D1_DEV(handle)->dlist_start = dlist_ptr + 1;
                D1_REG(index) = *dlist_ptr;
            } else {
                D1_REG(index) = value;
            }
            #else
            D1_REG(index) = value;
            #endif
            break;
        #if (D1_DLIST_INDIRECT == 1)
        case D1_DLISTINDIRECT:
            D1_DEV(handle)->dlist_indirect = value;
        #endif
        default:
            break;
    }
}

void *d1_allocmem(unsigned int size) {
    return d0_allocmem(size);
}

void d1_freemem(void *ptr) {
    d0_freemem(ptr);
}

unsigned int d1_memsize(void *ptr) {
    return d0_memsize(ptr);
}

void *d1_allocvidmem(d1_device *handle, int memtype, unsigned int size) {
    return d0_allocvidmem(size);
}

void d1_freevidmem(d1_device *handle, int memtype, void *ptr) {
    return d0_freevidmem(ptr);
}

int d1_queryarchitecture(d1_device *handle) {
    return d1_ma_unified;
}

void *d1_maptovidmem(d1_device *handle, void *ptr) {
    return ptr;
}

void *d1_mapfromvidmem(d1_device *handle, void *ptr) {
    return ptr;
}

int d1_copytovidmem(d1_device *handle, void *dst, const void *src, unsigned int size, int flags) {
    memcpy(dst, src, size);
    return 1;
}

int d1_cacheblockflush(d1_device *handle, int memtype, const void *ptr, unsigned int size) {
    SCB_CleanDCache_by_Addr((void *) ptr, size);
    return 1;
}

void GPU2D_IRQHandler(void) {
    // Get D/AVE2D interrupt status
    long dave_status = D1_REG(D2_STATUS);

    // Clear all interrupts triggered
    D1_REG(D2_IRQCTL) = D1_IRQCTL_CLEAR;

    // Check if DLIST IRQ triggered
    if (dave_status & D2C_IRQ_DLIST) {
        #if (D1_DLIST_INDIRECT == 1)
        if (s_isr_context->dlist_indirect
            && ((void *) *s_isr_context->dlist_start) != NULL) {
            D1_REG(D2_DLISTSTART) = *s_isr_context->dlist_start;

            ++s_isr_context->dlist_start;
        }
        #endif
        ++s_dlists_done;
    }
}

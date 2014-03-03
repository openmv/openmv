#include <libmp.h>
#include "xalloc.h"
#define BREAK() __asm__ volatile ("BKPT");

void *xalloc(uint32_t size)
{
    void *mem = gc_alloc(size);
    if (mem == NULL) {
        BREAK();
    }
    return mem;
}

void *xalloc0(uint32_t size)
{
    void *mem = xalloc(size);
    if (mem) {
        memset(mem, 0, size);
    }
    return mem;
}

void xfree(void *ptr)
{
    gc_free(ptr);
}

void *xrealloc(void *ptr, uint32_t size)
{
    ptr = gc_realloc(ptr, size);
    if (ptr == NULL) {
        BREAK();
    }
    return ptr;
}


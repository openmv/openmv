#include <libmp.h>
#include "xalloc.h"
#define BREAK() __asm__ volatile ("BKPT");

void *xalloc(size_t size)
{
    void *mem = gc_alloc(size);
    if (mem == NULL) {
        BREAK();
    }
    return mem;
}

void *xalloc0(size_t size)
{
    void *mem = xalloc(size);
    if (mem) {
        bzero(mem, size);
    }
    return mem;
}

void xfree(void *ptr)
{
    gc_free(ptr);
}

void *xrealloc(void *ptr, size_t size)
{
    ptr = gc_realloc(ptr, size);
    if (ptr == NULL) {
        BREAK();
    }
    return ptr;
}


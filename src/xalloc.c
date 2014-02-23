#include <libmp.h>
#include "xalloc.h"

void *xcalloc(size_t nmemb, size_t size)
{
    void *mem = gc_alloc(nmemb*size);
    if (mem) {
        bzero(mem, nmemb*size);
    }
    return mem;
}

void *xalloc(size_t size)
{
    return gc_alloc(size);
}

void xfree(void *ptr)
{
    gc_free(ptr);
}

void *xrealloc(void *ptr, size_t size)
{
    return gc_realloc(ptr, size);
}


#include <mp.h>
#include "mdefs.h"
#include "xalloc.h"

mp_obj_t oom_interrupt;
void xalloc_init()
{
    oom_interrupt= mp_obj_new_exception_msg(&mp_type_OSError, "Out of Memory!!");
}

void *xalloc(uint32_t size)
{
    void *mem = gc_alloc(size, false);
    if (mem == NULL) {
        mp_obj_exception_clear_traceback(oom_interrupt);
        pendsv_nlr_jump(oom_interrupt);
    }
    return mem;
}

void *xalloc0(uint32_t size)
{
    void *mem = gc_alloc(size, false);
    if (mem == NULL) {
        mp_obj_exception_clear_traceback(oom_interrupt);
        pendsv_nlr_jump(oom_interrupt);
    }
    memset(mem, 0, size);
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
        mp_obj_exception_clear_traceback(oom_interrupt);
        pendsv_nlr_jump(oom_interrupt);
    }
    return ptr;
}


#include <mpconfigport.h>

#define MICROPY_NLR_RAISE_HOOK                 \
    do {                                       \
        extern void fb_alloc_free_till_mark(); \
        fb_alloc_free_till_mark();             \
    } while (0);

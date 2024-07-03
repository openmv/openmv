#include <mpconfigport.h>

#define MICROPY_NLR_RAISE_HOOK                 \
    do {                                       \
        extern void fb_alloc_free_till_mark(); \
        fb_alloc_free_till_mark();             \
    } while (0);

#define MICROPY_ENABLE_VM_ABORT             (1)
#define MICROPY_GC_SPLIT_HEAP               (1)
#define CYW43_CHIPSET_FIRMWARE_INCLUDE_FILE "lib/cyw43-driver/firmware/w4343WA1_7_45_98_102_combined.h"
#define MICROPY_BANNER_NAME_AND_VERSION "OpenMV " OPENMV_GIT_TAG "; MicroPython " MICROPY_GIT_TAG

/*
 * Builds ST's VD6G driver (drivers/sensors/vd6g, BSD-3-Clause, unmodified) for the
 * VD66GY OpenMV driver. ST's code uses assert() to consume some return values, which
 * are unused when NDEBUG is set, so that warning is relaxed for this unit only.
 */
#include "board_config.h"
#if (OMV_VD66GY_ENABLE == 1)
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#include "vd6g/vd6g.c"
#endif

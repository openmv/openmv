/*
 * vd6g_build.c -- builds ST's VD6G sensor driver (Drivers/vd6g, BSD-3-Clause, unmodified).
 *
 * vd6g.c #includes its two patch tables (vd6g_patch.c, vd6g_vtpatch.c), so the Drivers/vd6g
 * folder is excluded from the build and compiled through this file instead.
 *
 * NDEBUG: ST's driver asserts on I2C errors inside its own error-trace path (e.g. while
 * polling the sensor state during boot). With asserts live, a missing or late camera would
 * halt the firmware instead of returning an error, so they are compiled out here (as in the
 * OpenMV build). Some return values are then unused, hence the warning pragma.
 */
#define NDEBUG
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "vd6g.c"

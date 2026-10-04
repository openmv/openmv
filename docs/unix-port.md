# Unix Port

The Unix port runs OpenMV's image processing library as a host MicroPython
binary on Linux, macOS, or Windows under WSL. There's no embedded toolchain
involvement; useful for algorithm development, CI testing, and reprocessing
recorded images.

For build and run instructions see [Unix Port Build](firmware.md#unix-port-build)
in the firmware doc. This page covers what the port does and doesn't include,
its memory model, and how it integrates with MicroPython's own unix variant
mechanism.

## What's available

- Full imlib (filters, feature detection, blob analysis, find_circles /
  lines / rects, QR codes, AprilTags, data matrices, barcodes, haar
  cascades, ORB keypoints).
- Image I/O across BMP, PNG, JPEG, GIF, MJPEG.
- ulab for ndarray support.
- `umalloc` for the same allocator stats interface as embedded ports.

## What's not

- Hardware modules: sensor / CSI, display, IMU, FIR, audio, ML accelerators.
- ML inference (TFLM and STAI need the embedded build).

## Memory model

The host build replaces embedded SRAM regions with a single host-malloc'd
UMA pool, default 64MB:

- The pool is allocated at library load and registered as a generic pool
  (flags=0). UMA's `uma_pool_find()` fallback selects this pool for any
  allocation regardless of requested attributes (UMA_FAST / UMA_DTCM /
  UMA_TRANSIENT). Embedded ports tag separate pools for these; unix has
  one generic pool.
- The framebuffer uses dynamic UMA allocation; fb_alloc temporaries come
  from the same pool.
- The streaming framebuffer is registered with `enabled=false` and is
  unused on host.

For memory-hungry workloads, override the default at compile time:

```bash
make TARGET=UNIX CFLAGS_USERMOD=-DOMV_UNIX_UMA_POOL_SIZE=$((256 * 1024 * 1024))
```

The macro is defined in `boards/UNIX/board_config.h` with `#ifndef`
guards.

## Integration with MicroPython

`make TARGET=UNIX` flows through the standard top-level pipeline
(`boards/UNIX/board_config.mk` -> `ports/unix/port_config.mk`) just like
any other board. `port_config.mk` for the unix port dispatches to
MicroPython's own unix variant build by invoking
`lib/micropython/ports/unix/Makefile` with `VARIANT=openmv`,
`VARIANT_DIR=$(pwd)/boards/UNIX`, `USER_C_MODULES=$(pwd)` under
`env -i` to insulate the variant build from the embedded ARM
cross-compile flags that OpenMV's outer make exports.

OpenMV's image library and common utilities are pulled in via
`modules/micropython.mk` when `PORT=unix`. CMSIS DSP fast-math sources
are not pulled in (they require ARM CMSIS Core integration that doesn't
cleanly compile on a non-ARM host); `ports/unix/unix_compat/arm_math.h`
provides libm-backed equivalents for `arm_sin_f32` / `arm_cos_f32` and
portable C versions of the SIMD intrinsics imlib needs.

### Initialisation order

A GCC constructor in `ports/unix/py_unix_stubs.c` runs at library load
time, before `mp_init()`:

1. Idempotency guard — return immediately if already initialised.
2. Intrinsic self-check — `__USAT` and `__USAT_ASR` are tested with
   known inputs to trap parameter-order regressions in the host
   intrinsic stubs (a swapped argument order silently produces 8x
   wrong filter output).
3. Allocate and zero-fill the UMA pool, then `uma_init()` and
   `uma_pool_add(pool, size, 0)`.
4. `framebuffer_init(FB_MAINFB_ID, ..., dynamic=true, enabled=true)`
   and `framebuffer_init(FB_STREAM_ID, ..., enabled=false)`.
5. `imlib_init()`.

None of these allocate from the GC heap (which isn't initialised yet).
If any of them ever grow a GC dependency, init must be moved to a
runtime hook called from MicroPython's startup path. A matching
`__attribute__((destructor))` frees the pool on process exit so
`dlopen` / `dlclose` and `mp_init` / `mp_deinit` cycles don't leak.

The embedded `framebuffer_init0()` (which expects linker-defined
`_sb_memory_start` / `_sb_memory_end` symbols) is unused on unix; we
provide trivial char stubs only to satisfy the link step.

## Testing

The unix port shares `scripts/unittest/run.py` with the embedded build.
The host wrapper `scripts/unittest/run_tests.py` invokes the unix
binary on `run.py`, supplying paths for `tests/`, `data/`, and `temp/`:

```bash
cd scripts/unittest
python3 run_tests.py
python3 run_tests.py --filter qrcodes
```

### Skip policy

A test is reported SKIPPED if it:

1. returns the string `"skip"` from its `unittest()` function,
2. raises `ImportError` on a known optional module name (sensor, csi,
   ml, fir, audio, display, tv, imu, tof, omv), or
3. raises `Exception` with `"SKIPPED"` in the message text (e.g.
   `raise Exception("SKIPPED: feature unavailable on this build")`).

Anything else is reported as FAILED. This is deliberately strict so a
regression breaking `import image` or any other always-available module
surfaces as a real failure rather than disappearing into SKIPPED. CI
for the unix port runs as part of `.github/workflows/firmware.yml` (the
`UNIX` matrix entry) and uses the same wrapper.

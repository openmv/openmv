# OpenMV Unix port variant makefile
include $(VARIANT_DIR)/board_config.mk

# UMA collection and image operations require a single-threaded VM.
override MICROPY_PY_THREAD = 0
override MICROPY_PY_THREAD_GIL = 0
override MICROPY_PY_FFI = 0

FROZEN_MANIFEST ?= $(VARIANT_DIR)/manifest.py

# OpenMV root for the c_module() entry in the manifest.
MICROPY_MANIFEST_OMV_DIR := $(TOP_DIR)

# queue.c uses C11 atomics. CFLAGS_EXTRA comes after the -std=gnu99 of the Unix port.
CFLAGS_EXTRA += -std=gnu11

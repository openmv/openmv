# SPDX-License-Identifier: MIT
#
# Copyright (C) 2025 OpenMV, LLC.
#
# OpenMV Unix port variant makefile

# Include board configuration (sets MICROPY_PY_ULAB and other feature flags)
include $(VARIANT_DIR)/omv_boardconfig.mk

FROZEN_MANIFEST ?= $(VARIANT_DIR)/manifest.py

# Add OpenMV modules directory to include path for patched MicroPython files
# (e.g., extmod/modtime.c includes py_clock.h)
CFLAGS_EXTRA += -I$(USER_C_MODULES)/modules

# Add board config directory for imlib_config.h and other board-specific headers
CFLAGS_EXTRA += -I$(VARIANT_DIR)

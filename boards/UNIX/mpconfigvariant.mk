# SPDX-License-Identifier: MIT
#
# Copyright (C) 2025 OpenMV, LLC.
#
# OpenMV Unix port variant makefile

FROZEN_MANIFEST ?= $(VARIANT_DIR)/manifest.py

# Add OpenMV modules directory to include path for patched MicroPython files
# (e.g., extmod/modtime.c includes py_clock.h)
CFLAGS_EXTRA += -I$(USER_C_MODULES)/modules

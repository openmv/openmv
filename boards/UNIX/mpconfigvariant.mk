# SPDX-License-Identifier: MIT
#
# Copyright (C) 2026 OpenMV, LLC.

include $(VARIANT_DIR)/board_config.mk

FROZEN_MANIFEST ?= $(VARIANT_DIR)/manifest.py

# OpenMV modules need to find py_clock.h via the patched extmod/modtime.c.
CFLAGS_EXTRA += -I$(USER_C_MODULES)/modules

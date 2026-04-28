# SPDX-License-Identifier: MIT
#
# Copyright (C) 2026 OpenMV, LLC.
#
# Unix port build orchestration.
#
# `make TARGET=UNIX` flows through the standard top-level pipeline
# (boards/UNIX/board_config.mk -> ports/unix/port_config.mk) and is
# dispatched from here to MicroPython's own unix variant build.

# Run sub-makes via `env -i` to insulate them from the embedded ARM
# cross-compile flags that OpenMV's outer make exports. Forward only
# the host environment vars the variant build legitimately needs:
# PATH/HOME/USER for the toolchain, LANG/LC_ALL/TMPDIR for locale and
# scratch space, and CCACHE_* so a developer's ccache config survives.
UNIX_ENV = env -i \
    PATH="$$PATH" HOME="$$HOME" USER="$$USER" \
    LANG="$$LANG" LC_ALL="$$LC_ALL" TMPDIR="$$TMPDIR" \
    CCACHE_DIR="$$CCACHE_DIR" CCACHE_BASEDIR="$$CCACHE_BASEDIR"

UNIX_VARIANT_ARGS = \
    VARIANT=openmv \
    VARIANT_DIR=$(TOP_DIR)/boards/UNIX \
    USER_C_MODULES=$(TOP_DIR)

.PHONY: all
all:
	$(Q)git submodule update --init --depth=1 \
		lib/apriltag lib/tflm/libtflm modules/ulab
	+$(Q)cd $(MICROPY_DIR)/ports/unix && exec $(UNIX_ENV) make submodules
	+$(Q)cd $(MICROPY_DIR)/mpy-cross && exec $(UNIX_ENV) make
	+$(Q)cd $(MICROPY_DIR)/ports/unix && exec $(UNIX_ENV) make $(UNIX_VARIANT_ARGS)

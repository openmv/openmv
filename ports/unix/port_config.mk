# SPDX-License-Identifier: MIT
#
# Copyright (C) 2026 OpenMV, LLC.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# The MicroPython Unix makefiles set the host compiler and linker options.
MPY_CROSS_DIR := $(TOP_DIR)/$(MICROPY_DIR)/mpy-cross
MPY_CROSS_BUILD := $(BUILD)/mpy-cross
MPY_CROSS := $(MPY_CROSS_BUILD)/mpy-cross
MPY_UNIX_DIR := $(TOP_DIR)/$(MICROPY_DIR)/ports/unix

# C modules come from the board manifest.
unexport USER_C_MODULES

# Tools set in the environment or on the command line replace the MicroPython defaults.
OMV_HOST_TOOL_ARGS := $(foreach tool,CC CXX CPP AS LD AR STRIP SIZE PYTHON,$(if $(filter environment command line,$(origin $(tool))),$(tool)="$($(tool))"))

# MicroPython does not rebuild objects when build options change. A changed
# option set discards the previous Unix objects, which would otherwise be
# reused or fail to link.
OMV_UNIX_CONFIG := DEBUG=$(if $(filter 1,$(DEBUG)),1,0) $(foreach feature,$(OMV_UNIX_FEATURES),$(feature)=$($(feature)))
OMV_UNIX_STAMP := $(BUILD)/omv_unix_config

# mpy-cross is built without the OpenMV variant, manifest and user modules.
all:
	$(Q)if [ "$$(cat $(OMV_UNIX_STAMP) 2>/dev/null)" != "$(OMV_UNIX_CONFIG)" ]; then \
		if [ -d $(BUILD) ]; then find $(BUILD) -mindepth 1 -maxdepth 1 ! -name mpy-cross -exec rm -rf {} +; fi; \
		mkdir -p $(BUILD) && echo "$(OMV_UNIX_CONFIG)" > $(OMV_UNIX_STAMP); \
	fi
	$(ECHO) "Building mpy-cross"
	+$(Q)$(MAKE) -C $(MPY_CROSS_DIR) BUILD=$(MPY_CROSS_BUILD) \
		FROZEN_MANIFEST= USER_C_MODULES= VARIANT_DIR= MICROPY_MPYCROSS= CFLAGS_EXTRA= \
		$(if $(filter 1,$(DEBUG)),DEBUG=1,DEBUG=) $(OMV_HOST_TOOL_ARGS)
	$(ECHO) "Building the Unix port"
	+$(Q)$(MAKE) -C $(MPY_UNIX_DIR) \
		MICROPY_MPYCROSS=$(abspath $(MPY_CROSS)) \
		VARIANT_DIR=$(OMV_BOARD_CONFIG_DIR) \
		$(if $(filter 1,$(DEBUG)),DEBUG=1,DEBUG=) BUILD=$(BUILD) \
		$(foreach feature,$(OMV_UNIX_FEATURES),$(feature)=$($(feature))) \
		$(OMV_HOST_TOOL_ARGS)
	$(MKDIR) -p $(FW_DIR)
	$(Q)cp $(BUILD)/micropython $(FW_DIR)/micropython
	$(ECHO) "Binary: $(FW_DIR)/micropython"

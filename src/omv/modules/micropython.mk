# SPDX-License-Identifier: MIT
#
# Copyright (C) 2013-2024 OpenMV, LLC.
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

# Add OpenMV common modules.
OMV_MOD_DIR := $(USERMOD_DIR)
SRC_USERMOD += $(wildcard $(OMV_MOD_DIR)/*.c)
SRC_USERMOD_CXX += $(wildcard $(OMV_MOD_DIR)/*.cpp)

# Add OpenMV port-specific modules.
OMV_PORT_MOD_DIR := $(OMV_MOD_DIR)/../ports/$(PORT)/modules
SRC_USERMOD += $(wildcard $(OMV_PORT_MOD_DIR)/*.c)
SRC_USERMOD_CXX += $(wildcard $(OMV_PORT_MOD_DIR)/*.cpp)

# Extra module flags.
CFLAGS_USERMOD += \
        -I$(OMV_MOD_DIR) \
        -I$(OMV_PORT_MOD_DIR) \
        -Wno-float-conversion

CXXFLAGS_USERMOD += \
        $(CFLAGS_USERMOD) \
        -std=c++11 \
        -fno-rtti \
        -fno-exceptions \
        -fno-use-cxa-atexit \
        -nodefaultlibs \
        -fno-unwind-tables \
        -fpermissive \
        -fno-threadsafe-statics \
        -fmessage-length=0 \
        $(filter-out -std=gnu99,$(CFLAGS))

# Add CubeAI module if enabled.
ifeq ($(MICROPY_PY_CUBEAI), 1)
SRC_USERMOD += $(OMV_MOD_DIR)/../../stm32cubeai/py_st_nn.c
endif

ifeq ($(MICROPY_PY_ULAB), 1)
# NOTE: overrides USERMOD_DIR
# Workaround to build and link ulab.
USERMOD_DIR := $(USERMOD_DIR)/ulab/code
include $(USERMOD_DIR)/micropython.mk
endif

ifeq ($(DEBUG), 0)
# Use a higher optimization level for user C modules.
$(BUILD)/modules/%.o: override CFLAGS += $(USERMOD_OPT)
endif

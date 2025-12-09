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

# Filter out CSI modules if disabled
ifneq ($(MICROPY_PY_CSI),1)
SRC_USERMOD := $(filter-out $(OMV_MOD_DIR)/py_csi.c, $(SRC_USERMOD))
endif
ifneq ($(MICROPY_PY_CSI_NG),1)
SRC_USERMOD := $(filter-out $(OMV_MOD_DIR)/py_csi_ng.c, $(SRC_USERMOD))
endif

# Add OpenMV port-specific modules.
OMV_PORT_MOD_DIR := $(OMV_MOD_DIR)/../ports/$(PORT)/modules
SRC_USERMOD += $(wildcard $(OMV_PORT_MOD_DIR)/*.c)
SRC_USERMOD_CXX += $(wildcard $(OMV_PORT_MOD_DIR)/*.cpp)

# Add Unix port-specific source files
ifeq ($(PORT),unix)
OMV_PORT_DIR := $(OMV_MOD_DIR)/../ports/$(PORT)
OMV_LIB_IMLIB_DIR := $(OMV_MOD_DIR)/../lib/imlib
OMV_COMMON_DIR := $(OMV_MOD_DIR)/../common

# Define TOP_DIR for common.mk and imlib.mk (they use $(TOP_DIR)/common and $(TOP_DIR)/lib/imlib)
# OMV_MOD_DIR is the modules directory, so TOP_DIR is one level up
TOP_DIR := $(OMV_MOD_DIR)/..

SRC_USERMOD += $(wildcard $(OMV_PORT_DIR)/*.c)

# Add CMSIS DSP library for Unix port to match ARM precision
CMSIS_DIR := $(OMV_MOD_DIR)/../lib/cmsis

# Fast math functions (portable C implementations)
SRC_USERMOD += $(CMSIS_DIR)/src/dsp/FastMathFunctions/arm_cos_f32.c
SRC_USERMOD += $(CMSIS_DIR)/src/dsp/FastMathFunctions/arm_sin_f32.c
SRC_USERMOD += $(CMSIS_DIR)/src/dsp/FastMathFunctions/arm_atan2_f32.c
SRC_USERMOD += $(CMSIS_DIR)/src/dsp/FastMathFunctions/arm_vexp_f32.c
SRC_USERMOD += $(CMSIS_DIR)/src/dsp/FastMathFunctions/arm_vlog_f32.c

# Common tables (sine/cosine lookup tables) - needed by sin/cos functions
SRC_USERMOD += $(CMSIS_DIR)/src/dsp/CommonTables/CommonTables.c

# Include common and imlib file lists
include $(OMV_COMMON_DIR)/common.mk
include $(OMV_LIB_IMLIB_DIR)/imlib.mk

# Add OpenMV common utilities for Unix port
# Filter out hardware-specific files
UNIX_COMMON_EXCLUDE := dma_alloc.c mp_utils.c nosys_stubs.c omv_csi.c \
                       omv_profiler.c pendsv.c tinyusb_debug.c trace.c unaligned_memcpy.c \
                       usbdbg.c vospi.c
UNIX_COMMON_SRC := $(filter-out $(UNIX_COMMON_EXCLUDE), $(COMMON_SRC_C))
SRC_USERMOD += $(addprefix $(OMV_COMMON_DIR)/, $(UNIX_COMMON_SRC))

# Add OpenMV image library for Unix port
# Include fmath.c to match embedded behavior (fast approximations)
SRC_USERMOD += $(addprefix $(OMV_LIB_IMLIB_DIR)/, $(IMLIB_SRC_C))

CFLAGS_USERMOD += \
    -I$(OMV_PORT_DIR) \
    -I$(OMV_PORT_DIR)/unix_compat \
    -I$(OMV_LIB_IMLIB_DIR) \
    -I$(OMV_MOD_DIR) \
    -I$(OMV_MOD_DIR)/.. \
    -I$(OMV_MOD_DIR)/../common \
    -I$(TOP)/py \
    -I$(CMSIS_DIR)/include \
    -I$(CMSIS_DIR)/include/dsp \
    -DUNIX \
    -DARM_MATH_CM7 \
    -DMICROPY_PY_CRC=1 \
    -Wno-unused-parameter \
    -Wno-missing-field-initializers \
    -Wno-cpp \
    -Wno-incompatible-pointer-types \
    -Wno-sign-compare \
    -Wno-float-conversion

# Force-include Unix compatibility headers
# Note: fmath.h not included as imlib provides its own implementation
CFLAGS_USERMOD += \
    -include $(OMV_PORT_DIR)/unix_compat/omv_force_include.h \
    -include $(OMV_PORT_DIR)/unix_compat/mp_compat.h
endif

# Extra module flags.
CFLAGS_USERMOD += \
        -std=gnu11 \
        -I$(OMV_MOD_DIR) \
        -I$(OMV_PORT_MOD_DIR) \
        -Wno-float-conversion

CXXFLAGS_USERMOD += \
        $(CFLAGS_USERMOD) \
        -std=gnu++11 \
        -fno-rtti \
        -fno-exceptions \
        -fno-use-cxa-atexit \
        -nodefaultlibs \
        -fno-unwind-tables \
        -fpermissive \
        -fno-threadsafe-statics \
        -fmessage-length=0 \
        $(filter-out -std=gnu11,$(CFLAGS))

# Add CubeAI module if enabled.
ifeq ($(MICROPY_PY_CUBEAI), 1)
SRC_USERMOD += $(OMV_MOD_DIR)/../../stm32cubeai/py_st_nn.c
endif

ifeq ($(strip $(MICROPY_PY_ULAB)), 1)
# NOTE: overrides USERMOD_DIR
# Workaround to build and link ulab.
USERMOD_DIR := $(USERMOD_DIR)/ulab/code
include $(USERMOD_DIR)/micropython.mk
endif

ifeq ($(DEBUG), 0)
# Use a higher optimization level for user C modules.
$(BUILD)/modules/%.o: override CFLAGS += $(USERMOD_OPT)
endif

ifeq ($(PROFILE_ENABLE), 1)
$(BUILD)/modules/py_ml.o: override CFLAGS += -finstrument-functions
$(BUILD)/modules/py_image.o: override CFLAGS += -finstrument-functions
$(BUILD)/modules/ulab/%.o: override CFLAGS += -finstrument-functions
endif

# Unix port: Suppress warnings that MicroPython's build system enables
ifeq ($(PORT), unix)
$(BUILD)/modules/%.o: override CFLAGS += -Wno-float-conversion -Wno-double-promotion -Wno-type-limits -Wno-absolute-value
$(BUILD)/modules/../lib/imlib/%.o: override CFLAGS += -Wno-float-conversion -Wno-double-promotion -Wno-type-limits -Wno-absolute-value -Wno-old-style-declaration -Wno-shift-negative-value -Wno-implicit-fallthrough -Wno-unused-but-set-parameter -Wno-empty-body -Wno-pointer-to-int-cast
# fmath.c uses type punning which triggers strict-aliasing warnings
$(BUILD)/modules/../lib/imlib/fmath.o: override CFLAGS += -Wno-strict-aliasing -Wno-uninitialized
# CMSIS DSP files: Don't force-include Unix compat headers (causes macro conflicts with CMSIS headers)
# Instead let CMSIS use its own defines
$(BUILD)/modules/../lib/cmsis/%.o: override CFLAGS += -Wno-unused-parameter -Wno-unknown-pragmas -Wno-unused-variable -Wno-maybe-uninitialized -Wno-unused-but-set-variable -U__WFI -U__WFE -U__SEV -U__NOP -U__DSB -U__ISB -U__DMB
endif


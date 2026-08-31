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

# Unix variant build. The embedded per-port port_config.mk pipeline is
# bypassed for unix; pull imlib, common, apriltag and protocol in via
# the user-C-modules mechanism instead.
ifeq ($(PORT),unix)
OMV_PORT_DIR := $(OMV_MOD_DIR)/../ports/$(PORT)
OMV_LIB_IMLIB_DIR := $(OMV_MOD_DIR)/../lib/imlib
OMV_COMMON_DIR := $(OMV_MOD_DIR)/../common
OMV_APRILTAG_DIR := $(OMV_MOD_DIR)/../lib/apriltag
TOP_DIR := $(OMV_MOD_DIR)/..

SRC_USERMOD += $(wildcard $(OMV_PORT_DIR)/*.c)

include $(OMV_COMMON_DIR)/common.mk
include $(OMV_LIB_IMLIB_DIR)/imlib.mk
include $(OMV_APRILTAG_DIR)/apriltag.mk

# Hardware-only common sources excluded from the host build.
UNIX_COMMON_EXCLUDE := mp_utils.c nosys_stubs.c omv_csi.c omv_cycles.c \
                       omv_i2c.c omv_profiler.c pendsv.c trace.c vospi.c
UNIX_COMMON_SRC := $(filter-out $(UNIX_COMMON_EXCLUDE),$(COMMON_SRC_C))
SRC_USERMOD += $(addprefix $(OMV_COMMON_DIR)/,$(UNIX_COMMON_SRC))
SRC_USERMOD += $(addprefix $(OMV_LIB_IMLIB_DIR)/,$(IMLIB_SRC_C))
SRC_USERMOD += $(addprefix $(OMV_APRILTAG_DIR)/,$(APRILTAG_SRC_C))

CFLAGS_USERMOD += \
    -I$(OMV_MOD_DIR) \
    -I$(OMV_MOD_DIR)/.. \
    -I$(OMV_LIB_IMLIB_DIR) \
    -I$(OMV_COMMON_DIR) \
    -I$(OMV_APRILTAG_DIR) \
    -I$(OMV_PORT_DIR) \
    -I$(OMV_PORT_DIR)/unix_compat \
    -I$(TOP_DIR)/lib/micropython/py \
    -I$(TOP_DIR)/protocol \
    -Wno-unused-parameter \
    -Wno-missing-field-initializers \
    -Wno-cpp \
    -Wno-sign-compare \
    -Wno-float-conversion

# Convert board_config.mk MICROPY_PY_* / MICROPY_SSL_* flags to defines
# so the host build sees the same conditionals the embedded build does.
# Mirrors the list propagated by common/micropy.mk.
UNIX_MPY_FLAGS := \
    MICROPY_PY_AUDIO \
    MICROPY_PY_BTREE \
    MICROPY_PY_CRC \
    MICROPY_PY_CSI \
    MICROPY_PY_CSI_NG \
    MICROPY_PY_DISPLAY \
    MICROPY_PY_FIR \
    MICROPY_PY_IMU \
    MICROPY_PY_ML \
    MICROPY_PY_ML_STAI \
    MICROPY_PY_ML_TFLM \
    MICROPY_PY_SSL \
    MICROPY_PY_TOF \
    MICROPY_PY_TV \
    MICROPY_PY_ULAB \
    MICROPY_PY_UMALLOC \
    MICROPY_PY_UNITTEST \
    MICROPY_PY_WINC1500 \
    MICROPY_SSL_MBEDTLS
$(foreach f,$(UNIX_MPY_FLAGS),$(if $(filter 1,$($(f))),$(eval CFLAGS_USERMOD += -D$(f)=1)))

# Force-include the unix compat shims into every translation unit so
# imlib's CMSIS / cmsis_gcc.h / fmath.h references resolve on the host.
CFLAGS_USERMOD += \
    -include $(OMV_PORT_DIR)/unix_compat/omv_force_include.h \
    -include $(OMV_PORT_DIR)/unix_compat/arm_math.h \
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

ifeq ($(MICROPY_PY_ULAB), 1)
# NOTE: overrides USERMOD_DIR
# Workaround to build and link ulab.
USERMOD_DIR := $(USERMOD_DIR)/ulab/code
include $(USERMOD_DIR)/micropython.mk
endif

# Add unit test modules if enabled.
ifeq ($(MICROPY_PY_UNITTEST), 1)
SRC_USERMOD += $(wildcard $(OMV_MOD_DIR)/unittests/*.c)
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

# Unix port: suppress warnings that MicroPython's stricter unix-port
# build flags would otherwise turn into noise across third-party imlib
# code (zbar, dmtx, haar) which was written for embedded compilers.
ifeq ($(PORT),unix)
$(BUILD)/modules/%.o: override CFLAGS += -Wno-float-conversion -Wno-double-promotion -Wno-type-limits -Wno-absolute-value
# Third-party imlib code (zbar, dmtx, haar) was written for embedded
# compilers and trips a wider set of warnings on the host.
$(BUILD)/modules/../lib/imlib/%.o: override CFLAGS += -Wno-float-conversion -Wno-double-promotion -Wno-type-limits -Wno-absolute-value -Wno-old-style-declaration -Wno-shift-negative-value -Wno-implicit-fallthrough -Wno-unused-but-set-parameter -Wno-empty-body -Wno-pointer-to-int-cast -Wno-incompatible-pointer-types
# fmath.c uses type punning that triggers strict-aliasing warnings.
$(BUILD)/modules/../lib/imlib/fmath.o: override CFLAGS += -Wno-strict-aliasing -Wno-uninitialized
# AprilTag upstream code raises a slightly different set of warnings.
$(BUILD)/modules/../lib/apriltag/%.o: override CFLAGS += -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-missing-braces -Wno-unused-function -Wno-format -Wno-double-promotion -Wno-float-conversion
endif

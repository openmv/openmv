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

# Add Unix port-specific source files
ifeq ($(PORT),unix)
OMV_PORT_DIR := $(OMV_MOD_DIR)/../ports/$(PORT)
OMV_LIB_IMLIB_DIR := $(OMV_MOD_DIR)/../lib/imlib
OMV_COMMON_DIR := $(OMV_MOD_DIR)/../common

SRC_USERMOD += $(wildcard $(OMV_PORT_DIR)/*.c)

# Add OpenMV common utilities for Unix port
SRC_USERMOD += $(OMV_COMMON_DIR)/array.c \
               $(OMV_COMMON_DIR)/umm_malloc.c \
               $(OMV_COMMON_DIR)/file_utils.c \
               $(OMV_COMMON_DIR)/mutex.c \
               $(OMV_COMMON_DIR)/queue.c

# Add OpenMV image library for Unix port
SRC_USERMOD += $(addprefix $(OMV_LIB_IMLIB_DIR)/, \
    agast.c                   \
    apriltag.c                \
    bayer.c                   \
    binary.c                  \
    blob.c                    \
    bmp.c                     \
    clahe.c                   \
    collections.c             \
    dmtx.c                    \
    draw.c                    \
    edge.c                    \
    eye.c                     \
    fast.c                    \
    fft.c                     \
    filter.c                  \
    font.c                    \
    framebuffer.c             \
    fsort.c                   \
    gif.c                     \
    haar.c                    \
    hog.c                     \
    hough.c                   \
    imlib.c                   \
    integral.c                \
    integral_mw.c             \
    isp.c                     \
    jpegd.c                   \
    jpege.c                   \
    kmeans.c                  \
    lab_tab.c                 \
    lbp.c                     \
    line.c                    \
    lodepng.c                 \
    lsd.c                     \
    mathop.c                  \
    mjpeg.c                   \
    orb.c                     \
    phasecorrelation.c        \
    png.c                     \
    point.c                   \
    ppm.c                     \
    qrcode.c                  \
    qsort.c                   \
    rainbow_tab.c             \
    rectangle.c               \
    selective_search.c        \
    sincos_tab.c              \
    stats.c                   \
    stereo.c                  \
    template.c                \
    xyz_tab.c                 \
    yuv.c                     \
    zbar.c                    \
)

CFLAGS_USERMOD += \
    -I$(OMV_PORT_DIR) \
    -I$(OMV_PORT_DIR)/unix_compat \
    -I$(OMV_LIB_IMLIB_DIR) \
    -I$(OMV_MOD_DIR) \
    -I$(OMV_MOD_DIR)/.. \
    -I$(OMV_MOD_DIR)/../common \
    -I$(OMV_MOD_DIR)/../boards/UNIX \
    -I$(OMV_MOD_DIR)/../lib/micropython \
    -I$(OMV_MOD_DIR)/../lib/micropython/py \
    -DUNIX \
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

ifeq ($(PROFILE_ENABLE), 1)
$(BUILD)/modules/py_ml.o: override CFLAGS += -finstrument-functions
$(BUILD)/modules/py_image.o: override CFLAGS += -finstrument-functions
$(BUILD)/modules/ulab/%.o: override CFLAGS += -finstrument-functions
endif

# Unix port: Suppress warnings that MicroPython's build system enables
ifeq ($(PORT), unix)
$(BUILD)/modules/%.o: override CFLAGS += -Wno-float-conversion -Wno-double-promotion -Wno-type-limits -Wno-absolute-value
$(BUILD)/modules/../lib/imlib/%.o: override CFLAGS += -Wno-float-conversion -Wno-double-promotion -Wno-type-limits -Wno-absolute-value -Wno-old-style-declaration -Wno-shift-negative-value -Wno-implicit-fallthrough
endif


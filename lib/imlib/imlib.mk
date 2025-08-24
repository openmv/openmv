# SPDX-License-Identifier: MIT
#
# Copyright (C) 2025 OpenMV, LLC.
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
#
# IMLIB Makefile

IMLIB_SRC_C += \
    agast.c \
    apriltag.c \
    bayer.c \
    binary.c \
    blob.c \
    bmp.c \
    clahe.c \
    collections.c \
    dmtx.c \
    draw.c \
    edge.c \
    eye.c \
    fast.c \
    fft.c \
    filter.c \
    fmath.c \
    font.c \
    framebuffer.c \
    fsort.c \
    gif.c \
    haar.c \
    hog.c \
    hough.c \
    imlib.c \
    integral.c \
    integral_mw.c \
    isp.c \
    jpegd.c \
    jpege.c \
    kmeans.c \
    lab_tab.c \
    lbp.c \
    line.c \
    lodepng.c \
    lsd.c \
    mathop.c \
    mjpeg.c \
    orb.c \
    phasecorrelation.c \
    png.c \
    point.c \
    ppm.c \
    qrcode.c \
    qsort.c \
    rainbow_tab.c \
    rectangle.c \
    selective_search.c \
    sincos_tab.c \
    stats.c \
    stereo.c \
    template.c \
    xyz_tab.c \
    yuv.c \
    zbar.c \

CFLAGS += -I$(TOP_DIR)/lib/imlib
$(BUILD)/lib/imlib/fmath.o: override CFLAGS += -fno-strict-aliasing

ifeq ($(CLANG_ENABLE),1)
OMV_CLANG_OBJ = $(BUILD)/lib/imlib/bayer.o
endif

# Enable instrumentation.
ifeq ($(PROFILE_ENABLE), 1)
$(BUILD)/lib/imlib/%.o: override CFLAGS += -finstrument-functions
# Clang does not support -finstrument-functions-exclude-file-list.
$(OMV_CLANG_OBJ): override CFLAGS := $(filter-out -finstrument-functions-exclude-file-list=%,$(CFLAGS))
endif

OMV_FIRM_OBJ += $(addprefix $(BUILD)/lib/imlib/, $(IMLIB_SRC_C:.c=.o))

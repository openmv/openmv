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
#
# OpenMV sources compiled by MicroPython's Unix user-module rules.
OMV_PORT_DIR := $(OMV_MOD_DIR)/../ports/$(PORT)
OMV_LIB_IMLIB_DIR := $(OMV_MOD_DIR)/../lib/imlib
OMV_COMMON_DIR := $(OMV_MOD_DIR)/../common
TOP_DIR := $(OMV_MOD_DIR)/..

SRC_USERMOD += $(wildcard $(OMV_PORT_DIR)/*.c)

include $(OMV_COMMON_DIR)/common.mk
include $(OMV_LIB_IMLIB_DIR)/imlib.mk

UNIX_COMMON_EXCLUDE := mp_utils.c omv_csi.c omv_i2c.c omv_profiler.c pendsv.c trace.c vospi.c
SRC_USERMOD += $(addprefix $(OMV_COMMON_DIR)/, $(filter-out $(UNIX_COMMON_EXCLUDE),$(COMMON_SRC_C)))
SRC_USERMOD += $(addprefix $(OMV_LIB_IMLIB_DIR)/, $(IMLIB_SRC_C))

# Paths relative to the repository root resolve through the modules vpath and give
# objects the $(BUILD)/lib/apriltag names that apriltag.mk uses for its flags.
include $(TOP_DIR)/lib/apriltag/apriltag.mk
SRC_USERMOD += $(addprefix $(APRILTAG_DIR)/, $(APRILTAG_SRC_C))

CFLAGS_USERMOD += \
    -I$(OMV_PORT_DIR) \
    -I$(OMV_PORT_DIR)/unix_compat \
    -I$(TOP_DIR)/lib/cmsis/include \
    -I$(OMV_LIB_IMLIB_DIR) \
    -I$(OMV_COMMON_DIR) \
    -I$(TOP_DIR) \
    -I$(TOP)/py \
    -I$(TOP_DIR)/protocol \
    -DOMV_OS_UNIX=1 \
    -DAPRILTAG_HAVE_CONFIG \
    -DCMSIS_MCU_H='<stdint.h>'

# Feature switches reach the compiler as definitions, as common/micropy.mk does for embedded ports.
CFLAGS_USERMOD += $(foreach feature,$(OMV_UNIX_FEATURES),-D$(feature)=$($(feature)))

# The CMSIS non-DSP __USAD8/__USADA8 fallbacks apply abs() to unsigned differences.
CFLAGS_USERMOD += -Wno-absolute-value

# MicroPython enables extra numeric conversion warnings for host code.
# Image operations intentionally mix integer pixels and floating-point math.
$(BUILD)/modules/%.o: override CFLAGS += -Wno-float-conversion -Wno-double-promotion
# imlib lookup tables use valid const static qualifier ordering.
$(BUILD)/modules/../lib/imlib/%.o: override CFLAGS += \
    -Wno-float-conversion -Wno-double-promotion -Wno-sign-compare \
    -Wno-old-style-declaration
# AprilTag mixes pixel indices with floating-point geometry; its own rules cover double promotion.
$(BUILD)/lib/apriltag/%.o: override CFLAGS += -Wno-float-conversion

# Image bindings compare validated signed dimensions with unsigned lengths.
$(addprefix $(BUILD)/modules/,py_gif.o py_helper.o py_image.o py_imageio.o): override CFLAGS += -Wno-sign-compare
# The legacy image stream bpp check compares an unsigned word with zero.
$(BUILD)/modules/py_imageio.o: override CFLAGS += -Wno-type-limits
# The cascade loader aligns its buffer with a 32-bit pointer cast; only the low bits matter.
$(BUILD)/modules/../lib/imlib/haar.o: override CFLAGS += -Wno-pointer-to-int-cast
# UMA aligns nonnegative size_t values against a positive integer constant.
$(BUILD)/modules/../common/umalloc.o: override CFLAGS += -Wno-sign-compare
# Required mp_arg_t entries intentionally leave the unused default zeroed.
$(BUILD)/modules/py_image.o: override CFLAGS += -Wno-missing-field-initializers
# DataMatrix keeps an unused diagnostic style and compiles fprintf to an empty statement.
$(BUILD)/modules/../lib/imlib/dmtx.o: override CFLAGS += -Wno-unused-but-set-parameter
# Compiled-out vendor diagnostics leave empty assert/statistics/printing branches.
$(addprefix $(BUILD)/modules/../lib/imlib/,dmtx.o zbar.o): override CFLAGS += -Wno-empty-body
# Eye gradient vectors initialize the used x/y fields and leave other fields zero.
$(BUILD)/modules/../lib/imlib/eye.o: override CFLAGS += -Wno-missing-field-initializers
# Vendor range checks compare values against limits of their own types.
$(addprefix $(BUILD)/modules/../lib/imlib/,clahe.o zbar.o): override CFLAGS += -Wno-type-limits

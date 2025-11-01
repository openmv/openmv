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
# Unix port configuration

# Unix port builds through MicroPython's Unix port infrastructure
# This file provides Unix-specific build configuration

# Compiler Flags for Unix
CFLAGS += -std=gnu11 \
          -Wall \
          -Warray-bounds \
          -Wdouble-promotion \
          -Wfloat-conversion \
          -Wno-unused-parameter \
          -Wno-missing-field-initializers \
          -I$(OMV_BOARD_CONFIG_DIR) \
          -I$(OMV_PORT_DIR) \
          -I$(OMV_PORT_DIR)/unix_compat \
          -DUNIX

# Unix-specific includes (compatibility wrappers)
CFLAGS += -include $(OMV_PORT_DIR)/unix_compat/omv_force_include.h
CFLAGS += -include $(OMV_PORT_DIR)/unix_compat/mp_compat.h
CFLAGS += -include $(OMV_PORT_DIR)/unix_compat/fmath.h

# Unix port source files
PORT_SRC_C += $(wildcard $(OMV_PORT_DIR)/*.c)

# Note: Unix port delegates actual building to MicroPython's Unix port
# This configuration is used when building via OpenMV's top-level Makefile

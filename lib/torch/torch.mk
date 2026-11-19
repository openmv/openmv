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
# PyTorch (ExecuTorch) Makefile

ifeq ($(MICROPY_PY_ML_TORCH), 1)
TORCH_DIR = $(TOP_DIR)/lib/torch/libtorch
TORCH_SRC_CC = torch_backend.cc

# These must match the flags libtorch was built with.
$(BUILD)/lib/torch/torch_backend.o: CXXFLAGS = \
    $(filter-out -std=gnu99 -std=gnu11,$(CFLAGS)) \
    -std=gnu++17 \
    -fno-rtti \
    -fno-exceptions \
    -fno-use-cxa-atexit \
    -nodefaultlibs \
    -fno-unwind-tables \
    -fno-threadsafe-statics \
    -Wno-double-promotion \
    -DC10_USING_CUSTOM_GENERATED_MACROS \
    -DET_LOG_ENABLED=0 \
    -DET_ENABLE_PROGRAM_VERIFICATION=1 \
    -DET_ENABLE_DEPRECATED_CONSTANT_BUFFER=0 \
    -I$(TORCH_DIR)/include

OMV_FIRM_OBJ += $(addprefix $(BUILD)/lib/torch/, $(TORCH_SRC_CC:.cc=.o))
ifneq ($(filter -DETHOS_U,$(CFLAGS)),)
LIBS += $(TORCH_DIR)/lib/libtorch-$(CPU)-ethosu.a
else
LIBS += $(TORCH_DIR)/lib/libtorch-$(CPU).a
endif
endif

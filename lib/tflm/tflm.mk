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
# TFLM Makefile

ifeq ($(MICROPY_PY_ML_TFLM), 1)
TFLM_SRC_CC = tflm_backend.cc

$(BUILD)/lib/tflm/tflm_backend.o: CXXFLAGS = \
    $(filter-out -std=gnu99 -std=gnu11,$(CFLAGS)) \
    -std=c++11 \
    -fno-rtti \
    -fno-exceptions \
    -fno-use-cxa-atexit \
    -nodefaultlibs \
    -fno-unwind-tables \
    -fpermissive \
    -fmessage-length=0 \
    -fno-threadsafe-statics \
    -Wno-double-promotion \
    -Wno-float-conversion \
    -DTF_LITE_STATIC_MEMORY \
    -DTF_LITE_DISABLE_X86_NEON \
    -DKERNELS_OPTIMIZED_FOR_SPEED \
    -DTF_LITE_STRIP_ERROR_STRINGS \
    -I$(TOP_DIR)/lib/tflm/libtflm/include \
    -I$(TOP_DIR)/lib/tflm/libtflm/include/third_party \
    -I$(TOP_DIR)/lib/tflm/libtflm/include/third_party/gemmlowp \
    -I$(TOP_DIR)/lib/tflm/libtflm/include/third_party/flatbuffers/include \
    -I$(TOP_DIR)/lib/tflm/libtflm/include/third_party/ethos_u_core_driver/include

OMV_CFLAGS += -I$(TOP_DIR)/lib/tflm/libtflm/include
OMV_CFLAGS += -I$(TOP_DIR)/lib/tflm/libtflm/include/third_party/ethos_u_core_driver/include
OMV_FIRM_OBJ += $(addprefix $(BUILD)/lib/tflm/, $(TFLM_SRC_CC:.cc=.o))
endif

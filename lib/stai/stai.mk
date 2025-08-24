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
# STAI Makefile

ifeq ($(MICROPY_PY_ML_STAI), 1)
STAI_SRC_C += \
    stai_backend.c \
    libstai/ll_aton/ai_reloc_network.c \
	libstai/ll_aton/ecloader.c \
	libstai/ll_aton/ll_aton.c \
	libstai/ll_aton/ll_aton_cipher.c \
	libstai/ll_aton/ll_aton_dbgtrc.c \
	libstai/ll_aton/ll_aton_debug.c \
	libstai/ll_aton/ll_aton_lib.c \
	libstai/ll_aton/ll_aton_lib_sw_operators.c \
	libstai/ll_aton/ll_aton_profiler.c \
	libstai/ll_aton/ll_aton_reloc_network.c \
	libstai/ll_aton/ll_aton_rt_main.c \
	libstai/ll_aton/ll_aton_runtime.c \
	libstai/ll_aton/ll_aton_util.c \
	libstai/ll_aton/ll_sw_float.c \
	libstai/ll_aton/ll_sw_integer.c \
	libstai/ll_aton/mcu_cache.c \
	libstai/ll_aton/npu_cache.c \

STAI_CFLAGS += \
    -DSTM32N657xx \
    -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 \
    -DLL_ATON_OSAL=LL_ATON_OSAL_BARE_METAL \
    -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC \
    -DLL_ATON_SW_FALLBACK \
    -DLL_ATON_RT_RELOC \
    -DLL_ATON_EB_DBG_INFO \
    -DLL_ATON_DBG_BUFFER_INFO_EXCLUDED=1 \
    -DLL_ATON_DUMP_DEBUG_API

STAI_CFLAGS += \
    -std=gnu99 \
    -mno-thumb-interwork \
    -ffast-math \
    -fsingle-precision-constant \
    -fvisibility=hidden \
    -fno-unwind-tables \
    -fstack-reuse=all \
    -ffunction-sections \
    -fdata-sections \
    -fno-math-errno \
    -fomit-frame-pointer

$(BUILD)/lib/stai/%.o: override CFLAGS += \
        $(STAI_CFLAGS)

$(BUILD)/lib/stai/libstai/ll_aton/%.o: override CFLAGS += \
        -Wno-format \
        -Wno-unused-variable \
        -Wno-dangling-pointer \
        -Wno-incompatible-pointer-types \
        -Wno-double-promotion \
        $(STAI_CFLAGS) \

# Enable instrumentation.
ifeq ($(PROFILE_ENABLE), 1)
$(BUILD)/lib/stai/stai_backend.o: override CFLAGS += -finstrument-functions
endif

OMV_CFLAGS += -I$(TOP_DIR)/lib/stai/libstai/include
OMV_FIRM_OBJ += $(addprefix $(BUILD)/lib/stai/, $(STAI_SRC_C:.c=.o))
endif


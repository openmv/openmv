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
# CMSIS Makefile

CMSIS_INC ?= $(PORT)

ifneq ($(STARTUP),)
CMSIS_SRC_S += cmsis/src/$(STARTUP).s
endif
ifneq ($(SYSTEM),)
CMSIS_SRC_C += cmsis/src/$(SYSTEM).c
endif

CMSIS_SRC_C += $(addprefix cmsis/src/dsp/Source/,\
	CommonTables/CommonTables.c \
	CommonTables/CommonTablesF16.c \
	FastMathFunctions/FastMathFunctions.c \
	FastMathFunctions/FastMathFunctionsF16.c \
)

# CMSIS-DSP real FFT and vector math, used by the ML ops.
ifeq ($(MICROPY_PY_ML_OPS), 1)
CMSIS_SRC_C += $(addprefix cmsis/src/dsp/Source/,\
	TransformFunctions/arm_rfft_fast_f32.c \
	TransformFunctions/arm_rfft_fast_init_f32.c \
	TransformFunctions/arm_cfft_f32.c \
	TransformFunctions/arm_cfft_init_f32.c \
	TransformFunctions/arm_cfft_radix8_f32.c \
	TransformFunctions/arm_bitreversal2.c \
	BasicMathFunctions/arm_mult_f32.c \
	BasicMathFunctions/arm_dot_prod_f32.c \
	ComplexMathFunctions/arm_cmplx_mag_squared_f32.c \
)
endif

# CMSIS-NN, used by the ML backends. The float kernels are not used.
ifneq ($(filter 1,$(MICROPY_PY_ML_TFLM) $(MICROPY_PY_ML_TORCH)),)
CMSIS_SRC_C += $(filter-out %_f16.c %_f32.c %/arm_nntables_flt.c, \
	$(patsubst $(TOP_DIR)/lib/%,%,$(wildcard $(TOP_DIR)/lib/cmsis/src/nn/Source/*/*.c)))
endif

HAL_CFLAGS += -I$(TOP_DIR)/lib/cmsis/include
HAL_CFLAGS += -I$(TOP_DIR)/lib/cmsis/src/dsp/Include
HAL_CFLAGS += -I$(TOP_DIR)/lib/cmsis/src/dsp/PrivateInclude
HAL_CFLAGS += -I$(TOP_DIR)/lib/cmsis/include/$(CMSIS_INC)
HAL_CFLAGS += -I$(TOP_DIR)/lib/cmsis/src/nn/Include

OMV_FIRM_OBJ += $(addprefix $(BUILD)/lib/, $(CMSIS_SRC_S:.s=.o))
OMV_FIRM_OBJ += $(addprefix $(BUILD)/lib/, $(CMSIS_SRC_C:.c=.o))

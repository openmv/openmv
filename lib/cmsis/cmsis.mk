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

CMSIS_SRC_C += $(addprefix cmsis/src/dsp/,\
	CommonTables/CommonTables.c \
	CommonTables/CommonTablesF16.c \
	FastMathFunctions/FastMathFunctions.c \
	FastMathFunctions/FastMathFunctionsF16.c \
)

HAL_CFLAGS += -I$(TOP_DIR)/lib/cmsis/include
HAL_CFLAGS += -I$(TOP_DIR)/lib/cmsis/include/$(CMSIS_INC)

OMV_FIRM_OBJ += $(addprefix $(BUILD)/lib/, $(CMSIS_SRC_S:.s=.o))
OMV_FIRM_OBJ += $(addprefix $(BUILD)/lib/, $(CMSIS_SRC_C:.c=.o))

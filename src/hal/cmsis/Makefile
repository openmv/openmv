# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# CMSIS Makefile

SRC_S = src/$(STARTUP).s
SRC_C = src/$(SYSTEM).c

SRC_C  += $(wildcard src/dsp/CommonTables/*.c)
SRC_C  += $(wildcard src/dsp/FastMathFunctions/*.c)
ifeq ($(CUBEAI), 1)
SRC_C  += $(wildcard src/dsp/MatrixFunctions/*.c)
endif
#SRC_C  += $(wildcard src/nn/ActivationFunctions/*.c)
#SRC_C  += $(wildcard src/nn/ConvolutionFunctions/*.c)
#SRC_C  += $(wildcard src/nn/FullyConnectedFunctions/*.c)
#SRC_C  += $(wildcard src/nn/NNSupportFunctions/*.c)
#SRC_C  += $(wildcard src/nn/PoolingFunctions/*.c)
#SRC_C  += $(wildcard src/nn/SoftmaxFunctions/*.c)
#SRC_C  += $(wildcard src/dsp/ComplexMathFunctions/*.c)
#SRC_C  += $(wildcard src/dsp/ControllerFunctions/*.c)
#SRC_C  += $(wildcard src/dsp/FilteringFunctions/*.c)
#SRC_C  += $(wildcard src/dsp/StatisticsFunctions/*.c)
#SRC_C  += $(wildcard src/dsp/SupportFunctions/*.c)
#SRC_C  += $(wildcard src/dsp/TransformFunctions/*.c)

OBJS  = $(addprefix $(BUILD)/, $(SRC_S:.s=.o))
OBJS += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ_DIRS = $(sort $(dir $(OBJS)))

all: $(OBJS)
$(OBJS): | $(OBJ_DIRS)

$(OBJ_DIRS):
	$(MKDIR) -p $@

$(BUILD)/%.o : %.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@

-include $(OBJS:%.o=%.d)

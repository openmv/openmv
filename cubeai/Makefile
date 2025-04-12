# This file is part of the OpenMV project.
#
# Copyright (c) 2019 STMicroelectronics
#
# This work is licensed under the MIT license, see the file LICENSE for details.

# Directories
CFLAGS_STNN = -IAI/Inc/
CFLAGS_STNN += -Idata/
CFLAGS_STNN += -I/
CFLAGS_STNN := $(CFLAGS) $(CFLAGS_STNN)

#Sources
SRCS += $(addprefix ../hal/cmsis/src/dsp/BasicMathFunctions/,\
	arm_shift_q7.c                                           \
	arm_shift_q15.c                                          \
	arm_dot_prod_f32.c                                       \
)

SRCS += $(addprefix ../hal/cmsis/src/dsp/SupportFunctions/,\
	arm_float_to_q7.c                                      \
	arm_float_to_q15.c                                     \
	arm_q7_to_float.c                                      \
	arm_q15_to_float.c                                     \
	arm_q7_to_q15.c                                        \
	arm_q15_to_q7.c                                        \
	)

#CONV_FUNCTION = $(wildcard ../cmsis/src/nn/ConvolutionFunctions/*.o)
#FIRM_OBJ := $(filter-out $(CONV_FUNCTION),$(FIRM_OBJ))

SRCS += $(addprefix ,\
	py_st_nn.c       \
	nn_st.c          \
   )

SRCS += $(addprefix data/,\
	network_data.c        \
	network.c             \
   )

OBJS = $(addprefix $(BUILD)/, $(SRCS:.c=.o))
OBJ_DIRS = $(sort $(dir $(OBJS)))

all: | $(OBJ_DIRS) $(OBJS)
$(OBJ_DIRS):
	$(MKDIR) -p $@

$(BUILD)/%.o : %.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS_STNN) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@

-include $(OBJS:%.o=%.d)

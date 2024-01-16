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

SRC_C += $(addprefix src/dsp/,\
	CommonTables/CommonTables.c \
	CommonTables/CommonTablesF16.c \
	FastMathFunctions/FastMathFunctions.c \
	FastMathFunctions/FastMathFunctionsF16.c \
)

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

$(BUILD)/%.o: %.S
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@

-include $(OBJS:%.o=%.d)

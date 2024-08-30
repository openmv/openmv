# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2020 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2020 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# ST Makefile
override CFLAGS += -Os

SRCS = $(wildcard src/*.c)
OBJS = $(addprefix $(BUILD)/, $(SRCS:.c=.o))
OBJ_DIRS = $(sort $(dir $(OBJS)))

all: | $(OBJ_DIRS) $(OBJS)
$(OBJ_DIRS):
	$(MKDIR) -p $@

$(BUILD)/%.o : %.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@

-include $(OBJS:%.o=%.d)

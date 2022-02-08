# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# LIBTF Makefile
GENERATED := $(BUILD)/libtf_builtin_models.h $(BUILD)/libtf_builtin_models.c
OBJS = $(BUILD)/libtf_builtin_models.o
OBJ_DIRS = $(sort $(dir $(OBJS)))

all: | $(OBJ_DIRS) $(OBJS)
$(OBJ_DIRS):
	$(MKDIR) -p $@

$(GENERATED): $(wildcard models/*)
	$(ECHO) "GEN $@"
	$(PYTHON) $(TOOLS)/$(TFLITE2C) --input models > $(BUILD)/libtf_builtin_models.c
	$(PYTHON) $(TOOLS)/$(TFLITE2C) --input models --header > $(BUILD)/libtf_builtin_models.h

$(OBJS): $(GENERATED)

$(BUILD)/%.o : %.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@

-include $(OBJS:%.o=%.d)

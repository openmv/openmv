# This file is part of the OpenMV project.
#
# Copyright (c) 2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# D/AVE 2D Makefile
SRC_C += $(addprefix src/, \
	dave_d0lib.c \
	dave_d0_mm_dynamic.c \
	dave_d0_mm_fixed_range.c \
	dave_d0_mm_fixed_range_fixed_blkcnt.c \
	dave_64bitoperation.c \
	dave_blit.c \
	dave_box.c \
	dave_circle.c \
	dave_context.c \
	dave_curve.c \
	dave_dlist.c \
	dave_driver.c \
	dave_edge.c \
	dave_errorcodes.c \
	dave_gradient.c \
	dave_hardware.c \
	dave_line.c \
	dave_math.c \
	dave_memory.c \
	dave_pattern.c \
	dave_perfcount.c \
	dave_polyline.c \
	dave_quad.c \
	dave_rbuffer.c \
	dave_render.c \
	dave_texture.c \
	dave_triangle.c \
	dave_utility.c \
	dave_viewport.c \
	dave_wedge.c \
)

OBJS = $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
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

$(BUILD)/src/dave_d0lib.o: override CFLAGS += -Wno-unused-value
$(BUILD)/src/dave_d0_mm_fixed_range.o: override CFLAGS += -Wno-unused-value

-include $(OBJS:%.o=%.d)

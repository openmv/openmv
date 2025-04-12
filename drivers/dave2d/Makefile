# SPDX-License-Identifier: MIT
#
# Copyright (C) 2024 OpenMV, LLC.
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

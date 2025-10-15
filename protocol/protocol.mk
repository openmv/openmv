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
# OpenMV Protocol Makefile

PROTOCOL_SRC_C += \
    omv_protocol.c \
    omv_protocol_channel_stdio.c \
    omv_protocol_channel_stream.c \
    omv_protocol_channel_profile.c \

ifeq ($(OMV_USB_STACK_TINYUSB), 1)
CFLAGS += -DOMV_USB_STACK_TINYUSB=1
PROTOCOL_SRC_C += omv_protocol_channel_tinyusb.c
endif

ifeq ($(OMV_USB_STACK_STMUSB), 1)
CFLAGS += -DOMV_USB_STACK_STMUSB=1
PROTOCOL_SRC_C += omv_protocol_channel_stmusb.c
endif

CFLAGS += -I$(TOP_DIR)/protocol
OMV_FIRM_OBJ += $(addprefix $(BUILD)/protocol/, $(PROTOCOL_SRC_C:.c=.o))


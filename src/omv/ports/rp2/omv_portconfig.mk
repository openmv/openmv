# SPDX-License-Identifier: MIT
#
# Copyright (C) 2021-2024 OpenMV, LLC.
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
# RP2 Makefile.
# This just invokes make in MicroPython RP2 port with OpenMV's cmake arg.

export Q=
export CC=
export CXX=

# Note this overrides USER_C_MODULES.
MPY_MKARGS += BOARD=$(TARGET) BUILD=$(BUILD)/rp2 USER_C_MODULES="" \
              OMV_CMAKE=$(TOP_DIR)/$(OMV_DIR)/ports/$(PORT)/omv_portconfig.cmake

###################################################
all: $(OPENMV)

$(FIRMWARE):
	$(MAKE)  -C $(MICROPY_DIR)/ports/$(PORT) $(MPY_MKARGS)

# This target generates the firmware image.
$(OPENMV): $(FIRMWARE)
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

size:
	$(SIZE) --format=SysV $(FW_DIR)/$(FIRMWARE).elf

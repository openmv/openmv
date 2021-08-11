# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# RP2 Makefile.
# This just invokes make in MicroPython RP2 port with OpenMV's cmake arg.

export PORT
export TARGET
export TOP_DIR
export CC=
export CXX=

# Note this overrides USER_C_MODULES.
MICROPY_ARGS += BOARD=$(TARGET) BUILD=$(BUILD)/rp2 OMV_CMAKE=$(TOP_DIR)/$(OMV_DIR)/ports/$(PORT)/omv_portconfig.cmake USER_C_MODULES=""

###################################################
all: $(OPENMV)

$(FIRMWARE):
	$(MAKE)  -C $(MICROPY_DIR)/ports/$(PORT) $(MICROPY_ARGS)

# This target generates the firmware image.
$(OPENMV): $(FIRMWARE)
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

size:
	$(SIZE) --format=SysV $(FW_DIR)/$(FIRMWARE).elf

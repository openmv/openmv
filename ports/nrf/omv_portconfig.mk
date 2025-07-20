# SPDX-License-Identifier: MIT
#
# Copyright (C) 2020-2024 OpenMV, LLC.
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

# Set startup and system files based on MCU.
LDSCRIPT  ?= nrf52xxx
HAL_DIR   ?= lib/nrfx
SYSTEM    ?= nrf/system_nrf52840
STARTUP   ?= nrf/startup_$(shell echo $(MCU) | tr '[:upper:]' '[:lower:]')
MCU_LOWER := $(shell echo $(MCU) | tr '[:upper:]' '[:lower:]')


export SD_DIR = $(TOP_DIR)/drivers/nrf

# Compiler Flags
CFLAGS += -std=gnu11 \
          -Wall \
          -Werror \
          -Warray-bounds \
          -mthumb \
          -nostartfiles \
          -fdata-sections \
          -ffunction-sections

CFLAGS += -D$(MCU) \
          -DARM_NN_TRUNCATE \
          -D__FPU_PRESENT=1 \
          -D__VFP_FP__ \
          -D$(TARGET) \
          -fsingle-precision-constant \
          -Wdouble-promotion \
          -mcpu=$(CPU) \
          -mtune=$(CPU) \
          -mfpu=$(FPU) \
          -mfloat-abi=hard \
          -DCMSIS_MCU_H='<$(MCU_LOWER).h>' \
          -DMP_PORT_NO_SOFTTIMER \
          $(OMV_BOARD_CFLAGS)

# Linker Flags
LDFLAGS = -mcpu=$(CPU) \
          -mabi=aapcs-linux \
          -mthumb \
          -mfpu=$(FPU) \
          -mfloat-abi=hard \
          -nostdlib \
          -Wl,--gc-sections \
          -Wl,--print-memory-usage \
          -Wl,--wrap=mp_usbd_task \
          -Wl,--wrap=tud_cdc_rx_cb \
          -Wl,--wrap=mp_hal_stdio_poll \
          -Wl,--wrap=mp_hal_stdout_tx_strn \
          -Wl,--no-warn-rwx-segment \
          -Wl,-Map=$(BUILD)/$(FIRMWARE).map \
          -Wl,-T$(BUILD)/$(LDSCRIPT).lds

OMV_CFLAGS += -I$(TOP_DIR)/$(COMMON_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/modules
OMV_CFLAGS += -I$(TOP_DIR)/ports/$(PORT)
OMV_CFLAGS += -I$(TOP_DIR)/ports/$(PORT)/modules
OMV_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)

MPY_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)
MPY_CFLAGS += -I$(BUILD)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/mp-readline
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/tinyusb/src
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/drivers/usb
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/drivers/bluetooth
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/modules/machine
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/modules/ubluepy
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/modules/music
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/modules/ble
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/modules/board
MPY_CFLAGS += -I$(TOP_DIR)/drivers/nrf/$(NRF_SOFTDEV)/$(NRF_SOFTDEV)_API/include/

# Disable LTO and set the SD
MPY_MKARGS += LTO=0 SD=$(SD)
MPY_MKARGS += CFLAGS_EXTRA="-std=gnu11"
ifeq ($(MICROPY_PY_ULAB), 1)
MPY_CFLAGS += -DMP_NEED_LOG2
endif

CFLAGS += $(HAL_CFLAGS) $(MPY_CFLAGS) $(OMV_CFLAGS)

# Firmware objects from .mk files.
include lib/cmsis/cmsis.mk
include lib/nrfx/nrfx.mk
include common/common.mk
include drivers/drivers.mk
include lib/imlib/imlib.mk
# include lib/tflm/tflm.mk
include ports/ports.mk
include common/micropy.mk

# Firmware objects from port.
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	mphalport.o \
	help.o \
	gccollect.o \
	pins_gen.o \
	pin_named_pins.o \
	fatfs_port.o \
	frozen_content.o \
	)

MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/,\
	littlefs/*.o \
	)

MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/libm/,\
	math.o \
	fmodf.o \
	nearbyintf.o \
	ef_sqrt.o \
	erf_lgamma.o\
	kf_rem_pio2.o \
	kf_sin.o \
	kf_cos.o \
	kf_tan.o \
	ef_rem_pio2.o \
	sf_sin.o \
	sf_cos.o \
	sf_tan.o \
	sf_frexp.o \
	sf_modf.o \
	sf_ldexp.o \
	sf_erf.o \
	asinfacosf.o\
	atanf.o  \
	atan2f.o \
	roundf.o \
	log1pf.o \
	acoshf.o \
	asinhf.o \
	atanhf.o \
	wf_lgamma.o \
	wf_tgamma.o \
	)

###################################################
all: $(FIRMWARE)
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

# This target builds MicroPython.
MICROPYTHON: | FIRM_DIRS
	$(MAKE) -C $(MICROPY_DIR)/ports/$(PORT) BUILD=$(BUILD)/$(MICROPY_DIR) $(MPY_MKARGS)

$(OMV_FIRM_OBJ): | MICROPYTHON

# This target bulds the firmware.
$(FIRMWARE): $(OMV_FIRM_OBJ)
	$(CPP) -P -E -I$(COMMON_DIR) -I$(OMV_BOARD_CONFIG_DIR) \
        ports/$(PORT)/$(LDSCRIPT).ld.S > $(BUILD)/$(LDSCRIPT).lds
	$(CC) $(LDFLAGS) $(OMV_FIRM_OBJ) $(MPY_FIRM_OBJ) -o $(FW_DIR)/$(FIRMWARE).elf $(LIBS) -lgcc
	$(OBJCOPY) -Oihex   $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).hex
	$(OBJCOPY) -Obinary $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin

# Flash the main firmware image
flash_image: $(FW_DIR)/$(FIRMWARE).hex
	nrfjprog --program $< --sectorerase -f nrf52
	nrfjprog --reset -f nrf52

include common/mkrules.mk

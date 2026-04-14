# SPDX-License-Identifier: MIT
#
# Copyright (C) 2023 OpenMV, LLC.
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

# Set startup and system files for CMSIS Makefile.
MCU_SERIES := $(shell echo $(MCU) | cut -c1-10)
LDSCRIPT ?= mimxrt
SYSTEM   ?= mimxrt/system_$(MCU_SERIES)
STARTUP  ?= mimxrt/startup_$(MCU_SERIES)
HAL_DIR  ?= lib/mimxrt/$(MCU_SERIES)

ROMFS_IMAGE := $(FW_DIR)/romfs.stamp
ROMFS_CONFIG := $(OMV_BOARD_CONFIG_DIR)/romfs_config.json

# Compiler Flags
CFLAGS += -std=gnu11 \
          -Wall \
          -Werror \
          -Warray-bounds \
          -nostartfiles \
          -fdata-sections \
          -ffunction-sections \
          -fno-inline-small-functions \
          -mfloat-abi=$(FABI) \
          -mthumb \
          -mcpu=$(CPU) \
          -mtune=$(CPU) \
          -mfpu=$(FPU)

# TODO: FIX HSE
CFLAGS += -DCPU_$(MCU) \
          -D$(TARGET) \
          -DARM_NN_TRUNCATE \
          -D__FPU_PRESENT=1 \
          -D__VFP_FP__ \
          -DHSE_VALUE=$(OMV_HSE_VALUE) \
          -DMICROPY_PY_MACHINE_SDCARD=1 \
          -DXIP_EXTERNAL_FLASH=1 \
	      -DXIP_BOOT_HEADER_ENABLE=1 \
	      -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
	      -DCFG_TUSB_MCU=OPT_MCU_MIMXRT1XXX \
          -DCFG_TUD_MAX_SPEED=OPT_MODE_HIGH_SPEED \
	      -DCFG_TUD_TASK_QUEUE_SZ=128 \
	      -DCPU_HEADER_H='<$(MCU_SERIES).h>' \
	      -DCMSIS_MCU_H='<$(MCU_SERIES).h>' \
	      -DCLOCK_CONFIG_H='<boards/$(MCU_SERIES)_clock_config.h>' \
          -DCSI_DRIVER_FRAG_MODE=1 \
          -DOMV_NOSYS_STUBS_ENABLE=1 \
          -D__START=main \
          -D__STARTUP_CLEAR_BSS \
          -D__STARTUP_INITIALIZE_RAMFUNCTION \
          $(OMV_BOARD_CFLAGS)

# Linker Flags
LDFLAGS = -mthumb \
          -mcpu=$(CPU) \
          -mfpu=$(FPU) \
          -mfloat-abi=hard \
          -mabi=aapcs-linux \
          -Wl,--print-memory-usage \
          -Wl,--gc-sections \
          -Wl,--wrap=tud_task_ext \
          -Wl,--wrap=mp_hal_stdio_poll \
          -Wl,--wrap=mp_hal_stdout_tx_strn \
          -Wl,-T$(BUILD)/$(LDSCRIPT).lds \
          -Wl,-Map=$(BUILD)/$(FIRMWARE).map

OMV_CFLAGS += -I$(TOP_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/$(COMMON_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/modules
OMV_CFLAGS += -I$(TOP_DIR)/ports/$(PORT)
OMV_CFLAGS += -I$(TOP_DIR)/ports/$(PORT)/modules
OMV_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)

MPY_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/mimxrt
MPY_CFLAGS += -DMICROPY_VFS_FAT=1

MPY_MKARGS += CMSIS_DIR=$(TOP_DIR)/$(CMSIS_DIR)
MPY_MKARGS += MCU_DIR=$(TOP_DIR)/$(HAL_DIR)
MPY_MKARGS += SUPPORTS_HARDWARE_FP_SINGLE=1
MPY_MKARGS += MICROPY_VFS_LFS2=0
MPY_MKARGS += CFLAGS_EXTRA="-std=gnu11"

CFLAGS += $(HAL_CFLAGS) $(MPY_CFLAGS) $(OMV_CFLAGS)

MPY_LIB_EXCLUDE = ! -name 'board_init.*' ! -name 'fsl_flexspi_nor_boot.*' \
                   ! -name 'resethandler*' ! -name 'help.*' \
                   ! -path '*/nxp_driver/*'

# Firmware objects from .mk files.
include lib/cmsis/cmsis.mk
include lib/mimxrt/mimxrt.mk
include common/common.mk
include drivers/drivers.mk
include lib/imlib/imlib.mk
include lib/apriltag/apriltag.mk
include lib/tflm/tflm.mk
include ports/ports.mk
include protocol/protocol.mk
include common/micropy.mk

# Libraries
ifeq ($(MICROPY_PY_AUDIO), 1)
LIBS += $(TOP_DIR)/$(LIBPDM_DIR)/libPDMFilter_CM7_GCC_wc32.a
endif

ifeq ($(MICROPY_PY_ML_TFLM), 1)
LIBS += $(TOP_DIR)/$(TENSORFLOW_DIR)/libtflm/lib/libtflm-$(CPU)+fp-release.a
endif

###################################################
all: $(ROMFS_IMAGE)
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

# This target builds the firmware.
$(FIRMWARE): $(OMV_FIRM_OBJ)
	$(ECHO) "GEN linker script"
	$(PYTHON) $(TOOLS_DIR)/$(GENLINK) --board $(TARGET) \
        --ldscript ports/$(PORT)/$(LDSCRIPT).ld.S > $(BUILD)/$(LDSCRIPT).lds
	$(CC) $(LDFLAGS) $(OMV_FIRM_OBJ) -o $(FW_DIR)/$(FIRMWARE).elf $(LIBS) -lm
	$(OBJCOPY) -Obinary -R .big_const* $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin

# This target generates the romfs image.
$(ROMFS_IMAGE): $(ROMFS_CONFIG) | $(FIRMWARE)
	$(ECHO) "GEN romfs image"
	$(PYTHON) $(TOOLS_DIR)/$(MKROMFS) \
            --top-dir $(TOP_DIR) --out-dir $(FW_DIR) \
            --build-dir $(BUILD) --config $(ROMFS_CONFIG)
	touch $@

include common/mkrules.mk

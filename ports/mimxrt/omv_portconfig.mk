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
ROMFS_CONFIG := $(OMV_BOARD_CONFIG_DIR)/romfs.json

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
MPY_CFLAGS += -I$(BUILD)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/tinyusb/src
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/lwip/src/include
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/shared/tinyusb
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/mimxrt
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/mimxrt/lwip_inc
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/shared/runtime

MPY_CFLAGS += -DMICROPY_PY_LWIP=$(MICROPY_PY_LWIP)
MPY_CFLAGS += -DMICROPY_PY_SSL=$(MICROPY_PY_SSL)
MPY_CFLAGS += -DMICROPY_PY_SSL_ECDSA_SIGN_ALT=$(MICROPY_PY_SSL_ECDSA_SIGN_ALT)
MPY_CFLAGS += -DMICROPY_SSL_MBEDTLS=$(MICROPY_SSL_MBEDTLS)
MPY_CFLAGS += -DMICROPY_PY_NETWORK_CYW43=$(MICROPY_PY_NETWORK_CYW43)
MPY_CFLAGS += -DMICROPY_PY_BLUETOOTH=$(MICROPY_PY_BLUETOOTH)
MPY_CFLAGS += -DMICROPY_BLUETOOTH_NIMBLE=$(MICROPY_BLUETOOTH_NIMBLE)
MPY_CFLAGS += -DMICROPY_PY_BLUETOOTH_USE_SYNC_EVENTS=1
MPY_CFLAGS += -DMICROPY_PY_OPENAMP=$(MICROPY_PY_OPENAMP)
MPY_CFLAGS += -DMICROPY_PY_OPENAMP_REMOTEPROC=$(MICROPY_PY_OPENAMP_REMOTEPROC) 
MPY_CFLAGS += -DMICROPY_VFS_FAT=1

MPY_MKARGS += CMSIS_DIR=$(TOP_DIR)/$(CMSIS_DIR)
MPY_MKARGS += MCU_DIR=$(TOP_DIR)/$(HAL_DIR)
MPY_MKARGS += SUPPORTS_HARDWARE_FP_SINGLE=1
MPY_MKARGS += MICROPY_VFS_LFS2=0
MPY_MKARGS += CFLAGS_EXTRA="-std=gnu11"
MPY_MKARGS += MICROPY_PY_LWIP=$(MICROPY_PY_LWIP)
MPY_MKARGS += MICROPY_PY_SSL=$(MICROPY_PY_SSL)
MPY_MKARGS += MICROPY_PY_SSL_ECDSA_SIGN_ALT=$(MICROPY_PY_SSL_ECDSA_SIGN_ALT)
MPY_MKARGS += MICROPY_SSL_MBEDTLS=$(MICROPY_SSL_MBEDTLS)
MPY_MKARGS += MICROPY_PY_NETWORK_CYW43=$(MICROPY_PY_NETWORK_CYW43)
MPY_MKARGS += MICROPY_PY_BLUETOOTH=$(MICROPY_PY_BLUETOOTH)
MPY_MKARGS += MICROPY_BLUETOOTH_NIMBLE=$(MICROPY_BLUETOOTH_NIMBLE)
MPY_MKARGS += MICROPY_PY_OPENAMP=$(MICROPY_PY_OPENAMP)
MPY_MKARGS += MICROPY_PY_OPENAMP_REMOTEPROC=$(MICROPY_PY_OPENAMP_REMOTEPROC)

CFLAGS += $(HAL_CFLAGS) $(MPY_CFLAGS) $(OMV_CFLAGS)

# Firmware objects from .mk files.
include lib/cmsis/cmsis.mk
include lib/mimxrt/mimxrt.mk
include common/common.mk
include drivers/drivers.mk
include lib/imlib/imlib.mk
include lib/tflm/tflm.mk
include ports/ports.mk
include common/micropy.mk

# Firmware objects from port.
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	boards/$(MCU_SERIES)_clock_config.o \
	dma_manager.o                       \
	eth.o                               \
	fatfs_port.o                        \
	frozen_content.o                    \
	flash.o                             \
	led.o                               \
	machine_bitstream.o                 \
	machine_can.o                       \
	machine_i2c.o                       \
	machine_led.o                       \
	machine_pin.o                       \
	machine_rtc.o                       \
	machine_sdcard.o                    \
	machine_spi.o                       \
	mimxrt_flash.o                      \
	mimxrt_sdram.o                      \
	modmimxrt.o                         \
	msc_disk.o                          \
	network_lan.o                       \
	mphalport.o                         \
	pin.o                               \
	pins_gen.o                          \
	sdcard.o                            \
	sdio.o                              \
	systick.o                           \
	ticks.o                             \
	usbd.o                              \
	hal/pwm_backport.o                  \
	hal/flexspi_nor_flash.o             \
	hal/qspi_nor_flash_config.o         \
)

# Ethernet physical driver.
ifeq ($(MICROPY_PY_LWIP), 1)
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/hal/,\
	phy/mdio/enet/fsl_enet_mdio.o           \
	phy/device/phydp83825/fsl_phydp83825.o  \
	phy/device/phydp83848/fsl_phydp83848.o  \
	phy/device/phyksz8081/fsl_phyksz8081.o  \
	phy/device/phylan8720/fsl_phylan8720.o  \
	phy/device/phyrtl8211f/fsl_phyrtl8211f.o\
)
endif

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

# This target builds MicroPython.
MICROPYTHON: | FIRM_DIRS
	$(MAKE) -C $(MICROPY_DIR)/ports/$(PORT) BUILD=$(BUILD)/$(MICROPY_DIR) $(MPY_MKARGS)

$(OMV_FIRM_OBJ): | MICROPYTHON

# This target builds the firmware.
$(FIRMWARE): $(OMV_FIRM_OBJ)
	$(CPP) -P -E -DLINKER_SCRIPT -I$(COMMON_DIR) -I$(OMV_BOARD_CONFIG_DIR) \
        ports/$(PORT)/$(LDSCRIPT).ld.S > $(BUILD)/$(LDSCRIPT).lds
	$(CC) $(LDFLAGS) $(OMV_FIRM_OBJ) $(MPY_FIRM_OBJ) -o $(FW_DIR)/$(FIRMWARE).elf $(LIBS) -lm
	$(OBJCOPY) -Obinary -R .big_const* $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin

# This target generates the romfs image.
$(ROMFS_IMAGE): $(ROMFS_CONFIG) | $(FIRMWARE)
	$(ECHO) "GEN romfs image"
	$(PYTHON) $(TOOLS_DIR)/$(MKROMFS) \
            --top-dir $(TOP_DIR) --out-dir $(FW_DIR) \
            --build-dir $(BUILD) --config $(ROMFS_CONFIG)
	touch $@

include common/mkrules.mk

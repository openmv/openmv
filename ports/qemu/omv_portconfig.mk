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

# Set startup and system files for CMSIS Makefile.
LDSCRIPT ?= qemu
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
          -mthumb \
          -mcpu=$(CPU) \
          -mfpu=$(FPU) \
          -mfloat-abi=hard

CFLAGS += -D$(TARGET) \
          -DQEMU_BUILD \
          -DARM_NN_TRUNCATE=1 \
          -DCMSIS_MCU_H=$(CMSIS_MCU_H) \
          $(OMV_BOARD_CFLAGS)

# Linker Flags
LDFLAGS = -mthumb \
          -nostartfiles \
          -mcpu=$(CPU) \
          -mfpu=$(FPU) \
          -mfloat-abi=hard \
          -mabi=aapcs-linux \
          -Wl,--print-memory-usage \
          -Wl,--gc-sections \
          -Wl,-T$(BUILD)/$(LDSCRIPT).lds \
          -Wl,-Map=$(BUILD)/$(FIRMWARE).map

OMV_CFLAGS += -I$(TOP_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/$(COMMON_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/modules
OMV_CFLAGS += -I$(TOP_DIR)/ports/$(PORT)
OMV_CFLAGS += -I$(TOP_DIR)/ports/$(PORT)/modules
OMV_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/protocol

MPY_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)
MPY_CFLAGS += -I$(BUILD)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/tinyusb/src
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/lwip/src/include
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/shared/tinyusb
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/qemu
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/qemu/lwip_inc
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/shared/runtime

MPY_CFLAGS += -DMICROPY_PY_LWIP=$(MICROPY_PY_LWIP)
MPY_CFLAGS += -DMICROPY_PY_SSL=$(MICROPY_PY_SSL)
MPY_CFLAGS += -DMICROPY_PY_SSL_ECDSA_SIGN_ALT=$(MICROPY_PY_SSL_ECDSA_SIGN_ALT)
MPY_CFLAGS += -DMICROPY_SSL_MBEDTLS=$(MICROPY_SSL_MBEDTLS)
MPY_CFLAGS += -DMICROPY_PY_NETWORK_CYW43=$(MICROPY_PY_NETWORK_CYW43)
MPY_CFLAGS += -DMICROPY_PY_BLUETOOTH=$(MICROPY_PY_BLUETOOTH)
MPY_CFLAGS += -DMICROPY_BLUETOOTH_NIMBLE=$(MICROPY_BLUETOOTH_NIMBLE)
MPY_CFLAGS += -DMICROPY_PY_BLUETOOTH_USE_SYNC_EVENTS=1
MPY_CFLAGS += -DMICROPY_FLOAT_IMPL=MICROPY_FLOAT_IMPL_FLOAT

MPY_MKARGS += CMSIS_DIR=$(TOP_DIR)/$(CMSIS_DIR)
MPY_MKARGS += MICROPY_VFS_LFS2=0
MPY_MKARGS += CFLAGS_EXTRA="-std=gnu11"
MPY_MKARGS += MICROPY_PY_LWIP=$(MICROPY_PY_LWIP)
MPY_MKARGS += MICROPY_PY_SSL=$(MICROPY_PY_SSL)
MPY_MKARGS += MICROPY_PY_SSL_ECDSA_SIGN_ALT=$(MICROPY_PY_SSL_ECDSA_SIGN_ALT)
MPY_MKARGS += MICROPY_SSL_MBEDTLS=$(MICROPY_SSL_MBEDTLS)
MPY_MKARGS += MICROPY_PY_NETWORK_CYW43=$(MICROPY_PY_NETWORK_CYW43)
MPY_MKARGS += MICROPY_PY_BLUETOOTH=$(MICROPY_PY_BLUETOOTH)
MPY_MKARGS += MICROPY_BLUETOOTH_NIMBLE=$(MICROPY_BLUETOOTH_NIMBLE)
MPY_MKARGS += MICROPY_FLOAT_IMPL=float
MPY_MKARGS += SUPPORTS_HARDWARE_FP_SINGLE=1

CFLAGS += $(HAL_CFLAGS) $(MPY_CFLAGS) $(OMV_CFLAGS)

# Firmware objects from .mk files.
include lib/cmsis/cmsis.mk
include common/common.mk
include drivers/drivers.mk
include lib/imlib/imlib.mk
include lib/tflm/tflm.mk
include ports/ports.mk
include common/micropy.mk

# Firmware objects from port.
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	uart.o                              \
	mphalport.o                         \
	frozen_content.o                    \
	mcu/arm/errorhandler.o              \
	mcu/arm/startup.o                   \
	mcu/arm/ticks.o                     \
	vfs_rom_ioctl.o                     \
)

ifeq ($(MICROPY_PY_ML_TFLM), 1)
ifeq ($(CPU),cortex-m55)
ifneq ($(filter -DETHOS_U,$(OMV_BOARD_CFLAGS)),)
LIBS += $(TOP_DIR)/$(TENSORFLOW_DIR)/libtflm/lib/libtflm-cortex-m55-u55-release.a
else
LIBS += $(TOP_DIR)/$(TENSORFLOW_DIR)/libtflm/lib/libtflm-cortex-m55-release.a
endif
else
LIBS += $(TOP_DIR)/$(TENSORFLOW_DIR)/libtflm/lib/libtflm-$(CPU)+fp-release.a
endif
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
	$(OBJCOPY) -Obinary $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin

$(ROMFS_IMAGE): $(ROMFS_CONFIG) | $(FIRMWARE)
	$(ECHO) "GEN romfs image"
	$(PYTHON) $(TOOLS_DIR)/$(MKROMFS) \
            --top-dir $(TOP_DIR) \
            --out-dir $(FW_DIR) \
            --build-dir $(BUILD)/lib/models \
            --vela-args $(VELA_ARGS) --config $(ROMFS_CONFIG)
	touch $@

ifeq ($(EMULATOR),FVP)
FVP_TELNET_PORT ?= 5555
FVP_BINARY = FVP_Corstone_SSE-300_Ethos-U55
FVP_ARGS += -C cpu0.CFGITCMSZ=14 \
            -C cpu0.semihosting-enable=1 \
            -C mps3_board.FPGA_SRAM_SIZE=8 \
            -C mps3_board.sse300.NUMVMBANK=2 \
            -C mps3_board.sse300.VM_BANK_SIZE=4096 \
            -C ethosu.num_macs=256 \
            -C mps3_board.uart0.unbuffered_output=1 \
            -C mps3_board.uart0.rx_overrun_mode=1 \
            -C mps3_board.telnetterminal0.start_telnet=1 \
            -C mps3_board.telnetterminal0.start_port=$(FVP_TELNET_PORT) \
            -C mps3_board.telnetterminal0.mode=raw \
            -C mps3_board.telnetterminal1.start_telnet=0 \
            -C mps3_board.telnetterminal2.start_telnet=0 \
            -C mps3_board.telnetterminal5.start_telnet=0 \
            -C mps3_board.visualisation.disable-visualisation=1 \
            --stat -q

ifdef OMV_ROMFS_PART0_ORIGIN
FVP_ARGS += --data $(FW_DIR)/romfs0.img@$(OMV_ROMFS_PART0_ORIGIN)
endif

ifdef OMV_ROMFS_PART1_ORIGIN
FVP_ARGS += --data $(FW_DIR)/romfs1.img@$(OMV_ROMFS_PART1_ORIGIN)
endif

run: $(ROMFS_IMAGE)
	FVP_MOUNT_DIR=$(TOP_DIR) $(FVP_BINARY) $(FVP_ARGS) -a $(FW_DIR)/$(FIRMWARE).elf
else
QEMU_SYSTEM = qemu-system-arm
QEMU_ARGS += -machine $(QEMU_MACHINE) -nographic -monitor null -semihosting

ifdef OMV_ROMFS_PART0_ORIGIN
QEMU_ARGS += -device loader,file=$(FW_DIR)/romfs0.img,addr=$(OMV_ROMFS_PART0_ORIGIN),force-raw=on
endif

ifdef OMV_ROMFS_PART1_ORIGIN
QEMU_ARGS += -device loader,file=$(FW_DIR)/romfs1.img,addr=$(OMV_ROMFS_PART1_ORIGIN),force-raw=on
endif

run: $(ROMFS_IMAGE)
	$(QEMU_SYSTEM) $(QEMU_ARGS) -serial pty -kernel $(FW_DIR)/$(FIRMWARE).elf
endif

include common/mkrules.mk

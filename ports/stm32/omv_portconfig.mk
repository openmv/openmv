# SPDX-License-Identifier: MIT
#
# Copyright (C) 2013-2024 OpenMV, LLC.
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
SYSTEM      ?= st/system_stm32
LDSCRIPT    ?= stm32
STARTUP     ?= st/startup_$(shell echo $(MCU) | tr '[:upper:]' '[:lower:]')
MCU_SERIES  := $(shell echo $(MCU) | cut -c6-7 | tr '[:upper:]' '[:lower:]')
MCU_LOWER   := $(shell echo $(MCU) | tr '[:upper:]' '[:lower:]')
HAL_DIR     := lib/stm32/$(MCU_SERIES)
CMSIS_INC   := st
CUBE_DIR    := $(TOOLS_DIR)/st/cubeprog/bin/

SIGN_TOOL = $(CUBE_DIR)STM32MP_SigningTool_CLI
PROG_TOOL = $(CUBE_DIR)STM32_Programmer_CLI
STLDR_DIR = $(CUBE_DIR)ExternalLoader/

ifeq ($(MCU_SERIES),$(filter $(MCU_SERIES),n6))
STEDGE_ARGS ?= --stedge-args "--target stm32n6"
STEDGE_DIR = tools/st/stedgeai
STEDGE_TOOLS = $(STEDGE_DIR)/stedgeai.stamp
endif

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
          -fsingle-precision-constant \
          -Wdouble-promotion \
          -mthumb \
          -mcpu=$(CPU) \
          -mtune=$(CPU) \
          -mfpu=$(FPU) \
          -mfloat-abi=hard

CFLAGS += -D$(MCU) \
          -D$(TARGET) \
          -DARM_NN_TRUNCATE \
          -D__VFP_FP__ \
          -DUSE_FULL_LL_DRIVER \
          -DHSE_VALUE=$(OMV_HSE_VALUE)\
          -DOMV_VTOR_BASE=$(OMV_FIRM_ADDR) \
          -DCMSIS_MCU_H='<$(MCU_LOWER).h>' \
          -DOMV_NOSYS_STUBS_ENABLE=1 \
          -DSTM32_HAL_H='<stm32$(MCU_SERIES)xx_hal.h>' \
          $(OMV_BOARD_CFLAGS)

# Linker Flags
LDFLAGS = -mthumb \
          -mcpu=$(CPU) \
          -mfpu=$(FPU) \
          -mfloat-abi=hard \
          -mabi=aapcs-linux \
          -Wl,--print-memory-usage \
          -Wl,--gc-sections \
          -Wl,-T$(BUILD)/$(LDSCRIPT).lds \
          -Wl,-Map=$(BUILD)/$(FIRMWARE).map

LDSCRIPT_FLAGS += -I$(COMMON_DIR) \
                  -I$(OMV_BOARD_CONFIG_DIR)

ifneq ($(OMV_RAMFUNC_OBJS),)
LDSCRIPT_FLAGS += -DOMV_RAMFUNC_EXC="$(addprefix *,$(OMV_RAMFUNC_OBJS))"
LDSCRIPT_FLAGS += -DOMV_RAMFUNC_INC="$(foreach obj,$(OMV_RAMFUNC_OBJS),*$(obj)(.text.* .rodata.*);)"
endif


OMV_CFLAGS += -I$(TOP_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/modules
OMV_CFLAGS += -I$(TOP_DIR)/$(LIBPDM_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/$(NEMA_DIR)/include
OMV_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)

MPY_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)
MPY_CFLAGS += -I$(BUILD)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/lwip/src/include
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/mbedtls/include
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/usbdev/core/inc
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/usbdev/class/inc
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/lwip_inc
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/shared/runtime

MPY_CFLAGS += -DMICROPY_PY_LWIP=$(MICROPY_PY_LWIP)
MPY_CFLAGS += -DMICROPY_PY_SSL=$(MICROPY_PY_SSL)
MPY_CFLAGS += -DMICROPY_SSL_MBEDTLS=$(MICROPY_SSL_MBEDTLS)
MPY_CFLAGS += -DMICROPY_PY_SSL_ECDSA_SIGN_ALT=$(MICROPY_PY_SSL_ECDSA_SIGN_ALT)
MPY_CFLAGS += -DMICROPY_PY_NETWORK_CYW43=$(MICROPY_PY_NETWORK_CYW43)
MPY_CFLAGS += -DMICROPY_PY_BLUETOOTH=$(MICROPY_PY_BLUETOOTH)
MPY_CFLAGS += -DMICROPY_BLUETOOTH_NIMBLE=$(MICROPY_BLUETOOTH_NIMBLE)
MPY_CFLAGS += -DMICROPY_PY_BLUETOOTH_USE_SYNC_EVENTS=1
MPY_CFLAGS += -DMICROPY_STREAMS_POSIX_API=1
MPY_CFLAGS += -DMICROPY_VFS_FAT=1

MPY_MKARGS += STM32LIB_CMSIS_DIR=$(TOP_DIR)/$(CMSIS_DIR)
MPY_MKARGS += STM32LIB_HAL_DIR=$(TOP_DIR)/$(HAL_DIR)
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
include lib/stm32/stm32.mk
include common/common.mk
include drivers/drivers.mk
include lib/imlib/imlib.mk
include lib/tflm/tflm.mk
include lib/stai/stai.mk
include ports/ports.mk
include common/micropy.mk

# Firmware objects from port.
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	stm32_it.o              \
	usbd_conf.o             \
	usbd_desc.o             \
	usbd_cdc_interface.o    \
	usbd_hid_interface.o    \
	usbd_msc_interface.o    \
	bufhelper.o             \
	usb.o                   \
	usrsw.o                 \
	eth.o                   \
	eth_phy.o               \
	help.o                  \
	flash.o                 \
	flashbdev.o             \
	spibdev.o               \
	storage.o               \
	rtc.o                   \
	irq.o                   \
	adc.o                   \
	dac.o                   \
	dma.o                   \
	uart.o                  \
	systick.o               \
	powerctrl.o             \
	i2c.o                   \
	pyb_i2c.o               \
	spi.o                   \
	qspi.o                  \
	pyb_spi.o               \
	can.o                   \
	fdcan.o                 \
	pyb_can.o               \
	pin.o                   \
	pin_defs_stm32.o        \
	pin_named_pins.o        \
	pins_$(TARGET).o        \
	timer.o                 \
	servo.o                 \
	rng.o                   \
	led.o                   \
	mphalport.o             \
	sdcard.o                \
	sdram.o                 \
	sdio.o                  \
	xspi.o                  \
	vfs_rom_ioctl.o         \
	fatfs_port.o            \
	extint.o                \
	modpyb.o                \
	modstm.o                \
	network_lan.o           \
	machine_i2c.o           \
	machine_spi.o           \
	machine_bitstream.o     \
	pybthread.o             \
	mpthreadport.o          \
	frozen_content.o        \
	usbdev/**/src/*.o       \
)

# Libraries
ifeq ($(MICROPY_PY_AUDIO), 1)
LIBS += $(TOP_DIR)/$(LIBPDM_DIR)/libPDMFilter_CM7_GCC_wc32.a
endif

ifeq ($(MCU_SERIES),$(filter $(MCU_SERIES),n6))
LIBS += $(TOP_DIR)/$(NEMA_DIR)/lib/libnemagfx-$(CPU)+fp.a
endif

ifeq ($(MICROPY_PY_ML_TFLM), 1)
LIBS += $(TOP_DIR)/$(TENSORFLOW_DIR)/libtflm/lib/libtflm-$(CPU)+fp-release.a
endif

ifeq ($(CUBEAI), 1)
include $(TOP_DIR)/$(CUBEAI_DIR)/cube.mk
endif

###################################################
all: $(ROMFS_IMAGE)
ifeq ($(OMV_ENABLE_BL), 1)
	$(SIZE) $(FW_DIR)/$(BOOTLOADER).elf
endif
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

$(STEDGE_TOOLS):
	@bash -c "source tools/ci.sh && ci_install_stedgeai $(STEDGE_DIR)"

# This target builds MicroPython.
MICROPYTHON: $(STEDGE_TOOLS) | FIRM_DIRS
	$(MAKE) -C $(MICROPY_DIR)/ports/$(PORT) BUILD=$(BUILD)/$(MICROPY_DIR) $(MPY_MKARGS)

$(OMV_FIRM_OBJ): | MICROPYTHON

# This target builds the firmware.
$(FIRMWARE): $(OMV_FIRM_OBJ)
	$(CPP) -P -E $(LDSCRIPT_FLAGS) ports/$(PORT)/$(LDSCRIPT).ld.S > $(BUILD)/$(LDSCRIPT).lds
	$(CC) $(LDFLAGS) $(OMV_FIRM_OBJ) $(MPY_FIRM_OBJ) -o $(FW_DIR)/$(FIRMWARE).elf $(LIBS) -lm
	$(OBJCOPY) -Obinary $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin
	$(PYTHON) $(MKDFU) -D $(DFU_DEVICE) \
        -b $(OMV_FIRM_ADDR):$(FW_DIR)/$(FIRMWARE).bin $(FW_DIR)/$(FIRMWARE).dfu
ifeq ($(OMV_ENABLE_BL), 1)
	# Pad the bootloader binary with 0xFF up to the firmware start.
	$(OBJCOPY) -I binary -O binary --pad-to $$(($(OMV_FIRM_ADDR) - $(OMV_FIRM_BASE))) \
        --gap-fill 0xFF $(FW_DIR)/$(BOOTLOADER).bin $(FW_DIR)/$(BOOTLOADER).bin
	$(CAT) $(FW_DIR)/$(BOOTLOADER).bin $(FW_DIR)/$(FIRMWARE).bin > $(FW_DIR)/openmv.bin
endif

# This target builds the bootloader.
$(BOOTLOADER): $(STEDGE_TOOLS) | FIRM_DIRS
ifeq ($(OMV_ENABLE_BL), 1)
	$(MAKE) -C $(TOP_DIR)/$(BOOT_DIR) BUILD=$(BUILD)/$(BOOT_DIR)
	$(OBJCOPY) -Obinary $(FW_DIR)/$(BOOTLOADER).elf $(FW_DIR)/$(BOOTLOADER).bin
ifeq ($(OMV_SIGN_BOOT), 1)
	$(SIGN_TOOL) -bin $(FW_DIR)/$(BOOTLOADER).bin -s -nk -t fsbl \
        -of $(OMV_SIGN_FLAGS) -hv $(OMV_SIGN_HDRV) -o $(FW_DIR)/$(BOOTLOADER).bin
	chmod +rw $(FW_DIR)/$(BOOTLOADER).bin
endif
	$(PYTHON) $(MKDFU) -D $(DFU_DEVICE) -b \
        $(OMV_BOOT_ADDR):$(FW_DIR)/$(BOOTLOADER).bin $(FW_DIR)/$(BOOTLOADER).dfu
endif

$(ROMFS_IMAGE): $(ROMFS_CONFIG) | $(FIRMWARE) $(BOOTLOADER)
	$(ECHO) "GEN romfs image"
	$(PYTHON) $(TOOLS_DIR)/$(MKROMFS) \
            --top-dir $(TOP_DIR) \
            --out-dir $(FW_DIR) \
            --build-dir $(BUILD)/lib/models \
            $(STEDGE_ARGS) --config $(ROMFS_CONFIG)
	touch $@

# Flash the bootloader
flash_boot::
	$(PYDFU) -u $(FW_DIR)/$(BOOTLOADER).dfu

# Flash the main firmware image
flash_image::
	$(PYDFU) -u $(FW_DIR)/$(FIRMWARE).dfu

# Flash the bootloader using dfu_util
flash_boot_dfu_util::
	dfu-util -a 0 -d $(DFU_DEVICE) -D $(FW_DIR)/$(BOOTLOADER).dfu

# Flash the main firmware image using dfu_util
flash_image_dfu_util::
	dfu-util -a 0 -d $(DFU_DEVICE) -D $(FW_DIR)/$(FIRMWARE).dfu

deploy: $(ROMFS_IMAGE)
	$(PROG_TOOL) -c port=SWD mode=HOTPLUG ap=1 \
        -el $(STLDR_DIR)/$(OMV_PROG_STLDR) -w $(FW_DIR)/openmv.bin $(OMV_FIRM_BASE) -hardRst

include common/mkrules.mk

#Copyright (C) 2023-2024 OpenMV, LLC.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Any redistribution, use, or modification in source or binary form
#    is done solely for personal benefit and not for any commercial
#    purpose or for monetary gain. For commercial licensing options,
#    please contact openmv@openmv.io
#
# THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
# OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Include OpenMV board config.
include $(OMV_BOARD_CONFIG_DIR)/omv_boardconfig.mk

LDSCRIPT  ?= alif
BUILD := $(BUILD)/$(MCU_CORE)
FIRMWARE := $(FIRMWARE)_$(MCU_CORE)
DAVE2D_DIR=drivers/dave2d
CORE_M55_HP := $(if $(filter M55_HP,$(MCU_CORE)),1,0)
CMSIS_MCU_H := '<system_utils.h>'

ROMFS_CONFIG := $(OMV_BOARD_CONFIG_DIR)/romfs.json
ROMFS_PART := $(if $(filter M55_HP,$(MCU_CORE)),0,1)
ROMFS_IMAGE := $(FW_DIR)/romfs$(ROMFS_PART).stamp

# Compiler Flags
CFLAGS += -std=gnu11 \
          -Wall \
          -Werror \
          -Warray-bounds \
          -nostartfiles \
          -fdata-sections \
          -ffunction-sections \
          -fno-inline-small-functions \
          -mfloat-abi=hard \
          -mthumb \
          -mcpu=$(CPU) \
          -mtune=$(CPU) \
          -mfpu=$(FPU) \
          -march=armv8.1-m.main+fp+mve.fp \
          -fsingle-precision-constant \
          -Wdouble-promotion

CFLAGS += -D__VFP_FP__ \
          -D$(TARGET) \
          -D$(MCU_CORE) \
          -DCORE_$(MCU_CORE)=1 \
          -D$(ARM_MATH) \
          -DARM_NN_TRUNCATE \
          -DETHOS_U \
          -DPINS_AF_H=$(PINS_AF_H) \
          -DCMSIS_MCU_H=$(CMSIS_MCU_H) \
          -DALIF_CMSIS_H=$(CMSIS_MCU_H) \
          -DOMV_NOSYS_STUBS_ENABLE=1 \
          -DTUSB_ALIF_NO_IRQ_CFG \
          -DWITH_MM_FIXED_RANGE #WITH_MM_DYNAMIC -DNO_MSIZE
ifeq ($(MCU_CORE),M55_HP)
CFLAGS += -DRTE_LPUART_SELECT_DMA0=1 \
          -DRTE_LPSPI_SELECT_DMA0=1 \
          -DRTE_LPSPI_SELECT_DMA0_GROUP=1 \
          -DRTE_LPI2S_SELECT_DMA0=1 \
          -DRTE_LPSPI_SELECT_DMA0=1 \
          -DRTE_LPPDM_SELECT_DMA0=1
endif

CLANG_ENABLE = 1
CLANG_FLAGS = -fshort-enums \
              --target=armv8.1m-none-eabi \
              -march=armv8.1-m.main+mve.fp+fp.dp \
              -Wno-shift-count-overflow \
              -Wno-ignored-optimization-argument \
              -Wno-unused-command-line-argument \
              -D__ARMCC_VERSION=6100100 \
              -DALIF_CMSIS_H=$(CMSIS_MCU_H) \
              $(filter-out -march% -fdisable-rtl%,$(CFLAGS))

OMV_CFLAGS += -I$(TOP_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/$(COMMON_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/modules/
OMV_CFLAGS += -I$(TOP_DIR)/ports/$(PORT)/
OMV_CFLAGS += -I$(TOP_DIR)/ports/$(PORT)/modules/
OMV_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)

MPY_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)
MPY_CFLAGS += -I$(BUILD)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/tinyusb/src
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/lwip/src/include
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/mbedtls/include
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/shared/runtime
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/alif
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/alif/tinyusb_port
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/alif/lwip_inc

MPY_CFLAGS += -DMICROPY_PY_CSI=$(MICROPY_PY_CSI)
MPY_CFLAGS += -DMICROPY_PY_CSI_NG=$(MICROPY_PY_CSI_NG)
MPY_CFLAGS += -DMICROPY_PY_LWIP=$(MICROPY_PY_LWIP)
MPY_CFLAGS += -DMICROPY_PY_SSL=$(MICROPY_PY_SSL)
MPY_CFLAGS += -DMICROPY_PY_SSL_ECDSA_SIGN_ALT=$(MICROPY_PY_SSL_ECDSA_SIGN_ALT)
MPY_CFLAGS += -DMICROPY_SSL_MBEDTLS=$(MICROPY_SSL_MBEDTLS)
MPY_CFLAGS += -DMICROPY_PY_NETWORK_CYW43=$(MICROPY_PY_NETWORK_CYW43)
MPY_CFLAGS += -DMICROPY_PY_BLUETOOTH=$(MICROPY_PY_BLUETOOTH)
MPY_CFLAGS += -DMICROPY_BLUETOOTH_NIMBLE=$(MICROPY_BLUETOOTH_NIMBLE)
MPY_CFLAGS += -DMICROPY_PY_OPENAMP=$(MICROPY_PY_OPENAMP)
MPY_CFLAGS += -DMICROPY_PY_OPENAMP_REMOTEPROC=$(MICROPY_PY_OPENAMP_REMOTEPROC) 

MPY_MKARGS += MCU_CORE=$(MCU_CORE)
MPY_MKARGS += MICROPY_VFS_LFS2=0
MPY_MKARGS += MICROPY_FLOAT_IMPL=float
MPY_MKARGS += ALIF_DFP_REL_HERE=$(TOP_DIR)/$(HAL_DIR)
MPY_MKARGS += CMSIS_DIR=$(TOP_DIR)/$(HAL_DIR)/cmsis/inc
MPY_MKARGS += CFLAGS_EXTRA="-std=gnu11"
MPY_MKARGS += MICROPY_PY_CSI=$(MICROPY_PY_CSI)
MPY_MKARGS += MICROPY_PY_CSI_NG=$(MICROPY_PY_CSI_NG)
MPY_MKARGS += MICROPY_PY_LWIP=$(MICROPY_PY_LWIP)
MPY_MKARGS += MICROPY_PY_SSL=$(MICROPY_PY_SSL)
MPY_MKARGS += MICROPY_PY_SSL_ECDSA_SIGN_ALT=$(MICROPY_PY_SSL_ECDSA_SIGN_ALT)
MPY_MKARGS += MICROPY_SSL_MBEDTLS=$(MICROPY_SSL_MBEDTLS)
MPY_MKARGS += MICROPY_PY_NETWORK_CYW43=$(MICROPY_PY_NETWORK_CYW43)
MPY_MKARGS += MICROPY_PY_BLUETOOTH=$(MICROPY_PY_BLUETOOTH)
MPY_MKARGS += MICROPY_BLUETOOTH_NIMBLE=$(MICROPY_BLUETOOTH_NIMBLE)
MPY_MKARGS += MICROPY_PY_OPENAMP=$(MICROPY_PY_OPENAMP)
MPY_MKARGS += MICROPY_PY_OPENAMP_REMOTEPROC=$(MICROPY_PY_OPENAMP_REMOTEPROC) 
MPY_MKARGS += MICROPY_MANIFEST_MCU_CORE=$(shell echo $(MCU_CORE) | awk -F'_' '{print tolower($$2)}')

ifeq ($(MCU_CORE),M55_HP)
MPY_MKARGS += MICROPY_PY_OPENAMP_HOST=1
else ifeq ($(MCU_CORE),M55_HE)
MPY_MKARGS += MICROPY_PY_OPENAMP_DEVICE=1
else
$(error Invalid MCU core specified))
endif

ifeq ($(MCU_CORE),M55_HP)
VELA_ARGS="--system-config RTSS_HP_SRAM_OSPI \
           --accelerator-config ethos-u55-256 \
           --memory-mode Shared_Sram"
else
VELA_ARGS="--system-config RTSS_HE_SRAM_MRAM \
           --accelerator-config ethos-u55-128 \
           --memory-mode Shared_Sram"
endif

CFLAGS += $(HAL_CFLAGS) $(MPY_CFLAGS) $(OMV_CFLAGS)

# Linker Flags
LDFLAGS = -mthumb \
          -mcpu=$(CPU) \
          -mfpu=$(FPU) \
          -mfloat-abi=hard \
          -mabi=aapcs-linux \
          -z noexecstack \
          -Wl,--print-memory-usage \
          -Wl,--gc-sections \
          -Wl,--no-warn-rwx-segment \
          -Wl,-Map=$(BUILD)/$(FIRMWARE).map \
          -Wl,-T$(BUILD)/$(LDSCRIPT).lds

ifeq ($(MCU_CORE),M55_HP)
# Linker Flags
LDFLAGS += -Wl,--wrap=mp_usbd_task \
           -Wl,--wrap=tud_cdc_rx_cb \
           -Wl,--wrap=mp_hal_stdio_poll \
           -Wl,--wrap=mp_hal_stdout_tx_strn
endif

# Firmware objects from .mk files.
include lib/cmsis/cmsis.mk
include lib/alif/alif.mk
include common/common.mk
include drivers/drivers.mk
include lib/imlib/imlib.mk
include lib/tflm/tflm.mk
include ports/ports.mk
include common/micropy.mk

# Firmware objects from port.
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	alif_flash.o \
	cyw43_port_spi.o \
	fatfs_port.o \
	frozen_content.o \
	machine_pin.o \
	machine_i2c.o \
	machine_spi.o \
	machine_rtc.o \
	modalif.o \
	mphalport.o \
	mpuart.o \
	msc_disk.o \
	ospi_ext.o \
	ospi_flash.o \
	system_tick.o \
	usbd.o \
	pins_board.o \
	se_services.o \
	vfs_rom_ioctl.o \
)

# Libraries
ifeq ($(MICROPY_PY_ML_TFLM), 1)
LIBS += $(TOP_DIR)/$(TENSORFLOW_DIR)/libtflm/lib/libtflm-$(CPU)-u55-release.a
endif

###################################################
all: $(ROMFS_IMAGE)
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

# This target builds MicroPython.
MICROPYTHON: | FIRM_DIRS
	$(MAKE) -C $(MICROPY_DIR)/ports/$(PORT) -f alif.mk BUILD=$(BUILD)/$(MICROPY_DIR) $(MPY_MKARGS) obj

$(OMV_FIRM_OBJ): | MICROPYTHON

# This target builds the firmware.
$(FIRMWARE): $(OMV_FIRM_OBJ)
	$(CPP) -P -E -DLINKER_SCRIPT -DCORE_$(MCU_CORE) \
        -I$(COMMON_DIR) -I$(OMV_BOARD_CONFIG_DIR) \
        ports/$(PORT)/$(LDSCRIPT).ld.S > $(BUILD)/$(LDSCRIPT).lds
	$(CC) $(LDFLAGS) $(OMV_FIRM_OBJ) $(MPY_FIRM_OBJ) -o $(FW_DIR)/$(FIRMWARE).elf $(LIBS) -lm
	$(OBJCOPY) -Obinary $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin
	BIN_SIZE=$$(stat -c%s "$(FW_DIR)/$(FIRMWARE).bin"); \
    PADDED_SIZE=$$(( (BIN_SIZE + 15) / 16 * 16 )); \
    if [ $$BIN_SIZE -lt $$PADDED_SIZE ]; then \
        dd if=/dev/zero bs=1 count=$$((PADDED_SIZE - BIN_SIZE)) >> $(FW_DIR)/$(FIRMWARE).bin; \
    fi

# This target generates the romfs image.
$(ROMFS_IMAGE): $(ROMFS_CONFIG) | $(FIRMWARE)
	$(ECHO) "GEN $(FW_DIR)/romfs_$(MCU_CORE).img"
	$(PYTHON) $(TOOLS_DIR)/$(MKROMFS) \
            --top-dir $(TOP_DIR) --build-dir $(BUILD) --out-dir $(FW_DIR) \
            --partition $(ROMFS_PART) --vela-args $(VELA_ARGS) --config $(ROMFS_CONFIG)
	touch $@

include common/mkrules.mk

# Copyright (C) 2023-2024 OpenMV, LLC.
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

# Include OpenMV board config first to set the port.
include $(OMV_BOARD_CONFIG_DIR)/omv_boardconfig.mk

FIRMWARE   := bootloader
LDSCRIPT   ?= stm32
SYSTEM     ?= st/system_stm32
STARTUP    ?= st/startup_$(shell echo $(MCU) | tr '[:upper:]' '[:lower:]')
MCU_LOWER   = $(shell echo $(MCU) | tr '[:upper:]' '[:lower:]')
MCU_SERIES := $(shell echo $(MCU) | cut -c6-7 | tr '[:upper:]' '[:lower:]')
MCU_SERIES_UPPER := $(shell echo $(MCU_SERIES) | tr '[:lower:]' '[:upper:]')
HAL_DIR     = lib/stm32/$(MCU_SERIES)

# Compiler Flags
CFLAGS += -std=gnu99 \
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

CFLAGS += -D$(MCU) \
          -D$(TARGET) \
          -DARM_NN_TRUNCATE \
          -D__VFP_FP__ \
          -DUSE_DEVICE_MODE \
          -DHSE_VALUE=$(OMV_HSE_VALUE)\
          -DOMV_VTOR_BASE=$(OMV_BOOT_ADDR) \
          -DOMV_BOOT_JUMP=$(OMV_FIRM_ADDR) \
          -DCMSIS_MCU_H='<$(MCU_LOWER).h>' \
          -DSTM32_HAL_H='<stm32$(MCU_SERIES)xx_hal.h>' \
          -DUSE_FULL_LL_DRIVER \
          -DCFG_TUSB_MCU=OPT_MCU_STM32$(MCU_SERIES_UPPER) \
          $(OMV_BOOT_CFLAGS) \
          $(OMV_BOARD_CFLAGS)

CFLAGS += -I$(OMV_BOARD_CONFIG_DIR) \
          -I$(TOP_DIR)/$(BOOT_DIR)/include \
          -I$(TOP_DIR)/$(BOOT_DIR)/$(PORT_DIR) \
          -I$(TOP_DIR)/$(CMSIS_DIR)/include \
          -I$(TOP_DIR)/$(CMSIS_DIR)/include/ \
          -I$(TOP_DIR)/$(CMSIS_DIR)/include/st \
          -I$(TOP_DIR)/$(HAL_DIR)/include/ \
          -I$(TOP_DIR)/$(HAL_DIR)/include/Legacy/ \
          -I$(TOP_DIR)/$(TINYUSB_DIR)/src

CPP_CFLAGS = -P \
             -E \
             -DBOOTLOADER \
             -DLINKER_SCRIPT \
             -I$(OMV_BOARD_CONFIG_DIR) \
             -I$(TOP_DIR)/$(COMMON_DIR) \
             -I$(TOP_DIR)/$(BOOT_DIR)/include

SRC_C += $(addprefix src/common/, \
	dfu.c \
	mpu.c \
	desc.c \
	main.c \
)

SRC_C += $(addprefix $(PORT_DIR)/, \
	stm32_port.c \
	stm32_qspi.c \
	stm32_xspi.c \
	stm32_flash.c \
)

SRC_C += $(addprefix $(TINYUSB_DIR)/, \
	src/tusb.c \
	src/class/dfu/dfu_device.c \
	src/common/tusb_fifo.c \
	src/device/usbd.c \
	src/device/usbd_control.c \
	src/portable/synopsys/dwc2/dcd_dwc2.c \
	src/portable/synopsys/dwc2/dwc2_common.c \
)

SRC_C += $(addprefix $(CMSIS_DIR)/src/,\
	$(STARTUP).o                       \
	$(SYSTEM).o                        \
)

SRC_C += $(addprefix $(HAL_DIR)/src/,\
	stm32$(MCU_SERIES)xx_hal.c \
	stm32$(MCU_SERIES)xx_hal_cortex.c \
	stm32$(MCU_SERIES)xx_hal_gpio.c \
	stm32$(MCU_SERIES)xx_hal_pwr.c \
	stm32$(MCU_SERIES)xx_hal_pwr_ex.c \
	stm32$(MCU_SERIES)xx_hal_rcc.c \
	stm32$(MCU_SERIES)xx_hal_rcc_ex.c \
	stm32$(MCU_SERIES)xx_hal_rng.c \
	stm32$(MCU_SERIES)xx_ll_rcc.c \
	stm32$(MCU_SERIES)xx_ll_usb.c \
)

ifeq ($(MCU_SERIES),$(filter $(MCU_SERIES),n6))
SRC_C += $(addprefix $(HAL_DIR)/src/,\
	stm32$(MCU_SERIES)xx_hal_xspi.c \
	stm32$(MCU_SERIES)xx_hal_bsec.c \
)
endif

ifeq ($(MCU_SERIES),$(filter $(MCU_SERIES),f4 f7 h7))
SRC_C += $(addprefix $(HAL_DIR)/src/,\
	stm32$(MCU_SERIES)xx_hal_qspi.c \
	stm32$(MCU_SERIES)xx_hal_flash.c \
	stm32$(MCU_SERIES)xx_hal_flash_ex.c \
)
endif

# Firmware objects
OBJS += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJS += $(addprefix $(BUILD)/, $(SRC_S:.s=.o))
OBJS_DIR = $(sort $(dir $(OBJS)))

all: $(FIRMWARE)
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

$(OBJS): | $(OBJS_DIR)

$(OBJS_DIR):
	$(MKDIR) -p $@

$(BUILD)/%.o : %.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/$(HAL_DIR)/%.o : $(TOP_DIR)/$(HAL_DIR)/%.c
	$(ECHO) "CC $(shell realpath --relative-to=pwd $<)"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/$(CMSIS_DIR)/%.o : $(TOP_DIR)/$(CMSIS_DIR)/%.c
	$(ECHO) "CC $(shell realpath --relative-to=pwd $<)"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/$(CMSIS_DIR)/%.o : $(TOP_DIR)/$(CMSIS_DIR)/%.s
	$(ECHO) "AS $(shell realpath --relative-to=pwd $<)"
	$(AS) $(AFLAGS) $< -o $@

$(BUILD)/$(TINYUSB_DIR)/%.o : $(TOP_DIR)/$(TINYUSB_DIR)/%.c
	$(ECHO) "CC $(shell realpath --relative-to=pwd $<)"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@

$(FIRMWARE): $(OBJS)
	$(CPP) $(CPP_CFLAGS) $(PORT_DIR)/$(LDSCRIPT).ld.S > $(BUILD)/$(LDSCRIPT).lds
	$(CC) $(LDFLAGS) $(OBJS) -o $(FW_DIR)/$(FIRMWARE).elf
	$(OBJCOPY) -Obinary $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin

-include $(OBJS:%.o=%.d)

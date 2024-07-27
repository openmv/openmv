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

LDSCRIPT ?= stm32
SYSTEM   ?= st/system_stm32fxxx
STARTUP  ?= st/startup_$(shell echo $(MCU) | tr '[:upper:]' '[:lower:]')
FIRMWARE := bootloader
MCU_SERIES_LOWER := $(shell echo $(MCU_SERIES) | tr '[:upper:]' '[:lower:]')

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
          -mfloat-abi=hard \
          -D$(CFLAGS_MCU)

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
          -D__FPU_PRESENT=1 \
          -D__VFP_FP__ \
          -DUSE_DEVICE_MODE \
          -DHSE_VALUE=$(OMV_HSE_VALUE)\
          -DVECT_TAB_OFFSET=0 \
          -DOMV_BOOT_JUMP_ADDR=$(MAIN_APP_ADDR) \
          -DSTM32_HAL_H=$(HAL_INC) \
          -DCMSIS_MCU_H=$(CMSIS_MCU_H) \
          -DUSE_FULL_LL_DRIVER \
          $(OMV_BOARD_EXTRA_CFLAGS)

CFLAGS += -I$(OMV_BOARD_CONFIG_DIR) \
          -I$(TOP_DIR)/$(BOOT_DIR)/common \
          -I$(TOP_DIR)/$(BOOT_DIR)/$(PORT_DIR) \
          -I$(TOP_DIR)/$(CMSIS_DIR)/include \
          -I$(TOP_DIR)/$(CMSIS_DIR)/include/ \
          -I$(TOP_DIR)/$(CMSIS_DIR)/include/st \
          -I$(TOP_DIR)/$(HAL_DIR)/include/ \
          -I$(TOP_DIR)/$(HAL_DIR)/include/Legacy/ \
          -I$(TOP_DIR)/$(TINYUSB_DIR)/src

SRC_C += $(addprefix common/, \
	desc.c \
	dfu.c \
	main.c \
)

SRC_C += $(addprefix $(PORT_DIR)/, \
	stm32_port.c \
	stm32_qspi.c \
	stm32_flash.c \
)

SRC_C += $(addprefix $(TINYUSB_DIR)/, \
	src/tusb.c \
	src/class/dfu/dfu_device.c \
	src/common/tusb_fifo.c \
	src/device/usbd.c \
	src/device/usbd_control.c \
	src/portable/synopsys/dwc2/dcd_dwc2.c \
)

SRC_C += $(addprefix $(CMSIS_DIR)/src/,\
	$(STARTUP).o                       \
	$(SYSTEM).o                        \
)

SRC_C += $(addprefix $(HAL_DIR)/src/,\
	$(MCU_SERIES_LOWER)_hal.c \
	$(MCU_SERIES_LOWER)_hal_cortex.c \
	$(MCU_SERIES_LOWER)_hal_flash.c \
	$(MCU_SERIES_LOWER)_hal_flash_ex.c \
	$(MCU_SERIES_LOWER)_hal_gpio.c \
	$(MCU_SERIES_LOWER)_hal_pwr.c \
	$(MCU_SERIES_LOWER)_hal_pwr_ex.c \
	$(MCU_SERIES_LOWER)_hal_rcc.c \
	$(MCU_SERIES_LOWER)_hal_rcc_ex.c \
	$(MCU_SERIES_LOWER)_hal_rng.c \
	$(MCU_SERIES_LOWER)_hal_qspi.c \
	$(MCU_SERIES_LOWER)_ll_rcc.c \
	$(MCU_SERIES_LOWER)_ll_usb.c \
)

# Firmware objects
OBJS += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJS += $(addprefix $(BUILD)/, $(SRC_S:.s=.o))
OBJS_DIR = $(sort $(dir $(OBJS)))

all: $(FIRMWARE)
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

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

FIRMWARE_OBJS: | $(OBJS_DIR) $(OBJS)

$(FIRMWARE): FIRMWARE_OBJS
	$(CPP) -P -E -DBOOTLOADER -DLINKER_SCRIPT -I$(OMV_BOARD_CONFIG_DIR) \
                    $(PORT_DIR)/$(LDSCRIPT).ld.S > $(BUILD)/$(LDSCRIPT).lds
	$(CC) $(LDFLAGS) $(OBJS) -o $(FW_DIR)/$(FIRMWARE).elf
	$(OBJCOPY) -Obinary $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin

-include $(OBJS:%.o=%.d)

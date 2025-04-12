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

LDSCRIPT  ?= alif
BUILD := $(BUILD)/bootloader
FIRMWARE := bootloader
CMSIS_MCU_H := '<system_utils.h>'

# Compiler Flags
CFLAGS += -std=gnu99 \
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
          -DBOOTLOADER=1 \
          -DOMV_NOSYS_STUBS_ENABLE=1 \
          -DTUSB_ALIF_NO_IRQ_CFG \
          -DOMV_BOOT_JUMP=$(OMV_FIRM_ADDR) \
          $(OMV_BOOT_CFLAGS) \
          $(OMV_BOARD_CFLAGS)

CFLAGS += -I$(OMV_BOARD_CONFIG_DIR) \
          -I$(TOP_DIR)/$(BOOT_DIR)/include \
          -I$(TOP_DIR)/$(BOOT_DIR)/$(PORT_DIR) \
          -I$(TOP_DIR)/$(CMSIS_DIR)/include \
          -I$(TOP_DIR)/$(HAL_DIR)/drivers/include \
          -I$(TOP_DIR)/$(HAL_DIR)/ospi_xip/source/ospi \
          -I$(TOP_DIR)/$(HAL_DIR)/se_services/include \
          -I$(TOP_DIR)/$(HAL_DIR)/Device/common/config \
          -I$(TOP_DIR)/$(HAL_DIR)/Device/common/include \
          -I$(TOP_DIR)/$(HAL_DIR)/Device/core/$(MCU_CORE)/config \
          -I$(TOP_DIR)/$(HAL_DIR)/Device/core/$(MCU_CORE)/include \
          -I$(TOP_DIR)/$(HAL_DIR)/Device/$(MCU_SERIES)/$(MCU_VARIANT) \
          -I$(TOP_DIR)/$(TINYUSB_DIR)/src

# Linker Flags
LDFLAGS = -mthumb \
          -mcpu=$(CPU) \
          -mfpu=$(FPU) \
          -mfloat-abi=hard \
          -mabi=aapcs-linux \
          -Wl,--print-memory-usage \
          -Wl,--gc-sections \
          -Wl,--no-warn-rwx-segment \
          -Wl,-Map=$(BUILD)/$(FIRMWARE).map \
          -Wl,-T$(BUILD)/$(LDSCRIPT).lds

SRC_C += $(addprefix src/common/, \
	dfu.c \
	mpu.c \
	desc.c \
	main.c \
)

SRC_C += $(addprefix $(PORT_DIR)/, \
	alif_ospi.c \
	alif_port.c \
	alif_flash.c \
	alif_services.c \
)

SRC_C += $(addprefix $(OMV_COMMON_DIR)/, \
	nosys_stubs.c \
)

SRC_C += $(addprefix $(TINYUSB_DIR)/, \
	src/tusb.c \
	src/class/dfu/dfu_device.c \
	src/common/tusb_fifo.c \
	src/device/usbd.c \
	src/device/usbd_control.c \
)

SRC_C += $(addprefix $(HAL_DIR)/, \
	drivers/source/pinconf.c \
	drivers/source/mhu_driver.c \
	drivers/source/mhu_receiver.c \
	drivers/source/mhu_sender.c \
	Device/common/source/clk.c \
	Device/common/source/dcd.c \
	Device/common/source/mpu_M55.c \
	Device/common/source/tgu_M55.c \
	Device/common/source/system_M55.c \
	Device/common/source/system_utils.c \
	Device/common/source/tcm_partition.c \
	Device/core/$(MCU_CORE)/source/startup_$(MCU_CORE).c \
	se_services/source/services_host_clocks.c \
	se_services/source/services_host_handler.c \
	se_services/source/services_host_power.c \
	se_services/source/services_host_system.c \
	se_services/source/services_host_maintenance.c \
)

# Firmware objects
OBJS += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJS += $(addprefix $(BUILD)/, $(SRC_S:.s=.o))
OBJS_DIR = $(sort $(dir $(OBJS)))

$(BUILD)/$(HAL_DIR)/se_services/source/services_host_boot.o: override CFLAGS += -Wno-stringop-truncation
$(BUILD)/$(HAL_DIR)/se_services/source/services_host_system.o: override CFLAGS += -Wno-maybe-uninitialized

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

$(BUILD)/$(TINYUSB_DIR)/%.o : $(TOP_DIR)/$(TINYUSB_DIR)/%.c
	$(ECHO) "CC $(shell realpath --relative-to=pwd $<)"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/$(OMV_COMMON_DIR)/%.o : $(TOP_DIR)/$(OMV_COMMON_DIR)/%.c
	$(ECHO) "CC $(shell realpath --relative-to=pwd $<)"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@

FIRMWARE_OBJS: | $(OBJS_DIR) $(OBJS)

$(FIRMWARE): FIRMWARE_OBJS
	$(CPP) -P -E -DBOOTLOADER -DLINKER_SCRIPT -DCORE_$(MCU_CORE) -I$(OMV_COMMON_DIR) \
                    -I$(OMV_BOARD_CONFIG_DIR) $(PORT_DIR)/$(LDSCRIPT).ld.S > $(BUILD)/$(LDSCRIPT).lds
	$(CC) $(LDFLAGS) $(OBJS) -o $(FW_DIR)/$(FIRMWARE).elf
	$(OBJCOPY) -Obinary $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin
	BIN_SIZE=$$(stat -c%s "$(FW_DIR)/$(FIRMWARE).bin"); \
    PADDED_SIZE=$$(( (BIN_SIZE + 15) / 16 * 16 )); \
    if [ $$BIN_SIZE -lt $$PADDED_SIZE ]; then \
        dd if=/dev/zero bs=1 count=$$((PADDED_SIZE - BIN_SIZE)) >> $(FW_DIR)/$(FIRMWARE).bin; \
    fi

-include $(OBJS:%.o=%.d)

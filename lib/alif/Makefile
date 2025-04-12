# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Alif SDK Makefile

SRCS += \
	drivers/source/adc.c \
	drivers/source/dma_ctrl.c \
	drivers/source/dma_op.c \
	drivers/source/i2c.c \
	drivers/source/i2s.c \
	drivers/source/i3c.c \
	drivers/source/pdm.c \
	drivers/source/pinconf.c \
	drivers/source/spi.c \
	drivers/source/uart.c \
	drivers/source/utimer.c \
	drivers/source/mhu_driver.c \
	drivers/source/mhu_receiver.c \
	drivers/source/mhu_sender.c \
	drivers/source/mram.c \
	ospi_xip/source/ospi/ospi_drv.c \
	se_services/source/services_host_application.c \
	se_services/source/services_host_boot.c \
	se_services/source/services_host_clocks.c \
	se_services/source/services_host_power.c \
	se_services/source/services_host_cryptocell.c \
	se_services/source/services_host_handler.c \
	se_services/source/services_host_system.c \
	se_services/source/services_host_maintenance.c \
	Device/common/source/clk.c \
	Device/common/source/dcd.c \
	Device/common/source/pm.c \
	Device/common/source/mpu_M55.c \
	Device/common/source/tgu_M55.c \
	Device/common/source/system_M55.c \
	Device/common/source/system_utils.c \
	Device/common/source/tcm_partition.c \
	Device/core/$(MCU_CORE)/source/startup_$(MCU_CORE).c \

OBJS = $(addprefix $(BUILD)/, $(SRCS:.c=.o))
OBJ_DIRS = $(sort $(dir $(OBJS)))

all: | $(OBJ_DIRS) $(OBJS)
$(OBJ_DIRS):
	$(MKDIR) -p $@

$(BUILD)/%.o : %.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@

$(BUILD)/drivers/source/spi.o: override CFLAGS += -Wno-maybe-uninitialized
$(BUILD)/drivers/source/i3c.o: override CFLAGS += -Wno-int-conversion
$(BUILD)/drivers/source/mram.o: override CFLAGS += -Wno-strict-aliasing
$(BUILD)/se_services/source/services_host_boot.o: override CFLAGS += -Wno-stringop-truncation
$(BUILD)/se_services/source/services_host_system.o: override CFLAGS += -Wno-maybe-uninitialized

-include $(OBJS:%.o=%.d)

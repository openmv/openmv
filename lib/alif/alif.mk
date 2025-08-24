# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Alif Makefile

HAL_SRC_C += \
	drivers/source/adc.c \
	drivers/source/crc.c \
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

HAL_CFLAGS += -I$(TOP_DIR)/lib/alif/drivers/include/
HAL_CFLAGS += -I$(TOP_DIR)/lib/alif/ospi_xip/source/ospi/
HAL_CFLAGS += -I$(TOP_DIR)/lib/alif/se_services/include
HAL_CFLAGS += -I$(TOP_DIR)/lib/alif/Device/common/config/
HAL_CFLAGS += -I$(TOP_DIR)/lib/alif/Device/common/include/
HAL_CFLAGS += -I$(TOP_DIR)/lib/alif/Device/core/$(MCU_CORE)/config/
HAL_CFLAGS += -I$(TOP_DIR)/lib/alif/Device/core/$(MCU_CORE)/include/
HAL_CFLAGS += -I$(TOP_DIR)/lib/alif/Device/$(MCU_SERIES)/$(MCU_VARIANT)/

OMV_FIRM_OBJ += $(addprefix $(BUILD)/lib/alif/, $(HAL_SRC_C:.c=.o))

$(BUILD)/lib/alif/drivers/source/spi.o: override CFLAGS += -Wno-maybe-uninitialized
$(BUILD)/lib/alif/drivers/source/i3c.o: override CFLAGS += -Wno-int-conversion
$(BUILD)/lib/alif/drivers/source/mram.o: override CFLAGS += -Wno-strict-aliasing
$(BUILD)/lib/alif/se_services/source/services_host_boot.o: override CFLAGS += -Wno-stringop-truncation
$(BUILD)/lib/alif/se_services/source/services_host_system.o: override CFLAGS += -Wno-maybe-uninitialized

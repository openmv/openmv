# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# CM4 firmware Makefile
SRC_C = $(wildcard src/*.c)
SRC_C += $(addprefix $(HAL_DIR)/src/,   \
	stm32h7xx_hal.c                     \
	stm32h7xx_hal_cortex.c              \
	stm32h7xx_hal_gpio.c                \
	stm32h7xx_hal_pwr.c                 \
	stm32h7xx_hal_pwr_ex.c              \
	stm32h7xx_hal_rcc.c                 \
	stm32h7xx_hal_rcc_ex.c              \
	stm32h7xx_hal_rtc.c                 \
	stm32h7xx_hal_rtc_ex.c              \
	stm32h7xx_hal_hsem.c                \
)
#SRCS += $(addprefix $(HAL_DIR)/src/, $(notdir $(wildcard ../$(HAL_DIR)/src/*.c)))

SRC_C += $(addprefix $(CMSIS_DIR)/src/, \
	$(SYSTEM).c                         \
)

SRC_S += $(addprefix $(CMSIS_DIR)/src/, \
	$(STARTUP).s                        \
)

OBJS  = $(addprefix $(BUILD)/, $(SRC_S:.s=.o))
OBJS += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ_DIRS = $(sort $(dir $(OBJS)))

all: | $(OBJ_DIRS) $(OBJS)
$(OBJ_DIRS):
	$(MKDIR) -p $@

$(BUILD)/$(HAL_DIR)/src/%.o : $(TOP_DIR)/$(HAL_DIR)/src/%.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/$(CMSIS_DIR)/src/%.o : $(TOP_DIR)/$(CMSIS_DIR)/src/%.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/$(CMSIS_DIR)/src/%.o : $(TOP_DIR)/$(CMSIS_DIR)/src/%.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@

$(BUILD)/%.o : %.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@

-include $(OBJS:%.o=%.d)

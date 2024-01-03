# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# STHAL Makefile
SRCS = $(addprefix src/,\
stm32h7xx_hal_adc.c\
stm32h7xx_hal_adc_ex.c\
stm32h7xx_hal_dac.c\
stm32h7xx_hal_dac_ex.c\
stm32h7xx_hal_dsi.c\
stm32h7xx_hal.c\
stm32h7xx_hal_cortex.c\
stm32h7xx_hal_crc.c\
stm32h7xx_hal_crc_ex.c\
stm32h7xx_hal_dcmi.c\
stm32h7xx_hal_dfsdm.c\
stm32h7xx_hal_dfsdm_ex.c\
stm32h7xx_hal_dma.c\
stm32h7xx_hal_dma_ex.c\
stm32h7xx_hal_dma2d.c\
stm32h7xx_hal_mdma.c\
stm32h7xx_hal_flash.c\
stm32h7xx_hal_flash_ex.c\
stm32h7xx_hal_gpio.c\
stm32h7xx_hal_hcd.c\
stm32h7xx_hal_hsem.c\
stm32h7xx_hal_i2c.c\
stm32h7xx_hal_i2c_ex.c\
stm32h7xx_hal_ltdc.c\
stm32h7xx_hal_ltdc_ex.c\
stm32h7xx_hal_pcd.c\
stm32h7xx_hal_pcd_ex.c\
stm32h7xx_hal_pwr.c\
stm32h7xx_hal_pwr_ex.c\
stm32h7xx_hal_rcc.c\
stm32h7xx_hal_rcc_ex.c\
stm32h7xx_hal_rng.c\
stm32h7xx_hal_rtc.c\
stm32h7xx_hal_rtc_ex.c\
stm32h7xx_hal_sai.c\
stm32h7xx_hal_sai_ex.c\
stm32h7xx_hal_sd.c\
stm32h7xx_hal_sdram.c\
stm32h7xx_hal_spi.c\
stm32h7xx_hal_fdcan.c\
stm32h7xx_hal_tim.c\
stm32h7xx_hal_tim_ex.c\
stm32h7xx_hal_uart.c\
stm32h7xx_hal_usart.c\
stm32h7xx_hal_jpeg.c\
stm32h7xx_hal_qspi.c\
stm32h7xx_ll_rcc.c\
stm32h7xx_ll_usb.c\
stm32h7xx_ll_sdmmc.c\
stm32h7xx_ll_fmc.c\
stm32h7xx_ll_delayblock.c\
)

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

-include $(OBJS:%.o=%.d)

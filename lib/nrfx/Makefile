# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# NRF HAL Makefile
SRCS = $(addprefix src/,\
nrfx_adc.c\
nrfx_clock.c\
nrfx_comp.c\
nrfx_dppi.c\
nrfx_egu.c\
nrfx_gpiote.c\
nrfx_i2s.c\
nrfx_ipc.c\
nrfx_lpcomp.c\
nrfx_nfct.c\
nrfx_nvmc.c\
nrfx_pdm.c\
nrfx_power.c\
nrfx_ppi.c\
nrfx_prs.c\
nrfx_pwm.c\
nrfx_qdec.c\
nrfx_qspi.c\
nrfx_rng.c\
nrfx_rtc.c\
nrfx_saadc.c\
nrfx_spi.c\
nrfx_spim.c\
nrfx_spis.c\
nrfx_systick.c\
nrfx_temp.c\
nrfx_timer.c\
nrfx_twi.c\
nrfx_twim.c\
nrfx_twis.c\
nrfx_twi_twim.c\
nrfx_uart.c\
nrfx_uarte.c\
nrfx_usbd.c\
nrfx_wdt.c\
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

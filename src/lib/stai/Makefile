# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# STAI Makefile
# override CFLAGS := $(CFLAGS) -Wno-unused-variable

override CFLAGS := $(CFLAGS) \
        -Wno-unused-variable \
        -Wno-dangling-pointer \
        -Wno-incompatible-pointer-types \
        -Wno-double-promotion \
        -DSTM32N657xx \
        -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 \
        -DLL_ATON_OSAL=LL_ATON_OSAL_BARE_METAL \
        -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC \
        -DLL_ATON_SW_FALLBACK \
        -DLL_ATON_RT_RELOC \
        -DLL_ATON_EB_DBG_INFO
        #-DLL_ATON_DUMP_DEBUG_API

SRC_C = stai_backend.c
SRC_C += $(wildcard models/*.c)

SRC_C += $(addprefix libstai/ll_aton/, \
	ecloader.c \
	ll_aton.c \
	ll_aton_cipher.c \
	ll_aton_dbgtrc.c \
	ll_aton_debug.c \
	ll_aton_lib.c \
	ll_aton_lib_sw_operators.c \
	ll_aton_n64.c \
	ll_aton_osal_freertos.c \
	ll_aton_osal_linux_bw.c \
	ll_aton_osal_linux_uio.c \
	ll_aton_osal_threadx.c \
	ll_aton_profiler.c \
	ll_aton_reloc_network.c \
	ll_aton_rt_main.c \
	ll_aton_runtime.c \
	ll_aton_stai_internal.c \
	ll_aton_util.c \
	ll_sw_float.c \
	ll_sw_integer.c \
	mcu_cache.c \
	npu_cache.c \
)

OBJS = $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
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

$(BUILD)/libstai/ll_aton/ecloader.o: override CFLAGS += -Wno-format
$(BUILD)/libstai/ll_aton/ll_aton_profiler.o: override CFLAGS += -Wno-format
-include $(OBJS:%.o=%.d)

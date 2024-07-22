# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# TFLM Makefile
override CFLAGS := $(CFLAGS) -Wno-unused-variable
GENERATED := $(BUILD)/tflm_builtin_models.h $(BUILD)/tflm_builtin_models.c

HDR_OBJS = $(BUILD)/tflm_builtin_models.o
LIB_OBJS = $(BUILD)/tflm_backend.o
OBJ_DIRS = $(sort $(dir $(HDR_OBJS)))
ifeq ($(strip $(VELA_ARGS)),)
VELA_ARGS := ""
endif

# Extra module flags.
CXXFLAGS += $(filter-out -std=gnu99,$(CFLAGS)) \
            -std=c++11 \
            -fno-rtti \
            -fno-exceptions \
            -fno-use-cxa-atexit \
            -nodefaultlibs \
            -fno-unwind-tables \
            -fpermissive \
            -fmessage-length=0 \
            -fno-threadsafe-statics \
            -Wno-double-promotion \
            -Wno-float-conversion \

CXXFLAGS += -DTF_LITE_STATIC_MEMORY \
            -DTF_LITE_DISABLE_X86_NEON \
            -DKERNELS_OPTIMIZED_FOR_SPEED \
            -DTF_LITE_STRIP_ERROR_STRINGS \
            -I$(TOP_DIR)/lib/tflm/libtflm/include/ \
            -I$(TOP_DIR)/lib/tflm/libtflm/include/third_party/ \
            -I$(TOP_DIR)/lib/tflm/libtflm/include/third_party/gemmlowp/ \
            -I$(TOP_DIR)/lib/tflm/libtflm/include/third_party/flatbuffers/include/

# Add CubeAI module if enabled.
ifeq ($(MICROPY_PY_CUBEAI), 1)
SRC_USERMOD += $(OMV_MOD_DIR)/../../stm32cubeai/py_st_nn.c
endif

all: | headers $(LIB_OBJS)
headers: | $(OBJ_DIRS) $(HDR_OBJS)

$(OBJ_DIRS):
	$(MKDIR) -p $@

$(GENERATED): $(wildcard models/*)
	$(ECHO) "GEN $(shell realpath --relative-to=pwd $@)"
	$(PYTHON) $(TOOLS)/$(TFLITE2C) --input models --build-dir ${BUILD} \
                                   --vela-args ${VELA_ARGS} > $(BUILD)/tflm_builtin_models.c
	$(PYTHON) $(TOOLS)/$(TFLITE2C) --input models --build-dir ${BUILD} \
                                   --vela-args ${VELA_ARGS} --header > $(BUILD)/tflm_builtin_models.h

$(HDR_OBJS): $(GENERATED)

$(BUILD)/%.o : %.c
	$(ECHO) "CC $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.cc
	$(ECHO) "CXX $<"
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/%.o : %.s
	$(ECHO) "AS $<"
	$(AS) $(AFLAGS) $< -o $@

-include $(HDR_OBJS:%.o=%.d)
-include $(LIB_OBJS:%.o=%.d)

OMV_MOD_DIR := $(USERMOD_DIR)
OMV_PORT_MOD_DIR := $(OMV_MOD_DIR)/../ports/$(PORT)/modules

# Add OpenMV common modules.
SRC_USERMOD += $(wildcard $(OMV_MOD_DIR)/*.c)

# Add OpenMV port-specific modules.
SRC_USERMOD += $(wildcard $(OMV_PORT_MOD_DIR)/*.c)

# Extra module flags.
CFLAGS_USERMOD += -I$(OMV_MOD_DIR) -I$(OMV_PORT_MOD_DIR) -Wno-float-conversion

# Add CubeAI module if enabled.
ifeq ($(MICROPY_PY_CUBEAI), 1)
SRC_USERMOD += $(OMV_MOD_DIR)/../../stm32cubeai/py_st_nn.c
endif

ifeq ($(MICROPY_PY_ULAB), 1)
# NOTE: overrides USERMOD_DIR
# Workaround to build and link ulab.
USERMOD_DIR := $(USERMOD_DIR)/ulab/code
include $(USERMOD_DIR)/micropython.mk
endif

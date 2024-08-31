# Add OpenMV common modules.
OMV_MOD_DIR := $(USERMOD_DIR)
SRC_USERMOD += $(wildcard $(OMV_MOD_DIR)/*.c)
SRC_USERMOD_CXX += $(wildcard $(OMV_MOD_DIR)/*.cpp)

# Add OpenMV port-specific modules.
OMV_PORT_MOD_DIR := $(OMV_MOD_DIR)/../ports/$(PORT)/modules
SRC_USERMOD += $(wildcard $(OMV_PORT_MOD_DIR)/*.c)
SRC_USERMOD_CXX += $(wildcard $(OMV_PORT_MOD_DIR)/*.cpp)

# Extra module flags.
CFLAGS_USERMOD += \
        -I$(OMV_MOD_DIR) \
        -I$(OMV_PORT_MOD_DIR) \
        -Wno-float-conversion

CXXFLAGS_USERMOD += \
        $(CFLAGS_USERMOD) \
        -std=c++11 \
        -fno-rtti \
        -fno-exceptions \
        -fno-use-cxa-atexit \
        -nodefaultlibs \
        -fno-unwind-tables \
        -fpermissive \
        -fno-threadsafe-statics \
        -fmessage-length=0 \
        $(filter-out -std=gnu99,$(CFLAGS))

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

ifeq ($(DEBUG), 0)
# Use a higher optimization level for user C modules.
$(BUILD)/modules/%.o: override CFLAGS += $(USERMOD_OPT)
endif

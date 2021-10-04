# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Top level Makefile

# Set verbosity
ifeq ($(V), 1)
Q =
else
Q = @
MAKEFLAGS += --silent
endif

# Commands
CC      = $(Q)arm-none-eabi-gcc
CXX     = $(Q)arm-none-eabi-g++
AS      = $(Q)arm-none-eabi-as
LD      = $(Q)arm-none-eabi-ld
AR      = $(Q)arm-none-eabi-ar
RM      = $(Q)rm
CPP     = $(Q)arm-none-eabi-cpp
SIZE    = $(Q)arm-none-eabi-size
STRIP   = $(Q)arm-none-eabi-strip -s
OBJCOPY = $(Q)arm-none-eabi-objcopy
OBJDUMP = $(Q)arm-none-eabi-objdump
PYTHON  = $(Q)python
MKDFU   = micropython/tools/dfu.py
PYDFU   = $(Q)../tools/pydfu.py
MKDIR   = $(Q)mkdir
ECHO    = $(Q)@echo
MAKE    = $(Q)make
CAT     = $(Q)cat

# Targets
TARGET ?= OPENMV4
OPENMV = openmv
FIRMWARE = firmware

# Directories
TOP_DIR=$(shell pwd)
BUILD=$(TOP_DIR)/build
FW_DIR=$(BUILD)/bin
OMV_DIR=omv
UVC_DIR=uvc
CM4_DIR=cm4
BOOTLDR_DIR=bootloader
CUBEAI_DIR=stm32cubeai
CMSIS_DIR=hal/cmsis
MICROPY_DIR=micropython
LEPTON_DIR=drivers/lepton
LSM6DS3_DIR=drivers/lsm6ds3
LSM6DSOX_DIR=drivers/lsm6dsox
WINC1500_DIR=drivers/winc1500
MLX90621_DIR=drivers/mlx90621
MLX90640_DIR=drivers/mlx90640
MLX90641_DIR=drivers/mlx90641
PIXART_DIR=drivers/pixart
LIBPDM_DIR=lib/libpdm
TENSORFLOW_DIR=lib/libtf
OMV_BOARD_CONFIG_DIR=$(TOP_DIR)/$(OMV_DIR)/boards/$(TARGET)/
MP_BOARD_CONFIG_DIR=$(TOP_DIR)/$(MICROPY_DIR)/ports/$(PORT)/boards/$(TARGET)/
MPY_LIB_DIR=$(TOP_DIR)/../scripts/libraries
FROZEN_MANIFEST=$(OMV_BOARD_CONFIG_DIR)/manifest.py
OMV_COMMON_DIR=$(TOP_DIR)/$(OMV_DIR)/common

# Debugging/Optimization
ifeq ($(DEBUG), 1)
ROM_TEXT_COMPRESSION = 0
CFLAGS += -Og -ggdb3 -Wno-maybe-uninitialized
else
DEBUG=0
ROM_TEXT_COMPRESSION = 1
CFLAGS += -O2 -DNDEBUG
MPY_CFLAGS += -DMICROPY_ROM_TEXT_COMPRESSION=1
endif

# Enable debug printf
ifeq ($(DEBUG_PRINTF), 1)
CFLAGS += -DOMV_DEBUG_PRINTF
endif

# Enable stack protection
ifeq ($(STACK_PROTECTOR), 1)
CFLAGS += -fstack-protector-all -DSTACK_PROTECTOR
endif

# Enable debug printf
ifeq ($(FB_ALLOC_STATS), 1)
CFLAGS += -DFB_ALLOC_STATS
endif

# Include OpenMV board config first to set the port.
include $(OMV_BOARD_CONFIG_DIR)/omv_boardconfig.mk

# Include MicroPython board config.
#include $(MP_BOARD_CONFIG_DIR)/mpconfigboard.mk

# Additional qstr definitions for OpenMV
OMV_SRC_QSTR := $(wildcard $(TOP_DIR)/$(OMV_DIR)/modules/*.c)
OMV_SRC_QSTR += $(wildcard $(TOP_DIR)/$(OMV_DIR)/ports/$(PORT)/modules/*.c)

# The following command line args are passed to MicroPython's top Makefile.
MICROPY_ARGS = PORT=$(PORT) BOARD=$(TARGET) DEBUG=$(DEBUG) MPY_LIB_DIR=$(MPY_LIB_DIR) MICROPY_ROM_TEXT_COMPRESSION=$(ROM_TEXT_COMPRESSION) USER_C_MODULES=$(TOP_DIR)/$(OMV_DIR) #OMV_SRC_QSTR="$(OMV_SRC_QSTR)"

# Configure additional built-in modules. Note must define both the CFLAGS and the Make command line args.
ifeq ($(MICROPY_PY_SENSOR), 1)
MPY_CFLAGS += -DMICROPY_PY_SENSOR=1
MICROPY_ARGS += MICROPY_PY_SENSOR=1
endif

ifeq ($(MICROPY_PY_WINC1500), 1)
MPY_CFLAGS += -DMICROPY_PY_WINC1500=1
MICROPY_ARGS += MICROPY_PY_WINC1500=1
endif

ifeq ($(MICROPY_PY_IMU), 1)
MPY_CFLAGS += -DMICROPY_PY_IMU=1
MICROPY_ARGS += MICROPY_PY_IMU=1
endif

ifeq ($(MICROPY_PY_ULAB), 1)
MPY_CFLAGS += -DMICROPY_PY_ULAB=1
MPY_CFLAGS += -DULAB_CONFIG_FILE="\"$(OMV_BOARD_CONFIG_DIR)/ulab_config.h\""
MICROPY_ARGS += MICROPY_PY_ULAB=1
endif

ifeq ($(MICROPY_PY_LWIP), 1)
MPY_CFLAGS += -DMICROPY_PY_LWIP=1
MICROPY_ARGS += MICROPY_PY_LWIP=1
endif

ifeq ($(MICROPY_PY_NETWORK_CYW43), 1)
MPY_CFLAGS += -DMICROPY_PY_NETWORK_CYW43=1
MICROPY_ARGS += MICROPY_PY_NETWORK_CYW43=1
endif

ifeq ($(MICROPY_PY_BLUETOOTH), 1)
MPY_CFLAGS += -DMICROPY_PY_BLUETOOTH=1
MICROPY_ARGS += MICROPY_PY_BLUETOOTH=1
endif

ifeq ($(MICROPY_BLUETOOTH_NIMBLE), 1)
MPY_CFLAGS += -DMICROPY_BLUETOOTH_NIMBLE=1
MPY_CFLAGS += -DMICROPY_PY_BLUETOOTH_USE_SYNC_EVENTS=1
MICROPY_ARGS += MICROPY_BLUETOOTH_NIMBLE=1
endif

ifeq ($(MICROPY_PY_AUDIO), 1)
MPY_CFLAGS += -DMICROPY_PY_AUDIO=1
MICROPY_ARGS += MICROPY_PY_AUDIO=1
endif

ifeq ($(MICROPY_PY_MICRO_SPEECH), 1)
MPY_CFLAGS += -DMICROPY_PY_MICRO_SPEECH=1
MICROPY_ARGS += MICROPY_PY_MICRO_SPEECH=1
endif

ifeq ($(MICROPY_PY_LCD), 1)
MPY_CFLAGS += -DMICROPY_PY_LCD=1
MICROPY_ARGS += MICROPY_PY_LCD=1
endif

ifeq ($(MICROPY_PY_TV), 1)
MPY_CFLAGS += -DMICROPY_PY_TV=1
MICROPY_ARGS += MICROPY_PY_TV=1
endif

ifeq ($(MICROPY_PY_BUZZER), 1)
MPY_CFLAGS += -DMICROPY_PY_BUZZER=1
MICROPY_ARGS += MICROPY_PY_BUZZER=1
endif

ifeq ($(CUBEAI), 1)
MPY_CFLAGS += -DMICROPY_PY_CUBEAI=1
MICROPY_ARGS += MICROPY_PY_CUBEAI=1
endif

# Include the port Makefile.
include $(OMV_DIR)/ports/$(PORT)/omv_portconfig.mk

clean:
	$(RM) -fr $(BUILD)

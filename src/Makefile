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
BOOTLDR_DIR=bootloader
CUBEAI_DIR=stm32cubeai
CMSIS_DIR=hal/cmsis
MICROPY_DIR=micropython
MLX_DIR=drivers/mlx
LEPTON_DIR=drivers/lepton
LSM6DS3_DIR=drivers/lsm6ds3
WINC1500_DIR=drivers/winc1500
LIBPDM_DIR=lib/libpdm
TENSORFLOW_DIR=lib/libtf
OMV_BOARD_CONFIG_DIR=$(TOP_DIR)/$(OMV_DIR)/boards/$(TARGET)/
MP_BOARD_CONFIG_DIR=$(TOP_DIR)/$(MICROPY_DIR)/ports/$(PORT)/boards/$(TARGET)/
MPY_LIB_DIR=$(TOP_DIR)/../scripts/libraries
FROZEN_MANIFEST=$(OMV_BOARD_CONFIG_DIR)/manifest.py

# Additional qstr definitions for OpenMV
OMV_QSTR_DEFS = $(TOP_DIR)/$(OMV_DIR)/modules/qstrdefsomv.h

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -Og -ggdb3 -Wno-maybe-uninitialized
else
DEBUG=0
CFLAGS += -O2 -ggdb3 -DNDEBUG
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

# Include the board config to set the port.
include $(OMV_BOARD_CONFIG_DIR)/omv_boardconfig.mk

# MicroPython board config.
#include $(MP_BOARD_CONFIG_DIR)/mpconfigboard.mk

# Include the port Makefile.
include $(OMV_DIR)/ports/$(PORT)/omv_port.mk

clean:
	$(RM) -fr $(BUILD)

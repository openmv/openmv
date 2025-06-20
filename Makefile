# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Top level Makefile

# Set verbosity
ifeq ($(V), 1)
export Q =
else
export Q = @
MAKEFLAGS += --silent
endif

# Default path to LLVM toolchain.
LLVM_PATH ?=/opt/LLVM-ET-Arm-18.1.3-Linux-x86_64/bin/

# Commands
export CC      = $(Q)arm-none-eabi-gcc
export CLANG   = $(Q)$(LLVM_PATH)/clang
export CXX     = $(Q)arm-none-eabi-g++
export AS      = $(Q)arm-none-eabi-as
export LD      = $(Q)arm-none-eabi-ld
export AR      = $(Q)arm-none-eabi-ar
export RM      = $(Q)rm
export CPP     = $(Q)arm-none-eabi-cpp
export SIZE    = $(Q)arm-none-eabi-size
export STRIP   = $(Q)arm-none-eabi-strip -s
export OBJCOPY = $(Q)arm-none-eabi-objcopy
export OBJDUMP = $(Q)arm-none-eabi-objdump
export PYTHON  = $(Q)python3
export MKDFU   = $(MICROPY_DIR)/tools/dfu.py
export PYDFU   = $(Q)tools/pydfu.py
export MKDIR   = $(Q)mkdir
export ECHO    = $(Q)@echo
export MAKE    = $(Q)make
export CAT     = $(Q)cat
export MKROMFS = mkromfs.py

# Targets
export OPENMV ?= openmv
export FIRMWARE ?= firmware
export BOOTLOADER ?= bootloader

# Jlink config
JLINK_INTERFACE ?= swd
JLINK_SPEED ?= 100000
JLINK_DEVICE ?= unspecified
JLINK_GDB_SERVER ?= /opt/JLink/JLinkGDBServer

ifeq ($(TARGET),)
  ifneq ($(MAKECMDGOALS),clean)
    $(error Invalid or no TARGET specified)
  endif
  TARGET=OPENMV4
endif

# Directories
export TOP_DIR=$(shell pwd)
export BUILD=$(TOP_DIR)/build
export TOOLS_DIR=$(TOP_DIR)/tools
export FW_DIR=$(BUILD)/bin
export BOOT_DIR=boot
export PORTS_DIR=ports
export CUBEAI_DIR=cubeai
export CMSIS_DIR=lib/cmsis
export MICROPY_DIR=lib/micropython
export NEMA_DIR=drivers/nema
export LIBPDM_DIR=lib/libpdm
export TENSORFLOW_DIR=lib/tflm
export COMMON_DIR=common
export CYW4343_FW_DIR=drivers/cyw4343/firmware/
export OMV_BOARD_CONFIG_DIR=$(TOP_DIR)/boards/$(TARGET)/
export OMV_PORT_DIR=$(TOP_DIR)/ports/$(PORT)
export MP_BOARD_CONFIG_DIR=$(TOP_DIR)/$(MICROPY_DIR)/ports/$(PORT)/boards/$(TARGET)/
export OMV_LIB_DIR=$(TOP_DIR)/scripts/libraries
export FROZEN_MANIFEST=$(OMV_BOARD_CONFIG_DIR)/manifest.py

# Debugging/Optimization
ifeq ($(DEBUG), 1)
ROM_TEXT_COMPRESSION = 0
CFLAGS += -Og -ggdb3
else
DEBUG=0
ROM_TEXT_COMPRESSION = 1
CFLAGS += -O2 -DNDEBUG
USERMOD_OPT = -O2
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

# Enable timing for some functions.
ifeq ($(PROFILE), 1)
CFLAGS += -DOMV_PROFILE_ENABLE=1
endif


# Include OpenMV board config first to set the port.
include $(OMV_BOARD_CONFIG_DIR)/omv_boardconfig.mk

# Include MicroPython board config.
#include $(MP_BOARD_CONFIG_DIR)/mpconfigboard.mk

# Additional qstr definitions for OpenMV
#OMV_SRC_QSTR := $(wildcard $(TOP_DIR)/modules/*.c)

# The following command line args are passed to MicroPython's top Makefile.
MPY_MKARGS = PORT=$(PORT) BOARD=$(TARGET) DEBUG=$(DEBUG) MICROPY_MANIFEST_OMV_LIB_DIR=$(OMV_LIB_DIR)\
             FROZEN_MANIFEST=$(FROZEN_MANIFEST) OMV_SRC_QSTR="$(OMV_SRC_QSTR)"\
             MICROPY_ROM_TEXT_COMPRESSION=$(ROM_TEXT_COMPRESSION) USER_C_MODULES=$(TOP_DIR)


# Disable broken optimization for CM55.
ifeq ($(CPU),cortex-m55)
CFLAGS += -fdisable-rtl-loop2_doloop
endif

# Configure additional built-in modules. Note must define both the CFLAGS and the Make command line args.
ifeq ($(MICROPY_PY_CSI), 1)
MPY_CFLAGS += -DMICROPY_PY_CSI=1
MPY_MKARGS += MICROPY_PY_CSI=1
endif

ifeq ($(MICROPY_PY_FIR), 1)
MPY_CFLAGS += -DMICROPY_PY_FIR=1
MPY_MKARGS += MICROPY_PY_FIR=1
endif

ifeq ($(MICROPY_PY_WINC1500), 1)
MPY_CFLAGS += -DMICROPY_PY_WINC1500=1
MPY_MKARGS += MICROPY_PY_WINC1500=1
MPY_PENDSV_ENTRIES += PENDSV_DISPATCH_WINC,
endif

ifeq ($(MICROPY_PY_IMU), 1)
MPY_CFLAGS += -DMICROPY_PY_IMU=1
MPY_MKARGS += MICROPY_PY_IMU=1
endif

ifeq ($(MICROPY_PY_BTREE), 1)
MPY_CFLAGS += -DMICROPY_PY_BTREE=1
MPY_MKARGS += MICROPY_PY_BTREE=1
endif

ifeq ($(MICROPY_PY_TOF), 1)
MPY_CFLAGS += -DMICROPY_PY_TOF=1
MPY_MKARGS += MICROPY_PY_TOF=1
endif

ifeq ($(MICROPY_PY_ULAB), 1)
MPY_CFLAGS += -DMICROPY_PY_ULAB=1
MPY_CFLAGS += -DULAB_CONFIG_FILE="\"$(OMV_BOARD_CONFIG_DIR)/ulab_config.h\""
MPY_MKARGS += MICROPY_PY_ULAB=1
endif

ifeq ($(MICROPY_PY_AUDIO), 1)
MPY_CFLAGS += -DMICROPY_PY_AUDIO=1
MPY_MKARGS += MICROPY_PY_AUDIO=1
endif

ifeq ($(MICROPY_PY_DISPLAY), 1)
MPY_CFLAGS += -DMICROPY_PY_DISPLAY=1
MPY_MKARGS += MICROPY_PY_DISPLAY=1
endif

ifeq ($(MICROPY_PY_TV), 1)
MPY_CFLAGS += -DMICROPY_PY_TV=1
MPY_MKARGS += MICROPY_PY_TV=1
endif

ifeq ($(MICROPY_PY_BUZZER), 1)
MPY_CFLAGS += -DMICROPY_PY_BUZZER=1
MPY_MKARGS += MICROPY_PY_BUZZER=1
endif

ifeq ($(CUBEAI), 1)
MPY_CFLAGS += -DMICROPY_PY_CUBEAI=1
MPY_MKARGS += MICROPY_PY_CUBEAI=1
endif

ifeq ($(MICROPY_PY_ML), 1)
MPY_CFLAGS += -DMICROPY_PY_ML=1
MPY_MKARGS += MICROPY_PY_ML=1
endif

ifeq ($(MICROPY_PY_ML_TFLM), 1)
MPY_CFLAGS += -DMICROPY_PY_ML_TFLM=1
MPY_MKARGS += MICROPY_PY_ML_TFLM=1
endif

ifeq ($(MICROPY_PY_ML_STAI), 1)
MPY_CFLAGS += -DMICROPY_PY_ML_STAI=1
MICROPY_ARGS += MICROPY_PY_ML_STAI=1
endif

MPY_PENDSV_ENTRIES := $(shell echo $(MPY_PENDSV_ENTRIES) | tr -d '[:space:]')
MPY_CFLAGS += -DMICROPY_BOARD_PENDSV_ENTRIES="$(MPY_PENDSV_ENTRIES)"
MPY_CFLAGS += -DMP_CONFIGFILE=\<$(OMV_PORT_DIR)/omv_mpconfigport.h\>

# Include the port Makefile.
include $(OMV_PORT_DIR)/omv_portconfig.mk

# Export variables for sub-make.
export PORT
export MCU
export TARGET
export CFLAGS
export AFLAGS
export LDFLAGS
export MPY_CFLAGS
export MPY_MKARGS
export USERMOD_OPT

clean:
	$(RM) -fr $(BUILD)

size:
ifeq ($(OMV_ENABLE_BL), 1)
	$(SIZE) --format=SysV $(FW_DIR)/$(BOOTLOADER).elf
endif
	$(SIZE) --format=SysV $(FW_DIR)/$(FIRMWARE).elf

jlink:
	${JLINK_GDB_SERVER} -speed ${JLINK_SPEED} -nogui \
        -if ${JLINK_INTERFACE} -halt -cpu cortex-m \
		-device ${JLINK_DEVICE} -novd ${JLINK_SCRIPT}

submodules:
	$(MAKE) -C $(MICROPY_DIR)/ports/$(PORT) BOARD=$(TARGET) submodules

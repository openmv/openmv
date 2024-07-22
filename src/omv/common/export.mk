# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Tools, directories and common variables that need to be exported when calling
# sub-Makefiles.

# Export Commands
export Q
export CC
export CXX
export AS
export LD
export AR
export RM
export CPP
export SIZE
export STRIP
export OBJCOPY
export OBJDUMP
export PYTHON
export MKDIR
export ECHO
export MAKE
export CAT
export TFLITE2C

# Export Flags
export CFLAGS
export AFLAGS
export LDFLAGS
export MPY_CFLAGS

# Export variables
export TARGET
export FIRMWARE
export SYSTEM
export STARTUP
export MICROPY_ARGS
export VELA_ARGS
export FROZEN_MANIFEST

# Export board config variables
export PORT
export HAL_DIR
export MCU
export MCU_SERIES
export MCU_VARIANT
export MCU_CORE

# Export Directories
export TOP_DIR
export BUILD
export TOOLS
export FW_DIR
export OMV_DIR
export CMSIS_DIR
export MICROPY_DIR
export LEPTON_DIR
export LSM6DS3_DIR
export LSM6DSOX_DIR
export WINC1500_DIR
export MLX90621_DIR
export MLX90640_DIR
export MLX90641_DIR
export VL53L5CX_DIR
export PIXART_DIR
export DISPLAY_DIR
export LIBPDM_DIR
export TENSORFLOW_DIR
export OMV_BOARD_CONFIG_DIR
export OMV_PORT_DIR
export MP_BOARD_CONFIG_DIR
export OMV_LIB_DIR
export OMV_COMMON_DIR

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

# include this first to set the port
include $(OMV_BOARD_CONFIG_DIR)/omv_boardconfig.mk
include $(OMV_DIR)/ports/$(PORT)/omv_portconfig.mk
#include $(MP_BOARD_CONFIG_DIR)/mpconfigboard.mk

# Compiler Flags
CFLAGS += -std=gnu99 -Wall -Werror -Warray-bounds -mthumb -nostartfiles -fdata-sections -ffunction-sections
CFLAGS += -D$(MCU) -D$(CFLAGS_MCU) -D$(ARM_MATH) -DARM_NN_TRUNCATE\
          -fsingle-precision-constant -Wdouble-promotion -mcpu=$(CPU) -mtune=$(CPU) -mfpu=$(FPU) -mfloat-abi=hard
CFLAGS += -D__FPU_PRESENT=1 -D__VFP_FP__ -DUSE_USB_FS -DUSE_DEVICE_MODE -DUSE_USB_OTG_ID=0 -DHSE_VALUE=$(OMV_HSE_VALUE)\
          -D$(TARGET) -DSTM32_HAL_H=$(HAL_INC) -DVECT_TAB_OFFSET=$(VECT_TAB_OFFSET) -DMAIN_APP_ADDR=$(MAIN_APP_ADDR)
CFLAGS += $(OMV_BOARD_EXTRA_CFLAGS)

HAL_CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/include/
HAL_CFLAGS += $(OMV_PORT_HAL_CFLAGS)

MPY_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)
MPY_CFLAGS += -I$(BUILD)/$(MICROPY_DIR)/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/mp-readline
MPY_CFLAGS += $(OMV_PORT_MPY_CFLAGS)

# In addition to CFLAGS, the following options are needed to set varilables
# in MicroPython's Makefile, to enable or disable compiling additional modules.
MICROPY_ARGS = BOARD=$(TARGET) DEBUG=$(DEBUG) QSTR_DEFS="$(OMV_QSTR_DEFS)" MPY_LIB_DIR=$(MPY_LIB_DIR)

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
ifeq ($(MICROPY_PY_AUDIO), 1)
MPY_CFLAGS += -DMICROPY_PY_AUDIO=1
MICROPY_ARGS += MICROPY_PY_AUDIO=1
endif
ifeq ($(MICROPY_PY_MICRO_SPEECH), 1)
MPY_CFLAGS += -DMICROPY_PY_MICRO_SPEECH=1
MICROPY_ARGS += MICROPY_PY_MICRO_SPEECH=1
endif

OMV_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/alloc/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/common/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/imlib/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/modules/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/sensors/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/ports/$(PORT)/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/ports/$(PORT)/modules/

OMV_CFLAGS += -I$(TOP_DIR)/$(LEPTON_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(MLX_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(LSM6DS3_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(TENSORFLOW_DIR)/$(CPU)/
OMV_CFLAGS += -I$(TOP_DIR)/$(WINC1500_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(LIBPDM_DIR)/

ifeq ($(OMV_ENABLE_BL), 1)
BL_CFLAGS  := $(CFLAGS) $(HAL_CFLAGS)
BL_CFLAGS  += -I$(OMV_BOARD_CONFIG_DIR)
BL_CFLAGS  += -I$(TOP_DIR)/$(BOOTLDR_DIR)/include/
# Linker Flags
BL_LDFLAGS = -mcpu=$(CPU) -mabi=aapcs-linux -mthumb -mfpu=$(FPU) -mfloat-abi=hard\
               -nostdlib -Wl,--gc-sections -Wl,-T$(BUILD)/$(BOOTLDR_DIR)/stm32fxxx.lds
endif

ifeq ($(OMV_ENABLE_UVC), 1)
UVC_CFLAGS := $(CFLAGS) $(HAL_CFLAGS)
UVC_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
UVC_CFLAGS += -I$(TOP_DIR)/$(UVC_DIR)/include/
UVC_CFLAGS += $(OMV_CFLAGS) $(MPY_CFLAGS)
# Linker Flags
UVC_LDFLAGS = -mcpu=$(CPU) -mabi=aapcs-linux -mthumb -mfpu=$(FPU) -mfloat-abi=hard\
               -nostdlib -Wl,--gc-sections -Wl,-T$(BUILD)/$(UVC_DIR)/stm32fxxx.lds
endif

CFLAGS += $(HAL_CFLAGS) $(MPY_CFLAGS) $(OMV_CFLAGS)

# Linker Flags
LDFLAGS = -mcpu=$(CPU) -mabi=aapcs-linux -mthumb -mfpu=$(FPU) -mfloat-abi=hard\
          -nostdlib -Wl,--gc-sections -Wl,-T$(BUILD)/$(LDSCRIPT).lds

#------------- Libraries ----------------#
LIBS += $(TOP_DIR)/$(TENSORFLOW_DIR)/$(CPU)/libtf*.a
ifeq ($(MICROPY_PY_AUDIO), 1)
LIBS += $(TOP_DIR)/$(LIBPDM_DIR)/libPDMFilter_CM7_GCC_wc32.a
endif

#------------- Firmware Objects ----------------#
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/CommonTables/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/FastMathFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/MatrixFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/nn/ActivationFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/nn/FullyConnectedFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/nn/NNSupportFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/nn/PoolingFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/nn/SoftmaxFunctions/*.o)
ifeq ($(CUBEAI),)
# Conflicts with NetworkRuntime library.
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/nn/ConvolutionFunctions/*.o)
endif

FIRM_OBJ += $(wildcard $(BUILD)/$(HAL_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(LEPTON_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MLX_DIR)/src/*.o)
ifeq ($(MICROPY_PY_IMU), 1)
FIRM_OBJ += $(wildcard $(BUILD)/$(LSM6DS3_DIR)/src/*.o)
endif
ifeq ($(MICROPY_PY_WINC1500), 1)
FIRM_OBJ += $(wildcard $(BUILD)/$(WINC1500_DIR)/src/*.o)
endif

#------------- OpenMV Objects ----------------#
FIRM_OBJ += $(addprefix $(BUILD)/$(CMSIS_DIR)/src/, \
	$(STARTUP).o                \
	$(SYSTEM).o                 \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/alloc/, \
	xalloc.o                    \
	fb_alloc.o                  \
	umm_malloc.o                \
   )

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/common/, \
	array.o                     \
	ff_wrapper.o                \
	ini.o                       \
	ringbuf.o                   \
	trace.o                     \
	mutex.o                     \
   )

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/sensors/,   \
	ov2640.o                    \
	ov5640.o                    \
	ov7690.o                    \
	ov7725.o                    \
	ov9650.o                    \
	mt9v034.o                   \
	lepton.o                    \
	hm01b0.o                    \
   )

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/modules/,   \
	py_clock.o                  \
	py_gif.o                    \
	py_helper.o                 \
	py_image.o                  \
	py_mjpeg.o                  \
	py_omv.o                    \
	py_sensor.o                 \
	py_tf.o                     \
   )

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/imlib/, \
	agast.o                     \
	apriltag.o                  \
	binary.o                    \
	blob.o                      \
	bmp.o                       \
	clahe.o                     \
	collections.o               \
	dmtx.o                      \
	draw.o                      \
	edge.o                      \
	eye.o                       \
	fast.o                      \
	fft.o                       \
	filter.o                    \
	fmath.o                     \
	font.o                      \
	framebuffer.o               \
	fsort.o                     \
	gif.o                       \
	haar.o                      \
	hog.o                       \
	hough.o                     \
	imlib.o                     \
	integral.o                  \
	integral_mw.o               \
	jpeg.o                      \
	kmeans.o                    \
	lab_tab.o                   \
	lbp.o                       \
	line.o                      \
	lsd.o                       \
	mathop.o                    \
	mjpeg.o                     \
	orb.o                       \
	phasecorrelation.o          \
	point.o                     \
	pool.o                      \
	ppm.o                       \
	qrcode.o                    \
	qsort.o                     \
	rainbow_tab.o               \
	rectangle.o                 \
	selective_search.o          \
	sincos_tab.o                \
	stats.o                     \
	template.o                  \
	xyz_tab.o                   \
	yuv_tab.o                   \
	zbar.o                      \
   )

FIRM_OBJ += $(wildcard $(BUILD)/$(OMV_DIR)/ports/$(PORT)/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(OMV_DIR)/ports/$(PORT)/modules/*.o)

#------------- MicroPy Objects -------------------#
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/py/*.o)
FIRM_OBJ += $(OMV_PORT_MPY_OBJ)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/boards/$(TARGET)/*.o)

ifeq ($(MICROPY_PY_ULAB), 1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/extmod/ulab/,\
	code/fft.o          \
	code/linalg.o       \
	code/ndarray.o      \
	code/numerical.o    \
	code/poly.o         \
	code/ulab.o         \
	code/vectorise.o    \
	code/create.o    	\
	code/approx.o		\
	code/filter.o		\
	code/compare.o		\
	code/extras.o		\
	)
endif

ifeq ($(MICROPY_PY_LWIP), 1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	sdio.o                     \
	lib/lwip/src/core/*.o      \
	lib/lwip/src/core/*/*.o    \
	lib/lwip/src/netif/*.o     \
	lib/lwip/src/apps/*/*.o    \
	extmod/modlwip.o           \
	extmod/moduwebsocket.o     \
	extmod/modwebrepl.o        \
	)
endif

ifeq ($(MICROPY_PY_NETWORK_CYW43), 1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	drivers/cyw43/*.o          \
	extmod/network_cyw43.o     \
	)
LIBS += $(MICROPY_DIR)/drivers/cyw43/libcyw43.a
endif

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/oofatfs/,\
	ff.o                \
	ffunicode.o         \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/drivers/,\
	bus/softspi.o       \
	dht/dht.o           \
	memory/spiflash.o   \
	)

#------------- CubeAI Objects -------------------#
ifeq ($(CUBEAI), 1)
include $(TOP_DIR)/stm32cubeai/cube.mk
endif

ifeq ($(OMV_ENABLE_BL), 1)
BOOTLOADER = bootloader
# Bootloader object files
BOOT_OBJ += $(wildcard $(BUILD)/$(BOOTLDR_DIR)/src/*.o)
BOOT_OBJ += $(wildcard $(BUILD)/$(HAL_DIR)/src/*.o)
BOOT_OBJ += $(addprefix $(BUILD)/$(CMSIS_DIR)/src/,\
	$(STARTUP).o                                \
	$(SYSTEM).o                                 \
	)
endif

ifeq ($(OMV_ENABLE_UVC), 1)
UVC = uvc
# UVC object files
UVC_OBJ += $(wildcard $(BUILD)/$(UVC_DIR)/src/*.o)
UVC_OBJ += $(wildcard $(BUILD)/$(HAL_DIR)/src/*.o)
UVC_OBJ += $(addprefix $(BUILD)/$(CMSIS_DIR)/src/,\
	$(STARTUP).o                                \
	$(SYSTEM).o                                 \
	)

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/alloc/, \
	fb_alloc.o                              \
	)

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/common/, \
	array.o                                 \
	trace.o                                 \
	mutex.o                                 \
	)

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/sensors/, \
	ov2640.o                                \
	ov5640.o                                \
	ov7690.o                                \
	ov7725.o                                \
	ov9650.o                                \
	mt9v034.o                               \
	lepton.o                                \
	hm01b0.o                                \
	)

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/imlib/,\
	lab_tab.o                               \
	xyz_tab.o                               \
	yuv_tab.o                               \
	rainbow_tab.o                           \
	jpeg.o                                  \
	fmath.o                                 \
	imlib.o                                 \
	framebuffer.o                           \
	)

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/ports/stm32/,\
	sensor.o                                \
	stm32fxxx_hal_msp.o                     \
	soft_i2c.o                              \
	cambus.o                                \
	)

UVC_OBJ += $(wildcard $(BUILD)/$(LEPTON_DIR)/src/*.o)
UVC_OBJ += $(wildcard $(BUILD)/$(MLX_DIR)/src/*.o)
ifeq ($(MICROPY_PY_IMU), 1)
UVC_OBJ += $(wildcard $(BUILD)/$(LSM6DS3_DIR)/src/*.o)
endif
endif

###################################################
#Export Variables
export Q
export CC
export AS
export LD
export AR
export SIZE
export OBJCOPY
export OBJDUMP
export MKDIR
export ECHO
export CFLAGS
export LDFLAGS
export TOP_DIR
export BUILD
export TARGET
export STARTUP
export SYSTEM
export FROZEN_MANIFEST
export PORT
###################################################
all: $(OPENMV)

$(BUILD):
	$(MKDIR) -p $@

$(FW_DIR):
	$(MKDIR) -p $@

FIRMWARE_OBJS: | $(BUILD) $(FW_DIR)
	$(MAKE)  -C $(CMSIS_DIR)                 BUILD=$(BUILD)/$(CMSIS_DIR)    CFLAGS="$(CFLAGS) -fno-strict-aliasing -MMD"
	$(MAKE)  -C $(MICROPY_DIR)/ports/$(PORT) BUILD=$(BUILD)/$(MICROPY_DIR)  $(MICROPY_ARGS)
	$(MAKE)  -C $(HAL_DIR)                   BUILD=$(BUILD)/$(HAL_DIR)      CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(LEPTON_DIR)                BUILD=$(BUILD)/$(LEPTON_DIR)   CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MLX_DIR)                   BUILD=$(BUILD)/$(MLX_DIR)      CFLAGS="$(CFLAGS) -MMD"
ifeq ($(MICROPY_PY_IMU), 1)
	$(MAKE)  -C $(LSM6DS3_DIR)              BUILD=$(BUILD)/$(LSM6DS3_DIR)  CFLAGS="$(CFLAGS) -MMD"
endif
ifeq ($(MICROPY_PY_WINC1500), 1)
	$(MAKE)  -C $(WINC1500_DIR)             BUILD=$(BUILD)/$(WINC1500_DIR) CFLAGS="$(CFLAGS) -MMD"
endif
	$(MAKE)  -C $(OMV_DIR)                  BUILD=$(BUILD)/$(OMV_DIR)      CFLAGS="$(CFLAGS) -MMD"
ifeq ($(CUBEAI), 1)
	$(MAKE)  -C $(CUBEAI_DIR)               BUILD=$(BUILD)/$(CUBEAI_DIR)   CFLAGS="$(CFLAGS) -fno-strict-aliasing -MMD"
endif
ifeq ($(OMV_ENABLE_UVC), 1)
UVC_OBJS: FIRMWARE_OBJS
	$(MAKE)  -C $(UVC_DIR)                  BUILD=$(BUILD)/$(UVC_DIR)      CFLAGS="$(UVC_CFLAGS) -MMD"
endif
ifeq ($(OMV_ENABLE_BL), 1)
BOOTLOADER_OBJS: FIRMWARE_OBJS
	$(MAKE)  -C $(BOOTLDR_DIR)              BUILD=$(BUILD)/$(BOOTLDR_DIR)  CFLAGS="$(BL_CFLAGS) -MMD"
endif

# This target generates the main/app firmware image located at 0x08010000
$(FIRMWARE): FIRMWARE_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(OMV_DIR)/ports/$(PORT)/$(LDSCRIPT).ld.S > $(BUILD)/$(LDSCRIPT).lds
	$(CC) $(LDFLAGS) $(FIRM_OBJ) -o $(FW_DIR)/$(FIRMWARE).elf $(LIBS) -lgcc
	$(OBJCOPY) -Obinary -R .big_const* $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin
	$(PYTHON) $(MKDFU) -D $(DFU_DEVICE) -b $(MAIN_APP_ADDR):$(FW_DIR)/$(FIRMWARE).bin $(FW_DIR)/$(FIRMWARE).dfu

ifeq ($(OMV_ENABLE_BL), 1)
# This target generates the bootloader.
$(BOOTLOADER): FIRMWARE_OBJS BOOTLOADER_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(BOOTLDR_DIR)/stm32fxxx.ld.S > $(BUILD)/$(BOOTLDR_DIR)/stm32fxxx.lds
	$(CC) $(BL_LDFLAGS) $(BOOT_OBJ) -o $(FW_DIR)/$(BOOTLOADER).elf -lgcc
	$(OBJCOPY) -Obinary $(FW_DIR)/$(BOOTLOADER).elf $(FW_DIR)/$(BOOTLOADER).bin
	$(PYTHON) $(MKDFU) -D $(DFU_DEVICE) -b 0x08000000:$(FW_DIR)/$(BOOTLOADER).bin $(FW_DIR)/$(BOOTLOADER).dfu
endif

ifeq ($(OMV_ENABLE_UVC), 1)
# This target generates the UVC firmware.
$(UVC): FIRMWARE_OBJS UVC_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(UVC_DIR)/stm32fxxx.ld.S > $(BUILD)/$(UVC_DIR)/stm32fxxx.lds
	$(CC) $(UVC_LDFLAGS) $(UVC_OBJ) -o $(FW_DIR)/$(UVC).elf -lgcc
	$(OBJCOPY) -Obinary $(FW_DIR)/$(UVC).elf $(FW_DIR)/$(UVC).bin
	$(PYTHON) $(MKDFU) -D $(DFU_DEVICE) -b $(MAIN_APP_ADDR):$(FW_DIR)/$(UVC).bin $(FW_DIR)/$(UVC).dfu
endif

# This target generates the uvc, bootloader and firmware images.
$(OPENMV): $(BOOTLOADER) $(UVC) $(FIRMWARE)
ifeq ($(OMV_ENABLE_BL), 1)
	$(SIZE) $(FW_DIR)/$(BOOTLOADER).elf
endif
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

size:
ifeq ($(OMV_ENABLE_BL), 1)
	$(SIZE) --format=SysV $(FW_DIR)/$(BOOTLOADER).elf
endif
	$(SIZE) --format=SysV $(FW_DIR)/$(FIRMWARE).elf

clean:
	$(RM) -fr $(BUILD)

# Flash the bootloader
flash_boot::
	$(PYDFU) -u $(FW_DIR)/$(BOOTLOADER).dfu

# Flash the main firmware image
flash_image::
	$(PYDFU) -u $(FW_DIR)/$(FIRMWARE).dfu

# Flash the bootloader using dfu_util
flash_boot_dfu_util::
	dfu-util -a 0 -d $(DFU_DEVICE) -D $(FW_DIR)/$(BOOTLOADER).dfu

# Flash the main firmware image using dfu_util
flash_image_dfu_util::
	dfu-util -a 0 -d $(DFU_DEVICE) -D $(FW_DIR)/$(FIRMWARE).dfu


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
BOOTLOADER = bootloader
UVC = uvc
FIRMWARE = firmware

# Directories
TOP_DIR=$(shell pwd)
BUILD=$(TOP_DIR)/build
FW_DIR=$(BUILD)/bin
CMSIS_DIR=cmsis
STUSB_DIR=stusb
MICROPY_DIR=micropython
OMV_DIR=omv
LEPTON_DIR=lepton
MLX_DIR=mlx
ST_DIR=st
TENSORFLOW_DIR=libtf
WINC1500_DIR=winc1500
BOOTLDR_DIR=bootloader
UVC_DIR=uvc
CUBEAI_DIR=stm32cubeai
LIBPDM_DIR=libpdm
OMV_BOARD_CONFIG_DIR=$(TOP_DIR)/$(OMV_DIR)/boards/$(TARGET)/
MP_BOARD_CONFIG_DIR=$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/boards/$(TARGET)/

# Additional qstr definitions for OpenMV
OMV_QSTR_DEFS = $(TOP_DIR)/$(OMV_DIR)/py/qstrdefsomv.h

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb3
else
DEBUG=0
CFLAGS += -O2 -ggdb3 -DNDEBUG
endif

# OpenMV1 is No longer supported
ifeq ($(TARGET), OPENMV1)
BRACKET = :)
$(error "This camera is no longer supported :O Have fun hacking the code $(BRACKET)")
endif

# Enable debug printf
ifeq ($(DEBUG_PRINTF), 1)
CFLAGS += -DDEBUG_PRINTF
endif

# Enable stack protection
ifeq ($(STACK_PROTECTOR), 1)
CFLAGS += -fstack-protector-all -DSTACK_PROTECTOR
endif

# Enable debug printf
ifeq ($(FB_ALLOC_STATS), 1)
CFLAGS += -DFB_ALLOC_STATS
endif

# Compiler Flags
include $(MP_BOARD_CONFIG_DIR)/mpconfigboard.mk
include $(OMV_BOARD_CONFIG_DIR)/omv_boardconfig.mk
CFLAGS += -std=gnu99 -Wall -Werror -Warray-bounds -mthumb -nostartfiles -fdata-sections -ffunction-sections
CFLAGS += -D$(MCU) -D$(CFLAGS_MCU) -D$(ARM_MATH) -DARM_NN_TRUNCATE\
          -fsingle-precision-constant -Wdouble-promotion -mcpu=$(CPU) -mtune=$(CPU) -mfpu=$(FPU) -mfloat-abi=hard
CFLAGS += -D__FPU_PRESENT=1 -D__VFP_FP__ -DUSE_USB_FS -DUSE_DEVICE_MODE -DUSE_USB_OTG_ID=0 -DHSE_VALUE=$(OMV_HSE_VALUE)\
          -D$(TARGET) -DSTM32_HAL_H=$(HAL_INC) -DVECT_TAB_OFFSET=$(VECT_TAB_OFFSET) -DMAIN_APP_ADDR=$(MAIN_APP_ADDR)
CFLAGS += $(OMV_BOARD_EXTRA_CFLAGS) -DOPENMV

ST_CFLAGS += -I. -Iinclude
ST_CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/
ST_CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/st
ST_CFLAGS += -I$(TOP_DIR)/$(STHAL_DIR)/include/
ST_CFLAGS += -I$(TOP_DIR)/$(STHAL_DIR)/include/Legacy/

MP_CFLAGS += -I$(BUILD)/$(MICROPY_DIR)/
MP_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/
MP_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py/
MP_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/mp-readline
MP_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs
MP_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/lwip/src/include/
MP_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/
MP_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/usbdev/core/inc/
MP_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/usbdev/class/inc/
MP_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/lwip_inc/
MP_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)

# In addition to CFLAGS, the following options are needed to set varilables
# in MicroPython's Makefile, to enable or disable compiling additional modules.
MICROPY_ARGS = BOARD=$(TARGET) DEBUG=$(DEBUG) QSTR_DEFS="$(OMV_QSTR_DEFS)"
ifeq ($(MICROPY_PY_WINC1500), 1)
MP_CFLAGS += -DMICROPY_PY_WINC1500=1
MICROPY_ARGS += MICROPY_PY_WINC1500=1
endif
ifeq ($(MICROPY_PY_IMU), 1)
MP_CFLAGS += -DMICROPY_PY_IMU=1
MICROPY_ARGS += MICROPY_PY_IMU=1
endif
ifeq ($(MICROPY_PY_ULAB), 1)
MP_CFLAGS += -DMICROPY_PY_ULAB=1
MICROPY_ARGS += MICROPY_PY_ULAB=1
endif
ifeq ($(MICROPY_PY_LWIP), 1)
MP_CFLAGS += -DMICROPY_PY_LWIP=1
MICROPY_ARGS += MICROPY_PY_LWIP=1
endif
ifeq ($(MICROPY_PY_NETWORK_CYW43), 1)
MP_CFLAGS += -DMICROPY_PY_NETWORK_CYW43=1
MICROPY_ARGS += MICROPY_PY_NETWORK_CYW43=1
endif
ifeq ($(MICROPY_PY_AUDIO), 1)
MP_CFLAGS += -DMICROPY_PY_AUDIO=1
MICROPY_ARGS += MICROPY_PY_AUDIO=1
endif
ifeq ($(MICROPY_PY_MICRO_SPEECH), 1)
MP_CFLAGS += -DMICROPY_PY_MICRO_SPEECH=1
MICROPY_ARGS += MICROPY_PY_MICRO_SPEECH=1
endif

OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/py/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/nn/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/img/
OMV_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/$(LEPTON_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(MLX_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(ST_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(TENSORFLOW_DIR)/$(CPU)/
OMV_CFLAGS += -I$(TOP_DIR)/$(WINC1500_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(LIBPDM_DIR)/

UVC_CFLAGS = $(CFLAGS)
UVC_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
UVC_CFLAGS += -I$(TOP_DIR)/$(UVC_DIR)/include/
ifeq ($(MICROPY_PY_IMU), 1)
# UVC firmware needs the IMU driver.
UVC_CFLAGS += -DMICROPY_PY_IMU=1
endif

BOOTLDR_CFLAGS = $(CFLAGS)
BOOTLDR_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
BOOTLDR_CFLAGS += -I$(TOP_DIR)/$(BOOTLDR_DIR)/include/

CFLAGS += $(ST_CFLAGS) $(MP_CFLAGS) $(OMV_CFLAGS)

# Linker Flags
LDFLAGS = -mcpu=$(CPU) -mabi=aapcs-linux -mthumb -mfpu=$(FPU) -mfloat-abi=hard\
          -nostdlib -Wl,--gc-sections -Wl,-T$(BUILD)/stm32fxxx.lds

# Linker Flags
BOOTLDR_LDFLAGS = -mcpu=$(CPU) -mabi=aapcs-linux -mthumb -mfpu=$(FPU) -mfloat-abi=hard\
               -nostdlib -Wl,--gc-sections -Wl,-T$(BUILD)/$(BOOTLDR_DIR)/stm32fxxx.lds

# Linker Flags
UVC_LDFLAGS = -mcpu=$(CPU) -mabi=aapcs-linux -mthumb -mfpu=$(FPU) -mfloat-abi=hard\
               -nostdlib -Wl,--gc-sections -Wl,-T$(BUILD)/$(UVC_DIR)/stm32fxxx.lds

#------------- Libraries ----------------#
LIBS = -lgcc
ifeq ($(MICROPY_PY_AUDIO), 1)
LIBS += $(TOP_DIR)/$(LIBPDM_DIR)/libPDMFilter_CM7_GCC_wc32.a
endif
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

FIRM_OBJ += $(wildcard $(BUILD)/$(STHAL_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(LEPTON_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MLX_DIR)/src/*.o)
ifeq ($(MICROPY_PY_IMU), 1)
FIRM_OBJ += $(wildcard $(BUILD)/$(ST_DIR)/src/*.o)
endif
ifeq ($(MICROPY_PY_WINC1500), 1)
FIRM_OBJ += $(wildcard $(BUILD)/$(WINC1500_DIR)/src/*.o)
endif

#------------- OpenMV Objects ----------------#
FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/, \
	main.o                                  \
	xalloc.o                                \
	fb_alloc.o                              \
	umm_malloc.o                            \
	ff_wrapper.o                            \
	ini.o                                   \
	framebuffer.o                           \
	array.o                                 \
	usbdbg.o                                \
	wifidbg.o                               \
	cambus.o                                \
	ov2640.o                                \
	ov5640.o                                \
	ov7690.o                                \
	ov7725.o                                \
	ov9650.o                                \
	mt9v034.o                               \
	lepton.o                                \
	hm01b0.o                                \
	sensor.o                                \
	stm32fxxx_hal_msp.o                     \
	soft_i2c.o                              \
	mutex.o                                 \
	trace.o                                 \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/img/,\
	binary.o                                \
	blob.o                                  \
	clahe.o                                 \
	draw.o                                  \
	qrcode.o                                \
	apriltag.o                              \
	dmtx.o                                  \
	zbar.o                                  \
	fmath.o                                 \
	fsort.o                                 \
	qsort.o                                 \
	fft.o                                   \
	filter.o                                \
	haar.o                                  \
	imlib.o                                 \
	collections.o                           \
	stats.o                                 \
	integral.o                              \
	integral_mw.o                           \
	kmeans.o                                \
	lab_tab.o                               \
	xyz_tab.o                               \
	yuv_tab.o                               \
	rainbow_tab.o                           \
	mathop.o                                \
	pool.o                                  \
	point.o                                 \
	rectangle.o                             \
	bmp.o                                   \
	ppm.o                                   \
	gif.o                                   \
	mjpeg.o                                 \
	fast.o                                  \
	agast.o                                 \
	orb.o                                   \
	template.o                              \
	phasecorrelation.o                      \
	font.o                                  \
	jpeg.o                                  \
	lbp.o                                   \
	eye.o                                   \
	hough.o                                 \
	line.o                                  \
	lsd.o                                   \
	sincos_tab.o                            \
	edge.o                                  \
	hog.o                                   \
	selective_search.o                      \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/nn/,\
	nn.o                                    \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/py/, \
	py_helper.o                             \
	py_omv.o                                \
	py_sensor.o                             \
	py_image.o                              \
	py_time.o                               \
	py_lcd.o                                \
	py_tv.o                                 \
	py_fir.o                                \
	py_gif.o                                \
	py_mjpeg.o                              \
	py_winc.o                               \
	py_cpufreq.o                            \
	py_nn.o                                 \
	py_tf.o                                 \
	py_imu.o                                \
	py_audio.o                              \
	py_micro_speech.o                       \
	)


FIRM_OBJ += $(addprefix $(BUILD)/$(CMSIS_DIR)/src/st/,\
	$(STARTUP).o                                \
	system_stm32fxxx.o                          \
	)

#------------- MicroPy Core -------------------#
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/py/*.o)

#------------- MicroPy Objects ----------------#
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	stm32_it.o              \
	usbd_conf.o             \
	usbd_desc.o             \
	usbd_cdc_interface.o    \
	usbd_hid_interface.o    \
	usbd_msc_interface.o    \
	pendsv.o                \
	bufhelper.o             \
	usb.o                   \
	gccollect.o             \
	help.o                  \
	flash.o                 \
	flashbdev.o             \
	spibdev.o               \
	storage.o               \
	rtc.o                   \
	irq.o                   \
	adc.o                   \
	dac.o                   \
	dma.o                   \
	uart.o                  \
	systick.o               \
	powerctrl.o             \
	i2c.o                   \
	pyb_i2c.o               \
	spi.o                   \
	qspi.o                  \
	pyb_spi.o               \
	can.o                   \
	fdcan.o                 \
	pyb_can.o               \
	pin.o                   \
	pin_defs_stm32.o        \
	pin_named_pins.o        \
	pins_$(TARGET).o        \
	timer.o                 \
	servo.o                 \
	rng.o                   \
	led.o                   \
	wdt.o                   \
	mphalport.o             \
	sdcard.o                \
	sdram.o                 \
	fatfs_port.o            \
	extint.o                \
	modpyb.o                \
	modstm.o                \
	moduos.o                \
	modutime.o              \
	modusocket.o            \
	modnetwork.o            \
	modmachine.o            \
	machine_i2c.o           \
	machine_spi.o           \
	machine_uart.o          \
	machine_adc.o           \
	machine_timer.o         \
	pybthread.o             \
	mpthreadport.o          \
	posix_helpers.o         \
	softtimer.o             \
	mbedtls/mbedtls_port.o  \
	)

# board object files
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/boards/$(TARGET)/*.o)

# object file for frozen bytecode (frozen .mpy files)
ifneq ($(FROZEN_MPY_DIR),)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,frozen_mpy.o)
endif

#------------- MicroPy Objects ----------------#
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/,\
	utils/mpirq.o               \
	utils/pyexec.o              \
	utils/printf.o              \
	utils/interrupt_char.o      \
	utils/sys_stdio_mphal.o     \
	utils/gchelper_m3.o         \
	libc/string0.o              \
	netutils/*.o                \
	timeutils/timeutils.o       \
	berkeley-db-1.xx/btree/*.o  \
	berkeley-db-1.xx/mpool/*.o  \
	embed/abort_.o              \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/libm/,\
	math.o              \
	roundf.o            \
	asinfacosf.o        \
	atanf.o             \
	atan2f.o            \
	fmodf.o             \
	log1pf.o            \
	acoshf.o            \
	asinhf.o            \
	atanhf.o            \
	kf_rem_pio2.o       \
	kf_sin.o            \
	kf_cos.o            \
	kf_tan.o            \
	ef_rem_pio2.o       \
	erf_lgamma.o        \
	sf_sin.o            \
	sf_cos.o            \
	sf_tan.o            \
	sf_frexp.o          \
	sf_modf.o           \
	sf_ldexp.o          \
	sf_erf.o            \
	wf_lgamma.o         \
	wf_tgamma.o         \
	nearbyintf.o        \
	thumb_vfp_sqrtf.o   \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/mp-readline/,\
	readline.o               \
	)

#------------- mbedtls -------------------#
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/lib/mbedtls/library/*.o)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/usbdev/, \
	core/src/usbd_core.o                \
	core/src/usbd_ctlreq.o              \
	core/src/usbd_ioreq.o               \
	class/src/usbd_cdc_msc_hid.o        \
	class/src/usbd_msc_bot.o            \
	class/src/usbd_msc_scsi.o           \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/extmod/,\
	modujson.o          \
	modure.o            \
	moduzlib.o          \
	moduhashlib.o       \
	modubinascii.o      \
	modurandom.o        \
	moduselect.o        \
	modutimeq.o         \
	moduheapq.o         \
	moductypes.o        \
	vfs.o               \
	vfs_fat.o           \
	vfs_fat_file.o      \
	vfs_reader.o        \
	vfs_fat_diskio.o    \
	vfs_blockdev.o      \
	virtpin.o           \
	machine_mem.o       \
	machine_i2c.o       \
	machine_spi.o       \
	machine_pulse.o     \
	machine_signal.o    \
	machine_pinbase.o   \
	utime_mphal.o       \
	modonewire.o        \
	uos_dupterm.o       \
	modframebuf.o       \
	modbtree.o          \
	moducryptolib.o     \
	modussl_mbedtls.o   \
	)

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
	lib/lwip/src/core/*.o      \
	lib/lwip/src/core/*/*.o    \
	lib/lwip/src/netif/*.o     \
	lib/lwip/src/apps/*/*.o    \
	extmod/modlwip.o           \
	extmod/moduwebsocket.o     \
	extmod/modwebrepl.o        \
	)
LIBS += $(MICROPY_DIR)/drivers/cyw43/libcyw43.a
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	sdio.o                  \
	)
endif

ifeq ($(MICROPY_PY_NETWORK_CYW43), 1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	drivers/cyw43/*.o          \
	extmod/network_cyw43.o     \
	)
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


ifeq ($(CUBEAI), 1)
include $(TOP_DIR)/stm32cubeai/cube.mk
endif

FIRM_OBJ += $(wildcard $(TOP_DIR)/$(TENSORFLOW_DIR)/$(CPU)/*.a)

# Bootloader object files
BOOT_OBJ += $(wildcard $(BUILD)/$(BOOTLDR_DIR)/src/*.o)
BOOT_OBJ += $(wildcard $(BUILD)/$(STHAL_DIR)/src/*.o)
BOOT_OBJ += $(addprefix $(BUILD)/$(CMSIS_DIR)/src/st/,\
	$(STARTUP).o                                \
	system_stm32fxxx.o                          \
	)

# UVC object files
UVC_OBJ += $(wildcard $(BUILD)/$(UVC_DIR)/src/*.o)
UVC_OBJ += $(wildcard $(BUILD)/$(STHAL_DIR)/src/*.o)
UVC_OBJ += $(addprefix $(BUILD)/$(CMSIS_DIR)/src/st/,\
	$(STARTUP).o                                \
	system_stm32fxxx.o                          \
	)

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/, \
	fb_alloc.o                              \
	framebuffer.o                           \
	array.o                                 \
	cambus.o                                \
	ov2640.o                                \
	ov5640.o                                \
	ov7690.o                                \
	ov7725.o                                \
	ov9650.o                                \
	mt9v034.o                               \
	lepton.o                                \
	hm01b0.o                                \
	sensor.o                                \
	stm32fxxx_hal_msp.o                     \
	soft_i2c.o                              \
	mutex.o                                 \
	trace.o                                 \
	)

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/img/,\
	lab_tab.o                               \
	xyz_tab.o                               \
	yuv_tab.o                               \
	rainbow_tab.o                           \
	jpeg.o                                  \
	fmath.o                                 \
	imlib.o                                 \
	)

UVC_OBJ += $(wildcard $(BUILD)/$(LEPTON_DIR)/src/*.o)
UVC_OBJ += $(wildcard $(BUILD)/$(MLX_DIR)/src/*.o)
ifeq ($(MICROPY_PY_IMU), 1)
UVC_OBJ += $(wildcard $(BUILD)/$(ST_DIR)/src/*.o)
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
export FROZEN_MPY_DIR
###################################################
all: $(OPENMV)

$(BUILD):
	$(MKDIR) -p $@

$(FW_DIR):
	$(MKDIR) -p $@

FIRMWARE_OBJS: | $(BUILD) $(FW_DIR)
	$(MAKE)  -C $(CMSIS_DIR)                BUILD=$(BUILD)/$(CMSIS_DIR)    CFLAGS="$(CFLAGS) -fno-strict-aliasing -MMD"
	$(MAKE)  -C $(STHAL_DIR)                BUILD=$(BUILD)/$(STHAL_DIR)    CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MICROPY_DIR)/ports/stm32  BUILD=$(BUILD)/$(MICROPY_DIR)  $(MICROPY_ARGS)
	$(MAKE)  -C $(LEPTON_DIR)               BUILD=$(BUILD)/$(LEPTON_DIR)   CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MLX_DIR)                  BUILD=$(BUILD)/$(MLX_DIR)      CFLAGS="$(CFLAGS) -MMD"
ifeq ($(MICROPY_PY_IMU), 1)
	$(MAKE)  -C $(ST_DIR)                   BUILD=$(BUILD)/$(ST_DIR)       CFLAGS="$(CFLAGS) -MMD"
endif
ifeq ($(MICROPY_PY_WINC1500), 1)
	$(MAKE)  -C $(WINC1500_DIR)             BUILD=$(BUILD)/$(WINC1500_DIR) CFLAGS="$(CFLAGS) -MMD"
endif
	$(MAKE)  -C $(OMV_DIR)                  BUILD=$(BUILD)/$(OMV_DIR)      CFLAGS="$(CFLAGS) -MMD"
ifeq ($(CUBEAI), 1)
	$(MAKE)  -C $(CUBEAI_DIR)               BUILD=$(BUILD)/$(CUBEAI_DIR)   CFLAGS="$(CFLAGS) -fno-strict-aliasing -MMD"
endif

UVC_OBJS: FIRMWARE_OBJS
	$(MAKE)  -C $(UVC_DIR)                  BUILD=$(BUILD)/$(UVC_DIR)      CFLAGS="$(UVC_CFLAGS) -MMD"

BOOTLOADER_OBJS: FIRMWARE_OBJS
	$(MAKE)  -C $(BOOTLDR_DIR)              BUILD=$(BUILD)/$(BOOTLDR_DIR)  CFLAGS="$(BOOTLDR_CFLAGS) -MMD"

# This target generates the main/app firmware image located at 0x08010000
$(FIRMWARE): FIRMWARE_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(OMV_DIR)/stm32fxxx.ld.S > $(BUILD)/stm32fxxx.lds
	$(CC) $(LDFLAGS) $(FIRM_OBJ) -o $(FW_DIR)/$(FIRMWARE).elf $(LIBS)
	$(OBJCOPY) -Obinary -R .big_const* $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin
	$(PYTHON) $(MKDFU) -D $(DFU_DEVICE) -b $(MAIN_APP_ADDR):$(FW_DIR)/$(FIRMWARE).bin $(FW_DIR)/$(FIRMWARE).dfu

# This target generates the bootloader. The bootloader binary is generated twice,
# once without padding, to generate the bootloader.dfu file, and once with padding
# up to the main fw image (MAIN_APP_ADDR), to generate a contiguous firmware.bin image.
$(BOOTLOADER): FIRMWARE_OBJS BOOTLOADER_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(BOOTLDR_DIR)/stm32fxxx.ld.S > $(BUILD)/$(BOOTLDR_DIR)/stm32fxxx.lds
	$(CC) $(BOOTLDR_LDFLAGS) $(BOOT_OBJ) -o $(FW_DIR)/$(BOOTLOADER).elf
	$(OBJCOPY) -Obinary $(FW_DIR)/$(BOOTLOADER).elf $(FW_DIR)/$(BOOTLOADER).bin
	$(PYTHON) $(MKDFU) -D $(DFU_DEVICE) -b 0x08000000:$(FW_DIR)/$(BOOTLOADER).bin $(FW_DIR)/$(BOOTLOADER).dfu
	$(OBJCOPY) -Obinary --pad-to=$(MAIN_APP_ADDR) --gap-fill=0xFF $(FW_DIR)/$(BOOTLOADER).elf $(FW_DIR)/$(BOOTLOADER).bin

# This target generates the UVC firmware.
$(UVC): FIRMWARE_OBJS UVC_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(UVC_DIR)/stm32fxxx.ld.S > $(BUILD)/$(UVC_DIR)/stm32fxxx.lds
	$(CC) $(UVC_LDFLAGS) $(UVC_OBJ) -o $(FW_DIR)/$(UVC).elf
	$(OBJCOPY) -Obinary $(FW_DIR)/$(UVC).elf $(FW_DIR)/$(UVC).bin
	$(PYTHON) $(MKDFU) -D $(DFU_DEVICE) -b $(MAIN_APP_ADDR):$(FW_DIR)/$(UVC).bin $(FW_DIR)/$(UVC).dfu

# This target generates a combined bootloader+main firmware image.
$(OPENMV): $(BOOTLOADER) $(UVC) $(FIRMWARE)
	$(CAT) $(FW_DIR)/$(BOOTLOADER).bin $(FW_DIR)/$(FIRMWARE).bin > $(FW_DIR)/$(OPENMV).bin
	$(PYTHON) $(MKDFU) -D $(DFU_DEVICE) -b 0x08000000:$(FW_DIR)/$(BOOTLOADER).bin -b $(MAIN_APP_ADDR):$(FW_DIR)/$(FIRMWARE).bin $(FW_DIR)/$(OPENMV).dfu
	$(SIZE) $(FW_DIR)/$(BOOTLOADER).elf
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

size:
	$(SIZE) --format=SysV $(FW_DIR)/$(BOOTLOADER).elf
	$(SIZE) --format=SysV $(FW_DIR)/$(FIRMWARE).elf

clean:
	$(RM) -fr $(BUILD)

# Flash the bootloader
flash_boot::
	$(PYDFU) -u $(FW_DIR)/$(BOOTLOADER).dfu

# Flash the main firmware image
flash_image::
	$(PYDFU) -u $(FW_DIR)/$(FIRMWARE).dfu

# Flash the bootloader + main firmware image (DFU)
flash_dfu::
	$(PYDFU) -m -u $(FW_DIR)/$(OPENMV).dfu

# Flash the bootloader + main firmware image (binary)
flash_bin::
	$(PYDFU) -m -u $(FW_DIR)/$(OPENMV).bin

# Flash the bootloader using dfu_util
flash_boot_dfu_util::
	dfu-util -a 0 -d $(DFU_DEVICE) -D $(FW_DIR)/$(BOOTLOADER).dfu

# Flash the main firmware image using dfu_util
flash_image_dfu_util::
	dfu-util -a 0 -d $(DFU_DEVICE) -D $(FW_DIR)/$(FIRMWARE).dfu

# Flash the bootloader + main firmware image (DFU) using dfu_util
flash_dfu_util::
	dfu-util -a 0 -d $(DFU_DEVICE) -D $(FW_DIR)/$(OPENMV).dfu

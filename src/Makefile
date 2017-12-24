# Set verbosity
ifeq ($(V), 1)
Q =
else
Q = @
endif

# Commands
CC      = $(Q)arm-none-eabi-gcc
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
PYDFU   = $(Q)../usr/pydfu.py
MKDIR   = $(Q)mkdir
ECHO    = $(Q)@echo
MAKE    = $(Q)make
CAT     = $(Q)cat

# Targets
OPENMV = openmv
BOOTLOADER = bootloader
FIRMWARE = firmware

# Directories
TOP_DIR=$(shell pwd)
BUILD=$(TOP_DIR)/build
CMSIS_DIR=cmsis
STUSB_DIR=stusb
MICROPY_DIR=micropython
OMV_DIR=omv
WINC1500_DIR=winc1500
BOOT_DIR=bootloader

# Additional qstr definitions for OpenMV
OMV_QSTR_DEFS = $(TOP_DIR)/$(OMV_DIR)/py/qstrdefsomv.h

# Target
TARGET ?= OPENMV3
FW_DIR=$(TOP_DIR)/../firmware/$(TARGET)
OMV_BOARD_CONFIG_DIR=$(TOP_DIR)/$(OMV_DIR)/boards/$(TARGET)/
MP_BOARD_CONFIG_DIR=$(TOP_DIR)/$(MICROPY_DIR)/stmhal/boards/$(TARGET)/

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb3 -DPENDSV_DEBUG
else
CFLAGS += -O2 -ggdb3 -DNDEBUG
endif

# OpenMV1 is No longer supported
ifeq ($(TARGET), OPENMV1)
BRACKET = :)
$(error "OPENMV1 is no longer supported :O Have fun hacking the code $(BRACKET)")
endif

# Enable debug printf
ifeq ($(DEBUG_PRINTF), 1)
CFLAGS += -DDEBUG_PRINTF
endif

# Enable stack protection
ifeq ($(STACK_PROTECTOR), 1)
CFLAGS += -fstack-protector-all -DSTACK_PROTECTOR
endif

# Compiler Flags
include $(OMV_BOARD_CONFIG_DIR)/omv_boardconfig.mk
CFLAGS += -std=gnu99 -Wall -Werror -Warray-bounds -mthumb -nostartfiles -mabi=aapcs-linux -fdata-sections -ffunction-sections
CFLAGS += -D$(MCU) -D$(CFLAGS_MCU) -D$(ARM_MATH) -fsingle-precision-constant -Wdouble-promotion -mcpu=$(CPU) -mtune=$(CPU) -mfpu=$(FPU) -mfloat-abi=hard
CFLAGS += -D__FPU_PRESENT=1 -D__VFP_FP__ -DUSE_USB_FS -DUSE_DEVICE_MODE -DUSE_USB_OTG_ID=0 -DHSE_VALUE=12000000 -D$(TARGET) -DSTM32_HAL_H=$(HAL_INC) -DVECT_TAB_OFFSET=$(VECT_TAB_OFFSET) -DMAIN_APP_ADDR=$(MAIN_APP_ADDR)

CFLAGS += -I. -Iinclude
CFLAGS += -I$(TOP_DIR)/$(BOOT_DIR)/include/
CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/
CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/st
CFLAGS += -I$(TOP_DIR)/$(STHAL_DIR)/include/

CFLAGS += -I$(BUILD)/$(MICROPY_DIR)/
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py/
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/mp-readline
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/stmhal/
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/stmhal/usbdev/core/inc/
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/stmhal/usbdev/class/inc/
CFLAGS += -I$(MP_BOARD_CONFIG_DIR)

CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/
CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/py/
CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/img/
CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
CFLAGS += -I$(TOP_DIR)/$(WINC1500_DIR)/include/

# Linker Flags
LDFLAGS = -mcpu=$(CPU) -mabi=aapcs-linux -mthumb -mfpu=$(FPU) -mfloat-abi=hard\
          -nostdlib -Wl,--gc-sections -Wl,-T$(BUILD)/stm32fxxx.lds

# Linker Flags
BOOT_LDFLAGS = -mcpu=$(CPU) -mabi=aapcs-linux -mthumb -mfpu=$(FPU) -mfloat-abi=hard\
               -nostdlib -Wl,--gc-sections -Wl,-T$(BUILD)/$(BOOT_DIR)/stm32fxxx.lds

#------------- Libraries ----------------#
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/CommonTables/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/FastMathFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/MatrixFunctions/*.o)
#FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/ComplexMathFunctions/*.o)
#FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/ControllerFunctions/*.o)
#FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/FilteringFunctions/*.o)
#FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/StatisticsFunctions/*.o)
#FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/SupportFunctions/*.o)
#FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/TransformFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(STHAL_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(WINC1500_DIR)/src/*.o)

#------------- OpenMV Objects ----------------#
FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/, \
	main.o                                  \
	xalloc.o                                \
	fb_alloc.o                              \
	umm_malloc.o                            \
	ff_wrapper.o                            \
	framebuffer.o                           \
	array.o                                 \
	usbdbg.o                                \
	cambus.o                                \
	ov9650.o                                \
	ov2640.o                                \
	ov7725.o                                \
	mt9v034.o                               \
	sensor.o                                \
	stm32fxxx_hal_msp.o                     \
	soft_i2c.o                              \
	mutex.o                                 \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/img/,\
	blob.o                                  \
	qrcode.o                                \
	apriltag.o                              \
	dmtx.o                                  \
	zbar.o                                  \
	fmath.o                                 \
	fsort.o                                 \
	fft.o                                   \
	haar.o                                  \
	imlib.o                                 \
	collections.o                           \
	stats.o                                 \
	morph.o                                 \
	integral.o                              \
	integral_mw.o                           \
	kmeans.o                                \
	lab_tab.o                               \
	xyz_tab.o                               \
	yuv_tab.o                               \
	rainbow_tab.o                           \
	rgb2rgb_tab.o                           \
	invariant_tab.o                         \
	midpoint.o                              \
	mean.o                                  \
	mode.o                                  \
	median.o                                \
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
	lsd.o                                   \
	sincos_tab.o                            \
	edge.o                                  \
	hog.o                                   \
	lenet.o                                 \
	lenet_model_num.o                       \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/py/, \
	py_helper.o                             \
	py_sensor.o                             \
	py_image.o                              \
	py_time.o                               \
	py_lcd.o                                \
	py_fir.o                                \
	py_gif.o                                \
	py_mjpeg.o                              \
	py_winc.o                               \
	py_cpufreq.o                            \
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
	usbd_msc_storage.o      \
	pendsv.o                \
	bufhelper.o             \
	usb.o                   \
	gchelper.o              \
	gccollect.o             \
	help.o                  \
	flash.o                 \
	storage.o               \
	rtc.o                   \
	irq.o                   \
	adc.o                   \
	dac.o                   \
	dma.o                   \
	uart.o                  \
	systick.o               \
	i2c.o                   \
	spi.o                   \
	can.o                   \
	pin.o                   \
	pin_defs_stmhal.o       \
	pin_named_pins.o        \
	pins_$(TARGET).o        \
	timer.o                 \
	servo.o                 \
	rng.o                   \
	led.o                   \
	wdt.o                   \
	mphalport.o             \
	sdcard.o                \
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
	)

#------------- MicroPy Objects ----------------#
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/,\
	utils/pyexec.o          \
	utils/printf.o          \
	utils/interrupt_char.o  \
	utils/sys_stdio_mphal.o \
	libc/string0.o          \
	netutils/netutils.o     \
	timeutils/timeutils.o   \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/libm/,\
	math.o          \
	roundf.o        \
	asinfacosf.o    \
	atanf.o         \
	atan2f.o        \
	fmodf.o         \
	log1pf.o        \
	acoshf.o        \
	asinhf.o        \
	atanhf.o        \
	kf_rem_pio2.o   \
	kf_sin.o        \
	kf_cos.o        \
	kf_tan.o        \
	ef_rem_pio2.o   \
	erf_lgamma.o    \
	sf_sin.o        \
	sf_cos.o        \
	sf_tan.o        \
	sf_frexp.o      \
	sf_modf.o       \
	sf_ldexp.o      \
	sf_erf.o        \
	wf_lgamma.o     \
	wf_tgamma.o     \
	nearbyintf.o    \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/mp-readline/,\
	readline.o               \
	)


FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/usbdev/, \
	core/src/usbd_core.o                            \
	core/src/usbd_ctlreq.o                          \
	core/src/usbd_ioreq.o                           \
	class/src/usbd_cdc_msc_hid.o        \
	class/src/usbd_msc_bot.o            \
	class/src/usbd_msc_scsi.o           \
	class/src/usbd_msc_data.o           \
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
	vfs.o               \
	vfs_fat.o           \
	vfs_fat_file.o      \
	vfs_fat_misc.o      \
	vfs_reader.o        \
	vfs_fat_diskio.o    \
	virtpin.o           \
	machine_mem.o       \
	machine_i2c.o       \
	machine_spi.o       \
	machine_pulse.o     \
	machine_signal.o    \
	machine_pinbase.o   \
	utime_mphal.o       \
	modonewire.o        \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/oofatfs/,\
	ff.o                \
	option/unicode.o    \
	)

# Bootloader object files
BOOT_OBJ += $(wildcard $(BUILD)/$(BOOT_DIR)/src/*.o)
BOOT_OBJ += $(wildcard $(BUILD)/$(STHAL_DIR)/src/*.o)

BOOT_OBJ += $(addprefix $(BUILD)/$(CMSIS_DIR)/src/st/,\
	$(STARTUP).o                                \
	system_stm32fxxx.o                          \
	)

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
###################################################
all: $(OPENMV)

$(BUILD):
	$(MKDIR) -p $@

BOOTLOADER_OBJS: FIRMWARE_OBJS
	$(MAKE)  -C $(BOOT_DIR)           BUILD=$(BUILD)/$(BOOT_DIR)

FIRMWARE_OBJS:
	$(MAKE)  -C $(CMSIS_DIR)          BUILD=$(BUILD)/$(CMSIS_DIR) CFLAGS="$(CFLAGS) -fno-strict-aliasing"
	$(MAKE)  -C $(STHAL_DIR)          BUILD=$(BUILD)/$(STHAL_DIR) CFLAGS="$(CFLAGS) -fno-strict-aliasing"
	$(MAKE)  -C $(MICROPY_DIR)/stmhal BUILD=$(BUILD)/$(MICROPY_DIR) BOARD=$(TARGET) QSTR_DEFS=$(OMV_QSTR_DEFS)
	$(MAKE)  -C $(WINC1500_DIR)       BUILD=$(BUILD)/$(WINC1500_DIR)
	$(MAKE)  -C $(OMV_DIR)            BUILD=$(BUILD)/$(OMV_DIR)

# This target generates the main/app firmware image located at 0x08010000
$(FIRMWARE): FIRMWARE_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(OMV_DIR)/stm32fxxx.ld.S > $(BUILD)/stm32fxxx.lds
	$(CC) $(LDFLAGS) $(FIRM_OBJ) -o $(FW_DIR)/$(FIRMWARE).elf
	$(OBJCOPY) -Obinary $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin
	$(PYTHON) $(MKDFU) -b $(MAIN_APP_ADDR):$(FW_DIR)/$(FIRMWARE).bin $(FW_DIR)/$(FIRMWARE).dfu

# This target generates the bootloader. The bootloader binary is generated twice,
# once without padding, to generate the bootloader.dfu file, and once with padding
# up to the main fw image (MAIN_APP_ADDR), to generate a contiguous firmware.bin image.
$(BOOTLOADER): FIRMWARE_OBJS BOOTLOADER_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(BOOT_DIR)/stm32fxxx.ld.S > $(BUILD)/$(BOOT_DIR)/stm32fxxx.lds
	$(CC) $(BOOT_LDFLAGS) $(BOOT_OBJ) -o $(FW_DIR)/$(BOOTLOADER).elf
	$(OBJCOPY) -Obinary $(FW_DIR)/$(BOOTLOADER).elf $(FW_DIR)/$(BOOTLOADER).bin
	$(PYTHON) $(MKDFU) -b 0x08000000:$(FW_DIR)/$(BOOTLOADER).bin $(FW_DIR)/$(BOOTLOADER).dfu
	$(OBJCOPY) -Obinary --pad-to=$(MAIN_APP_ADDR) $(FW_DIR)/$(BOOTLOADER).elf $(FW_DIR)/$(BOOTLOADER).bin

# This target generates a combined bootloader+main firmware image.
$(OPENMV): $(BOOTLOADER) $(FIRMWARE) | $(BUILD)
	$(CAT) $(FW_DIR)/$(BOOTLOADER).bin $(FW_DIR)/$(FIRMWARE).bin > $(FW_DIR)/$(OPENMV).bin
	$(PYTHON) $(MKDFU) -b 0x08000000:$(FW_DIR)/$(BOOTLOADER).bin -b $(MAIN_APP_ADDR):$(FW_DIR)/$(FIRMWARE).bin $(FW_DIR)/$(OPENMV).dfu
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

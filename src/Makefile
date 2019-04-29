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
UVC = uvc
FIRMWARE = firmware

# Directories
TOP_DIR=$(shell pwd)
BUILD=$(TOP_DIR)/build
CMSIS_DIR=cmsis
STUSB_DIR=stusb
MICROPY_DIR=micropython
OMV_DIR=omv
LEPTON_DIR=lepton
MLX_DIR=mlx
WINC1500_DIR=winc1500
BOOTLDR_DIR=bootloader
UVC_DIR=uvc

# Additional qstr definitions for OpenMV
OMV_QSTR_DEFS = $(TOP_DIR)/$(OMV_DIR)/py/qstrdefsomv.h

# Target
TARGET ?= OPENMV3
FW_DIR=$(TOP_DIR)/../firmware/$(TARGET)
OMV_BOARD_CONFIG_DIR=$(TOP_DIR)/$(OMV_DIR)/boards/$(TARGET)/
MP_BOARD_CONFIG_DIR=$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/boards/$(TARGET)/

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb3 -DPENDSV_DEBUG
else
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

# Compiler Flags
include $(OMV_BOARD_CONFIG_DIR)/omv_boardconfig.mk
CFLAGS += -std=gnu99 -Wall -Werror -Warray-bounds -mthumb -nostartfiles -fdata-sections -ffunction-sections
CFLAGS += -D$(MCU) -D$(CFLAGS_MCU) -D$(ARM_MATH) -DARM_MATH_DSP -DARM_NN_TRUNCATE\
          -fsingle-precision-constant -Wdouble-promotion -mcpu=$(CPU) -mtune=$(CPU) -mfpu=$(FPU) -mfloat-abi=hard
CFLAGS += -D__FPU_PRESENT=1 -D__VFP_FP__ -DUSE_USB_FS -DUSE_DEVICE_MODE -DUSE_USB_OTG_ID=0 -DHSE_VALUE=12000000\
          -D$(TARGET) -DSTM32_HAL_H=$(HAL_INC) -DVECT_TAB_OFFSET=$(VECT_TAB_OFFSET) -DMAIN_APP_ADDR=$(MAIN_APP_ADDR)

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
MP_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/
MP_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/usbdev/core/inc/
MP_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/usbdev/class/inc/
MP_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)

OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/py/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/nn/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/img/
OMV_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/$(LEPTON_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(MLX_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(WINC1500_DIR)/include/

UVC_CFLAGS = $(CFLAGS)
UVC_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
UVC_CFLAGS += -I$(TOP_DIR)/$(UVC_DIR)/include/

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
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/CommonTables/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/FastMathFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/MatrixFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/nn/ActivationFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/nn/ConvolutionFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/nn/FullyConnectedFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/nn/NNSupportFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/nn/PoolingFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/nn/SoftmaxFunctions/*.o)

#FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/ComplexMathFunctions/*.o)
#FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/ControllerFunctions/*.o)
#FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/FilteringFunctions/*.o)
#FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/StatisticsFunctions/*.o)
#FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/SupportFunctions/*.o)
#FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/TransformFunctions/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(STHAL_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(LEPTON_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MLX_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(WINC1500_DIR)/src/*.o)

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
	ov9650.o                                \
	ov2640.o                                \
	ov7725.o                                \
	mt9v034.o                               \
	lepton.o                                \
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
	rgb2rgb_tab.o                           \
	invariant_tab.o                         \
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
	shadow_removal.o                        \
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
	flashbdev.o             \
	storage.o               \
	rtc.o                   \
	irq.o                   \
	adc.o                   \
	dac.o                   \
	dma.o                   \
	uart.o                  \
	systick.o               \
	i2c.o                   \
	pyb_i2c.o               \
	spi.o                   \
	can.o                   \
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
	pybthread.o             \
	mpthreadport.o          \
	)

# object file for frozen bytecode (frozen .mpy files)
ifneq ($(FROZEN_MPY_DIR),)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,frozen_mpy.o)
endif

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
	core/src/usbd_core.o                \
	core/src/usbd_ctlreq.o              \
	core/src/usbd_ioreq.o               \
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
	moductypes.o        \
	vfs.o               \
	vfs_fat.o           \
	vfs_fat_file.o      \
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
	uos_dupterm.o       \
	modframebuf.o       \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/oofatfs/,\
	ff.o                \
	option/unicode.o    \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/drivers/,\
	bus/softspi.o       \
	dht/dht.o           \
	memory/spiflash.o   \
	)

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
	ov9650.o                                \
	ov2640.o                                \
	ov7725.o                                \
	mt9v034.o                               \
	lepton.o                                \
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
	rgb2rgb_tab.o                           \
	invariant_tab.o                         \
	jpeg.o                                  \
	fmath.o                                 \
	imlib.o                                 \
	)

UVC_OBJ += $(wildcard $(BUILD)/$(LEPTON_DIR)/src/*.o)
UVC_OBJ += $(wildcard $(BUILD)/$(MLX_DIR)/src/*.o)
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
	$(MAKE)  -C $(BOOTLDR_DIR)              BUILD=$(BUILD)/$(BOOTLDR_DIR)  CFLAGS="$(BOOTLDR_CFLAGS) -MMD"

UVC_OBJS: FIRMWARE_OBJS
	$(MAKE)  -C $(UVC_DIR)                  BUILD=$(BUILD)/$(UVC_DIR)      CFLAGS="$(UVC_CFLAGS) -MMD"

FIRMWARE_OBJS:
	$(MAKE)  -C $(CMSIS_DIR)                BUILD=$(BUILD)/$(CMSIS_DIR)    CFLAGS="$(CFLAGS) -fno-strict-aliasing -MMD"
	$(MAKE)  -C $(STHAL_DIR)                BUILD=$(BUILD)/$(STHAL_DIR)    CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MICROPY_DIR)/ports/stm32  BUILD=$(BUILD)/$(MICROPY_DIR)  BOARD=$(TARGET) QSTR_DEFS=$(OMV_QSTR_DEFS)
	$(MAKE)  -C $(LEPTON_DIR)               BUILD=$(BUILD)/$(LEPTON_DIR)   CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MLX_DIR)                  BUILD=$(BUILD)/$(MLX_DIR)      CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(WINC1500_DIR)             BUILD=$(BUILD)/$(WINC1500_DIR) CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(OMV_DIR)                  BUILD=$(BUILD)/$(OMV_DIR)      CFLAGS="$(CFLAGS) -MMD"

# This target generates the main/app firmware image located at 0x08010000
$(FIRMWARE): FIRMWARE_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(OMV_DIR)/stm32fxxx.ld.S > $(BUILD)/stm32fxxx.lds
	$(CC) $(LDFLAGS) $(FIRM_OBJ) -o $(FW_DIR)/$(FIRMWARE).elf -lgcc
	$(OBJCOPY) -Obinary $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin
	$(PYTHON) $(MKDFU) -b $(MAIN_APP_ADDR):$(FW_DIR)/$(FIRMWARE).bin $(FW_DIR)/$(FIRMWARE).dfu

# This target generates the bootloader. The bootloader binary is generated twice,
# once without padding, to generate the bootloader.dfu file, and once with padding
# up to the main fw image (MAIN_APP_ADDR), to generate a contiguous firmware.bin image.
$(BOOTLOADER): FIRMWARE_OBJS BOOTLOADER_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(BOOTLDR_DIR)/stm32fxxx.ld.S > $(BUILD)/$(BOOTLDR_DIR)/stm32fxxx.lds
	$(CC) $(BOOTLDR_LDFLAGS) $(BOOT_OBJ) -o $(FW_DIR)/$(BOOTLOADER).elf
	$(OBJCOPY) -Obinary $(FW_DIR)/$(BOOTLOADER).elf $(FW_DIR)/$(BOOTLOADER).bin
	$(PYTHON) $(MKDFU) -b 0x08000000:$(FW_DIR)/$(BOOTLOADER).bin $(FW_DIR)/$(BOOTLOADER).dfu
	$(OBJCOPY) -Obinary --pad-to=$(MAIN_APP_ADDR) --gap-fill=0xFF $(FW_DIR)/$(BOOTLOADER).elf $(FW_DIR)/$(BOOTLOADER).bin

# This target generates the UVC firmware.
$(UVC): FIRMWARE_OBJS UVC_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(UVC_DIR)/stm32fxxx.ld.S > $(BUILD)/$(UVC_DIR)/stm32fxxx.lds
	$(CC) $(UVC_LDFLAGS) $(UVC_OBJ) -o $(FW_DIR)/$(UVC).elf
	$(OBJCOPY) -Obinary $(FW_DIR)/$(UVC).elf $(FW_DIR)/$(UVC).bin
	$(PYTHON) $(MKDFU) -b $(MAIN_APP_ADDR):$(FW_DIR)/$(UVC).bin $(FW_DIR)/$(UVC).dfu

# This target generates a combined bootloader+main firmware image.
$(OPENMV): $(BOOTLOADER) $(UVC) $(FIRMWARE) | $(BUILD)
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

# Flash the bootloader using dfu_util
flash_boot_dfu_util::
	dfu-util -a 0 -d 0483:df11 -D $(FW_DIR)/$(BOOTLOADER).dfu

# Flash the main firmware image using dfu_util
flash_image_dfu_util::
	dfu-util -a 0 -d 0483:df11 -D $(FW_DIR)/$(FIRMWARE).dfu

# Flash the bootloader + main firmware image (DFU) using dfu_util
flash_dfu_util::
	dfu-util -a 0 -d 0483:df11 -D $(FW_DIR)/$(OPENMV).dfu

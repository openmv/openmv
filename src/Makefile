BIN     = openmv
CC      = arm-none-eabi-gcc
AS      = arm-none-eabi-as
LD      = arm-none-eabi-ld
AR      = arm-none-eabi-ar
RM      = rm
CCP     = arm-none-eabi-cpp
SIZE    = arm-none-eabi-size
STRIP   = arm-none-eabi-strip -s
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
PYTHON  = python
DFU     = micropython/tools/dfu.py
MKDIR   = mkdir
ECHO    = @echo

# Directories
TOP_DIR=$(shell pwd)
BUILD=$(TOP_DIR)/build
CMSIS_DIR=cmsis
STHAL_DIR=sthal
STUSB_DIR=stusb
FATFS_DIR=fatfs
MICROPY_DIR=micropython
OMV_DIR=omv
WINC1500_DIR=winc1500

# Target
TARGET ?= OPENMV2
FW_DIR=$(TOP_DIR)/../firmware/$(TARGET)/

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb3
else
CFLAGS += -O2 -ggdb3 -DNDEBUG
endif

# Verbose
ifeq ($(V), 1)
Q =
else
Q = @
endif

# Compiler Flags
CFLAGS += -std=gnu99 -Wall -Werror -mlittle-endian -mthumb -nostartfiles -mabi=aapcs-linux -fdata-sections -ffunction-sections
CFLAGS += -fsingle-precision-constant -Wdouble-promotion -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -DARM_MATH_CM4 -D__FPU_PRESENT=1 -D__FPU_USED=1 -DUSE_USB_FS -DUSE_DEVICE_MODE -DUSE_USB_OTG_ID=0 -DHSE_VALUE=12000000 -D$(TARGET)
ifeq ($(TARGET), OPENMV1)
    CFLAGS += -DSTM32F407xx
else
    CFLAGS += -DSTM32F427xx
endif

CFLAGS += -I. -Iinclude
CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/
CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/st
CFLAGS += -I$(TOP_DIR)/$(STHAL_DIR)/include/
CFLAGS += -I$(TOP_DIR)/$(FATFS_DIR)/include/

CFLAGS += -I$(BUILD)/$(MICROPY_DIR)/
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py/
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/mp-readline
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/stmhal/
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/stmhal/usbdev/core/inc/
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/stmhal/usbdev/class/inc/
CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/stmhal/boards/$(TARGET)/

CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/
CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/py/
CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/img/
CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/boards/$(TARGET)/
CFLAGS += -I$(TOP_DIR)/$(WINC1500_DIR)/include/

# Linker Flags

LDFLAGS = -mcpu=cortex-m4 -mabi=aapcs-linux -mthumb -mlittle-endian -mfloat-abi=hard\
          -mfpu=fpv4-sp-d16 -nostdlib -Wl,--gc-sections -Wl,-T$(BUILD)/stm32f4xx.lds

#------------- Libraries ----------------#
#OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/st/*.o)
OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/CommonTables/*.o)
OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/FastMathFunctions/*.o)
OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/MatrixFunctions/*.o)
#OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/ComplexMathFunctions/*.o)
#OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/ControllerFunctions/*.o)
#OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/FilteringFunctions/*.o)
#OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/StatisticsFunctions/*.o)
#OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/SupportFunctions/*.o)
#OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/TransformFunctions/*.o)
OBJ += $(wildcard $(BUILD)/$(STHAL_DIR)/src/*.o)
OBJ += $(wildcard $(BUILD)/$(FATFS_DIR)/src/*.o)
OBJ += $(wildcard $(BUILD)/$(FATFS_DIR)/src/option/*.o)
OBJ += $(wildcard $(BUILD)/$(WINC1500_DIR)/src/*.o)

#------------- OpenMV Objects ----------------#
OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/,    \
	main.o                                  \
	xalloc.o                                \
	array.o                                 \
	usbdbg.o                                \
	sccb.o                                  \
	ov9650.o                                \
	ov2640.o                                \
	ov7725.o                                \
	sensor.o                                \
	stm32f4xx_hal_msp.o                     \
	soft_i2c.o                              \
	mutex.o                                 \
	)

ifeq ($(TARGET), OPENMV1)
OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/,    \
	sdcard_spi.o                            \
	)
else
OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/,    \
	sdcard_sdio.o                           \
	)
endif

OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/img/,\
	blob.o                                  \
	fmath.o                                 \
	haar.o                                  \
	imlib.o                                 \
	integral.o                              \
	kmeans.o                                \
	lab_tab.o                               \
	xyz_tab.o                               \
	yuv_tab.o                               \
	rainbow_tab.o                           \
	median.o                                \
	point.o                                 \
	ppm.o                                   \
	rectangle.o                             \
	fast.o                                  \
	freak.o                                 \
	template.o                              \
	font.o                                  \
	jpeg.o                                  \
	lbp.o                                   \
	eye.o                                   \
   )

OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/py/, \
	py_sensor.o                             \
	py_image.o                              \
	py_time.o                               \
	mlx90620.o                              \
	)

ifeq ($(TARGET), OPENMV1)
OBJ += $(addprefix $(BUILD)/$(CMSIS_DIR)/src/st/,\
	system_stm32f4xx.o                          \
	startup_stm32f407xx.o                       \
	)
else
OBJ += $(addprefix $(BUILD)/$(CMSIS_DIR)/src/st/,\
	system_stm32f4xx.o                          \
	startup_stm32f427xx.o                       \
	)
endif

#------------- MicroPy Core -------------------#
OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/py/*.o)

#------------- MicroPy Objects ----------------#
OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	stm32f4xx_it.o          \
	usbd_conf.o             \
	usbd_desc.o             \
	usbd_cdc_interface.o    \
	usbd_msc_storage.o      \
	pendsv.o                \
	bufhelper.o             \
	usb.o                   \
	printf.o                \
	gchelper.o              \
	gccollect.o             \
	pybstdio.o              \
	pyexec.o                \
	help.o                  \
	input.o                 \
	import.o                \
	lexerfatfs.o            \
	flash.o                 \
	diskio.o                \
    fsusermount.o           \
	storage.o               \
	file.o                  \
	rtc.o                   \
	irq.o                   \
	adc.o                   \
	dac.o                   \
	dma.o                   \
	uart.o                  \
	systick.o               \
	i2c.o                   \
	spi.o                   \
	pin.o                   \
	pin_defs_stmhal.o       \
	pin_named_pins.o        \
	pins_$(TARGET).o        \
	timer.o                 \
	servo.o                 \
	rng.o                   \
	led.o                   \
	mphal.o                 \
	extint.o                \
	modpyb.o                \
	modstm.o                \
	moduos.o                \
	modutime.o              \
	moduselect.o            \
	modusocket.o            \
	modnetwork.o            \
    )

#------------- MicroPy Objects ----------------#
OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/,\
	libc/string0.o          \
	netutils/netutils.o     \
	timeutils/timeutils.o   \
    )

OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/libm/,\
	math.o          \
	asinfacosf.o    \
	atanf.o         \
	atan2f.o        \
	fmodf.o         \
	roundf.o        \
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
    )

OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/mp-readline/,\
	readline.o               \
    )


OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/usbdev/, \
	core/src/usbd_core.o                            \
	core/src/usbd_ctlreq.o                          \
	core/src/usbd_ioreq.o                           \
	class/src/usbd_cdc_msc_hid.o        \
	class/src/usbd_msc_bot.o            \
	class/src/usbd_msc_scsi.o           \
	class/src/usbd_msc_data.o           \
	)

###################################################
#Export Variables
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
###################################################
all: $(BUILD)/$(BIN).elf

$(BUILD):
	$(MKDIR) -p $@

objs: | $(BUILD)
	$(MAKE)  -C $(CMSIS_DIR)          BUILD=$(BUILD)/$(CMSIS_DIR)
	$(MAKE)  -C $(STHAL_DIR)          BUILD=$(BUILD)/$(STHAL_DIR)
	$(MAKE)  -C $(FATFS_DIR)          BUILD=$(BUILD)/$(FATFS_DIR)
	$(MAKE)  -C $(MICROPY_DIR)/stmhal BUILD=$(BUILD)/$(MICROPY_DIR) BOARD=$(TARGET)
	$(MAKE)  -C $(OMV_DIR)            BUILD=$(BUILD)/$(OMV_DIR)
	$(MAKE)  -C $(WINC1500_DIR)       BUILD=$(BUILD)/$(WINC1500_DIR)

$(BUILD)/$(BIN).elf: objs
	$(CCP) -P -E -D$(TARGET) $(OMV_DIR)/stm32f4xx.ld.S > $(BUILD)/stm32f4xx.lds
	$(CC) $(LDFLAGS) $(OBJ) $(LIB) -o $(FW_DIR)/$(BIN).elf
	$(OBJCOPY) -Obinary $(FW_DIR)/$(BIN).elf $(FW_DIR)/$(BIN).bin
	$(PYTHON) $(DFU) -b 0x08000000:$(FW_DIR)/$(BIN).bin $(FW_DIR)/$(BIN).dfu
	$(SIZE) $(FW_DIR)/$(BIN).elf

size:
	$(SIZE) $(FW_DIR)/$(BIN).elf

clean:
	$(RM) -fr $(BUILD)

flash::
	../usr/pydfu.py -u $(FW_DIR)/$(BIN).dfu

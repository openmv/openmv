# SPDX-License-Identifier: MIT
#
# Copyright (C) 2020-2024 OpenMV, LLC.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# Set startup and system files based on MCU.
LDSCRIPT  ?= nrf52xxx
HAL_DIR   ?= hal/nrfx
SYSTEM    ?= nrf/system_nrf52840
STARTUP   ?= nrf/startup_$(shell echo $(MCU) | tr '[:upper:]' '[:lower:]')
MCU_LOWER := $(shell echo $(MCU) | tr '[:upper:]' '[:lower:]')

export SD_DIR = $(TOP_DIR)/drivers/nrf

# Compiler Flags
CFLAGS += -std=gnu99 \
          -Wall \
          -Werror \
          -Warray-bounds \
          -mthumb \
          -nostartfiles \
          -fdata-sections \
          -ffunction-sections

CFLAGS += -D$(MCU) \
          -DARM_NN_TRUNCATE \
          -D__FPU_PRESENT=1 \
          -D__VFP_FP__ \
          -D$(TARGET) \
          -fsingle-precision-constant \
          -Wdouble-promotion \
          -mcpu=$(CPU) \
          -mtune=$(CPU) \
          -mfpu=$(FPU) \
          -mfloat-abi=hard \
          -DCMSIS_MCU_H='<$(MCU_LOWER).h>' \
          -DMP_PORT_NO_SOFTTIMER \
          $(OMV_BOARD_CFLAGS)

# Disable LTO and set the SD
MPY_MKARGS += LTO=0 SD=$(SD)

HAL_CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/
HAL_CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/nrf
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/include/
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/include/hal/
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/include/soc/
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/include/prs/

ifeq ($(MICROPY_PY_ULAB), 1)
MPY_CFLAGS += -DMP_NEED_LOG2
endif
MPY_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)
MPY_CFLAGS += -I$(BUILD)/$(MICROPY_DIR)/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/mp-readline
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/tinyusb/src
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/drivers/usb
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/drivers/bluetooth
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/modules/machine
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/modules/ubluepy
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/modules/music
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/modules/ble
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/nrf/modules/board
MPY_CFLAGS += -I$(TOP_DIR)/drivers/nrf/$(NRF_SOFTDEV)/$(NRF_SOFTDEV)_API/include/

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
OMV_CFLAGS += -I$(TOP_DIR)/$(LSM6DS3_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(WINC1500_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(MLX90621_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(MLX90640_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(MLX90641_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(TENSORFLOW_DIR)/$(CPU)/
OMV_CFLAGS += -I$(TOP_DIR)/$(LIBPDM_DIR)/

CFLAGS += $(HAL_CFLAGS) $(MPY_CFLAGS) $(OMV_CFLAGS)

# Linker Flags
LDFLAGS = -mcpu=$(CPU) \
          -mabi=aapcs-linux \
          -mthumb \
          -mfpu=$(FPU) \
          -mfloat-abi=hard \
          -nostdlib \
          -Wl,--gc-sections \
          -Wl,--print-memory-usage \
          -Wl,--wrap=mp_usbd_task \
          -Wl,--wrap=tud_cdc_rx_cb \
          -Wl,--wrap=mp_hal_stdio_poll \
          -Wl,--wrap=mp_hal_stdout_tx_strn \
          -Wl,--no-warn-rwx-segment \
          -Wl,-Map=$(BUILD)/$(FIRMWARE).map \
          -Wl,-T$(BUILD)/$(LDSCRIPT).lds

#------------- Firmware Objects ----------------#
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/CommonTables/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/FastMathFunctions/*.o)

FIRM_OBJ += $(wildcard $(BUILD)/$(HAL_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(LEPTON_DIR)/src/*.o)
ifeq ($(MICROPY_PY_IMU), 1)
FIRM_OBJ += $(wildcard $(BUILD)/$(LSM6DS3_DIR)/src/*.o)
endif
ifeq ($(MICROPY_PY_WINC1500), 1)
FIRM_OBJ += $(wildcard $(BUILD)/$(WINC1500_DIR)/src/*.o)
endif
FIRM_OBJ += $(wildcard $(BUILD)/$(MLX90621_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MLX90640_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MLX90641_DIR)/src/*.o)

#------------- OpenMV Objects ----------------#
FIRM_OBJ += $(addprefix $(BUILD)/$(CMSIS_DIR)/src/, \
	$(STARTUP).o                \
	$(SYSTEM).o                 \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/alloc/, \
	xalloc.o                    \
	fb_alloc.o                  \
	umm_malloc.o                \
	dma_alloc.o                 \
	unaligned_memcpy.o          \
   )

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/common/, \
	array.o                     \
	ringbuf.o                   \
	trace.o                     \
	mutex.o                     \
	pendsv.o                    \
	usbdbg.o                    \
	tinyusb_debug.o             \
	file_utils.o                \
	mp_utils.o                  \
	omv_csi.o                   \
   )

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/sensors/,   \
	ov2640.o                    \
	ov5640.o                    \
	ov7670.o                    \
	ov7690.o                    \
	ov7725.o                    \
	ov9650.o                    \
	mt9v0xx.o                   \
	mt9m114.o                   \
	lepton.o                    \
	hm01b0.o                    \
   )

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/imlib/, \
	agast.o                     \
	apriltag.o                  \
	bayer.o                     \
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
	isp.o                       \
	jpegd.o                     \
	jpege.o                     \
	lodepng.o                   \
	png.o                       \
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
	ppm.o                       \
	qrcode.o                    \
	qsort.o                     \
	rainbow_tab.o               \
	rectangle.o                 \
	selective_search.o          \
	sincos_tab.o                \
	stats.o                     \
	stereo.o                    \
	template.o                  \
	xyz_tab.o                   \
	yuv.o                       \
	zbar.o                      \
   )

FIRM_OBJ += $(wildcard $(BUILD)/$(OMV_DIR)/ports/$(PORT)/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/modules/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/modules/nrf/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/ports/$(PORT)/modules/*.o)

#------------- MicroPy Objects -------------------#
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/py/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/boards/$(TARGET)/*.o)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	mphalport.o                     \
	help.o                          \
	gccollect.o                     \
	pins_gen.o                      \
	pin_named_pins.o                \
	fatfs_port.o                    \
	drivers/flash.o                 \
	drivers/rng.o                   \
	drivers/softpwm.o               \
	drivers/ticker.o                \
	drivers/bluetooth/ble_drv.o     \
	drivers/bluetooth/ble_uart.o    \
	drivers/usb/usb_cdc.o           \
	frozen_content.o                \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/extmod/,\
	machine_adc.o       \
	machine_adc_block.o \
	machine_i2c.o       \
	machine_spi.o       \
	machine_pwm.o       \
	machine_mem.o       \
	machine_uart.o      \
	machine_signal.o    \
	modjson.o           \
	modselect.o         \
	modre.o             \
	modframebuf.o       \
	modasyncio.o        \
	moductypes.o        \
	modhashlib.o        \
	moddeflate.o        \
	modheapq.o          \
	modbinascii.o       \
	modrandom.o         \
    modtime.o           \
	modvfs.o            \
	os_dupterm.o        \
	modmachine.o        \
	modos.o             \
	modplatform.o       \
	vfs.o               \
	vfs_fat.o           \
	vfs_lfs.o           \
	vfs_fat_file.o      \
	vfs_fat_diskio.o    \
	vfs_reader.o        \
	vfs_blockdev.o      \
	virtpin.o           \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/,\
	littlefs/*.o                                    \
	tinyusb/src/common/tusb_fifo.o                  \
	tinyusb/src/device/usbd.o                       \
	tinyusb/src/device/usbd_control.o               \
	tinyusb/src/class/cdc/cdc_device.o              \
	tinyusb/src/tusb.o                              \
	tinyusb/src/portable/nordic/nrf5x/dcd_nrf5x.o   \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/shared/,\
	libc/printf.o               \
	libc/string0.o              \
	runtime/pyexec.o            \
	runtime/mpirq.o             \
	runtime/interrupt_char.o    \
	runtime/sys_stdio_mphal.o   \
	runtime/stdout_helpers.o    \
	timeutils/timeutils.o       \
	readline/readline.o         \
	tinyusb/mp_usbd.o           \
	tinyusb/mp_usbd_cdc.o       \
	tinyusb/mp_usbd_descriptor.o \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/libm/,\
	math.o          \
	fmodf.o         \
	nearbyintf.o    \
	ef_sqrt.o       \
	erf_lgamma.o    \
	kf_rem_pio2.o   \
	kf_sin.o        \
	kf_cos.o        \
	kf_tan.o        \
	ef_rem_pio2.o   \
	sf_sin.o        \
	sf_cos.o        \
	sf_tan.o        \
	sf_frexp.o      \
	sf_modf.o       \
	sf_ldexp.o      \
	sf_erf.o        \
	asinfacosf.o    \
	atanf.o         \
	atan2f.o        \
	roundf.o        \
	log1pf.o        \
	acoshf.o        \
	asinhf.o        \
	atanhf.o        \
	wf_lgamma.o     \
	wf_tgamma.o     \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/modules/,\
	machine/spi.o                       \
	machine/i2c.o                       \
	machine/pin.o                       \
	machine/timer.o                     \
	machine/rtcounter.o                 \
	machine/temp.o                      \
	os/microbitfs.o                     \
	board/modboard.o                    \
	board/led.o                         \
	ubluepy/modubluepy.o                \
	ubluepy/ubluepy_peripheral.o        \
	ubluepy/ubluepy_service.o           \
	ubluepy/ubluepy_characteristic.o    \
	ubluepy/ubluepy_uuid.o              \
	ubluepy/ubluepy_delegate.o          \
	ubluepy/ubluepy_constants.o         \
	ubluepy/ubluepy_descriptor.o        \
	ubluepy/ubluepy_scanner.o           \
	ubluepy/ubluepy_scan_entry.o        \
	music/modmusic.o                    \
	music/musictunes.o                  \
	ble/modble.o                        \
	)

ifeq ($(MICROPY_PY_ULAB), 1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/modules/ulab/code/,\
	ndarray.o                       \
	ndarray_operators.o             \
	ndarray_properties.o            \
	numpy/approx.o                  \
	numpy/bitwise.o                 \
	numpy/carray/carray.o           \
	numpy/carray/carray_tools.o     \
	numpy/compare.o                 \
	numpy/create.o                  \
	numpy/fft/fft.o                 \
	numpy/fft/fft_tools.o           \
	numpy/filter.o                  \
	numpy/io/io.o                   \
	numpy/linalg/linalg.o           \
	numpy/linalg/linalg_tools.o     \
	numpy/ndarray/ndarray_iter.o    \
	numpy/numerical.o               \
	numpy/numpy.o                   \
	numpy/poly.o                    \
	numpy/random/random.o           \
	numpy/stats.o                   \
	numpy/transform.o               \
	numpy/vector.o                  \
	scipy/linalg/linalg.o           \
	scipy/optimize/optimize.o       \
	scipy/scipy.o                   \
	scipy/signal/signal.o           \
	scipy/special/special.o         \
	ulab.o                          \
	ulab_tools.o                    \
	user/user.o                     \
	utils/utils.o                   \
	)
endif

###################################################
all: $(OPENMV)

$(BUILD):
	$(MKDIR) -p $@

$(FW_DIR):
	$(MKDIR) -p $@

FIRMWARE_OBJS: | $(BUILD) $(FW_DIR)
	$(MAKE)  -C $(CMSIS_DIR)                 BUILD=$(BUILD)/$(CMSIS_DIR)    CFLAGS="$(CFLAGS) -fno-strict-aliasing -MMD"
	$(MAKE)  -C $(MICROPY_DIR)/ports/$(PORT) BUILD=$(BUILD)/$(MICROPY_DIR)  $(MPY_MKARGS)
	$(MAKE)  -C $(HAL_DIR)                   BUILD=$(BUILD)/$(HAL_DIR)      CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MLX90621_DIR)              BUILD=$(BUILD)/$(MLX90621_DIR) CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MLX90640_DIR)              BUILD=$(BUILD)/$(MLX90640_DIR) CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MLX90641_DIR)              BUILD=$(BUILD)/$(MLX90641_DIR) CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(OMV_DIR)                   BUILD=$(BUILD)/$(OMV_DIR)      CFLAGS="$(CFLAGS) -MMD"

$(FIRMWARE): FIRMWARE_OBJS
	$(CPP) -P -E -I$(OMV_COMMON_DIR) -I$(OMV_BOARD_CONFIG_DIR) \
                   $(OMV_DIR)/ports/$(PORT)/$(LDSCRIPT).ld.S > $(BUILD)/$(LDSCRIPT).lds
	$(CC) $(LDFLAGS) $(FIRM_OBJ) -o $(FW_DIR)/$(FIRMWARE).elf $(LIBS) -lgcc
	$(OBJCOPY) -Oihex   $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).hex
	$(OBJCOPY) -Obinary $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin

# This target generates the firmware image.
$(OPENMV): $(FIRMWARE)
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

size:
	$(SIZE) --format=SysV $(FW_DIR)/$(FIRMWARE).elf

# Flash the main firmware image
flash_image: $(FW_DIR)/$(FIRMWARE).hex
	nrfjprog --program $< --sectorerase -f nrf52
	nrfjprog --reset -f nrf52

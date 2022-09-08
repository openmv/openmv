# Set startup and system files based on MCU.
SYSTEM    ?= st/system_stm32fxxx
STARTUP   ?= st/startup_$(shell echo $(MCU) | tr '[:upper:]' '[:lower:]')
LDSCRIPT  ?= stm32fxxx

# Compiler Flags
CFLAGS += -std=gnu99 -Wall -Werror -Warray-bounds -mthumb -nostartfiles -fdata-sections -ffunction-sections
CFLAGS += -fno-inline-small-functions -D$(MCU) -D$(CFLAGS_MCU) -D$(ARM_MATH) -DARM_NN_TRUNCATE\
          -fsingle-precision-constant -Wdouble-promotion -mcpu=$(CPU) -mtune=$(CPU) -mfpu=$(FPU) -mfloat-abi=hard
CFLAGS += -D__FPU_PRESENT=1 -D__VFP_FP__ -DUSE_DEVICE_MODE -DHSE_VALUE=$(OMV_HSE_VALUE)\
          -D$(TARGET) -DVECT_TAB_OFFSET=$(VECT_TAB_OFFSET) -DMAIN_APP_ADDR=$(MAIN_APP_ADDR) -DSTM32_HAL_H=$(HAL_INC)
CFLAGS += $(OMV_BOARD_EXTRA_CFLAGS)

HAL_CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/
HAL_CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/st
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/include/
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/include/Legacy/

MPY_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)
MPY_CFLAGS += -I$(BUILD)/$(MICROPY_DIR)/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/lwip/src/include/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/usbdev/core/inc/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/usbdev/class/inc/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/lwip_inc/
MPY_CFLAGS += -DMICROPY_PY_USSL=1 -DMICROPY_SSL_MBEDTLS=1
MICROPY_ARGS += MICROPY_PY_USSL=1 MICROPY_SSL_MBEDTLS=1 MICROPY_PY_BTREE=1

OMV_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/alloc/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/common/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/imlib/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/modules/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/sensors/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/templates/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/ports/$(PORT)/
OMV_CFLAGS += -I$(TOP_DIR)/$(OMV_DIR)/ports/$(PORT)/modules/

OMV_CFLAGS += -I$(TOP_DIR)/$(LEPTON_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(LSM6DS3_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(LSM6DSOX_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(VL53L5CX_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(WINC1500_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(MLX90621_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(MLX90640_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(MLX90641_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(PIXART_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(TENSORFLOW_DIR)/
OMV_CFLAGS += -I$(BUILD)/$(TENSORFLOW_DIR)/
OMV_CFLAGS += -I$(TOP_DIR)/$(LIBPDM_DIR)/

ifeq ($(OMV_ENABLE_BL), 1)
CFLAGS     += -DOMV_ENABLE_BOOTLOADER
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

ifeq ($(OMV_ENABLE_CM4), 1)
CFLAGS     += -DM4_APP_ADDR=$(M4_APP_ADDR)
ifeq ($(DEBUG), 1)
CM4_CFLAGS += -Og -ggdb3 -Wno-maybe-uninitialized
else
CM4_CFLAGS += -O2 -DNDEBUG
endif
CM4_CFLAGS += -std=gnu99 -Wall -Werror -Warray-bounds -mthumb -nostartfiles -fdata-sections -ffunction-sections
CM4_CFLAGS += -D$(MCU) -D$(CFLAGS_MCU) -D$(ARM_MATH) -DARM_NN_TRUNCATE -DCORE_CM4\
              -fsingle-precision-constant -Wdouble-promotion -mcpu=cortex-m4 -mtune=cortex-m4 -mfpu=$(FPU) -mfloat-abi=hard
CM4_CFLAGS += -D__FPU_PRESENT=1 -D__VFP_FP__ -DHSE_VALUE=$(OMV_HSE_VALUE)\
              -D$(TARGET) -DVECT_TAB_OFFSET=$(M4_VECT_TAB_OFFSET) -DMAIN_APP_ADDR=$(M4_APP_ADDR) -DSTM32_HAL_H=$(HAL_INC)
CM4_CFLAGS += $(HAL_CFLAGS)
CM4_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
CM4_CFLAGS += -I$(TOP_DIR)/$(CM4_DIR)/include/
# Linker Flags
CM4_LDFLAGS = -mcpu=cortex-m4 -mabi=aapcs-linux -mthumb -mfpu=$(FPU) -mfloat-abi=hard\
               -nostdlib -Wl,--gc-sections -Wl,-T$(BUILD)/$(CM4_DIR)/stm32fxxx.lds
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

FIRM_OBJ += $(wildcard $(BUILD)/$(HAL_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(LEPTON_DIR)/src/*.o)
ifeq ($(MICROPY_PY_IMU), 1)
FIRM_OBJ += $(wildcard $(BUILD)/$(LSM6DS3_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(LSM6DSOX_DIR)/src/*.o)
endif
ifeq ($(MICROPY_PY_WINC1500), 1)
FIRM_OBJ += $(wildcard $(BUILD)/$(WINC1500_DIR)/src/*.o)
endif
FIRM_OBJ += $(wildcard $(BUILD)/$(MLX90621_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MLX90640_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MLX90641_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(VL53L5CX_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(PIXART_DIR)/src/*.o)

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
	ff_wrapper.o                \
	ini.o                       \
	ringbuf.o                   \
	trace.o                     \
	mutex.o                     \
	usbdbg.o                    \
	sensor_utils.o              \
	factoryreset.o              \
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
	hm0360.o                    \
	gc2145.o                    \
	paj6100.o                   \
	frogeye2020.o               \
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
	jpegd.o                     \
	jpeg.o                      \
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
	pool.o                      \
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

FIRM_OBJ += $(wildcard $(BUILD)/$(TENSORFLOW_DIR)/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(OMV_DIR)/ports/$(PORT)/*.o)

#------------- MicroPy Objects -------------------#
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/modules/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/ports/$(PORT)/modules/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/py/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/boards/$(TARGET)/*.o)

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
	eth.o                   \
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
	network_lan.o           \
	modmachine.o            \
	machine_i2c.o           \
	machine_spi.o           \
	machine_uart.o          \
	machine_adc.o           \
	machine_timer.o         \
	machine_bitstream.o     \
	pybthread.o             \
	mpthreadport.o          \
	posix_helpers.o         \
	softtimer.o             \
	mbedtls/mbedtls_port.o  \
	frozen_content.o        \
	)

#------------- MicroPy Objects ----------------#
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/shared/,\
	libc/printf.o               \
	libc/string0.o              \
	libc/abort_.o               \
	runtime/mpirq.o             \
	runtime/pyexec.o            \
	runtime/interrupt_char.o    \
	runtime/sys_stdio_mphal.o   \
	runtime/gchelper_m3.o       \
	runtime/gchelper_native.o   \
	runtime/stdout_helpers.o    \
	netutils/*.o                \
	timeutils/timeutils.o       \
	readline/readline.o         \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/,\
	berkeley-db-1.xx/btree/*.o  \
	berkeley-db-1.xx/mpool/*.o  \
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

#------------- mbedtls -------------------#
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/lib/mbedtls/library/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/lib/mbedtls_errors/*.o)

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
	machine_bitstream.o \
	utime_mphal.o       \
	modonewire.o        \
	uos_dupterm.o       \
	modframebuf.o       \
	modbtree.o          \
	moducryptolib.o     \
	modussl_mbedtls.o   \
	moduasyncio.o       \
	modusocket.o        \
	modnetwork.o        \
	moduplatform.o      \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/oofatfs/,\
	ff.o                \
	ffunicode.o         \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/drivers/,\
	bus/softspi.o       \
	dht/dht.o           \
	memory/spiflash.o   \
	)

ifeq ($(MICROPY_PY_ULAB), 1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/modules/ulab/,\
	code/ndarray.o                      \
	code/ndarray_operators.o            \
	code/ndarray_properties.o           \
	code/numpy/approx.o                 \
	code/numpy/carray/carray.o          \
	code/numpy/carray/carray_tools.o    \
	code/numpy/compare.o                \
	code/numpy/create.o                 \
	code/numpy/fft/fft.o                \
	code/numpy/fft/fft_tools.o          \
	code/numpy/filter.o                 \
	code/numpy/linalg/linalg.o          \
	code/numpy/linalg/linalg_tools.o    \
	code/numpy/ndarray/ndarray_iter.o   \
	code/numpy/numerical.o              \
	code/numpy/numpy.o                  \
	code/numpy/poly.o                   \
	code/numpy/stats.o                  \
	code/numpy/transform.o              \
	code/numpy/vector.o                 \
	code/scipy/linalg/linalg.o          \
	code/scipy/optimize/optimize.o      \
	code/scipy/scipy.o                  \
	code/scipy/signal/signal.o          \
	code/scipy/special/special.o        \
	code/ulab.o                         \
	code/ulab_tools.o                   \
	code/user/user.o                    \
	code/utils/utils.o                  \
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
	mpnetworkport.o            \
	)
endif

ifeq ($(MICROPY_PY_NETWORK_CYW43), 1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	drivers/cyw43/*.o          \
	extmod/network_cyw43.o     \
	)
LIBS += $(MICROPY_DIR)/drivers/cyw43/libcyw43.a
endif

ifeq ($(MICROPY_BLUETOOTH_NIMBLE),1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/mynewt-nimble/,\
	ext/tinycrypt/src/*.o                   \
	nimble/host/services/gap/src/*.o        \
	nimble/host/services/gatt/src/*.o       \
	nimble/host/src/*.o                     \
	nimble/host/util/src/*.o                \
	nimble/transport/uart/src/*.o           \
	porting/nimble/src/*.o                  \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	mpbthciport.o                               \
	mpnimbleport.o                              \
	extmod/nimble/modbluetooth_nimble.o         \
	extmod/nimble/nimble/nimble_npl_os.o        \
	extmod/nimble/hal/hal_uart.o                \
	extmod/modbluetooth.o                       \
	)
endif

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
	dma_alloc.o                             \
	unaligned_memcpy.o                      \
	)

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/common/, \
	array.o                                 \
	trace.o                                 \
	mutex.o                                 \
	sensor_utils.o                          \
	)

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/sensors/, \
	ov2640.o                                \
	ov5640.o                                \
	ov7690.o                                \
	ov7670.o                                \
	ov7725.o                                \
	ov9650.o                                \
	mt9v0xx.o                               \
	mt9m114.o                               \
	lepton.o                                \
	hm01b0.o                                \
	hm0360.o                                \
	gc2145.o                                \
	paj6100.o                               \
	frogeye2020.o                           \
	)

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/imlib/,\
	lab_tab.o                               \
	xyz_tab.o                               \
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
	ulpi.o                                  \
	)

UVC_OBJ += $(wildcard $(BUILD)/$(LEPTON_DIR)/src/*.o)
ifeq ($(MICROPY_PY_IMU), 1)
UVC_OBJ += $(wildcard $(BUILD)/$(LSM6DS3_DIR)/src/*.o)
UVC_OBJ += $(wildcard $(BUILD)/$(LSM6DSOX_DIR)/src/*.o)
endif
UVC_OBJ += $(wildcard $(BUILD)/$(MLX90621_DIR)/src/*.o)
UVC_OBJ += $(wildcard $(BUILD)/$(MLX90640_DIR)/src/*.o)
UVC_OBJ += $(wildcard $(BUILD)/$(MLX90641_DIR)/src/*.o)
UVC_OBJ += $(wildcard $(BUILD)/$(VL53L5CX_DIR)/src/*.o)
UVC_OBJ += $(wildcard $(BUILD)/$(PIXART_DIR)/src/*.o)
endif

ifeq ($(OMV_ENABLE_CM4), 1)
CM4 = cm4
# CM4 object files
CM4_OBJ += $(wildcard $(BUILD)/$(CM4_DIR)/src/*.o)
CM4_OBJ += $(wildcard $(BUILD)/$(CM4_DIR)/$(HAL_DIR)/src/*.o)
CM4_OBJ += $(addprefix $(BUILD)/$(CM4_DIR)/$(CMSIS_DIR)/src/, \
	$(STARTUP).o                \
	$(SYSTEM).o                 \
)
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
export TOOLS
export TARGET
export STARTUP
export SYSTEM
export FROZEN_MANIFEST
export PORT
export HAL_DIR
export CMSIS_DIR
export PYTHON
export TFLITE2C
###################################################
all: $(OPENMV)

$(BUILD):
	$(MKDIR) -p $@

$(FW_DIR):
	$(MKDIR) -p $@

FIRMWARE_OBJS: | $(BUILD) $(FW_DIR)
	$(MAKE)  -C $(CMSIS_DIR)                 BUILD=$(BUILD)/$(CMSIS_DIR)        CFLAGS="$(CFLAGS) -fno-strict-aliasing -MMD"
	$(MAKE)  -C $(TENSORFLOW_DIR)            BUILD=$(BUILD)/$(TENSORFLOW_DIR)   CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MICROPY_DIR)/ports/$(PORT) BUILD=$(BUILD)/$(MICROPY_DIR)      $(MICROPY_ARGS)
	$(MAKE)  -C $(HAL_DIR)                   BUILD=$(BUILD)/$(HAL_DIR)          CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(LEPTON_DIR)                BUILD=$(BUILD)/$(LEPTON_DIR)       CFLAGS="$(CFLAGS) -MMD"
ifeq ($(MICROPY_PY_IMU), 1)
	$(MAKE)  -C $(LSM6DS3_DIR)               BUILD=$(BUILD)/$(LSM6DS3_DIR)      CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(LSM6DSOX_DIR)              BUILD=$(BUILD)/$(LSM6DSOX_DIR)     CFLAGS="$(CFLAGS) -MMD"
endif
ifeq ($(MICROPY_PY_WINC1500), 1)
	$(MAKE)  -C $(WINC1500_DIR)              BUILD=$(BUILD)/$(WINC1500_DIR)     CFLAGS="$(CFLAGS) -MMD"
endif
	$(MAKE)  -C $(MLX90621_DIR)              BUILD=$(BUILD)/$(MLX90621_DIR)     CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MLX90640_DIR)              BUILD=$(BUILD)/$(MLX90640_DIR)     CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MLX90641_DIR)              BUILD=$(BUILD)/$(MLX90641_DIR)     CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(VL53L5CX_DIR)              BUILD=$(BUILD)/$(VL53L5CX_DIR)     CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(PIXART_DIR)                BUILD=$(BUILD)/$(PIXART_DIR)       CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(OMV_DIR)                   BUILD=$(BUILD)/$(OMV_DIR)          CFLAGS="$(CFLAGS) -MMD"
ifeq ($(CUBEAI), 1)
	$(MAKE)  -C $(CUBEAI_DIR)                BUILD=$(BUILD)/$(CUBEAI_DIR)       CFLAGS="$(CFLAGS) -fno-strict-aliasing -MMD"
endif
ifeq ($(OMV_ENABLE_UVC), 1)
UVC_OBJS: FIRMWARE_OBJS
	$(MAKE)  -C $(UVC_DIR)                   BUILD=$(BUILD)/$(UVC_DIR)          CFLAGS="$(UVC_CFLAGS) -MMD"
endif
ifeq ($(OMV_ENABLE_CM4), 1)
CM4_OBJS: FIRMWARE_OBJS
	$(MAKE)  -C $(CM4_DIR)                   BUILD=$(BUILD)/$(CM4_DIR)          CFLAGS="$(CM4_CFLAGS) -MMD"
endif
ifeq ($(OMV_ENABLE_BL), 1)
BOOTLOADER_OBJS: FIRMWARE_OBJS
	$(MAKE)  -C $(BOOTLDR_DIR)               BUILD=$(BUILD)/$(BOOTLDR_DIR)      CFLAGS="$(BL_CFLAGS) -MMD"
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

ifeq ($(OMV_ENABLE_CM4), 1)
# This target generates CM4 firmware for dual core micros.
$(CM4): FIRMWARE_OBJS CM4_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(CM4_DIR)/stm32fxxx.ld.S > $(BUILD)/$(CM4_DIR)/stm32fxxx.lds
	$(CC) $(CM4_LDFLAGS) $(CM4_OBJ) -o $(FW_DIR)/$(CM4).elf -lgcc
	$(OBJCOPY) -Obinary $(FW_DIR)/$(CM4).elf $(FW_DIR)/$(CM4).bin
	$(PYTHON) $(MKDFU) -D $(DFU_DEVICE) -b $(M4_APP_ADDR):$(FW_DIR)/$(CM4).bin $(FW_DIR)/$(CM4).dfu
endif

# This target generates the uvc, bootloader and firmware images.
$(OPENMV): $(BOOTLOADER) $(UVC) $(CM4) $(FIRMWARE)
ifeq ($(OMV_ENABLE_BL), 1)
	# Generate a contiguous firmware image for factory programming
	$(OBJCOPY) -Obinary --pad-to=$(MAIN_APP_ADDR) --gap-fill=0xFF $(FW_DIR)/$(BOOTLOADER).elf $(FW_DIR)/$(BOOTLOADER)_padded.bin
	$(CAT) $(FW_DIR)/$(BOOTLOADER)_padded.bin $(FW_DIR)/$(FIRMWARE).bin > $(FW_DIR)/$(OPENMV).bin
	$(RM) $(FW_DIR)/$(BOOTLOADER)_padded.bin
	$(SIZE) $(FW_DIR)/$(BOOTLOADER).elf
endif
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

size:
ifeq ($(OMV_ENABLE_BL), 1)
	$(SIZE) --format=SysV $(FW_DIR)/$(BOOTLOADER).elf
endif
	$(SIZE) --format=SysV $(FW_DIR)/$(FIRMWARE).elf

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

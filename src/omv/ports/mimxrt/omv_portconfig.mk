# Set startup and system files based on MCU.
SYSTEM    ?= mimxrt/system_$(MCU_SERIES)
STARTUP   ?= mimxrt/startup_$(MCU_SERIES)
LDSCRIPT  ?= mimxrt

# Compiler Flags
# TODO: -Wdouble-promotion
CFLAGS += -std=gnu99 -Wall -Werror -Warray-bounds -nostartfiles -fdata-sections -ffunction-sections \
          -fno-inline-small-functions -mfloat-abi=$(FABI) -mthumb -mcpu=$(CPU) -mtune=$(CPU) -mfpu=$(FPU)
# TODO: FIX HSE
CFLAGS += -DCPU_$(MCU_VARIANT)\
          -D$(TARGET)\
          -D$(ARM_MATH) -DARM_NN_TRUNCATE\
          -D__FPU_PRESENT=1 -D__VFP_FP__\
          -DHSE_VALUE=$(OMV_HSE_VALUE) \
          -DMICROPY_PY_MACHINE_SDCARD=1 \
          -DXIP_EXTERNAL_FLASH=1 \
	      -DXIP_BOOT_HEADER_ENABLE=1 \
	      -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
	      -DCFG_TUSB_MCU=OPT_MCU_MIMXRT1XXX \
	      -DCPU_HEADER_H='<$(MCU_SERIES).h>' \
	      -DCMSIS_MCU_H=$(CMSIS_MCU_H) \
	      -DCLOCK_CONFIG_H='<boards/$(MCU_SERIES)_clock_config.h>' \
          -DCSI_DRIVER_FRAG_MODE=1\
          -D__START=main\
          -D__STARTUP_CLEAR_BSS\
          -D__STARTUP_INITIALIZE_RAMFUNCTION\
          $(OMV_BOARD_EXTRA_CFLAGS)\

HAL_CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/
HAL_CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/mimxrt
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/drivers

MPY_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)
MPY_CFLAGS += -I$(BUILD)/$(MICROPY_DIR)/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/tinyusb/src
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/lwip/src/include/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/mimxrt/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/mimxrt/lwip_inc/
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/shared/runtime/
MPY_CFLAGS += -DMICROPY_VFS_FAT=1

ifeq ($(MICROPY_PY_SYS_SETTRACE), 1)
MPY_CFLAGS += -DMICROPY_PY_SYS_SETTRACE=1
endif

ifeq ($(MICROPY_PY_LWIP), 0)
MICROPY_ARGS += MICROPY_PY_LWIP=0 MICROPY_PY_SSL=0
else
MPY_CFLAGS += -DMICROPY_PY_SSL=1 -DMICROPY_SSL_MBEDTLS=1
MICROPY_ARGS += MICROPY_PY_SSL=1 MICROPY_SSL_MBEDTLS=1
endif
MICROPY_ARGS += MCU_DIR=$(TOP_DIR)/$(HAL_DIR) CMSIS_DIR=$(TOP_DIR)/$(CMSIS_DIR)\
                SUPPORTS_HARDWARE_FP_SINGLE=1 MICROPY_VFS_LFS2=0

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

ifeq ($(OMV_ENABLE_UVC), 1)
UVC_CFLAGS := $(CFLAGS) $(HAL_CFLAGS)
UVC_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
UVC_CFLAGS += -I$(TOP_DIR)/$(UVC_DIR)/include/
UVC_CFLAGS += $(OMV_CFLAGS) $(MPY_CFLAGS)
# Linker Flags
UVC_LDFLAGS = -mcpu=$(CPU) -mabi=aapcs-linux -mthumb -mfpu=$(FPU) -mfloat-abi=hard -nostdlib \
              -Wl,--print-memory-usage -Wl,--gc-sections -Wl,-T$(BUILD)/$(UVC_DIR)/$(LD_SCRIPT).lds
endif

CFLAGS += $(HAL_CFLAGS) $(MPY_CFLAGS) $(OMV_CFLAGS)
# Linker Flags
LDFLAGS = -mcpu=$(CPU) -mabi=aapcs-linux -mthumb -mfpu=$(FPU) -mfloat-abi=hard -nostdlib \
          -Wl,--print-memory-usage -Wl,--gc-sections -Wl,-T$(BUILD)/$(LDSCRIPT).lds \
          -Wl,--wrap=tud_cdc_rx_cb -Wl,--wrap=mp_hal_stdout_tx_strn

#------------- Libraries ----------------#
LIBS += $(TOP_DIR)/$(TENSORFLOW_DIR)/$(CPU)/libtf*.a
ifeq ($(MICROPY_PY_AUDIO), 1)
LIBS += $(TOP_DIR)/$(LIBPDM_DIR)/libPDMFilter_CM7_GCC_wc32.a
endif

#------------- Firmware Objects ----------------#
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/CommonTables/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/FastMathFunctions/*.o)

FIRM_OBJ += $(wildcard $(BUILD)/$(HAL_DIR)/drivers/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(LEPTON_DIR)/src/*.o)
ifeq ($(MICROPY_PY_IMU), 1)
FIRM_OBJ += $(wildcard $(BUILD)/$(LSM6DS3_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(LSM6DSOX_DIR)/src/*.o)
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
	unaligned_memcpy.o          \
   )

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/common/, \
	array.o                     \
	ini.o                       \
	ringbuf.o                   \
	trace.o                     \
	mutex.o                     \
	vospi.o                     \
	pendsv.o                    \
	usbdbg.o                    \
	tinyusb_debug.o             \
	file_utils.o                \
	boot_utils.o                \
	sensor_utils.o              \
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
	boards/$(MCU_SERIES)_clock_config.o \
	dma_manager.o                       \
	eth.o                               \
	fatfs_port.o                        \
	frozen_content.o                    \
	flash.o                             \
	led.o                               \
	machine_bitstream.o                 \
	machine_can.o                       \
	machine_i2c.o                       \
	machine_led.o                       \
	machine_pin.o                       \
	machine_rtc.o                       \
	machine_sdcard.o                    \
	machine_spi.o                       \
	mbedtls/mbedtls_port.o              \
	mimxrt_flash.o                      \
	mimxrt_sdram.o                      \
	modmimxrt.o                         \
	mpnetworkport.o                     \
	msc_disk.o                          \
	network_lan.o                       \
	mphalport.o                         \
	pin.o                               \
	pins_gen.o                          \
	sdcard.o                            \
	sdio.o                              \
	systick.o                           \
	ticks.o                             \
	tusb_port.o                         \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/hal/,\
	pwm_backport.o                  \
	flexspi_nor_flash.o             \
	qspi_nor_flash_config.o         \
	)

ifeq ($(MICROPY_PY_LWIP), 1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/hal/,\
	phy/mdio/enet/fsl_enet_mdio.o           \
	phy/device/phydp83825/fsl_phydp83825.o  \
	phy/device/phydp83848/fsl_phydp83848.o  \
	phy/device/phyksz8081/fsl_phyksz8081.o  \
	phy/device/phylan8720/fsl_phylan8720.o  \
	phy/device/phyrtl8211f/fsl_phyrtl8211f.o\
	)
endif


FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/shared/,\
	libc/printf.o               \
	libc/string0.o              \
	netutils/netutils.o         \
	netutils/trace.o            \
	netutils/dhcpserver.o       \
	runtime/gchelper_thumb2.o   \
	runtime/gchelper_native.o   \
	runtime/interrupt_char.o    \
	runtime/mpirq.o             \
	runtime/pyexec.o            \
	runtime/stdout_helpers.o    \
	runtime/sys_stdio_mphal.o   \
	runtime/softtimer.o         \
	timeutils/timeutils.o       \
	readline/readline.o         \
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

FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/lib/mbedtls/library/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/lib/mbedtls_errors/*.o)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/tinyusb/src/, \
	class/cdc/cdc_device.o          \
	class/dfu/dfu_rt_device.o       \
	class/hid/hid_device.o          \
	class/midi/midi_device.o        \
	class/msc/msc_device.o          \
	class/usbtmc/usbtmc_device.o    \
	class/vendor/vendor_device.o    \
	common/tusb_fifo.o              \
	device/usbd.o                   \
	device/usbd_control.o           \
	tusb.o                          \
	portable/chipidea/ci_hs/dcd_ci_hs.o \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/extmod/,\
	machine_adc.o       \
	machine_adc_block.o \
	machine_bitstream.o \
	machine_i2c.o       \
	machine_i2s.o       \
	machine_mem.o       \
	machine_pinbase.o   \
	machine_pulse.o     \
	machine_pwm.o       \
	machine_signal.o    \
	machine_spi.o       \
	machine_timer.o     \
	machine_uart.o      \
	machine_wdt.o       \
	modframebuf.o       \
	modmachine.o        \
	modnetwork.o        \
	modonewire.o        \
	modasyncio.o        \
	modbinascii.o       \
	modcryptolib.o      \
	moddeflate.o        \
	moductypes.o        \
	modhashlib.o        \
	modheapq.o          \
	modjson.o           \
	modos.o             \
	modplatform.o       \
	modrandom.o         \
	modre.o             \
	modselect.o         \
	modsocket.o         \
	modssl_axtls.o      \
	modssl_mbedtls.o    \
	modtime.o           \
	os_dupterm.o        \
	vfs_blockdev.o      \
	vfs_fat_diskio.o    \
	vfs_fat_file.o      \
	vfs_fat.o           \
	vfs.o               \
	vfs_posix_file.o    \
	vfs_posix.o         \
	vfs_reader.o        \
	virtpin.o           \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/oofatfs/,\
	ff.o                \
	ffunicode.o         \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/drivers/,\
	bus/softspi.o       \
	dht/dht.o           \
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
	code/numpy/io/io.o                  \
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
	lib/lwip/src/core/*.o      \
	lib/lwip/src/core/*/*.o    \
	lib/lwip/src/netif/*.o     \
	lib/lwip/src/apps/*/*.o    \
	extmod/modlwip.o           \
	extmod/modwebsocket.o      \
	extmod/modwebrepl.o        \
	extmod/network_lwip.o      \
	)
endif

ifeq ($(MICROPY_PY_NETWORK_CYW43), 1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	lib/cyw43-driver/src/*.o    \
	extmod/network_cyw43.o      \
	)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/drivers/,\
	cyw43/cywbt.o               \
	)
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

ifeq ($(OMV_ENABLE_UVC), 1)
UVC = uvc
# UVC object files
UVC_OBJ += $(wildcard $(BUILD)/$(UVC_DIR)/src/*.o)
UVC_OBJ += $(wildcard $(BUILD)/$(HAL_DIR)/drivers/*.o)
UVC_OBJ += $(addprefix $(BUILD)/$(CMSIS_DIR)/src/,\
	$(STARTUP).o                                \
	$(SYSTEM).o                                 \
	)

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/alloc/, \
	fb_alloc.o                              \
	unaligned_memcpy.o                      \
	)

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/common/, \
	array.o                                 \
	trace.o                                 \
	mutex.o                                 \
	vospi.o                                 \
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

UVC_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/ports/$(PORT),\
	sensor.o                                \
	soft_i2c.o                              \
	ulpi.o                                  \
	omv_spi.o                               \
	omv_i2c.o                               \
	omv_gpio.o                              \
	mimxrt_hal.o                            \
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
export AFLAGS
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
	$(MAKE)  -C $(MLX90621_DIR)              BUILD=$(BUILD)/$(MLX90621_DIR)     CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MLX90640_DIR)              BUILD=$(BUILD)/$(MLX90640_DIR)     CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MLX90641_DIR)              BUILD=$(BUILD)/$(MLX90641_DIR)     CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(VL53L5CX_DIR)              BUILD=$(BUILD)/$(VL53L5CX_DIR)     CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(PIXART_DIR)                BUILD=$(BUILD)/$(PIXART_DIR)       CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(OMV_DIR)                   BUILD=$(BUILD)/$(OMV_DIR)          CFLAGS="$(CFLAGS) -MMD"
ifeq ($(OMV_ENABLE_UVC), 1)
UVC_OBJS: FIRMWARE_OBJS
	$(MAKE)  -C $(UVC_DIR)                   BUILD=$(BUILD)/$(UVC_DIR)          CFLAGS="$(UVC_CFLAGS) -MMD"
endif

# This target generates the main/app firmware image located at 0x08010000
$(FIRMWARE): FIRMWARE_OBJS
	$(CPP) -P -E -DLINKER_SCRIPT -I$(OMV_BOARD_CONFIG_DIR) $(OMV_DIR)/ports/$(PORT)/$(LDSCRIPT).ld.S > $(BUILD)/$(LDSCRIPT).lds
	$(CC) $(LDFLAGS) $(FIRM_OBJ) -o $(FW_DIR)/$(FIRMWARE).elf $(LIBS) -lgcc
	$(OBJCOPY) -Obinary -R .big_const* $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin
#	$(PYTHON) $(MKDFU) -D $(DFU_DEVICE) -b $(MAIN_APP_ADDR):$(FW_DIR)/$(FIRMWARE).bin $(FW_DIR)/$(FIRMWARE).dfu

ifeq ($(OMV_ENABLE_UVC), 1)
# This target generates the UVC firmware.
$(UVC): FIRMWARE_OBJS UVC_OBJS
	$(CPP) -P -E -I$(OMV_BOARD_CONFIG_DIR) $(UVC_DIR)/$(LD_SCRIPT).ld.S > $(BUILD)/$(UVC_DIR)/$(LD_SCRIPT).lds
	$(CC) $(UVC_LDFLAGS) $(UVC_OBJ) -o $(FW_DIR)/$(UVC).elf -lgcc
	$(OBJCOPY) -Obinary $(FW_DIR)/$(UVC).elf $(FW_DIR)/$(UVC).bin
	$(PYTHON) $(MKDFU) -D $(DFU_DEVICE) -b $(MAIN_APP_ADDR):$(FW_DIR)/$(UVC).bin $(FW_DIR)/$(UVC).dfu
endif

# This target generates the uvc and firmware images.
$(OPENMV): $(UVC) $(FIRMWARE)
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

size:
	$(SIZE) --format=SysV $(FW_DIR)/$(FIRMWARE).elf

# Flash the main firmware image
flash_image::
	$(PYDFU) -u $(FW_DIR)/$(FIRMWARE).dfu

# Flash the main firmware image using dfu_util
flash_image_dfu_util::
	dfu-util -a 0 -d $(DFU_DEVICE) -D $(FW_DIR)/$(FIRMWARE).dfu

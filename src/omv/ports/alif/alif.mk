#Copyright (C) 2023-2024 OpenMV, LLC.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Any redistribution, use, or modification in source or binary form
#    is done solely for personal benefit and not for any commercial
#    purpose or for monetary gain. For commercial licensing options,
#    please contact openmv@openmv.io
#
# THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
# OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Include OpenMV board config.
include $(OMV_BOARD_CONFIG_DIR)/omv_boardconfig.mk

LDSCRIPT  ?= alif
BUILD := $(BUILD)/$(MCU_CORE)
FIRMWARE := $(FIRMWARE)_$(MCU_CORE)
DAVE2D_DIR=drivers/dave2d
CORE_M55_HP := $(if $(filter M55_HP,$(MCU_CORE)),1,0)
CMSIS_MCU_H := '<system_utils.h>'

ROMFS_CONFIG := $(OMV_BOARD_CONFIG_DIR)/romfs.json
ROMFS_PART := $(if $(filter M55_HE,$(MCU_CORE)),0,1)
ROMFS_IMAGE := $(FW_DIR)/romfs$(ROMFS_PART).stamp

# Compiler Flags
CFLAGS += -std=gnu99 \
          -Wall \
          -Werror \
          -Warray-bounds \
          -nostartfiles \
          -fdata-sections \
          -ffunction-sections \
          -fno-inline-small-functions \
          -mfloat-abi=hard \
          -mthumb \
          -mcpu=$(CPU) \
          -mtune=$(CPU) \
          -mfpu=$(FPU) \
          -march=armv8.1-m.main+fp+mve.fp \
          -fsingle-precision-constant \
          -Wdouble-promotion

CFLAGS += -D__VFP_FP__ \
          -D$(TARGET) \
          -D$(MCU_CORE) \
          -DCORE_$(MCU_CORE)=1 \
          -D$(ARM_MATH) \
          -DARM_NN_TRUNCATE \
          -DETHOS_U \
          -DPINS_AF_H=$(PINS_AF_H) \
          -DCMSIS_MCU_H=$(CMSIS_MCU_H) \
          -DOMV_NOSYS_STUBS_ENABLE=1 \
          -DTUSB_ALIF_NO_IRQ_CFG \
          -DWITH_MM_FIXED_RANGE #WITH_MM_DYNAMIC -DNO_MSIZE
ifeq ($(MCU_CORE),M55_HP)
CFLAGS += -DRTE_LPUART_SELECT_DMA0=1 \
          -DRTE_LPSPI_SELECT_DMA0=1 \
          -DRTE_LPSPI_SELECT_DMA0_GROUP=1 \
          -DRTE_LPI2S_SELECT_DMA0=1 \
          -DRTE_LPSPI_SELECT_DMA0=1 \
          -DRTE_LPPDM_SELECT_DMA0=1
endif

CFLAGS_H += -DALIF_CMSIS_H=$(CMSIS_MCU_H)

CLANG_FLAGS = -fshort-enums \
              --target=armv8.1m-none-eabi \
              -march=armv8.1-m.main+mve.fp+fp.dp \
              -Wno-shift-count-overflow \
              -Wno-ignored-optimization-argument \
              -Wno-unused-command-line-argument \
              -D__ARMCC_VERSION=6100100 \
              -DALIF_CMSIS_H=$(CMSIS_MCU_H) \
              $(filter-out -march%,$(CFLAGS))

HAL_CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/
HAL_CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/alif

HAL_CFLAGS += -I$(OMV_BOARD_CONFIG_DIR)
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/drivers/include/
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/ospi_xip/source/ospi/
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/se_services/include
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/Device/common/config/
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/Device/common/include/
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/Device/core/$(MCU_CORE)/config/
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/Device/core/$(MCU_CORE)/include/
HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/Device/$(MCU_SERIES)/$(MCU_VARIANT)/

MPY_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)
MPY_CFLAGS += -I$(MP_BOARD_CONFIG_DIR)
MPY_CFLAGS += -I$(BUILD)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/tinyusb/src
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/lwip/src/include
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/mbedtls/include
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/shared/runtime
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/alif
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/alif/tinyusb_port
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/alif/lwip_inc

MPY_CFLAGS += -DMICROPY_PY_CSI=$(MICROPY_PY_CSI)
MPY_CFLAGS += -DMICROPY_PY_LWIP=$(MICROPY_PY_LWIP)
MPY_CFLAGS += -DMICROPY_PY_SSL=$(MICROPY_PY_SSL)
MPY_CFLAGS += -DMICROPY_SSL_MBEDTLS=$(MICROPY_SSL_MBEDTLS)
MPY_CFLAGS += -DMICROPY_PY_NETWORK_CYW43=$(MICROPY_PY_NETWORK_CYW43)
MPY_CFLAGS += -DMICROPY_PY_BLUETOOTH=$(MICROPY_PY_BLUETOOTH)
MPY_CFLAGS += -DMICROPY_BLUETOOTH_NIMBLE=$(MICROPY_BLUETOOTH_NIMBLE)

MPY_MKARGS += MCU_CORE=$(MCU_CORE)
MPY_MKARGS += MICROPY_VFS_LFS2=0
MPY_MKARGS += MICROPY_FLOAT_IMPL=float
MPY_MKARGS += ALIF_DFP_REL_HERE=$(TOP_DIR)/$(HAL_DIR)
MPY_MKARGS += CMSIS_DIR=$(TOP_DIR)/$(HAL_DIR)/cmsis/inc
MPY_MKARGS += MICROPY_PY_CSI=$(MICROPY_PY_CSI)
MPY_MKARGS += MICROPY_PY_LWIP=$(MICROPY_PY_LWIP)
MPY_MKARGS += MICROPY_PY_SSL=$(MICROPY_PY_SSL)
MPY_MKARGS += MICROPY_SSL_MBEDTLS=$(MICROPY_SSL_MBEDTLS)
MPY_MKARGS += MICROPY_PY_NETWORK_CYW43=$(MICROPY_PY_NETWORK_CYW43)
MPY_MKARGS += MICROPY_PY_BLUETOOTH=$(MICROPY_PY_BLUETOOTH)
MPY_MKARGS += MICROPY_BLUETOOTH_NIMBLE=$(MICROPY_BLUETOOTH_NIMBLE)
MPY_MKARGS += MICROPY_PY_OPENAMP=$(MICROPY_PY_OPENAMP)
MPY_MKARGS += MICROPY_PY_OPENAMP_REMOTEPROC=$(MICROPY_PY_OPENAMP_REMOTEPROC) 
MPY_MKARGS += MICROPY_MANIFEST_MCU_CORE=$(shell echo $(MCU_CORE) | awk -F'_' '{print tolower($$2)}')

ifeq ($(MCU_CORE),M55_HP)
MPY_MKARGS += MICROPY_PY_OPENAMP_HOST=1
else ifeq ($(MCU_CORE),M55_HE)
MPY_MKARGS += MICROPY_PY_OPENAMP_DEVICE=1
else
$(error Invalid MCU core specified))
endif

ifeq ($(MCU_CORE),M55_HP)
VELA_ARGS="--system-config RTSS_HP_DTCM_MRAM \
           --accelerator-config ethos-u55-256 \
           --memory-mode Shared_Sram"
else
VELA_ARGS="--system-config RTSS_HE_SRAM_MRAM \
           --accelerator-config ethos-u55-128 \
           --memory-mode Shared_Sram"
endif

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
OMV_CFLAGS += -I$(TOP_DIR)/$(LSM6DSM_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(VL53L5CX_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(VL53L8CX_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(WINC1500_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(MLX90621_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(MLX90640_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(MLX90641_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(PIXART_DIR)/include/
OMV_CFLAGS += -I$(TOP_DIR)/$(DAVE2D_DIR)/include

CFLAGS += $(HAL_CFLAGS) $(MPY_CFLAGS) $(OMV_CFLAGS)

# Linker Flags
LDFLAGS = -mthumb \
          -mcpu=$(CPU) \
          -mfpu=$(FPU) \
          -mfloat-abi=hard \
          -mabi=aapcs-linux \
          -z noexecstack \
          -Wl,--print-memory-usage \
          -Wl,--gc-sections \
          -Wl,--no-warn-rwx-segment \
          -Wl,-Map=$(BUILD)/$(FIRMWARE).map \
          -Wl,-T$(BUILD)/$(LDSCRIPT).lds

ifeq ($(MCU_CORE),M55_HP)
# Linker Flags
LDFLAGS += -Wl,--wrap=mp_usbd_task \
           -Wl,--wrap=tud_cdc_rx_cb \
           -Wl,--wrap=mp_hal_stdio_poll \
           -Wl,--wrap=mp_hal_stdout_tx_strn
endif

# CMSIS Objects
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/CommonTables/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(CMSIS_DIR)/src/dsp/FastMathFunctions/*.o)

# HAL Objects
FIRM_OBJ += $(wildcard $(BUILD)/$(HAL_DIR)/drivers/source/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(HAL_DIR)/Device/common/source/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(HAL_DIR)/Device/core/$(MCU_CORE)/source/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(HAL_DIR)/ospi_xip/source/ospi/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(HAL_DIR)/se_services/source/*.o)

# Drivers
FIRM_OBJ += $(wildcard $(BUILD)/$(LEPTON_DIR)/src/*.o)
ifeq ($(MICROPY_PY_IMU), 1)
FIRM_OBJ += $(wildcard $(BUILD)/$(LSM6DSM_DIR)/src/*.o)
endif
FIRM_OBJ += $(wildcard $(BUILD)/$(MLX90621_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MLX90640_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MLX90641_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(VL53L5CX_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(VL53L8CX_DIR)/src/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(PIXART_DIR)/src/*.o)
ifeq ($(MCU_CORE),M55_HP)
FIRM_OBJ += $(wildcard $(BUILD)/$(DAVE2D_DIR)/src/*.o)
endif

# OpenMV Objects
FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/alloc/, \
	xalloc.o                    \
	fb_alloc.o                  \
	umm_malloc.o                \
	unaligned_memcpy.o          \
   )

FIRM_OBJ += $(addprefix $(BUILD)/$(OMV_DIR)/common/, \
	array.o                     \
	ringbuf.o                   \
	trace.o                     \
	mutex.o                     \
	vospi.o                     \
	pendsv.o                    \
	usbdbg.o                    \
	tinyusb_debug.o             \
	file_utils.o                \
	mp_utils.o                  \
	nosys_stubs.o               \
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
	hm0360.o                    \
	gc2145.o                    \
	pag7920.o                   \
	pag7936.o                   \
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

#------------- MicroPy Objects -------------------#
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/modules/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/ports/$(PORT)/modules/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/py/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/boards/$(TARGET)/*.o)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	alif_flash.o \
	cyw43_port_spi.o \
	fatfs_port.o \
	frozen_content.o \
	machine_pin.o \
	machine_i2c.o \
	machine_spi.o \
	machine_rtc.o \
	modalif.o \
	mphalport.o \
	mpnetworkport.o \
	mpuart.o \
	msc_disk.o \
	ospi_ext.o \
	ospi_flash.o \
	system_tick.o \
	usbd.o \
	pins_board.o \
	se_services.o \
	vfs_rom_ioctl.o \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/, \
	lib/tinyusb/src/tusb.o \
	lib/tinyusb/src/class/cdc/cdc_device.o \
	lib/tinyusb/src/class/msc/msc_device.o \
	lib/tinyusb/src/common/tusb_fifo.o \
	lib/tinyusb/src/device/usbd.o \
	lib/tinyusb/src/device/usbd_control.o \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/shared/,\
	libc/printf.o \
	libc/string0.o \
	readline/readline.o \
	runtime/gchelper_thumb2.o \
	runtime/gchelper_native.o \
	runtime/interrupt_char.o \
	runtime/mpirq.o \
	runtime/pyexec.o \
	runtime/softtimer.o  \
	runtime/stdout_helpers.o \
	runtime/sys_stdio_mphal.o \
	timeutils/timeutils.o \
	tinyusb/mp_usbd.o \
	tinyusb/mp_usbd_cdc.o \
	tinyusb/mp_usbd_descriptor.o \
	)

FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/lib/mbedtls/library/*.o)
FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/lib/mbedtls_errors/*.o)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/extmod/,\
	machine_adc.o           \
	machine_adc_block.o     \
	machine_bitstream.o     \
	machine_i2c.o           \
	machine_i2s.o           \
	machine_mem.o           \
	machine_pinbase.o       \
	machine_pulse.o         \
	machine_pwm.o           \
	machine_signal.o        \
	machine_spi.o           \
	machine_timer.o         \
	machine_uart.o          \
	machine_usb_device.o    \
	machine_wdt.o           \
	modasyncio.o            \
	modbinascii.o           \
	modbtree.o              \
	modcryptolib.o          \
	moddeflate.o            \
	modframebuf.o           \
	modhashlib.o            \
	modheapq.o              \
	modjson.o               \
	modmachine.o            \
	modmarshal.o            \
	modnetwork.o            \
	modonewire.o            \
	modopenamp.o            \
	modopenamp_remoteproc.o \
	modopenamp_remoteproc_store.o \
	modos.o                 \
	modplatform.o           \
	modrandom.o             \
	modre.o                 \
	modselect.o             \
	modsocket.o             \
	modtls_axtls.o          \
	modtls_mbedtls.o        \
	modtime.o               \
	moductypes.o            \
	modvfs.o                \
	network_esp_hosted.o    \
	network_ninaw10.o       \
	network_wiznet5k.o      \
	os_dupterm.o            \
	vfs.o                   \
	vfs_blockdev.o          \
	vfs_fat.o               \
	vfs_fat_diskio.o        \
	vfs_fat_file.o          \
	vfs_lfs.o               \
	vfs_rom.o               \
	vfs_rom_file.o          \
	vfs_posix.o             \
	vfs_posix_file.o        \
	vfs_reader.o            \
	virtpin.o               \
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
	scipy/integrate/integrate.o     \
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

ifeq ($(MICROPY_PY_NETWORK), 1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/shared/,\
	netutils/dhcpserver.o \
	netutils/netutils.o \
	netutils/trace.o \
	)
endif

ifeq ($(MICROPY_SSL_MBEDTLS), 1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	mbedtls/mbedtls_port.o  \
	)
endif

ifeq ($(MICROPY_PY_NETWORK_CYW43), 1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/cyw43-driver/,\
	src/cyw43_bthci_uart.o \
	src/cyw43_ctrl.o \
	src/cyw43_lwip.o \
	src/cyw43_ll.o \
	src/cyw43_sdio.o \
	src/cyw43_spi.o \
	src/cyw43_stats.o \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	extmod/network_cyw43.o \
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

ifeq ($(MICROPY_PY_OPENAMP),1)
FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/openamp/metal/,\
	device.o                        \
	dma.o                           \
	init.o                          \
	io.o                            \
	irq.o                           \
	log.o                           \
	shmem.o                         \
	softirq.o                       \
	version.o                       \
	system/micropython/condition.o  \
	system/micropython/device.o     \
	system/micropython/io.o         \
	system/micropython/irq.o        \
	system/micropython/shmem.o      \
	system/micropython/time.o       \
	)

FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,   \
	mpmetalport.o                                   \
	mpremoteprocport.o                              \
	lib/open-amp/lib/virtio/virtio.o                \
	lib/open-amp/lib/virtio/virtqueue.o             \
	lib/open-amp/lib/virtio_mmio/virtio_mmio_drv.o  \
	lib/open-amp/lib/rpmsg/rpmsg.o                  \
	lib/open-amp/lib/rpmsg/rpmsg_virtio.o           \
	lib/open-amp/lib/remoteproc/elf_loader.o        \
	lib/open-amp/lib/remoteproc/remoteproc.o        \
	lib/open-amp/lib/remoteproc/remoteproc_virtio.o \
	lib/open-amp/lib/remoteproc/rsc_table_parser.o  \
	)
endif

# Libraries
ifeq ($(MICROPY_PY_ML_TFLM), 1)
OMV_CFLAGS += -I$(BUILD)/$(TENSORFLOW_DIR)/
OMV_CFLAGS += -I$(TOP_DIR)/$(TENSORFLOW_DIR)/libtflm/include
OMV_CFLAGS += -I$(TOP_DIR)/$(TENSORFLOW_DIR)/libtflm/include/third_party/ethos_u_core_driver/include/

FIRM_OBJ += $(addprefix $(BUILD)/$(TENSORFLOW_DIR)/, \
	tflm_backend.o \
	)
LIBS += $(TOP_DIR)/$(TENSORFLOW_DIR)/libtflm/lib/libtflm-$(CPU)-u55-release.a
endif

export VELA_ARGS
export CLANG_FLAGS
export USE_CLANG=1

all: $(FIRMWARE) .WAIT $(ROMFS_IMAGE)
	$(SIZE) $(FW_DIR)/$(FIRMWARE).elf

FIRMWARE_OBJS:
	$(MAKE)  -C $(CMSIS_DIR)                 BUILD=$(BUILD)/$(CMSIS_DIR)        CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(HAL_DIR)                   BUILD=$(BUILD)/$(HAL_DIR)          CFLAGS="$(CFLAGS) -MMD"
ifeq ($(MCU_CORE),M55_HP)
	$(MAKE)  -C $(DAVE2D_DIR)                BUILD=$(BUILD)/$(DAVE2D_DIR)       CFLAGS="$(CFLAGS) -MMD"
endif
	$(MAKE)  -C $(MICROPY_DIR)/ports/$(PORT) -f alif.mk BUILD=$(BUILD)/$(MICROPY_DIR) $(MPY_MKARGS) obj
ifeq (0, 1)
	$(MAKE)  -C $(LEPTON_DIR)                BUILD=$(BUILD)/$(LEPTON_DIR)       CFLAGS="$(CFLAGS) -MMD"
endif
ifeq ($(MICROPY_PY_IMU), 1)
	$(MAKE)  -C $(LSM6DSM_DIR)               BUILD=$(BUILD)/$(LSM6DSM_DIR)      CFLAGS="$(CFLAGS) -MMD"
endif
	$(MAKE)  -C $(MLX90621_DIR)              BUILD=$(BUILD)/$(MLX90621_DIR)     CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MLX90640_DIR)              BUILD=$(BUILD)/$(MLX90640_DIR)     CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(MLX90641_DIR)              BUILD=$(BUILD)/$(MLX90641_DIR)     CFLAGS="$(CFLAGS) -MMD"
	$(MAKE)  -C $(VL53L5CX_DIR)              BUILD=$(BUILD)/$(VL53L5CX_DIR)     CFLAGS="$(CFLAGS) $(CFLAGS_H) -MMD"
	$(MAKE)  -C $(VL53L8CX_DIR)              BUILD=$(BUILD)/$(VL53L8CX_DIR)     CFLAGS="$(CFLAGS) $(CFLAGS_H) -MMD"
ifeq ($(MICROPY_PY_ML_TFLM), 1)
	$(MAKE)  -C $(TENSORFLOW_DIR)            BUILD=$(BUILD)/$(TENSORFLOW_DIR)   CFLAGS="$(CFLAGS) -MMD"
endif
	$(MAKE)  -C $(OMV_DIR)                   BUILD=$(BUILD)/$(OMV_DIR)          CFLAGS="$(CFLAGS) $(CFLAGS_H) -MMD"

$(ROMFS_IMAGE): $(ROMFS_CONFIG)
	$(ECHO) "GEN $(FW_DIR)/romfs_$(MCU_CORE).img"
	$(PYTHON) $(TOOLS)/$(MKROMFS) --top-dir $(TOP_DIR) --build-dir $(BUILD) --out-dir $(FW_DIR) \
                                  --partition $(ROMFS_PART) --vela-args $(VELA_ARGS) --config $(ROMFS_CONFIG)
	touch $@

$(FIRMWARE): FIRMWARE_OBJS
	$(CPP) -P -E -DLINKER_SCRIPT -DCORE_$(MCU_CORE) -I$(OMV_COMMON_DIR) -I$(OMV_BOARD_CONFIG_DIR) \
                    $(OMV_DIR)/ports/$(PORT)/$(LDSCRIPT).ld.S > $(BUILD)/$(LDSCRIPT).lds
	$(CC) $(LDFLAGS) $(FIRM_OBJ) -o $(FW_DIR)/$(FIRMWARE).elf $(LIBS) -lm
	$(OBJCOPY) -Obinary $(FW_DIR)/$(FIRMWARE).elf $(FW_DIR)/$(FIRMWARE).bin
	BIN_SIZE=$$(stat -c%s "$(FW_DIR)/$(FIRMWARE).bin"); \
    PADDED_SIZE=$$(( (BIN_SIZE + 15) / 16 * 16 )); \
    if [ $$BIN_SIZE -lt $$PADDED_SIZE ]; then \
        dd if=/dev/zero bs=1 count=$$((PADDED_SIZE - BIN_SIZE)) >> $(FW_DIR)/$(FIRMWARE).bin; \
    fi

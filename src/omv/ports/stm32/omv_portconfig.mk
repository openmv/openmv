# Set startup and system files based on MCU.
SYSTEM    ?= st/system_stm32fxxx
STARTUP   ?= st/startup_$(shell echo $(MCU) | tr '[:upper:]' '[:lower:]')
LDSCRIPT  ?= stm32fxxx

OMV_PORT_CFLAGS     += -DSTM32_HAL_H=$(HAL_INC)
OMV_PORT_HAL_CFLAGS += -I$(TOP_DIR)/$(CMSIS_DIR)/include/st
OMV_PORT_HAL_CFLAGS += -I$(TOP_DIR)/$(HAL_DIR)/include/Legacy/

OMV_PORT_MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs
OMV_PORT_MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/lwip/src/include/
OMV_PORT_MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/
OMV_PORT_MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/usbdev/core/inc/
OMV_PORT_MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/usbdev/class/inc/
OMV_PORT_MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/stm32/lwip_inc/

#------------- MicroPy Objects ----------------#
OMV_PORT_MPY_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
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
	modusocket.o            \
	network_lan.o           \
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
	frozen_content.o        \
	)

#------------- MicroPy Objects ----------------#
OMV_PORT_MPY_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/,\
	utils/mpirq.o               \
	utils/pyexec.o              \
	utils/printf.o              \
	utils/interrupt_char.o      \
	utils/sys_stdio_mphal.o     \
	utils/gchelper_m3.o         \
	utils/gchelper_native.o     \
	libc/string0.o              \
	netutils/*.o                \
	timeutils/timeutils.o       \
	berkeley-db-1.xx/btree/*.o  \
	berkeley-db-1.xx/mpool/*.o  \
	embed/abort_.o              \
	mp-readline/readline.o      \
	)

OMV_PORT_MPY_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/libm/,\
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
OMV_PORT_MPY_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/lib/mbedtls/library/*.o)
OMV_PORT_MPY_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/lib/mbedtls_errors/*.o)

OMV_PORT_MPY_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/usbdev/, \
	core/src/usbd_core.o                \
	core/src/usbd_ctlreq.o              \
	core/src/usbd_ioreq.o               \
	class/src/usbd_cdc_msc_hid.o        \
	class/src/usbd_msc_bot.o            \
	class/src/usbd_msc_scsi.o           \
	)

OMV_PORT_MPY_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/extmod/,\
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
	moduasyncio.o       \
	)

OMV_PORT_MPY_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/oofatfs/,\
	ff.o                \
	ffunicode.o         \
	)

OMV_PORT_MPY_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/drivers/,\
	bus/softspi.o       \
	dht/dht.o           \
	memory/spiflash.o   \
	)


# SPDX-License-Identifier: MIT
#
# Copyright (C) 2025 OpenMV, LLC.
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
#
# Collects MicroPython object files

# Add core py object files
MPY_FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/py/*.o)

# Add extmod object files.
MPY_FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/extmod/*.o)

# Add shared object files.
MPY_FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/shared/**/*.o)

# Add driver object files.
MPY_FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/drivers/*.o)
MPY_FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/drivers/**/*.o)

# Add C user modules.
MPY_FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/modules/*.o)
MPY_FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/modules/**/*.o)
MPY_FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/ports/$(PORT)/modules/*.o)

# Add lwIP core, netif, apps, and related network modules
ifeq ($(MICROPY_PY_LWIP), 1)
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
	mpnetworkport.o \
    lib/lwip/src/core/*.o \
    lib/lwip/src/core/*/*.o \
    lib/lwip/src/netif/*.o \
    lib/lwip/src/apps/*/*.o \
)
endif

# Add mbedTLS crypto and error libraries
ifeq ($(MICROPY_SSL_MBEDTLS), 1)
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
    mbedtls/*.o \
    lib/mbedtls/library/*.o \
    lib/mbedtls_errors/*.o \
)
endif

# Add CYW43 Wi-Fi driver and network glue
ifeq ($(MICROPY_PY_NETWORK_CYW43), 1)
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/cyw43-driver/,\
    src/cyw43_bthci_uart.o \
    src/cyw43_ctrl.o \
    src/cyw43_lwip.o \
    src/cyw43_ll.o \
    src/cyw43_sdio.o \
    src/cyw43_spi.o \
    src/cyw43_stats.o \
)
endif

# Add NimBLE Bluetooth stack and glue code
ifeq ($(MICROPY_BLUETOOTH_NIMBLE),1)
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
    mpbthciport.o \
    mpnimbleport.o \
    extmod/nimble/modbluetooth_nimble.o \
    extmod/nimble/nimble/nimble_npl_os.o \
    extmod/nimble/hal/hal_uart.o \
)

MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/,\
    mynewt-nimble/ext/tinycrypt/src/*.o \
    mynewt-nimble/nimble/host/services/gap/src/*.o \
    mynewt-nimble/nimble/host/services/gatt/src/*.o \
    mynewt-nimble/nimble/host/src/*.o \
    mynewt-nimble/nimble/host/util/src/*.o \
    mynewt-nimble/nimble/transport/uart/src/*.o \
    mynewt-nimble/porting/nimble/src/*.o \
)
endif

# Add TinyUSB library objects.
ifeq ($(OMV_USB_STACK_TINYUSB), 1)
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/lib/tinyusb/, \
	src/*.o \
	src/common/*.o \
	src/device/*.o \
	src/class/**/*.o \
)

MPY_FIRM_OBJ += \
    $(wildcard $(BUILD)/$(MICROPY_DIR)/lib/tinyusb/src/portable/**/**/*.o)
endif

# Add oofatfs library
MPY_FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/lib/oofatfs/*.o)

# Add OpenAMP metal layer and remoteproc/rpmsg stack
ifeq ($(MICROPY_PY_OPENAMP),1)
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/openamp/metal/,\
    device.o \
    dma.o \
    init.o \
    io.o \
    irq.o \
    log.o \
    shmem.o \
    softirq.o \
    version.o \
    system/micropython/condition.o \
    system/micropython/device.o \
    system/micropython/io.o \
    system/micropython/irq.o \
    system/micropython/shmem.o \
    system/micropython/time.o \
)

MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/,\
    mpmetalport.o \
    mpremoteprocport.o \
    lib/open-amp/lib/virtio/virtio.o \
    lib/open-amp/lib/virtio/virtqueue.o \
    lib/open-amp/lib/virtio_mmio/virtio_mmio_drv.o \
    lib/open-amp/lib/rpmsg/rpmsg.o \
    lib/open-amp/lib/rpmsg/rpmsg_virtio.o \
    lib/open-amp/lib/remoteproc/elf_loader.o \
    lib/open-amp/lib/remoteproc/remoteproc.o \
    lib/open-amp/lib/remoteproc/remoteproc_virtio.o \
    lib/open-amp/lib/remoteproc/rsc_table_parser.o \
)
endif

# Add ULAB NumPy/SciPy-like modules
ifeq ($(MICROPY_PY_ULAB), 1)
MPY_FIRM_OBJ += $(addprefix $(BUILD)/$(MICROPY_DIR)/modules/ulab/code/,\
    ndarray.o \
    ndarray_operators.o \
    ndarray_properties.o \
    numpy/approx.o \
    numpy/bitwise.o \
    numpy/carray/carray.o \
    numpy/carray/carray_tools.o \
    numpy/compare.o \
    numpy/create.o \
    numpy/fft/fft.o \
    numpy/fft/fft_tools.o \
    numpy/filter.o \
    numpy/io/io.o \
    numpy/linalg/linalg.o \
    numpy/linalg/linalg_tools.o \
    numpy/ndarray/ndarray_iter.o \
    numpy/numerical.o \
    numpy/numpy.o \
    numpy/poly.o \
    numpy/random/random.o \
    numpy/stats.o \
    numpy/transform.o \
    numpy/vector.o \
    scipy/integrate/integrate.o \
    scipy/linalg/linalg.o \
    scipy/optimize/optimize.o \
    scipy/scipy.o \
    scipy/signal/signal.o \
    scipy/special/special.o \
    ulab.o \
    ulab_tools.o \
    user/user.o \
    utils/utils.o \
)
endif

# Add board object files (board.o, board_init etc...)
MPY_FIRM_OBJ += $(wildcard $(BUILD)/$(MICROPY_DIR)/boards/$(TARGET)/*.o)

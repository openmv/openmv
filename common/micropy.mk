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
# Configures built-in MicroPython modules, builds MicroPython, and
# archives all objects into libmicropython.a for linking.
#
# Ports must define MPY_LIB_EXCLUDE before including this file to
# exclude port-specific objects that conflict with the OpenMV build.

# +-----------------------------------------------------+
# | Module configuration                                |
# +-----------------------------------------------------+
# Note: must define both the CFLAGS and Make args.
ifeq ($(MICROPY_PY_CSI), 1)
MPY_CFLAGS += -DMICROPY_PY_CSI=1
MPY_MKARGS += MICROPY_PY_CSI=1
endif

ifeq ($(MICROPY_PY_CSI_NG), 1)
MPY_CFLAGS += -DMICROPY_PY_CSI_NG=1
MPY_MKARGS += MICROPY_PY_CSI_NG=1
endif

ifeq ($(MICROPY_PY_FIR), 1)
MPY_CFLAGS += -DMICROPY_PY_FIR=1
MPY_MKARGS += MICROPY_PY_FIR=1
endif

ifeq ($(MICROPY_PY_WINC1500), 1)
MPY_CFLAGS += -DMICROPY_PY_WINC1500=1
MPY_MKARGS += MICROPY_PY_WINC1500=1
MPY_PENDSV_ENTRIES += PENDSV_DISPATCH_WINC,
endif

ifeq ($(MICROPY_PY_IMU), 1)
MPY_CFLAGS += -DMICROPY_PY_IMU=1
MPY_MKARGS += MICROPY_PY_IMU=1
endif

ifeq ($(MICROPY_PY_CRC), 1)
MPY_CFLAGS += -DMICROPY_PY_CRC=1
MPY_MKARGS += MICROPY_PY_CRC=1
endif

ifeq ($(MICROPY_PY_BTREE), 1)
MPY_CFLAGS += -DMICROPY_PY_BTREE=1
MPY_MKARGS += MICROPY_PY_BTREE=1
endif

ifeq ($(MICROPY_PY_TOF), 1)
MPY_CFLAGS += -DMICROPY_PY_TOF=1
MPY_MKARGS += MICROPY_PY_TOF=1
endif

ifeq ($(MICROPY_PY_AUDIO), 1)
MPY_CFLAGS += -DMICROPY_PY_AUDIO=1
MPY_MKARGS += MICROPY_PY_AUDIO=1
endif

ifeq ($(MICROPY_PY_DISPLAY), 1)
MPY_CFLAGS += -DMICROPY_PY_DISPLAY=1
MPY_MKARGS += MICROPY_PY_DISPLAY=1
endif

ifeq ($(MICROPY_PY_TV), 1)
MPY_CFLAGS += -DMICROPY_PY_TV=1
MPY_MKARGS += MICROPY_PY_TV=1
endif

ifeq ($(CUBEAI), 1)
MPY_CFLAGS += -DMICROPY_PY_CUBEAI=1
MPY_MKARGS += MICROPY_PY_CUBEAI=1
endif

ifeq ($(MICROPY_PY_ML), 1)
MPY_CFLAGS += -DMICROPY_PY_ML=1
MPY_MKARGS += MICROPY_PY_ML=1
endif

ifeq ($(MICROPY_PY_ML_TFLM), 1)
MPY_CFLAGS += -DMICROPY_PY_ML_TFLM=1
MPY_MKARGS += MICROPY_PY_ML_TFLM=1
endif

ifeq ($(MICROPY_PY_ML_STAI), 1)
MPY_CFLAGS += -DMICROPY_PY_ML_STAI=1
MPY_MKARGS += MICROPY_PY_ML_STAI=1
endif

ifeq ($(MICROPY_PY_UNITTEST), 1)
MPY_CFLAGS += -DMICROPY_PY_UNITTEST=1
MPY_MKARGS += MICROPY_PY_UNITTEST=1
endif

ifeq ($(MICROPY_PY_UMALLOC), 1)
MPY_CFLAGS += -DMICROPY_PY_UMALLOC=1
MPY_MKARGS += MICROPY_PY_UMALLOC=1
endif

ifeq ($(MICROPY_PY_PROTOCOL), 1)
MPY_CFLAGS += -DMICROPY_PY_PROTOCOL=1
MPY_MKARGS += MICROPY_PY_PROTOCOL=1
ifeq ($(OMV_USB_STACK_TINYUSB), 1)
MPY_PENDSV_ENTRIES += PENDSV_DISPATCH_OMV_PROTOCOL,
endif
endif

# +-----------------------------------------------------+
# | lwIP core, netif, apps, and related network modules.|
# +-----------------------------------------------------+
ifeq ($(MICROPY_PY_LWIP), 1)
MPY_CFLAGS += -DMICROPY_PY_LWIP=1
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/lwip/src/include
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/ports/$(PORT)/lwip_inc

MPY_MKARGS += MICROPY_PY_LWIP=1
endif

# +-----------------------------------------------------+
# | SSL support and mbedTLS crypto libraries.           |
# +-----------------------------------------------------+
ifeq ($(MICROPY_SSL_MBEDTLS), 1)
MPY_CFLAGS += -DMICROPY_PY_SSL=1
MPY_CFLAGS += -DMICROPY_PY_SSL_ECDSA_SIGN_ALT=$(MICROPY_PY_SSL_ECDSA_SIGN_ALT)
MPY_CFLAGS += -DMICROPY_SSL_MBEDTLS=1
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/mbedtls/include

MPY_MKARGS += MICROPY_PY_SSL=1
MPY_MKARGS += MICROPY_PY_SSL_ECDSA_SIGN_ALT=$(MICROPY_PY_SSL_ECDSA_SIGN_ALT)
MPY_MKARGS += MICROPY_SSL_MBEDTLS=1
endif

# +-----------------------------------------------------+
# | CYW43 Wi-Fi driver and network glue.                |
# +-----------------------------------------------------+
ifeq ($(MICROPY_PY_NETWORK_CYW43), 1)
MPY_CFLAGS += -DMICROPY_PY_NETWORK_CYW43=1
MPY_MKARGS += MICROPY_PY_NETWORK_CYW43=1
endif

# +-----------------------------------------------------+
# | NimBLE Bluetooth stack and glue code.               |
# +-----------------------------------------------------+
ifeq ($(MICROPY_BLUETOOTH_NIMBLE),1)
MPY_CFLAGS += -DMICROPY_PY_BLUETOOTH=1
MPY_CFLAGS += -DMICROPY_PY_BLUETOOTH_USE_SYNC_EVENTS=1
MPY_CFLAGS += -DMICROPY_BLUETOOTH_NIMBLE=1

MPY_MKARGS += MICROPY_PY_BLUETOOTH=1
MPY_MKARGS += MICROPY_BLUETOOTH_NIMBLE=1
endif

# +-----------------------------------------------------+
# | TinyUSB library.                                    |
# +-----------------------------------------------------+
ifeq ($(OMV_USB_STACK_TINYUSB), 1)
MPY_CFLAGS += -DMICROPY_HW_TINYUSB_STACK=1
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/tinyusb/src
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/shared/tinyusb
endif

# +-----------------------------------------------------+
# | oofatfs library.                                    |
# +-----------------------------------------------------+
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/lib/oofatfs

# +-----------------------------------------------------+
# | OpenAMP metal layer and remoteproc/rpmsg stack.     |
# +-----------------------------------------------------+
# Always export these flags to override default board config
ifeq (1, 1)
MICROPY_PY_OPENAMP ?= 0
MICROPY_PY_OPENAMP_REMOTEPROC ?= 0

MPY_CFLAGS += -DMICROPY_PY_OPENAMP=$(MICROPY_PY_OPENAMP)
MPY_CFLAGS += -DMICROPY_PY_OPENAMP_REMOTEPROC=$(MICROPY_PY_OPENAMP_REMOTEPROC)

MPY_MKARGS += MICROPY_PY_OPENAMP=$(MICROPY_PY_OPENAMP)
MPY_MKARGS += MICROPY_PY_OPENAMP_REMOTEPROC=$(MICROPY_PY_OPENAMP_REMOTEPROC)
endif

# +-----------------------------------------------------+
# | ULAB NumPy/SciPy-like modules.                     |
# +-----------------------------------------------------+
ifeq ($(MICROPY_PY_ULAB), 1)
MPY_CFLAGS += -DMICROPY_PY_ULAB=1
MPY_CFLAGS += -DULAB_CONFIG_FILE="\"$(OMV_BOARD_CONFIG_DIR)/ulab_config.h\""
MPY_MKARGS += MICROPY_PY_ULAB=1
endif

# +-----------------------------------------------------+
# | MicroPython common includes and flags               |
# +-----------------------------------------------------+
MPY_CFLAGS += -I$(BUILD)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/py
MPY_CFLAGS += -I$(TOP_DIR)/$(MICROPY_DIR)/shared/runtime

MPY_CFLAGS += -DMICROPY_HW_USB_VID=$(OMV_USB_VID)
MPY_CFLAGS += -DMICROPY_HW_USB_PID=$(OMV_USB_PID)
MPY_CFLAGS += -DMP_CONFIGFILE=\<$(OMV_PORT_DIR)/mp_config.h\>

MPY_PENDSV_ENTRIES := $(shell echo $(MPY_PENDSV_ENTRIES) | tr -d '[:space:]')
MPY_CFLAGS += -DMICROPY_BOARD_PENDSV_ENTRIES="$(MPY_PENDSV_ENTRIES)"

# +-----------------------------------------------------+
# | MicroPython build rules.                            |
# +-----------------------------------------------------+
.DEFAULT_GOAL := all
MPY_MAKE_FILE ?=
MPY_MAKE_TARGET ?=
MPY_LIB = $(BUILD)/lib/libmicropython.a

.PHONY: FORCE
$(MPY_LIB): FORCE | FIRM_DIRS
	$(MAKE) -C $(MICROPY_DIR)/ports/$(PORT) $(MPY_MAKE_FILE) \
        BUILD=$(BUILD)/$(MICROPY_DIR) $(MPY_MKARGS) $(MPY_MAKE_TARGET)
	$(ECHO) "AR $@"
	$(RM) -f $@
	$(AR) rcs $@ $$(find $(BUILD)/$(MICROPY_DIR) -name '*.o' \
        ! -path '$(BUILD)/$(MICROPY_DIR)$(TOP_DIR)/*' \
        ! -name 'main.*' \
        ! -name 'pendsv.*' \
        $(MPY_LIB_EXCLUDE))

LIBS += -Wl,--whole-archive $(MPY_LIB) -Wl,--no-whole-archive

$(OMV_FIRM_OBJ): | $(MPY_LIB)

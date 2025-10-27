# SPDX-License-Identifier: MIT
#
# Copyright (C) 2013-2024 OpenMV, LLC.
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
# Drivers Makefile

# Boson driver sources
ifeq ($(OMV_BOSON_ENABLE), 1)
DRIVER_SRC_C += $(addprefix boson/src/, \
    Client_API.c \
    Client_Dispatcher.c \
    Client_Interface.c \
    Client_Packager.c \
    dataServiceClient.c \
    fifo.c \
    flirChannels.c \
    flirCRC.c \
    FSLP.c \
    I2C_Connector.c \
    MultiServiceSupport.c \
    Serializer_BuiltIn.c \
    Serializer_Struct.c \
    serialPortAdapter.c \
    timeoutLogic.c \
    UART_Connector.c \
    UPTClient.c \
)

$(BUILD)/drivers/boson/src/%.o: override CFLAGS += \
    -Wno-maybe-uninitialized \
    -Wno-unused-variable \
    -Wno-array-parameter \
    -Wno-strict-aliasing \
    -Wno-address-of-packed-member \
    -Wno-uninitialized \
    -Wno-format

CFLAGS += -DOMV_BOSON_ENABLE=1
CFLAGS += -I$(TOP_DIR)/drivers/boson/include
endif   # OMV_BOSON_ENABLE

# D/AVE 2D driver sources
ifeq ($(PORT),alif)
DRIVER_SRC_C += $(addprefix dave2d/src/, \
    dave_64bitoperation.c \
    dave_blit.c \
    dave_box.c \
    dave_circle.c \
    dave_context.c \
    dave_curve.c \
    dave_d0lib.c \
    dave_d0_mm_dynamic.c \
    dave_d0_mm_fixed_range.c \
    dave_d0_mm_fixed_range_fixed_blkcnt.c \
    dave_dlist.c \
    dave_driver.c \
    dave_edge.c \
    dave_errorcodes.c \
    dave_gradient.c \
    dave_hardware.c \
    dave_line.c \
    dave_math.c \
    dave_memory.c \
    dave_pattern.c \
    dave_perfcount.c \
    dave_polyline.c \
    dave_quad.c \
    dave_rbuffer.c \
    dave_render.c \
    dave_texture.c \
    dave_triangle.c \
    dave_utility.c \
    dave_viewport.c \
    dave_wedge.c \
)

CFLAGS += -I$(TOP_DIR)/drivers/dave2d/include
$(BUILD)/drivers/dave2d/src/%.o: override CFLAGS += -Wno-unused-value
endif   # D/AVE 2D / Alif port.

# Display driver sources
ifeq ($(MICROPY_PY_DISPLAY), 1)
DRIVER_SRC_C += $(addprefix display/src/, \
    cec.c \
)
CFLAGS += -I$(TOP_DIR)/drivers/display/include
endif   # MICROPY_PY_DISPLAY

# GenX320 driver sources
ifeq ($(OMV_GENX320_ENABLE), 1)
DRIVER_SRC_C += $(addprefix genx320/src/, \
    firmware.c \
    genx320_issd_cpi_evt.c \
    genx320_issd_cpi_histo.c \
    psee_genx320.c \
)

CFLAGS += -DOMV_GENX320_ENABLE=1
CFLAGS += -I$(TOP_DIR)/drivers/genx320/include
endif # OMV_GENX320_ENABLE

# Lepton driver sources
ifeq ($(OMV_LEPTON_SDK_ENABLE), 1)
DRIVER_SRC_C += $(addprefix lepton/src/, \
    crc16fast.c \
    LEPTON_AGC.c \
    LEPTON_I2C_Protocol.c \
    LEPTON_I2C_Service.c \
    LEPTON_OEM.c \
    LEPTON_RAD.c \
    LEPTON_SDK.c \
    LEPTON_SYS.c \
    LEPTON_VID.c \
)

CFLAGS += -I$(TOP_DIR)/drivers/lepton/include
endif # OMV_LEPTON_SDK_ENABLE

ifeq ($(MICROPY_PY_IMU), 1)
# LSM6DS3 sensor driver sources
DRIVER_SRC_C += $(addprefix lsm6ds3/src/, \
    lsm6ds3_reg.c \
    lsm6ds3tr_c_reg.c \
)
CFLAGS += -I$(TOP_DIR)/drivers/lsm6ds3/include

# LSM6DSM sensor driver sources
DRIVER_SRC_C += $(addprefix lsm6dsm/src/, \
    lsm6dsm_reg.c \
)
CFLAGS += -I$(TOP_DIR)/drivers/lsm6dsm/include/

# LSM6DSOX sensor driver sources
DRIVER_SRC_C += $(addprefix lsm6dsox/src/, \
    lsm6dsox_reg.c \
)
CFLAGS += -I$(TOP_DIR)/drivers/lsm6dsox/include
endif   # MICROPY_PY_IMU

ifeq ($(MICROPY_PY_FIR), 1)
# MLX90621 driver sources
DRIVER_SRC_C += $(addprefix mlx90621/src/, \
    MLX90621_API.c \
    MLX90621_I2C_Driver.c \
)
CFLAGS += -I$(TOP_DIR)/drivers/mlx90621/include

# MLX90640 driver sources
DRIVER_SRC_C += $(addprefix mlx90640/src/, \
    MLX90640_API.c \
    MLX90640_I2C_Driver.c \
)
CFLAGS += -I$(TOP_DIR)/drivers/mlx90640/include

# MLX90641 driver sources
DRIVER_SRC_C += $(addprefix mlx90641/src/, \
    MLX90641_API.c \
    MLX90641_I2C_Driver.c \
)
CFLAGS += -I$(TOP_DIR)/drivers/mlx90641/include
endif # MICROPY_PY_FIR

# PixArt sensor driver sources
DRIVER_SRC_C += $(addprefix pixart/src/, \
    pixspi.c \
)
CFLAGS += -I$(TOP_DIR)/drivers/pixart/include

# Image sensor drivers sources
DRIVER_SRC_C += $(addprefix sensors/, \
    boson.c \
    frogeye2020.c \
    gc2145.c \
    genx320.c \
    hm01b0.c \
    hm0360.c \
    lepton.c \
    mt9m114.c \
    mt9v0xx.c \
    ov2640.c \
    ov5640.c \
    ov7670.c \
    ov7690.c \
    ov7725.c \
    ov9650.c \
    pag7920.c \
    pag7936.c \
    ps5520.c \
    paj6100.c \
    softcsi.c \
    vd551g1.c \
)
CFLAGS += -I$(TOP_DIR)/drivers/sensors

# VL53L5CX driver sources
ifeq ($(OMV_TOF_VL53L5CX_ENABLE), 1)
DRIVER_SRC_C += $(addprefix vl53l5cx/src/, \
    platform.c \
    vl53l5cx_api.c \
    vl53l5cx_plugin_detection_thresholds.c \
    vl53l5cx_plugin_motion_indicator.c \
    vl53l5cx_plugin_xtalk.c \
)

CFLAGS += -DOMV_TOF_VL53L5CX_ENABLE=1
CFLAGS += -I$(TOP_DIR)/drivers/vl53l5cx/include
endif

# VL53L8CX driver sources
ifeq ($(OMV_TOF_VL53L8CX_ENABLE), 1)
DRIVER_SRC_C += $(addprefix vl53l8cx/src/, \
    platform.c \
    vl53l8cx_api.c \
    vl53l8cx_plugin_detection_thresholds.c \
    vl53l8cx_plugin_motion_indicator.c \
    vl53l8cx_plugin_xtalk.c \
)

CFLAGS += -DOMV_TOF_VL53L8CX_ENABLE=1
CFLAGS += -I$(TOP_DIR)/drivers/vl53l8cx/include/
endif

# WINC1500 Wi-Fi module sources
ifeq ($(MICROPY_PY_WINC1500), 1)
DRIVER_SRC_C += $(addprefix winc1500/src/, \
    flexible_flash.c \
    m2m_ate_mode.c \
    m2m_crypto.c \
    m2m_hif.c \
    m2m_ota.c \
    m2m_periph.c \
    m2m_ssl.c \
    m2m_wifi.c \
    nmasic.c \
    nm_bsp.c \
    nmbus.c \
    nm_bus_wrapper.c \
    nm_common.c \
    nmdrv.c \
    nmi2c.c \
    nmspi.c \
    nmuart.c \
    programmer.c \
    socket.c \
    spi_flash.c \
    winc.c \
)
CFLAGS += -I$(TOP_DIR)/drivers/winc1500/include
endif   # MICROPY_PY_WINC1500

OMV_FIRM_OBJ += $(addprefix $(BUILD)/drivers/, $(DRIVER_SRC_C:.c=.o))

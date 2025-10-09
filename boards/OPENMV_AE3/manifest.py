# OpenMV library
add_library("openmv-lib", "$(OMV_LIB_DIR)")

# Drivers
require("lsm6dsox")
freeze ("$(OMV_LIB_DIR)/", "ssd1351.py")
freeze ("$(OMV_LIB_DIR)/", "pca9674a.py")
freeze ("$(OMV_LIB_DIR)/", "vl53l1x.py")
freeze ("$(OMV_LIB_DIR)/", "machine.py")
freeze ("$(OMV_LIB_DIR)/", "display.py")

# Bluetooth
require("aioble")

# Networking
require("ssl")
require("ntptime")
require("webrepl")
freeze ("$(OMV_LIB_DIR)/", "rtsp.py")
freeze ("$(OMV_LIB_DIR)/", "mqtt.py")
freeze ("$(OMV_LIB_DIR)/", "requests.py")

# Utils
require("time")
require("logging")
require("collections-defaultdict")
require("types")
freeze ("$(OMV_LIB_DIR)/", "romfs.py")

# Libraries
require("ml", library="openmv-lib")
include("$(MPY_DIR)/extmod/asyncio")
freeze ("$(OMV_LIB_DIR)/", "openamp.py")

# Boot script.
freeze("$(OMV_LIB_DIR)/alif/$(MCU_CORE)", "_boot.py")

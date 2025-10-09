# OpenMV library
add_library("openmv-lib", "$(OMV_LIB_DIR)")

# Drivers
require("onewire")
require("dht")
require("neopixel")
freeze ("$(OMV_LIB_DIR)/", "modbus.py")
freeze ("$(OMV_LIB_DIR)/", "pid.py")
freeze ("$(OMV_LIB_DIR)/", "gt911.py")
freeze ("$(OMV_LIB_DIR)/", "st7701.py")
freeze ("$(OMV_LIB_DIR)/", "machine.py")
freeze ("$(OMV_LIB_DIR)/", "display.py")

# Bluetooth
require("aioble")

# Networking
require("ssl")
require("ntptime")
require("webrepl")
freeze ("$(OMV_LIB_DIR)/", "rpc.py")
freeze ("$(OMV_LIB_DIR)/", "rtsp.py")
freeze ("$(OMV_LIB_DIR)/", "mqtt.py")
freeze ("$(OMV_LIB_DIR)/", "requests.py")

# Utils
require("time")
require("senml")
require("logging")
freeze ("$(OMV_LIB_DIR)/", "mutex.py")

# Libraries
require("ml", library="openmv-lib")
include("$(MPY_DIR)/extmod/asyncio")

# Boot script
freeze ("$(OMV_LIB_DIR)/", "_boot.py")

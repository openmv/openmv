include("$(MPY_DIR)/extmod/asyncio")

# Drivers
require("lsm6dsox")
require("onewire")
require("ds18x20")
require("dht")
require("neopixel")
freeze ("$(OMV_LIB_DIR)/", "modbus.py")
freeze ("$(OMV_LIB_DIR)/", "pid.py")
freeze ("$(OMV_LIB_DIR)/", "vl53l1x.py")
freeze ("$(OMV_LIB_DIR)/", "machine.py")

# Networking
require("ntptime")
require("webrepl")
freeze ("$(OMV_LIB_DIR)/", "rpc.py")
freeze ("$(OMV_LIB_DIR)/", "rtsp.py")
freeze ("$(OMV_LIB_DIR)/", "mqtt.py")
freeze ("$(OMV_LIB_DIR)/", "urequests.py")

# Utils
require("time")
require("senml")
require("logging")
freeze ("$(OMV_LIB_DIR)/", "mutex.py")

# Bluetooth
require("aioble")
freeze ("$(OMV_LIB_DIR)/", "ble_advertising.py")

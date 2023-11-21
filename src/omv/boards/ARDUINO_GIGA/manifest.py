include("$(MPY_DIR)/extmod/uasyncio")

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

# Networking
require("ntptime")
require("webrepl")
freeze ("$(OMV_LIB_DIR)/", "rpc.py")
freeze ("$(OMV_LIB_DIR)/", "rtsp.py")
freeze ("$(OMV_LIB_DIR)/", "mjpeg.py")
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

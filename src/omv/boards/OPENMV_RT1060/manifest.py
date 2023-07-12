include("$(MPY_DIR)/extmod/uasyncio")

# Drivers
require("onewire")
require("ds18x20")
require("dht")
require("neopixel")
freeze ("$(OMV_LIB_DIR)/", "lora.py")
freeze ("$(OMV_LIB_DIR)/", "modbus.py")
freeze ("$(OMV_LIB_DIR)/", "pid.py")

# Networking
require("ntptime")
require("webrepl")
freeze ("$(OMV_LIB_DIR)/", "rpc.py")
freeze ("$(OMV_LIB_DIR)/", "rtsp.py")
freeze ("$(OMV_LIB_DIR)/", "mqtt.py")
freeze ("$(MPY_DIR)/lib/micropython-lib/python-ecosys/urequests", "urequests.py")

# Utils
require("time")
require("senml")
require("logging")
freeze ("$(OMV_LIB_DIR)/", "mutex.py")

# Bluetooth
require("aioble")
freeze ("$(OMV_LIB_DIR)/", "ble_advertising.py")

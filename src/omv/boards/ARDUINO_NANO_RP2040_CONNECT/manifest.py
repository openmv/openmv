include("$(MPY_DIR)/extmod/asyncio")
freeze ("$(PORT_DIR)/modules")

# Drivers
require("lsm6dsox")
require("espflash")
require("onewire")
require("ds18x20")
require("dht")
require("neopixel")
freeze ("$(OMV_LIB_DIR)/", "machine.py")

# Networking
require("ntptime")
require("webrepl")
freeze ("$(OMV_LIB_DIR)/", "urequests.py")

# Utils
require("time")
require("senml")
require("logging")

# Bluetooth
require("aioble")
freeze ("$(OMV_LIB_DIR)/", "ble_advertising.py")

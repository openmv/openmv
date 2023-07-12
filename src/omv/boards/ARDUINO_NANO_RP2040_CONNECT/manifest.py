include("$(MPY_DIR)/extmod/uasyncio")
freeze ("$(PORT_DIR)/modules")

# Drivers
require("lsm6dsox")
require("espflash")
require("onewire")
require("ds18x20")
require("dht")
require("neopixel")

# Networking
require("ntptime")
require("webrepl")
freeze ("$(MPY_DIR)/lib/micropython-lib/python-ecosys/urequests", "urequests.py")

# Utils
require("time")
require("senml")
require("logging")

# Bluetooth
require("aioble")
freeze ("$(OMV_LIB_DIR)/", "ble_advertising.py")

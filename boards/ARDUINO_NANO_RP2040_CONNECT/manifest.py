# Drivers
require("lsm6dsox")
require("espflash")
require("onewire")
require("ds18x20")
require("dht")
require("neopixel")
freeze ("$(OMV_LIB_DIR)/", "machine.py")

# Bluetooth
require("aioble")

# Networking
require("ssl")
require("ntptime")
require("webrepl")
freeze ("$(OMV_LIB_DIR)/", "mqtt.py")
freeze ("$(OMV_LIB_DIR)/", "requests.py")

# Utils
require("time")
require("senml")
require("logging")

# Libraries
include("$(MPY_DIR)/extmod/asyncio")
freeze ("$(PORT_DIR)/modules", "rp2.py")

# Boot script
freeze ("$(OMV_LIB_DIR)/", "_boot.py")

include("$(MPY_DIR)/extmod/asyncio")

# Networking
require("ntptime")
require("webrepl")

# Drivers
require("onewire")
require("ds18x20")
require("dht")
require("neopixel")
freeze ("$(OMV_LIB_DIR)/", "machine.py")

# Utils
require("time")
require("logging")

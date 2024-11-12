include("$(MPY_DIR)/extmod/asyncio")
freeze ("$(PORT_DIR)/modules", "rp2.py")

# Filesystem
freeze ("$(OMV_LIB_DIR)/", "_boot.py")

# Drivers
require("lsm6dsox")
require("espflash")
require("onewire")
require("ds18x20")
require("dht")
require("neopixel")
freeze ("$(OMV_LIB_DIR)/", "machine.py")

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

# Bluetooth
require("aioble")

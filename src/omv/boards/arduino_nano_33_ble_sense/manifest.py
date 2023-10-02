#include("$(MPY_DIR)/extmod/uasyncio")

# Drivers
require("hts221")
require("lps22h")
require("bmm150")
require("bmi270")
require("hs3003")
require("onewire")
require("ds18x20")
require("dht")
require("neopixel")
freeze("$(BOARD_DIR)/modules/", "imu.py")
freeze("$(OMV_LIB_DIR)/apds9960/apds9960")

# Utils
require("time")
#require("logging")

freeze("$(PORT_DIR)/modules/scripts", "_mkfs.py")

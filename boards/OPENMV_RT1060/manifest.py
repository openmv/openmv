# OpenMV library
add_library("openmv-lib", "$(OMV_LIB_DIR)")

# Drivers
require("onewire")
require("ds18x20")
require("dht")
require("neopixel")
freeze ("$(OMV_LIB_DIR)/", "modbus.py")
freeze ("$(OMV_LIB_DIR)/", "pid.py")
freeze ("$(OMV_LIB_DIR)/", "bno055.py")
freeze ("$(OMV_LIB_DIR)/", "ssd1306.py")
freeze ("$(OMV_LIB_DIR)/", "ssd1351.py")
freeze ("$(OMV_LIB_DIR)/", "pca9674a.py")
freeze ("$(OMV_LIB_DIR)/", "tb6612.py")
freeze ("$(OMV_LIB_DIR)/", "vl53l1x.py")
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
require("protocol", library="openmv-lib")
include("$(MPY_DIR)/extmod/asyncio")

# Boot script
# freeze ("$(OMV_LIB_DIR)/", "_boot.py")

freeze("$(OMV_LIB_DIR)/", "app_controller.py")
freeze("$(OMV_LIB_DIR)/", "config.py")
freeze("$(OMV_LIB_DIR)/", "db_store.py")
freeze("$(OMV_LIB_DIR)/", "detect.py")
freeze("$(OMV_LIB_DIR)/", "enc.py")
freeze("$(OMV_LIB_DIR)/", "enc_priv.py")
freeze("$(OMV_LIB_DIR)/", "enc_pub.py")
freeze("$(OMV_LIB_DIR)/", "fs_utils.py")
freeze("$(OMV_LIB_DIR)/", "gps_driver.py")
freeze("$(OMV_LIB_DIR)/", "internet_driver.py")
freeze("$(OMV_LIB_DIR)/", "logger.py")
freeze("$(OMV_LIB_DIR)/", "message_codec.py")
freeze("$(OMV_LIB_DIR)/", "boot.py")
freeze("$(OMV_LIB_DIR)/", "_sx126x.py")
freeze("$(OMV_LIB_DIR)/", "sx126x.py")
freeze("$(OMV_LIB_DIR)/", "sx1262.py")
freeze("$(OMV_LIB_DIR)/", "utils.py")
freeze("$(OMV_LIB_DIR)/", "wifi_comm.py")
freeze("$(OMV_LIB_DIR)/", "power_mgmt.py")
freeze("$(OMV_LIB_DIR)/", "clock_utils.py")
freeze("$(OMV_LIB_DIR)/", "config_store.py")
freeze("$(OMV_LIB_DIR)/", "watchdog.py")

# rsa package
# freeze("$(OMV_LIB_DIR)/rsa")
freeze("$(OMV_LIB_DIR)/", "rsa/__init__.py")
freeze("$(OMV_LIB_DIR)/", "rsa/_compat.py")
freeze("$(OMV_LIB_DIR)/", "rsa/asn1.py")
freeze("$(OMV_LIB_DIR)/", "rsa/common.py")
freeze("$(OMV_LIB_DIR)/", "rsa/core.py")
freeze("$(OMV_LIB_DIR)/", "rsa/key.py")
freeze("$(OMV_LIB_DIR)/", "rsa/machine_size.py")
freeze("$(OMV_LIB_DIR)/", "rsa/pem.py")
freeze("$(OMV_LIB_DIR)/", "rsa/pkcs1.py")
freeze("$(OMV_LIB_DIR)/", "rsa/prime.py")
freeze("$(OMV_LIB_DIR)/", "rsa/randnum.py")
freeze("$(OMV_LIB_DIR)/", "rsa/transform.py")


# WINC Firmware Update Script.
#
# This script updates the ATWINC1500 WiFi module firmware.
# Copy the firmware image to uSD card before running this script.
# NOTE: Older fimware versions are no longer supported by the host driver.
# NOTE: The latest firmware (19.6.1) only works on ATWINC1500-MR210PB.

import network

# Init wlan module in Download mode.
wlan = network.WINC(mode=network.WINC.MODE_FIRMWARE)

# For ATWINC1500-MR210PB only.
wlan.fw_update("/winc_19_6_1.bin")

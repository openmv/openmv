# WINC Firmware Update Script.
#
# This script updates the ATWINC1500 WiFi module firmware.
# Copy the firmware image to uSD card before running this script.
# NOTE: Firmware version 19.5.2 does NOT support ATWINC1500-MR210PA.

import network

# Init wlan module in Download mode.
wlan = network.WINC(mode=network.WINC.MODE_FIRMWARE)

# Start the firmware update process.
# For ATWINC1500-MR210PA/B
#wlan.fw_update("/winc_19_4_4.bin")

# For ATWINC1500-MR210PB only.
wlan.fw_update("/winc_19_5_2.bin")

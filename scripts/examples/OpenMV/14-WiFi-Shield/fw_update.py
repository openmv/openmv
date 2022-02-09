# WINC Firmware Update Script.
#
# This script updates the ATWINC1500 WiFi module firmware.
# 1) Copy the firmware image to a FAT32/exFAT SD card.
# 2) Safe remove/eject the SD card (or umount on Linux).
# 3) Reset the camera from the IDE.
# 4) Run this script to update the firmware.
#
# NOTE: Older fimware versions are no longer supported by the host driver.
# NOTE: The latest firmware (19.7.6) only works on ATWINC1500-MR210PB.
# NOTE: Firmware is at <openmv-ide-install-dir>/share/qtcreator/firmware/WINC1500/winc_19_7_6.bin

import network

# Init wlan module in Download mode.
wlan = network.WINC(mode=network.WINC.MODE_FIRMWARE)

# For ATWINC1500-MR210PB only.
wlan.fw_update("/winc_19_7_6.bin")

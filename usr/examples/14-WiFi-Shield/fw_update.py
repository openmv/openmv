# WINC Firmware Update Script
#
# To start have a successful firmware update create a "firmware" folder on the
# uSD card and but a bin file in it. The firmware update code will load that
# new firmware onto the WINC module.

import network

# Init wlan module in Download mode.
wlan = network.WINC(True)
#print("Firmware version:", wlan.fw_version())

# Start the firmware update process.
wlan.fw_update()
#print("Firmware version:", wlan.fw_version())

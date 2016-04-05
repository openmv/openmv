'''
    Firmware update examples
    Note: copy the WINC1500/firmware folder to uSD
'''
import time, network

# Init wlan module in Download mode
wlan = network.WINC(True)
#print("Firmware version:", wlan.fw_version())

# Start the firmware update process.
wlan.fw_update()
#print("Firmware version:", wlan.fw_version())
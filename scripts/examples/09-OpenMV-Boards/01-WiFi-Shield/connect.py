# Connect Example
#
# This example shows how to connect your OpenMV Cam with a WiFi shield to the net.

import network

SSID = ""  # Network SSID
KEY = ""  # Network key

# Init wlan module and connect to network
print("Trying to connect... (may take a while)...")

wlan = network.WINC()
wlan.connect(SSID, key=KEY, security=wlan.WPA_PSK)

# We should have a valid IP now via DHCP
print(wlan.ifconfig())

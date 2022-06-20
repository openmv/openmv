# Connect Example
#
# This example shows how to connect your OpenMV Cam with a WiFi shield to the net.

import network

SSID='' # Network SSID
KEY=''  # Network key

# Init wlan module and connect to network
print("Trying to connect. Note this may take a while...")

wlan = network.WLAN(network.STA_IF)
wlan.deinit()
wlan.active(True)
wlan.connect(SSID, KEY, timeout=30000)

# We should have a valid IP now via DHCP
print("WiFi Connected ", wlan.ifconfig())

# DNS Example
#
# This example shows how to get the IP address for websites via DNS.

import network, usocket

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
print(usocket.getaddrinfo("www.google.com", 80)[0][4])

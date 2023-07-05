# DNS Example
#
# This example shows how to get the IP address for websites via DNS.

import network
import usocket

# AP info
SSID='' # Network SSID
KEY=''  # Network key

# Init wlan module and connect to network
print("Trying to connect... (may take a while)...")

wlan = network.WINC()
wlan.connect(SSID, key=KEY, security=wlan.WPA_PSK)

# We should have a valid IP now via DHCP
print(wlan.ifconfig())
print(usocket.getaddrinfo("www.google.com", 80)[0][4])

# DNS Example
#
# This example shows how to get the IP address for websites via DNS.

import network, usocket

SSID='' # Network SSID
KEY=''  # Network key

# Init wlan module and connect to network
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(SSID, KEY)

while not wlan.isconnected():
    print("Trying to connect to \"{:s}\"...".format(SSID))
    time.sleep_ms(1000)

# We should have a valid IP now via DHCP
print("WiFi Connected ", wlan.ifconfig())
print(usocket.getaddrinfo("www.google.com", 80)[0][4])

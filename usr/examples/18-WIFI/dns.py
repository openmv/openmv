# Simple DNS example 
import time, pyb, network, usocket

# AP info
SSID='' # Network SSID
KEY=''  # Network key

# Init wlan module and connect to network
wlan = network.WINC()
wlan.connect(SSID, key=KEY, security=wlan.WPA_PSK)

# We should have a valid IP now via DHCP
print(wlan.ifconfig())
print(usocket.getaddrinfo("www.google.com", 80)[0][4])

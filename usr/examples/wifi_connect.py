# Simple WiFi scan example
import time, pyb, network

SSID='' # Network SSID
KEY=''  # Network key

# Init wlan module and connect to network
wlan = network.WINC()
wlan.connect(SSID, key=KEY, security=wlan.WPA_PSK)

# We should have a valid IP now via DHCP
print(wlan.ifconfig())

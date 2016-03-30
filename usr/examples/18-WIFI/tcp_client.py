# Simple NTP client
import time, pyb, network, usocket

# AP info
SSID='' # Network SSID
KEY=''  # Network key

# Init wlan module and connect to network
wlan = network.WINC()
wlan.connect(SSID, key=KEY, security=wlan.WPA_PSK)

# We should have a valid IP now via DHCP
print(wlan.ifconfig())

# Get addr info via DNS
addr = usocket.getaddrinfo("www.google.com", 80)[0][4]

# Create a new socket and connect to addr
client = usocket.socket(usocket.AF_INET, usocket.SOCK_STREAM)
client.connect(addr)

# Set timeout to 1s
client.settimeout(1.0)

# Send HTTP request and recv response
client.send("GET / HTTP/1.0\r\n\r\n")
print(client.recv(1024))

# Close socket
client.close()

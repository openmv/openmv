# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Simple HTTPS client example.

import network
import socket
import ssl
import time

# AP info
SSID = ""  # Network SSID
KEY = ""  # Network key

PORT = 443
HOST = "www.google.com"

# Init wlan module and connect to network
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(SSID, KEY)

while not wlan.isconnected():
    print('Trying to connect to "{:s}"...'.format(SSID))
    time.sleep_ms(1000)

# We should have a valid IP now via DHCP
print("WiFi Connected ", wlan.ifconfig())

# Get addr info via DNS
addr = socket.getaddrinfo(HOST, PORT)[0][4]
print(addr)

# Create a new socket and connect to addr
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

client.connect(addr)

# Set timeout
client.settimeout(3.0)

client = ssl.wrap_socket(client, server_hostname=HOST)

# Send HTTP request and recv response
request = "GET / HTTP/1.1\r\n"
request += "HOST: %s\r\n"
request += "User-Agent: Mozilla/5.0\r\n"
request += "Connection: keep-alive\r\n\r\n"
# Add more headers if needed.
client.write(request % (HOST) + "\r\n")

response = client.read(1024)
for l in response.split(b"\r\n"):
    print(l.decode())

# Close socket
client.close()

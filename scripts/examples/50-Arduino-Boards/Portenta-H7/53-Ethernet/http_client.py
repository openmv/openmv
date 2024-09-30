# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Ethernet LAN HTTP client example.
import network
import socket

PORT = 80
HOST = "www.google.com"

lan = network.LAN()
lan.active(True)
lan.ifconfig("dhcp")

# We should have a valid IP now via DHCP
print(lan.ifconfig())

# Get addr info via DNS
addr = socket.getaddrinfo(HOST, PORT)[0][4]
print(addr)

# Create a new socket and connect to addr
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(addr)

# Set timeout
client.settimeout(3.0)

# Send HTTP request and recv response
client.send("GET / HTTP/1.1\r\nHost: %s\r\n\r\n" % (HOST))
print(client.recv(1024))

# Close socket
client.close()

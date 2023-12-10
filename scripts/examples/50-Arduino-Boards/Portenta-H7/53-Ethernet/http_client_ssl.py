# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Ethernet LAN HTTP client example.
import network
import usocket
import ussl

PORT = 443
HOST = "www.google.com"

lan = network.LAN()
lan.active(True)
lan.ifconfig("dhcp")

# We should have a valid IP now via DHCP
print(lan.ifconfig())

# Get addr info via DNS
addr = usocket.getaddrinfo(HOST, PORT)[0][4]
print(addr)

# Create a new socket and connect to addr
client = usocket.socket(usocket.AF_INET, usocket.SOCK_STREAM)
client.connect(addr)

# Set timeout
client.settimeout(3.0)
client = ussl.wrap_socket(client, server_hostname=HOST)

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

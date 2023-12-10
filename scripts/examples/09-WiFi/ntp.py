# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# NTP Example
#
# This example shows how to get the current time using NTP.

import network
import socket
import struct
import time

SSID = ""  # Network SSID
KEY = ""  # Network key

TIMESTAMP = 2208988800

if time.gmtime(0)[0] == 2000:
    TIMESTAMP += 946684800

# Init wlan module and connect to network
print("Trying to connect... (This may take a while)...")
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(SSID, KEY)

while not wlan.isconnected():
    print('Trying to connect to "{:s}"...'.format(SSID))
    time.sleep_ms(1000)

# We should have a valid IP now via DHCP
print("WiFi Connected ", wlan.ifconfig())

# Create new socket
client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Get addr info via DNS
addr = socket.getaddrinfo("pool.ntp.org", 123)[0][4]

# Send query
client.sendto("\x1b" + 47 * "\0", addr)
data, address = client.recvfrom(1024)

# Print time
t = struct.unpack(">IIIIIIIIIIII", data)[10] - TIMESTAMP
print("Year:%d Month:%d Day:%d Time: %d:%d:%d" % (time.localtime(t)[0:6]))

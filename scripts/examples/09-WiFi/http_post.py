# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Post files with HTTP/Post requests module example

import network
import requests

# AP info
SSID = ""  # Network SSID
KEY = ""  # Network key
URL = 'http://192.168.1.102:8080/upload'

# Init wlan module and connect to network
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(SSID, KEY)

while not wlan.isconnected():
    print('Trying to connect to "{:s}"...'.format(SSID))
    time.sleep_ms(1000)

# We should have a valid IP now via DHCP
print("WiFi Connected ", wlan.ifconfig())

headers = {
    'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10.12; rv:55.0) Gecko/20100101 Firefox/55.0',
    # Add more headers if needed
}

# Send some files
files = {
    'image1': ('example1.jpg', open('example1.jpg', 'rb')),
    'image2': ('example2.jpg', open('example2.jpg', 'rb')),
}

r = requests.post(URL, files=files, headers=headers, auth=('admin', 'testadmin'))
# Or send JSON data
# r = requests.post(URL, json={'some': 'data'}, headers=headers, auth=('admin', 'testadmin'))

print(r.status_code, r.reason)
print(r.headers, r.content)

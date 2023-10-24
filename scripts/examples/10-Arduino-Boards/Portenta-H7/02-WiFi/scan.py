# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Scan Example
#
# This example shows how to scan for networks with the WiFi shield.

import time
import network

wlan = network.WLAN(network.STA_IF)
wlan.active(True)

print("Scanning...")
while True:
    scan_result = wlan.scan()
    for ap in scan_result:
        print(
            "SSID: %s BSSID: %s Channel: %d RSSI: %d Auth: %d"
            % (ap[0], ":".join(["%X" % i for i in ap[1]]), ap[2], ap[3], ap[4])
        )
    print()
    time.sleep_ms(1000)

# Scan Example
#
# This example shows how to scan for networks with the WiFi shield.

import time, network

wlan = network.WLAN(network.STA_IF)
wlan.deinit()
wlan.active(True)

while (True):
    scan_result = wlan.scan()
    for ap in scan_result:
        print(ap)
    print()
    time.sleep_ms(1000)

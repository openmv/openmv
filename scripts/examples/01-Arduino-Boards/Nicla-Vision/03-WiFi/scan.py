# Scan Example
#
# This example shows how to scan for networks with the WiFi shield.

import time, network

wlan = network.WLAN(network.STA_IF)
wlan.deinit()
wlan.active(True)

print("Scanning...")
while (True):
    scan_result = wlan.scan()
    for ap in scan_result:
        print("SSID: %s BSSID: %s Channel: %d RSSI: %d Auth: %d"
                %(ap[0], ":".join(["%X"%i for i in ap[1]]), ap[2], ap[3], ap[4]))
    print()
    time.sleep_ms(1000)

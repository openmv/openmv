# Scan Example
#
# This example shows how to scan for networks with the WiFi shield.

import time
import network

wlan = network.WINC()
print("\nFirmware version:", wlan.fw_version())

while (True):
    scan_result = wlan.scan()
    for ap in scan_result:
        print("Channel:%d RSSI:%d Auth:%d BSSID:%s SSID:%s"%(ap))
    print()
    time.sleep_ms(1000)

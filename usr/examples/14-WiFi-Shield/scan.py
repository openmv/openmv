# Simple WiFi scan example
import time, network

wlan = network.WINC()
print("\nFirmware version:", wlan.fw_version())

while (True):
    scan_result = wlan.scan()
    for ap in scan_result:
        print("Channel:%d RSSI:%d Auth:%d BSSID:%s SSID:%s"%(ap))
    print()
    time.sleep(1000)
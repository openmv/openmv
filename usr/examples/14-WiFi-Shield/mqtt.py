# MQTT Example.
# This example shows how to use the MQTT library.
#
# 1) Copy the mqtt.py library to OpenMV storage.
# 2) Install the mosquitto client on PC and run the following command:
#    mosquitto_sub -h test.mosquitto.org -t "openmv/test" -v
#
import time, network
from mqtt import MQTTClient

SSID='mux' # Network SSID
KEY='j806fVnT7tObdCYE'  # Network key

# Init wlan module and connect to network
print("Trying to connect... (may take a while)...")

wlan = network.WINC()
wlan.connect(SSID, key=KEY, security=wlan.WPA_PSK)

# We should have a valid IP now via DHCP
print(wlan.ifconfig())

client = MQTTClient("openmv", "test.mosquitto.org", port=1883)
client.connect()

while (True):
    client.publish("openmv/test", "Hello World!")
    time.sleep(1000)

# MQTT Example.
# This example shows how to use the MQTT library to subscribe to a topic.
#
# 1) Copy the mqtt.py library to OpenMV storage.
# 2) Run this script on the OpenMV camera.
# 3) Install the mosquitto client on PC and run the following command:
#    mosquitto_pub -t "openmv/test" -m "Hello World!" -h test.mosquitto.org -p 1883
#
# NOTE: If the mosquitto broker is unreachable, try another broker (For example: broker.hivemq.com)
import time, network
from mqtt import MQTTClient

SSID='' # Network SSID
KEY=''  # Network key

# Init wlan module and connect to network
print("Trying to connect... (may take a while)...")

wlan = network.WINC()
wlan.connect(SSID, key=KEY, security=wlan.WPA_PSK)

# We should have a valid IP now via DHCP
print(wlan.ifconfig())

client = MQTTClient("openmv", "test.mosquitto.org", port=1883)
client.connect()

def callback(topic, msg):
    print(topic, msg)

# must set callback first
client.set_callback(callback)
client.subscribe("openmv/test")

while (True):
    client.check_msg() # poll for messages.
    time.sleep_ms(1000)

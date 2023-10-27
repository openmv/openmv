# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# MQTT Example.
# This example shows how to use the MQTT library to subscribe to a topic.
#
# 1) Copy the mqtt.py library to OpenMV storage.
# 2) Run this script on the OpenMV camera.
# 3) Install the mosquitto client on PC and run the following command:
#    mosquitto_pub -t "openmv/test" -m "Hello World!" -h test.mosquitto.org -p 1883
#
# NOTE: If the mosquitto broker is unreachable, try another broker (For example: broker.hivemq.com)

import time
import network
from mqtt import MQTTClient

SSID = ""  # Network SSID
KEY = ""  # Network key

# Init wlan module and connect to network
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(SSID, KEY)

while not wlan.isconnected():
    print('Trying to connect to "{:s}"...'.format(SSID))
    time.sleep_ms(1000)

# We should have a valid IP now via DHCP
print("WiFi Connected ", wlan.ifconfig())

client = MQTTClient("openmv", "test.mosquitto.org", port=1883)
client.connect()


def callback(topic, msg):
    print(topic, msg)


# must set callback first
client.set_callback(callback)
client.subscribe("openmv/test")

while True:
    client.check_msg()  # poll for messages.
    time.sleep_ms(1000)

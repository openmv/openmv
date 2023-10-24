# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
from time import sleep_ms
from machine import Pin, I2C
from apds9960 import uAPDS9960 as APDS9960

bus = I2C(1, sda=Pin(13), scl=Pin(14))
apds = APDS9960(bus)

print("Light Sensor Test")
print("=================")
apds.enableLightSensor()

while True:
    sleep_ms(250)
    val = apds.readAmbientLight()
    print("AmbientLight={}".format(val))

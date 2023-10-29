# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
import time
import lps22h
from machine import Pin, I2C

bus = I2C(1, scl=Pin(15), sda=Pin(14))
lps = lps22h.LPS22H(bus)

while True:
    pressure = lps.pressure()
    temperature = lps.temperature()
    print("Pressure: %.2f hPa Temperature: %.2f C" % (pressure, temperature))
    time.sleep_ms(100)

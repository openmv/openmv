# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Relative humidity and temperature sensor example.
# Note Arduino Nano BLE 33 Sense Rev2 uses the HS3003.

import time
from hts221 import HTS221
from machine import Pin, I2C

bus = I2C(1, scl=Pin(15), sda=Pin(14))
try:
    hts = HTS221(bus)
except OSError:
    from hs3003 import HS3003

    hts = HS3003(bus)

while True:
    rH = hts.humidity()
    temp = hts.temperature()
    print("rH: %.2f%% T: %.2fC" % (rH, temp))
    time.sleep_ms(100)

# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
from machine import I2C
from vl53l1x import VL53L1X
import time

i2c = I2C(2)
distance = VL53L1X(i2c)

while True:
    print("range: mm ", distance.read())
    time.sleep_ms(50)

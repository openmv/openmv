# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Servo Shield Example.
#
# This example demonstrates the servo shield. Please follow these steps:
#
#   1. Connect a servo to any PWM output.
#   2. Connect a 3.7v battery (or 5V source) to VIN and GND.
#   3. Copy pca9685.py and servo.py to OpenMV and reset it.
#   4. Connect and run this script in the IDE.

import time
from servo import Servos
from machine import SoftI2C, Pin

i2c = SoftI2C(sda=Pin("P5"), scl=Pin("P4"))
servo = Servos(i2c, address=0x40, freq=50, min_us=650, max_us=2800, degrees=180)

while True:
    for i in range(0, 8):
        servo.position(i, 0)
    time.sleep_ms(500)
    for i in range(0, 8):
        servo.position(i, 180)
    time.sleep_ms(500)

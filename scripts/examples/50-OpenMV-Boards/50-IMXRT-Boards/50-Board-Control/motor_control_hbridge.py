# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Motor Control Example
#
# This example shows off how to control an hbridge motor
# controller with "P0"/"P7" controlling a left-motor hbridge
# and "P2"/"P8" controlling a right-motor hbridge.

import time
from machine import Pin

p0 = Pin("P0", Pin.OUT)
p2 = Pin("P2", Pin.OUT)
p7 = Pin("P7", Pin.OUT)
p8 = Pin("P8", Pin.OUT)

p0.value(0)
p2.value(0)
p7.value(0)
p8.value(0)

while True:
    p0.value(not p0.value())
    time.sleep_ms(2000)
    p7.value(not p7.value())
    time.sleep_ms(2000)
    p0.value(not p0.value())
    time.sleep_ms(2000)
    p7.value(not p7.value())
    time.sleep_ms(2000)

    p2.value(not p2.value())
    time.sleep_ms(2000)
    p8.value(not p8.value())
    time.sleep_ms(2000)
    p2.value(not p2.value())
    time.sleep_ms(2000)
    p8.value(not p8.value())
    time.sleep_ms(2000)

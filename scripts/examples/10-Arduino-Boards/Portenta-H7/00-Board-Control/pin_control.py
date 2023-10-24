# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Pin Control Example
#
# This example shows how to use the I/O pins in GPIO mode on your OpenMV Cam.

from pyb import Pin

# Connect a switch to pin 0 that will pull it low when the switch is closed.
# Pin 1 will then light up.
pin0 = Pin("D0", Pin.IN, Pin.PULL_UP)
pin1 = Pin("D1", Pin.OUT_PP, Pin.PULL_NONE)

while True:
    pin1.value(not pin0.value())

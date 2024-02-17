# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Power Good Pin
#
# The power good pin is an internal Pin on your OpenMV Cam
# which goes low when the board external power and high when
# it does not. You can use this pin to tell if you are running
# an external battery.
#
# Run this example with a battery attached and then disconnect
# your OpenMV Cam from the computer to see the red light turn on.
# When you reconnect the camera to power it will turn off.

import machine

pg = machine.Pin("PG", machine.Pin.IN)
r = machine.LED("LED_RED")

while True:
    r.value(pg.value())

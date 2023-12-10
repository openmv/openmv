# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# ExtInt Wake-Up from Stop Mode Example
# This example demonstrates using external interrupts to wake up from low-power mode.

import time
import pyb
import machine
from pyb import Pin, ExtInt


def callback(line):
    pass


led = pyb.LED(3)
pin = Pin("PG12", Pin.IN, Pin.PULL_UP)
ext = ExtInt(pin, ExtInt.IRQ_FALLING, Pin.PULL_UP, callback)

# Enter Stop Mode. Note the IDE will disconnect.
machine.sleep()

while True:
    led.on()
    time.sleep_ms(100)
    led.off()
    time.sleep_ms(100)

# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Pin IRQ Example
#
# This example shows how to use a GPIO interrupt to react to a pin edge.
# Pressing the user switch (SW) toggles the blue LED via the IRQ handler.

import time
from machine import Pin, LED

led = LED("LED_BLUE")
sw = Pin("SW", Pin.IN)


def handler(pin):
    led.toggle()


# The user switch is active-low, so trigger on the falling edge.
sw.irq(handler=handler, trigger=Pin.IRQ_FALLING)

while True:
    time.sleep_ms(100)

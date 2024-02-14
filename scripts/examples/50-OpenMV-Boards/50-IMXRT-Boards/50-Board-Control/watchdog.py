# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Watch Dog Example
#
# This example shows how to use the Watch Dog Timer.
#
# When you press the SW button software will stop feeding
# the watchdog timer causing a system reset.

import time
from machine import WDT, Pin, LED

sw = Pin("SW", Pin.IN)
b = LED("LED_BLUE")
wdt = WDT(timeout=2000)

while True:
    b.on()
    time.sleep_ms(150)
    b.off()
    time.sleep_ms(100)
    b.on()
    time.sleep_ms(150)
    b.off()
    time.sleep_ms(600)

    if sw.value():
        wdt.feed()

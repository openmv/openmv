# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Blinky example

import time
from machine import LED

led = LED("LED_BLUE")

while True:
    led.on()
    time.sleep_ms(500)
    led.off()
    time.sleep_ms(500)

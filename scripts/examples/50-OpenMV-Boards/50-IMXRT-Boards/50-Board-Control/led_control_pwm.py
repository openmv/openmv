# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# LED Control Example
#
# This example shows how to control your OpenMV Cam's built-in LEDs.

import time
from machine import PWM

r = PWM("LED_RED", freq=200, duty_u16=65535)
b = PWM("LED_BLUE", freq=200, duty_u16=65535)
g = PWM("LED_GREEN", freq=200, duty_u16=65535)


while True:
    for i in range(0, 65536, 256):
        r.duty_u16(65535 - i)
        time.sleep_ms(10)
    r.duty_u16(65535)

    for i in range(0, 65536, 256):
        g.duty_u16(65535 - i)
        time.sleep_ms(10)
    g.duty_u16(65535)

    for i in range(0, 65536, 256):
        b.duty_u16(65535 - i)
        time.sleep_ms(10)
    b.duty_u16(65535)

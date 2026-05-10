# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# PWM Control Example
#
# This example shows how to do PWM with your OpenMV Cam.

import time
from machine import PWM

# P0 and P1 share the same UTIMER channel so they need
# to have the same frequency.
p0 = PWM("P0", freq=100, duty_u16=32768)
p1 = PWM("P1", freq=100, duty_u16=32768)

# P2 and P3 share the same UTIMER channel so they need
# to have the same frequency.
p2 = PWM("P2", freq=100, duty_u16=32768)
p3 = PWM("P3", freq=100, duty_u16=32768)

while True:
    for i in range(0, 65536, 256):
        p0.duty_u16(65535 - i)
        time.sleep_ms(10)
    p0.duty_u16(32768)

    for i in range(0, 65536, 256):
        p1.duty_u16(65535 - i)
        time.sleep_ms(10)
    p1.duty_u16(32768)

    for i in range(0, 65536, 256):
        p2.duty_u16(65535 - i)
        time.sleep_ms(10)
    p2.duty_u16(32768)

    for i in range(0, 65536, 256):
        p3.duty_u16(65535 - i)
        time.sleep_ms(10)
    p3.duty_u16(32768)

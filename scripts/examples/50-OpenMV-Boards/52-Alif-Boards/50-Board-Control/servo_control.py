# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Servo Control Example
#
# This example shows how to use your OpenMV Cam to control servos.
#
# Servos need 50 Hz PWM with a 1000us to 2000us pulse width.

import time
from machine import PWM

# P0 and P1 share the same UTIMER channel so they need
# to have the same frequency.
p0 = PWM("P0", freq=50, duty_ns=(2000 * 1000))
p1 = PWM("P1", freq=50, duty_ns=(2000 * 1000))

# P2 and P3 share the same UTIMER channel so they need
# to have the same frequency.
p2 = PWM("P2", freq=50, duty_ns=(2000 * 1000))
p3 = PWM("P3", freq=50, duty_ns=(2000 * 1000))


while True:
    for i in range(1000, 2000, 100):
        p0.duty_ns(i * 1000)
        p1.duty_ns(i * 1000)
        p2.duty_ns(i * 1000)
        p3.duty_ns(i * 1000)
        time.sleep_ms(1000)

    for i in range(2000, 1000, -100):
        p0.duty_ns(i * 1000)
        p1.duty_ns(i * 1000)
        p2.duty_ns(i * 1000)
        p3.duty_ns(i * 1000)
        time.sleep_ms(1000)

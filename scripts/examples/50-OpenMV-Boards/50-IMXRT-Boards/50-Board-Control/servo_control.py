# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Servo Control Example
#
# This example shows how to use your OpenMV Cam to control servos.
#
# Servos need 50 Hz PWM with a 1000us to 2000us pulse width.

import time
from machine import PWM

# P7 and P8 may share the same PWM module they need
# to have the same frequency.
p7 = PWM("P7", freq=50, duty_ns=(2000 * 1000))
p8 = PWM("P8", freq=50, duty_ns=(2000 * 1000))

# P9 and P10 may share the same PWM module they need
# to have the same frequency.
p9 = PWM("P9", freq=50, duty_ns=(2000 * 1000))
p10 = PWM("P10", freq=50, duty_ns=(2000 * 1000))


while True:
    for i in range(1000, 2000, 100):
        p7.duty_ns(i * 1000)
        p8.duty_ns(i * 1000)
        p9.duty_ns(i * 1000)
        p10.duty_ns(i * 1000)
        time.sleep_ms(1000)

    for i in range(2000, 1000, -100):
        p7.duty_ns(i * 1000)
        p8.duty_ns(i * 1000)
        p9.duty_ns(i * 1000)
        p10.duty_ns(i * 1000)
        time.sleep_ms(1000)

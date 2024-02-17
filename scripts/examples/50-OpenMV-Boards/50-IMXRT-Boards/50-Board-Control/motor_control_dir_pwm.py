# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Motor Control Example
#
# This example shows how to control a DIR and PWM motor controller
# with your OpenMV Cam. "P0"/"P2" are DIR pins for left/right motors
# and "P7"/"P8" are the PWM (speed) for the left/right motors.

import time
from machine import Pin, PWM

p0 = Pin("P0", Pin.OUT)
p2 = Pin("P2", Pin.OUT)
p7 = PWM("P7", freq=10000, duty_u16=0)
p8 = PWM("P8", freq=10000, duty_u16=0)

p0.value(0)
p2.value(0)

while True:
    for i in range(0, 65535, 100):
        p7.duty_u16(i)
        time.sleep_ms(10)

    for i in range(65535, 0, -100):
        p7.duty_u16(i)
        time.sleep_ms(10)

    p0.value(not p0.value())

    for i in range(0, 65535, 100):
        p7.duty_u16(i)
        time.sleep_ms(10)

    for i in range(65535, 0, -100):
        p7.duty_u16(i)
        time.sleep_ms(10)

    p0.value(not p0.value())

    for i in range(0, 65535, 100):
        p8.duty_u16(i)
        time.sleep_ms(10)

    for i in range(65535, 0, -100):
        p8.duty_u16(i)
        time.sleep_ms(10)

    p2.value(not p2.value())

    for i in range(0, 65535, 100):
        p8.duty_u16(i)
        time.sleep_ms(10)

    for i in range(65535, 0, -100):
        p8.duty_u16(i)
        time.sleep_ms(10)

    p2.value(not p2.value())

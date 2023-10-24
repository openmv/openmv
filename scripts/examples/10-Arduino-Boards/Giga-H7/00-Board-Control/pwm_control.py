# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# PWM Control Example
#
# This example shows how to use PWM.

import time
from pyb import Pin, Timer


class PWM:
    def __init__(self, pin, tim, ch):
        self.pin = pin
        self.tim = tim
        self.ch = ch


pwms = {
    "PWM1": PWM("D7", 3, 1),
    "PWM2": PWM("D8", 4, 3),
    "PWM3": PWM("D9", 4, 4),
}

# Generate a 1KHz square wave with 50% cycle on the following PWM.
for k, pwm in pwms.items():
    tim = Timer(pwm.tim, freq=1000)  # Frequency in Hz
    ch = tim.channel(pwm.ch, Timer.PWM, pin=Pin(pwm.pin), pulse_width_percent=50)

while True:
    time.sleep_ms(1000)

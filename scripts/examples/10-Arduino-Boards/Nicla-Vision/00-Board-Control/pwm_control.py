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
    "PWM1": PWM("PE12", 1, 1),
    "PWM2": PWM("PE11", 1, 2),
    #   'PWM3' : PWM('PA9',  1, 2),
    "PWM3": PWM("PA10", 1, 3),
    "PWM4": PWM("PE14", 1, 4),
    "PWM5": PWM("PB8", 4, 3),
    "PWM6": PWM("PB9", 4, 4),
}

# Generate a 1KHz square wave with 50% cycle on the following PWM.
for k, pwm in pwms.items():
    tim = Timer(pwm.tim, freq=1000)  # Frequency in Hz
    ch = tim.channel(pwm.ch, Timer.PWM, pin=Pin(pwm.pin), pulse_width_percent=50)

while True:
    time.sleep_ms(1000)

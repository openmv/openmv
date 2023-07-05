# PWM Control Example
#
# This example shows how to do PWM with your OpenMV Cam.
#
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#                      !!! NOTE !!!
# THIS IS THE ONLY FREE PIN WHEN USING THE VISION SHIELD.
# DO NOT USE ANY OTHER PIN WHILE USING THE VISION SHIELD.
# PWM7/PH15
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

import time
from pyb import Pin, Timer


class PWM:
    def __init__(self, pin, tim, ch):
        self.pin = pin
        self.tim = tim
        self.ch = ch


pwms = {
    #   'PWM1' : PWM('PA8',  1, 1), # TIM1 is Reserved.
    "PWM2": PWM("PC6", 3, 1),
    "PWM3": PWM("PC7", 3, 2),
    #   'PWM4' : PWM('PG7',  0, 0), # HRTIM not supported.
    "PWM5": PWM("PJ11", 8, 2),
    "PWM6": PWM("PK1", 8, 3),
    "PWM7": PWM("PH15", 8, 3),
}

# Generate a 1KHz square wave with 50% cycle on the following PWM.
for k, pwm in pwms.items():
    tim = Timer(pwm.tim, freq=1000)  # Frequency in Hz
    ch = tim.channel(pwm.ch, Timer.PWM, pin=Pin(pwm.pin), pulse_width_percent=50)

while True:
    time.sleep_ms(1000)

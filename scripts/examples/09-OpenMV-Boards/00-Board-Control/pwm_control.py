# PWM Control Example
#
# This example shows how to do PWM with your OpenMV Cam.

import time
from pyb import Pin, Timer

tim = Timer(4, freq=1000)  # Frequency in Hz
# Generate a 1KHz square wave on TIM4 with 50%, 75% and 50% duty cycles on channels 1, 2 and 3 respectively.
ch1 = tim.channel(1, Timer.PWM, pin=Pin("P7"), pulse_width_percent=50)
ch2 = tim.channel(2, Timer.PWM, pin=Pin("P8"), pulse_width_percent=75)
ch3 = tim.channel(3, Timer.PWM, pin=Pin("P9"), pulse_width_percent=50)

while True:
    time.sleep_ms(1000)

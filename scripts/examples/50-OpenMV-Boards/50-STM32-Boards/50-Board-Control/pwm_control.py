# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# PWM Control Example
#
# This example shows how to do PWM with your OpenMV Cam.

import time
from pyb import Pin, Timer

tim = Timer(4, freq=1000)  # Frequency in Hz
# Generate a 1KHz square wave on TIM4 with 50% and 75% duty cycles on channels 1 and 2 respectively.
ch1 = tim.channel(1, Timer.PWM, pin=Pin("P7"), pulse_width_percent=50)
ch2 = tim.channel(2, Timer.PWM, pin=Pin("P8"), pulse_width_percent=75)

while True:
    time.sleep_ms(1000)

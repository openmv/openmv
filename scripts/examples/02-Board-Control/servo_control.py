# Servo Control Example
#
# This example shows how to use your OpenMV Cam to control servos.

import time
from pyb import Servo

s1 = Servo(1) # P7
s2 = Servo(2) # P8

while(True):
    for i in range(1000):
        s1.pulse_width(1000 + i)
        s2.pulse_width(1999 - i)
        time.sleep(10)
    for i in range(1000):
        s1.pulse_width(1999 - i)
        s2.pulse_width(1000 + i)
        time.sleep(10)

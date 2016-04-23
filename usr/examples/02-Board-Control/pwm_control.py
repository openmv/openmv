# PWM Control Example
#
# This example shows how to do PWM with your OpenMV Cam.
#
# WARNING: PWM control is... not easy with MicroPython. You have to use
# the correct timer with the correct pins and channels. As for what the
# correct values are - who knows. If you need to change the pins from the
# example below please try out different timer/channel/pin configs.

import pyb, time

t2 = pyb.Timer(1, freq=1000)

ch1 = t2.channel(2, pyb.Timer.PWM, pin=pyb.Pin("P0"))
ch2 = t2.channel(3, pyb.Timer.PWM, pin=pyb.Pin("P1"))

while(True):
    for i in range(100):
        ch1.pulse_width_percent(i)
        ch2.pulse_width_percent(100-i)
        time.sleep(5)
    for i in range(100):
        ch1.pulse_width_percent(100-i)
        ch2.pulse_width_percent(i)
        time.sleep(5)

# Motor Shield Power Driver Example
#
# This example shows off how to use your motor shield
# to control high current switches and so on. Since
# the motor shield provides two H-Bridge drivers it's
# able to provide 4 high current push-pull outputs.
# Each output can do up to 1A respectively.

import pyb

# These pins will be the ones we control to drive the
# H-Bridge sides as we like.
pinA = pyb.Pin("P3", pyb.Pin.OUT_PP, pyb.Pin.PULL_NONE)
pinB = pyb.Pin("P2", pyb.Pin.OUT_PP, pyb.Pin.PULL_NONE)
pinC = pyb.Pin("P1", pyb.Pin.OUT_PP, pyb.Pin.PULL_NONE)
pinD = pyb.Pin("P0", pyb.Pin.OUT_PP, pyb.Pin.PULL_NONE)

# Create a timer object running at 1KHz which which will power the
# PWM output on our OpenMV Cam. Just needs to be created once.
tim = pyb.Timer(4, freq=1000)

# These PWM channels will set the PWM percentage on the H-Bridge
# driver pair above. If you'd like to change the driver power
pinABPower = tim.channel(1, pyb.Timer.PWM, pin=pyb.Pin("P7"), pulse_width_percent=100)
pinCDPower = tim.channel(2, pyb.Timer.PWM, pin=pyb.Pin("P8"), pulse_width_percent=100)

while True:
    pyb.delay(1000)
    pinA.value(0)
    pinB.value(1)
    pinC.value(0)
    pinD.value(1)

    pyb.delay(1000)
    pinA.value(1)
    pinB.value(0)
    pinC.value(1)
    pinD.value(0)

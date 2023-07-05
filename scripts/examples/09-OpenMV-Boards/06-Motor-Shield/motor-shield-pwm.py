# Motor Shield PWM Example
#
# This example shows off how to control the motor shield on your
# OpenMV Cam. The motor shield is controlled by using the PYB module
# which lets you do PWM to control the speed and set digital I/O pin
# states. The motor shield needs 6 I/O pins for both motors.

import pyb

# These pins control our direction while the other PWN pins
# below control the speed. The direction for each motor
# is set by an H-Bridge where A0/1 are the two sides of
# one H-Bridge driver. B0/1 are another H-Bridge.
pinADir0 = pyb.Pin("P3", pyb.Pin.OUT_PP, pyb.Pin.PULL_NONE)
pinADir1 = pyb.Pin("P2", pyb.Pin.OUT_PP, pyb.Pin.PULL_NONE)
pinBDir0 = pyb.Pin("P1", pyb.Pin.OUT_PP, pyb.Pin.PULL_NONE)
pinBDir1 = pyb.Pin("P0", pyb.Pin.OUT_PP, pyb.Pin.PULL_NONE)

# Dir0/1 must be not equal to each other for forward or backwards
# operation. If they are equal then that's a brake operation.
# If they are not equal then the motor will spin one way other the
# other depending on it's hookup and the value of dir 0.
pinADir0.value(0)
pinADir1.value(1)

# Dir0/1 must be not equal to each other for forward or backwards
# operation. If they are equal then that's a brake operation.
# If they are not equal then the motor will spin one way other the
# other depending on it's hookup and the value of dir 0.
pinBDir0.value(0)
pinBDir1.value(1)

# Create a timer object running at 1KHz which which will power the
# PWM output on our OpenMV Cam. Just needs to be created once.
tim = pyb.Timer(4, freq=1000)

# Use the timer object to create two PWM outputs on the OpenMV Cam.
# These timers control the speed of the motors. You will be setting
# the PWM percentage of these timers repeatedly in your loop.
chA = tim.channel(1, pyb.Timer.PWM, pin=pyb.Pin("P7"))
chB = tim.channel(2, pyb.Timer.PWM, pin=pyb.Pin("P8"))

while True:
    for i in range(100):
        pyb.delay(100)
        chA.pulse_width_percent(i)
        chB.pulse_width_percent(99 - i)

    for i in range(100):
        pyb.delay(100)
        chA.pulse_width_percent(99 - i)
        chB.pulse_width_percent(i)

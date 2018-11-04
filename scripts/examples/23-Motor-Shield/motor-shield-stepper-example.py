# Motor Shield Stepper Motor Example
#
# This example shows off how to use your motor shield
# to control 1 stepper motor.

import time
from motor_shield_stepper import Stepper

stepper = Stepper() # default rpm=2, power=50
stepper.set_speed(1) # rpm = 1
stepper.set_power(80)
while (True):
    stepper.step(200)
    time.sleep(1000)

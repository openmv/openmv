# Motor Shield DC Motor Example
#
# This example shows off how to use your motor shield
# to control 2 DC motor.

import time
from tb6612 import Motor

m1 = Motor(1) # motor 1: A0 and A1
m2 = Motor(2) # motor 2: B0 and B1

while (True):
    m1.set_speed(100) # Forward
    m2.set_speed(100) # Forward
    time.sleep_ms(1000)

    m1.set_speed(0) # Stop
    m2.set_speed(0) # Stop
    time.sleep_ms(1000)

    m1.set_speed(-100) # Reverse
    m2.set_speed(-100) # Reverse
    time.sleep_ms(1000)

    m1.set_speed(0) # Stop
    m2.set_speed(0) # Stop
    time.sleep_ms(1000)

    m1.set_speed(-50) # Reverse slow
    m2.set_speed(-50) # Reverse slow
    time.sleep_ms(1000)

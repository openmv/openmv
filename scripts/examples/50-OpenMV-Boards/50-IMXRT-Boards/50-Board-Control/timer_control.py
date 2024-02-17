# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Timer Control Example
#
# This example shows how to use a timer for callbacks.

import time
from machine import LED
from machine import Timer

blue_led = LED("LED_BLUE")


# we will receive the timer object when being called
# Note: functions that allocate memory are Not allowed in callbacks
def tick(timer):
    blue_led.toggle()


# The machine module currently only supports virtual timers via -1.
tim = Timer(-1, freq=1, callback=tick)  # create a timer object - trigger at 1Hz
print(tim)

while True:
    time.sleep_ms(1000)

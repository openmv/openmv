# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Timer Control Example
#
# This example shows how to use a timer for callbacks.

import time
from pyb import LED
from pyb import Timer

blue_led = LED(3)


# we will receive the timer object when being called
# Note: functions that allocate memory are Not allowed in callbacks
def tick(timer):
    blue_led.toggle()


tim = Timer(2, freq=1)  # create a timer object using timer 2 - trigger at 1Hz
tim.callback(tick)  # set the callback to our tick function

while True:
    time.sleep_ms(1000)

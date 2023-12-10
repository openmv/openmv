# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Timer Test Example
#
# This example tests all the timers.

import time
from pyb import LED
from pyb import Timer

blue_led = LED(3)


# Note: functions that allocate memory are Not allowed in callbacks
def tick(timer):
    blue_led.toggle()


print("")
for i in range(1, 18):
    try:
        print("Testing TIM%d... " % (i), end="")
        tim = Timer(i, freq=10, callback=tick)
        time.sleep_ms(1000)
        tim.deinit()
    except ValueError as e:
        print(e)
        continue
    print("done!")

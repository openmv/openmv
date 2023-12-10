# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# RTC Example
#
# This example shows how to use the RTC.
import time
from pyb import RTC

rtc = RTC()
rtc.datetime((2013, 7, 9, 2, 0, 0, 0, 0))

while True:
    print(rtc.datetime())
    time.sleep_ms(1000)

# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# RTC Example
#
# This example shows how to use the RTC.

import time
from machine import RTC

rtc = RTC()

# Comment this out to initialize the RTC time and date.
# (year, month, day, weekday, hour, minute, second, microsecond)
# rtc.datetime((2026, 1, 1, 4, 12, 0, 0, 0))

while True:
    time.sleep_ms(100)
    print(rtc.datetime())

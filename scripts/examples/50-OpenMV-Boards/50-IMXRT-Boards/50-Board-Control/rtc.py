# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# RTC Example
#
# This example shows how to use the RTC.

import time
from machine import RTC

rtc = RTC()

# Comment this out to initialize the RTC time and date.
# After doing this the RTC will keep the time until
# power is completely lost by the system.
# rtc.init((2023, 8, 8, 14, 15, 0, 0, 0))

while True:
    time.sleep_ms(100)
    print(rtc.now())

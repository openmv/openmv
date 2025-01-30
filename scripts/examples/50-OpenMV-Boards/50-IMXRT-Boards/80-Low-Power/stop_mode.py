# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Stop Mode Example
# This example demonstrates using the low-power Stop Mode.

import machine

# Create and init RTC object.
rtc = machine.RTC()
# (year, month, day[, hour[, minute[, second[, microsecond[, tzinfo]]]]])
rtc.datetime((2014, 5, 1, 4, 13, 0, 0, 0))

# Print RTC info.
print(rtc.datetime())

# Enable RTC interrupts every 5 seconds.
rtc.alarm(rtc.ALARM0, 5000)

# Enter Stop Mode.
# Note the IDE will disconnect.
machine.deepsleep()

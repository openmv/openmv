# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Deep Sleep Mode Example
# This example demonstrates the low-power deep sleep mode.
import machine

# Create and init RTC object.
rtc = machine.RTC()

# (year, month, day, weekday, hour, minute, second, microsecond)
rtc.datetime((2026, 1, 1, 4, 12, 0, 0, 0))

# Print RTC info.
print(rtc.datetime())

# Enter Deepsleep Mode and wake up after 30 seconds.
machine.deepsleep(30000)

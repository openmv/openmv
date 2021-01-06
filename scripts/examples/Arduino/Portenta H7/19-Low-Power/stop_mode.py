# Stop Mode Example
# This example demonstrates using the low-power Stop Mode.

import time, pyb, machine

# Create and init RTC object.
rtc = pyb.RTC()
# (year, month, day[, hour[, minute[, second[, microsecond[, tzinfo]]]]])
rtc.datetime((2014, 5, 1, 4, 13, 0, 0, 0))

# Print RTC info.
print(rtc.datetime())

# Enable RTC interrupts every 5 seconds.
rtc.wakeup(5000)

# Enter Stop Mode.
# Note the IDE will disconnect.
machine.sleep()

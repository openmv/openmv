# Deepsleep Mode Example
# This example demonstrates using the low-power Deepsleep Mode.
# Note the camera will reset after wake-up from deep sleep. To find out if the cause of reset
# is deep sleep, call the machine.reset_cause() function and test for machine.DEEPSLEEP_RESET
import pyb, machine

# Create and init RTC object.
rtc = pyb.RTC()

# (year, month, day[, hour[, minute[, second[, microsecond[, tzinfo]]]]])
rtc.datetime((2014, 5, 1, 4, 13, 0, 0, 0))

# Print RTC info.
print(rtc.datetime())

# Enable RTC interrupts every 5 seconds.
# Note the camera will RESET after wakeup from Deepsleep Mode.
rtc.wakeup(5000)

# Enter Deepsleep Mode.
machine.deepsleep()

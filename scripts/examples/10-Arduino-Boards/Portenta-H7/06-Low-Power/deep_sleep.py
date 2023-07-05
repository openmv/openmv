# Deep Sleep Mode Example
# This example demonstrates the low-power deep sleep mode plus sensor shutdown.
# Note the camera will reset after wake-up from deep sleep. To find out if the cause of reset
# is deep sleep, call the machine.reset_cause() function and test for machine.DEEPSLEEP_RESET
import pyb
import machine
import sensor

# Create and init RTC object.
rtc = pyb.RTC()

# (year, month, day[, hour[, minute[, second[, microsecond[, tzinfo]]]]])
rtc.datetime((2014, 5, 1, 4, 13, 0, 0, 0))

# Print RTC info.
print(rtc.datetime())

sensor.reset()

# Shutdown the sensor (pulls PWDN high).
sensor.shutdown(True)

# Enable RTC interrupts every 30 seconds.
# Note the camera will RESET after wakeup from Deepsleep Mode.
rtc.wakeup(30000)

# Enter Deepsleep Mode.
machine.deepsleep()

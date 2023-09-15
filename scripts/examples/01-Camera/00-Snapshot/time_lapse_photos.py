# Time Lapse Photos (Credit nedhorning)
#
# This example shows off how to take time lapse photos using your OpenMV
# Cam and using the RTC module along with a timer interrupt to achieve
# very low power operation.
#
# Note that if the USB is still plugged in when the camera is taking
# pictures it will run the bootloader each time. Please power the camera
# from something other than USB to not have the bootloader run.

import pyb
import machine
import sensor
import os

# Create and init RTC object. This will allow us to set the current time for
# the RTC and let us set an interrupt to wake up later on.
rtc = pyb.RTC()
newFile = False

try:
    os.stat("time.txt")
except (
    OSError
):  # If the log file doesn't exist then set the RTC and set newFile to True
    # datetime format: year, month, day, weekday (Monday=1, Sunday=7),
    # hours (24 hour clock), minutes, seconds, subseconds (counts down from 255 to 0)
    rtc.datetime((2018, 3, 9, 5, 13, 0, 0, 0))
    newFile = True

# Extract the date and time from the RTC object.
dateTime = rtc.datetime()
year = str(dateTime[0])
month = "%02d" % dateTime[1]
day = "%02d" % dateTime[2]
hour = "%02d" % dateTime[4]
minute = "%02d" % dateTime[5]
second = "%02d" % dateTime[6]
subSecond = str(dateTime[7])

newName = (
    "I" + year + month + day + hour + minute + second
)  # Image file name based on RTC

# Enable RTC interrupts every 10 seconds, camera will RESET after wakeup from deepsleep Mode.
rtc.wakeup(10000)

BLUE_LED_PIN = 3

sensor.reset()  # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.VGA)
sensor.skip_frames(time=1000)  # Let new settings take affect.

# Let folks know we are about to take a picture.
pyb.LED(BLUE_LED_PIN).on()

if newFile:  # If log file does not exist then create it.
    with open(
        "time.txt", "a"
    ) as timeFile:  # Write text file to keep track of date, time and image number.
        timeFile.write(
            "Date and time format: year, month, day, hours, minutes, seconds, subseconds"
            + "\n"
        )
        timeFile.write(
            newName
            + ","
            + year
            + ","
            + month
            + ","
            + day
            + ","
            + hour
            + ","
            + minute
            + ","
            + second
            + ","
            + subSecond
            + "\n"
        )
else:
    with open(
        "time.txt", "a"
    ) as timeFile:  # Append to date, time and image number to text file.
        timeFile.write(
            newName
            + ","
            + year
            + ","
            + month
            + ","
            + day
            + ","
            + hour
            + ","
            + minute
            + ","
            + second
            + ","
            + subSecond
            + "\n"
        )

if not "images" in os.listdir():
    os.mkdir("images")  # Make a temp directory

# Take photo and save to SD card
img = sensor.snapshot()
img.save("images/" + newName, quality=90)
pyb.LED(BLUE_LED_PIN).off()

# Enter Deepsleep Mode (i.e. the OpenMV Cam effectively turns itself off except for the RTC).
machine.deepsleep()

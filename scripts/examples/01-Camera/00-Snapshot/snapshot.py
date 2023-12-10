# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Snapshot Example
#
# Note: You will need an SD card to run this example.
# You can use your OpenMV Cam to save image files.

import sensor
import time
import machine

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)  # Set frame size to QVGA (320x240)
sensor.skip_frames(time=2000)  # Wait for settings take effect.

led = machine.LED("LED_BLUE")

start = time.ticks_ms()
while time.ticks_diff(time.ticks_ms(), start) < 3000:
    sensor.snapshot()
    led.toggle()

led.off()

img = sensor.snapshot()
img.save("example.jpg")  # or "example.bmp" (or others)

raise (Exception("Please reset the camera to see the new file."))

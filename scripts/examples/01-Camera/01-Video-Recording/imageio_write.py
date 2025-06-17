# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Image Writer Example
#
# NOTE: This example requires an SD card.
#
# This example shows how to use the Image Writer object to record a raw video file
# for later analysis using the Image Reader object.

import sensor
import image
import time
import machine

record_time = 10000  # 10 seconds in milliseconds

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)  # Set frame size to QQVGA (160x120)
sensor.skip_frames(time=2000)  # Wait for settings take effect.
clock = time.clock()  # Create a clock object to track the FPS.

led = machine.LED("LED_RED")
stream = image.ImageIO("stream.bin", "w")

# Red LED on means we are capturing frames.
led.on()

start = time.ticks_ms()
while time.ticks_diff(time.ticks_ms(), start) < record_time:
    clock.tick()
    img = sensor.snapshot()
    # Modify the image if you feel like here...
    stream.write(img)
    print(clock.fps())

stream.close()
led.off()

raise (Exception("Please reset the camera to see the new file."))

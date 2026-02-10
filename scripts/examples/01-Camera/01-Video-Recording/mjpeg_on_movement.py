# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# MJPEG Video Recording on Movement Example
#
# Note: You will need an SD card to run this example.
#
# You can use your OpenMV Cam to record mjpeg files. You can either feed the
# recorder object JPEG frames or RGB565/Grayscale frames. Once you've finished
# recording a Mjpeg file you can use VLC to play it. If you are on Ubuntu then
# the built-in video player will work too.
#
# This example demonstrates using frame differencing with your OpenMV Cam to do
# motion detection. After motion is detected your OpenMV Cam will take video.

import csi
import time
import mjpeg
import os
import machine
import random

csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.pixformat(csi.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
csi0.framesize(csi.QVGA)  # Set frame size to QVGA (320x240)
csi0.snapshot(time=2000)  # Wait for settings take effect.
csi0.auto_whitebal(False)  # Turn off white balance.

led = machine.LED("LED_RED")

if not "temp" in os.listdir():
    os.mkdir("temp")  # Make a temp directory

while True:
    print("About to save background image...")
    csi0.snapshot(time=2000)  # Give the user time to get ready.

    csi0.snapshot().save("temp/bg.bmp")
    print("Saved background image - Now detecting motion!")

    diff = 10  # We'll say we detected motion after 10 frames of motion.
    while diff:
        img = csi0.snapshot()
        img.difference("temp/bg.bmp")
        stats = img.statistics()
        # Stats 5 is the max of the lighting color channel. The below code
        # triggers when the lighting max for the whole image goes above 20.
        # The lighting difference maximum should be zero normally.
        if stats[5] > 20:
            diff -= 1

    led.on()
    m = mjpeg.Mjpeg("example-%d.mjpeg" % random.getrandbits(32))

    clock = time.clock()  # Tracks FPS.
    for i in range(200):
        clock.tick()
        m.write(csi0.snapshot())
        print(clock.fps())

    m.close()
    led.off()
    print("Restarting...")

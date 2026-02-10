# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Snapshot on Movement Example
#
# Note: You will need an SD card to run this example.
#
# This example demonstrates using frame differencing with your OpenMV Cam to do
# motion detection. After motion is detected your OpenMV Cam will take picture.

import csi
import random
import os
import machine

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
    print("Movement detected! Saving image...")
    csi0.snapshot().save("temp/snapshot-%d.jpg" % random.getrandbits(32))  # Save Pic.
    led.off()

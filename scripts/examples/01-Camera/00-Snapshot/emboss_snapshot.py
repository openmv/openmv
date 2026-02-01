# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Emboss Snapshot Example
#
# Note: You will need an SD card to run this example.
# You can use your OpenMV Cam to save modified image files.

import csi
import time
import machine

csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.pixformat(csi.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
csi0.framesize(csi.QVGA)  # Set frame size to QVGA (320x240)
csi0.snapshot(time=2000)  # Wait for settings take effect.

led = machine.LED("LED_BLUE")

start = time.ticks_ms()
while time.ticks_diff(time.ticks_ms(), start) < 3000:
    csi0.snapshot()
    led.toggle()

led.off()

img = csi0.snapshot()
img.morph(1, [+2, +1, +0, +1, +1, -1, +0, -1, -2])  # Emboss the image.
img.save("example.jpg")  # or "example.bmp" (or others)

raise (Exception("Please reset the camera to see the new file."))

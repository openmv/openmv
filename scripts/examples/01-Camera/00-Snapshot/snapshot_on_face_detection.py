# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Snapshot on Face Detection Example
#
# Note: You will need an SD card to run this example.
#
# This example demonstrates using face tracking on your OpenMV Cam to take a
# picture.

import sensor
import image
import random
import machine

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.GRAYSCALE)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)  # Set frame size to QVGA
sensor.skip_frames(time=2000)  # Wait for settings take effect.

led = machine.LED("LED_RED")

# Load up a face detection HaarCascade. This is object that your OpenMV Cam
# can use to detect faces using the find_features() method below. Your OpenMV
# Cam has fontalface HaarCascade built-in. By default, all the stages of the
# HaarCascade are loaded. However, You can adjust the number of stages to speed
# up processing at the expense of accuracy. The frontalface HaarCascade has 25
# stages.
face_cascade = image.HaarCascade("/rom/haarcascade_frontalface.cascade", stages=25)

while True:
    print("About to start detecting faces...")
    sensor.skip_frames(time=2000)  # Give the user time to get ready.

    print("Now detecting faces!")
    diff = 10  # We'll say we detected a face after 10 frames.

    while diff:
        img = sensor.snapshot()
        # Threshold can be between 0.0 and 1.0. A higher threshold results in a
        # higher detection rate with more false positives. The scale value
        # controls the matching scale allowing you to detect smaller faces.
        faces = img.find_features(face_cascade, threshold=0.5, scale_factor=1.5)

        if faces:
            diff -= 1
            for r in faces:
                img.draw_rectangle(r)
    led.on()
    print("Face detected! Saving image...")
    sensor.snapshot().save("snapshot-%d.jpg" % random.getrandbits(32))  # Save Pic.
    led.off()

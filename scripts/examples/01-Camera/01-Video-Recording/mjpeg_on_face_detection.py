# MJPEG Video Recording on Face Detection Example
#
# Note: You will need an SD card to run this example.
#
# You can use your OpenMV Cam to record mjpeg files. You can either feed the
# recorder object JPEG frames or RGB565/Grayscale frames. Once you've finished
# recording a Mjpeg file you can use VLC to play it. If you are on Ubuntu then
# the built-in video player will work too.
#
# This example demonstrates using face tracking on your OpenMV Cam to take a
# mjpeg.

import sensor
import image
import time
import mjpeg
import pyb

RED_LED_PIN = 1
BLUE_LED_PIN = 3

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.GRAYSCALE)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)  # Set frame size to QQVGA (160x120)
sensor.skip_frames(time=2000)  # Wait for settings take effect.

# Load up a face detection HaarCascade. This is object that your OpenMV Cam
# can use to detect faces using the find_features() method below. Your OpenMV
# Cam has fontalface HaarCascade built-in. By default, all the stages of the
# HaarCascade are loaded. However, You can adjust the number of stages to speed
# up processing at the expense of accuracy. The frontalface HaarCascade has 25
# stages.
face_cascade = image.HaarCascade("frontalface", stages=25)

while True:
    pyb.LED(RED_LED_PIN).on()
    print("About to start detecting faces...")
    sensor.skip_frames(time=2000)  # Give the user time to get ready.

    pyb.LED(RED_LED_PIN).off()
    print("Now detecting faces!")
    pyb.LED(BLUE_LED_PIN).on()

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

    m = mjpeg.Mjpeg("example-%d.mjpeg" % pyb.rng())

    clock = time.clock()  # Tracks FPS.
    print("You're on camera!")
    for i in range(200):
        clock.tick()
        m.add_frame(sensor.snapshot())
        print(clock.fps())

    m.close(clock.fps())
    pyb.LED(BLUE_LED_PIN).off()
    print("Restarting...")

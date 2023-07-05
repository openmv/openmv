# Snapshot on Movement Example
#
# Note: You will need an SD card to run this example.
#
# This example demonstrates using frame differencing with your OpenMV Cam to do
# motion detection. After motion is detected your OpenMV Cam will take picture.

import sensor
import pyb
import os

RED_LED_PIN = 1
BLUE_LED_PIN = 3

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)  # Set frame size to QVGA (320x240)
sensor.skip_frames(time=2000)  # Wait for settings take effect.
sensor.set_auto_whitebal(False)  # Turn off white balance.

if not "temp" in os.listdir():
    os.mkdir("temp")  # Make a temp directory

while True:
    pyb.LED(RED_LED_PIN).on()
    print("About to save background image...")
    sensor.skip_frames(time=2000)  # Give the user time to get ready.

    pyb.LED(RED_LED_PIN).off()
    sensor.snapshot().save("temp/bg.bmp")
    print("Saved background image - Now detecting motion!")
    pyb.LED(BLUE_LED_PIN).on()

    diff = 10  # We'll say we detected motion after 10 frames of motion.
    while diff:
        img = sensor.snapshot()
        img.difference("temp/bg.bmp")
        stats = img.statistics()
        # Stats 5 is the max of the lighting color channel. The below code
        # triggers when the lighting max for the whole image goes above 20.
        # The lighting difference maximum should be zero normally.
        if stats[5] > 20:
            diff -= 1

    pyb.LED(BLUE_LED_PIN).off()
    print("Movement detected! Saving image...")
    sensor.snapshot().save("temp/snapshot-%d.jpg" % pyb.rng())  # Save Pic.

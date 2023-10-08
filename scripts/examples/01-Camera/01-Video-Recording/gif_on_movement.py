# GIF Video Recording on Movement Example
#
# Note: You will need an SD card to run this example.
#
# You can use your OpenMV Cam to record gif files. You can either feed the
# recorder object RGB565 frames or Grayscale frames. Use photo editing software
# like GIMP to compress and optimize the Gif before uploading it to the web.
#
# This example demonstrates using frame differencing with your OpenMV Cam to do
# motion detection. After motion is detected your OpenMV Cam will take video.

import sensor
import time
import gif
import os
import machine
import random

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)  # Set frame size to QQVGA (160x120)
sensor.skip_frames(time=2000)  # Wait for settings take effect.
sensor.set_auto_whitebal(False)  # Turn off white balance.

led = machine.LED("LED_RED")

if not "temp" in os.listdir():
    os.mkdir("temp")  # Make a temp directory

while True:
    print("About to save background image...")
    sensor.skip_frames(time=2000)  # Give the user time to get ready.

    sensor.snapshot().save("temp/bg.bmp")
    print("Saved background image - Now detecting motion!")

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

    led.on()
    g = gif.Gif("example-%d.gif" % random.getrandbits(32), loop=True)

    clock = time.clock()  # Tracks FPS.
    for i in range(100):
        clock.tick()
        # clock.avg() returns the milliseconds between frames - gif delay is in
        g.add_frame(sensor.snapshot(), delay=int(clock.avg() / 10))  # centiseconds.
        print(clock.fps())

    g.close()
    led.off()
    print("Restarting...")

# MJPEG Video Recording Example
#
# Note: You will need an SD card to run this demo.
#
# You can use your OpenMV Cam to record mjpeg files. You can either feed the
# recorder object JPEG frames or RGB565/Grayscale frames. Once you've finished
# recording a Mjpeg file you can use VLC to play it. If you are on Ubuntu then
# the built-in video player will work too.

import sensor
import time
import mjpeg
import pyb

RED_LED_PIN = 1
BLUE_LED_PIN = 3

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)  # Set frame size to QVGA (320x240)
sensor.skip_frames(time=2000)  # Wait for settings take effect.
clock = time.clock()  # Create a clock object to track the FPS.

pyb.LED(RED_LED_PIN).on()
sensor.skip_frames(time=2000)  # Give the user time to get ready.

pyb.LED(RED_LED_PIN).off()
pyb.LED(BLUE_LED_PIN).on()

m = mjpeg.Mjpeg("example.mjpeg")

print("You're on camera!")
for i in range(200):
    clock.tick()
    m.add_frame(sensor.snapshot())
    print(clock.fps())

m.close(clock.fps())
pyb.LED(BLUE_LED_PIN).off()
print("Done! Reset the camera to see the saved recording.")

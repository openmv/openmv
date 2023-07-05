# GIF Video Recording Example
#
# Note: You will need an SD card to run this example.
#
# You can use your OpenMV Cam to record gif files. You can either feed the
# recorder object RGB565 frames or Grayscale frames. Use photo editing software
# like GIMP to compress and optimize the Gif before uploading it to the web.

import sensor
import time
import gif
import pyb

RED_LED_PIN = 1
BLUE_LED_PIN = 3

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)  # Set frame size to QQVGA (160x120)
sensor.skip_frames(time=2000)  # Wait for settings take effect.
clock = time.clock()  # Create a clock object to track the FPS.

pyb.LED(RED_LED_PIN).on()
sensor.skip_frames(time=2000)  # Give the user time to get ready.

pyb.LED(RED_LED_PIN).off()
pyb.LED(BLUE_LED_PIN).on()

g = gif.Gif("example.gif", loop=True)

print("You're on camera!")
for i in range(100):
    clock.tick()
    # clock.avg() returns the milliseconds between frames - gif delay is in
    g.add_frame(sensor.snapshot(), delay=int(clock.avg() / 10))  # centiseconds.
    print(clock.fps())

g.close()
pyb.LED(BLUE_LED_PIN).off()
print("Done! Reset the camera to see the saved recording.")

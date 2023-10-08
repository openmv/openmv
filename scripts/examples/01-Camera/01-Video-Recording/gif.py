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
import machine

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)  # Set frame size to QQVGA (160x120)
sensor.skip_frames(time=2000)  # Wait for settings take effect.

led = machine.LED("LED_RED")

led.on()
g = gif.Gif("example.gif", loop=True)

clock = time.clock()  # Create a clock object to track the FPS.
for i in range(100):
    clock.tick()
    # clock.avg() returns the milliseconds between frames - gif delay is in
    g.add_frame(sensor.snapshot(), delay=int(clock.avg() / 10))  # centiseconds.
    print(clock.fps())

g.close()
led.off()

raise (Exception("Please reset the camera to see the new file."))

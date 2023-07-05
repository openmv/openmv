# High FPS Example
#
# This example shows off how to make the frame rate of the global shutter camera extremely
# high. To do so you need to set the resolution to a low value such that pixel binning is
# activated on the camera and then reduce the maximum exposure time.
#
# When the resolution is 320x240 or less the camera reads out pixels 2x faster. When the
# resolution is 160x120 or less the camera reads out pixels 4x faster. This happens due
# to pixel binning which is automatically activated for you to increase the readout speed.
#
# While the readout speed may increase the camera must still expose the image for the request
# time so you will not get the maximum readout speed unless you reduce the exposure time too.
# This results in a dark image however so YOU NEED A LOT of lighting for high FPS.

import sensor
import time

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.GRAYSCALE)  # Set pixel format to GRAYSCALE
sensor.set_framesize(sensor.QQVGA)  # Set frame size to QQVGA (160x120)
sensor.skip_frames(time=2000)  # Wait for settings take effect.
clock = time.clock()  # Create a clock object to track the FPS.

sensor.set_auto_exposure(True, exposure_us=5000)  # make smaller to go faster

while True:
    clock.tick()  # Update the FPS clock.
    img = sensor.snapshot()  # Take a picture and return the image.
    print(clock.fps())  # Note: OpenMV Cam runs about half as fast when connected
    # to the IDE. The FPS should increase once disconnected.

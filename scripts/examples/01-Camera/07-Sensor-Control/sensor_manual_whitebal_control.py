# Sensor Manual Whitebal Control
#
# This example shows off how to control the camera sensor's
# white balance gain manually versus letting the AWB control run.

# White balance is achieve by adjusting R/G/B gain values
# such that the average color of the image is gray. The
# automatic white balance (AWB) algorithm does this for
# you but usually ends up with a different result each
# time you turn the camera on making it hard to get
# color tracking settings right. By manually recording
# the gain values you like and then forcing them to
# the sensor on startup you can control the colors
# the camera sees.

import sensor
import time

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)  # Set frame size to QVGA (320x240)
sensor.skip_frames(time=2000)  # Wait for settings take effect.
clock = time.clock()  # Create a clock object to track the FPS.

# You can control the white balance gains here. The first value is the
# R gain in db, and then the G gain in db, followed by the B gain in db.
#
# Uncomment the below line with gain values you like (get them from the print out).
#
# sensor.set_auto_whitebal(False, rgb_gain_db = (0.0, 0.0, 0.0))

# Note: Putting (0.0, 0.0, 0.0) for the gain results in something close to zero
# comming out. Do not expect the exact value going in to be equal to the value
# comming out.

while True:
    clock.tick()  # Update the FPS clock.
    img = sensor.snapshot()  # Take a picture and return the image.
    print(clock.fps(), sensor.get_rgb_gain_db())  # Prints the AWB current RGB gains.

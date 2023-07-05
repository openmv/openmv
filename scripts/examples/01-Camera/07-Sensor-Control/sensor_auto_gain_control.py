# Sensor Auto Gain Control
#
# This example shows off how to control the sensor's gain
# using the automatic gain control algorithm.

# What's the difference between gain and exposure control?
#
# Well, by increasing the exposure time for the image you're getting more
# light on the camera. This gives you the best signal to noise ratio. You
# in general always want to increase the expsoure time... except, when you
# increase the exposure time you decrease the maximum possible frame rate
# and if anything moves in the image it will start to blur more with a
# higher exposure time. Gain control allows you to increase the output per
# pixel using analog and digital multipliers... however, it also amplifies
# noise. So, it's best to let the exposure increase as much as possible
# and then use gain control to make up any remaining ground.

# We can achieve the above by setting a gain ceiling on the automatic
# gain control algorithm. Once this is set the algorithm will have to
# increase the exposure time to meet any gain needs versus using gain
# to do so. However, this comes at the price of the exposure time varying
# more when the lighting changes versus the exposure being constant and
# the gain changing.

import sensor
import time

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)  # Set frame size to QVGA (320x240)

# The gain db ceiling maxes out at about 24 db for the OV7725 sensor.
sensor.set_auto_gain(True, gain_db_ceiling=16.0)  # Default gain.

# Note! If you set the gain ceiling to low without adjusting the exposure control
# target value then you'll just get a lot of oscillation from the exposure
# control if it's on.

sensor.skip_frames(time=2000)  # Wait for settings take effect.
clock = time.clock()  # Create a clock object to track the FPS.

while True:
    clock.tick()  # Update the FPS clock.
    img = sensor.snapshot()  # Take a picture and return the image.
    print(
        "FPS %f, Gain %f dB, Exposure %d us"
        % (clock.fps(), sensor.get_gain_db(), sensor.get_exposure_us())
    )

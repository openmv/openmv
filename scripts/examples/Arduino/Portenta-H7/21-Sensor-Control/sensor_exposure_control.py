# Sensor Exposure Control
#
# This example shows off how to cotnrol the camera sensor's
# exposure manually versus letting auto exposure control run.

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

import sensor, image, time

# Change this value to adjust the exposure. Try 10.0/0.1/etc.
EXPOSURE_TIME_SCALE = 1.0

sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)

# Print out the initial exposure time for comparison.
print("Initial exposure == %d" % sensor.get_exposure_us())

sensor.skip_frames(time = 2000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.

# You have to turn automatic gain control and automatic white blance off
# otherwise they will change the image gains to undo any exposure settings
# that you put in place...
sensor.set_auto_gain(False)
# Need to let the above settings get in...
sensor.skip_frames(time = 500)

current_exposure_time_in_microseconds = sensor.get_exposure_us()
print("Current Exposure == %d" % current_exposure_time_in_microseconds)

# Auto exposure control (AEC) is enabled by default. Calling the below function
# disables sensor auto exposure control. The additionally "exposure_us"
# argument then overrides the auto exposure value after AEC is disabled.
sensor.set_auto_exposure(False, \
    exposure_us = int(current_exposure_time_in_microseconds * EXPOSURE_TIME_SCALE))

print("New exposure == %d" % sensor.get_exposure_us())
# sensor.get_exposure_us() returns the exact camera sensor exposure time
# in microseconds. However, this may be a different number than what was
# commanded because the sensor code converts the exposure time in microseconds
# to a row/pixel/clock time which doesn't perfectly match with microseconds...

# If you want to turn auto exposure back on do: sensor.set_auto_exposure(True)
# Note that the camera sensor will then change the exposure time as it likes.

# Doing: sensor.set_auto_exposure(False)
# Just disables the exposure value update but does not change the exposure
# value the camera sensor determined was good.

while(True):
    clock.tick()                    # Update the FPS clock.
    img = sensor.snapshot()         # Take a picture and return the image.
    print(clock.fps())              # Note: OpenMV Cam runs about half as fast when connected
                                    # to the IDE. The FPS should increase once disconnected.

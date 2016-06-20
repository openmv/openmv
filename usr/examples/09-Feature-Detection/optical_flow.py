# Optical Flow Example
#
# Your OpenMV Cam can use optical flow to determine the displacement between
# two images. This allows your OpenMV Cam to track movement like how your laser
# mouse tracks movement. By tacking the difference between successive images
# you can determine instaneous displacement with your OpenMV Cam too!

import sensor, image, time

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.B64x32) # or B40x30 or B64x64
clock = time.clock() # Tracks FPS.

# NOTE: The find_displacement function works by taking the 2D FFTs of the old
# and new images and compares them using phase correlation. Your OpenMV Cam
# only has enough memory to work on two 64x64 FFTs (or 128x32, 32x128, or etc).
old = sensor.snapshot()

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.

    [delta_x, delta_y, response] = old.find_displacement(img)

    old = img.copy()

    print("%0.1f X\t%0.1f Y\t%0.2f QoR\t%0.2f FPS" % \
        (delta_x, delta_y, response, clock.fps()))

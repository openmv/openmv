# Optical Flow Example
#
# Your OpenMV Cam can use optical flow to determine the displacement between
# two images. This allows your OpenMV Cam to track movement like how your laser
# mouse tracks movement. By tacking the difference between successive images
# you can determine instaneous displacement with your OpenMV Cam too!

import sensor, image, time

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QVGA) # or sensor.QQVGA (or others)
sensor.skip_frames(10) # Let new settings take affect.
clock = time.clock() # Tracks FPS.

# Create a down sampled copy of the image. Down sampling is by 5 (horizontally)
# and 4 (vertically). This results in a 64x60 image from QVGA.
old = sensor.snapshot().mean_pooled(5, 4)

# NOTE: The find_displacement function works by taking the 2D FFTs of the old
# and new images and compares them using phase correlation. Your OpenMV Cam
# only has enough memory to work on two 64x64 FFTs (or 128x32, 32x128, or etc).

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.

    # Down sample the current image in place.
    img.mean_pool(5, 4)

    # Delta X is the x displacement. Note that it is only valid for small
    # amounts of displacement before being ambiguous...

    # Delta Y is the x displacement. Note that it is only valid for small
    # amounts of displacement before being ambiguous...

    # Reponse is the quality of the displacement info. When it goes below
    # 0.10 or so the quality of the results are poor...

    [delta_x, delta_y, response] = old.find_displacement(img)
    print("%0.1f X\t%0.1f Y\t%0.2f QoR\t%0.2f FPS" % \
        (delta_x, delta_y, response, clock.fps()))

    # Uncomment this to get the difference between frames
    # old = img.copy()

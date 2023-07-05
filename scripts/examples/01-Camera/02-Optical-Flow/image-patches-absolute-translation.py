# Image Patches Absolute Optical Flow Translation
#
# This example shows off using your OpenMV Cam to measure translation
# in the X and Y direction by comparing the current and a previous
# image against each other. Note that only X and Y translation is
# handled - not rotation/scale in this mode.
#
# However, this examples goes beyond doing optical flow on the whole
# image at once. Instead it breaks up the process by working on groups
# of pixels in the image. This gives you a "new" image of results.
#
# NOTE that surfaces need to have some type of "edge" on them for the
# algorithm to work. A featureless surface produces crazy results.

BLOCK_W = 16 # pow2
BLOCK_H = 16 # pow2

# To run this demo effectively please mount your OpenMV Cam on a steady
# base and SLOWLY translate it to the left, right, up, and down and
# watch the numbers change. Note that you can see displacement numbers
# up +- half of the hoizontal and vertical resolution.

import sensor
import time

# NOTE!!! You have to use a small power of 2 resolution when using
# find_displacement(). This is because the algorithm is powered by
# something called phase correlation which does the image comparison
# using FFTs. A non-power of 2 resolution requires padding to a power
# of 2 which reduces the usefulness of the algorithm results. Please
# use a resolution like B128X128 or B128X64 (2x faster).

# Your OpenMV Cam supports power of 2 resolutions of 64x32, 64x64,
# 128x64, and 128x128. If you want a resolution of 32x32 you can create
# it by doing "img.pool(2, 2)" on a 64x64 image.

sensor.reset()                         # Reset and initialize the sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # Set pixel format to GRAYSCALE (or RGB565)
sensor.set_framesize(sensor.B128X128)  # Set frame size to 128x128... (or 128x64)...
sensor.skip_frames(time = 2000)        # Wait for settings take effect.
clock = time.clock()                   # Create a clock object to track the FPS.

# Take from the main frame buffer's RAM to allocate a second frame buffer.
# There's a lot more RAM in the frame buffer than in the MicroPython heap.
# However, after doing this you have a lot less RAM for some algorithms...
# So, be aware that it's a lot easier to get out of RAM issues now.
extra_fb = sensor.alloc_extra_fb(sensor.width(), sensor.height(), sensor.GRAYSCALE)
extra_fb.replace(sensor.snapshot())

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.

    for y in range(0, sensor.height(), BLOCK_H):
        for x in range(0, sensor.width(), BLOCK_W):
            # For this example we never update the old image to measure absolute change.
            displacement = extra_fb.find_displacement(img, \
                roi = (x, y, BLOCK_W, BLOCK_H), template_roi = (x, y, BLOCK_W, BLOCK_H))

            # Below 0.1 or so (YMMV) and the results are just noise.
            if(displacement.response() > 0.1):
                pixel_x = x + (BLOCK_W//2) + int(displacement.x_translation())
                pixel_y = y + (BLOCK_H//2) + int(displacement.y_translation())
                img.draw_line((x + BLOCK_W//2, y + BLOCK_H//2, pixel_x, pixel_y), \
                    color = 255)
            else:
                img.draw_line((x + BLOCK_W//2, y + BLOCK_H//2, x + BLOCK_W//2, y + BLOCK_H//2), \
                    color = 0)

    print(clock.fps())

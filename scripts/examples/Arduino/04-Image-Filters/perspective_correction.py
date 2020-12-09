# Perspective Correction
#
# This example shows off how to use the rotation_corr() to fix perspective
# issues related to how your OpenMV Cam is mounted.

import sensor, image, time

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

# The image will be warped such that the following points become the new:
#
#   (0,   0)
#   (w-1, 0)
#   (w-1, h-1)
#   (0,   h-1)
#
# Try setting the points below to the corners of a quadrilateral
# (in clock-wise order) in the field-of-view. You can get points
# on the image by clicking and dragging on the frame buffer and
# recording the values shown in the histogram widget.

w = sensor.width()
h = sensor.height()

TARGET_POINTS = [(0,   0),   # (x, y) CHANGE ME!
                 (w-1, 0),   # (x, y) CHANGE ME!
                 (w-1, h-1), # (x, y) CHANGE ME!
                 (0,   h-1)] # (x, y) CHANGE ME!

while(True):
    clock.tick()

    img = sensor.snapshot().rotation_corr(corners = TARGET_POINTS)

    print(clock.fps())

# Rotation Correction
#
# This example shows off how to use the rotation_corr() to both correct for
# perspective distortion and then to rotate the new corrected image in 3D
# space aftwards to handle movement.

import sensor
import time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)
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

TARGET_POINTS = [
    (0, 0),  # (x, y) CHANGE ME!
    (w - 1, 0),  # (x, y) CHANGE ME!
    (w - 1, h - 1),  # (x, y) CHANGE ME!
    (0, h - 1),
]  # (x, y) CHANGE ME!

# Degrees per frame to rotation by...
X_ROTATION_DEGREE_RATE = 5
Y_ROTATION_DEGREE_RATE = 0.5
Z_ROTATION_DEGREE_RATE = 0
X_OFFSET = 0
Y_OFFSET = 0

ZOOM_AMOUNT = 1  # Lower zooms out - Higher zooms in.
FOV_WINDOW = 25  # Between 0 and 180. Represents the field-of-view of the scene
# window when rotating the image in 3D space. When closer to
# zero results in lines becoming straighter as the window
# moves away from the image being rotated in 3D space. A large
# value moves the window closer to the image in 3D space which
# results in the more perspective distortion and sometimes
# the image in 3D intersecting the scene window.

x_rotation_counter = 0
y_rotation_counter = 0
z_rotation_counter = 0

while True:
    clock.tick()

    img = sensor.snapshot().rotation_corr(
        x_rotation=x_rotation_counter,
        y_rotation=y_rotation_counter,
        z_rotation=z_rotation_counter,
        x_translation=X_OFFSET,
        y_translation=Y_OFFSET,
        zoom=ZOOM_AMOUNT,
        fov=FOV_WINDOW,
        corners=TARGET_POINTS,
    )

    x_rotation_counter += X_ROTATION_DEGREE_RATE
    y_rotation_counter += Y_ROTATION_DEGREE_RATE
    z_rotation_counter += Z_ROTATION_DEGREE_RATE

    print(clock.fps())

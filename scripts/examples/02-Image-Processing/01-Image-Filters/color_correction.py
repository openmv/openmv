# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Color Correction Example
#
# This example shows off using the color correction matrix multiplication
# method to apply generic matrix multiplications to images.
#
# In the example below we are going to:
#     1. Convert the image to YUV from RGB.
#     2. Apply a rotation matrix to the UV components to cause a Hue Shift.
#     3. Apply a scaling value to the UV components to cause a Saturation Shift.
#     4. Convert the image back from YUV to RGB.
#
# However, instead of applying these 4 steps to the image one a time we can apply
# them to each other before hand to produce a 3x3 matrix suitable for feeding to
# the color correction matrix method.
#
# By converting the color space to YUV this separates the "value" of pixels from
# their "hue" and "saturation". The "hue" then is just the rotation angle given
# the U/V components and the "saturation" is their magnitude.
#
# |Y|   |    0.299,     0.587,     0.114|   |R|
# |U| = |-0.168736, -0.331264,       0.5| * |G|
# |V|   |      0.5, -0.418688, -0.081312|   |B|
#
# |Y_rot|   |1,           0,            0|   |Y|
# |U_rot| = |0, math.cos(a), -math.sin(a)| * |U|
# |V_rot|   |0, math.sin(a),  math.cos(a)|   |V|
#
# |Y_rot_scaled|   |1, 0,  0|   |Y|
# |U_rot_scaled| = |0, s,  0| * |U|
# |V_rot_scaled|   |0, 0,  s|   |V|
#
# |R_rot_scaled|          |    0.299,     0.587,     0.114|   |Y_rot_scaled|
# |R_rot_scaled| = INVERSE|-0.168736, -0.331264,       0.5| * |U_rot_scaled|
# |R_rot_scaled|          |      0.5, -0.418688, -0.081312|   |V_rot_scaled|
#
# Note: The ccm() method can accept 3x3 and 3x4 matrices. 3x4 matrices are for
# if you want an offset to by applied. In this case things look like:
#
# |Y|   |    0.299,     0.587,     0.114,  y_offset|   |R|
# |U| = |-0.168736, -0.331264,       0.5,  u_offset| * |G|
# |V|   |      0.5, -0.418688, -0.081312,  v_offset|   |B|
#                                                      |1|
#
# Keep in mind that the CCM method is just doing:
#
# |R'|                |R|      |R'|                |R|
# |G'| = 3x3 Matrix * |G|  or  |G'| = 3x4 Matrix * |G|
# |B'|                |B|      |B'|                |B|
#                                                  |1|
#
# If you are creating intermediate values using matrix math you need to end
# up back at RGB values for the final matrix you pass to CCM().
#
# Finally, you are free to do the matrix formation using 4x4 matrices. The last
# row will be ignored if you pass a 4x4 matrix (e.g. it will be treated as 3x4).

from ulab import numpy as np
import sensor
import time
import math

# Set to 0 for a grayscale image. Set above 1.0 to pump-up the saturation.
UV_SCALE = 1.0

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)

# These are the standard coefficents for converting RGB to YUV.
rgb2yuv = np.array([[    0.299,     0.587,     0.114], # noqa
                    [-0.168736, -0.331264,       0.5], # noqa
                    [      0.5, -0.418688, -0.081312]], dtype=np.float) # noqa

# Now get the inverse so we can get back to RGB from YUV.
yuv2rgb = np.linalg.inv(rgb2yuv)

clock = time.clock()

# r will be the angle by which we rotate the colors on the UV plane by.
r = 0

while True:
    clock.tick()

    # Increment in a loop.
    r = (r + 1) % 360
    a = math.radians(r)

    # This is a rotation matrix which we will apply on the UV components of YUV values.
    # https://en.wikipedia.org/wiki/Rotation_matrix
    rot = np.array([[1,           0,            0], # noqa
                    [0, math.cos(a), -math.sin(a)], # noqa
                    [0, math.sin(a),  math.cos(a)]], dtype=np.float) # noqa

    # This is the scale matrix
    scale = np.array([[1,        0, 0], # noqa
                      [0, UV_SCALE, 0], # noqa
                      [0,        0, UV_SCALE]], dtype=np.float) # noqa

    # Now compute the final matrix using matrix multiplication.
    m = np.dot(yuv2rgb, np.dot(scale, np.dot(rot, rgb2yuv)))

    # Apply the color transformation (m.flatten().tolist() also works)
    img = sensor.snapshot().ccm(m.tolist())

    print(clock.fps())

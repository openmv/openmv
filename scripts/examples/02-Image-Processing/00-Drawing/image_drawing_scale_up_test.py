# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Image Scaling Up Drawing Test
#
# This script tests the performance and quality of the draw_image()
# method which can perform nearest neighbor, bilinear, bicubic, and
# area scaling along with color channel extraction, alpha blending,
# color palette application, and alpha palette application.

# DISABLE THE FRAME BUFFER TO SEE THE REAL FPS

import csi
import image
import time

csi0 = csi.CSI()
hint = 0  # image.BILINEAR image.BICUBIC

bounce_div = 32

big_img = image.Image(128, 128, csi.RGB565, copy_to_fb=True)
# big_img.to_grayscale()
# big_img.to_bitmap()

small_img = image.Image(4, 4, csi.RGB565)
small_img.set_pixel(0, 0, (0, 0, 127))
small_img.set_pixel(1, 0, (47, 255, 199))
small_img.set_pixel(2, 0, (0, 188, 255))
small_img.set_pixel(3, 0, (0, 0, 127))
small_img.set_pixel(0, 1, (0, 176, 255))
small_img.set_pixel(1, 1, (222, 0, 0))
small_img.set_pixel(2, 1, (50, 255, 195))
small_img.set_pixel(3, 1, (86, 255, 160))
small_img.set_pixel(0, 2, (255, 211, 0))
small_img.set_pixel(1, 2, (83, 255, 163))
small_img.set_pixel(2, 2, (255, 211, 0))
small_img.set_pixel(3, 2, (0, 80, 255))
small_img.set_pixel(0, 3, (255, 118, 0))
small_img.set_pixel(1, 3, (127, 0, 0))
small_img.set_pixel(2, 3, (0, 144, 255))
small_img.set_pixel(3, 3, (50, 255, 195))
# small_img.to_grayscale()
# small_img.to_bitmap()

x_bounce = 0
x_bounce_toggle = 0

y_bounce = 0
y_bounce_toggle = 0

clock = time.clock()
while True:
    clock.tick()

    big_img.clear()
    big_img.draw_image(
        small_img,
        x_bounce // bounce_div,
        y_bounce // bounce_div,
        x_scale=32,
        y_scale=32,
        hint=hint,
    )
    csi0.flush()

    x_bounce += x_bounce_toggle
    if abs(x_bounce // bounce_div) >= (big_img.width() * 1.1):
        x_bounce_toggle = -x_bounce_toggle

    y_bounce += y_bounce_toggle
    if abs(y_bounce // bounce_div) >= (big_img.height() * 1.1):
        y_bounce_toggle = -y_bounce_toggle

    print(clock.fps())

# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Image Drawing Color Table with Alpha Table Test
#
# This script tests the performance and quality of the draw_image()
# method which can perform nearest neighbor, bilinear, bicubic, and
# area scaling along with color channel extraction, alpha blending,
# color palette application, and alpha palette application.

import sensor
import image
import time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)

hint = image.BICUBIC  # image.BILINEAR image.BICUBIC

# RGB channel extraction is done after scaling normally, this
# may produce false colors. Set this flag to do it before.
#
hint |= 0  # image.EXTRACT_RGB_CHANNEL_FIRST

# Color table application is done after scaling normally, this
# may produce false colors. Set this flag to do it before.
#
hint |= 0  # image.APPLY_COLOR_PALETTE_FIRST

small_img = image.Image(4, 4, sensor.RGB565)
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

big_img = image.Image(128, 128, sensor.RGB565)
big_img.draw_image(small_img, 0, 0, x_scale=32, y_scale=32, hint=hint)
# big_img.to_grayscale()
# big_img.to_bitmap()

alpha_lut = image.Image(256, 1, sensor.GRAYSCALE)
for i in range(256):
    alpha_lut.set_pixel(i, 0, 255 if i > 127 else 0)

alpha_div = 1
alpha_value = 0
alpha_step = 2

x_bounce = sensor.width() // 2
x_bounce_toggle = 1

y_bounce = sensor.height() // 2
y_bounce_toggle = 1

clock = time.clock()
while True:
    clock.tick()

    img = sensor.snapshot()
    # img.to_grayscale()
    # img.to_bitmap()
    img.draw_image(
        big_img,
        x_bounce,
        y_bounce,
        rgb_channel=-1,
        alpha=alpha_value // alpha_div,
        color_palette=sensor.PALETTE_IRONBOW,
        alpha_palette=alpha_lut,
        hint=hint | image.CENTER,
    )

    x_bounce += x_bounce_toggle
    if abs(x_bounce - (img.width() // 2)) >= (img.width() // 2):
        x_bounce_toggle = -x_bounce_toggle

    y_bounce += y_bounce_toggle
    if abs(y_bounce - (img.height() // 2)) >= (img.height() // 2):
        y_bounce_toggle = -y_bounce_toggle

    alpha_value += alpha_step
    if not alpha_value or alpha_value // alpha_div == 256:
        alpha_step = -alpha_step

    print(clock.fps())

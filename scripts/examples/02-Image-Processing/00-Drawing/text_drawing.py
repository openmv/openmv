# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Text Drawing
#
# This example shows off drawing text on the OpenMV Cam.

import csi
import image
import time
from random import randint

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)  # or GRAYSCALE...
csi0.framesize(csi.QVGA)  # or QQVGA...
csi0.snapshot(time=2000)

# draw_string() uses the built-in TrueType font (Roboto Mono, 10 px) by default. Any
# TrueType font in ROMFS or on the filesystem can be loaded at a pixel size with
# image.Font(). Boards with a large ROMFS also have the proportional Roboto family,
# e.g. image.Font("/rom/Roboto-Bold.ttf", 24).
font = image.Font("/rom/RobotoMono-Regular.ttf", 24)

clock = time.clock()

while True:
    clock.tick()

    img = csi0.snapshot()

    for i in range(10):
        x = randint(0, 2 * img.width()) - img.width() // 2
        y = randint(0, 2 * img.height()) - img.height() // 2

        r = randint(0, 127) + 128
        g = randint(0, 127) + 128
        b = randint(0, 127) + 128

        # Text is anti-aliased and UTF-8. scale multiplies the font size, alpha blends
        # the text into the image and hint rotates/mirrors it (e.g. image.ROTATE_90).
        img.draw_string((x, y), "Hello World!", color=(r, g, b), font=font, scale=1, alpha=255, hint=0)

    # The width, height and baseline of a string can be measured before drawing it.
    w, h, baseline = font.measure("FPS %.1f" % clock.fps())
    img.draw_rectangle((0, 0, w, h), color=(0, 0, 0), fill=True)
    img.draw_string((0, 0), "FPS %.1f" % clock.fps(), color=(255, 255, 255), font=font)

    print(clock.fps())

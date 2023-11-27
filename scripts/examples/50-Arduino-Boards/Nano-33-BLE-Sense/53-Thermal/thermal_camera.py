# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Thermal Camera Demo
#
# This example shows off how to overlay a heatmap onto your OpenMV Cam's
# live video output from the main camera.

import image
import time
import fir

IMAGE_SCALE = 5  # Scale image to 5x.
drawing_hint = image.BICUBIC  # or image.BILINEAR or 0 (nearest neighbor)

# Initialize the thermal sensor
fir.init()
w = fir.width()
h = fir.height()

if fir.type() == fir.FIR_AMG8833:
    IMAGE_SCALE = IMAGE_SCALE * 2

# FPS clock
clock = time.clock()

while True:
    clock.tick()

    try:
        img = fir.snapshot(
            x_scale=IMAGE_SCALE,
            y_scale=IMAGE_SCALE,
            color_palette=image.PALETTE_IRONBOW,
            hint=drawing_hint,
            copy_to_fb=True,
        )
    except OSError:
        continue

    # Print FPS.
    print(clock.fps())

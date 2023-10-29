# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Thermal Camera Demo
#
# This example shows how to use common low-res FIR sensors (like MLX or AMG).
# NOTE: Only the AMG8833 is currently enabled for NANO RP2040.

import image
import time
import fir

IMAGE_SCALE = 5  # Higher scaling uses more memory.
drawing_hint = image.BICUBIC  # or image.BILINEAR or 0 (nearest neighbor)

# Initialize the thermal sensor
fir.init()  # Auto-detects the connected sensor.
w = fir.width() * IMAGE_SCALE
h = fir.height() * IMAGE_SCALE

# FPS clock
clock = time.clock()

while True:
    clock.tick()

    try:
        img = fir.snapshot(
            x_size=w,
            y_size=h,
            color_palette=fir.PALETTE_IRONBOW,
            hint=drawing_hint,
            copy_to_fb=True,
        )
    except OSError:
        continue

    # Print FPS.
    print(clock.fps())

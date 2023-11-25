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

IMAGE_SCALE = 5  # Scale image to 5x.
drawing_hint = image.BICUBIC  # or image.BILINEAR or 0 (nearest neighbor)

# Initialize the thermal sensor
fir.init()  # Auto-detects the connected sensor.

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

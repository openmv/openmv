# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Time of Flight overlay Demo
#
# This example shows off how to overlay a depth map onto
# OpenMV Cam's live video output from the main camera.

import image
import time
import tof

IMAGE_SCALE = 10  # Higher scaling uses more memory.
drawing_hint = image.BILINEAR  # or image.BILINEAR or 0 (nearest neighbor)

# Initialize the ToF sensor
tof.init()  # Auto-detects the connected sensor.
w = tof.width() * IMAGE_SCALE
h = tof.height() * IMAGE_SCALE

# FPS clock
clock = time.clock()

while True:
    clock.tick()

    try:
        img = tof.snapshot(
            x_size=w,
            y_size=h,
            color_palette=tof.PALETTE_IRONBOW,
            hint=drawing_hint,
            copy_to_fb=True,
            scale=(0, 4000),
        )
    except OSError:
        continue

    # Print FPS.
    print(clock.fps())

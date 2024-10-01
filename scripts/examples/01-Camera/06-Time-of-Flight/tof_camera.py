# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Time of flight camera Demo.

import image
import time
import tof

IMAGE_SCALE = 10  # Scale image to 10x.

# Initialize the ToF sensor
tof.init()  # Auto-detects the connected sensor.

# FPS clock
clock = time.clock()

while True:
    clock.tick()

    try:
        img = tof.snapshot(
            vflip=True,
            hmirror=True,
            x_scale=IMAGE_SCALE,
            y_scale=IMAGE_SCALE,
            hint=image.BILINEAR,
            scale=(0, 4000),
            copy_to_fb=True,
            color_palette=tof.PALETTE_IRONBOW,
        )
        img.flush()
    except RuntimeError as e:
        tof.reset()

    print(clock.fps())

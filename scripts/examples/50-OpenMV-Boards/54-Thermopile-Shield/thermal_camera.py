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

drawing_hint = image.BICUBIC  # or image.BILINEAR or 0 (nearest neighbor)

# Initialize the thermal sensor
fir.init()
w = fir.width()
h = fir.height()

if fir.type() == fir.FIR_MLX90621:
    w = w * 10
    h = h * 10
elif fir.type() == fir.FIR_MLX90640:
    w = w * 10
    h = h * 10
elif fir.type() == fir.FIR_MLX90641:
    w = w * 10
    h = h * 10
elif fir.type() == fir.FIR_AMG8833:
    w = w * 20
    h = h * 20

# FPS clock
clock = time.clock()

while True:
    clock.tick()

    try:
        img = fir.snapshot(
            x_size=w,
            y_size=h,
            color_palette=image.PALETTE_IRONBOW,
            hint=drawing_hint,
            copy_to_fb=True,
        )
    except OSError:
        continue

    # Print FPS.
    print(clock.fps())

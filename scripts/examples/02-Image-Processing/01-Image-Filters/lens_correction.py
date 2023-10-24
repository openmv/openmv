# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Lens Correction
#
# This example shows off how to use the lens correction method to fix lens
# distortion in an image. You need to do this for qrcode / barcode / data matrix
# detection. Increase the strength below until lines are straight in the view.
# Zoom in (higher) or out (lower) until you see enough of the image.

import sensor
import time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)
clock = time.clock()

while True:
    clock.tick()

    img = sensor.snapshot().lens_corr(strength=1.8, zoom=1.0)

    print(clock.fps())

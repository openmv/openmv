# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Vertical Flip - Horizontal Mirror - Transpose
#
# This example shows off how to vertically flip, horizontally mirror, or
# transpose an image. Note that:
#
# vflip=False, hmirror=False, transpose=False -> 0 degree rotation
# vflip=True,  hmirror=False, transpose=True  -> 90 degree rotation
# vflip=True,  hmirror=True,  transpose=False -> 180 degree rotation
# vflip=False, hmirror=True,  transpose=True  -> 270 degree rotation

import sensor
import time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)
clock = time.clock()

ticks = time.ticks_ms()
counter = 0

while True:
    clock.tick()

    img = sensor.snapshot().replace(
        vflip=(counter // 2) % 2,
        hmirror=(counter // 4) % 2,
        transpose=(counter // 8) % 2,
    )

    if time.ticks_diff(time.ticks_ms(), ticks) > 1000:
        ticks = time.ticks_ms()
        counter += 1

    print(clock.fps())

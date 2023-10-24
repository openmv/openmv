# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Cartoon Filter
#
# This example shows off a simple cartoon filter on images. The cartoon
# filter works by joining similar pixel areas of an image and replacing
# the pixels in those areas with the area mean.

import sensor
import time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)  # or GRAYSCALE...
sensor.set_framesize(sensor.QVGA)  # or QQVGA...
sensor.skip_frames(time=2000)
clock = time.clock()

while True:
    clock.tick()

    # seed_threshold controls the maximum area growth of a colored
    # region. Making this larger will merge more pixels.

    # floating_threshold controls the maximum pixel-to-pixel difference
    # when growing a region. Settings this very high will quickly combine
    # all pixels in the image. You should keep this small.

    # cartoon() will grow regions while both thresholds are satisfied...

    img = sensor.snapshot().cartoon(seed_threshold=0.05, floating_thresholds=0.05)

    print(clock.fps())

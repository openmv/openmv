#!/usr/bin/env python3
# This work is licensed under the MIT license.
# Copyright (c) 2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Unix port smoke test. Exercises the core image processing entry
# points so a broken build is caught before the full unit-test suite.

import image

img = image.Image(320, 240, image.RGB565)
assert img.width() == 320 and img.height() == 240, "image size mismatch"

img.draw_rectangle((10, 10, 50, 50), color=(255, 0, 0))
img.draw_circle((160, 120, 40), color=(0, 255, 0))

lab = image.rgb_to_lab((120, 200, 120))
assert isinstance(lab, tuple) and len(lab) == 3, "rgb_to_lab return shape"

img.to_grayscale()
assert img.format() == image.GRAYSCALE, "to_grayscale format"

print("smoke test passed")

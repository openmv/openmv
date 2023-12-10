# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Copy image to framebuffer.
#
# This example shows how to load and display an image.

import image
import time

# Load image
img = image.Image("/example.bmp", copy_to_fb=True)

# Add a small delay to allow the IDE to read the loaded image.
time.sleep_ms(1000)

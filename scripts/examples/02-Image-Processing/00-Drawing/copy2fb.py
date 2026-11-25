# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Copy image to framebuffer.
#
# This example shows how to load and display an image.

import image

# Load image
img = image.Image("example.bmp", copy_to_fb=True)

# Show the image in the IDE; flush() blocks until the IDE has picked up
# the frame (or a timeout, default 1000 ms, expires).
img.flush()

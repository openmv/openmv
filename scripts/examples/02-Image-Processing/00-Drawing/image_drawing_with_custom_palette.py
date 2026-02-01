# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Draw Image Example with custom color palette
#
# This example shows off how to draw images in the frame buffer with a custom generated color palette.

import csi
import image
import time

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)  # or GRAYSCALE...
csi0.framesize(csi.QQVGA)  # or QQVGA...
csi0.snapshot(time=2000)

clock = time.clock()

# the color palette is actually an image, this allows you to use image ops to create palettes
# the image must have 256 entries i.e. 256x1, 64x4, 16x16 and have the format rgb565

# Initialise palette source colors into an image
palette_source_colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 0, 255)]
palette_source_color_image = image.Image(len(palette_source_colors), 1, csi.RGB565)
for i, color in enumerate(palette_source_colors):
    palette_source_color_image[i] = color

# Scale the image to palette width and smooth them
palette = image.Image(256, 1, csi.RGB565)
palette.draw_image(
    palette_source_color_image,
    0,
    0,
    x_scale=palette.width() / palette_source_color_image.width(),
)
palette.mean(int(palette.width() / palette_source_color_image.width() / 2))

while True:
    clock.tick()

    img = csi0.snapshot()
    # Get a copy of grayscale image before converting to color
    img_copy = img.copy()

    img.to_rgb565()

    palette_boundary_inset = int(csi0.width() / 40)
    palette_scale_x = (csi0.width() - palette_boundary_inset * 2) / palette.width()

    img.draw_image(img_copy, 0, 0, color_palette=palette)
    img.draw_image(
        palette,
        palette_boundary_inset,
        palette_boundary_inset,
        x_scale=palette_scale_x,
        y_scale=8,
    )
    img.draw_rectangle(
        palette_boundary_inset,
        palette_boundary_inset,
        int(palette.width() * palette_scale_x),
        8,
        color=(255, 255, 255),
        thickness=1,
    )

    print(clock.fps())

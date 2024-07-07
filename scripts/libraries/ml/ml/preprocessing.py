# This file is part of the OpenMV project.
#
# Copyright (c) 2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.

import image


class Normalization:
    def __init__(
        self,
        image=None,
        scale=(0.0, 1.0),
        mean=(0.0, 0.0, 0.0),
        stdev=(1.0, 1.0, 1.0),
        roi=None,
    ):
        self.image = image
        self.scale = scale
        self.mean = mean
        self.stdev = stdev
        self.roi = roi

    def __call__(self, *args):
        if len(args) == 1:
            img = args[0]
            if not isinstance(img, image.Image):
                raise ValueError("Expected an image input")
            if self.roi is None:
                self.roi = (0, 0, img.width(), img.height())
            return Normalization(img, self.scale, self.mean, self.stdev, self.roi)
        buffer, shape, dtype = args
        # Create an image using the input tensor as buffer.
        img = image.Image(shape[2], shape[1], self.image.format(), buffer=buffer)
        # Copy and scale (if needed) the input image to the input buffer.
        hints = image.BILINEAR | image.CENTER | image.SCALE_ASPECT_EXPAND | image.BLACK_BACKGROUND
        img.draw_image(self.image, 0, 0, roi=self.roi, hint=hints)
        # Scale and convert the image to input tensor data.
        img.unpack(buffer, dtype, scale=self.scale, mean=self.mean, stdev=self.stdev)

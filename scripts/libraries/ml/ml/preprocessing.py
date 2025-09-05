# Copyright (C) 2024 OpenMV, LLC.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Any redistribution, use, or modification in source or binary form
#    is done solely for personal benefit and not for any commercial
#    purpose or for monetary gain. For commercial licensing options,
#    please contact openmv@openmv.io
#
# THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
# OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
import image
from ulab import numpy as np


class Normalization:
    def __init__(
        self,
        scale=(0.0, 1.0),
        mean=(0.0, 0.0, 0.0),
        stdev=(1.0, 1.0, 1.0),
        roi=None,
    ):
        self.scale = scale
        self.mean = mean
        self.stdev = stdev
        self.roi = roi
        self._image = None

    def __call__(self, *args):
        if len(args) == 1:
            img = args[0]
            if not isinstance(img, image.Image):
                raise ValueError("Expected an image input")
            if self.roi is None:
                self.roi = (0, 0, img.width(), img.height())
            n = Normalization(self.scale, self.mean, self.stdev, self.roi)
            n._image = img
            return n

        buffer, shape, dtype = args

        # Create an image using the input tensor as buffer.
        if len(shape) != 4:
            raise ValueError("Expected input tensor with shape: (1, H, W, C)")
        b, h, w, c = shape
        if b != 1:
            raise ValueError("Expected batches to be 1")
        if c != 1 and c != 3:
            raise ValueError("Expected channels to be 1 or 3")

        # Place the image buffer at the end of the input buffer so we can convert it in-place.
        pixfmt = image.GRAYSCALE if c == 1 else image.RGB565
        offset = len(buffer) - (w * h * (1 if c == 1 else 2))
        img = image.Image(w, h, pixfmt, buffer=memoryview(buffer)[offset:])

        # Copy and scale (if needed) the input image to the input buffer.
        hints = image.BILINEAR | image.CENTER | image.SCALE_ASPECT_EXPAND | image.BLACK_BACKGROUND
        img.draw_image(self._image, 0, 0, roi=self.roi, hint=hints)

        # Convert the image in-place into an ndarray input tensor.
        array = img.to_ndarray(dtype, buffer=buffer)

        # Normalize the input tensor.
        if dtype == ord('f'):
            fscale = (self.scale[1] - self.scale[0]) / 255.0
            fadd = self.scale[0]

            def grayscale(x):
                return (x[0] * 0.299) + (x[1] * 0.587) + (x[2] * 0.114)

            if c == 1:
                fadd = (fadd - grayscale(self.mean)) / grayscale(self.stdev)
                fscale = fscale / grayscale(self.stdev)
            else:
                fadd = (fadd - np.array(self.mean)) / np.array(self.stdev)
                fscale = fscale / np.array(self.stdev)

            # Apply normalization in-place (must be done in two steps for ulab).
            array *= fscale
            array += fadd

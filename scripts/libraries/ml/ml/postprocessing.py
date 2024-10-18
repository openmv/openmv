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
import math
import image
from ml.utils import NMS


# FOMO generates an image per class, where each pixel represents the centroid
# of the trained object. These images are processed with `find_blobs()` to
# extract centroids, and `get_stats()` is used to get their scores. Overlapping
# detections are then filtered with NMS and positions are mapped back to the
# original image, and a list of (rect, score) tuples is returned for each class,
# representing detected objects.
class fomo_postprocess:
    def __init__(self, threshold=0.4):
        self.threshold_list = [(math.ceil(threshold * 255), 255)]

    def __call__(self, model, inputs, outputs):
        n, oh, ow, oc = model.output_shape[0]
        nms = NMS(ow, oh, inputs[0].roi)
        for i in range(oc):
            img = image.Image(outputs[0][0, :, :, i] * 255)
            blobs = img.find_blobs(
                self.threshold_list, x_stride=1, area_threshold=1, pixels_threshold=1
            )
            for b in blobs:
                rect = b.rect()
                x, y, w, h = rect
                score = (
                    img.get_statistics(thresholds=self.threshold_list, roi=rect).l_mean() / 255.0
                )
                nms.add_bounding_box(x, y, x + w, y + h, score, i)
        return nms.get_bounding_boxes()

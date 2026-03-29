# Copyright (C) 2025 OpenMV, LLC.
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
from ml.utils import *
from micropython import const
from ulab import numpy as np


_NO_DETECTION = const(())


class Fomo:
    _FOMO_CLASSES = const(1)

    def __init__(self, threshold=0.4, w_scale=1.414214, h_scale=1.414214,
                 nms_threshold=0.1, nms_sigma=0.001):
        self.threshold = threshold
        self.w_scale = w_scale
        self.h_scale = h_scale
        self.nms_threshold = nms_threshold
        self.nms_sigma = nms_sigma

    def __call__(self, model, inputs, outputs):
        ob, oh, ow, oc = model.output_shape[0]
        scale = model.output_scale[0]
        t = quantize(model, self.threshold)

        # Reshape the output to a 2D array
        row_outputs = outputs[0].reshape((oh * ow, oc))

        # Threshold all the scores
        score_indices = row_outputs[:, _FOMO_CLASSES:]
        score_indices = threshold(score_indices, t, scale, find_max=True, find_max_axis=1)
        if not len(score_indices):
            return _NO_DETECTION

        # Get the bounding boxes that have a valid score
        bb = dequantize(model, np.take(row_outputs, score_indices, axis=0))

        # Extract rows and columns
        bb_rows = score_indices // ow
        bb_cols = score_indices % ow

        # Get the score information
        bb_scores = np.max(bb[:, _FOMO_CLASSES:], axis=1)

        # Get the class information
        bb_classes = np.argmax(bb[:, _FOMO_CLASSES:], axis=1) + _FOMO_CLASSES

        # Scale the bounding boxes to have enough integer precision for NMS
        ib, ih, iw, ic = model.input_shape[0]
        x_center = ((bb_cols + 0.5) / ow) * iw
        y_center = ((bb_rows + 0.5) / oh) * ih
        w_rel = np.full(len(bb_cols), self.w_scale / ow) * iw
        h_rel = np.full(len(bb_rows), self.h_scale / oh) * ih

        nms = NMS(iw, ih, inputs[0].roi)
        for i in range(bb.shape[0]):
            nms.add_bounding_box(x_center[i] - (w_rel[i] / 2),
                                 y_center[i] - (h_rel[i] / 2),
                                 x_center[i] + (w_rel[i] / 2),
                                 y_center[i] + (h_rel[i] / 2),
                                 bb_scores[i], bb_classes[i])
        return nms.get_bounding_boxes(threshold=self.nms_threshold, sigma=self.nms_sigma)

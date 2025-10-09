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


class BlazeFace:
    _BLAZEFACE_CX = const(0)
    _BLAZEFACE_CY = const(1)
    _BLAZEFACE_CW = const(2)
    _BLAZEFACE_CH = const(3)
    _BLAZEFACE_KP = const(4)

    def __init__(self, threshold=0.6, anchors=None, nms_threshold=0.1, nms_sigma=0.1):
        self.threshold = threshold
        self.anchors = anchors

        if self.anchors is None:
            self.anchors = np.empty((896, 2))
            idx = 0

            # Generate anchors for 16x16 grid with 2 duplicates and
            # 8x8 grid with 6 duplicates to match the model output size.
            for grid_size, scales in [(16, 2), (8, 6)]:
                for gy in range(grid_size):
                    cy = (gy + 0.5) / grid_size
                    for gx in range(grid_size):
                        cx = (gx + 0.5) / grid_size
                        for _ in range(scales):
                            self.anchors[idx, 0] = cx
                            self.anchors[idx, 1] = cy
                            idx += 1

        self.nms_threshold = nms_threshold
        self.nms_sigma = nms_sigma

    def __call__(self, model, inputs, outputs):
        ib, ih, iw, ic = model.input_shape[0]
        nms = NMS(iw, ih, inputs[0].roi)
        output_len = outputs[0].shape[1]

        self.blazeface_post_process(ih, iw, nms, model, inputs, outputs, 1, 0,
                                    self.threshold, self.anchors[:output_len])

        if output_len < len(self.anchors):
            self.blazeface_post_process(ih, iw, nms, model, inputs, outputs, 2, 3,
                                        self.threshold, self.anchors[output_len:])

        return nms.get_bounding_boxes(threshold=self.nms_threshold, sigma=self.nms_sigma)

    def blazeface_post_process(self, ih, iw, nms, model, inputs, outputs, score_idx, cords_idx, t, anchors):
        s_oh, s_ow, s_oc = model.output_shape[score_idx]
        scale = model.output_scale[score_idx]
        t = quantize(model, logit(t), index=score_idx)

        # Threshold all the scores
        score_row_outputs = outputs[score_idx].reshape((s_oh * s_ow * s_oc))
        score_indices = threshold(score_row_outputs, t, scale)
        if not len(score_indices):
            return _NO_DETECTION

        # Get the score information
        bb_scores = np.take(score_row_outputs, score_indices, axis=0)
        bb_scores = sigmoid(dequantize(model, bb_scores, index=score_idx))

        # Get the bounding boxes that have a valid score
        c_oh, c_ow, c_oc = model.output_shape[cords_idx]
        cords_row_outputs = outputs[cords_idx].reshape((c_oh * c_ow, c_oc))
        bb = dequantize(model, np.take(cords_row_outputs, score_indices, axis=0), index=cords_idx)

        # Get the anchor box information
        bb_a_array = np.take(anchors, score_indices, axis=0)

        # Compute the bounding box information
        ax = bb_a_array[:, _BLAZEFACE_CX]
        ay = bb_a_array[:, _BLAZEFACE_CY]
        x_center = bb[:, _BLAZEFACE_CX] / iw + ax
        y_center = bb[:, _BLAZEFACE_CY] / ih + ay
        w_rel = bb[:, _BLAZEFACE_CW] / iw * 0.5
        h_rel = bb[:, _BLAZEFACE_CH] / ih * 0.5

        # Get the keypoint information
        row_count = bb.shape[0]
        keypoints = np.empty((row_count, (c_oc - _BLAZEFACE_KP) // 2, 2))
        keypoints[:, :, 0] = (bb[:, _BLAZEFACE_KP::2] / iw + ax.reshape((row_count, 1))) * iw
        keypoints[:, :, 1] = (bb[:, _BLAZEFACE_KP + 1::2] / ih + ay.reshape((row_count, 1))) * ih

        # Scale the bounding boxes to have enough integer precision for NMS
        xmin = (x_center - w_rel) * iw
        ymin = (y_center - h_rel) * ih
        xmax = (x_center + w_rel) * iw
        ymax = (y_center + h_rel) * ih

        for i in range(bb.shape[0]):
            nms.add_bounding_box(xmin[i], ymin[i], xmax[i], ymax[i], bb_scores[i], 0, keypoints=keypoints[i])

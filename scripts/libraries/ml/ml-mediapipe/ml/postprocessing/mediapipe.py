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


class mediapipe_detection_postprocess:
    _MEDIAPIPE_CX = const(0)
    _MEDIAPIPE_CY = const(1)
    _MEDIAPIPE_CW = const(2)
    _MEDIAPIPE_CH = const(3)
    _MEDIAPIPE_KP = const(4)

    def __init__(self, threshold=0.6, anchors=None, anchor_grid=None, scores=[], cords=[],
                 nms_threshold=0.1, nms_sigma=0.1):
        self.threshold = threshold
        self.anchors = anchors
        self.scores = scores
        self.cords = cords

        if self.anchors is None:
            anchor_count = sum((g * g) * d for g, d in anchor_grid)
            self.anchors = np.empty((anchor_count, 2))
            idx = 0

            for grid_size, scales in anchor_grid:
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

        self.detection_post_process(ih, iw, nms, model, inputs, outputs, self.scores[0], self.cords[0],
                                    self.threshold, self.anchors[:output_len])

        if output_len < len(self.anchors):
            self.detection_post_process(ih, iw, nms, model, inputs, outputs, self.scores[1], self.cords[1],
                                        self.threshold, self.anchors[output_len:])

        return nms.get_bounding_boxes(threshold=self.nms_threshold, sigma=self.nms_sigma)[0]

    def detection_post_process(self, ih, iw, nms, model, inputs, outputs, score_idx, cords_idx, t, anchors):
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
        ax = bb_a_array[:, _MEDIAPIPE_CX]
        ay = bb_a_array[:, _MEDIAPIPE_CY]
        x_center = bb[:, _MEDIAPIPE_CX] / iw + ax
        y_center = bb[:, _MEDIAPIPE_CY] / ih + ay
        w_rel = bb[:, _MEDIAPIPE_CW] / iw * 0.5
        h_rel = bb[:, _MEDIAPIPE_CH] / ih * 0.5

        # Get the keypoint information
        row_count = bb.shape[0]
        keypoints = np.empty((row_count, (c_oc - _MEDIAPIPE_KP) // 2, 2))
        keypoints[:, :, 0] = (bb[:, _MEDIAPIPE_KP::2] / iw + ax.reshape((row_count, 1))) * iw
        keypoints[:, :, 1] = (bb[:, _MEDIAPIPE_KP + 1::2] / ih + ay.reshape((row_count, 1))) * ih

        # Scale the bounding boxes to have enough integer precision for NMS
        xmin = (x_center - w_rel) * iw
        ymin = (y_center - h_rel) * ih
        xmax = (x_center + w_rel) * iw
        ymax = (y_center + h_rel) * ih

        for i in range(bb.shape[0]):
            nms.add_bounding_box(xmin[i], ymin[i], xmax[i], ymax[i], bb_scores[i], 0, keypoints=keypoints[i])


class BlazeFace(mediapipe_detection_postprocess):
    def __init__(self, threshold=0.6, anchors=None, nms_threshold=0.1, nms_sigma=0.1):
        super().__init__(threshold=threshold, anchors=anchors,
                         anchor_grid=[(16, 2), (8, 6)], scores=[1, 2], cords=[0, 3],
                         nms_threshold=nms_threshold, nms_sigma=nms_sigma)


class BlazePalm(mediapipe_detection_postprocess):
    def __init__(self, threshold=0.6, anchors=None, nms_threshold=0.1, nms_sigma=0.1):
        super().__init__(threshold=threshold, anchors=anchors,
                         anchor_grid=[(24, 2), (12, 6)], scores=[0], cords=[1],
                         nms_threshold=nms_threshold, nms_sigma=nms_sigma)


class HandLandmarks:
    def __init__(self, threshold=0.6, nms_threshold=0.1, nms_sigma=0.1):
        self.threshold = threshold
        self.nms_threshold = nms_threshold
        self.nms_sigma = nms_sigma

    def __call__(self, model, inputs, outputs):
        ib, ih, iw, ic = model.input_shape[0]
        nms = NMS(iw, ih, inputs[0].roi)

        score = outputs[2][0, 0]
        if score < self.threshold:
            return _NO_DETECTION

        cords = outputs[3][0, :]

        # Get the keypoint information
        keypoints = np.empty((len(cords) // 3, 3))
        keypoints[:, 0] = cords[0::3]
        keypoints[:, 1] = cords[1::3]
        keypoints[:, 2] = cords[2::3]

        # Get bounding box information
        xmin = np.min(keypoints[:, 0])
        ymin = np.min(keypoints[:, 1])
        xmax = np.max(keypoints[:, 0])
        ymax = np.max(keypoints[:, 1])

        left_right = outputs[0][0, 0] > 0.5

        nms.add_bounding_box(xmin, ymin, xmax, ymax, score, left_right, keypoints=keypoints)
        return nms.get_bounding_boxes(threshold=self.nms_threshold, sigma=self.nms_sigma)


class FaceLandmarks:
    def __init__(self, threshold=0.6, nms_threshold=0.1, nms_sigma=0.1):
        self.threshold = threshold
        self.nms_threshold = nms_threshold
        self.nms_sigma = nms_sigma

    def __call__(self, model, inputs, outputs):
        ib, ih, iw, ic = model.input_shape[0]
        nms = NMS(iw, ih, inputs[0].roi)

        score = sigmoid(outputs[1][0, 0, 0, 0])
        if score < self.threshold:
            return _NO_DETECTION

        cords = outputs[0][0, 0, 0, :]

        # Get the keypoint information
        keypoints = np.empty((len(cords) // 3, 3))
        keypoints[:, 0] = cords[0::3]
        keypoints[:, 1] = cords[1::3]
        keypoints[:, 2] = cords[2::3]

        # Get bounding box information
        xmin = np.min(keypoints[:, 0])
        ymin = np.min(keypoints[:, 1])
        xmax = np.max(keypoints[:, 0])
        ymax = np.max(keypoints[:, 1])

        nms.add_bounding_box(xmin, ymin, xmax, ymax, score, 0, keypoints=keypoints)
        return nms.get_bounding_boxes(threshold=self.nms_threshold, sigma=self.nms_sigma)[0]

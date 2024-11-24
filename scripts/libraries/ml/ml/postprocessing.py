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
from micropython import const
from ulab import numpy as np


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


class yolo_v2_postprocess:
    _YOLO_V2_TX = const(0)
    _YOLO_V2_TY = const(1)
    _YOLO_V2_TW = const(2)
    _YOLO_V2_TH = const(3)
    _YOLO_V2_SCORE = const(4)
    _YOLO_V2_CLASSES = const(5)

    def __init__(self, score_threshold=0.6, anchors=None):
        self.score_threshold = score_threshold
        if anchors is not None:
            self.anchors = anchors
        else:
            self.anchors = np.array([[0.98830, 3.36060],
                                     [2.11940, 5.37590],
                                     [3.05200, 9.13360],
                                     [5.55170, 9.30660],
                                     [9.72600, 11.1422]], dtype=np.float)
        self.anchors_len = len(self.anchors)

    def __call__(self, model, inputs, outputs):
        ob, oh, ow, oc = model.output_shape[0]
        class_count = (oc // self.anchors_len) - _YOLO_V2_CLASSES

        def sigmoid(x):
            return 1.0 / (1.0 + np.exp(-x))

        def mod(a, b):
            return a - (b * (a // b))

        def softmax(x):
            e_x = np.exp(x - np.max(x))
            return e_x / np.sum(e_x)

        # Reshape the output to a 2D array
        colum_outputs = outputs[0].reshape((oh * ow * self.anchors_len,
                                            _YOLO_V2_CLASSES + class_count))

        # Threshold all the scores
        score_indices = sigmoid(colum_outputs[:, _YOLO_V2_SCORE])
        score_indices = np.nonzero(score_indices > self.score_threshold)
        if isinstance(score_indices, tuple):
            score_indices = score_indices[0]
        if not len(score_indices):
            return []

        # Get the bounding boxes that have a valid score
        bb = np.take(colum_outputs, score_indices, axis=0)

        # Extract rows, columns, and anchor indices
        bb_rows = score_indices // (ow * self.anchors_len)
        bb_cols = mod(score_indices // self.anchors_len, ow)
        bb_anchors = mod(score_indices, self.anchors_len)

        # Get the anchor box information
        bb_a_array = [self.anchors[i] for i in bb_anchors.tolist()]
        bb_a_array = np.array(bb_a_array)

        # Get the score information
        bb_scores = sigmoid(bb[:, _YOLO_V2_SCORE])

        # Get the class information
        bb_classes = []
        for i in range(len(score_indices)):
            s = softmax(bb[i, _YOLO_V2_CLASSES:])
            bb_classes.append(np.argmax(s))
        bb_classes = np.array(bb_classes, dtype=np.uint16)

        # Compute the bounding box information
        x_center = (bb_cols + sigmoid(bb[:, _YOLO_V2_TX])) / ow
        y_center = (bb_rows + sigmoid(bb[:, _YOLO_V2_TY])) / oh
        w_rel = (bb_a_array[:, 0] * np.exp(bb[:, _YOLO_V2_TW])) / ow
        h_rel = (bb_a_array[:, 1] * np.exp(bb[:, _YOLO_V2_TH])) / oh

        # Scale the bounding boxes to have enough integer precision for NMS
        ib, ih, iw, ic = model.input_shape[0]
        x_center = x_center * iw
        y_center = y_center * ih
        w_rel = w_rel * iw
        h_rel = h_rel * ih

        nms = NMS(iw, ih, inputs[0].roi)
        for i in range(len(bb)):
            nms.add_bounding_box(x_center[i] - (w_rel[i] / 2),
                                 y_center[i] - (h_rel[i] / 2),
                                 x_center[i] + (w_rel[i] / 2),
                                 y_center[i] + (h_rel[i] / 2),
                                 bb_scores[i], bb_classes[i])
        return nms.get_bounding_boxes()

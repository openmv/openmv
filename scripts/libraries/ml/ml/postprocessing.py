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
import ml.utils
from micropython import const
from ulab import numpy as np


_NO_DETECTION = const(())
_FOMO_CLASSES = const(1)
_YOLO_V2_TX = const(0)
_YOLO_V2_TY = const(1)
_YOLO_V2_TW = const(2)
_YOLO_V2_TH = const(3)
_YOLO_V2_SCORE = const(4)
_YOLO_V2_CLASSES = const(5)
_YOLO_V5_CX = const(0)
_YOLO_V5_CY = const(1)
_YOLO_V5_CW = const(2)
_YOLO_V5_CH = const(3)
_YOLO_V5_SCORE = const(4)
_YOLO_V5_CLASSES = const(5)
_YOLO_V8_CX = const(0)
_YOLO_V8_CY = const(1)
_YOLO_V8_CW = const(2)
_YOLO_V8_CH = const(3)
_YOLO_V8_CLASSES = const(4)


def dequantize(value, dtype, zero_point, scale):
    if dtype == 'f':
        return value
    return (value - zero_point) * scale


def sigmoid(x):
    return 1.0 / (1.0 + np.exp(-x))


def logit(x):
    return np.log(x / (1.0 - x))


def mod(a, b):
    return a - (b * (a // b))


def softmax(x):
    e_x = np.exp(x - np.max(x, axis=1, keepdims=True))
    return e_x / np.sum(e_x, axis=1, keepdims=True)


def threshold(scores, threshold, scale, find_max=False, find_max_axis=1):
    if scale > 0:
        if find_max:
            scores = np.max(scores, axis=find_max_axis)
        return np.nonzero(scores > threshold)[0]
    else:
        if find_max:
            scores = np.min(scores, axis=find_max_axis)
        return np.nonzero(scores < threshold)[0]


class fomo_postprocess:
    def __init__(self, threshold=0.4, w_scale=1.414214, h_scale=1.414214,
                 nms_threshold=0.1, nms_sigma=0.001,
                 scale_aspect=ml.utils.NMS_SCALE_ASPECT_KEEP):
        self.threshold = threshold
        self.w_scale = w_scale
        self.h_scale = h_scale
        self.nms_threshold = nms_threshold
        self.nms_sigma = nms_sigma
        self.scale_aspect = scale_aspect

    def __call__(self, model, inputs, outputs):
        ob, oh, ow, oc = model.output_shape[0]
        s = model.output_scale[0]
        zp = model.output_zero_point[0]
        dt = model.output_dtype[0]
        t = (self.threshold / s) + zp

        # Reshape the output to a 2D array
        row_outputs = outputs[0].reshape((oh * ow, oc))

        # Threshold all the scores
        score_indices = row_outputs[:, _FOMO_CLASSES:]
        score_indices = threshold(score_indices, t, s, find_max=True, find_max_axis=1)
        if not len(score_indices):
            return _NO_DETECTION

        # Get the bounding boxes that have a valid score
        bb = dequantize(np.take(row_outputs, score_indices, axis=0), dt, zp, s)

        # Extract rows and columns
        bb_rows = score_indices // ow
        bb_cols = mod(score_indices, ow)

        # Get the score information
        bb_scores = np.max(bb[:, _FOMO_CLASSES:], axis=1)

        # Get the class information
        bb_classes = np.argmax(bb[:, _FOMO_CLASSES:], axis=1) + _FOMO_CLASSES

        # Compute the bounding box information
        x_center = (bb_cols + 0.5) / ow
        y_center = (bb_rows + 0.5) / oh
        w_rel = np.full(len(bb_cols), self.w_scale / ow)
        h_rel = np.full(len(bb_rows), self.h_scale / oh)

        return ml.utils.box_nms(x_center, y_center, w_rel, h_rel, bb_scores,
                                bb_classes, model.input_shape[0][1:3], inputs[0].roi,
                                self.nms_threshold, self.nms_sigma, self.scale_aspect)


# This is a lightweight version of the tiny yolo v2 object detection algorithm.
# It was optimized to work well on embedded devices with limited computational resources.
class yolo_v2_postprocess:
    def __init__(self, threshold=0.6, anchors=None, nms_threshold=0.1, nms_sigma=0.1,
                 scale_aspect=ml.utils.NMS_SCALE_ASPECT_KEEP):
        self.threshold = threshold
        self.anchors = anchors
        if self.anchors is None:
            self.anchors = np.array([[0.98830, 3.36060],
                                     [2.11940, 5.37590],
                                     [3.05200, 9.13360],
                                     [5.55170, 9.30660],
                                     [9.72600, 11.1422]])
        self.anchors_len = len(self.anchors)
        self.nms_threshold = nms_threshold
        self.nms_sigma = nms_sigma
        self.scale_aspect = scale_aspect

    def __call__(self, model, inputs, outputs):
        ob, oh, ow, oc = model.output_shape[0]
        s = model.output_scale[0]
        zp = model.output_zero_point[0]
        dt = model.output_dtype[0]
        t = (logit(self.threshold) / s) + zp
        class_count = (oc // self.anchors_len) - _YOLO_V2_CLASSES

        # Reshape the output to a 2D array
        row_outputs = outputs[0].reshape((oh * ow * self.anchors_len,
                                          _YOLO_V2_CLASSES + class_count))

        # Threshold all the scores
        score_indices = row_outputs[:, _YOLO_V2_SCORE]
        score_indices = threshold(score_indices, t, s)
        if not len(score_indices):
            return _NO_DETECTION

        # Get the bounding boxes that have a valid score
        bb = dequantize(np.take(row_outputs, score_indices, axis=0), dt, zp, s)

        # Extract rows, columns, and anchor indices
        bb_rows = score_indices // (ow * self.anchors_len)
        bb_cols = mod(score_indices // self.anchors_len, ow)
        bb_anchors = mod(score_indices, self.anchors_len)

        # Get the anchor box information
        bb_a_array = np.take(self.anchors, bb_anchors, axis=0)

        # Get the score information
        bb_scores = sigmoid(bb[:, _YOLO_V2_SCORE])

        # Get the class information
        bb_classes = np.argmax(softmax(bb[:, _YOLO_V2_CLASSES:]), axis=1)

        # Compute the bounding box information
        x_center = (bb_cols + sigmoid(bb[:, _YOLO_V2_TX])) / ow
        y_center = (bb_rows + sigmoid(bb[:, _YOLO_V2_TY])) / oh
        w_rel = (bb_a_array[:, 0] * np.exp(bb[:, _YOLO_V2_TW])) / ow
        h_rel = (bb_a_array[:, 1] * np.exp(bb[:, _YOLO_V2_TH])) / oh

        return ml.utils.box_nms(x_center, y_center, w_rel, h_rel, bb_scores, bb_classes,
                                model.input_shape[0][1:3], inputs[0].roi,
                                self.nms_threshold, self.nms_sigma, self.scale_aspect)


# This is a lightweight version of the YOLO (You Only Look Once) object detection algorithm.
# It is designed to work well on embedded devices with limited computational resources.
class yolo_lc_postprocess(yolo_v2_postprocess):
    def __init__(self, threshold=0.6, anchors=None, nms_threshold=0.1, nms_sigma=0.1,
                 scale_aspect=ml.utils.NMS_SCALE_ASPECT_KEEP):
        if anchors is None:
            anchors = np.array([[0.076023, 0.258508],
                                [0.163031, 0.413531],
                                [0.234769, 0.702585],
                                [0.427054, 0.715892],
                                [0.748154, 0.857092]])
        super().__init__(threshold, anchors, nms_threshold, nms_sigma, scale_aspect)


class yolo_v5_postprocess:
    def __init__(self, threshold=0.6, nms_threshold=0.1, nms_sigma=0.1,
                 scale_aspect=ml.utils.NMS_SCALE_ASPECT_KEEP):
        self.threshold = threshold
        self.nms_threshold = nms_threshold
        self.nms_sigma = nms_sigma
        self.scale_aspect = scale_aspect

    def __call__(self, model, inputs, outputs):
        oh, ow, oc = model.output_shape[0]
        s = model.output_scale[0]
        zp = model.output_zero_point[0]
        dt = model.output_dtype[0]
        t = (self.threshold / s) + zp
        class_count = oc - _YOLO_V5_CLASSES

        # Reshape the output to a 2D array
        row_outputs = outputs[0].reshape((oh * ow, _YOLO_V5_CLASSES + class_count))

        # Threshold all the scores
        score_indices = row_outputs[:, _YOLO_V5_SCORE]
        score_indices = threshold(score_indices, t, s)
        if not len(score_indices):
            return _NO_DETECTION

        # Get the bounding boxes that have a valid score
        bb = dequantize(np.take(row_outputs, score_indices, axis=0), dt, zp, s)

        # Get the score information
        bb_scores = bb[:, _YOLO_V5_SCORE]

        # Get the class information
        bb_classes = np.argmax(bb[:, _YOLO_V5_CLASSES:], axis=1)

        # Compute the bounding box information
        x_center = bb[:, _YOLO_V5_CX]
        y_center = bb[:, _YOLO_V5_CY]
        w_rel = bb[:, _YOLO_V5_CW]
        h_rel = bb[:, _YOLO_V5_CH]

        return ml.utils.box_nms(x_center, y_center, w_rel, h_rel, bb_scores, bb_classes,
                                model.input_shape[0][1:3], inputs[0].roi,
                                self.nms_threshold, self.nms_sigma, self.scale_aspect)


class yolo_v8_postprocess:
    def __init__(self, threshold=0.6, nms_threshold=0.1, nms_sigma=0.1,
                 scale_aspect=ml.utils.NMS_SCALE_ASPECT_KEEP):
        self.threshold = threshold
        self.nms_threshold = nms_threshold
        self.nms_sigma = nms_sigma
        self.scale_aspect = scale_aspect

    def __call__(self, model, inputs, outputs):
        oh, ow, oc = model.output_shape[0]
        s = model.output_scale[0]
        zp = model.output_zero_point[0]
        dt = model.output_dtype[0]
        t = (self.threshold / s) + zp
        class_count = ow - _YOLO_V8_CLASSES

        # Reshape the output to a 2D array
        row_outputs = outputs[0].reshape((oh * (_YOLO_V8_CLASSES + class_count), oc)).T

        # Threshold all the scores
        score_indices = row_outputs[:, _YOLO_V8_CLASSES:]
        score_indices = threshold(score_indices, t, s, find_max=True, find_max_axis=1)
        if not len(score_indices):
            return _NO_DETECTION

        # Get the bounding boxes that have a valid score
        bb = dequantize(np.take(row_outputs, score_indices, axis=0), dt, zp, s)

        # Get the score information
        bb_scores = np.max(bb[:, _YOLO_V8_CLASSES:], axis=1)

        # Get the class information
        bb_classes = np.argmax(bb[:, _YOLO_V8_CLASSES:], axis=1)

        # Compute the bounding box information
        x_center = bb[:, _YOLO_V8_CX]
        y_center = bb[:, _YOLO_V8_CY]
        w_rel = bb[:, _YOLO_V8_CW]
        h_rel = bb[:, _YOLO_V8_CH]

        return ml.utils.box_nms(x_center, y_center, w_rel, h_rel, bb_scores, bb_classes,
                                model.input_shape[0][1:3], inputs[0].roi,
                                self.nms_threshold, self.nms_sigma, self.scale_aspect)

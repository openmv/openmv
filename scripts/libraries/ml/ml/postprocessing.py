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
from ml.utils import NMS
from micropython import const
from ulab import numpy as np


_NO_DETECTION = const(())


def logit(x):
    return np.log(x / (1.0 - x))


def sigmoid(x):
    return 1.0 / (1.0 + np.exp(-x))


def mod(a, b):
    return a - (b * (a // b))


def threshold(scores, threshold, scale, find_max=False, find_max_axis=1):
    if scale > 0:
        if find_max:
            scores = np.max(scores, axis=find_max_axis)
        return np.nonzero(scores > threshold)[0]
    else:
        if find_max:
            scores = np.min(scores, axis=find_max_axis)
        return np.nonzero(scores < threshold)[0]


def quantize(model, value, index=0):
    if model.output_dtype[index] == 'f':
        return value
    return (value / model.output_scale[index]) + model.output_zero_point[index]


def dequantize(model, value, index=0):
    if model.output_dtype[index] == 'f':
        return value
    return (value - float(model.output_zero_point[index])) * model.output_scale[index]


class fomo_postprocess:
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
        bb_cols = mod(score_indices, ow)

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


# This is a lightweight version of the tiny yolo v2 object detection algorithm.
# It was optimized to work well on embedded devices with limited computational resources.
class yolo_v2_postprocess:
    _YOLO_V2_TX = const(0)
    _YOLO_V2_TY = const(1)
    _YOLO_V2_TW = const(2)
    _YOLO_V2_TH = const(3)
    _YOLO_V2_SCORE = const(4)
    _YOLO_V2_CLASSES = const(5)

    def __init__(self, threshold=0.6, anchors=None, nms_threshold=0.1, nms_sigma=0.1):
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

    def __call__(self, model, inputs, outputs):

        def softmax(x):
            e_x = np.exp(x - np.max(x, axis=1, keepdims=True))
            return e_x / np.sum(e_x, axis=1, keepdims=True)

        ob, oh, ow, oc = model.output_shape[0]
        scale = model.output_scale[0]
        t = quantize(model, logit(self.threshold))
        class_count = (oc // self.anchors_len) - _YOLO_V2_CLASSES

        # Reshape the output to a 2D array
        row_outputs = outputs[0].reshape((oh * ow * self.anchors_len,
                                          _YOLO_V2_CLASSES + class_count))

        # Threshold all the scores
        score_indices = row_outputs[:, _YOLO_V2_SCORE]
        score_indices = threshold(score_indices, t, scale)
        if not len(score_indices):
            return _NO_DETECTION

        # Get the bounding boxes that have a valid score
        bb = dequantize(model, np.take(row_outputs, score_indices, axis=0))

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

        # Scale the bounding boxes to have enough integer precision for NMS
        ib, ih, iw, ic = model.input_shape[0]
        x_center = x_center * iw
        y_center = y_center * ih
        w_rel = w_rel * iw
        h_rel = h_rel * ih

        nms = NMS(iw, ih, inputs[0].roi)
        for i in range(bb.shape[0]):
            nms.add_bounding_box(x_center[i] - (w_rel[i] / 2),
                                 y_center[i] - (h_rel[i] / 2),
                                 x_center[i] + (w_rel[i] / 2),
                                 y_center[i] + (h_rel[i] / 2),
                                 bb_scores[i], bb_classes[i])
        return nms.get_bounding_boxes(threshold=self.nms_threshold, sigma=self.nms_sigma)


# This is a lightweight version of the YOLO (You Only Look Once) object detection algorithm.
# It is designed to work well on embedded devices with limited computational resources.
class yolo_lc_postprocess(yolo_v2_postprocess):
    def __init__(self, threshold=0.6, anchors=None, nms_threshold=0.1, nms_sigma=0.1):
        if anchors is None:
            anchors = np.array([[0.076023, 0.258508],
                                [0.163031, 0.413531],
                                [0.234769, 0.702585],
                                [0.427054, 0.715892],
                                [0.748154, 0.857092]])
        super().__init__(threshold, anchors, nms_threshold, nms_sigma)


class yolo_v5_postprocess:
    _YOLO_V5_CX = const(0)
    _YOLO_V5_CY = const(1)
    _YOLO_V5_CW = const(2)
    _YOLO_V5_CH = const(3)
    _YOLO_V5_SCORE = const(4)
    _YOLO_V5_CLASSES = const(5)

    def __init__(self, threshold=0.6, nms_threshold=0.1, nms_sigma=0.1):
        self.threshold = threshold
        self.nms_threshold = nms_threshold
        self.nms_sigma = nms_sigma

    def __call__(self, model, inputs, outputs):
        oh, ow, oc = model.output_shape[0]
        scale = model.output_scale[0]
        t = quantize(model, self.threshold)
        class_count = oc - _YOLO_V5_CLASSES

        # Reshape the output to a 2D array
        row_outputs = outputs[0].reshape((oh * ow, _YOLO_V5_CLASSES + class_count))

        # Threshold all the scores
        score_indices = row_outputs[:, _YOLO_V5_SCORE]
        score_indices = threshold(score_indices, t, scale)
        if not len(score_indices):
            return _NO_DETECTION

        # Get the bounding boxes that have a valid score
        bb = dequantize(model, np.take(row_outputs, score_indices, axis=0))

        # Get the score information
        bb_scores = bb[:, _YOLO_V5_SCORE]

        # Get the class information
        bb_classes = np.argmax(bb[:, _YOLO_V5_CLASSES:], axis=1)

        # Compute the bounding box information
        x_center = bb[:, _YOLO_V5_CX]
        y_center = bb[:, _YOLO_V5_CY]
        w_rel = bb[:, _YOLO_V5_CW] * 0.5
        h_rel = bb[:, _YOLO_V5_CH] * 0.5

        # Scale the bounding boxes to have enough integer precision for NMS
        ib, ih, iw, ic = model.input_shape[0]
        xmin = (x_center - w_rel) * iw
        ymin = (y_center - h_rel) * ih
        xmax = (x_center + w_rel) * iw
        ymax = (y_center + h_rel) * ih

        nms = NMS(iw, ih, inputs[0].roi)
        for i in range(bb.shape[0]):
            nms.add_bounding_box(xmin[i], ymin[i], xmax[i], ymax[i],
                                 bb_scores[i], bb_classes[i])
        return nms.get_bounding_boxes(threshold=self.nms_threshold, sigma=self.nms_sigma)


class yolo_v8_postprocess:
    _YOLO_V8_CX = const(0)
    _YOLO_V8_CY = const(1)
    _YOLO_V8_CW = const(2)
    _YOLO_V8_CH = const(3)
    _YOLO_V8_CLASSES = const(4)

    def __init__(self, threshold=0.6, nms_threshold=0.1, nms_sigma=0.1):
        self.threshold = threshold
        self.nms_threshold = nms_threshold
        self.nms_sigma = nms_sigma

    def __call__(self, model, inputs, outputs):
        oh, ow, oc = model.output_shape[0]
        scale = model.output_scale[0]
        t = quantize(model, self.threshold)
        class_count = ow - _YOLO_V8_CLASSES

        # Reshape the output to a 2D array
        column_outputs = outputs[0].reshape((oh * (_YOLO_V8_CLASSES + class_count), oc))

        # Threshold all the scores
        score_indices = column_outputs[_YOLO_V8_CLASSES:, :]
        score_indices = threshold(score_indices, t, scale, find_max=True, find_max_axis=0)
        if not len(score_indices):
            return _NO_DETECTION

        # Get the bounding boxes that have a valid score
        bb = dequantize(model, np.take(column_outputs, score_indices, axis=1))

        # Get the score information
        bb_scores = np.max(bb[_YOLO_V8_CLASSES:, :], axis=0)

        # Get the class information
        bb_classes = np.argmax(bb[_YOLO_V8_CLASSES:, :], axis=0)

        # Compute the bounding box information
        x_center = bb[_YOLO_V8_CX, :]
        y_center = bb[_YOLO_V8_CY, :]
        w_rel = bb[_YOLO_V8_CW, :] * 0.5
        h_rel = bb[_YOLO_V8_CH, :] * 0.5

        # Scale the bounding boxes to have enough integer precision for NMS
        ib, ih, iw, ic = model.input_shape[0]
        xmin = (x_center - w_rel) * iw
        ymin = (y_center - h_rel) * ih
        xmax = (x_center + w_rel) * iw
        ymax = (y_center + h_rel) * ih

        nms = NMS(iw, ih, inputs[0].roi)
        for i in range(bb.shape[1]):
            nms.add_bounding_box(xmin[i], ymin[i], xmax[i], ymax[i],
                                 bb_scores[i], bb_classes[i])
        return nms.get_bounding_boxes(threshold=self.nms_threshold, sigma=self.nms_sigma)


class mediapipe_face_detection_postprocess:
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

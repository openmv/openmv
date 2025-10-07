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


class YoloV5:
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


class YoloV8:
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

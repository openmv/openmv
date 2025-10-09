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
from ulab import numpy as np


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


class NMS:
    def __init__(
        self,
        window_w,
        window_h,
        roi,
    ):
        self.window_w = window_w
        self.window_h = window_h
        self.roi = roi
        if roi[2] < 1 or roi[3] < 1:
            raise ValueError("Invalid ROI dimensions!")
        self.boxes = []

    def add_bounding_box(self, xmin, ymin, xmax, ymax, score, label_index, keypoints=None):
        if score >= 0.0 and score <= 1.0:
            xmin = max(0.0, min(xmin, self.window_w))
            ymin = max(0.0, min(ymin, self.window_h))
            xmax = max(0.0, min(xmax, self.window_w))
            ymax = max(0.0, min(ymax, self.window_h))
            w = int(xmax - xmin)
            h = int(ymax - ymin)
            if w > 0 and h > 0:
                self.boxes.append([int(xmin), int(ymin), w, h, score, label_index, keypoints])

    def get_bounding_boxes(self, threshold=0.1, sigma=0.1):
        sorted_boxes = sorted(self.boxes, key=lambda x: x[4], reverse=True)
        sigma_scale = (-1.0 / sigma) if (sigma > 0.0) else 0.0

        def iou(box1, box2):
            x1 = max(box1[0], box2[0])
            y1 = max(box1[1], box2[1])
            x2 = min(box1[0] + box1[2], box2[0] + box2[2])
            y2 = min(box1[1] + box1[3], box2[1] + box2[3])
            w = max(0, x2 - x1)
            h = max(0, y2 - y1)
            intersection = w * h
            union = (box1[2] * box1[3]) + (box2[2] * box2[3]) - intersection
            return float(intersection) / float(union)

        # Perform Non Max Supression.

        max_index = 0
        output_boxes = []
        max_label_index = 0

        while len(sorted_boxes):
            box = sorted_boxes.pop(max_index)
            output_boxes.append(box)
            max_label_index = max(max_label_index, box[5])

            # Compare and supress the remaining boxes in the list against the max.

            for i in range(len(sorted_boxes)):
                v = iou(box, sorted_boxes[i])
                sorted_boxes[i][4] = sorted_boxes[i][4] * math.exp(sigma_scale * v * v)
                if sorted_boxes[i][4] < threshold:
                    sorted_boxes[i][4] = 0.0

            # Filter out supressed boxes and find the next largest.

            sorted_boxes = list(filter(lambda x: x[4] > 0.0, sorted_boxes))
            if len(sorted_boxes):
                max_index = max(enumerate(sorted_boxes), key=lambda x: x[1][4])[0]

        # Map the output boxes back to the input image.

        x_scale = self.roi[2] / float(self.window_w)
        y_scale = self.roi[3] / float(self.window_h)
        scale = min(x_scale, y_scale)
        x_offset = ((self.roi[2] - (self.window_w * scale)) / 2) + self.roi[0]
        y_offset = ((self.roi[3] - (self.window_h * scale)) / 2) + self.roi[1]

        for i in range(len(output_boxes)):
            output_boxes[i][0] = int((output_boxes[i][0] * scale) + x_offset)
            output_boxes[i][1] = int((output_boxes[i][1] * scale) + y_offset)
            output_boxes[i][2] = int(output_boxes[i][2] * scale)
            output_boxes[i][3] = int(output_boxes[i][3] * scale)
            keypoints = output_boxes[i][6]
            if keypoints is not None:
                keypoints *= scale
                keypoints[:, 0] += x_offset
                keypoints[:, 1] += y_offset

        # Create a list per class with (rect, score) tuples.

        output_list = [[] for i in range(max_label_index + 1)]

        for i in range(len(output_boxes)):
            rect_score = [output_boxes[i][:4], output_boxes[i][4]]
            keypoints = output_boxes[i][6]
            if keypoints is not None:
                rect_score.append(keypoints)
            output_list[output_boxes[i][5]].append(tuple(rect_score))

        return output_list


def draw_predictions(
    image,
    boxes,
    labels,
    colors,
    format="pascal_voc",
    font_width=8,
    font_height=10,
    text_color=(255, 255, 255),
):
    image_w = image.width()
    image_h = image.height()
    for i, (x, y, w, h) in enumerate(boxes):
        label = labels[i]
        box_color = colors[i]

        if format == "pascal_voc":
            x = int(x * image_w)
            y = int(y * image_h)
            w = int(w * image_w) - x
            h = int(h * image_h) - y

        image.draw_rectangle(x, y, w, h, color=box_color)
        image.draw_rectangle(
            x,
            y - font_height,
            len(label) * font_width,
            font_height,
            fill=True,
            color=box_color,
        )
        image.draw_string(x, y - font_height, label.upper(), text_color)

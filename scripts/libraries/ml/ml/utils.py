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
from micropython import const
from ulab import numpy as np


NMS_SCALE_ASPECT_KEEP = const(0)
NMS_SCALE_ASPECT_EXPAND = const(1)
NMS_SCALE_ASPECT_IGNORE = const(2)


def box_nms(
    x_center,                           # shape: (N,)
    y_center,                           # shape: (N,)
    w_rel,                              # shape: (N,)
    h_rel,                              # shape: (N,)
    scores,                             # shape: (N,)
    classes,                            # shape: (N,)
    input_shape,                        # (input_h, input_w)
    image_roi,                          # (image_x, image_y, image_w, image_h)
    threshold=0.1,                      # IoU threshold for NMS
    sigma=0.1,                          # Sigma for NMS
    scale_aspect=NMS_SCALE_ASPECT_KEEP  # Scale aspect ratio
):
    N = len(classes)
    assert all(N == len(arr) for arr in [x_center, y_center, w_rel, h_rel, scores]), \
        f"Inconsistent lengths: classes={N}, x_center={len(x_center)}, " \
        f"y_center={len(y_center)}, w_rel={len(w_rel)}, h_rel={len(h_rel)}, " \
        f"scores={len(scores)}"

    input_h, input_w = input_shape
    roi_x, roi_y, roi_w, roi_h = image_roi

    sigma_inv = (-1.0 / sigma) if (sigma > 0.0) else 0.0

    x_scale = roi_w / float(input_w)
    y_scale = roi_h / float(input_h)

    if scale_aspect == NMS_SCALE_ASPECT_KEEP:
        scale = min(x_scale, y_scale)
        x_scale = scale
        y_scale = scale
    elif scale_aspect == NMS_SCALE_ASPECT_EXPAND:
        scale = max(x_scale, y_scale)
        x_scale = scale
        y_scale = scale
    elif scale_aspect != NMS_SCALE_ASPECT_IGNORE:
        raise ValueError("Invalid scale_aspect value!")

    x_offset = ((roi_w - (input_w * x_scale)) * 0.5) + roi_x
    y_offset = ((roi_h - (input_h * y_scale)) * 0.5) + roi_y

    # Convert boxes to (x1, y1, x2, y2) format.
    w_rel_2 = w_rel * 0.5
    h_rel_2 = h_rel * 0.5
    x1 = (x_center - w_rel_2) * input_w
    y1 = (y_center - h_rel_2) * input_h
    x2 = (x_center + w_rel_2) * input_w
    y2 = (y_center + h_rel_2) * input_h
    areas = (x2 - x1) * (y2 - y1)
    boxes = np.array([x1, y1, x2, y2, areas]).T

    # Allocate output list for each class.
    output = [[] for _ in range(int(np.max(classes)) + 1)]

    # Filter out invalid boxes.
    valid_indices = np.nonzero((areas > 0.0) & (scores > threshold) & (scores <= 1.0))[0]
    if not len(valid_indices):
        return output

    # Sort boxes by scores in descending order.
    valid_scores = np.take(scores, valid_indices, axis=0)
    sorted_valid_score_indices = np.argsort(valid_scores, axis=0)[::-1]
    sorted_valid_indices = np.take(valid_indices, sorted_valid_score_indices, axis=0)

    while True:
        # Grab the box with the highest score.
        i = sorted_valid_indices[0]
        x1i, y1i, x2i, y2i, area = boxes[i]

        # Project and store the box.
        px = round((x1i * x_scale) + x_offset)
        py = round((y1i * y_scale) + y_offset)
        pw = round((x2i - x1i) * x_scale)
        ph = round((y2i - y1i) * y_scale)
        output[classes[i]].append(((px, py, pw, ph), scores[i]))

        # Stop if there's only one box left.
        if len(sorted_valid_indices) == 1:
            break

        # Get the rest of the boxes.
        sorted_valid_indices = sorted_valid_indices[1:]
        boxes = np.take(boxes, sorted_valid_indices, axis=0)
        scores = np.take(scores, sorted_valid_indices, axis=0)
        classes = np.take(classes, sorted_valid_indices, axis=0)

        # Compute IoU of the max box with the rest.
        xx1 = np.maximum(x1i, boxes[:, 0])
        yy1 = np.maximum(y1i, boxes[:, 1])
        xx2 = np.minimum(x2i, boxes[:, 2])
        yy2 = np.minimum(y2i, boxes[:, 3])
        iw = np.maximum(0.0, xx2 - xx1)
        ih = np.maximum(0.0, yy2 - yy1)
        intersection = iw * ih
        union = area + boxes[:, 4] - intersection
        iou = intersection / (union + 1e-6)
        scores *= np.exp((iou ** 2.0) * sigma_inv)

        # Filter out boxes with low scores.
        valid_indices = np.nonzero(scores > threshold)[0]
        if not len(valid_indices):
            break

        # Sort boxes by scores in descending order.
        valid_scores = np.take(scores, valid_indices, axis=0)
        sorted_valid_score_indices = np.argsort(valid_scores, axis=0)[::-1]
        sorted_valid_indices = np.take(valid_indices, sorted_valid_score_indices, axis=0)

    return output


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

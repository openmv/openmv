# Copyright (C) 2026 OpenMV, LLC.
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
from ml.utils import NMS, dequantize
from ulab import numpy as np


class MoveNetLightning:
    """Post-processor for MoveNet Lightning/Thunder heatmap-based models.

    The model outputs a single tensor of shape (1, h, w, k) where k is the
    number of keypoints (17 for COCO pose). Each (h, w) slice is a heatmap
    for one keypoint. The argmax of each heatmap gives the keypoint location.

    Returns a list of ((x, y, w, h), score, keypoints) tuples via NMS,
    where keypoints is an ndarray of shape (k, 2) in image coordinates.
    """

    def __init__(self, threshold=0.3):
        self.threshold = threshold

    def __call__(self, model, inputs, outputs):
        ib, ih, iw, ic = model.input_shape[0]
        nms = NMS(iw, ih, inputs[0].roi)

        b, h, w, k = model.output_shape[0]
        heatmaps = dequantize(model, outputs[0].reshape((h * w, k)), index=0)

        arg_pred = np.argmax(heatmaps, axis=0)
        val_pred = np.max(heatmaps, axis=0)

        arg_y = arg_pred // w
        arg_x = arg_pred - arg_y * w

        # Normalized coordinates [0, 1]
        pred_x = (arg_x + 0.5) / w
        pred_y = (arg_y + 0.5) / h

        # Build keypoints array in input image coordinates
        keypoints = np.empty((k, 2))
        keypoints[:, 0] = pred_x * iw
        keypoints[:, 1] = pred_y * ih

        # Average confidence of valid keypoints as the detection score
        valid_kp_x = []
        valid_kp_y = []
        for i in range(k):
            if val_pred[i] > self.threshold:
                valid_kp_x.append(float(keypoints[i, 0]))
                valid_kp_y.append(float(keypoints[i, 1]))

        if not valid_kp_x:
            return ()

        score = float(np.sum(val_pred * (val_pred > self.threshold))) / len(valid_kp_x)

        xmin = min(valid_kp_x)
        ymin = min(valid_kp_y)
        xmax = max(valid_kp_x)
        ymax = max(valid_kp_y)

        nms.add_bounding_box(xmin, ymin, xmax, ymax, score, 0, keypoints=keypoints)
        return nms.get_bounding_boxes()[0]

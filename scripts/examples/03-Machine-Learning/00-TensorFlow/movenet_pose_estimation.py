# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off Google's MoveNet Lightning pose estimation model.
#
# NOTE: This example requires an OpenMV Cam with an NPU like the AE3 or N6 to run real-time.

import csi
import time
import ml
from ml.postprocessing.movenet import MoveNetLightning

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.window((192, 192))
csi0.snapshot(time=2000)

clock = time.clock()

# Load MoveNet Lightning model with postprocessing.
model = ml.Model(
    "/rom/movenet_lightning_heatmaps_192_int8_pc.tflite",
    postprocess=MoveNetLightning(threshold=0.3),
)
print(model)

# COCO 17-keypoint skeleton connections.
# Keypoints: 0 nose, 1 left_eye, 2 right_eye, 3 left_ear, 4 right_ear,
# 5 left_shoulder, 6 right_shoulder, 7 left_elbow, 8 right_elbow,
# 9 left_wrist, 10 right_wrist, 11 left_hip, 12 right_hip,
# 13 left_knee, 14 right_knee, 15 left_ankle, 16 right_ankle
skeleton_lines = [
    (0, 1), (0, 2), (1, 3), (2, 4),
    (5, 6), (5, 7), (7, 9), (6, 8), (8, 10),
    (5, 11), (6, 12), (11, 12),
    (11, 13), (13, 15), (12, 14), (14, 16),
]

while True:
    clock.tick()
    img = csi0.snapshot()

    # predict() calls postprocess automatically.
    # Returns a list of (rect, score, keypoints) tuples.
    for r, score, keypoints in model.predict([img]):
        # keypoints: ndarray (17, 2) in image coordinates.
        # Index: 0 nose, 1 left_eye, 2 right_eye, 3 left_ear, 4 right_ear,
        #        5 left_shoulder, 6 right_shoulder, 7 left_elbow, 8 right_elbow,
        #        9 left_wrist, 10 right_wrist, 11 left_hip, 12 right_hip,
        #        13 left_knee, 14 right_knee, 15 left_ankle, 16 right_ankle
        ml.utils.draw_skeleton(
            img, keypoints, skeleton_lines,
            kp_color=(255, 0, 0), line_color=(0, 255, 0),
        )

    print(clock.fps(), "fps")

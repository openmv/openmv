# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off Google's MoveNet Pose Estimation model.
#
# NOTE: This example requires an OpenMV Cam with an NPU like the AE3 or N6 to run real-time.

import csi
import time
import ml
from ml.postprocessing.mediapipe import MoveNet

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

# Load built-in pose detection model
model = ml.Model("/rom/movenet_singlepose_192.tflite", postprocess=MoveNet(threshold=0.4))
print(model)

# Line connections between body joints for drawing the body skeleton.
body_lines = ((0, 1), (0, 2), (1, 3), (2, 4), (0, 5), (0, 6), (5, 6), (5, 7),
              (7, 9), (6, 8), (8, 10), (5, 11), (6, 12), (11, 12), (11, 13), (13, 15),
              (12, 14), (14, 16))


# Remove keypoints with low confidence and skeleton lines that connect to them.
def filter_keypoints(keypoints, threshold=0.4):
    valid = {i for i, kp in enumerate(keypoints) if kp[2] > threshold}
    remap = {old: new for new, old in enumerate(sorted(valid))}
    f_keypoints = [kp for i, kp in enumerate(keypoints) if i in valid]
    f_body_lines = [(remap[a], remap[b]) for a, b in body_lines if a in valid and b in valid]
    return f_keypoints, f_body_lines


clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    # joints is a list of ((x, y, w, h), score, keypoints) tuples
    joints = model.predict([img])

    # Draw bounding boxes around the detected persons and keypoints.
    for r, score, keypoints in joints:
        ml.utils.draw_predictions(img, [r], ("person",), ((0, 0, 255),), format=None)

        # keypoints: ndarray (17, 3) of body joints (x, y, score)
        # Indices follow COCO convention:
        # 0: nose
        # 1: left_eye, 2: right_eye
        # 3: left_ear, 4: right_ear
        # 5: left_shoulder, 6: right_shoulder
        # 7: left_elbow, 8: right_elbow
        # 9: left_wrist, 10: right_wrist
        # 11: left_hip, 12: right_hip
        # 13: left_knee, 14: right_knee
        # 15: left_ankle, 16: right_ankle
        f_keypoints, f_body_lines = filter_keypoints(keypoints)
        ml.utils.draw_skeleton(img, f_keypoints, f_body_lines,
                               kp_color=(255, 0, 0), line_color=(0, 255, 0))

    print(clock.fps(), "fps")

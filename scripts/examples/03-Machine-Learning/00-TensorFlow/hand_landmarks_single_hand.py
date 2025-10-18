# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off Google's MediaPipe Hand Landmarks Detection model for a single hand.
#
# NOTE: This exaxmple requires an OpenMV Cam with an NPU like the AE3 or N6 to run real-time.

import csi
import time
import ml
from ml.preprocessing import Normalization
from ml.postprocessing.mediapipe import BlazePalm
from ml.postprocessing.mediapipe import HandLandmarks

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

# Load built-in palm detection model
palm_detection = ml.Model("/rom/palm_detection_full_192.tflite", postprocess=BlazePalm(threshold=0.4))
print(palm_detection)

# Load built-in hand landmark model
hand_landmarks = ml.Model("/rom/hand_landmarks_full_224.tflite", postprocess=HandLandmarks(threshold=0.4))
print(hand_landmarks)

# Line connections between hand joints for drawing the hand skeleton.
hand_lines = ((0, 1), (1, 2), (2, 3), (3, 4), (0, 5), (5, 6), (6, 7), (7, 8),
              (5, 9), (9, 10), (10, 11), (11, 12), (9, 13), (13, 14), (14, 15), (15, 16),
              (13, 17), (17, 18), (18, 19), (19, 20), (0, 17))

# Visualization parameters.
palm_colors = [(0, 0, 255)]
kp_color = (255, 0, 0)
line_color = (0, 255, 0)

# Tracking vars.
n = None

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    if n is None:
        # palms is a list of ((x, y, w, h), score, keypoints) tuples
        palms = palm_detection.predict([img])

        if palms:
            for r, score, keypoints in palms[0]:
                # rect is (x, y, w, h) - enlarge by 3x for hand landmarks model
                wider_rect = (r[0] - r[2], r[1] - r[3], r[2] * 3, r[3] * 3)
                # Operate on just the ROI of the detected palm
                n = Normalization(roi=wider_rect)

    else:
        # hands is a list of ((x, y, w, h), score, keypoints) tuples
        # index 0 (if present) is left hand
        # index 1 (if present) is right hand
        hands = hand_landmarks.predict([n(img)])

        # No hands detected, reset the tracker.
        if not hands:
            n = None
            continue

        # Draw bounding boxes around the detected hands and keypoints.
        for i, detections in enumerate(hands):
            for r, score, keypoints in detections:
                ml.utils.draw_predictions(img, [r], ["right" if i else "left"], [(0, 0, 255)], format=None)

                # keypoints: ndarray (21, 3) of hand joints (x, y, z)
                # Indices follow MediaPipe convention:
                # 0: wrist
                # Thumb: 1 cmc, 2 mcp, 3 ip, 4 tip
                # Index: 5 mcp, 6 pip, 7 dip, 8 tip
                # Middle: 9 mcp, 10 pip, 11 dip, 12 tip
                # Ring: 13 mcp, 14 pip, 15 dip, 16 tip
                # Pinky: 17 mcp, 18 pip, 19 dip, 20 tip
                # (cmc=base, mcp=knuckle, pip=mid, dip=distal, ip=thumb joint, tip=fingertip)
                ml.utils.draw_skeleton(img, keypoints, hand_lines, kp_color=kp_color, line_color=line_color)

                # Center new_wider_rect on hand for tracking
                new_wider_rect = (r[0] + (r[2] // 2) - (wider_rect[2] // 2),
                                  r[1] + (r[3] // 2) - (wider_rect[3] // 2),
                                  wider_rect[2],
                                  wider_rect[3])
                # Operate on just the ROI of the detected hand
                n = Normalization(roi=new_wider_rect)

    print(clock.fps(), "fps")

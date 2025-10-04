# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off Google's MediaPipe Face Landmark Detection model for a single face.
#
# NOTE: This exaxmple requires an OpenMV Cam with an NPU like the AE3 or N6 to run real-time.

import csi
import time
import ml
from ml.preprocessing import Normalization
from ml.postprocessing.mediapipe import BlazeFace
from ml.postprocessing.mediapipe import FaceLandmarks

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

# Load built-in face detection model
face_detection = ml.Model("/rom/blazeface_front_128.tflite", postprocess=BlazeFace(threshold=0.4))
print(face_detection)

# Load built-in face landmark model
face_landmarks = ml.Model("/rom/face_landmarks_192.tflite", postprocess=FaceLandmarks(threshold=0.4))
print(face_landmarks)

# Tracking vars.
n = None

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    if n is None:
        # faces is a list of ((x, y, w, h), score, keypoints) tuples
        for r, score, keypoints in face_detection.predict([img]):
            # rect is (x, y, w, h) - enlarge by 2x for face landmarks model
            wider_rect = (r[0] - r[2] // 2, r[1] - r[3] // 2, r[2] * 2, r[3] * 2)
            # Operate on just the ROI of the detected face
            n = Normalization(roi=wider_rect)

    else:
        # marks is a list of ((x, y, w, h), score, keypoints) tuples
        marks = face_landmarks.predict([n(img)])

        # No faces detected, reset the tracker.
        if not marks:
            n = None
            continue

        # Draw bounding boxes around the detected faces and keypoints.
        for r, score, keypoints in marks:
            ml.utils.draw_predictions(img, [r], ("face",), ((0, 0, 255),), format=None)

            # keypoints is a ndarray of shape (468, 3) where each keypoint is (x, y, z)
            ml.utils.draw_keypoints(img, keypoints, radius=0, color=(255, 0, 0))

            # Center new_wider_rect on face for tracking
            new_wider_rect = (r[0] + (r[2] // 2) - (wider_rect[2] // 2),
                              r[1] + (r[3] // 2) - (wider_rect[3] // 2),
                              wider_rect[2],
                              wider_rect[3])
            # Operate on just the ROI of the detected face
            n = Normalization(roi=new_wider_rect)

    print(clock.fps(), "fps")

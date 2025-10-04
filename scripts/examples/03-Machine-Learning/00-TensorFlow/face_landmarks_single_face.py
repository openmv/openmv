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
face_detection = ml.Model("/rom/blazeface_front_128.tflite")
print(face_detection)

# Load built-in face landmark model
face_landmarks = ml.Model("/rom/face_landmarks_192.tflite")
print(face_landmarks)

# Create the face detection post-processor. This post-processor dynamically
# generates anchors for the model input size which should only be done once.
face_detection_postprocess = BlazeFace(threshold=0.6)

# Create the face landmarks post-processor.
face_landmarks_postprocess = FaceLandmarks(threshold=0.4)

# Visualization parameters.
face_labels = ["face"]
face_colors = [(0, 0, 255)]
keypoint_color = (255, 0, 0)

# Tracking vars.
n = None

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    if n is None:
        # faces is a list of ((x, y, w, h), score, keypoints) tuples
        faces = face_detection.predict([img], callback=face_detection_postprocess)

        if faces:
            for r, score, keypoints in faces[0]:
                # rect is (x, y, w, h) - enlarge by 2x for face landmarks model
                wider_rect = (r[0] - r[2] // 2, r[1] - r[3] // 2, r[2] * 2, r[3] * 2)
                # Operate on just the ROI of the detected face
                n = Normalization(roi=wider_rect)

    else:
        # marks is a list of ((x, y, w, h), score, keypoints) tuples
        marks = face_landmarks.predict([n(img)], callback=face_landmarks_postprocess)

        # No faces detected, reset the tracker.
        if not marks:
            n = None
            continue

        # Draw bounding boxes around the detected faces and keypoints.
        for i, detections in enumerate(marks):
            for r, score, keypoints in detections:
                ml.utils.draw_predictions(img, [r], face_labels, face_colors, format=None)

                # keypoints is a ndarray of shape (468, 3) where each keypoint is (x, y, z)
                ml.utils.draw_keypoint_dots(img, keypoints, color=keypoint_color)

                # Center new_wider_rect on face for tracking
                new_wider_rect = (r[0] + (r[2] // 2) - (wider_rect[2] // 2),
                                  r[1] + (r[3] // 2) - (wider_rect[3] // 2),
                                  wider_rect[2],
                                  wider_rect[3])
                # Operate on just the ROI of the detected face
                n = Normalization(roi=new_wider_rect)

    print(clock.fps(), "fps")

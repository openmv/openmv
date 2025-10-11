# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off Google's MediaPipe Face Detection model.

import csi
import time
import ml
from ml.postprocessing.mediapipe import BlazeFace

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

# Load built-in face detection model
model = ml.Model("/rom/blazeface_front_128.tflite")
print(model)

# Create the face detection post-processor. This post-processor dynamically
# generates anchors for the model input size which should only be done once.
face_detection_postprocess = BlazeFace(threshold=0.6)

# Visualization parameters.
face_labels = ["face"]
face_colors = [(0, 0, 255)]
keypoint_color = (255, 0, 0)

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    # faces is a list of ((x, y, w, h), score, keypoints) tuples
    faces = model.predict([img], callback=face_detection_postprocess)

    # Draw bounding boxes around the detected faces and keypoints.
    if faces:
        for r, score, keypoints in faces[0]:
            ml.utils.draw_predictions(img, [r], face_labels, face_colors, format=None)

            # keypoints is a ndarray of shape (6, 2)
            # 0 - right eye (x, y)
            # 1 - left eye (x, y)
            # 2 - nose (x, y)
            # 3 - mouth (x, y)
            # 4 - right ear (x, y)
            # 5 - left ear (x, y)
            ml.utils.draw_keypoints(img, keypoints, color=keypoint_color)

    print(clock.fps(), "fps")

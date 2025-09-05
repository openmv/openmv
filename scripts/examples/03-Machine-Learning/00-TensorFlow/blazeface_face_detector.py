# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off Google's MediaPipe BlazeFace face detection model.

import csi
import time
import ml
from ml.postprocessing import mediapipe_face_detection_postprocess

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
face_detection_postprocess = mediapipe_face_detection_postprocess(threshold=0.6)

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    # faces is a list of ((x, y, w, h), score, keypoints) tuples
    faces = model.predict([img], callback=face_detection_postprocess)

    # Draw bounding boxes around the detected faces and keypoints.
    if faces:
        for r, score, keypoints in faces[0]:
            ml.utils.draw_predictions(img, [r], ["face"], [(0, 0, 255)], format=None)
            # keypoints is a ndarray of shape (6, 2)
            # 0 - right eye (x, y)
            # 1 - left eye (x, y)
            # 2 - nose (x, y)
            # 3 - mouth (x, y)
            # 4 - right ear (x, y)
            # 5 - left ear (x, y)
            for kp in keypoints.tolist():
                img.draw_circle(int(kp[0]), int(kp[1]), 4, color=(255, 0, 0))

    print(clock.fps(), "fps")

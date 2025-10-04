# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off Google's MediaPipe Palm Detection model.

import csi
import time
import ml
from ml.postprocessing import mediapipe_palm_detection_postprocess

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

# Load built-in palm detection model
model = ml.Model("/rom/palm_detection_full_192.tflite")
print(model)

# Create the palm detection post-processor. This post-processor dynamically
# generates anchors for the model input size which should only be done once.
palm_detection_postprocess = mediapipe_palm_detection_postprocess(threshold=0.4)

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    # palms is a list of ((x, y, w, h), score, keypoints) tuples
    palms = model.predict([img], callback=palm_detection_postprocess)

    # Draw bounding boxes around the detected palms and keypoints.
    if palms:
        for r, score, keypoints in palms[0]:
            ml.utils.draw_predictions(img, [r], ["palm"], [(0, 0, 255)], format=None)
            # keypoints is a ndarray of shape (7, 2)
            # 0 - wrist (x, y)
            # 1 - index finger mcp (x, y)
            # 2 - middle finger mcp (x, y)
            # 3 - ring finger mcp (x, y)
            # 4 - pinky mcp  (x, y)
            # 5 - thumb cmc (x, y)
            # 6 - thumb mcp (x, y)
            #
            # mcp = Metacarpophalangeal Joint - the knuckle
            # cmc = Carpometacarpal Joint - the base of the thumb
            for kp in keypoints.tolist():
                img.draw_circle(int(kp[0]), int(kp[1]), 4, color=(255, 0, 0))

            # Draw lines between hand joints.
            lines = [(0, 1), (1, 2), (2, 3), (3, 4), (4, 0), (0, 5), (5, 6)]
            for l in lines:
                img.draw_line(int(keypoints[l[0]][0]), int(keypoints[l[0]][1]),
                              int(keypoints[l[1]][0]), int(keypoints[l[1]][1]), color=(0, 255, 0))

    print(clock.fps(), "fps")

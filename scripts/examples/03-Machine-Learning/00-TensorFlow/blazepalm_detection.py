# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off Google's MediaPipe Palm Detection model.
#
# NOTE: This exaxmple requires an OpenMV Cam with an NPU like the AE3 or N6 to run real-time.

import csi
import time
import ml
from ml.postprocessing.mediapipe import BlazePalm

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

# Load built-in palm detection model
model = ml.Model("/rom/palm_detection_full_192.tflite", postprocess=BlazePalm(threshold=0.4))
print(model)

# Line connections between hand joints for drawing the hand skeleton.
palm_lines = ((0, 1), (1, 2), (2, 3), (3, 4), (4, 0), (0, 5), (5, 6))

# Visualization parameters.
palm_labels = ["palm"]
palm_colors = [(0, 0, 255)]
kp_color = (255, 0, 0)
line_color = (0, 255, 0)

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    # palms is a list of ((x, y, w, h), score, keypoints) tuples
    palms = model.predict([img])

    # Draw bounding boxes around the detected palms and keypoints.
    if palms:
        for r, score, keypoints in palms[0]:
            ml.utils.draw_predictions(img, [r], palm_labels, palm_colors, format=None)

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
            ml.utils.draw_skeleton(img, keypoints, palm_lines, kp_color=kp_color, line_color=line_color)

    print(clock.fps(), "fps")

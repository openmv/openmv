# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# TensorFlow Lite Object Detection Example
#
# This examples uses the builtin FOMO model to detect faces.

import csi
import time
import ml
from ml.postprocessing.edgeimpulse import Fomo
import math
csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.pixformat(csi.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
csi0.framesize(csi.QVGA)  # Set frame size to QVGA (320x240)
csi0.window((240, 240))  # Set 240x240 window.
csi0.snapshot(time=2000)  # Let the camera adjust.

# Load built-in FOMO face detection model
model = ml.Model("/rom/fomo_face_detection.tflite", postprocess=Fomo(threshold=0.4))
print(model)

# Alternatively, models can be loaded from the filesystem storage.
# model = ml.Model('<object_detection_modelwork>.tflite', load_to_fb=True)
# labels = [line.rstrip('\n') for line in open("labels.txt")]

colors = [  # Add more colors if you are detecting more than 7 types of classes at once.
    (255, 0, 0),
    (0, 255, 0),
    (255, 255, 0),
    (0, 0, 255),
    (255, 0, 255),
    (0, 255, 255),
    (255, 255, 255),
]

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    for i, detection_list in enumerate(model.predict([img])):
        if i == 0:
            continue  # background class
        if len(detection_list) == 0:
            continue  # no detections for this class?

        print("********** %s **********" % model.labels[i])
        for (x, y, w, h), score in detection_list:
            center_x = math.floor(x + (w / 2))
            center_y = math.floor(y + (h / 2))
            print(f"x {center_x}\ty {center_y}\tscore {score}")
            img.draw_circle((center_x, center_y, 12), color=colors[i])

    print(clock.fps(), "fps", end="\n")

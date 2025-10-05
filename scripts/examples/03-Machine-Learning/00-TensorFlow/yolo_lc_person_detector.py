# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# TensorFlow Lite YOLO LC Person Detector Example
#
# YOLO LC is a variant of YOLOV2 that is fast enough to run on OpenMV Cams without NPUs.

import csi
import time
import ml
from ml.postprocessing.darknet import YoloLC

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

# Load built-in person detection model
model = ml.Model("/rom/yolo_lc_192.tflite", postprocess=YoloLC(threshold=0.4))
print(model)

# Visualization parameters.
n = len(model.labels)
model_class_colors = [(int(255 * i // n), int(255 * (n - i - 1) // n), 255) for i in range(n)]

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    # boxes is a list of list per class of ((x, y, w, h), score) tuples
    boxes = model.predict([img])

    # Draw bounding boxes around the detected objects
    for i, class_detections in enumerate(boxes):
        rects = [r for r, score in class_detections]
        labels = [model.labels[i] for j in range(len(rects))]
        colors = [model_class_colors[i] for j in range(len(rects))]
        ml.utils.draw_predictions(img, rects, labels, colors, format=None)

    print(clock.fps(), "fps")

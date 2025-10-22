# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# TensorFlow Lite YOLO V5 Example
#
# This example runs a YOLO V5 object detection model.
# Please see OpenMV IDE's model zoo for example yolo v5 models.
#
# You can train your own custom YOLOV5 models using Edge Impulse:
# https://github.com/edgeimpulse/ml-block-yolov5
#
# NOTE: This exaxmple requires an OpenMV Cam with an NPU like the AE3 or N6 to run real-time.

import csi
import time
import ml
from ml.postprocessing.ultralytics import YoloV5

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

# Load YOLO V5 model from ROM FS.
model = ml.Model("/rom/<model_file_name>", postprocess=YoloV5(threshold=0.4))
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

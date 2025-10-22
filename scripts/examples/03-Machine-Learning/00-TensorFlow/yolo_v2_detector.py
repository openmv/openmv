# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# TensorFlow Lite YOLO V2 Example
#
# This example runs a YOLO V2 object detection model.
# Please see OpenMV IDE's model zoo for example yolo v2 models.
#
# For more information on YOLO V2, please see:
# https://github.com/STMicroelectronics/stm32ai-modelzoo/tree/main/object_detection/tiny_yolo_v2
#
# NOTE: This exaxmple requires an OpenMV Cam with an NPU like the AE3 or N6 to run real-time.

import csi
import time
import ml
from ml.postprocessing.darknet import YoloV2

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

# Load YOLO V2 model from ROM FS.
model = ml.Model("/rom/<model_file_name>", postprocess=YoloV2(threshold=0.4))
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

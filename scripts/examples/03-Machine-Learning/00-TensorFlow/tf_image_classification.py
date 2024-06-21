# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# TensorFlow Lite Mobilenet V1 Example
#
# Google's Mobilenet V1 detects 1000 classes of objects
#
# WARNING: Mobilenet is trained on ImageNet and isn't meant to classify anything
# in the real world. It's just designed to score well on the ImageNet dataset.
# This example just shows off running mobilenet on the OpenMV Cam. However, the
# default model is not really usable for anything. You have to use transfer
# learning to apply the model to a target problem by re-training the model.
#
# NOTE: This example only works on the OpenMV Cam H7 Plus (that has SDRAM) and better!
# To get the models please see the CNN Network library in OpenMV IDE under
# Tools -> Machine Vision. The labels are there too.
# You should insert a microSD card into your camera and copy-paste the mobilenet_labels.txt
# file and your chosen model into the root folder for this script to work.

import sensor
import time
import tf

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)  # Set frame size to QVGA (320x240)
sensor.set_windowing((240, 240))  # Set 240x240 window.
sensor.skip_frames(time=2000)  # Let the camera adjust.

mobilenet_version = "1"  # 1
mobilenet_width = "0.5"  # 1.0, 0.75, 0.50, 0.25
mobilenet_resolution = "128"  # 224, 192, 160, 128

mobilenet = "mobilenet_v%s_%s_%s_quant.tflite" % (
    mobilenet_version,
    mobilenet_width,
    mobilenet_resolution,
)

net = tf.Model(mobilenet, load_to_fb=True)
labels = [line.rstrip("\n") for line in open("mobilenet_labels.txt")]

clock = time.clock()
while True:
    clock.tick()

    img = sensor.snapshot()

    print("**********\nTop 5 Detections")
    # This combines the labels and confidence values into a list of tuples
    # and then sorts that list by the confidence values.
    sorted_list = sorted(
        zip(labels, net.predict(img)), key=lambda x: x[1], reverse=True
    )
    for i in range(5):
        print("%s = %f" % (sorted_list[i][0], sorted_list[i][1]))
    print(clock.fps(), "fps")

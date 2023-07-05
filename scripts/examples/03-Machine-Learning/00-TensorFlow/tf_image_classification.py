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
# NOTE: This example only works on the OpenMV Cam H7 Pro (that has SDRAM) and better!
# To get the models please see the CNN Network library in OpenMV IDE under
# Tools -> Machine Vision. The labels are there too.
# You should insert a microSD card into your camera and copy-paste the mobilenet_labels.txt
# file and your chosen model into the root folder for ths script to work.
#
# In this example we slide the detector window over the image and get a list
# of activations. Note that use a CNN with a sliding window is extremely compute
# expensive so for an exhaustive search do not expect the CNN to be real-time.

import sensor
import time
import os
import tf

sensor.reset()                         # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)    # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)      # Set frame size to QVGA (320x240)
sensor.set_windowing((240, 240))       # Set 240x240 window.
sensor.skip_frames(time=2000)          # Let the camera adjust.

mobilenet_version = "1" # 1
mobilenet_width = "0.5" # 1.0, 0.75, 0.50, 0.25
mobilenet_resolution = "128" # 224, 192, 160, 128

mobilenet = "mobilenet_v%s_%s_%s_quant.tflite" % (mobilenet_version, mobilenet_width, mobilenet_resolution)
labels = [line.rstrip('\n') for line in open("mobilenet_labels.txt")]

clock = time.clock()
while(True):
    clock.tick()

    img = sensor.snapshot()

    # net.classify() will run the network on an roi in the image (or on the whole image if the roi is not
    # specified). A classification score output vector will be generated for each location. At each scale the
    # detection window is moved around in the ROI using x_overlap (0-1) and y_overlap (0-1) as a guide.
    # If you set the overlap to 0.5 then each detection window will overlap the previous one by 50%. Note
    # the computational work load goes WAY up the more overlap. Finally, for multi-scale matching after
    # sliding the network around in the x/y dimensions the detection window will shrink by scale_mul (0-1)
    # down to min_scale (0-1). For example, if scale_mul is 0.5 the detection window will shrink by 50%.
    # Note that at a lower scale there's even more area to search if x_overlap and y_overlap are small...

    # default settings just do one detection... change them to search the image...
    # Setting x_overlap=-1 forces the window to stay centered in the ROI in the x direction always. If
    # y_overlap is not -1 the method will search in all vertical positions.
    # Setting y_overlap=-1 forces the window to stay centered in the ROI in the y direction always. If
    # x_overlap is not -1 the method will serach in all horizontal positions.

    for obj in tf.classify(mobilenet, img, min_scale=1.0, scale_mul=0.5, x_overlap=0.0, y_overlap=0.0):
        print("**********\nTop 5 Detections at [x=%d,y=%d,w=%d,h=%d]" % obj.rect())
        img.draw_rectangle(obj.rect())
        # This combines the labels and confidence values into a list of tuples
        # and then sorts that list by the confidence values.
        sorted_list = sorted(zip(labels, obj.output()), key = lambda x: x[1], reverse = True)
        for i in range(5):
            print("%s = %f" % (sorted_list[i][0], sorted_list[i][1]))
    print(clock.fps(), "fps")

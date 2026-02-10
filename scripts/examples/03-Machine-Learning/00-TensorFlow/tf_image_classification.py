# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# TensorFlow Lite Person Detection Example
#
# This example runs on the OpenMV RT1062 to detect people
# using the built-in MobileNet model.

import csi
import time
import ml
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)

model = ml.Model("/rom/person_detect.tflite")
print(model)

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    # This combines the labels and confidence values into a list of tuples
    # and then sorts that list by the confidence values.
    scores = sorted(
        zip(model.labels, model.predict([img])[0].flatten().tolist()),
        key=lambda x: x[1],
        reverse=True
    )

    print(clock.fps(), "fps\t", "%s = %f\t" % (scores[0][0], scores[0][1]))

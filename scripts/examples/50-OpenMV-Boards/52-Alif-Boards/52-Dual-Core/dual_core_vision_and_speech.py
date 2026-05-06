# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Dual-Core Vision and Speech Example
#
# This example runs BlazeFace face detection on the HP core and the MicroSpeech
# keyword spotter on the HE core simultaneously. The most recent heard
# keyword is drawn on the camera image.
#
# The built-in MicroSpeech model only recognizes the keywords "Yes" and "No".

import csi
import time
import openamp
import ml
from ml.postprocessing.mediapipe import BlazeFace


label = None
label_ticks = 0
LABEL_HOLD_MS = 2000


def task_callback(src_addr, data):
    global label, label_ticks
    label = data.decode()
    label_ticks = time.ticks_ms()


# This async function runs on the HE core.
@openamp.async_remote(task_callback)
async def task1(ept):
    from ml.apps import MicroSpeech
    speech = MicroSpeech(gain_db=24)
    while True:
        l, scores = speech.listen(timeout=0, threshold=0.70)
        if l:
            ept.send(l)


# Start the HE core before initializing the camera on the HP core.
rproc = openamp.RemoteProc(0x80320000)
rproc.start()

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)

# BlazeFace requires a square image for the best results.
csi0.window((400, 400))

model = ml.Model("/rom/blazeface_front_128.tflite", postprocess=BlazeFace(threshold=0.4))
print(model)

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    # faces is a list of ((x, y, w, h), score, keypoints) tuples
    for r, score, keypoints in model.predict([img]):
        ml.utils.draw_predictions(img, [r], ("face",), ((0, 0, 255),), format=None)
        ml.utils.draw_keypoints(img, keypoints, color=(255, 0, 0))

    if label is not None:
        if time.ticks_diff(time.ticks_ms(), label_ticks) < LABEL_HOLD_MS:
            img.draw_string((4, 4), f"Heard: {label}", color=(255, 0, 0), scale=2)
        else:
            label = None

    print(clock.fps(), "fps")

# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the FLIR Lepton 3.5 with a thermal
# super-resolution model, which upscales the 160x120 temperatures 4x to
# 640x480.
#
# The model runs at 160x120 and predicts, for each of the 4x4 output pixels of
# a Lepton pixel, blend weights over the 3x3 neighboring pixels (convex
# upsampling). The output is a weighted average of the measured temperatures.
#
# The model was trained on the PBVS and Tau2-101 thermal datasets from CIDIS,
# ESPOL. The training code is at https://github.com/openmv/openmv-ml

import csi
import image
import ml
import time
# https://micropython-ulab.readthedocs.io/en/latest/index.html
from ulab import numpy as np

# The model's sizes and input normalization.
W, H, SCALE = 160, 120, 4
OW, OH = W * SCALE, H * SCALE
MIN_STD = 0.5
LOGIT_SCALE, LOGIT_OFFSET = 2, 3

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)
csi0.framesize(csi.QQVGA)
# Measurement mode: the frames are temperatures.
csi0.ioctl(csi.IOCTL_LEPTON_SET_MODE, True)

model = ml.Model("/rom/lepton_sr.tflite")
print(model)

# The int8 weights are used as they are, so their scale and offset are the output's.
WEIGHT_SCALE = model.output_scale[0] * LOGIT_SCALE
WEIGHT_OFFSET = model.output_zero_point[0] + LOGIT_OFFSET / model.output_scale[0]


def update_features(features, temps):
    # The temperature normalized by the frame's mean and std, the model computes
    # the steps between pixels itself.
    k = 1 / (4 * max(np.std(temps), MIN_STD))
    features[:] = np.clip((temps - np.mean(temps)) * k, -1, 1)


def raw_outputs(model, inputs, outputs):
    # Returns the int8 outputs, they're converted to float without a callback.
    return outputs


temps = np.zeros((H, W), dtype=np.float)
features = np.zeros((H, W), dtype=np.float)

clock = time.clock()

# The model's output, 0-255, and the same with the color palette.
sr = image.Image(OW, OH, image.GRAYSCALE)
canvas = image.Image(OW, OH, image.RGB565)

while True:
    clock.tick()

    csi0.ioctl(csi.IOCTL_LEPTON_READ_TEMPS, temps)

    update_features(features, temps)
    weights = model.predict([features.reshape((1, H, W, 1))], callback=raw_outputs)[0]

    # Map the frame's range to 0-255 of the palette. The values are scaled before
    # upsampling, as there are fewer.
    lo, hi = np.min(temps), np.max(temps)
    scaled = (temps - lo) * (255 / max(hi - lo, 0.1))

    # Each output pixel is a weighted average of the 3x3 neighboring pixels.
    ml.convex_upsample(scaled, weights[0], SCALE, scale=WEIGHT_SCALE, offset=WEIGHT_OFFSET, out=sr)
    canvas.draw_image(sr, 0, 0, color_palette=image.PALETTE_IRONBOW)

    # Push the canvas to the jpeg buffer for the IDE to pull and display.
    canvas.flush()

    print("%.1f fps, %.1f to %.1f C" % (clock.fps(), lo, hi))

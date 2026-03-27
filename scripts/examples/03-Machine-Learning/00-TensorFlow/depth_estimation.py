# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# TensorFlow Fast Depth Example
#
# This example shows off monocular depth estimation.
#
# NOTE: This exaxmple requires an OpenMV Cam with an NPU like the AE3 or N6 to run real-time.

import csi
import time
import ml
import image
from ulab import numpy as np

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

# Load Depth Model
model = ml.Model("/rom/fast_depth_224.tflite")
print(model)

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    # Extract the depth array from the model output and reshape it to (h, w)
    depth_array = model.predict([img])[0].reshape(model.output_shape[0][1:3])

    # Scale the depth array to 0-255 and convert to an image for display
    depth_image = image.Image(depth_array * (255.0 / np.max(depth_array)))

    # Show the depth image
    img.draw_image(depth_image, alpha=255, color_palette=image.PALETTE_DEPTH,
                   hint=image.SCALE_ASPECT_IGNORE | image.BILINEAR)

    print(clock.fps(), "fps")

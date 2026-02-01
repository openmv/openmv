# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# STM32 CUBE.AI on OpenMV MNIST Example
# See https://github.com/openmv/openmv/blob/master/src/stm32cubeai/README.MD

import csi
import time
import nn_st
csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.contrast(3)
csi0.brightness(0)
csi0.auto_gain(True)
csi0.auto_exposure(True)
csi0.pixformat(csi.GRAYSCALE)  # Set pixel format to Grayscale
csi0.framesize(csi.QQQVGA)  # Set frame size to 80x60
csi0.snapshot(time=2000)  # Wait for settings take effect.
clock = time.clock()  # Create a clock object to track the FPS.

# [CUBE.AI] Initialize the network
net = nn_st.loadnnst("network")

nn_input_sz = 28  # The NN input is 28x28

while True:
    clock.tick()  # Update the FPS clock.
    img = csi0.snapshot()  # Take a picture and return the image.

    # Crop in the middle (avoids vignetting)
    img.crop(
        (
            img.width() // 2 - nn_input_sz // 2,
            img.height() // 2 - nn_input_sz // 2,
            nn_input_sz,
            nn_input_sz,
        )
    )

    # Binarize the image
    img.midpoint(2, bias=0.5, threshold=True, offset=5, invert=True)

    # [CUBE.AI] Run the inference
    out = net.predict(img)
    print("Network argmax output: {}".format(out.index(max(out))))
    img.draw_string(0, 0, str(out.index(max(out))))
    print(
        "FPS {}".format(clock.fps())
    )  # Note: OpenMV Cam runs about half as fast when connected

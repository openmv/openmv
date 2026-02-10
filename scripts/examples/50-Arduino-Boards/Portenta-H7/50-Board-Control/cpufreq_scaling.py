# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# CPU frequency scaling example.
#
# This example shows how to use the cpufreq module to change the CPU frequency on the fly.
import csi
import image
import time
import cpufreq

csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.pixformat(csi.GRAYSCALE)  # Set pixel format to RGB565 (or GRAYSCALE)
csi0.framesize(csi.QVGA)  # Set frame size to QVGA (320x240)
clock = time.clock()  # Create a clock object to track the FPS.


def test_image_processing():
    for i in range(0, 50):
        clock.tick()  # Update the FPS clock.
        img = csi0.snapshot()  # Take a picture and return the image.
        img.find_edges(image.EDGE_CANNY, threshold=(50, 80))


print("\nFrequency Scaling Test...")
for f in cpufreq.get_supported_frequencies():
    print("Testing CPU Freq: %dMHz..." % (f))
    cpufreq.set_frequency(f)
    clock.reset()
    test_image_processing()
    freqs = cpufreq.get_current_frequencies()
    print(
        "CPU Freq:%dMHz HCLK:%dMhz PCLK1:%dMhz PCLK2:%dMhz FPS:%.2f"
        % (freqs[0], freqs[1], freqs[2], freqs[3], clock.fps())
    )

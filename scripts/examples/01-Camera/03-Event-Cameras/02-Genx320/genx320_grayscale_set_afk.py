# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off using the GenX320 event sensor from Prophesee and controlling
# AntiFlicKer (AFK) filter bloc in the GenX320 digital pipeline.
# AFK allows to detect periodic changes of given frequencies in the scene and filter them out.

import sensor
import time

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)  # Must always be grayscale.
sensor.set_framesize(sensor.B320X320)  # Must always be 320x320.
sensor.set_brightness(128)  # Leave at 128 generally (this is the default).
sensor.set_contrast(16)  # Increase to make the image pop.
sensor.set_framerate(100)

# Enables AFK filter to remove periodic data in the frequency range from 130Hz to 160Hz
sensor.ioctl(sensor.IOCTL_GENX320_SET_AFK, 1, 130, 160)
# Disables AFK filter
# sensor.ioctl(sensor.IOCTL_GENX320_SET_AFK, 0)

clock = time.clock()

while True:
    clock.tick()

    img = sensor.snapshot()

    print(clock.fps())

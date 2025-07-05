# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off hot pixel calibration
# using the genx320 event camera from Prophesee.

import sensor
import time

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)  # Must always be grayscale.
sensor.set_framesize(sensor.B320X320)  # Must always be 320x320.
sensor.set_brightness(128)  # Leave at 128 generally (this is the default).
sensor.set_contrast(16)  # Increase to make the image pop.

# The default frame rate is 50 FPS. You can change it between ~20 FPS and ~350 FPS.
sensor.set_framerate(50)

# Show uncalibrated image first.
sensor.skip_frames(time=5000)

CALIBRATION_EVENT_COUNT = 100  # Number of events to collect for calibration.
CALIBRATION_SIGMA = 0.5  # Standard deviation for hot pixel detection.
disabled_pixels = sensor.ioctl(sensor.IOCTL_GENX320_CALIBRATE,
                               CALIBRATION_EVENT_COUNT, CALIBRATION_SIGMA)
print(f'Disabled {disabled_pixels} hot pixels.')

clock = time.clock()

while True:
    clock.tick()

    img = sensor.snapshot()
    # img.median(1) # noise cleanup.

    print(clock.fps())

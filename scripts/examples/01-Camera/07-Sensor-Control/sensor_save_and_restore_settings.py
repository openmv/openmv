# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Sensor Save and Restore Settings
#
# This example shows off how to save and then restore camera settings
# after a sensor reset and/or powerdown.

import csi
import time

# Set pixel format to RGB565 (or GRAYSCALE)
target_pixformat = csi.RGB565

# Set frame size to QVGA (320x240)
target_framesize = csi.QVGA

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(target_pixformat)
csi0.framesize(target_framesize)

# Delay 3 seconds to let the camera auto functions run and measure the fps.
t = time.ticks_ms()
clock = time.clock()
fps = 0
while time.ticks_diff(time.ticks_ms(), t) < 3000:
    clock.tick()
    csi0.snapshot()
    fps = clock.fps()

# The sensor should have time now to warm up and adjust to the current environment.
# Grab the settings now before turning off the sensor.

gain = csi0.gain_db()
exposure = csi0.exposure_us()
rgb_gain = csi0.rgb_gain_db()

print("Gain == %f db" % gain)
print("Exposure == %d us" % exposure)
print("RGB Gain == %f db, %f db, %f db" % (rgb_gain[0], rgb_gain[1], rgb_gain[2]))

print("Powering Off Sensor")
csi0.shutdown(True)
time.sleep(2)
del csi0

# Disable all settling time delays in the sensor driver code. Turning this off WILL result in
# corrupted images appearing when modifying settings. However, now you can change sensor settings
# in bulk quickly.

ts = time.ticks_ms()
print("Powering On Sensor")
csi0 = csi.CSI(delays=False)
csi0.reset()
csi0.pixformat(target_pixformat)
csi0.framesize(target_framesize)
csi0.auto_gain(False, gain_db=gain)
csi0.auto_exposure(False, exposure_us=exposure)
csi0.auto_whitebal(False, rgb_gain_db=rgb_gain)

# You need to delay before calling snapshot as the image coming out of the camera right now is
# most likely to be very corrupt. The exact amount of time to delay below is application
# dependent. However, you should probably wait for a frame.
time.sleep(1.0 / fps)

print("Restore Delay %d ms" % (time.ticks_ms() - ts))

gain = csi0.gain_db()
exposure = csi0.exposure_us()
rgb_gain = csi0.rgb_gain_db()

print("Gain == %f db" % gain)
print("Exposure == %d us" % exposure)
print("RGB Gain == %f db, %f db, %f db" % (rgb_gain[0], rgb_gain[1], rgb_gain[2]))

clock = time.clock()

# Image should look like it did before save and restore.
while True:
    clock.tick()
    img = csi0.snapshot()
    # print(clock.fps())

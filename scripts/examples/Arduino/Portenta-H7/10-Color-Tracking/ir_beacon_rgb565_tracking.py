# IR Beacon RGB565 Tracking Example
#
# This example shows off IR beacon RGB565 tracking using the OpenMV Cam.

import sensor, image, time

thresholds = (100, 100, 0, 0, 0, 0) # thresholds for bright white light from IR.

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.VGA)
sensor.set_windowing((240, 240)) # 240x240 center pixels of VGA
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False) # must be turned off for color tracking
clock = time.clock()

# Only blobs that with more pixels than "pixel_threshold" and more area than "area_threshold" are
# returned by "find_blobs" below. Change "pixels_threshold" and "area_threshold" if you change the
# camera resolution. "merge=True" merges all overlapping blobs in the image.

while(True):
    clock.tick()
    img = sensor.snapshot()
    for blob in img.find_blobs([thresholds], pixels_threshold=200, area_threshold=200, merge=True):
        ratio = blob.w() / blob.h()
        if (ratio >= 0.5) and (ratio <= 1.5): # filter out non-squarish blobs
            img.draw_rectangle(blob.rect())
            img.draw_cross(blob.cx(), blob.cy())
    print(clock.fps())

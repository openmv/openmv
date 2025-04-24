# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Lepton Get Object Temp Example
#
# This example shows off how to get an object's temperature using color tracking.

# By turning the AGC off and setting a max and min temperature range you can make the lepton into
# a great sensor for seeing objects of a particular temperature. That said, the FLIR lepton is a
# microblobometer and not a thermophile. So, it needs to re-calibrate itself often (which is called
# flat-field-correction - FFC). Additionally, microblobmeter devices require pprocessing support
# onboard to deal with the effects of temperature drift which is called radiometry support.

# FLIR Lepton Shutter Note: FLIR Leptons with radiometry and a shutter will pause the video often
# as they heatup to re-calibrate. This will happen less and less often as the sensor temperature
# stabilizes. You can force the re-calibration to not happen if you need to via the lepton API.
# However, it is not recommended because the image will degrade overtime.

# If you are using a LEPTON other than the Lepton 3.5 this script may not work perfectly as other
# leptons don't have radiometry support or they don't activate their calibration process often
# enough to deal with temperature changes (FLIR 2.5).

import sensor
import time
import display
import image

# Color Tracking Thresholds (Grayscale Min, Grayscale Max)
threshold_list = [(200, 255)]

# Set the target temp range here
min_temp_in_celsius = 20.0
max_temp_in_celsius = 40.0

print("Resetting Lepton...")
# These settings are applied on reset
sensor.reset()
sensor.ioctl(sensor.IOCTL_LEPTON_SET_MODE, True)
sensor.ioctl(
    sensor.IOCTL_LEPTON_SET_RANGE, min_temp_in_celsius, max_temp_in_celsius
)
print(
    "Lepton Res (%dx%d)"
    % (
        sensor.ioctl(sensor.IOCTL_LEPTON_GET_WIDTH),
        sensor.ioctl(sensor.IOCTL_LEPTON_GET_HEIGHT),
    )
)
print(
    "Radiometry Available: "
    + ("Yes" if sensor.ioctl(sensor.IOCTL_LEPTON_GET_RADIOMETRY) else "No")
)

sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.LCD)
sensor.skip_frames(time=5000)
clock = time.clock()
lcd = display.SPIDisplay()

# Only blobs that with more pixels than "pixel_threshold" and more area than "area_threshold" are
# returned by "find_blobs" below. Change "pixels_threshold" and "area_threshold" if you change the
# camera resolution. "merge=True" merges all overlapping blobs in the image.


def map_g_to_temp(g):
    return (
        (g * (max_temp_in_celsius - min_temp_in_celsius)) / 255.0
    ) + min_temp_in_celsius


while True:
    clock.tick()
    img = sensor.snapshot()
    blob_stats = []
    blobs = img.find_blobs(
        threshold_list, pixels_threshold=200, area_threshold=200, merge=True
    )
    # Collect stats into a list of tuples
    for blob in blobs:
        blob_stats.append(
            (
                blob.x(),
                blob.y(),
                map_g_to_temp(
                    img.get_statistics(
                        thresholds=threshold_list, roi=blob.rect()
                    ).mean()
                ),
            )
        )
    img.to_rainbow(color_palette=image.PALETTE_IRONBOW)  # color it
    # Draw stuff on the colored image
    for blob in blobs:
        img.draw_rectangle(blob.rect())
        img.draw_cross(blob.cx(), blob.cy())
    for blob_stat in blob_stats:
        img.draw_string(
            blob_stat[0], blob_stat[1] - 10, "%.2f C" % blob_stat[2], mono_space=False
        )
    lcd.write(img)
    print(
        "FPS %f - Lepton Temp: %f C"
        % (clock.fps(), sensor.ioctl(sensor.IOCTL_LEPTON_GET_FPA_TEMP))
    )

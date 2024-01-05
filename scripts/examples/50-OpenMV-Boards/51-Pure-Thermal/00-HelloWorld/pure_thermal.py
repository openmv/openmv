# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Pure Thermal Example Script
#
# Thanks for buying the Pure Thermal OpenMV! This example script shows off thermal video
# overlay onto the color camera image and driving the attached LCD screen and HDMI output.

import sensor
import image
import time
import display
import fir
import math
import tfp410


# Color Tracking Thresholds (Grayscale Min, Grayscale Max)
threshold_list = [(200, 255)]

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.WVGA)
time.sleep_ms(50)

fir.init(fir.FIR_LEPTON)
fir_img = image.Image(fir.width(), fir.height(), sensor.GRAYSCALE)
time.sleep_ms(50)

lcd = display.RGBDisplay(framesize=display.FWVGA, refresh=60)
lcd.backlight(True)
hdmi = tfp410.TFP410()
time.sleep_ms(50)

alpha_pal = image.Image(256, 1, sensor.GRAYSCALE)
for i in range(256):
    alpha_pal[i] = int(math.pow((i / 255), 2) * 255)

to_min = None
to_max = None


def map_g_to_temp(g):
    return ((g * (to_max - to_min)) / 255.0) + to_min


while True:
    img = sensor.snapshot()
    # ta: Ambient temperature
    # ir: Object temperatures (IR array)
    # to_min: Minimum object temperature
    # to_max: Maximum object temperature
    ta, ir, to_min, to_max = fir.read_ir()

    fir.draw_ir(fir_img, ir, color_palette=None)
    fir_img_size = fir_img.width() * fir_img.height()

    # Find IR Blobs
    blobs = fir_img.find_blobs(threshold_list,
                               pixels_threshold=(fir_img_size // 100),
                               area_threshold=(fir_img_size // 100),
                               merge=True)

    # Collect stats into a list of tuples
    blob_stats = []
    for b in blobs:
        blob_stats.append((b.rect(), map_g_to_temp(img.get_statistics(thresholds=threshold_list,
                                                                      roi=b.rect()).mean())))
    x_scale = img.width() / fir_img.width()
    y_scale = img.height() / fir_img.height()
    img.draw_image(fir_img, 0, 0, x_scale=x_scale, y_scale=y_scale,
                   color_palette=image.PALETTE_IRONBOW,
                   alpha_palette=alpha_pal,
                   hint=image.BICUBIC)

    # Draw stuff on the colored image
    for b in blobs:
        img.draw_rectangle(int(b.rect()[0] * x_scale), int(b.rect()[1] * y_scale),
                           int(b.rect()[2] * x_scale), int(b.rect()[3] * y_scale))
        img.draw_cross(int(b.cx() * x_scale), int(b.cy() * y_scale))
    for blob_stat in blob_stats:
        img.draw_string(int((blob_stat[0][0] * x_scale) + 4), int((blob_stat[0][1] * y_scale) + 1),
                        '%.2f C' % blob_stat[1], mono_space=False, scale=2)

    # Draw ambient, min and max temperatures.
    img.draw_string(4, 0, 'Lepton Temp: %0.2f C' % ta,
                    color=(255, 255, 255), mono_space=False, scale=2)
    img.draw_string(4, 18, 'Min Temp: %0.2f C' % to_min,
                    color=(255, 255, 255), mono_space=False, scale=2)
    img.draw_string(4, 36, 'Max Temp: %0.2f C' % to_max,
                    color=(255, 255, 255), mono_space=False, scale=2)

    lcd.write(img, hint=(image.BILINEAR | image.CENTER | image.SCALE_ASPECT_KEEP))

# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Boson Temperature Image Example
#
# This example shows off how to make a temperature image from the FLIR Boson's 16-bit video,
# like the FLIR Lepton's measurement mode. Each frame is read into a ndarray and mapped onto a
# fixed temperature range, so a color always means the same temperature, and the hottest and
# coldest points are marked with their temperatures.
#
# * Radiometric cores (an "R" in the part number) output T-Linear data, the temperature of each
#   pixel in 0.01 Kelvin units.
# * Non-radiometric cores output raw counts, which are converted with the same approximate model
#   as boson_get_object_temp.py. Run boson_temp_calibration.py first, the shipped response gains
#   are only a starting point and are often far off for a given camera.

import csi
import image
import json
import math
import time
from ulab import numpy as np

MIN_TEMP_C = 20.0  # Temperature mapped to the bottom of the color palette.
MAX_TEMP_C = 40.0  # Temperature mapped to the top of the color palette.

# What nearly every real target is (skin, paint, tape, plastic).
EMISSIVITY = 0.95
AMBIENT_C = 22.0  # The room temperature reflected by the target.

KELVIN_OFFSET = 273.15

# Counts -> temperature model for non-radiometric cores, see boson_get_object_temp.py.
ANCHOR_COUNTS = 32768
PLANCK_B = 1450.0
RESPONSE_GAINS = {640: 1.2426155e06, 320: 1.8934190e06}  # High gain.

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)  # Must always be grayscale.
csi0.framesize(csi.QVGA if csi0.cid() == csi.BOSON320 else csi.VGA)
w, h = csi0.width(), csi0.height()  # Get the size before raw mode doubles the capture width.

radiometric = csi0.ioctl(csi.IOCTL_BOSON_GET_RADIOMETRY_CAPABLE)
print("Radiometry: " + ("Yes" if radiometric else "No"))

# T-Linear and the counts calibration are both for high gain.
csi0.ioctl(csi.IOCTL_BOSON_SET_GAIN_MODE, csi.BOSON_GAIN_HIGH)

response_gain = RESPONSE_GAINS[w]
if radiometric:
    try:
        # The camera takes emissivity as a percentage (50-100).
        csi0.ioctl(csi.IOCTL_BOSON_SET_EMISSIVITY, EMISSIVITY * 100.0)
        csi0.ioctl(csi.IOCTL_BOSON_SET_TEMP_BACKGROUND, AMBIENT_C + KELVIN_OFFSET)
    except (OSError, RuntimeError):
        pass
else:
    # A per-unit calibration saved by boson_temp_calibration.py overrides the default.
    try:
        with open("boson_cal.json") as f:
            response_gain = float(json.load(f)["high"])
        print("Using boson_cal.json")
    except (OSError, ValueError, KeyError):
        print("No boson_cal.json - temperatures are uncalibrated")

csi0.ioctl(csi.IOCTL_BOSON_SET_MODE, csi.BOSON_MODE_RAW)


def radiance(kelvin):
    # Planck band radiance at kelvin, in the sensor's arbitrary units.
    return 1.0 / (math.exp(PLANCK_B / kelvin) - 1.0)


def to_counts(celsius, fpa_kelvin):
    if radiometric:
        return (celsius + KELVIN_OFFSET) * 100.0
    seen = (EMISSIVITY * radiance(celsius + KELVIN_OFFSET)) + \
        ((1.0 - EMISSIVITY) * radiance(AMBIENT_C + KELVIN_OFFSET))
    return ANCHOR_COUNTS + (response_gain * (seen - radiance(fpa_kelvin)))


def to_celsius(counts, fpa_kelvin):
    if radiometric:
        return (counts / 100.0) - KELVIN_OFFSET
    seen = radiance(fpa_kelvin) + ((counts - ANCHOR_COUNTS) / response_gain)
    obj = (seen - ((1.0 - EMISSIVITY) * radiance(AMBIENT_C + KELVIN_OFFSET))) / EMISSIVITY
    try:
        return (PLANCK_B / math.log((1.0 / obj) + 1.0)) - KELVIN_OFFSET
    except (ValueError, ZeroDivisionError):
        return float("nan")


frame = np.zeros((h, w), dtype=np.uint16)
scaled = np.zeros((h, w), dtype=np.float)
gray_buf = bytearray(w * h)
img = image.Image(w, h, image.RGB565)
unit = "C" if radiometric else "C~"  # "~" marks approximate temperatures.
fpa_kelvin = 0.0
fpa_ticks = time.ticks_add(time.ticks_ms(), -1000)
clock = time.clock()

while True:
    clock.tick()
    csi0.ioctl(csi.IOCTL_BOSON_READ_FRAME, frame)

    # The counts are relative to the camera's own temperature, which drifts slowly. Reading it
    # takes a CCI transaction (~20ms), so only refresh it once a second.
    if (not radiometric) and (time.ticks_diff(time.ticks_ms(), fpa_ticks) >= 1000):
        fpa_kelvin = csi0.ioctl(csi.IOCTL_BOSON_GET_FPA_TEMP) + KELVIN_OFFSET
        fpa_ticks = time.ticks_ms()

    # Map the temperature range to 0-255, image.Image() clamps values outside it.
    lo = to_counts(MIN_TEMP_C, fpa_kelvin)
    hi = to_counts(MAX_TEMP_C, fpa_kelvin)
    # In-place ops reuse one buffer, a 640x512 float frame is 1.3MB.
    scaled *= 0.0
    scaled += frame
    scaled -= lo
    scaled *= 255.0 / (hi - lo)
    gray = image.Image(scaled, buffer=gray_buf)
    img.draw_image(gray, 0, 0, color_palette=image.PALETTE_IRONBOW)

    hot = np.argmax(frame)
    cold = np.argmin(frame)
    for i, color in ((hot, (255, 255, 255)), (cold, (0, 255, 255))):
        x = i % w
        y = i // w
        img.draw_cross((x, y), color=color, size=8)
        img.draw_string((min(x + 6, w - 64), max(y - 14, 0)),
                        "%.1f %s" % (to_celsius(frame[y, x], fpa_kelvin), unit), color=color)

    img.draw_string((4, h - 14), "%.0f-%.0f %s" % (MIN_TEMP_C, MAX_TEMP_C, unit), color=(255, 255, 255))
    img.flush()

    print("FPS %.2f - Center: %.2f %s" % (
        clock.fps(), to_celsius(frame[h // 2, w // 2], fpa_kelvin), unit))

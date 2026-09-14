# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Boson Temperature Calibration Example
#
# One-point calibration for Bosons WITHOUT factory radiometric calibration.
# boson_get_object_temp.py converts spot meter counts to temperature with:
#
#   counts = ANCHOR + GAIN * (L(T) - L(T_fpa))    L(T) = 1 / (exp(B/T) - 1)
#
# The anchor and B are fixed by the sensor; the GAIN is per-unit (and per gain
# state) and is what this example measures. Fill the spot meter box with a
# reference whose temperature you know and run this script. The reference to
# reach for is melting ice water: 0 C by physics rather than by someone's
# estimate, matte (so reflections barely matter), and far from the camera's
# own temperature, which is where the fit has the most leverage.
#
# The result is printed and saved to boson_cal.json, which
# boson_get_object_temp.py loads automatically.

import csi
import image
import json
import math
import time

REFERENCE_C = 0.0  # the reference target's temperature (melting ice water)
REFERENCE_EMISSIVITY = 0.97  # ice; a matte reference keeps this uncritical
AMBIENT_C = 22.0  # the room reflected by the reference
SAMPLES = 64  # spot meter samples averaged for the fit

ANCHOR_COUNTS = 32768
PLANCK_B = 1450.0
KELVIN_OFFSET = 273.15

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)  # Must always be grayscale.
csi0.framesize(csi.QVGA if csi0.cid() == csi.BOSON320 else csi.VGA)
csi0.color_palette(image.PALETTE_IRONBOW)

major, minor, patch = csi0.ioctl(csi.IOCTL_BOSON_GET_SOFTWARE_REV)
if major < 3:
    raise Exception("Boson firmware 2.x predates the spot meter")
if csi0.ioctl(csi.IOCTL_BOSON_GET_RADIOMETRY_CAPABLE):
    raise Exception("This core is factory calibrated - no host calibration needed")

# The response gain is per gain state; pin high gain (the state every camera
# idles in, and the one boson_get_object_temp.py resolves auto mode to). Set
# BOSON_GAIN_LOW here to calibrate the low-gain slot instead.
GAIN_MODE = csi.BOSON_GAIN_HIGH
csi0.ioctl(csi.IOCTL_BOSON_SET_GAIN_MODE, GAIN_MODE)

# Point the spot meter at the center of the frame - aim that at the reference.
max_w, max_h = csi0.ioctl(csi.IOCTL_BOSON_GET_SPOT_METER_ROI_MAX)
roi = ((csi0.width() - max_w) // 2, (csi0.height() - max_h) // 2, max_w, max_h)
csi0.ioctl(csi.IOCTL_BOSON_SET_SPOT_METER_ENABLE, True)
csi0.ioctl(csi.IOCTL_BOSON_SET_SPOT_METER_ROI, roi)

# The counts are offset-corrected against the shutter at the last FFC, so run
# one now to take that baseline at the camera's current temperature.
csi0.ioctl(csi.IOCTL_BOSON_RUN_FFC)
for _ in range(100):
    if csi0.ioctl(csi.IOCTL_BOSON_GET_FFC_STATUS) == csi.BOSON_FFC_STATUS_COMPLETE:
        break
    time.sleep_ms(50)


def radiance(kelvin):
    # Planck band radiance at kelvin, in the sensor's arbitrary units.
    return 1.0 / (math.exp(PLANCK_B / kelvin) - 1.0)


# Average the spot meter counts and FPA temperature over a few seconds.
print("Sampling the reference - keep it filling the box...")
counts = 0.0
fpa = 0.0
for i in range(SAMPLES):
    img = csi0.snapshot()
    img.draw_rectangle(roi, color=(255, 255, 255))
    counts += csi0.ioctl(csi.IOCTL_BOSON_GET_SPOT_METER_STATS)[0]
    fpa += csi0.ioctl(csi.IOCTL_BOSON_GET_FPA_TEMP)
counts /= SAMPLES
fpa_kelvin = (fpa / SAMPLES) + KELVIN_OFFSET
ref_kelvin = REFERENCE_C + KELVIN_OFFSET

# Too close to the camera's own temperature and every gain fits equally well:
# the counts delta is tiny and the slope it implies is dominated by noise.
if abs(ref_kelvin - fpa_kelvin) < 5.0:
    raise Exception("Reference is within 5 C of the camera - use a colder or hotter one")

# The camera saw the reference diluted by its emissivity plus the room
# reflected in it, so fit the gain against what it actually saw:
#   counts = ANCHOR + GAIN * (L_seen - L(T_fpa))
seen = ((REFERENCE_EMISSIVITY * radiance(ref_kelvin))
        + ((1.0 - REFERENCE_EMISSIVITY) * radiance(AMBIENT_C + KELVIN_OFFSET)))
gain = (counts - ANCHOR_COUNTS) / (seen - radiance(fpa_kelvin))
if gain <= 0.0:
    raise Exception("Fit failed (gain %f) - is the reference filling the box?" % gain)

state = "high" if GAIN_MODE == csi.BOSON_GAIN_HIGH else "low"
print("counts %.0f, FPA %.2f C -> response gain %.6e (%s gain)" % (
    counts, fpa_kelvin - KELVIN_OFFSET, gain, state))

# Save for boson_get_object_temp.py, one slot per gain state.
try:
    with open("boson_cal.json") as f:
        cal = json.load(f)
except (OSError, ValueError):
    cal = {}
cal[state] = gain
with open("boson_cal.json", "w") as f:
    json.dump(cal, f)
print("Saved to boson_cal.json")

# Read the reference back with the new gain - it should sit near REFERENCE_C.
clock = time.clock()

while True:
    clock.tick()

    # The filesystem write above can stall parallel-bus frame capture (the
    # capture is re-armed by the snapshot after a timeout), so retry on it.
    try:
        img = csi0.snapshot()
    except RuntimeError:
        continue

    try:
        counts = csi0.ioctl(csi.IOCTL_BOSON_GET_SPOT_METER_STATS)[0]
        fpa_kelvin = csi0.ioctl(csi.IOCTL_BOSON_GET_FPA_TEMP) + KELVIN_OFFSET
        seen = radiance(fpa_kelvin) + ((counts - ANCHOR_COUNTS) / gain)
        obj = (seen - ((1.0 - REFERENCE_EMISSIVITY)
                       * radiance(AMBIENT_C + KELVIN_OFFSET))) / REFERENCE_EMISSIVITY
        kelvin = PLANCK_B / math.log((1.0 / obj) + 1.0)
        print("FPS %.2f - reference reads %.2f C" % (clock.fps(), kelvin - KELVIN_OFFSET))
    except (OSError, RuntimeError, ValueError, ZeroDivisionError):
        print("FPS %.2f - no reading" % clock.fps())

    img.draw_rectangle(roi, color=(255, 255, 255))

# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Boson Get Object Temp Example
#
# This example shows off how to measure an object's temperature with the FLIR Boson's
# hardware spot meter. It works on every FLIR Boson and Boson+ (320 or 640):
#
# * Radiometric cores (an "R" in the part number) report calibrated temperatures directly.
# * Non-radiometric cores still report calibrated-ish temperatures once TLinear is enabled,
#   and if the camera refuses, the spot meter's raw counts are converted to an approximate
#   temperature on the host (the same compromise the FLIR Lepton driver makes).
# * Cameras running Boson application firmware 2.x predate the whole spot meter and
#   radiometry command set, so no temperature can be measured over CCI at all.

import csi
import image
import json
import math
import time

# What nearly every real target is (skin, paint, tape, plastic). Only shiny
# metal is far off this - and there it matters enormously.
EMISSIVITY = 0.95
AMBIENT_C = 22.0  # The room temperature reflected by the target.

KELVIN_OFFSET = 273.15

# Counts -> temperature model for the fallback path. The spot meter counts are
# taken at the end of the Boson's 16-bit pipeline and are offset-corrected
# against the shutter at the last FFC, so mid-scale (2^15) means "as warm as
# the camera". The deviation from mid-scale is a measurement of radiance, which
# converts to temperature through Planck's law:
#
#   counts = ANCHOR + GAIN * (L(T) - L(T_fpa))    L(T) = 1 / (exp(B/T) - 1)
#
# B is fixed by the sensor's 8-14um spectral band (B = 14388/lambda_eff). The
# gain is per-unit and per-gain-state - the values below were fitted on the
# bench against melting ice and are only a starting point.
ANCHOR_COUNTS = 32768
PLANCK_B = 1450.0
RESPONSE_GAINS = {
    640: {csi.BOSON_GAIN_HIGH: 1.2426155e06, csi.BOSON_GAIN_LOW: 1.4718071e06},
    320: {csi.BOSON_GAIN_HIGH: 1.8934190e06, csi.BOSON_GAIN_LOW: 1.3338160e06},
}

# A per-unit calibration saved by boson_temp_calibration.py overrides the
# shipped defaults - run that example against melting ice water to make one.
try:
    with open("boson_cal.json") as f:
        cal = json.load(f)
    for gains in RESPONSE_GAINS.values():
        for state, key in (("high", csi.BOSON_GAIN_HIGH), ("low", csi.BOSON_GAIN_LOW)):
            if state in cal:
                gains[key] = float(cal[state])
except (OSError, ValueError):
    pass

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.GRAYSCALE)  # Must always be grayscale.
csi0.framesize(csi.QVGA if csi0.cid() == csi.BOSON320 else csi.VGA)
csi0.color_palette(image.PALETTE_IRONBOW)

# Boson application firmware 2.x NAKs the entire spot meter and radiometry
# command set, so there is no source of 16-bit thermal data over CCI.
major, minor, patch = csi0.ioctl(csi.IOCTL_BOSON_GET_SOFTWARE_REV)
print("Boson firmware: %d.%d.%d" % (major, minor, patch))
if major < 3:
    raise Exception("Boson firmware 2.x predates the spot meter - cannot measure temperature")

# Radiometric capable means the core shipped with a factory radiometric
# calibration. A core without it still answers most of the calls below, but
# its temperatures are approximate at best.
radiometric = csi0.ioctl(csi.IOCTL_BOSON_GET_RADIOMETRY_CAPABLE)
print("Radiometry: " + ("Yes" if radiometric else "No"))

# TLinear and temperature stabilization are the engine behind every temperature
# reading, and the driver's factory-restore clears them on reset. A NAK here
# just means the camera cannot run them - the counts fallback still works.
try:
    csi0.ioctl(csi.IOCTL_BOSON_SET_TLINEAR_ENABLE, True)
    csi0.ioctl(csi.IOCTL_BOSON_SET_TEMP_STABLE_ENABLE, True)
    # The camera takes emissivity as a percentage (50-100).
    csi0.ioctl(csi.IOCTL_BOSON_SET_EMISSIVITY, EMISSIVITY * 100.0)
    csi0.ioctl(csi.IOCTL_BOSON_SET_TEMP_BACKGROUND, AMBIENT_C + KELVIN_OFFSET)
except (OSError, RuntimeError):
    pass

# Point the hardware spot meter at the center of the frame, clamped to this
# camera's maximum ROI (128x128 on the 640s, 64x64 on the 320s).
max_w, max_h = csi0.ioctl(csi.IOCTL_BOSON_GET_SPOT_METER_ROI_MAX)
roi = ((csi0.width() - max_w) // 2, (csi0.height() - max_h) // 2, max_w, max_h)
csi0.ioctl(csi.IOCTL_BOSON_SET_SPOT_METER_ENABLE, True)
csi0.ioctl(csi.IOCTL_BOSON_SET_SPOT_METER_ROI, roi)
try:
    # Kelvin so the temperature stats are unit-unambiguous.
    csi0.ioctl(csi.IOCTL_BOSON_SET_SPOT_METER_MODE, csi.BOSON_SPOT_METER_KELVIN)
except (OSError, RuntimeError):
    pass


def plausible(kelvin):
    # An uncooled microbolometer cannot see anywhere near these bounds, so a
    # value outside them is a bad read (or a bad conversion), not a real scene.
    return (kelvin is not None) and (200.0 < kelvin < 1000.0)


def radiance(kelvin):
    # Planck band radiance at kelvin, in the sensor's arbitrary units.
    return 1.0 / (math.exp(PLANCK_B / kelvin) - 1.0)


def counts_to_kelvin(counts, fpa_kelvin, gain_mode):
    # Best available counts conversion, in order of decreasing trust. The Planck
    # paths are only meaningful on a radiometrically calibrated core - a core
    # without one still answers them, with confidently wrong numbers.
    if radiometric:
        # The camera's own Planck solver (counts are an argument, not read from
        # the sensor), then the same maths on our side with the camera's RBFO
        # coefficients.
        try:
            kelvin = csi0.ioctl(csi.IOCTL_BOSON_GET_TEMP_FROM_COUNTS, csi.BOSON_RBFO_DEFAULT, counts)
            if plausible(kelvin):
                return kelvin
        except (OSError, RuntimeError):
            pass
        try:
            r, b, f, o = csi0.ioctl(csi.IOCTL_BOSON_GET_RBFO, csi.BOSON_RBFO_FACTORY,
                                    gain_mode == csi.BOSON_GAIN_LOW)
            kelvin = b / math.log((r / (counts - o)) + f)
            if plausible(kelvin):
                return kelvin
        except (OSError, RuntimeError, ValueError, ZeroDivisionError):
            pass
    # The anchor-and-delta model above, pivoted on the FPA temperature, which is
    # re-read every frame so the reading follows the camera as it self-heats.
    # The response differs ~1.4x between gain states (auto resolves to high
    # gain). Works on every Boson with the spot meter - approximate, but useful.
    gains = RESPONSE_GAINS[320 if csi0.cid() == csi.BOSON320 else 640]
    gain = gains.get(gain_mode, gains[csi.BOSON_GAIN_HIGH])
    try:
        seen = radiance(fpa_kelvin) + ((counts - ANCHOR_COUNTS) / gain)
        # Strip out the room reflected by the target:
        # L_seen = e * L(T_obj) + (1 - e) * L(T_ambient)
        obj = (seen - ((1.0 - EMISSIVITY) * radiance(AMBIENT_C + KELVIN_OFFSET))) / EMISSIVITY
        kelvin = PLANCK_B / math.log((1.0 / obj) + 1.0)
    except (ValueError, ZeroDivisionError, OverflowError):
        return None
    return kelvin if plausible(kelvin) else None


temp_stats_ok = True


def spot_meter_kelvin():
    # Temperature stats, falling back to counts + conversion on a NAK. Returns
    # (mean, min, max, (min_x, min_y), (max_x, max_y), approximate).
    global temp_stats_ok
    if temp_stats_ok:
        try:
            mean, dev, min_t, min_x, min_y, max_t, max_x, max_y = csi0.ioctl(
                csi.IOCTL_BOSON_GET_SPOT_METER_TEMP_STATS)
            return mean, min_t, max_t, (min_x, min_y), (max_x, max_y), not radiometric
        except (OSError, RuntimeError):
            # Do not keep asking a camera that has told us no.
            temp_stats_ok = False
            print("Temp stats unavailable - falling back to counts conversion")
    mean, dev, min_c, min_x, min_y, max_c, max_x, max_y = csi0.ioctl(
        csi.IOCTL_BOSON_GET_SPOT_METER_STATS)
    # Read the shared conversion inputs once per frame, not once per value.
    fpa_kelvin = csi0.ioctl(csi.IOCTL_BOSON_GET_FPA_TEMP) + KELVIN_OFFSET
    try:
        gain_mode = csi0.ioctl(csi.IOCTL_BOSON_GET_GAIN_MODE)
    except (OSError, RuntimeError):
        gain_mode = csi.BOSON_GAIN_HIGH
    return (counts_to_kelvin(mean, fpa_kelvin, gain_mode),
            counts_to_kelvin(min_c, fpa_kelvin, gain_mode),
            counts_to_kelvin(max_c, fpa_kelvin, gain_mode),
            (min_x, min_y), (max_x, max_y), True)


clock = time.clock()
last = None  # last good reading: (mean, min_t, max_t, max_loc, approximate)

while True:
    clock.tick()
    img = csi0.snapshot()

    try:
        mean, min_t, max_t, min_loc, max_loc, approximate = spot_meter_kelvin()
        if plausible(mean) and plausible(min_t) and plausible(max_t):
            last = (mean, min_t, max_t, max_loc, approximate)
            print("FPS %.2f - mean %.2f C, min %.2f C, max %.2f C%s" % (
                clock.fps(), mean - KELVIN_OFFSET, min_t - KELVIN_OFFSET,
                max_t - KELVIN_OFFSET, " (approximate)" if approximate else ""))
        else:
            print("FPS %.2f - no reading" % clock.fps())
    except (OSError, RuntimeError):
        pass

    # Redraw from the last good reading every frame so the overlay never
    # flickers when a read fails or returns junk (e.g. during an FFC).
    img.draw_rectangle(roi, color=(255, 255, 255))
    if last:
        mean, min_t, max_t, max_loc, approximate = last
        img.draw_cross(max_loc, color=(255, 255, 255))
        img.draw_string((roi[0], roi[1] - 16), "%.1f C%s" % (
            mean - KELVIN_OFFSET, "~" if approximate else ""), color=(255, 255, 255))

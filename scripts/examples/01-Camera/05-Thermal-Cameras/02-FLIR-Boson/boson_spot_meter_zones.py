# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Boson Spot Meter Zones Example
#
# The Boson has ONE hardware spot meter, so monitoring several regions means
# time-multiplexing it: park it on each region in turn, let it settle, read it,
# move on. This example rotates the spot meter through a list of regions at a
# rate you control and reports each region's temperature.
#
# Radiometric cores report calibrated temperatures; on other cameras the spot
# meter counts are converted on the host and the readings are marked with "~" -
# run boson_temp_calibration.py once against a known reference to make them
# accurate (see boson_get_object_temp.py for how the conversion works).

import csi
import image
import json
import math
import time

ROI_DWELL_MS = 2000  # how long the spot meter parks on each region
ROI_SETTLE_MS = 300  # ignore readings this soon after a move

EMISSIVITY = 0.95  # what nearly every real target is; shiny metal is far off
AMBIENT_C = 22.0  # the room temperature reflected by the target

KELVIN_OFFSET = 273.15

# Counts -> temperature model for cameras that do not report temperatures -
# see boson_get_object_temp.py. The gain is per-unit and per-gain-state; the
# values below are starting points.
ANCHOR_COUNTS = 32768
PLANCK_B = 1450.0
RESPONSE_GAINS = {
    640: {csi.BOSON_GAIN_HIGH: 1.2426155e06, csi.BOSON_GAIN_LOW: 1.4718071e06},
    320: {csi.BOSON_GAIN_HIGH: 1.8934190e06, csi.BOSON_GAIN_LOW: 1.3338160e06},
}

# A per-unit calibration saved by boson_temp_calibration.py overrides the
# shipped defaults.
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

# Boson application firmware 2.x predates the spot meter command set.
major, minor, patch = csi0.ioctl(csi.IOCTL_BOSON_GET_SOFTWARE_REV)
if major < 3:
    raise Exception("Boson firmware 2.x predates the spot meter")

radiometric = csi0.ioctl(csi.IOCTL_BOSON_GET_RADIOMETRY_CAPABLE)

# TLinear and temperature stabilization are what make the spot meter report
# temperatures instead of counts. A NAK just means the camera cannot run
# them - the counts fallback still works.
try:
    csi0.ioctl(csi.IOCTL_BOSON_SET_TLINEAR_ENABLE, True)
    csi0.ioctl(csi.IOCTL_BOSON_SET_TEMP_STABLE_ENABLE, True)
    # The camera takes emissivity as a percentage (50-100).
    csi0.ioctl(csi.IOCTL_BOSON_SET_EMISSIVITY, EMISSIVITY * 100.0)
    csi0.ioctl(csi.IOCTL_BOSON_SET_TEMP_BACKGROUND, AMBIENT_C + KELVIN_OFFSET)
except (OSError, RuntimeError):
    pass

csi0.ioctl(csi.IOCTL_BOSON_SET_SPOT_METER_ENABLE, True)
try:
    # Kelvin so the temperature stats are unit-unambiguous.
    csi0.ioctl(csi.IOCTL_BOSON_SET_SPOT_METER_MODE, csi.BOSON_SPOT_METER_KELVIN)
except (OSError, RuntimeError):
    pass

# Regions to monitor: the center plus the four quadrants, clamped to this
# camera's maximum spot meter size (128x128 on the 640s, 64x64 on the 320s).
max_w, max_h = csi0.ioctl(csi.IOCTL_BOSON_GET_SPOT_METER_ROI_MAX)
w, h = csi0.width(), csi0.height()
rw, rh = min(max_w, w // 4), min(max_h, h // 4)
ROIS = [
    ((w - rw) // 2, (h - rh) // 2, rw, rh),
    ((w // 4) - (rw // 2), (h // 4) - (rh // 2), rw, rh),
    ((3 * w // 4) - (rw // 2), (h // 4) - (rh // 2), rw, rh),
    ((w // 4) - (rw // 2), (3 * h // 4) - (rh // 2), rw, rh),
    ((3 * w // 4) - (rw // 2), (3 * h // 4) - (rh // 2), rw, rh),
]


def plausible(kelvin):
    # A value outside what an uncooled microbolometer can see is a bad read.
    return (kelvin is not None) and (200.0 < kelvin < 1000.0)


def radiance(kelvin):
    # Planck band radiance at kelvin, in the sensor's arbitrary units.
    return 1.0 / (math.exp(PLANCK_B / kelvin) - 1.0)


def counts_to_kelvin(counts, fpa_kelvin, gain_mode):
    # The anchor-and-delta model, pivoted on the FPA temperature so the
    # reading follows the camera as it self-heats.
    gains = RESPONSE_GAINS[320 if csi0.cid() == csi.BOSON320 else 640]
    gain = gains.get(gain_mode, gains[csi.BOSON_GAIN_HIGH])
    try:
        seen = radiance(fpa_kelvin) + ((counts - ANCHOR_COUNTS) / gain)
        # Strip out the room reflected by the target.
        obj = (seen - ((1.0 - EMISSIVITY) * radiance(AMBIENT_C + KELVIN_OFFSET))) / EMISSIVITY
        kelvin = PLANCK_B / math.log((1.0 / obj) + 1.0)
    except (ValueError, ZeroDivisionError, OverflowError):
        return None
    return kelvin if plausible(kelvin) else None


temp_stats_ok = True


def spot_meter_kelvin():
    # Returns (mean, min, max, (max_x, max_y), approximate), falling back to
    # counts + conversion on a NAK.
    global temp_stats_ok
    if temp_stats_ok:
        try:
            mean, dev, min_t, min_x, min_y, max_t, max_x, max_y = csi0.ioctl(
                csi.IOCTL_BOSON_GET_SPOT_METER_TEMP_STATS)
            return mean, min_t, max_t, (max_x, max_y), not radiometric
        except (OSError, RuntimeError):
            # Do not keep asking a camera that has told us no.
            temp_stats_ok = False
            print("Temp stats unavailable - falling back to counts conversion")
    mean, dev, min_c, min_x, min_y, max_c, max_x, max_y = csi0.ioctl(
        csi.IOCTL_BOSON_GET_SPOT_METER_STATS)
    fpa_kelvin = csi0.ioctl(csi.IOCTL_BOSON_GET_FPA_TEMP) + KELVIN_OFFSET
    try:
        gain_mode = csi0.ioctl(csi.IOCTL_BOSON_GET_GAIN_MODE)
    except (OSError, RuntimeError):
        gain_mode = csi.BOSON_GAIN_HIGH
    return (counts_to_kelvin(mean, fpa_kelvin, gain_mode),
            counts_to_kelvin(min_c, fpa_kelvin, gain_mode),
            counts_to_kelvin(max_c, fpa_kelvin, gain_mode),
            (max_x, max_y), True)


clock = time.clock()
roi_index = 0
roi_moved_ms = time.ticks_ms()
csi0.ioctl(csi.IOCTL_BOSON_SET_SPOT_METER_ROI, ROIS[roi_index])
readings = [None] * len(ROIS)  # last (label, max_loc) per region

while True:
    clock.tick()
    img = csi0.snapshot()

    # Rotate the spot meter through the regions at the configured rate.
    if time.ticks_diff(time.ticks_ms(), roi_moved_ms) >= ROI_DWELL_MS:
        try:
            roi_index = (roi_index + 1) % len(ROIS)
            csi0.ioctl(csi.IOCTL_BOSON_SET_SPOT_METER_ROI, ROIS[roi_index])
        except (OSError, RuntimeError):
            pass
        roi_moved_ms = time.ticks_ms()

    # Sample the region the meter is parked on - once it has settled after
    # the move. The other regions keep their last reading.
    if time.ticks_diff(time.ticks_ms(), roi_moved_ms) >= ROI_SETTLE_MS:
        try:
            mean, min_t, max_t, max_loc, approximate = spot_meter_kelvin()
            if plausible(mean) and plausible(min_t) and plausible(max_t):
                marker = "~" if approximate else ""
                readings[roi_index] = ("%.1f C%s" % (mean - KELVIN_OFFSET, marker), max_loc)
                print("FPS %.2f - roi %d: mean %.2f C%s, min %.2f C%s, max %.2f C%s" % (
                    clock.fps(), roi_index, mean - KELVIN_OFFSET, marker,
                    min_t - KELVIN_OFFSET, marker, max_t - KELVIN_OFFSET, marker))
        except (OSError, RuntimeError):
            pass

    # Redraw every region every frame so the overlay never flickers: the
    # rectangle, its last reading above it, and the active region's hotspot.
    for i in range(len(ROIS)):
        img.draw_rectangle(ROIS[i], color=(255, 255, 255),
                           thickness=(2 if i == roi_index else 1))
        if readings[i]:
            img.draw_string((ROIS[i][0], ROIS[i][1] - 16), readings[i][0],
                            color=(255, 255, 255))
    if readings[roi_index]:
        img.draw_cross(readings[roi_index][1], color=(255, 255, 255))

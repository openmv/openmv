# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Text Transforms (OpenMV N6)
#
# draw_string() takes the same 3x3 transform matrix as draw_image(). On the OpenMV N6
# the GPU applies it, so text can be rotated to any angle, scaled, sheared or put in
# perspective at no extra cost. Only the N6 has a GPU that does this; on other cameras
# use the hint argument (image.ROTATE_90 and friends) instead, as this example does
# when it is not on an N6.
#
# The matrix acts on image coordinates. To turn text about a point, move that point to
# the origin, transform, and move it back: about() below does that.

import csi
import image
import math
import os
import time
from ulab import numpy as np

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.snapshot(time=2000)

WHITE, YELLOW, CYAN, GREEN, RED = (255, 255, 255), (255, 220, 0), (0, 255, 255), (0, 255, 160), (255, 80, 80)

HAS_GPU_TRANSFORM = "N6" in os.uname().machine


def translate(tx, ty):
    return np.array([[1.0, 0.0, tx], [0.0, 1.0, ty], [0.0, 0.0, 1.0]])


def rotate(degrees):
    c, s = math.cos(math.radians(degrees)), math.sin(math.radians(degrees))
    return np.array([[c, -s, 0.0], [s, c, 0.0], [0.0, 0.0, 1.0]])


def scale(sx, sy):
    return np.array([[sx, 0.0, 0.0], [0.0, sy, 0.0], [0.0, 0.0, 1.0]])


def perspective(ky):
    # Negative values make the top of the text narrower, as if it were leaning away.
    return np.array([[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, ky, 1.0]])


def about(cx, cy, *matrices):
    m = translate(cx, cy)
    for k in matrices:
        m = np.dot(m, k)
    return np.dot(m, translate(-cx, -cy))


# The GPU samples the text image without filtering, so text that is drawn at its final
# size and then turned has stepped edges. Rendering at twice the size and halving it in
# the matrix (scale(0.5, 0.5)) gives smooth edges; these fonts are twice the size shown.
title = image.Font(None, 96)
medium = image.Font(None, 44)
small = image.Font(None, 32)


def centered(img, text, f, cx, cy, color, *matrices, alpha=255):
    # Draws text with its centre at (cx, cy), transformed about that point.
    w, h, _ = f.measure(text)
    img.draw_string((int(cx - w / 2), int(cy - h / 2)), text, color=color, font=f, alpha=alpha,
                    transform=about(cx, cy, *matrices, scale(0.5, 0.5)))


def draw(img, frame):
    w, h = img.width(), img.height()
    cx, cy = w // 2, h // 2 + 30

    if not HAS_GPU_TRANSFORM:
        img.draw_string((8, 8), "transform needs the OpenMV N6's GPU.", color=WHITE, font=image.Font(None, 20))
        img.draw_string((8, 36), "hint rotates by quarter turns everywhere:", color=WHITE, font=image.Font(None, 20))
        img.draw_string((8, 70), "ROTATE_90", color=YELLOW, font=small, hint=image.ROTATE_90)
        img.draw_string((60, 70), "ROTATE_180", color=CYAN, font=small, hint=image.ROTATE_180)
        img.draw_string((w - 48, 70), "ROTATE_270", color=GREEN, font=small, hint=image.ROTATE_270)
        return

    # A title in perspective, like a film crawl.
    centered(img, "OpenMV N6", title, cx, 44, YELLOW, perspective(-0.006 - 0.003 * math.sin(frame / 12)))

    # A dial: labels turned to their bearing around a ring, and the ring itself turning.
    radius = 104
    img.draw_circle((cx, cy, radius - 16), color=CYAN, thickness=2)
    for i, name in enumerate(("N", "NE", "E", "SE", "S", "SW", "W", "NW")):
        bearing = i * 45 + frame * 1.5
        px = cx + radius * math.sin(math.radians(bearing))
        py = cy - radius * math.cos(math.radians(bearing))
        centered(img, name, small, px, py, RED if name == "N" else GREEN, rotate(bearing))

    # Text spinning about its centre while it grows and shrinks.
    zoom = 1.0 + 0.35 * math.sin(frame / 9)
    centered(img, "GPU text", title, cx, cy, WHITE, rotate(frame * 4), scale(zoom * 0.5, zoom * 0.5))

    # Each letter of a string placed on a wave and turned to follow its slope.
    text = "anti-aliased TrueType"
    amplitude, period = 14.0, 220.0
    pen = cx - medium.measure(text)[0] / 4
    for ch in text:
        gw, gh, _ = medium.measure(ch)
        if ch != " ":
            phase = (pen - cx) / period * 2 * math.pi + frame / 6
            slope = math.degrees(math.atan(amplitude * math.cos(phase) * 2 * math.pi / period))
            centered(img, ch, medium, pen + gw / 4, h - 34 + amplitude * math.sin(phase), CYAN, rotate(slope))
        pen += gw / 2


clock = time.clock()
frame = 0

while True:
    clock.tick()
    img = csi0.snapshot()
    draw(img, frame)
    frame += 1
    print(clock.fps())

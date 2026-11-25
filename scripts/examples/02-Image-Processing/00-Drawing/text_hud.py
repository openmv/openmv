# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Text HUD
#
# This example draws a heads-up display over the camera image with the built-in font:
# an FPS counter, right-aligned text, a labelled box that follows a target, and a
# status bar. It runs on every OpenMV Cam. Text is anti-aliased, and after the first
# frame every glyph comes from the glyph cache, so a full HUD costs well under a
# millisecond per string.

import csi
import image
import math
import time

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)

WHITE, BLACK, GREEN, YELLOW, RED = (255, 255, 255), (0, 0, 0), (0, 255, 0), (255, 220, 0), (255, 60, 60)

# draw_string() with no font uses the built-in font: 10 pixels tall, fixed pitch, each
# character in a 6x10 pixel cell. Font() is that font as an object, for measuring text.
font = image.Font()
big = image.Font(None, 20)  # The same typeface at 20 pixels.


def label(img, x, y, text, color=WHITE, f=font, align=image.Font.LEFT):
    # Text on a black box. measure() returns the size of the text before it is drawn, and
    # align says whether x is the left edge, the centre or the right edge of the text.
    w, h, baseline = f.measure(text)
    left = x - (w * align) // 2  # LEFT, CENTER and RIGHT are 0, 1 and 2.
    img.draw_rectangle((left - 2, y - 1, w + 4, h + 2), color=BLACK, fill=True)
    img.draw_string((x, y), text, color=color, font=f, align=align)


def draw(img, frame, fps):
    w, h = img.width(), img.height()

    # Top row: FPS on the left, the frame size right aligned.
    label(img, 4, 3, "FPS %5.1f" % fps, GREEN)
    label(img, w - 4, 3, "%dx%d RGB565" % (w, h), align=image.Font.RIGHT)

    # A target that wanders around the image, with a label that follows it.
    cx = int(w / 2 + (w / 3) * math.cos(frame / 25))
    cy = int(h / 2 + (h / 4) * math.sin(frame / 16))
    img.draw_rectangle((cx - 30, cy - 22, 60, 44), color=YELLOW, thickness=2)
    label(img, cx - 30, cy - 22 - font.line_height - 2, "target %2d%%" % (60 + 39 * math.sin(frame / 9)), YELLOW)
    img.draw_cross((cx, cy), color=YELLOW, size=6)
    # An outline does the job of the box: the text reads over anything behind it.
    img.draw_string((cx, cy + 26), "x=%d y=%d" % (cx, cy), color=WHITE, align=image.Font.CENTER, stroke_width=1)

    # Status bar: larger text, a blinking indicator and a clock.
    img.draw_rectangle((0, h - 26, w, 26), color=BLACK, fill=True)
    img.draw_string((6, h - 23), "OpenMV", color=WHITE, font=big)
    if (frame // 10) % 2:
        img.draw_circle((w - 112, h - 13, 5), color=RED, fill=True)
    img.draw_string((w - 6, h - 18), "REC %02d:%02d.%d" % (frame // 600, (frame // 10) % 60, frame % 10),
                    color=RED, align=image.Font.RIGHT)


clock = time.clock()
frame = 0

while True:
    clock.tick()
    img = csi0.snapshot()
    draw(img, frame, clock.fps())
    frame += 1

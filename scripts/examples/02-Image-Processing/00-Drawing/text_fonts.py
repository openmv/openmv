# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Text Fonts
#
# This example shows the TrueType fonts in the camera's ROMFS: every face drawn in
# itself, and the built-in font at a range of sizes. Any TrueType font can be loaded
# from ROMFS or the filesystem at any pixel size with image.Font(path, size).
#
# Which fonts a camera ships depends on its ROMFS: every camera has Roboto Mono
# Regular, the built-in font; larger cameras add Bold and Italic faces and the
# proportional Roboto family; the N6 and AE3 add Chinese, Japanese, Korean and Arabic.

import csi
import image
import os
import time

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)

WHITE, BLACK, YELLOW, CYAN = (255, 255, 255), (0, 0, 0), (255, 220, 0), (0, 255, 255)

# One Font object per face, at 14 pixels. A font in ROMFS is used in place and costs no
# RAM; print(font) shows its attributes, including its size in bytes.
faces = [image.Font("/rom/" + name, 14) for name in sorted(os.listdir("/rom")) if name.endswith(".ttf")]
for f in faces:
    print(f)

# The built-in font at several sizes. Font(None, size) is the built-in typeface, and it
# keeps the built-in font's fallbacks.
sizes = [image.Font(None, s) for s in (8, 10, 12, 16, 20, 24, 32, 40)]


def draw(img, frame):
    img.draw_rectangle((0, 0, img.width(), img.height()), color=BLACK, fill=True)
    page = (frame // 40) % 2

    if page == 0 or len(faces) < 2:
        # Sizes: the line height comes from the font, so text of any size stacks up cleanly.
        y = 4
        for f in sizes:
            img.draw_string((4, y), "%d px Quartz" % f.size, color=WHITE, font=f)
            y += f.line_height + 1
    else:
        # Faces: each font's file name, drawn with that font. has() says whether a font
        # (and its fallbacks) can draw a string without a missing-glyph box.
        # Fonts for other scripts have taller lines; step by the built-in font's so the
        # list is evenly spaced.
        y = 3
        step = min(sizes[4].line_height + 1, (img.height() - 6) // len(faces))
        for f in faces:
            name = f.path.split("/")[-1][:-4]
            sample = name if f.has(name) else "(%d glyphs)" % f.glyphs
            img.draw_string((4, y), sample, color=YELLOW if "Mono" in name else CYAN, font=f)
            img.draw_string((img.width() - 70, y + 3), "%6d B" % f.bytes, color=WHITE)
            y += step


clock = time.clock()
frame = 0

while True:
    clock.tick()
    img = csi0.snapshot()
    draw(img, frame)
    frame += 1
    print(clock.fps())

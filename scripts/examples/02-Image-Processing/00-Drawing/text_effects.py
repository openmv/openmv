# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Text Effects
#
# This example shows what can be done with text beyond drawing it: outlines and
# shadows so it reads over any background, alpha blending, rotation and mirroring,
# size changes, per-letter colour, a wrapped and centred typewriter and a scrolling
# ticker. It runs on every OpenMV Cam.

import csi
import image
import math
import time

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)

WHITE, BLACK, YELLOW, CYAN, ORANGE = (255, 255, 255), (0, 0, 0), (255, 220, 0), (0, 255, 255), (255, 140, 0)

title = image.Font(None, 28)
body = image.Font(None, 14)

# Text that never changes can be rendered once and drawn as an image each frame, which is
# the cheapest way to draw it. Font.render() returns the text as a grayscale image of how
# much of each pixel it covers; draw_image() turns that into coloured text given a colour
# palette (here one colour) and an alpha palette that makes coverage the transparency.
ticker = body.render("   Anti-aliased TrueType text   *   UTF-8   *   any size   *   alpha, rotation, mirroring   *")
yellow = image.Image(256, 1, image.RGB565)
yellow.draw_rectangle((0, 0, 256, 1), color=YELLOW, fill=True)
ramp = image.Image(256, 1, image.GRAYSCALE)
for i in range(256):
    ramp.set_pixel((i, 0), i)


def rainbow(i):
    # A colour wheel: three sine waves a third of a turn apart.
    return tuple(int(128 + 127 * math.sin(i + k)) for k in (0.0, 2.1, 4.2))


def draw(img, frame):
    w, h = img.width(), img.height()

    # Outlined text reads over any background. stroke_width is the outline's thickness in
    # pixels and stroke_color its colour (black unless given).
    img.draw_string((4, 2), "Outlined", color=WHITE, font=title, stroke_width=2)

    # A drop shadow: the same text in black, offset and half transparent.
    img.draw_string((w - 141, 9), "Shadow", color=BLACK, font=title, alpha=128)
    img.draw_string((w - 144, 6), "Shadow", color=YELLOW, font=title)

    # alpha fades text in and out.
    a = int(140 + 115 * math.sin(frame / 8))
    img.draw_string((28, 40), "alpha=%3d" % a, color=CYAN, font=title, alpha=a)

    # scale multiplies the font's size. Text is redrawn from its outlines at the new
    # size, so it stays sharp.
    s = 1.5 + 0.7 * math.sin(frame / 10)
    tw, th, _ = body.measure("scale", scale=s)
    img.draw_string((w - 70 - tw // 2, 58 - th // 2), "scale", color=ORANGE, font=body, scale=s)

    # A colour per letter: draw the letters one at a time, advancing by each one's width.
    x = (w - title.measure("RAINBOW")[0]) // 2
    for i, ch in enumerate("RAINBOW"):
        # The outline adds a pixel all round, so step back by it to keep the letters' spacing.
        img.draw_string((x - 1, 75), ch, color=rainbow(i * 0.8 - frame / 5), font=title, stroke_width=1)
        x += title.measure(ch)[0]

    # hint rotates and mirrors whole strings. A one pixel outline keeps white text readable
    # when the camera is looking at something white.
    mid = (76 + title.line_height + h - 64) // 2 - 8
    img.draw_string((4, 44), "ROTATE_90", color=WHITE, font=body, stroke_width=1, hint=image.ROTATE_90)
    img.draw_string((w - 20, 76), "ROTATE_270", color=WHITE, font=body, stroke_width=1, hint=image.ROTATE_270)
    img.draw_string((30, mid), "HMIRROR", color=WHITE, font=body, stroke_width=1, hint=image.HMIRROR)
    img.draw_string((w // 2 - 22, mid), "VFLIP", color=WHITE, font=body, stroke_width=1, hint=image.VFLIP)
    img.draw_string((w - 120, mid), "ROTATE_180", color=WHITE, font=body, stroke_width=1, hint=image.ROTATE_180)

    # A typewriter: more of the string each frame. wrap_width breaks the text into lines
    # that fit, and align centres each of them on x.
    msg = "Typed one letter at a time, wrapped and centred."
    n = (frame // 2) % (len(msg) + 16)
    img.draw_string((w // 2, h - 60), msg[:n] + ("_" if frame % 6 < 3 else ""), color=WHITE, font=body,
                    align=image.Font.CENTER, wrap_width=w - 80, stroke_width=1)

    # A ticker: the pre-rendered text scrolling through a bar. What leaves the image is clipped.
    img.draw_rectangle((0, h - 24, w, 24), color=BLACK, fill=True)
    x = -((frame * 3) % ticker.width())
    while x < w:
        img.draw_image(ticker, x, h - 20, color_palette=yellow, alpha_palette=ramp)
        x += ticker.width()


clock = time.clock()
frame = 0

while True:
    clock.tick()
    img = csi0.snapshot()
    draw(img, frame)
    frame += 1
    print(clock.fps())

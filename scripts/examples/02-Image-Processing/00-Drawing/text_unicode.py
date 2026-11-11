# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Text in Many Languages
#
# draw_string() takes UTF-8 text. When the font being used lacks a character it falls
# back to the other fonts in the camera's ROMFS, so one call draws mixed languages.
# Arabic is shaped and laid out right to left automatically: write it as you would type
# it. Characters no font has are drawn as a small box.
#
# Latin, Greek and Cyrillic are on every camera with a large ROMFS. Chinese, Japanese,
# Korean and Arabic are on the N6 and AE3. Font.has() tells you what the camera you are
# on can draw, so a script can choose its text.

import csi
import image
import time

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)
csi0.snapshot(time=2000)

WHITE, BLACK, GREEN, GRAY, YELLOW = (255, 255, 255), (0, 0, 0), (0, 255, 0), (120, 120, 120), (255, 220, 0)

# Ideographs need about 16 pixels to be legible. Font(None, 18) is the built-in font,
# with its fallbacks, at 18 pixels.
font = image.Font(None, 18)
small = image.Font()

greetings = (
    ("English", "Hello, world!"),
    ("Español", "¡Hola, mundo!"),
    ("Français", "Ça va très bien"),
    ("Deutsch", "Grüße aus München"),
    ("Ελληνικά", "Γειά σου Κόσμε"),
    ("Русский", "Привет, мир!"),
    ("中文", "你好，世界"),
    ("日本語", "こんにちは世界"),
    ("한국어", "안녕하세요 세계"),
    ("العربية", "مرحبا بالعالم"),
)


def draw(img, frame):
    w, h = img.width(), img.height()
    img.draw_rectangle((0, 0, w, h), color=BLACK, fill=True)

    y = 3
    for language, text in greetings:
        if font.has(text):
            # Right to left text is drawn from the right edge.
            if language == "العربية":
                img.draw_string((w - 6, y), text, color=WHITE, font=font, align=image.Font.RIGHT)
            else:
                img.draw_string((92, y), text, color=WHITE, font=font)
            name = language if small.has(language) else "ok"
            img.draw_string((4, y + 5), name, color=GREEN)
        else:
            img.draw_string((92, y + 5), "not in this camera's fonts", color=GRAY)
            img.draw_string((4, y + 5), language if small.has(language) else "--", color=GRAY)
        y += font.line_height - 1

    # Mixed scripts and numbers in one string, with no font argument at all.
    mixed = "温度 23.5°C  مرحبا  FPS %d" % (30 + frame % 3)
    if small.has(mixed):
        img.draw_string((4, h - 13), mixed, color=YELLOW)
    else:
        img.draw_string((4, h - 13), "Temp 23.5°C  FPS %d" % (30 + frame % 3), color=YELLOW)


clock = time.clock()
frame = 0

while True:
    clock.tick()
    img = csi0.snapshot()
    draw(img, frame)
    frame += 1
    print(clock.fps())

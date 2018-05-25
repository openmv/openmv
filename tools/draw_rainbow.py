#! /usr/bin/env python
import colorsys
from PIL import Image

NUM_COL=256
SAT=1.0
VAL=1.0
OFFSET=220
tup = [colorsys.hsv_to_rgb(1.0-(i/float(NUM_COL+OFFSET)), SAT, VAL) for i in range(OFFSET, NUM_COL+OFFSET)]
col = [(int(r*255), int(g*255), int(b*255)) for r,g,b in tup]

REPY=3
w=64
h=NUM_COL*REPY
im = Image.new('RGB', (w, h))
for y in range(0, h):
    for x in range(0, w):
        im.putpixel((x, y), col[y/REPY])

im.show()

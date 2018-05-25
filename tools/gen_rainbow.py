#!/usr/bin/env python
# -*- coding: utf-8 -*-

NUM_COL=256
SAT=1.0
VAL=1.0
OFFSET=220

import sys, colorsys
sys.stdout.write("#include <stdint.h>\n")

tup = [colorsys.hsv_to_rgb(1.0-(i/float(NUM_COL+OFFSET)), SAT, VAL) for i in range(OFFSET, NUM_COL+OFFSET)]
col = [(int((((r*255)*31)+127.5)/255)&0x1F)<<11 |
       (int((((g*255)*63)+127.5)/255)&0x3F)<<5 |
       (int((((b*255)*31)+127.5)/255)&0x1F) for r,g,b in tup]

sys.stdout.write("const uint16_t rainbow_table[%d] = {\n" % NUM_COL)
for i in range(NUM_COL):
    if not (i % 8):
        sys.stdout.write("    ")
    sys.stdout.write("0x%04X" % (((col[i]&0x00ff)<<8)|((col[i]&0xff00)>>8)))
    if (i + 1) % 8:
        sys.stdout.write(", ")
    elif i != (NUM_COL-1):
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

#! /usr/bin/env python
import colorsys

NUM_COL=256
SAT=1.0
VAL=1.0
OFFSET=220
tup = [colorsys.hsv_to_rgb(1.0-(i/float(NUM_COL+OFFSET)), SAT, VAL) for i in range(OFFSET, NUM_COL+OFFSET)]
col = [((int(r*255)*31/255)&0x1F)<<11 |
       ((int(g*255)*63/255)&0x3F)<<5 |
       ((int(b*255)*31/255)&0x1F) for r,g,b in tup]

print \
"#include <stdint.h>\n"\
"const uint16_t rainbow_table[%d] = {"%(NUM_COL)

for i in range(0, NUM_COL):
    if (i%4)==0 and i != (NUM_COL-1):
        if i >0:
            print ""
        print "    ",
    print "0x%X,"%((col[i] & 0xff)<<8 |(col[i] & 0xff00) >> 8),
print "\n};"

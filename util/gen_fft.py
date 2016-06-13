#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, math
# https://www.nayuki.io/page/fast-fourier-transform-in-x86-assembly

sys.stdout.write("const float cos_table[512] = {\n")
for i in range(512):
    if not (i % 8):
        sys.stdout.write("    ")
    sys.stdout.write("%9.6ff" % math.cos((math.pi * i) / 512))
    if (i + 1) % 8:
        sys.stdout.write(", ")
    elif i != 511:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

sys.stdout.write("const float sin_table[512] = {\n")
for i in range(512):
    if not (i % 8):
        sys.stdout.write("    ")
    sys.stdout.write("%9.6ff" % math.sin((math.pi * i) / 512))
    if (i + 1) % 8:
        sys.stdout.write(", ")
    elif i != 511:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

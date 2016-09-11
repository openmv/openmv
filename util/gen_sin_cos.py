#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, math

sys.stdout.write("const float sin_table[360] = {\n")
for i in range(360):
    if not (i % 8):
        sys.stdout.write("    ")
    sys.stdout.write("%9.6ff" % math.sin(math.radians(i)))
    if (i + 1) % 8:
        sys.stdout.write(", ")
    elif i != 359:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

sys.stdout.write("\n")
sys.stdout.write("const float cos_table[360] = {\n")
for i in range(360):
    if not (i % 8):
        sys.stdout.write("    ")
    sys.stdout.write("%9.6ff" % math.cos(math.radians(i)))
    if (i + 1) % 8:
        sys.stdout.write(", ")
    elif i != 359:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

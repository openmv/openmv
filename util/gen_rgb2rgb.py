#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
sys.stdout.write("#include <stdint.h>\n")

sys.stdout.write("const uint8_t rb528_table[32] = {\n")
for i in range(32):
    if not (i % 8):
        sys.stdout.write("    ")
    sys.stdout.write("%3d" % (((i * 255) + 15.5) // 31))
    if (i + 1) % 8:
        sys.stdout.write(", ")
    elif i != 31:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

sys.stdout.write("const uint8_t g628_table[64] = {\n")
for i in range(64):
    if not (i % 8):
        sys.stdout.write("    ")
    sys.stdout.write("%3d" % (((i * 255) + 31.5) // 63))
    if (i + 1) % 8:
        sys.stdout.write(", ")
    elif i != 63:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

sys.stdout.write("const uint8_t rb825_table[256] = {\n")
for i in range(256):
    if not (i % 8):
        sys.stdout.write("    ")
    sys.stdout.write("%3d" % (((i * 31) + 127.5) // 255))
    if (i + 1) % 8:
        sys.stdout.write(", ")
    elif i != 255:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

sys.stdout.write("const uint8_t g826_table[256] = {\n")
for i in range(256):
    if not (i % 8):
        sys.stdout.write("    ")
    sys.stdout.write("%3d" % (((i * 63) + 127.5) // 255))
    if (i + 1) % 8:
        sys.stdout.write(", ")
    elif i != 255:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

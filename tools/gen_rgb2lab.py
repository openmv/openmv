#!/usr/bin/env python
# -*- coding: utf-8 -*-
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This script generates RGB to LAB lookup table.
# See https://en.wikipedia.org/wiki/SRGB (The reverse transformation)

def lin(c):
    return 100 * ((c/12.92) if (c<=0.04045) else pow((c+0.055)/1.055, 2.4))

import sys, math
sys.stdout.write("#include <stdint.h>\n")
l_list = []
a_list = []
b_list = []
sys.stdout.write("const int8_t lab_table[98304] = {\n") # 65536 * 3 / 2
for i in range(65536):

    r = ((((i >> 11) & 31) * 255) + 15.5) // 31
    g = ((((i >> 5) & 63) * 255) + 31.5) // 63
    b = (((i & 31) * 255) + 15.5) // 31

    # https://en.wikipedia.org/wiki/SRGB (The reverse transformation)

    r_lin = lin(r / 255.0)
    g_lin = lin(g / 255.0)
    b_lin = lin(b / 255.0)

    x = (r_lin * 0.4124) + (g_lin * 0.3576) + (b_lin * 0.1805);
    y = (r_lin * 0.2126) + (g_lin * 0.7152) + (b_lin * 0.0722);
    z = (r_lin * 0.0193) + (g_lin * 0.1192) + (b_lin * 0.9505);

    # https://en.wikipedia.org/wiki/Lab_color_space (CIELAB-CIEXYZ conversions)
    def f(t):
        return pow(t, (1/3.0)) if (t>0.008856) else ((7.787037*t)+0.137931)

    x = f(x / 095.047)
    y = f(y / 100.000)
    z = f(z / 108.883)

    l = int(round(116 * y)) - 16;
    a = int(round(500 * (x-y)));
    b = int(round(200 * (y-z)));

    l_list.append(int(round(116 * y)) - 16)
    a_list.append(int(round(500 * (x-y))))
    b_list.append(int(round(200 * (y-z))))

lab = list(zip(l_list, a_list, b_list))
for i, (x, y) in enumerate(zip(lab[0::2], lab[1::2])):
    out = [(i1 + i2)/2.0 for i1, i2 in zip(x, y)]
    sys.stdout.write(" %4d, %4d, %4d" % (out[0], out[1], out[2]))
    if (i + 1) % 4:
        sys.stdout.write(", ")
    elif i != 65535//2:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

sys.stdout.write("const float xyz_table[256] = {\n")
for i in range(256):

    if not (i % 8):
        sys.stdout.write("    ")
    sys.stdout.write("%9.6ff" % lin(i / 255.0))
    if (i + 1) % 8:
        sys.stdout.write(", ")
    elif i != 255:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

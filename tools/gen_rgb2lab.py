#!/usr/bin/env python
# -*- coding: utf-8 -*-

# https://en.wikipedia.org/wiki/SRGB (The reverse transformation)
def lin(c):
    return 100 * ((c/12.92) if (c<=0.04045) else pow((c+0.055)/1.055, 2.4))

import sys
sys.stdout.write("#include <stdint.h>\n")

sys.stdout.write("const int8_t lab_table[196608] = {\n") # 65536 * 3
for i in range(65536):

    r = ((((i >> 3) & 31) * 255) + 15.5) // 31
    g = (((((i & 7) << 3) | (i >> 13)) * 255) + 31.5) // 63
    b = ((((i >> 8) & 31) * 255) + 15.5) // 31

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

    sys.stdout.write("    %4d, %4d, %4d" % (l, a, b))
    if (i + 1) % 4:
        sys.stdout.write(", ")
    elif i != 65535:
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

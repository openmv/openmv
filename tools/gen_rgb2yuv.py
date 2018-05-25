#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
sys.stdout.write("#include <stdint.h>\n")

sys.stdout.write("const int8_t yuv_table[196608] = {\n") # 65536 * 3
for i in range(65536):

    r = ((((i >> 3) & 31) * 255) + 15.5) // 31
    g = (((((i & 7) << 3) | (i >> 13)) * 255) + 31.5) // 63
    b = ((((i >> 8) & 31) * 255) + 15.5) // 31

    # https://en.wikipedia.org/wiki/YCbCr (ITU-R BT.601 conversion)
    y = int((r * +0.299000) + (g * +0.587000) + (b * +0.114000)) - 128;
    u = int((r * -0.168736) + (g * -0.331264) + (b * +0.500000));
    v = int((r * +0.500000) + (g * -0.418688) + (b * -0.081312));

    sys.stdout.write("    %4d, %4d, %4d" % (y, u, v))
    if (i + 1) % 4:
        sys.stdout.write(", ")
    elif i != 65535:
        sys.stdout.write(",\n")
    else:
        sys.stdout.write("\n};\n")

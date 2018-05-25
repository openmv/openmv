#!/usr/bin/env python
# -*- coding: utf-8 -*-

# https://en.wikipedia.org/wiki/SRGB (The reverse transformation)
def lin(c):
    return 100.0 * ((c/12.92) if (c<=0.04045) else pow((c+0.055)/1.055, 2.4))

import sys, math
sys.stdout.write("#include <stdint.h>\n")

if False: # 1D illumination invariant image

    sys.stdout.write("const uint8_t invariant_table[65536] = {\n") # 65536 * 1
    for i in range(65536):

        r = ((((i >> 3) & 31) * 255) + 15.5) // 31
        g = (((((i & 7) << 3) | (i >> 13)) * 255) + 31.5) // 63
        b = ((((i >> 8) & 31) * 255) + 15.5) // 31

        # http://ai.stanford.edu/~alireza/publication/cic15.pdf

        r_lin = lin(r / 255.0) + 1.0
        g_lin = lin(g / 255.0) + 1.0
        b_lin = lin(b / 255.0) + 1.0

        r_lin_sharp = (r_lin *  0.9968) + (g_lin *  0.0228) + (b_lin * 0.0015);
        g_lin_sharp = (r_lin * -0.0071) + (g_lin *  0.9933) + (b_lin * 0.0146);
        b_lin_sharp = (r_lin *  0.0103) + (g_lin * -0.0161) + (b_lin * 0.9839);

        lin_sharp_avg = r_lin_sharp * g_lin_sharp * b_lin_sharp
        lin_sharp_avg_p = math.pow(lin_sharp_avg, 1.0/3.0) if (lin_sharp_avg > 0.0) else 0.0

        r_lin_sharp_div = 0.0
        g_lin_sharp_div = 0.0
        b_lin_sharp_div = 0.0

        if lin_sharp_avg_p > 0.0:
            lin_sharp_avg_d = 1.0 / lin_sharp_avg_p
            r_lin_sharp_div = r_lin_sharp * lin_sharp_avg_d
            g_lin_sharp_div = g_lin_sharp * lin_sharp_avg_d
            b_lin_sharp_div = b_lin_sharp * lin_sharp_avg_d

        r_lin_sharp_div_log = math.log(r_lin_sharp_div) if (r_lin_sharp_div > 0.0) else 0.0
        g_lin_sharp_div_log = math.log(g_lin_sharp_div) if (g_lin_sharp_div > 0.0) else 0.0
        b_lin_sharp_div_log = math.log(b_lin_sharp_div) if (b_lin_sharp_div > 0.0) else 0.0

        chi_x = (r_lin_sharp_div_log * 0.7071) + (g_lin_sharp_div_log * -0.7071) + (b_lin_sharp_div_log *  0.0000)
        chi_y = (r_lin_sharp_div_log * 0.4082) + (g_lin_sharp_div_log *  0.4082) + (b_lin_sharp_div_log * -0.8164)
        chi_int = max(min(int(round(math.exp((chi_x * 0.9326) + (chi_y * -0.3609)) * 127.5)), 255), 0)

        if not (i % 16):
            sys.stdout.write("    ")
        sys.stdout.write("%3d" % chi_int)
        if (i + 1) % 16:
            sys.stdout.write(", ")
        elif i != 65535:
            sys.stdout.write(",\n")
        else:
            sys.stdout.write("\n};\n")

if True: # 2D illumination invariant image

    sys.stdout.write("const uint16_t invariant_table[65536] = {\n") # 65536 * 2
    for i in range(65536):

        r = ((((i >> 3) & 31) * 255) + 15.5) // 31
        g = (((((i & 7) << 3) | (i >> 13)) * 255) + 31.5) // 63
        b = ((((i >> 8) & 31) * 255) + 15.5) // 31

        # http://ai.stanford.edu/~alireza/publication/cic15.pdf

        r_lin = lin(r / 255.0) + 1.0
        g_lin = lin(g / 255.0) + 1.0
        b_lin = lin(b / 255.0) + 1.0

        r_lin_sharp = (r_lin *  0.9968) + (g_lin *  0.0228) + (b_lin * 0.0015);
        g_lin_sharp = (r_lin * -0.0071) + (g_lin *  0.9933) + (b_lin * 0.0146);
        b_lin_sharp = (r_lin *  0.0103) + (g_lin * -0.0161) + (b_lin * 0.9839);

        lin_sharp_avg = r_lin_sharp * g_lin_sharp * b_lin_sharp
        lin_sharp_avg_p = math.pow(lin_sharp_avg, 1.0/3.0) if (lin_sharp_avg > 0.0) else 0.0

        r_lin_sharp_div = 0.0
        g_lin_sharp_div = 0.0
        b_lin_sharp_div = 0.0

        if lin_sharp_avg_p > 0.0:
            lin_sharp_avg_d = 1.0 / lin_sharp_avg_p
            r_lin_sharp_div = r_lin_sharp * lin_sharp_avg_d
            g_lin_sharp_div = g_lin_sharp * lin_sharp_avg_d
            b_lin_sharp_div = b_lin_sharp * lin_sharp_avg_d

        r_lin_sharp_div_log = math.log(r_lin_sharp_div) if (r_lin_sharp_div > 0.0) else 0.0
        g_lin_sharp_div_log = math.log(g_lin_sharp_div) if (g_lin_sharp_div > 0.0) else 0.0
        b_lin_sharp_div_log = math.log(b_lin_sharp_div) if (b_lin_sharp_div > 0.0) else 0.0

        chi_x = (r_lin_sharp_div_log * 0.7071) + (g_lin_sharp_div_log * -0.7071) + (b_lin_sharp_div_log *  0.0000)
        chi_y = (r_lin_sharp_div_log * 0.4082) + (g_lin_sharp_div_log *  0.4082) + (b_lin_sharp_div_log * -0.8164)

        e_t_x =  0.9326
        e_t_y = -0.3609

        p_th_00 = e_t_x * e_t_x
        p_th_01 = e_t_x * e_t_y
        p_th_10 = e_t_y * e_t_x
        p_th_11 = e_t_y * e_t_y
       
        x_th_x = (p_th_00 * chi_x) + (p_th_01 * chi_y)
        x_th_y = (p_th_10 * chi_x) + (p_th_11 * chi_y)
       
        r_chi = (x_th_x *  0.7071) + (x_th_y *  0.4082)
        g_chi = (x_th_x * -0.7071) + (x_th_y *  0.4082)
        b_chi = (x_th_x *  0.0000) + (x_th_y * -0.8164)

        r_chi_invariant = math.exp(r_chi)
        g_chi_invariant = math.exp(g_chi)
        b_chi_invariant = math.exp(b_chi)

        chi_invariant_sum = r_chi_invariant + g_chi_invariant + b_chi_invariant
        
        r_chi_invariant_m = 0.0
        g_chi_invariant_m = 0.0
        b_chi_invariant_m = 0.0

        if chi_invariant_sum > 0.0:
            chi_invariant_sum = 1.0 / chi_invariant_sum
            r_chi_invariant_m = r_chi_invariant * chi_invariant_sum
            g_chi_invariant_m = g_chi_invariant * chi_invariant_sum
            b_chi_invariant_m = b_chi_invariant * chi_invariant_sum

        rgb565 = (int((((r_chi_invariant_m*255)*31)+127.5)/255)&0x1F)<<11 | \
                 (int((((g_chi_invariant_m*255)*63)+127.5)/255)&0x3F)<<5 | \
                 (int((((b_chi_invariant_m*255)*31)+127.5)/255)&0x1F)

        if not (i % 16):
            sys.stdout.write("    ")
        sys.stdout.write("0x%04X" % (((rgb565&0x00ff)<<8)|((rgb565&0xff00)>>8)))
        if (i + 1) % 16:
            sys.stdout.write(", ")
        elif i != 65535:
            sys.stdout.write(",\n")
        else:
            sys.stdout.write("\n};\n")

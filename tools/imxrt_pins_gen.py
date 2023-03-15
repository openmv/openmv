#!/usr/bin/env python3
# This file is part of the OpenMV project.
#
# Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.

import sys
import re
import argparse
from collections import defaultdict

def parse_header_file(file_path):
    pads = defaultdict(dict)
    gpios = {}
    for i in range(0, 11):
        gpios[i] = 0

    af_pattern = "\s([0x]*[0-9a-fA-F]+)U*,*" * 5
    pad_pattern = [
        re.compile("#define IOMUXC_GPIO_(\w+_\d\d)_([a-zA-Z0-9]+)_(\w+)" + af_pattern),
        re.compile("#define IOMUXC_SNVS_(PMIC\w+_REQ)_([a-zA-Z0-9]+)_(\w+)"+ af_pattern),
        re.compile("#define IOMUXC_SNVS_(WAKEUP)_([a-zA-Z0-9]+)_*(\w*)"+ af_pattern)
    ]
    with open(file_path, "r") as file:
        for line in file:
            for r in pad_pattern:
                match = re.match(r, line)
                if match:
                    match = match.groups()
                    if (match[1].startswith("GPIO")):
                        gpios[int(match[1][4:])] += 1
                    pads[match[0]][int(match[-4], 16)] = match[1:]
                    break
    return pads, gpios

def main():
    parser = argparse.ArgumentParser(description = "Usage: python gen_iomux_pins.py <fsl_iomuxc.h>.")
    parser.add_argument("--iomux", action = "store", help = "Input iomux header files.", required=True)
    parser.add_argument("--header", action = "store_true", help = "Generate header file.", required=False, default=False)

    args = parser.parse_args()

    print("// THIS FILE IS AUTO-GENERATED DO NOT EDIT.\n")
    print("/*")
    print(" * This file is part of the OpenMV project.")
    print(" *")
    print(" * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>")
    print(" * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>")
    print(" *")
    print(" * This work is licensed under the MIT license, see the file LICENSE for details.")
    print(" */")

    pads, gpios = parse_header_file(args.iomux)

    if (args.header):
        # Generate the header file
        print("#ifndef __IMXRT_PADS_H__")
        print("#define __IMXRT_PADS_H__\n")
        print("#define MIMXRT_PAD_COUNT {:d}\n".format(len(pads)))

        for pad_name, pad_afs in pads.items():
            af_gpio = pad_afs[5] # GPIO AF is always 5
            for af_idx, af in pad_afs.items():
                if af_idx == 5:
                    af_name = "GPIO"
                elif af[0] in ["ENET", "ENET2", "SEMC"]:
                    af_name = "_".join(af[0:2])
                else:
                    af_name = af[0]
                print("#define {:s}_AF_{:s} {:s}".format(pad_name, af_name, af[-4]))
            print()

        for pad_name, pad_afs in pads.items():
            afs_count = len(pad_afs.keys()) - 1 # remove gpio AF.
            print("extern const imxrt_pad_t imxrt_pad_{:s};".format(pad_name))

        print("#endif // __IMXRT_PADS_H__")
    else:
        print("#include \"omv_gpio.h\"")
        print("#include \"fsl_gpio.h\"")
        print("#include \"fsl_iomuxc.h\"")
        print("#include \"mimxrt_pads.h\"")
        
        print()
        irq_handlers_list = []
        for port, pin_count in gpios.items():
            if (pin_count):
                print("static omv_gpio_irq_descr_t gpio_irq_descr_gpio{:d}[{:d}] = {{ NULL }};".format(port, pin_count))
                irq_handlers_list.append("    [{:d}] = gpio_irq_descr_gpio{:d},\n".format(port, port))
        print()
        print("omv_gpio_irq_descr_t *const gpio_irq_descr_table[{:d}] = {{\n{:s}}};".format(
                    len(gpios.keys()), "".join(irq_handlers_list)))

        # Generate the C file
        reg_mask = lambda r: int(r, 16) & 0x0000FFFF
        reg_base = lambda r: (int(r, 16) & 0xFFFF0000) >> 16
        for pad_name, pad_afs in pads.items():
            print()
            pad_afs_list = []
            for af_idx, af in pad_afs.items():
                if (af_idx == 5):
                    continue
                mux_reg, mux_mode, input_reg, input_daisy, config_reg = af[-5:]
                pad_afs_list.append("\n    {{ {:d}, 0x{:04X}, {:d} }},    // {:s}".format(
                            af_idx, reg_mask(input_reg), int(input_daisy, 16), af[0]))
            print("static const imxrt_pad_af_t imxrt_pad_{:s}_AF[{:d}] = {{{:s}\n}};".format(
                        pad_name, len(pad_afs_list), "".join(pad_afs_list)))

            # GPIO AF is always 5, use it to get mux_reg and config_reg which
            # are the same for all AFs, and 0, 0, for input register/daisy
            gpio, pin, mux_reg, mux_mode, input_reg, input_daisy, config_reg = pad_afs[5][-7:]
            afs_count = len(pad_afs.keys()) - 1 # remove GPIO AF.
            print("const imxrt_pad_t imxrt_pad_{:s} ="
                    " {{ {:s}, {:d}, 0x{:04X}, 0x{:04X}, 0x{:04X}, {:d}, imxrt_pad_{:s}_AF }};".
                    format(pad_name, gpio, int(pin[2:]), reg_base(mux_reg),
                        reg_mask(mux_reg), reg_mask(config_reg), afs_count, pad_name))

if __name__ == "__main__":
    main()

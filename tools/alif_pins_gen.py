#!/usr/bin/env python3
# This file is part of the OpenMV project.
#
# Copyright (c) 2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.

import sys
import re
import argparse
import xml.etree.ElementTree as ET

def parse_svd_file(svd_file):
    tree = ET.parse(svd_file)
    root = tree.getroot()

    pins_data = []

    pins_element = root.find(".//pins")

    if pins_element is not None:
        for pin_elem in pins_element.findall(".//pin"):
            pin_name = pin_elem.get("name")
            
            # Use regular expression to check if pin_name matches the pattern P<digit>_<digit>
            if re.match(r"^P\d+_\d+$", pin_name):
                mux_modes = pin_elem.get("muxModes").split(", ")
                
                # Replace "Reserved" with None and pad to 8 functions with None
                mux_modes = [None if mode == "Reserved" else mode for mode in mux_modes]
                mux_modes.extend([None] * (8 - len(mux_modes)))
                
                pins_data.append((pin_name,) + tuple(mux_modes))

    return pins_data

def main():
    parser = argparse.ArgumentParser(description = "Usage: python gen_iomux_pins.py <fsl_iomuxc.h>.")
    parser.add_argument("--svd", action = "store", help = "Input SVD file.", required=True)

    args = parser.parse_args()

    svd_file = sys.argv[1]
    pins_data = parse_svd_file(args.svd)

    print("// THIS FILE IS AUTO-GENERATED DO NOT EDIT.\n")
    print("/*")
    print(" * This file is part of the OpenMV project.")
    print(" *")
    print(" * Copyright (c) 2024 Ibrahim Abdelkader <iabdalkader@openmv.io>")
    print(" * Copyright (c) 2024 Kwabena W. Agyeman <kwagyeman@openmv.io>")
    print(" *")
    print(" * This work is licensed under the MIT license, see the file LICENSE for details.")
    print(" */")

    for pin_info in pins_data:
        pin_name, *mux_modes = pin_info
        port, pin = pin_name.split("P")[1].split("_")
        pin_name = "PORT_"+port+"_PIN_"+pin
        print("// ", pin_name)
        for i, function_name in enumerate(mux_modes):
            if function_name is not None:
                # Remove the GPIO number from the generated mux GPIO function
                if i == 0 and function_name.startswith("GPIO"):
                    function_name = "GPIO"
                # Remove the last part of a mux function name if it has 3 parts
                parts = function_name.split("_")
                if len(parts) == 3:
                    function_name = "_".join(parts[:2])

                print(f"#define {f'{pin_name}_AF_{function_name}'.ljust(30)} PINMUX_ALTERNATE_FUNCTION_{i}")
        print("")

if __name__ == "__main__":
    main()

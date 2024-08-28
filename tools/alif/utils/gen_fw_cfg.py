#!/usr/bin/env python3
"""
    __author__ = ""
    __copyright__ = "ALIF Seminconductor"
    __version__ = "0.1.0"
    __status__ = "Dev"
"""
# pylint: disable=unused-argument, invalid-name, bare-except

import sys
import argparse
import json
import os
import utils.toc_common

TOOL_VERSION ="0.1.001"      # Define Version constant for each separate tool

EXIT_WITH_ERROR = 1

FW_CFG_FILE = "build/fw_cfg.json"

def mram_size_to_address(mram_size):
    mram_size_int = 0
    if mram_size == '1.8':       # @todo this should be improved
       mram_size_int = 1888256   # 0x001CD000 - manual mapping, since 1.8MB is not exactly 0x001CC800
    else:
        mram_size_int = int(float(mram_size) * 1024 * 1024)
    mram_base_int = int("0x80000000", 16)
    stoc_start = mram_base_int + mram_size_int
    atoc_end = stoc_start - 1
    atoc_end_str = "0x" + hex(atoc_end).lstrip("0x").upper()
    stoc_start_str = "0x" + hex(stoc_start).lstrip("0x").upper()
    return atoc_end_str, stoc_start_str

def handle_sram_rev_b(sram_size, firewall_components, protected_areas):

    if "13.5" == sram_size: # no SRAM wounding
        #print("**** no SRAM wounding")
        return

    # load the definitions for REV_B
    with open('firewall/sram_rev_b.json', "r") as json_file:
        sram_json = json.load(json_file)

    # Wound the Modem TCMs - FC #12
    firewall_components.append(sram_json["firewall_components"][0])
    protected_areas.append(sram_json["protected_areas"][0])

    if "8.25" == sram_size: # no other wounding
        #print("**** 8.25MB wounding")
        return

    if "5.75" == sram_size: # E3 with 5.75MB SRAM
        # Wound SRAM1 - FC #4 - XNVM
        firewall_components.append(sram_json["firewall_components"][1])
        protected_areas.append(sram_json["protected_areas"][1])
        #print("**** 5.75MB wounding")
        return

    if "4.5" == sram_size: # E1
        # Wound SRAM1 - FC #4 - XNVM
        firewall_components.append(sram_json["firewall_components"][1])
        protected_areas.append(sram_json["protected_areas"][1])

        # Wound M55-HP TCMs - 0x50000000-0x57FFFFFF
        firewall_components.append(sram_json["firewall_components"][4])
        protected_areas.append(sram_json["protected_areas"][4])
        #print("**** 4.5MB wounding")
        return

    if "3.75" == sram_size: #E3 with 3.75MB SRAM:
        # Wound SRAM0 - FC #5 - CVM
        firewall_components.append(sram_json["firewall_components"][3])
        protected_areas.append(sram_json["protected_areas"][3])

        # Wound SRAM1 to 2MB
        firewall_components.append(sram_json["firewall_components"][2])
        protected_areas.append(sram_json["protected_areas"][2])
        #print("**** 3.75MB wounding")
        return

    # Invalid sram_size - wound SRAM0, SRAM1, and the M55-HP TCMs
    firewall_components.append(sram_json["firewall_components"][1])
    protected_areas.append(sram_json["protected_areas"][1])
    firewall_components.append(sram_json["firewall_components"][3])
    protected_areas.append(sram_json["protected_areas"][3])
    firewall_components.append(sram_json["firewall_components"][4])
    protected_areas.append(sram_json["protected_areas"][4])
    print("**** Invalid SRAM size!")

def write_json(json_content):
    out_file = FW_CFG_FILE
    if (os.path.exists(out_file)):
        os.remove(out_file)

    with open(out_file, 'w') as f:
        json.dump(json_content, f, indent=2)

def gen_fw_cfg_icv(family, mram_size, sram_size, device_revision):

    atoc_end_address, stoc_start_address = mram_size_to_address(mram_size)
    print(family)
    if family == "Balletto-B1-Series":
        print("*** generate FW cfg for Spark")
        with open('firewall/fw_cfg_spark.json', "r") as json_file:
            spark_json = json.load(json_file)
            spark_json["firewall_components"][1]["configured_regions"][0]["end_address"] = atoc_end_address
            spark_json["protected_areas"][2]["start_address"] = stoc_start_address
            write_json(spark_json)
        return

    # MRAM
    # The following code is tightly coupled with the structure of the file mram.json.
    # Should we define the data structures in Python code instead of reading them from a JSON file?
    with open('firewall/mram.json', "r") as json_file:
        mram_json = json.load(json_file)

    mram_json["firewall_components"][0]["configured_regions"][0]["end_address"] = atoc_end_address
    mram_json["protected_areas"][0]["start_address"] = stoc_start_address

    is_rev_a = device_revision == 'A0' or device_revision == 'A1'
    # generate Master FC configuration
    if is_rev_a:
        extra_cfg = "firewall/fw_cfg_rev_a.json"
        with open(extra_cfg, "r") as extra_cfg_file:
            extra_cfg_json = json.load(extra_cfg_file)
            firewall_components = extra_cfg_json['firewall_components']
    else:
        firewall_components = []

    # generate Slave FC configuration
    firewall_components.append(mram_json['firewall_components'][0])
    protected_areas = mram_json['protected_areas']

    # SRAM
    if not is_rev_a: # rev B
        handle_sram_rev_b(sram_size, firewall_components, protected_areas)

    out_json = {}
    out_json['firewall_components'] = firewall_components
    out_json['protected_areas'] = protected_areas

    write_json(out_json)

def update_fw_cfg_oem():
    with open('build/config/app-device-config.json', "r") as json_file:
        json_text = json.load(json_file)
    with open('build/config/app-device-config.json', "w") as json_file:
        json.dump(json_text, json_file, indent=2)

        # TODO: update/generate the Master FC sections
        #print("json_text: ", json_text)
        #json.dump("", json_file, indent=2)


def main():
    """
    """
    if sys.version_info.major == 2:
        print("[ERROR] You need Python 3 for this application!")
        sys.exit(EXIT_WITH_ERROR)

    # Deal with Command Line
    parser = argparse.ArgumentParser(description=
                                     'Generate STOC device configuration')
    parser.add_argument("-m", "--mram", type=str, default="5.5", help='MRAM size in MB, default 5.5')
    parser.add_argument("-s", "--sram", type=str, default="13.5", help="SRAM size in MB, default 13.5")
    parser.add_argument("-V" , "--version",
                        help="Display Version Number", action="store_true")
    args = parser.parse_args()
    if args.version:
        print(TOOL_VERSION)
        sys.exit()

    mram_size = args.mram.rstrip('0').rstrip('.')
    sram_size = args.sram.rstrip('0').rstrip('.')

    gen_fw_cfg_icv("", mram_size, sram_size)

if __name__ == "__main__":
    main()

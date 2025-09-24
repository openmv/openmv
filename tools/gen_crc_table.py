#!/usr/bin/env python3
#
# This file is part of the OpenMV project.
# Copyright (C) 2025 OpenMV, LLC.
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Generate CRC lookup tables for OpenMV CRC implementation

import argparse


def generate_crc16_table(polynomial):
    """Generate CRC16 lookup table"""
    table = []
    for i in range(256):
        crc = i << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ polynomial
            else:
                crc = crc << 1
            crc &= 0xFFFF
        table.append(crc)
    return table


def generate_crc32_table(polynomial):
    """Generate CRC32 lookup table"""
    table = []
    for i in range(256):
        crc = i << 24
        for _ in range(8):
            if crc & 0x80000000:
                crc = (crc << 1) ^ polynomial
            else:
                crc = crc << 1
            crc &= 0xFFFFFFFF
        table.append(crc)
    return table


def format_c_array(table, name, data_type):
    """Format array as C code"""
    lines = [f"static const {data_type} {name}[256] = {{"]

    for i in range(0, len(table), 8):
        row = table[i : i + 8]
        if data_type == "uint16_t":
            formatted_row = ", ".join(f"0x{val:04X}" for val in row)
        else:  # uint32_t
            formatted_row = ", ".join(f"0x{val:08X}" for val in row)
        lines.append(f"    {formatted_row},")

    lines.append("};")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description="Generate CRC lookup tables for OpenMV CRC implementation"
    )
    parser.add_argument(
        "--crc16-poly",
        type=lambda x: int(x, 0),
        default=0xF94F,
        help="CRC16 polynomial (default: 0xF94F)",
    )
    parser.add_argument(
        "--crc32-poly",
        type=lambda x: int(x, 0),
        default=0xFA567D89,
        help="CRC32 polynomial (default: 0xFA567D89)",
    )

    args = parser.parse_args()

    # Generate CRC16 table
    crc16_table = generate_crc16_table(args.crc16_poly)

    # Generate CRC32 table
    crc32_table = generate_crc32_table(args.crc32_poly)

    print(f"// CRC16 lookup table for polynomial 0x{args.crc16_poly:04X}")
    print(format_c_array(crc16_table, "crc16_table", "uint16_t"))
    print()
    print(f"// CRC32 lookup table for polynomial 0x{args.crc32_poly:08X}")
    print(format_c_array(crc32_table, "crc32_table", "uint32_t"))


if __name__ == "__main__":
    main()

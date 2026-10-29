#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (C) 2026 OpenMV, LLC.
#
# Generate a complete linker script from a port's .ld.S template and
# board_config.h. Replaces the standalone CPP step in the build.
#
# The script:
#   1. Parses board_config.h to discover GC, UMA, and DMA block definitions
#   2. Generates linker script fragments for those blocks/tables
#   3. Concatenates the generated fragment with the port's .ld.S template
#   4. Pipes everything through CPP to produce the final .lds
#
# UMA blocks with UMA_DMA_Dn flags are embedded inside the corresponding
# .dma.memoryN section to share its MPU region and power-of-2 alignment.
#
# Usage:
#   python3 tools/gen_linker.py \
#       --board FOO \
#       --ldscript ports/stm32/stm.ld.S \
#       > build/stm.lds

import argparse
import os
import re
import subprocess
import sys


def parse_defines(path, cpp_flags):
    """Run CPP -dM on a header and return {name: value} for all defines."""
    cmd = ["arm-none-eabi-cpp", "-dM", "-E"] + cpp_flags + [path]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"cpp -dM failed: {result.stderr}", file=sys.stderr)
        sys.exit(1)
    defines = {}
    for line in result.stdout.splitlines():
        m = re.match(r"#define\s+(\w+)\s+(.*)", line)
        if m:
            defines[m.group(1)] = m.group(2).strip()
    return defines


def collect_numbered(defines, prefix, suffix):
    """Collect indices where prefix<N>suffix is defined."""
    indices = []
    n = 0
    while f"{prefix}{n}{suffix}" in defines:
        indices.append(n)
        n += 1
    return indices


def block_dma_domain(n, defines):
    """Return DMA domain number by matching UMA_DMA_Dn in the flags string."""
    flags = defines.get(f"OMV_UMA_BLOCK{n}_FLAGS", "")
    m = re.search(r"UMA_DMA_D(\d+)", flags)
    return int(m.group(1)) if m else None


def dma_region_needs_align(memory):
    """ARMv7-M MPU regions must be aligned to their size. TCMs are never cached, so
    their DMA regions need no MPU region and no alignment."""
    return not memory.endswith("TCM")


def collect_dma_domains(defines):
    """Collect which DMA domains exist (D1, D2, D3, ...)."""
    domains = []
    for key in defines:
        m = re.match(r"OMV_DMA_MEMORY_D(\d+)", key)
        if m:
            domains.append(int(m.group(1)))
    domains.sort()
    return domains


def gen_fragment(defines):
    """Generate the GC/UMA/DMA linker script fragment."""
    gc_blocks = collect_numbered(defines, "OMV_GC_BLOCK", "_MEMORY")
    uma_blocks = collect_numbered(defines, "OMV_UMA_BLOCK", "_MEMORY")
    dma_domains = collect_dma_domains(defines)

    # Classify UMA blocks: standalone vs DMA-embedded.
    standalone = []
    dma_embedded = {d: [] for d in dma_domains}

    for n in uma_blocks:
        domain = block_dma_domain(n, defines)
        if domain is not None and domain in dma_embedded:
            dma_embedded[domain].append(n)
        else:
            standalone.append(n)

    lines = []
    I = "  "  # 2-space indent to match common.ld.S style

    # --- Extra GC blocks (block 0 is always defined in common.ld.S) ---
    extra_gc = [n for n in gc_blocks if n > 0]
    if extra_gc:
        for n in extra_gc:
            lines.append(f".gc.block{n} (NOLOAD) : ALIGN(4)")
            lines.append("{")
            lines.append(f"{I}. = . + OMV_GC_BLOCK{n}_SIZE;")
            lines.append(f"{I}. = ALIGN(4);")
            lines.append(f"}} >OMV_GC_BLOCK{n}_MEMORY")

        lines.append(".gc.blocks.table (READONLY) : ALIGN(4)")
        lines.append("{")
        lines.append(f"{I}_gc_blocks_table_start = .;")
        for n in extra_gc:
            lines.append(f"{I}LONG (ADDR(.gc.block{n})) LONG (SIZEOF(.gc.block{n}))")
        lines.append(f"{I}_gc_blocks_table_end = .;")
        lines.append(f"{I}. = ALIGN(4);")
        lines.append("} > FLASH_TEXT")

    # --- DMA sections with embedded UMA blocks ---
    for d in dma_domains:
        embedded = dma_embedded.get(d, [])
        lines.append(f".dma.memory{d} (NOLOAD) : ALIGN(32)")
        lines.append("{")
        lines.append(f"{I}*(.d{d}_dma_buffer)")
        for n in embedded:
            lines.append(f"{I}. = ALIGN(32);")
            lines.append(f"{I}_uma_dma_block{n}_start = .;")
            lines.append(f"{I}. = . + OMV_UMA_BLOCK{n}_SIZE;")
        lines.append(f"{I}. = OMV_ALIGN_DMA_SECTION(.dma.memory{d});")
        lines.append(f"}} >OMV_DMA_MEMORY_D{d}")
        if dma_region_needs_align(defines[f"OMV_DMA_MEMORY_D{d}"]):
            lines.append(f"OMV_ASSERT_DMA_SECTION(.dma.memory{d})")

    # --- DMA memory table ---
    has_dma0 = "OMV_DMA_MEMORY" in defines or "OMV_MAIN_MEMORY" in defines
    if has_dma0 and dma_region_needs_align(defines.get("OMV_DMA_MEMORY", defines.get("OMV_MAIN_MEMORY"))):
        lines.append("OMV_ASSERT_DMA_SECTION(.dma.memory0)")
    if has_dma0 or dma_domains:
        lines.append(".dma.memory.table (READONLY) : ALIGN(4)")
        lines.append("{")
        lines.append(f"{I}_dma_memory_table_start = .;")
        if has_dma0:
            lines.append(f"{I}LONG (ADDR(.dma.memory0)) LONG (SIZEOF(.dma.memory0))")
        for d in dma_domains:
            lines.append(f"{I}LONG (ADDR(.dma.memory{d})) LONG (SIZEOF(.dma.memory{d}))")
        lines.append(f"{I}_dma_memory_table_end = .;")
        lines.append(f"{I}. = ALIGN(4);")
        lines.append("} > FLASH_TEXT")

    # --- Standalone UMA block sections ---
    for n in standalone:
        lines.append(f".uma.block{n} (NOLOAD) : ALIGN(4)")
        lines.append("{")
        lines.append(f"{I}. = . + OMV_UMA_BLOCK{n}_SIZE;")
        lines.append(f"{I}. = ALIGN(4);")
        lines.append(f"}} >OMV_UMA_BLOCK{n}_MEMORY")

    # --- UMA blocks table ---
    if uma_blocks:
        lines.append(".uma.blocks.table (READONLY) : ALIGN(4)")
        lines.append("{")
        lines.append(f"{I}_uma_blocks_table_start = .;")
        for n in uma_blocks:
            domain = block_dma_domain(n, defines)
            if domain is not None and domain in dma_embedded:
                lines.append(
                    f"{I}LONG (_uma_dma_block{n}_start) "
                    f"LONG (OMV_UMA_BLOCK{n}_SIZE) "
                    f"LONG (OMV_UMA_BLOCK{n}_FLAGS)"
                )
            else:
                lines.append(
                    f"{I}LONG (ADDR(.uma.block{n})) "
                    f"LONG (SIZEOF(.uma.block{n})) "
                    f"LONG (OMV_UMA_BLOCK{n}_FLAGS)"
                )
        lines.append(f"{I}_uma_blocks_table_end = .;")
        lines.append(f"{I}. = ALIGN(4);")
        lines.append("} > FLASH_TEXT")

    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description="Generate a complete linker script with GC/UMA/DMA sections."
    )
    parser.add_argument("--board", required=True,
                        help="Board/target name (e.g. MPS2_AN500)")
    parser.add_argument("--ldscript", required=True,
                        help="Path to port's .ld.S template")

    # Strip leading '--' separator if present.
    args, cpp_flags = parser.parse_known_args()
    if cpp_flags and cpp_flags[0] == "--":
        cpp_flags = cpp_flags[1:]

    board_dir = f"boards/{args.board}"
    cpp_flags += ["-DLINKER_SCRIPT", f"-Icommon", f"-I{board_dir}"]

    # 1. Parse board_config.h for define names (CPP resolves conditionals).
    defines = parse_defines(f"{board_dir}/board_config.h", cpp_flags)

    # 2. Generate the fragment.
    fragment = gen_fragment(defines)

    # 3. Read the port's linker script template.
    with open(args.ldscript) as f:
        ldscript = f.read()

    # 4. Wrap fragment in its own SECTIONS block and append after ldscript.
    fragment = "SECTIONS\n{\n" + fragment + "\n}\n"
    combined = ldscript + "\n" + fragment
    cmd = ["arm-none-eabi-cpp", "-P", "-E"] + cpp_flags + ["-"]
    result = subprocess.run(cmd, input=combined, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"cpp failed: {result.stderr}", file=sys.stderr)
        sys.exit(1)

    sys.stdout.write(result.stdout)


if __name__ == "__main__":
    main()

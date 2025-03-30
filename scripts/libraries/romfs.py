import os
import uctypes


def ls_romfs():
    # Define possible alignment sizes in descending order
    alignments = [128, 64, 32, 16, 8, 4]

    for fname in os.listdir("/rom"):
        with open("/rom/" + fname, "rb") as file:
            address = uctypes.addressof(file) & 0xFFFFFFF
            size = len(memoryview(file))
            aligned = False
            # Check alignment for each size, starting from the highest alignment
            for alignment in alignments:
                if address % alignment == 0:
                    print(
                        f"addr: 0x{address:08X}  size: {size:<8}  alignment: {alignment:<4}  name: {fname}"
                    )
                    aligned = True
                    break
            # If not aligned to any of the specified sizes
            if not aligned:
                print(
                    f"addr: 0x{address:08X}  size: {size:<8}  alignment: NOT aligned name: {fname}"
                )

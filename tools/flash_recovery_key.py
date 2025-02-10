#!/usr/bin/env python3
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
# 
# Generates a binary that triggers flash recovery process when downloaded
# to special DFU partitions.
if __name__ == "__main__":
    with open("key.bin", "wb") as f:
        f.write(b'\x15\x9e}B\x96\x1aq\xebs\xa3&)+\x08\t\x0e' + bytearray(4080))
    print("Recovery key generated and saved.")

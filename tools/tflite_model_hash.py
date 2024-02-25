#!/usr/bin/env python3
# This file is part of the OpenMV project.
#
# Copyright (c) 2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.

import tflite
import struct
import sys


with open(sys.argv[1], "rb") as f:
    model = tflite.Model.GetRootAs(f.read())

opcodes = []
graph = model.Subgraphs(0)
for i in range(graph.OperatorsLength()):
    op = graph.Operators(i)
    opcode = model.OperatorCodes(op.OpcodeIndex()).BuiltinCode()
    opcodes.append(opcode)
    # print(tflite.opcode2name(opcode))


def hash_djb2(s):
    hash = 5381
    for x in s:
        hash = ((hash << 5) + hash) + x
    return hash & 0xFFFFFFFF


packed = struct.pack("I" * len(opcodes), *opcodes)
print(hex(hash_djb2(packed)))

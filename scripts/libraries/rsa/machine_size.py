# -*- coding: utf-8 -*-
# SPDX-FileCopyrightText: 2011 Sybren A. Stüvel <sybren@stuvel.eu>
#
# SPDX-License-Identifier: Apache-2.0

"""
`rsa.machine_size`
====================================================

Detection of 32-bit and 64-bit machines and byte alignment.
"""

import sys

try:
    from typing import Tuple

    try:
        from typing import Literal
    except ImportError:
        from typing_extensions import Literal
except ImportError:
    pass


MAX_INT = sys.maxsize
MAX_INT64 = (1 << 63) - 1
MAX_INT32 = (1 << 31) - 1
MAX_INT16 = (1 << 15) - 1

# Determine the word size of the processor.
if MAX_INT == MAX_INT64:
    # 64-bit processor.
    MACHINE_WORD_SIZE = 64
elif MAX_INT == MAX_INT32:
    # 32-bit processor.
    MACHINE_WORD_SIZE = 32
else:
    # Else we just assume 64-bit processor keeping up with modern times.
    MACHINE_WORD_SIZE = 64


def get_word_alignment(
    num: int,
    force_arch: int = 64,
    _machine_word_size: Literal[64, 32] = MACHINE_WORD_SIZE,
) -> Tuple[int, int, int, str]:
    """
    Returns alignment details for the given number based on the platform
    Python is running on.

    :param int num:
        Unsigned integral number.
    :param force_arch:
        If you don't want to use 64-bit unsigned chunks, set this to
        anything other than 64. 32-bit chunks will be preferred then.
        Default 64 will be used when on a 64-bit machine.
    :param _machine_word_size:
        (Internal) The machine word size used for alignment.
    :return:
        4-tuple::

            (word_bits, word_bytes,
             max_uint, packing_format_type)
    """
    max_uint64 = 0xFFFFFFFFFFFFFFFF
    max_uint32 = 0xFFFFFFFF
    max_uint16 = 0xFFFF
    max_uint8 = 0xFF

    if force_arch == 64 and _machine_word_size >= 64 and num > max_uint32:
        # 64-bit unsigned integer.
        return 64, 8, max_uint64, "Q"
    if num > max_uint16:
        # 32-bit unsigned integer
        return 32, 4, max_uint32, "L"
    if num > max_uint8:
        # 16-bit unsigned integer.
        return 16, 2, max_uint16, "H"
    # 8-bit unsigned integer.
    return 8, 1, max_uint8, "B"

#!/usr/bin/python3
"""
   @brief Clock Info decode and display

   __author__ onyettr
"""
# pylint: disable=unused-argument, invalid-name, consider-using-f-string
import struct
from isp_print import isp_print_color
from toc_decode import format_hex, format_cpu
from toc_decode import TOC_IMAGE_CPU_LAST

def display_cpu_info(message):
    """
        display_cpu
        show CPU status return
    """
    isp_print_color("blue", " +--------+------+------------+\n")
    isp_print_color("blue", " |  CPU   |Booted| Boot Addr  |\n")
    isp_print_color("blue", " +--------+------+------------+\n")

    # unpack the data in the response. 7 CPUS x 6-bytes of data
    # @todo magic numbers need to go away
    for stride in range(2,42,6):
        element = message[stride:stride+6]      # Slice one record

        (cpu_id,) = struct.unpack("<B",bytes(element[0:1]))
        (booted,) = struct.unpack("<B",bytes(element[1:2]))
        (address,)= struct.unpack("<I",bytes(element[2:6]))

        if cpu_id >= TOC_IMAGE_CPU_LAST:  # valid values [0..8]
            continue
        if booted > 1:  # valid values: 0 or 1
            continue

        boot_string = "YES" if booted else " "

        isp_print_color("blue",
                        " |%7s |%6s| %10s |\n" % (
                        format_cpu(cpu_id),
                        boot_string.center(6),
                        format_hex(address)
                        ))

    isp_print_color("blue", " +--------+------+------------+\n")
    print(" ")

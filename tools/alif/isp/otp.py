#!/usr/bin/python3

"""
   @brief OTP decode and display
   __author__ onyettr
"""
# pylint: disable=unused-argument, invalid-name
from isp_print import isp_print_color

def display_otp_info(word_offset, response):
    """
        display_otp_info
            display otp data from Target
    """
    otp_value = int.from_bytes(response[2:6],"little")
    isp_print_color("blue","0x{:04x} {}".format(word_offset,hex(otp_value)))
    print(" ")
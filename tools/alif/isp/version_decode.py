#!/usr/bin/python3

"""
   @brief Version data decode

   __author__ onyettr
"""
# pylint: disable=unused-argument, invalid-name
import struct
from isp_print import isp_print_color
from otp_mfgr_decode import decode_otp_manufacture

# Wounding bits 
OTP_WOUNDING_ALIF_DFT       = (1 << 0)
OTP_WOUNDING_RESERVED_0     = (1 << 1)
OTP_WOUNDING_RESERVED_1     = (1 << 2)
OTP_WOUNDING_DISABLE_A32_0  = (1 << 3)
OTP_WOUNDING_DISABLE_A32_1  = (1 << 4)
OTP_WOUNDING_DISABLE_M55_HP = (1 << 5)
OTP_WOUNDING_DISABLE_MODEM  = (1 << 6)
OTP_WOUNDING_DISABLE_GNSS   = (1 << 7)

# Wounding bit meanings
WOUNDING_lut = {
    0 : "DFT    Disable",
    1 : "Reserved",
    2 : "Reserved",
    3 : "A32_0  Disable",
    4 : "A32_1  Disable",
    5 : "M55_HP Disable",
    6 : "Modem  Disable",
    7 : "GNSS   Disable"
}

# Life Cycle State
LCS_lut = {
    0 : "CM",
    1 : "DM",
    5 : "SE",
    7 : "RMA"
}

def display_version_info(message):
    """
        display_version_info
            display version data sent back from Target
    """

def display_string(field):
    #isp_print_color("blue"," " + str(field) + "\t\t=  ")
    for ch in field:
        l = hex(ch)[2:]
        if len(l) == 1:
            l = '0' + l
        isp_print_color("blue", l) 
    print("")

def version_decode(isp, message):
    """
        version_decode
            parse version data
    """

    if len(message) == 7:
        isp_print_response("blue",message)

    if len(message) == 136:
        (version,ALIF_PN,) = struct.unpack("<I16s",bytes(message[2:22]))

        isp_print_color("blue"," Version\t=  ")
        isp_print_color("blue", "0x%04X\n" % (version))

        isp_print_color("blue"," ALIF_PN\t=  ")
        raw_part = int.from_bytes(ALIF_PN,"little")
        if raw_part == 0x0:
            ascii_list = "0x0"
        else:
            ascii_list = [chr(ch) for ch in ALIF_PN]
        isp_print_color("blue", "%s\n" % (''.join(ascii_list)))

        HBK0 = (message[22:38]) #bytes 22-37 (limit 38)
        isp_print_color("blue"," HBK0\t\t=  ")
        display_string(HBK0)

        HBK1 = (message[38:54]) #bytes 38-53 (limit 54)
        isp_print_color("blue"," HBK1\t\t=  ")
        display_string(HBK1)

        HBK_FW = (message[54:74]) #bytes 54-73 (limit 74)
        isp_print_color("blue"," HBK_FW\t\t=  ")
        #isp_print_color("blue",hex(int.from_bytes(HBK_FW,"little")))
        display_string(HBK_FW)

        config = (message[74:78]) #bytes 74-77 (limit 78)
        wounding_bits = int.from_bytes(config,"little")
        isp_print_color("blue"," Wounding\t=  ")
        isp_print_color("blue",hex(int.from_bytes(config,"little")))
        for each_bit in range(8):
            if (wounding_bits >> each_bit) & 0x1 == 0x1:
                isp_print_color("blue","\n\t")
                isp_print_color("blue",WOUNDING_lut.get(each_bit))
        print("")

        DCU = (message[78:94]) #bytes 78-93 (limit 94)
        isp_print_color("blue"," DCU\t\t=  ")
        isp_print_color("blue",hex(int.from_bytes(DCU,"little")))
        print("")

        MfgData = (message[94:126]) #bytes 94-125 (limit 126)
        MfgDatatInt = int.from_bytes(MfgData,"little")
        isp_print_color("blue"," MfgData\t=  ")
        #isp_print_color("blue",hex(MfgDatatInt))
        display_string(MfgData)
        if  MfgDatatInt != 0:
            decode_otp_manufacture(MfgData)

        SerialN = (message[127:134]) #bytes 127-133 (limit 134)
        isp_print_color("blue"," SerialN\t=  ")
        isp_print_color("blue",hex(int.from_bytes(SerialN,"little")))
        print("")

        LCS = message[134]
        isp_print_color("blue"," LCS\t\t=  0x%x (%s)" % (
                        LCS,
                        LCS_lut.get(LCS))
                        )
        print("")
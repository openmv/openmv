#!/usr/bin/python3

"""
   @brief OTP data decode - Alif Manufacturing info
   - 256 Bits (32 bytes) of total data
     * CP1    72    9
     * CP2    72    9
   __author__ onyettr
"""
# pylint: disable=unused-argument, invalid-name, consider-using-f-string
import struct
from isp_print import isp_print_color

SRAM0_SIZE_lut = {
        0 : "4.0",
        1 : "2.0"
    }

SRAM1_SIZE_lut = {
        0 : "2.5",
        1 : "0.0"
    }

MRAM_SIZE_lut = {
        0 : "6.0",
        1 : "4.5",
        2 : "3.0",
        3 : "1.5"
    }

fab_id_lut = {
        0x0 : "UAS"
    }

class CP_data_t:
    """
        OTP manufactoring data
        @todo need to add more methods
    """
    # pylint: disable=too-many-instance-attributes
    def __init__(self):
        """
            CP Fuses - 72bits
        """
        self.cp_mfgr_id_x_loc = 0
        self.cp_mfgr_id_y_loc = 0
        self.cp_mfgr_id_wfr_id = 0
        self.cp_mfgr_id_lot_id = 0
        self.cp_mfgr_id_lot_id_year = 0
        self.cp_mfgr_id_lot_id_FABID = 0
        self.cp_mfgr_id_lot_id_workweek = 0
        self.cp_mfgr_id_lot_id_serialno = 0
        self.cp_memory_sizes = 0
        self.cp_memory_sizes_SRAM0 = 0
        self.cp_memory_sizes_SRAM1 = 0
        self.cp_memory_sizes_MRAM = 0
        self.cp_test_revision = 0
        self.cp_test_revision_revision = 0
        self.cp_temperature = 0
        self.cp_last_byte = 0
        self.cp_CP1_write_ok = 0
        self.cp_SRAM_repaired = 0
        self.cp_future = 0

    def otp_manufacture_bank_0_decode(self, message):
        """
            otp_manufacture_cp_fuses_decode
                parse otp Mfgr data
        """

        if len(message) == 0:
            return

        """
         CP1 decode 72 Bits / 9 BYTES
         Byte         Field description
           0        x-loc
           1        y-loc
           2        lot _id - 32 bits
             3      year
             4      FabId:2 Work Week: 6
             5      Serial No
             6        RAM Sizes SRAM0:1 SRAM1:1 MRAM:2 CP1 Bin#:4
           7        Test ProgramRev:4 Temperature:4
           8        SRAM repaired:1 FUTURE:6
           
              0        1      2      3       4      5      6      7 
           ['0x97', '0x68', '0x2', '0x59', '0x4', '0x1', '0x0', '0x0',
        """
#        hexized = [hex(x) for x in message]
#        print(hexized)
        (self.cp_mfgr_id_x_loc,) = struct.unpack("<B",bytes(message[0:1]))
        (self.cp_mfgr_id_y_loc,) = struct.unpack("<B",bytes(message[1:2]))
        (self.cp_mfgr_id_wfr_id,) = struct.unpack("<B",bytes(message[2:3]))
        (self.cp_mfgr_id_lot_id_year,) = struct.unpack("<B",bytes(message[3:4]))
#        print("Year ", hex(self.cp_mfgr_id_lot_id_year))
        (self.cp_mfgr_id_lot_id_serialno,) = struct.unpack("<B",bytes(message[5:6]))

        (byte0,) = struct.unpack("<B",bytes(message[4:5]))
#        (self.cp_mfgr_id_x_loc,) = struct.unpack("<B",bytes(message[3:4]))
#        (self.cp_mfgr_id_y_loc,) = struct.unpack("<B",bytes(message[2:3]))
#        (self.cp_mfgr_id_wfr_id,) = struct.unpack("<B",bytes(message[1:2]))
#        (self.cp_mfgr_id_lot_id_year,) = struct.unpack("<B",bytes(message[0:1]))
#        (byte0,) = struct.unpack("<B",bytes(message[7:8]))
#        (self.cp_mfgr_id_lot_id_serialno,) = struct.unpack("<B",bytes(message[6:7]))

#        print("Byte0 ", bin(byte0))
#        print("Type  ", type(byte0))
        self.cp_mfgr_id_lot_id_FABID = (byte0 & 0x3)
        self.cp_mfgr_id_lot_id_workweek = (byte0 & 0xC)
#        print("WW    ", hex(self.cp_mfgr_id_lot_id_workweek))
#        print("FABID ", hex(self.cp_mfgr_id_lot_id_FABID))

    def display_manufacture_otp_info(self):
        """
            display_display_manufacture_otp_info
                display otp data sent back from Target
        """

        isp_print_color('blue', "\t+ x-loc    %d\n" %
                        (self.cp_mfgr_id_x_loc))
        isp_print_color('blue', "\t+ y-loc    %d\n" %
                        (self.cp_mfgr_id_y_loc))
        isp_print_color('blue', "\t+ Wafer ID %d\n" %
                        (self.cp_mfgr_id_wfr_id))
#        isp_print_color('blue', "\t+ lot_id   %d\n" %
#                        (self.cp_mfgr_id_lot_id))
        size_bit = self.cp_mfgr_id_lot_id & 0x1
        isp_print_color('blue', "\t+ lot ID   %s%c%02X%03X\n" %
                        (
                         fab_id_lut.get(self.cp_mfgr_id_lot_id_FABID),
                         chr(self.cp_mfgr_id_lot_id_year),
                         self.cp_mfgr_id_lot_id_workweek,
                         self.cp_mfgr_id_lot_id_serialno
                        ))
#        isp_print_color('blue', "\t+ Week#    %d\n" %
#                        (self.cp_mfgr_id_lot_id_workweek))
#        isp_print_color('blue', "\t+ Seriall#  %d\n" %
#                        (self.cp_mfgr_id_lot_id_serialno))
#        isp_print_color('blue', "\t+ SRAM0 Size      %s MB\n" % (
#                        SRAM0_SIZE_lut.get(size_bit)))
#        size_bit = self.cp_mfgr_id_lot_id & 0x2
#        isp_print_color('blue', "\t+ SRAM1 Size      %s MB\n" % (
#                        SRAM1_SIZE_lut.get(size_bit)))
#        size_bit = (self.cp_mfgr_id_lot_id & 0xC) >> 2
#        isp_print_color('blue', "\t+ MRAM  Size      %s MB\n" % (
#                        MRAM_SIZE_lut.get(size_bit)))

def swap32(x):
    return (((x << 24) & 0xFF000000) |
            ((x <<  8) & 0x00FF0000) |
            ((x >>  8) & 0x0000FF00) |
            ((x >> 24) & 0x000000FF))

def decode_otp_manufacture(message):
    """
        decode_otp_manufacture
            decode and display the OTP bits for manufatcuring
    """
    Bank0 = CP_data_t()

    Bank0.otp_manufacture_bank_0_decode(message)
#    isp_print_color('blue', "CP1\n")
    Bank0.display_manufacture_otp_info()

#    CP2_info.display_manufacture_otp_info()

if __name__ == "__main__":
    message = [ 0x1, 0x2, 0x3, 0x0, 0x0,
                0x1, 0x2, 0x3, 0x0, 0x0,
                0x1, 0x2, 0x3, 0x0, 0x0,
                0x1, 0x2, 0x3, 0x0, 0x0,
                0x1, 0x2, 0x3, 0x0, 0x0,
                0x1, 0x2, 0x3, 0x0, 0x0,
                0x1, 0x2, 0x3, 0x0, 0x0,
                0x1, 0x2, 0x3, 0x0, 0x0]
    """
    Unit #1:   0x2F002144:  0x5902FF0B
               0x2F002148:  0x00000104

    Decoded by ATE as:
           x-loc: 11
           y-loc: 255
           wafer ID: 2
           lot ID: UASY04001 
    """
    real_data_1 = [
         0x59, 0x02, 0xFF, 0x0B,    # OTP Bank 0 
         0x00, 0x00, 0x01, 0x04]    # OTP Bank 2

#    decode_otp_manufacture(real_data_1) # OTP Bank 0

    """
    Unit #2:    0x2F002144:  0x5902EF10       
                0x2F002148:  0x00000104

    Decoded by ATE s:
           x-loc: 16
           y-loc: 239
           wafer ID: 2
           lot ID: UASY04001 
    """
    real_data_2 = [
         0x59, 0x02, 0xEF, 0x10,    # OTP Bank 0 
         0x00, 0x00, 0x01, 0x04]    # OTP Bank 2
#    decode_otp_manufacture(real_data_2) # OTP Bank 0

    """
     Unit #3:   0x2F002144:  0x5902FE01       
                0x2F002148:  0x00000104

    Decoded by ATE as:
           x-loc: 1
           y-loc: 254
           wafer ID: 2
           lot ID: UASY04001 

    """
    real_data_3 = [
         0x59, 0x02, 0xFE, 0x01,    # OTP Bank 0 
         0x00, 0x00, 0x01, 0x04]    # OTP Bank 2
#    decode_otp_manufacture(real_data_3) # OTP Bank 0

    """
    Unit #4:    0x2F002144:  0x5902ED12
                0x2F002148:  0x00000104

    Decoded by ATE as:
           x-loc: 18
           y-loc: 237
           wafer ID: 2
           lot ID: UASY04001 
    """
    real_data_4 = [
         0x12, 0xED, 0x02, 0x59,
         0x00, 0x00, 0x04, 0x01]    # OTP Bank 2
#         0x59, 0x02, 0xED, 0x12,    # OTP Bank 0 
#         0x00, 0x00, 0x01, 0x04]    # OTP Bank 2

    decode_otp_manufacture(real_data_4) # OTP Bank 0
    print("")

    """
    Real data form device
    Unit #x:      0x2F002144:  0x59026897
                  0x2F002148:  0x00000104
    As read from ISP
     MfgData        =  0x10459026897
        ['0x97', '0x68', '0x2', '0x59', 
         '0x4', '0x1', '0x0', '0x0', 
     Endianess
    """
    real_data_5 = [
         0x97, 0x68, 0x2, 0x59,
         0x0,  0x0,  0x4,  0x1]
    decode_otp_manufacture(real_data_5) # OTP Bank 0

    print("")

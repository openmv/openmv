#!/usr/bin/env python3
"""
    device_probe.py
    discovery class for finding out what is on the other end of the SE-UART
"""
import struct
from isp_core import isp_start
from isp_core import isp_stop
from isp_core import isp_build_packet
from isp_protocol import ISP_COMMAND_DATA_RESPONSE
from isp_protocol import ISP_COMMAND_ENQUIRY
from isp_protocol import ISP_COMMAND_GET
from isp_protocol import ISP_PACKET_DATA_FIELD
from isp_protocol import ISP_PACKET_COMMAND_FIELD
from isp_protocol import ISP_SOURCE_SEROM
from isp_protocol import ISP_SOURCE_SERAM
from isp_protocol import ISP_GET_REVISION

# Part# & Rev definitions
PART_UNKNOWN = 0
REVISIONS = {
             '0 0 0 0'   : 'UNKNOWN',
             '0 161 0 0' : 'A1',
             '0 176 0 0' : 'B0',
             '0 178 0 0' : 'B2',
             '0 179 0 0' : 'B3',
             '0 180 0 0' : 'B4',
             '1 160 0 0' : 'A0',
             '2 160 0 0' : 'EG'  # super hack...
            }

# bootloader stages
STAGE_UNKOWN    = 0
STAGE_SEROM     = 1
STAGE_SERAM     = 2
STAGE_TEXT      = ['UNKNOWN','SEROM','SERAM']

class device_get_attributes:
    """ device_get_attributes
    """
    # attributes from the device
    part_number = 'PART_UNKNOWN'
    revision = 'UNKNOWN'
    stage = STAGE_UNKOWN
    env = 'UNKNOWN'     # DEV or PROD environment

    def __init__(self, isp):
        """
            ctor
        """
        self.isp = isp
        # retrieve device Part#

        # retrieve device revision
        self.revision, self.part_number, self.env = self.__get_device_info(isp)
        # probe and set device bootloader stage
        self.stage = self.__get_device_stage(isp)

    def __get_device_info(self, isp):
        """
            obtain the SoC Device information
        """
        isp_start(isp)
        rev = 'ERROR'
        message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_REVISION])
        cmd = message[ISP_PACKET_COMMAND_FIELD]
        if len(message) == 0:
            print("[ERROR] Target did not respond")
            return rev

        if cmd == ISP_COMMAND_DATA_RESPONSE:
            # silicon revision
            lst = message[2:6]
            rev1 = ' '.join(map(str,lst))
            (version,) = struct.unpack("<I",bytes(message[2:6]))
            try:
                rev = REVISIONS[rev1]
            except:
                print("[ERROR] Unknown device revision 0x%04X" % (version))

            # device Part#
            lst = message[6:21]     # char 16 in Part# is TBD (reserved for future if needed)
            ascii_list = [chr(ch) for ch in lst]
            part_number = ''.join(ascii_list)

            # HBK0
            lst = (message[22:38])
            env = ''
            for ch in lst:
                l = hex(ch)[2:]
                if len(l) == 1:
                    l = '0' + l
                env += l
            env = env.lower()

        isp_stop(isp)

        return rev, part_number, env


    def __get_device_stage(self,isp):
        """
            Return target source mode from Enquiry
            - Find out the device bootloader stage
        """
        probe_result = STAGE_UNKOWN

        isp_start(isp)
        message = isp_build_packet(isp, ISP_COMMAND_ENQUIRY)

        if len(message) == 0:
            print("[ERROR] Target did not respond")
            return probe_result

        cmd = message[ISP_PACKET_COMMAND_FIELD]

        if cmd == ISP_COMMAND_DATA_RESPONSE:
            state  = message[ISP_PACKET_DATA_FIELD]
            if state & ISP_SOURCE_SEROM:
                probe_result = STAGE_SEROM
            if state & ISP_SOURCE_SERAM:
                probe_result = STAGE_SERAM
        isp_stop(isp)

        return probe_result

#!/usr/bin/python3
"""
    ISP Core commands implementation

    In System Programming (ISP) protocol implementation


   __author__ onyettr
"""
# pylint: disable=unused-argument, invalid-name
import os
import signal
import time
import sys
import struct
from isp_protocol import *
from isp_print import isp_print_color, isp_print_response, isp_print_message
from serom_errors import *
from toc_decode import display_toc_info, toc_decode_toc_info
from toc_decode import TOC_IMAGE_CPU_A32_0, TOC_IMAGE_CPU_A32_1
from toc_decode import TOC_IMAGE_CPU_M55_HP, TOC_IMAGE_CPU_M55_HE, TOC_IMAGE_CPU_MODEM
from toc_decode import TOC_IMAGE_CPU_GNSS, TOC_IMAGE_CPU_DSP
from version_decode import version_decode
from cpu_decode import display_cpu_info
from power_decode import display_power_info
from clock_decode import display_clock_info
from otp import display_otp_info
#from serom_trace import *

#from seram_trace import trace_buffer_decode
from trace_decode import trace_buffer_decode

# ISP Command set
# Command : Value
isp_command_lookup = {
            ISP_COMMAND_START_ISP           : "COMMAND_START_ISP     ",
            ISP_COMMAND_STOP_ISP            : "COMMAND_STOP_ISP      ",
            ISP_COMMAND_ABORT               : "COMMAND_ABORT         ",
            ISP_COMMAND_DOWNLOAD_DATA       : "COMMAND_DOWNLOAD_DATA ",
            ISP_COMMAND_DOWNLOAD_DONE       : "COMMAND_DOWNLOAD_DONE ",
            ISP_COMMAND_BURN_MRAM           : "COMMAND_BURN_MRAM     ",
            ISP_COMMAND_RESET_DEVICE        : "COMMAND_RESET_DEVICE  ",
            ISP_COMMAND_PRINT_DATA          : "COMMAND_PRINT_DATA    ",
            ISP_COMMAND_MRAM_WRITE          : "COMMAND_MRAM_WRITE    ",
            ISP_COMMAND_SECURE_DEBUG        : "COMMAND_SECURE_DEBUG  ",
            ISP_COMMAND_ENQUIRY             : "COMMAND_ENQUIRY       ",
            ISP_COMMAND_GET_BAUD_RATE       : "COMMAND_GET_BAUD_RATE ",
            ISP_COMMAND_SET_BAUD_RATE       : "COMMAND_SET_BAUD_RATE ",
            ISP_COMMAND_VERIFY_IMAGE        : "COMMAND_VERIFY_IMAGE  ",
            ISP_COMMAND_GET_TOC_INFO        : "COMMAND_GET_TOC_INFO  ",
            ISP_COMMAND_OTP_WRITE           : "COMMAND_OTP_WRITE     ",
            ISP_COMMAND_OTP_READ            : "COMMAND_OTP_READ      ",
            ISP_COMMAND_GET                 : "COMMAND_GET           ",
            ISP_COMMAND_ERASE_MRAM          : "COMMAND_ERASE_MRAM    ",
            ISP_COMMAND_SET_MAINTENANCE_FLAG: "COMMAND_SET_MAINTENANCE_FLAG",
            ISP_COMMAND_MRAM_READ           : "COMMAND_MRAM_READ     ",
            ISP_COMMAND_SET                 : "COMMAND_SET           ",
            ISP_COMMAND_DMPU                : "COMMAND_DMPU          ",
            ISP_COMMAND_SRAM_WRITE          : "COMMAND SRAM WRITE    ",
            ISP_COMMAND_VERIFY_AND_EXECUTE  : "ISP_COMMAND_VERIFY_AND_EXECUTE",
            ISP_COMMAND_DATA_RESPONSE       : "COMMAND_DATA_RESPONSE ",
            ISP_COMMAND_ACK                 : "COMMAND_ACK           ",
            ISP_COMMAND_NAK                 : "COMMAND_NAK           "
}

# ISP Sub commands
isp_subcommand_lookup = {
            ISP_GET_TOC_INFO                : "GET_TOC_INFO          ",
            ISP_GET_REVISION                : "GET_REVISION          ",
            ISP_GET_BAUDRATE                : "GET_BAUDRATE          ",
            ISP_GET_BANNER                  : "GET_BANNER            ",
            ISP_GET_MRAM_CONTENTS           : "GET_MRAM_CONTENTS     ",
            ISP_GET_CPU_STATUS              : "GET_CPU_STATUS        ",
            ISP_GET_SERAM_METRICS           : "GET_SERAM_METRICS     ",
            ISP_GET_LOG_BUFFER_INFO         : "GET_LOG_BUFFER_INFO   ",
            ISP_GET_LOG_DATA                : "GET_LOG_DATA          ",
            ISP_GET_SERAM_UPDT_ADDR         : "GET_STREAM_UPDT_ADDR  ",
            ISP_GET_TRACE_BUFFER            : "GET_TRACE_BUFFER      ",
            ISP_GET_SERAM_TRACE_BUFFER      : "GET_SERAM_TRACE_BUFFER",
            ISP_GET_SECURE_DEBUG_TOKENS     : "GET_SECURE_DEBUG_TOKENS",
            ISP_GET_POWER_INFO              : "GET_POWER_INFO        ",
            ISP_GET_PEEK_ADDRESS            : "GET ADDRESS           ",
            ISP_GET_CLOCK_INFO              : "GET CLOCK_INFO        ",
            ISP_GET_ECC_KEY                 : "GET ECC_KEY           ",
            ISP_GET_FIREWALL_CONFIG         : "GET FIREWALL_CONFIG   ",
            ISP_SET_PRINTING_ENABLE         : "SET PRINTING ENABLE   ",
            ISP_SET_PRINTING_DISABLE        : "SET PRINTING DISABLE  ",
            ISP_SET_LOGGING_ENABLE          : "SET LOGGING ENABLE    ",
            ISP_SET_LOGGING_DISABLE         : "SET LOGGING DISABLE   ",
            ISP_SET_POKE_ADDRESS            : "SET ADDRESS           "
}

error_lookup = {
            ISP_SUCCESS                     : "ISP_SUCCESS           ",
            ISP_ERROR                       : "ISP_ERROR             ",
            ISP_READ_TIMEOUT                : "ISP_READ_TIMEOUT      ",
            ISP_WRITE_TIMEOUT               : "ISP_WRITE_TIMEOUT     ",
            ISP_UNKNOWN_COMMAND             : "ISP_UNKNOWN_COMMAND   ",
            ISP_COMMAND_STOP                : "ISP_COMMAND_STOP      ",
            ISP_CHECKSUM_ERROR              : "ISP_CHECKSUM_ERROR    ",
            ISP_NOT_SUPPORTED               : "ISP_NOT_SUPPORTED     ",
            ISP_BAD_PACKET_SIZE             : "ISP_BAD_PACKET_SIZE   ",
            ISP_BAD_DESTINATION_ADDR        : "ISP_BAD_DEST_ADDRESS  ",
            ISP_UNEXPECTED_COMMAND          : "ISP_UNEXPECTED_COMMAND",
            ISP_HOST_AUTH_FAILED            : "ISP_HOST_AUTH_FAILED  ",
            ISP_NOT_ALLOWED                 : "ISP_NOT_ALLOWED       ",
            ISP_BAD_AUTH_TOKEN              : "ISP_BAD_AUTH_TOKEN    ",
            ISP_IMAGE_AUTHENTICATION_FAILED : "ISP_IMAGE_AUTHENTICATION_FAILED",
            ISP_IMAGE_VERIFICATION_FAILED   : "ISP_IMAGE_VERIFICATION_FAILED",
            ISP_SECURE_DEBUG_FAILED         : "ISP_SECURE_DEBUG_FAILED",
            ISP_MRAM_WRITE_FAILED           : "ISP_MRAM_WRITE_FAILED ",
            ISP_UNKNOWN_GET_OBJECT          : "ISP_UNKNOWN_GET_OBJECT",
            ISP_ERROR_ILLEGAL_ADDRESS       : "ISP_ERROR_ILLEGAL_ADDRESS",
            ISP_ERROR_MISALIGNED            : "ISP_ERROR_MISALIGNED",
            ISP_UNKNOWN_MRAM_ERROR          : "ISP_UNKNOWN_MRAM_ERROR",
            ISP_MRAM_LENGTH_TOO_LARGE       : "ISP_MRAM_LENGTH_TOO_LARGE",
            ISP_UNKNOWN_SET_OBJECT          : "ISP_UNKNOWN_SET_OBJECT",
            ISP_CODE_VERIFY_FAILURE         : "ISP_CODE_VERIFY_FAILURE",
            ISP_INVALID_CODE_SIZE           : "ISP_INVALID_CODE_SIZE",
            ISP_MFG_DATA_MISCOMPARE         : "ISP_MFG_DATA_MISCOMPARE",
            ISP_SERAM_JUMP_FAILED           : "ISP_SERAM_JUMP_FAILED",
            ISP_MRAM_CONTROLLER_ERROR       : "ISP_MRAM_CONTROLLER_ERROR"
}

cpu_name_lut = {
            TOC_IMAGE_CPU_A32_0             : "A32_0 ",
            TOC_IMAGE_CPU_A32_1             : "A32_1 ",
            TOC_IMAGE_CPU_M55_HP            : "M55-HP",
            TOC_IMAGE_CPU_M55_HE            : "M55-HE",
            TOC_IMAGE_CPU_MODEM             : "Modem ",
            TOC_IMAGE_CPU_GNSS              : "GNSS  ",
            TOC_IMAGE_CPU_DSP               : "DSP   "
}

serom_error_lookup = {
            SEROM_STATUS_SUCCESS                     : "SEROM_STATUS_SUCCESS",
            SEROM_BSV_INIT_FAIL                      : "SEROM_BSV_INIT_FAIL",
            SEROM_BSV_LCS_GET_AND_INIT_FAIL          : "SEROM_BSV_LCS_GET_AND_INIT_FAIL",
            SEROM_BSV_LCS_GET_FAIL                   : "SEROM_BSV_LCS_GET_FAIL",
            SEROM_BSV_SEC_MODE_SET_FAIL              : "SEROM_BSV_SEC_MODE_SET_FAIL",
            SEROM_BSV_PRIV_MODE_SET_FAIL             : "SEROM_BSV_PRIV_MODE_SET_FAIL",
            SEROM_BSV_CORE_CLK_GATING_ENABLE_FAIL    : "SEROM_BSV_CORE_CLK_GATING_ENABLE_FAIL",
            SEROM_MRAM_INITIALIZATION_FAILURE        : "SEROM_MRAM_INITIALIZATION_FAILURE",
            SEROM_MRAM_INITIALIZATION_TIMEOUT        : "SEROM_MRAM_INITIALIZATION_TIMEOUT",
            SEROM_MRAM_WRITE_FAILURE                 : "SEROM_MRAM_WRITE_FAILURE",
            SEROM_ATOC_EXT_HDR_OFFSET_ZERO           : "SEROM_ATOC_EXT_HDR_OFFSET_ZERO",
            SEROM_ATOC_EXT_HDR_OFFSET_TOO_LARGE      : "SEROM_ATOC_EXT_HDR_OFFSET_TOO_LARGE",
            SEROM_ATOC_OBJECT_OFFSET_ZERO            : "SEROM_ATOC_OBJECT_OFFSET_ZERO",
            SEROM_ATOC_OBJECT_OFFSET_MISALIGNED      : "SEROM_ATOC_OBJECT_OFFSET_MISALIGNED",
            SEROM_ATOC_OBJECT_OFFSET_TOO_LARGE       : "SEROM_ATOC_OBJECT_OFFSET_TOO_LARGE",
            SEROM_ATOC_OBJECT_OFFSET_TOO_SMALL       : "SEROM_ATOC_OBJECT_OFFSET_TOO_SMALL",
            SEROM_ATOC_EXT_HDR_OFFSET_MISALIGNED     : "SEROM_ATOC_EXT_HDR_OFFSET_MISALIGNED",
            SEROM_ATOC_HEADER_OFFSET_INVALID         : "SEROM_ATOC_HEADER_OFFSET_INVALID",
            SEROM_ATOC_HEADER_CRC32_ERROR            : "SEROM_ATOC_HEADER_CRC32_ERROR",
            SEROM_ATOC_HEADER_STRING_INVALID         : "SEROM_ATOC_HEADER_STRING_INVALID",
            SEROM_ATOC_NUM_TOC_ENTRIES_INVALID       : "SEROM_ATOC_NUM_TOC_ENTRIES_INVALID",
            SEROM_CONTENT_CERTIFICATE_NULL           : "SEROM_CONTENT_CERTIFICATE_NULL",
            SEROM_CERTIFICATE_NULL                   : "SEROM_CERTIFICATE_NULL",
            SEROM_CERTIFICATE_CHAIN_INVALID          : "SEROM_CERTIFICATE_CHAIN_INVALID",
            SEROM_INVALID_OEM_ROT                    : "SEROM_INVALID_OEM_ROT",
            SEROM_CERTIFICATE_ERROR_BASE             : "SEROM_CERTIFICATE_ERROR_BASE",
            SEROM_CERTIFICATE_1_ERROR                : "SEROM_CERTIFICATE_1_ERROR",
            SEROM_CERTIFICATE_2_ERROR                : "SEROM_CERTIFICATE_2_ERROR",
            SEROM_CERTIFICATE_3_ERROR                : "SEROM_CERTIFICATE_3_ERROR",
            SEROM_BOOT_CODE_LOAD_ADDR_INVALID        : "SEROM_BOOT_CODE_LOAD_ADDR_INVALID",
            SEROM_BOOT_VERIFY_IN_MEMORY_CASE_INVALID : "SEROM_BOOT_VERIFY_IN_MEMORY_CASE_INVALID",
            SEROM_BOOT_ZERO_IMAGE_LENGTH_INVALID     : "SEROM_BOOT_ZERO_IMAGE_LENGTH_INVALID",
            SEROM_BOOT_ENCRYPTED_IMAGE_INVALID       : "SEROM_BOOT_ENCRYPTED_IMAGE_INVALID",
            SEROM_BOOT_VERIFY_IN_FLASH_CASE_INVALID  : "SEROM_BOOT_VERIFY_IN_FLASH_CASE_INVALID",
            SEROM_BOOT_IMAGE_LENGTH_TOO_LARGE        : "SEROM_BOOT_IMAGE_LENGTH_TOO_LARGE",
            SEROM_BOOT_RAW_IMAGE_LOADING_NOT_ALLOWED : "SEROM_BOOT_RAW_IMAGE_LOADING_NOT_ALLOWED",
            SEROM_BOOT_SERAM_JUMP_RETURN_ERROR       : "SEROM_BOOT_SERAM_JUMP_RETURN_ERROR",
            SEROM_BOOT_FAILED                        : "SEROM_BOOT_FAILED",
            SEROM_BOOT_JUMP_ADDRESS_NOT_VALID        : "SEROM_BOOT_JUMP_ADDRESS_NOT_VALID",
            SEROM_BOTH_BANKS_INVALID                 : "SEROM_BOTH_BANKS_INVALID",
            SEROM_ATOC_EXT_HDR_OFFSET_TOO_SMALL      : "SEROM_ATOC_EXT_HDR_OFFSET_TOO_SMALL",
            SEROM_BOOT_END_OF_MAIN_ERROR             : "SEROM_BOOT_END_OF_MAIN_ERROR",
            SEROM_INVALID_NULL_PTR                   : "SEROM_INVALID_NULL_PTR",
            SEROM_INVALID_TOC_OFFSET                 : "SEROM_INVALID_TOC_OFFSET",
}

def convert_from_ms(milliseconds):
    """
        from the web
    """
    seconds, milliseconds = divmod(milliseconds,1000)
    minutes, seconds = divmod(seconds,60)
    hours, minutes = divmod(minutes, 60)
    days, hours = divmod(hours, 24)

#    days = 0
#    seconds = (milliseconds/1000)%60
#    seconds = int(seconds)
#    minutes = (milliseconds/(1000*60))%60
#    minutes = int(minutes)
#    hours = (milliseconds/(1000*60*60))%24
#    hours = int(hours)
    return days, hours, minutes, seconds

class CtrlCHandler():
    """
        CtrlCHandler - handler for Ctrl-C key press
    """
    def __init__(self):
        """
            ctor
        """
        self.running = False
        signal.signal(signal.SIGINT, self.Handler)

    def Handler(self, signal_number, frame):
        """
            Handle the interupt signal
        """
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        self.running = True

    def Handler_reset(self):
        """
            start over
        """
        self.running = False

    def Handler_exit(self):
        """
            return handler state
        """
        return self.running

def int_to_bytes(number):
    """
        int_to_bytes
        - integer number gets turned into bytes
    """
    _hrepr = hex(number).replace('0x', '')
    return number.to_bytes(4,byteorder='little')

def isp_decode_packet(isp, prompt_str, packet):
    """
       DecodePacket

       prints details of header and packet
    """

    command = ISP_UNKNOWN_COMMAND

    if isp.getVerbose() is True:
        print("%s" %prompt_str, end='')
        print(" length= %3d" %len(packet), end='')

    # Decode further
    if len(packet) > ISP_PACKET_HEADER_LENGTH:
        lastbyte = len(packet)
        command = packet[ISP_PACKET_COMMAND_FIELD]
# Uncomment for more details
#        hexized = [hex(x) for x in packet]
#        print(hexized)

        cmdstr = isp_command_lookup.get(command)
        if not cmdstr:
            cmdstr =   ">> COMMAND_UNKNOWN << "
            command = ISP_UNKNOWN_COMMAND
        if isp.getVerbose() is True:
            print(" cmd    = %3x" %command, end='')
            print(" command=", cmdstr, end='')
            if (command == ISP_COMMAND_GET or command == ISP_COMMAND_SET):
                sub_command = int.from_bytes(packet[2:6], "little")
                sub_command_str = isp_subcommand_lookup.get(sub_command)
                if not sub_command_str:
                    sub_command_str = ">> SUB COMMAND UNKNOWN <<"
                print(" sub command= %4x %s"
                      %(sub_command, sub_command_str),end='')
                if sub_command == ISP_GET_PEEK_ADDRESS:
                    address = int.from_bytes(packet[6:10], "little")
                    print("address= 0x%8x" %(address))
                if sub_command == ISP_SET_POKE_ADDRESS:
                    address = int.from_bytes(packet[6:10], "little")
                    data = int.from_bytes(packet[10:14], "little")
                    print("address= 0x%x, data= 0x%x" %(address,data))
            print(" chksum =", hex(packet[lastbyte-1]),end='')

#            if command != ISP_COMMAND_DOWNLOAD_DATA:
#                print(" packet=", packet,end='')

        if command == ISP_COMMAND_NAK:  # NACK, check error code#
# Uncomment for more details
#            hexized = [hex(x) for x in packet]
#            print(hexized)
            errstr = error_lookup.get(packet[ISP_PACKET_DATA_FIELD])

            if not errstr:
                errstr = ">> ERROR UNKNOWN (" + hex(packet[ISP_PACKET_DATA_FIELD]) + ") <<"
            if isp.getVerbose() is False:
                print("%s" %prompt_str, end='')
                print(" length= %3d" %len(packet), end='')
                print(" command=", cmdstr, end='')
                print(" chksum=", hex(packet[lastbyte-1]),end='')
                print(" error=", errstr)
            else:
                print(" error=", errstr, end='')

            # print the extended error code, if it exist
            payload_begin = ISP_PACKET_DATA_FIELD + 1   # skip the NACK error code
            payload_end = len(packet) - 1               # skip the checksum

            payload = packet[payload_begin:payload_end]
            if len(payload) > 0:
                hexized = [hex(x) for x in payload]
                print(" extended error=", hexized)

        if command == ISP_COMMAND_DATA_RESPONSE:
            payload = packet[ISP_PACKET_DATA_FIELD:len(packet)-1]
            if isp.getVerbose() is True:
                if packet[ISP_TOC_COMMAND_FIELD] == ISP_TOC_INFO_START_MARKER:
                    print(" ISP_TOC_INFO_START_MARKER ", end='')
                if packet[ISP_TOC_COMMAND_FIELD] == ISP_TOC_INFO_END_MARKER:
                    print(" ISP_TOC_INFO_STOP_MARKER ", end='')
                if packet[ISP_TOC_COMMAND_FIELD] == ISP_DATA_START_MARKER:
                    print(" ISP_DATA_START_MARKER ", end='')
                if packet[ISP_TOC_COMMAND_FIELD] == ISP_DATA_END_MARKER:
                    print(" ISP_DATA_END_MARKER   ", end='')

#                for each_word in range(0,160,4):
#                    trace_value = payload[each_word:each_word+4]
#                    value = int.from_bytes(trace_value,byteorder='little')
#                    print("{:4d} 0x{:08x} ".format(each_word,value))

                hexed = [hex(x) for x in payload]
                print(" response payload=", hexed, end='')
                print(hexed)

    if isp.getVerbose() is True:
        print(" ")
        sys.stdout.flush()

    return command

def isp_readmessage(isp):
    """
        read a message from the serial port

        Length; Command; [<payload>]; Checksum
    """

    packet_header = isp.readSerial(ISP_PACKET_HEADER_LENGTH)
    if len(packet_header) != ISP_PACKET_HEADER_LENGTH:
        return []

    packet_length = packet_header[ISP_PACKET_LENGTH_FIELD]
    if packet_length < ISP_PACKET_HEADER_LENGTH + 1:
        return []

# Uncomment to test NAK packages with payload (also indent the lines after 'else:' so that Python doesn't complain
    #if packet_header[ISP_PACKET_COMMAND_FIELD] == ISP_COMMAND_NAK:
    #    packet_contents = list(bytearray(isp.readSerial(packet_length - 3)))
    #    packet_contents += [0x1, 0x2, 0x3, 0x4]
    #    packet_contents += list(bytearray(isp.readSerial(1)))
    #else:
    # read the rest of the packet (including the checksum)
    packet_contents = list(bytearray(isp.readSerial(packet_length - ISP_PACKET_HEADER_LENGTH)))
    if packet_contents == []:
        return []

    packet = packet_header + packet_contents
#    print("message = ", packet)

    return packet

def isp_wait(isp):
    """
        wait for trigger to proceed
    """
    _event = isp.eventFlag.wait(1)    # Wait for operation to complete
    isp.eventFlag.clear()

def isp_signal(isp):
    """
        tigeger signal to wait
    """
    isp.eventFlag.set()    # tell TX we are done


def isp_build_packet(isp, isp_command, isp_sub_commands = None, delay=0):
    """
        build the packet and send, receive and decode response
    """
    if isp_sub_commands is None:
        isp_sub_commands = []

    cmd_packet = [0x00, isp_command]

    for sub in isp_sub_commands:
        if sub == 'padding':
            cmd_packet.append(0x55)
            continue

        if isinstance( sub, int ):
            cmd_packet = cmd_packet + list(int_to_bytes(sub))
        else:
            cmd_packet = cmd_packet + list(sub)

    cmd_packet[0] = len(cmd_packet) + 1
    cmd_packet = isp.checkSum(cmd_packet)  # add the checksum packet

    isp.writeSerial(bytearray(cmd_packet))
    isp_decode_packet(isp,"TX--> ", cmd_packet)
    if delay>0:
        time.sleep(delay)
    message = isp_readmessage(isp)
    command = isp_decode_packet(isp,"RX<-- ", message)

    while command == ISP_COMMAND_PRINT_DATA:
        isp_print_message('blue', message)

        message = isp_readmessage(isp)
        command = isp_decode_packet(isp,"RX<-- ", message)

    return message

def isp_test_target(baud_rate, isp):
    """
        isp_test_target

        This command is similar to isp_start() but it does NOT exit
        when the target did not respond...Int rather resturns:
         1 - when target responded
        -1 - when target did not respond
    """
    cmd_packet = [0x03, ISP_COMMAND_START_ISP]    # start isp
    cmd_packet = isp.checkSum(cmd_packet)  # add the checksum packet

    isp.writeSerial(bytearray(cmd_packet))
    isp_decode_packet(isp,"TX--> ", cmd_packet)

    # ISP START always succeeds, so there is no need to read the Ack like for the other commands.
    # Two things to look for are -
    # 1. ISP_COMMAND_ACK byte - to verify that the target responded
    # 2. Any leftover data, e.g., Flicker animations - it needs to be read/flushed
    got_ack = False
    while True:
        data = isp.readSerial(1)
        if len(data) == 0:
            break
        if data[0] == ISP_COMMAND_ACK:
            got_ack = True
    if not got_ack:
        sys.stdout.flush()
        return -1

    if isp.getVerbose() is True:
        print("RX<--                           command= COMMAND_ACK")

    if got_ack == True:
        return 1

def isp_start(isp):
    """
        isp_start

        ISP_COMMAND_START puts the SERAM isp_handler into an ISP mode. Flicker
        will be disabled, however there maybe some residual characters which
        get consumed.
    """
    cmd_packet = [0x03, ISP_COMMAND_START_ISP]    # start isp
    cmd_packet = isp.checkSum(cmd_packet)  # add the checksum packet

    isp.writeSerial(bytearray(cmd_packet))
    isp_decode_packet(isp,"TX--> ", cmd_packet)

    # ISP START always succeeds, so there is no need to read the Ack like for the other commands.
    # Two things to look for are -
    # 1. ISP_COMMAND_ACK byte - to verify that the target responded
    # 2. Any leftover data, e.g., Flicker animations - it needs to be read/flushed
    got_ack = False
    while True:
        data = isp.readSerial(1)
        if len(data) == 0:
            break
        if data[0] == ISP_COMMAND_ACK:
            got_ack = True
    if not got_ack:
        print("[ERROR] Target did not respond")
        sys.stdout.flush()
        # isp_start is called before any dynamic baud rate change, so it is ok to sys.exit()
        isp.closeSerial()
        sys.exit(-1)

    if isp.getVerbose() is True:
        print("RX<--                           command= COMMAND_ACK")

def isp_stop(isp):
    """
        isp_Stop
        - Send Command to Stop the isp handler on the target
    """
    isp_build_packet(isp, ISP_COMMAND_STOP_ISP)

def getFilesize(fileHandle):
    """
        getFileSize
        - given a file handle return the size in bytes
    """
    fileHandle.seek(0,os.SEEK_END)
    sizeinbytes = fileHandle.tell()

    return sizeinbytes

def isp_reset(isp):
    """
        isp_reset
    """
    isp_build_packet(isp, ISP_COMMAND_RESET_DEVICE)

def isp_secure_debug(isp, auth_token, data):
    """
        isp_secure_debug
    """
    subCmd = []
    if auth_token != 0x0:
        subCmd.append(auth_token)

    if len(data) != 0:
        subCmd.append(data)

    #print("*** sub command: ", subCmd)
    isp_build_packet(isp, ISP_COMMAND_SECURE_DEBUG, subCmd)

def isp_download_start(isp, address, size, auth_token):
    """
        isp_download_start
    """
    subCmd = [address,size]
    if auth_token != 0x0:
        subCmd.append(auth_token)
    isp_build_packet(isp, ISP_COMMAND_DOWNLOAD_START, subCmd)

def isp_download_data(isp, data):
    """
        isp_download_data
    """
    message = isp_build_packet(isp, ISP_COMMAND_DOWNLOAD_DATA,
                               ['padding', 'padding', bytearray(data)])
    if len(message) == 0:
        print("[ERROR] No response")
        return False

    command = message[ISP_PACKET_COMMAND_FIELD]
    if command == ISP_COMMAND_NAK:
        print("[ERROR] Command NAK")
        return False

    return True

def isp_download_done(isp, delay = 0):
    """
        isp_download_done
        Does not wait for reader task
    """
    isp_build_packet(isp, ISP_COMMAND_DOWNLOAD_DONE, None, delay)

def isp_burn_mram(isp, address, file_size, auth_token):
    """
        isp_burn_mram
    """
    subCmd = [address,file_size]
    if auth_token != 0x0:
        subCmd.append(auth_token)
    message = isp_build_packet(isp, ISP_COMMAND_BURN_MRAM, subCmd)

    # same error handling as in isp_download_data
    if len(message) == 0:
        print("[ERROR] No response")
        return False

    command = message[ISP_PACKET_COMMAND_FIELD]
    if command == ISP_COMMAND_NAK:
        print("[ERROR] Command NAK")
        return False

    return True


def isp_enquiry(isp):
    """
        isp_enquiry
    """
    message = isp_build_packet(isp, ISP_COMMAND_ENQUIRY)

    if len(message) == 0:
        return

    cmd = message[ISP_PACKET_COMMAND_FIELD]
    if cmd == ISP_COMMAND_DATA_RESPONSE:
        #isp_print_response("blue",message)
        Maintenance_string = ""
        error_string = ""
        length = message[ISP_PACKET_LENGTH_FIELD]
        state  = message[ISP_PACKET_DATA_FIELD]

        ErrorCode = (message[3:7])  # bytes 3 through 6 (limit 7)
        ExtendedErrorCode = [0,0,0,0]
        ExtendedErrorCode = (message[7:11])  # bytes 7 through 10 (limit 11)
        if state & ISP_SOURCE_SEROM:
            isp_print_color("blue","SEROM")
            error_string = serom_error_lookup.get(int.from_bytes(ErrorCode,"little"))
            Maintenance_string = "<None>"
        if state & ISP_SOURCE_SERAM:
            isp_print_color("blue","SERAM")
            MaintenanceMode = int.from_bytes(message[11:15],"little")
            Maintenance_string = "Enabled" if (MaintenanceMode == 1) else "Disabled "

        isp_print_color("blue"," Error =  ")
        isp_print_color("blue",hex(int.from_bytes(ErrorCode,"little")))
        if error_string:
            isp_print_color("blue"," (" + error_string + ") ")
        isp_print_color("blue"," Extended Error =  ")
        isp_print_color("blue",hex(int.from_bytes(ExtendedErrorCode,"little")))
        isp_print_color("blue"," Maintenance Mode = " + Maintenance_string)
        print(" ")


def isp_show_maintenance_mode(isp, maintenance_mode):
    """
        isp_show_maintenance_mode
        - disply the maintence mode status
    """
    Maintenance_string = "Enabled" if (maintenance_mode == 1) else "Disabled "
    isp_print_color("blue","Maintenance Mode = " + Maintenance_string)
    print(" ")

def isp_get_maintenance_status(isp):
    """
        isp_get_maintenance_status
        - This is wrapper around isp_enquiry which returns the error code
          data as well, this is ignored.
    """
    MaintenanceMode = 0
    message = isp_build_packet(isp, ISP_COMMAND_ENQUIRY)
    if len(message) != 0:
        cmd = message[ISP_PACKET_COMMAND_FIELD]

        if cmd == ISP_COMMAND_DATA_RESPONSE:
#           isp_print_response("blue",message)
            length = message[ISP_PACKET_LENGTH_FIELD]
            state  = message[ISP_PACKET_DATA_FIELD]
            if state & ISP_SOURCE_SERAM:
                MaintenanceMode = int.from_bytes(message[11:15],"little")

    return MaintenanceMode

def isp_get_revision(isp):
    # we will add the source (SEROM or SERAM)

    """
        isp_enquiry
    """
    message = isp_build_packet(isp, ISP_COMMAND_ENQUIRY)
    if len(message) == 0:
        return
    cmd = message[ISP_PACKET_COMMAND_FIELD]
    if cmd == ISP_COMMAND_DATA_RESPONSE:
        state  = message[ISP_PACKET_DATA_FIELD]
        isp_print_color("blue"," Source\t\t=  ")
        if state & ISP_SOURCE_SEROM:
            isp_print_color("blue","SEROM\n")
        if state & ISP_SOURCE_SERAM:
            isp_print_color("blue","SERAM\n")



    """
        isp_get_revision handler
    """
    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_REVISION])

    if len(message) == 0:
        return
    cmd = message[ISP_PACKET_COMMAND_FIELD]

    if cmd == ISP_COMMAND_DATA_RESPONSE:
        version_decode(isp,message)

def isp_get_baud_rate(isp):
    """
        isp_get_baud_rate
    """
    baud_rate = 0
    message = isp_build_packet(isp, ISP_COMMAND_GET_BAUD_RATE)
    baud_rate = int.from_bytes(message[2:6],"little")
    isp_print_color("blue",baud_rate)
    print(" ")

    return baud_rate

def isp_get_toc_data(isp):
    """
        isp_get_toc_data
    """
    isp_print_color("blue",
    " +----------+--------+------------+------------+------------+------------+----------+-----------+--------+----------+\n")
    isp_print_color("blue",
    " |   Name   |  CPU   | Store Addr |  Obj Addr  | Dest Addr  | Boot Addr  |   Size   |  Version  |  Flags | Time (ms)|\n")
    isp_print_color("blue",
    " +----------+--------+------------+------------+------------+------------+----------+-----------+--------+----------+\n")

    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_TOC_INFO])

    if len(message) == 0:
        return

    # This uses a "stream" of packets sent back from SERAM
    while (message[ISP_PACKET_COMMAND_FIELD] == ISP_COMMAND_DATA_RESPONSE and
           message[ISP_DATA_START_FIELD] == ISP_DATA_START_MARKER):

        display_toc_info(message[3::]) # Skip header, payload only

        message = isp_readmessage(isp)
        command = isp_decode_packet(isp,"RX<-- ", message)

        if len(message) == 0:
            return
    isp_print_color("blue",
        " +----------+--------+------------+------------+------------+------------+----------+-----------+--------+----------+\n")

def isp_get_power_data(isp):
    """
        isp_get_power_data
    """

    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_POWER_INFO])

    if len(message) == 0:
        return
    command = message[ISP_PACKET_COMMAND_FIELD]
    if command == ISP_COMMAND_NAK:
        return

    display_power_info(message[2::]) # Skip header, payload only
#    display_power_info(message) # Skip header, payload only

def isp_get_clock_data(isp):
    """
        isp_get_power_data
    """

    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_CLOCK_INFO])

    if len(message) == 0:
        return
    command = message[ISP_PACKET_COMMAND_FIELD]
    if command == ISP_COMMAND_NAK:
        return

    display_clock_info(message[2::]) # Skip header, payload only

def isp_get_banner(isp):
    """
        isp_get_banner - retrieve seram banner
            sub-command ISP_GET_BANNER
    """
    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_BANNER])

    if len(message) == 0:
        return
    command = message[ISP_PACKET_COMMAND_FIELD]
    if command == ISP_COMMAND_DATA_RESPONSE:
        isp_print_message("blue", message)
        print(" ")

def isp_get_mram_contents(isp):
    """
        isp_get_mram_contents - retrieve MRAM details
            sub-command ISP_GET_MRAM_CONTENTS
    """
    delay = 10
    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_MRAM_CONTENTS], delay)

    if len(message) == 0:
        return

    while (message[ISP_PACKET_COMMAND_FIELD] == ISP_COMMAND_DATA_RESPONSE and
           message[ISP_TOC_COMMAND_FIELD] == ISP_TOC_INFO_START_MARKER):
        toc_decode_toc_info(isp, message)

        message = isp_readmessage(isp)
        isp_decode_packet(isp,"RX<-- ", message)

        if len(message) == 0:
            return

def isp_get_cpu_status(isp):
    """
        isp_get_cpu_state

        ISP_DATA_RESPONSE packet for return data

         CPU_ID   Booted  Boot Addess
        <1-byte> <1-byte>  <4-bytes>   = 6-bytes
    """
    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_CPU_STATUS])

    if len(message) == 0:      # Nothing came back
        return
    command = message[ISP_PACKET_COMMAND_FIELD]
    if command == ISP_COMMAND_DATA_RESPONSE:
        display_cpu_info(message)

def isp_get_seram_metrics(isp):
    """
        isp_get_seram_metrics

        ISP_DATA_RESPONSE packet for return data

         Task Name  Size       Used
        <16-bytes> <4-byte>  <4-bytes>   = 24-bytes
    """
    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_SERAM_METRICS])

    if len(message) == 0:      # Nothing came back
        return

    if message[ISP_PACKET_COMMAND_FIELD] == ISP_COMMAND_DATA_RESPONSE:
        isp_print_color("blue", " +-------------------------------+\n")
        isp_print_color("blue", " |   TaskName      | Size | Used |\n")
        isp_print_color("blue", " +-----------------+------+------+\n")

        # unpack the data in the response. 2 CPUS x 6-bytes of data
        # @todo magic numbers need to go away and less cut and paste code
        packet_start = ISP_PACKET_DATA_FIELD
        element = message[packet_start::]

        # uint32_t  number of tasks
        # uint32_t  uptime
        # struct {
        #    uint32_t stack_size
        #    uint32_t stack_used
        #    char task_name[16]
        #} [number_of_tasks]
        (number_of_tasks,) = struct.unpack("<I",bytes(element[0:4]))
        (uptime,) = struct.unpack("<I",bytes(element[4:8]))

        packet_start = 8  # skip over first 2 words
        for each_task in range(number_of_tasks):
            stack_size, stack_used, seram_task_name = \
               struct.unpack("<II16s", bytes(element[packet_start:packet_start+24]))
            stack_left = (stack_size*4) - (stack_used*4)
            name_str = ''.join([chr(e) for e in seram_task_name])

            print_string = \
              " | {:16s} | {:4d} | {:4d} |\n".format(str(name_str),
                                              stack_size*4,
                                              stack_left)
            isp_print_color("blue", print_string)
            packet_start = packet_start + 24
        isp_print_color("blue", " +-----------------+------+------+\n")

        days, hours, minutes, seconds = convert_from_ms(uptime)
        isp_print_color("blue",
         " SES uptime {}:{:02}:{:02}:{:02}".format(days,hours,minutes,seconds))
        print(" ")

def isp_get_trace_buffer(isp):
    """
        isp_get_trace_buffer
            SEROM pull back the trace buffer and processes through the decoder
    """
    delay = 0
    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_TRACE_BUFFER], delay)

    if len(message) == 0:
        return

    # This uses a "stream" of packets sent back from SERAM
    trace_buffer = message[3:7]
    while (message[ISP_PACKET_COMMAND_FIELD] == ISP_COMMAND_DATA_RESPONSE and
           message[ISP_DATA_START_FIELD] == ISP_DATA_START_MARKER):
#
        message = isp_readmessage(isp)
        command = isp_decode_packet(isp,"RX<-- ", message)

        if len(message) == 0:
            return
        trace_buffer += message[3:7]

    for each_word in range(0,516,4):
        trace_value = trace_buffer[each_word:each_word+4]
        value = int.from_bytes(trace_value,byteorder='little')
# DEBUG lives here
#        print("{:4d} 0x{:08x} ".format(each_word,value))
    trace_buffer_decode(trace_buffer, 0)

def isp_get_seram_trace_buffer(isp):
    """
        isp_get_seram_trace_buffer
            SERAM pull back the trace buffer and processes through the decoder
    """
    delay = 0
    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_SERAM_TRACE_BUFFER], delay)

    if len(message) == 0:
        return

    # This uses a "stream" of packets sent back from SERAM
    trace_buffer = message[3:7]
    while (message[ISP_PACKET_COMMAND_FIELD] == ISP_COMMAND_DATA_RESPONSE and
           message[ISP_DATA_START_FIELD] == ISP_DATA_START_MARKER):
#
        message = isp_readmessage(isp)
        command = isp_decode_packet(isp,"RX<-- ", message)

        if len(message) == 0:
            return
        trace_buffer += message[3:7]

    for each_word in range(0,516,4):
        trace_value = trace_buffer[each_word:each_word+4]
        value = int.from_bytes(trace_value,byteorder='little')
# DEBUG lives here
#        print("{:4d} 0x{:08x} ".format(each_word,value))
    trace_buffer_decode(trace_buffer, 1)

def isp_get_address(isp, address):
    """
        isp_get_address
    """
    delay = 0
    message = isp_build_packet(isp, ISP_COMMAND_GET,
                               [ISP_GET_PEEK_ADDRESS,address],
                               delay)
    if len(message) != 0:
        cmd = message[ISP_PACKET_COMMAND_FIELD]

        if cmd == ISP_COMMAND_DATA_RESPONSE:
            length = message[ISP_PACKET_LENGTH_FIELD]
            value  = int.from_bytes(message[ISP_PACKET_DATA_FIELD:6],"little")
            print(hex(value))

def isp_set_address(isp, address, data):
    """
        isp_set_address
    """
    delay = 0
    message = isp_build_packet(isp, ISP_COMMAND_SET,
                               [ISP_SET_POKE_ADDRESS,address,data],
                               delay)
    if len(message) != 0:
        cmd = message[ISP_PACKET_COMMAND_FIELD]

        if cmd == ISP_COMMAND_DATA_RESPONSE:
            length = message[ISP_PACKET_LENGTH_FIELD]
            value  = int.from_bytes(message[ISP_PACKET_DATA_FIELD:6],"little")
            print(hex(value))

def isp_set_log_enable(isp, enable):
    """
        SET capability - LOGGER enable/disable
    """
#    delay = 0
    if enable:
        response = isp_build_packet(isp, ISP_COMMAND_SET, [ISP_SET_LOGGING_ENABLE])
    else:
        response = isp_build_packet(isp, ISP_COMMAND_SET, [ISP_SET_LOGGING_DISABLE])

    if len(response) == 0:
        return

    command = response[ISP_PACKET_COMMAND_FIELD]
    if command == ISP_COMMAND_NAK:
        error_string = error_lookup.get(response[ISP_PACKET_DATA_FIELD])
        if not error_string:
            error_string = "UNKNOWN"
        isp_print_color("blue","[ERROR] {}".format(error_string))
        print(" ")
        return  # Exit this function

    return

def isp_set_print_enable(isp, enable):
    """
        SET capability - PRINT enable/disable
    """
    delay = 0
    if enable:
        response = isp_build_packet(isp, ISP_COMMAND_SET, [ISP_SET_PRINTING_ENABLE])
    else:
        response = isp_build_packet(isp, ISP_COMMAND_SET, [ISP_SET_PRINTING_DISABLE])

    if len(response) == 0:
        return

    command = response[ISP_PACKET_COMMAND_FIELD]
    if command == ISP_COMMAND_NAK:
        error_string = error_lookup.get(response[ISP_PACKET_DATA_FIELD])
        if not error_string:
            error_string = "UNKNOWN"
        isp_print_color("blue","[ERROR] {}".format(error_string))
        print(" ")
        return  # Exit this function

    return

def isp_get(isp,getobject):
    """
        isp_get
            get an "object"
    """
    isp_build_packet(isp, ISP_COMMAND_GET)

def isp_set(isp,getobject):
    """
        isp_set
            set an attribute "object"
    """
    isp_build_packet(isp, ISP_COMMAND_SET)

def isp_read_otp(isp,word_offset):
    """
        isp_read_otp
            retrieve otp @ word_offset
    """
    response = isp_build_packet(isp, ISP_COMMAND_OTP_READ, [int(word_offset)])
    if len(response) == 0:
        return

    command = response[ISP_PACKET_COMMAND_FIELD]
    if command == ISP_COMMAND_NAK:
        error_string = error_lookup.get(response[ISP_PACKET_DATA_FIELD])
        if not error_string:
            error_string = "UNKNOWN"
        otp_value = int.from_bytes(response[2:6],"little")
        isp_print_color("blue","[ERROR] 0x{:04x} {}".format(
            word_offset, error_string))
        print(" ")

        return  # Exit this function

    if command != ISP_COMMAND_DATA_RESPONSE:
        isp_print_color("blue","[ERROR] No response")
        return None  # Exit this function

    # Display the OTP data
    display_otp_info(word_offset, response)

def isp_set_baud_rate(isp, baud_rate):
    """
        isp_get_baud_rate
    """
    isp_build_packet(isp, ISP_COMMAND_SET_BAUD_RATE, [baud_rate])

def isp_set_host_baud_rate(isp, baud_rate):
    """
        set the host baud rate
    """
    isp.setBaudRate(baud_rate)

def isp_mram_write(isp, offset, data):
    """
        isp_Mram_Write
        - send data to write to the MRAM - SEROM style
        (used by icv-recovery.py and recovery.py)
    """
    isp_build_packet(isp, ISP_COMMAND_MRAM_WRITE, [offset, bytearray(data)])

def isp_read_mram(isp,word_offset=0x0):
    """
        isp_read_mram
            retrieve mram @ word_offset
    """
    response = isp_build_packet(isp, ISP_COMMAND_MRAM_READ, [int(word_offset),b'\x10'])
    if len(response) == 0:
        return

    command = response[ISP_PACKET_COMMAND_FIELD]
    if command == ISP_COMMAND_NAK:
        error_string = error_lookup.get(response[ISP_PACKET_DATA_FIELD])
        if not error_string:
            error_string = "UNKNOWN"
        isp_print_color("blue","[ERROR] 0x{:04x} {}".format(
                         word_offset, error_string))
        print(" ")
    if command != ISP_COMMAND_DATA_RESPONSE:
        return None

    # write out the 4 int32 values of the MRAM line
    isp_print_color("blue","[0x{:x}] ".format(word_offset))
    for each_word in range(2,18,4):
        mram_value = response[each_word:each_word+4]
        value = int.from_bytes(mram_value,byteorder='little')
        isp_print_color("blue","0x{:08x} ".format(value))
    print(" ")

def isp_mram_erase(isp, address, size=0,pattern=0x00000000):
    """
        isp_erase_mram
        - erase ATOC memory from <address> for <size> with <pattern>
    """
    delay = 0
    if size >= 1048576:    # ACK will take some time at this size
        delay = 20
    isp_build_packet(isp, ISP_COMMAND_ERASE_MRAM, [address,size,pattern], delay)

def isp_sram_write(isp, offset, data):
    """
        isp_Sram_Write
        - send data to write to the SE RAM - SEROM only
        (used by recovery-execute-package.py)
    """

    #print("* write to offset ", hex(offset))
    #print("* data: ", data)
    isp_build_packet(isp, ISP_COMMAND_SRAM_WRITE, [offset, bytearray(data)])

def isp_set_maintenance_flag(isp):
    """
        isp_set_maintenance_flag
        - Send Command to set the maintenance flag on the target
    """
    message = isp_build_packet(isp, ISP_COMMAND_SET_MAINTENANCE_FLAG)

def isp_get_log_data(isp):
    """
        isp_get_log_data
    """

    # get log buffer info
    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_LOG_BUFFER_INFO])
    if len(message) == 0:
        return None

    command = message[ISP_PACKET_COMMAND_FIELD]
    if command != ISP_COMMAND_DATA_RESPONSE:
        return None

    (buffer_size,) = struct.unpack("<I",bytes(message[2:6]))
    (head_offset,) = struct.unpack("<I",bytes(message[6:10]))
    (tail_offset,) = struct.unpack("<I",bytes(message[10:14]))

    #print('buffer_size: ', buffer_size)
    #print('head_offset: ', head_offset)
    #print('tail_offset: ', tail_offset)

    # get log data
    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_LOG_DATA])
    if len(message) == 0:
        return None

    log_data = []
    while message[ISP_PACKET_COMMAND_FIELD] == ISP_COMMAND_DATA_RESPONSE:
        # process message
        data = message[ISP_PACKET_DATA_FIELD:len(message)-1]
        #print("data: ", data)

        log_data += data

        message = isp_readmessage(isp)
        command = isp_decode_packet(isp,"RX<-- ", message)
        if len(message) == 0:
            break

    #print("log_data: ", log_data)
    return (buffer_size, head_offset, tail_offset, log_data)

def isp_get_seram_update_address(isp):
    """
        isp_get_seram_update_address
    """
    address = 0

    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_SERAM_UPDT_ADDR])
    _expected_length = ISP_PACKET_HEADER_LENGTH + 4 + 1
    if len(message) == ISP_PACKET_HEADER_LENGTH:
        return address

    command = message[ISP_PACKET_COMMAND_FIELD]
    if command == ISP_COMMAND_DATA_RESPONSE:
        address = int.from_bytes(message[2:6],"little")

    return address

def isp_get_ecc_key(isp):
    """
        isp_get_ecc_key
    """
    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_ECC_KEY])
    if len(message) == 0:
        return

    command = message[ISP_PACKET_COMMAND_FIELD]
    if command != ISP_COMMAND_DATA_RESPONSE:
        print("ECC query error")
        return

    ecc_key = message[2:66]
    print("ECC key (HEX): ", "".join("{:02X}".format(n) for n in ecc_key))


def isp_get_firewall_config(isp):
    """
        isp_get_firewall_config
    """
    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_FIREWALL_CONFIG])
    if len(message) == 0:
        return

    command = message[ISP_PACKET_COMMAND_FIELD]
    if command != ISP_COMMAND_DATA_RESPONSE:
        print("Firewall configuration query error")
        return

    firewall_config = message[2:202]
    print("Firewall configuration:")
    for idx in range(0, 200, 2):
        if 0 == firewall_config[idx]:
            continue
        data = int.from_bytes(firewall_config[idx:idx+2],"little")
        print("FC: " + str(data & 0xF) + " region: " + str((data & 0xFF0) >> 4))

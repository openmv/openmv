#!/usr/bin/python3

"""
   @brief Table Of Contents 

   __author__ onyettr
"""
# pylint: disable=unused-argument, invalid-name
import struct
from isp_print import isp_print_color

# CPU Values
TOC_IMAGE_CPU_ID_MASK = 0x0000000F
TOC_IMAGE_CPU_A32_0   = 0
TOC_IMAGE_CPU_A32_1   = 1
TOC_IMAGE_CPU_M55_HP  = 2
TOC_IMAGE_CPU_M55_HE  = 3
TOC_IMAGE_CPU_MODEM   = 4
TOC_IMAGE_CPU_GNSS    = 5
TOC_IMAGE_CPU_DSP     = 6
TOC_IMAGE_CPU_RISC_V  = 7
TOC_IMAGE_CPU_SE      = 8
TOC_IMAGE_CPU_LAST    = 9

# TOC Flags
TOC_IMAGE_COMPRESSED  = 0x10
TOC_IMAGE_LOAD        = 0x20
TOC_IMAGE_BOOT        = 0x40
TOC_IMAGE_ENCRYPT     = 0x80
TOC_IMAGE_DEFERRED    = 0x100

FLAG_STRING_COMPRESSED = 0
FLAG_STRING_LOAD_IMAGE = 1
FLAG_STRING_VERIFY     = 2
FLAG_STRING_CPU_BOOTED = 3
FLAG_STRING_ENCRYPTED  = 4
FLAG_STRING_DEFERRED   = 5
FLAG_STRING_END        = 6
FLAG_STRING_SIZE       = 10

# TOC Types
STOC_TOC_HEADER       = 0
ATOC_TOC_HEADER       = 1
MINI_TOC_HEADER       = 2

TOC_RESPONSE_PACKET_SIZE = 54

cpu_name_lut = {
            TOC_IMAGE_CPU_A32_0             : "A32_0 ",
            TOC_IMAGE_CPU_A32_1             : "A32_1 ",
            TOC_IMAGE_CPU_M55_HP            : "M55-HP",
            TOC_IMAGE_CPU_M55_HE            : "M55-HE",
            TOC_IMAGE_CPU_MODEM             : "Modem ",
            TOC_IMAGE_CPU_GNSS              : "GNSS  ",
            TOC_IMAGE_CPU_DSP               : "DSP   ",
            TOC_IMAGE_CPU_RISC_V            : "RISC-V"
}

toctype_lookup = {
            STOC_TOC_HEADER                 : " STOC ",
            ATOC_TOC_HEADER                 : " ATOC ",
            MINI_TOC_HEADER                 : " MTOC "
}

def debug_output(debug_data):
    """
        debug_output 
            dumps the packet data byte by byte
    """
    print("".join('%c ' % i for i in debug_data[3:11]))

def flags_to_string(flags):
    """
        Convert TOC Flags to something you can read
    """
    flag_string = list('       ')

    if flags & TOC_IMAGE_COMPRESSED:
        flag_string[FLAG_STRING_COMPRESSED] = 'C'
    else:
        flag_string[FLAG_STRING_COMPRESSED] = 'u'
        
    if flags & TOC_IMAGE_LOAD:
        flag_string[FLAG_STRING_LOAD_IMAGE] = 'L'

    if flags & TOC_IMAGE_DEFERRED:
        flag_string[FLAG_STRING_DEFERRED] = 'D'

    return ''.join(str(x) for x in flag_string)

def digit_count(digits):
    """
        given an integer (digits) count the number of digits
        return this 
    """
    counter = 0
    while abs(digits) >= 10**counter:
        counter += 1

    return counter

def toc_version_to_string(version_id,width=8):
    """
        convert the TOC version into a string
    """
    major = (version_id >> 24) & 0xFF
    minor = (version_id >> 16) & 0xFF
    patch = (version_id >>  8) & 0xFF
    major_length = digit_count(major)

    version_string = "{:}.{:}.{:}".format(major,minor,patch).rjust(width,' ')

    return version_string.center(width,' ')

def format_flags(entry_name,flags,width=8):
    """
        format the flags 
    """
    search_words = "SERAM0 SERAM1"
    if entry_name.strip() in search_words:
        return "-------".center(width, ' ')
    return '{:}'.format(flags,width).ljust(width,' ')
 
def format_hex(hex_number, width=10):
    """
        format a hex_number into a 'width' string
    """
    if hex_number == 0x0:
        return "----------".center(width, ' ')
    return '{0:#0{1}x}'.format(hex_number,width)

def format_time(time_value, width=8):
    """
        format the process_time into a 'width' string
    """
    return '{:.2f}'.format(time_value).rjust(width,' ')

def format_cpu(cpu_value, entry_name='', width=7):
    """
        format the cpu into a 'width' string
        SERAM0/1 and DEVICE are special cases and 
    """
    search_words = "SERAM0 SERAM1 DEVICE"
    if (entry_name != '' and entry_name.strip() in search_words):
        cpu_name = "CM0+"
    elif (entry_name != '' and entry_name.strip() == "* SERAM0"):
        cpu_name = "CM0+"
    elif (entry_name != '' and entry_name.strip() == "* SERAM1"):
        cpu_name = "CM0+"
    else:
        cpu_name = cpu_name_lut.get(cpu_value)
        if cpu_name is None:
            cpu_name = "Unknown"

    return '{:}'.format(cpu_name).rjust(width,' ')

def format_name(entry_name,width=10):
    """
        format_name 
            strip off any NULL terminations
    """
    return entry_name.decode().rstrip('\x00').rjust(width,' ')

def toc_decode_toc_info(isp, message):
    """
        toc_decode_toc_info
            parse a toc header
    """
    (address,) = struct.unpack("<I",bytes(message[3:7]))
    (toctype,) = struct.unpack("<I",bytes(message[7:11]))
    header = message[11:19]

    # Dig deeper into the Header
    (header_size,) = struct.unpack("<H",bytes(message[19:21]))
    (num_toc,) = struct.unpack("<H",bytes(message[21:23]))
    (entry_size,) = struct.unpack("<H",bytes(message[23:25]))
    (version,) = struct.unpack("<H",bytes(message[25:27]))

    header_str = ''.join([chr(e) for e in header])
    isp_print_color('blue', toctype_lookup.get(toctype) + " " + 
                    hex(address) + '\n')
    if (header_size > 0) and (toctype != MINI_TOC_HEADER):
        isp_print_color('blue', "\t+ header        " + header_str       + '\n')
        isp_print_color('blue', "\t+ header_size   " + str(header_size) + '\n')
        isp_print_color('blue', "\t+ # toc entries " + str(num_toc)     + '\n')
        isp_print_color('blue', "\t+ entry_size    " + str(entry_size)  + '\n')
        isp_print_color('blue', "\t+ version       " + str(hex(version))+ '\n')

def display_toc_info(message):
    """
        display_toc_info
            display toc entries sent back from Target
    """
    (entry_name,cpu,store_address,object_address,load_address, 
     boot_address,image_size,version,process_time,flags) = \
        struct.unpack("<8sIIIIIIII7s",bytes(message[0:47]))

    flag_string = flags.decode()
    process_time_ms = (0.01 * process_time)/1000
    name_string = format_name(entry_name)
    cpu_string = format_cpu(cpu,name_string)

    isp_print_color("blue",
        " |%8s|%7s | %10s | 0x%08X | %10s | %10s | %8d |%11s|%6s| %s |\n" % (
         name_string,
         cpu_string,
         format_hex(store_address),
         object_address,
         format_hex(load_address),
         format_hex(boot_address),
         image_size,
         toc_version_to_string(version),
         format_flags(name_string,flag_string[:5]),
         format_time(process_time_ms)))

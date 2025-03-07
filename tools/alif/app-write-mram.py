#!/usr/bin/env python3
"""
    MRAM (NVM) burning / writing utility
    Support
        - APPLICATION TOC package writing
        - image write at any address in NVM (MRAM)
        - image erase of any | range of addresses in NVM (MRAM)

    __author__ = ""
    __copyright__ = "ALIF Seminconductor"
    __version__ = "0.16.0"
    __status__ = "Dev"    "
"""
import os
import sys
import signal
import argparse
from utils.discover import getValues, getJlinkSN
import utils.config
from utils.config import *
from utils.user_validations import validateArgList
sys.path.append("./isp")
from serialport import serialPort
from serialport import COM_BAUD_RATE_MAXIMUM
#import ispcommands
from isp_core import *
from isp_util import *
import device_probe
import pylink
import datetime
#from array import array

#  Version                  Feature
# 0.23.000     Added probing to detect device stage/Part#/Rev
# 0.22.000     Added support for MAC OS (J-Link not included yet)
# 0.21.000     get baudrate from DBs
# 0.20.007     Added padding option for binaries not multiple of 16
# 0.20.006     Added Python error exit code
# 0.20.005     Fixed J-Link+Pylink issue writing binaries
# 0.20.004     REV_A0 dynamic baud rate support
# 0.20.003     remove maintenance mode output unless -v is used)
# 0.20.002     Reset option set as default 
# 0.20.001     Added Erase option
# 0.20.000     Added option to skip user managed images
# 0.19.000     Removed JTAG access
# 0.16.000     Addition of baud rate increase for bulk transfer
# 0.15.000     Fixes for Block sizes and left overs
TOOL_VERSION ="0.23.000"        # Define Version constant for each separate tool

EXIT_WITH_ERROR = 1

def readImageList(dsFile):
    """
        readImageList = read images from arm-ds command file
    """
    try:
        f = open(dsFile, 'r')
        imageList = f.read()
        f.close()
        # search for image list
        start = imageList.find('args')
        end = imageList.find('continue')
        if start == -1 or end == -1:
            print('[ERROR] There is a problem with the Debug Script information')
            sys.exit(EXIT_WITH_ERROR)
        imageList = imageList[start + 5: end]
    except:
        print('[ERROR] opening Debug Script file',dsFile)
        sys.exit(EXIT_WITH_ERROR)

    return imageList    

def app_mram_erase(isp,args, ALIF_BASE_ADDRESS, ALIF_MRAM_SIZE):
    """
        app_mram_erase
        - erase the Application are of MRAM
        args 
        argv[0]    'erase'    Operation, can be ignored
        argv[1]    <address>  Starting address
        argv{2]    <size>     Length of bytes to erase
        argv[3]    <pattern>  Optional pattern to erase with
        argc                  Checked before this is called 
    """
    argv = args.split()
    argc = len(argv)

    # nombres magiques, maybe not needed?
    address = 0x80000000
    erase_len = 16
    pattern = 0x00000000

    if argc >= 3:
        address = int(argv[1],base=16)
        erase_len = int(argv[2],base=16)
        erase_len_fmt = "{:,}".format(erase_len)

        if argc == 4:
            pattern = int(argv[3],base=16)
        print("[INFO] %s 0x%x %d (%s)" %(argv[0],address,erase_len,
              erase_len_fmt))

        # Validate user request for erasing is in OEM
        if (address > ALIF_BASE_ADDRESS or
            address + erase_len > ALIF_BASE_ADDRESS):

            if address + erase_len > ALIF_BASE_ADDRESS:
                print("[ERROR] illegal address " + hex(address+erase_len) + 
                      " (" + hex(address) + " + " + hex(erase_len) + ")")
            else:
                print("[ERROR] illegal address " + hex(address))
            return

        isp_mram_erase(isp,address,erase_len,pattern)

def main():
    if sys.version_info.major == 2:
        print("[ERROR] You need Python 3 for this application!")
        sys.exit(EXIT_WITH_ERROR)

    exit_code = 0
    # Deal with Command Line
    parser = argparse.ArgumentParser(description=
                                     'NVM Burner for Application TOC Package')
    parser.add_argument("-d" , "--discover", action='store_true', \
                        default=False, help="COM port discovery")
    parser.add_argument("-b", "--baudrate", help="serial port baud rate",
                        type=int)
    parser.add_argument("-e", "--erase", type=str,
                        help='ERASE [APP | <start address> <size> [<pattern>] ]')
    # creating a mutually exclusive group for -i IMAGES and -S 
    # (skip option doesn't make sense in a user provided list...)
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-i", "--images", type=str,
                        default="Application TOC Package",
                        help='images list to burn into NVM \
                        ("/path/image1.bin 0x80001000 /path/image2.bin 0x80003000")')
    parser.add_argument("-a", "--auth_image", action='store_true', 
                        help="authenticate the image by sending its signature file", 
                        default=False)
    parser.add_argument("-m", "--method", type=str,
                       help="loading method [JTAG | ISP]")                        
    group.add_argument("-S", "--skip", 
                        help='write ATOC only - skip user managed images',
                        action="store_true")                        
    parser.add_argument("-s", "--switch", 
                        help="dynamic baud rate switch toggle, default=on",
                        action="store_false")
    parser.add_argument("-p", "--pad",  
                        help="pad the binary if size is not multiple of 16", action="store_true")                        
    parser.add_argument("-nr", "--no_reset", default=False, 
                        help="do not reset target before operation", action="store_true")                        
    parser.add_argument("-V" , "--version",
                        help="Display Version Number", action="store_true")
    parser.add_argument("-v" , "--verbose",
                        help="verbosity mode", action="store_true")

    args = parser.parse_args()
    if args.version:
        print(TOOL_VERSION)
        sys.exit()

    # memory defines for Alif/OEM MRAM Addresses and Sizes
    load_global_config()
    DEVICE_PART_NUMBER = utils.config.DEVICE_PART_NUMBER
    DEVICE_REVISION = utils.config.DEVICE_REVISION
    DEVICE_REV_BAUD_RATE = utils.config.DEVICE_REV_BAUD_RATE
    MRAM_BASE_ADDRESS = utils.config.MRAM_BASE_ADDRESS
    ALIF_BASE_ADDRESS = utils.config.ALIF_BASE_ADDRESS
    OEM_BASE_ADDRESS = utils.config.APP_BASE_ADDRESS
    MRAM_SIZE = utils.config.MRAM_SIZE
    ALIF_MRAM_SIZE = utils.config.ALIF_MRAM_SIZE
    OEM_MRAM_SIZE = utils.config.APP_MRAM_SIZE
    MRAM_BURN_INTERFACE = utils.config.MRAM_BURN_INTERFACE
    JTAG_ADAPTER = utils.config.JTAG_ADAPTER

    os.system('')                   # Help MS-DOS window with ESC sequences
    
    print('Writing MRAM with parameters:')
    print('Device Part# ' + DEVICE_PART_NUMBER + ' - Rev: ' + DEVICE_REVISION)   
    print('- Available MRAM: ' + str(OEM_MRAM_SIZE) + ' bytes')

#    baud_rate = args.baudrate
#    if DEVICE_REVISION == 'A0':
#        """
#            For REV_A0 we need to change the default used by ALL other devices
#        """
#        default_baud_rate = COM_BAUD_RATE_REV_A0
#        if baud_rate == COM_BAUD_RATE_DEFAULT:
#            baud_rate = default_baud_rate
#    else:
#        default_baud_rate = COM_BAUD_RATE_DEFAULT
#        baud_rate = args.baudrate

#    if baud_rate != default_baud_rate:
#        dynamic_baud_rate_switch = False
#    else:
#        dynamic_baud_rate_switch = args.switch

    baud_rate = DEVICE_REV_BAUD_RATE[DEVICE_REVISION]
    if args.baudrate is not None:
        baud_rate = args.baudrate

    dynamic_baud_rate_switch = args.switch

    argList = ''
    action = 'Burning: '
    method = None

    if args.method == None:
        method = MRAM_BURN_INTERFACE
    elif args.method.upper() == 'ISP':
        method = 'isp'
    elif args.method.upper() == 'JTAG':
        method = 'jtag'
    else:
        print("[ERROR] Unknown connection method", args.method)
        sys.exit(EXIT_WITH_ERROR)

    if args.erase:
        action = 'Erasing: '
        argList = 'erase '
        if args.erase.upper() == 'APP':
            argList += hex(OEM_BASE_ADDRESS) + ' ' + hex(OEM_MRAM_SIZE)
        else:
            if args.erase.strip() == '':
                print("[ERROR] erase arguments are empty")
                sys.exit(EXIT_WITH_ERROR)
            argList += args.erase
    elif args.images != "Application TOC Package":
        argList = args.images
    else:
        dsFile = 'bin/application_package.ds'
        argList = readImageList(dsFile)

    if args.skip:
        idx = argList.find('../build/AppTocPackage.bin 0x')
        argList = argList[idx: idx + 37]

    # validate all parameters
    argList = validateArgList(action, argList.strip(), args.pad)

    if sys.platform == "linux" or sys.platform == "darwin":
        argList = argList.replace('../','')

    print("[INFO]", action + argList)

    
    if method == 'jtag' and not args.erase:     # erase via jtag not yet supported!
        jlinkSN = getJlinkSN()
        if jlinkSN == -1:
            print('J-Link device not found!')
            sys.exit(EXIT_WITH_ERROR)
    
        print('J-Link SN: ' + jlinkSN)  

        jlink = pylink.JLink()
        jlink.open(serial_no=int(jlinkSN))
        print(jlink.product_name)
        if not jlink.opened():
            print('Error opening J-Link')
            sys.exit(EXIT_WITH_ERROR)

        if not jlink.connected():
            print('Error connecting J-Link')
            sys.exit(EXIT_WITH_ERROR)

        print("about to connect to M0+..")
        jlink.connect("CORTEX-M0+", verbose=True)
        #jlink.connect('AC302F8A82562_HE', verbose=True)
        if not jlink.target_connected():
            print('Error connecting target')
            print('Please, power-cycle the board')
            sys.exit(EXIT_WITH_ERROR)

        argParams = argList.strip().split(" ")
        lenParams = len(argParams)
        i = 0
        while i < lenParams:
            f = argParams[i][3:]
            if sys.platform == "linux":
                f = argParams[i][:]

            print('File name: ' + f)
            file_size = os.path.getsize(f)
            print("File Size is :", file_size, "bytes")
            print('MRAM starting address: ' + argParams[i+1])
            addr = globalToLocalAddress(argParams[i+1])
            print("Writing the file to MRAM...")
            # read the file
            f = open(f,"rb")
            #fileData = array("I")
            #fileData.fromfile(f, int(file_size/4))
            fileData = list(f.read(file_size))
            f.close()
    
            # write the data to MRAM
            t1 = datetime.datetime.now()    
            #jlink.memory_write32(int(addr, 16), fileData)
            jlink.memory_write8(int(addr, 16), fileData)
            t2 = datetime.datetime.now()
            print('Done in ' + str(t2-t1))
            i += 2

        #jlink.reset()  # the reset causes an issue and the board will need to power cycle...

        return # end jlink tests

    if method == 'jtag' and args.erase:     # erase via jtag not yet supported!
        print('[INFO] Erase is only supported via ISP')

    dynamic_string = "Enabled" if dynamic_baud_rate_switch else "Disabled"
    print("[INFO] baud rate ", baud_rate)
    print("[INFO] dynamic baud rate change ", dynamic_string)

    handler = CtrlCHandler()
    isp = serialPort(baud_rate)           # Serial dabbling open up port.

    if args.discover:            # discover the COM ports if requested
        isp.discoverSerialPorts()

    errorCode = isp.openSerial()
    if errorCode is False:
        print("[ERROR] isp openSerial failed for %s" %isp.getPort())
        sys.exit(EXIT_WITH_ERROR)
    
    print("[INFO] %s open Serial port success" %isp.getPort())

    isp.setBaudRate(baud_rate)
    isp.setVerbose(args.verbose)

    # be sure device is not in SEROM Recovery Mode
    device = device_probe.device_get_attributes(isp)    
    if device.stage != device_probe.STAGE_SERAM:
        print('[ERROR] The device is in RECOVERY MODE! Please use Recovery option in Maintenance Tool to recover the device!')
        sys.exit(EXIT_WITH_ERROR)

    print('[INFO] Detected Device:')
    partDetected = device.part_number
    print('Part# ' + partDetected + ' - Rev: ' + device.revision)

    partDescription = getPartDescription(partDetected)

    if partDescription != DEVICE_PART_NUMBER:
        print('[WARN] ************ Part# detected is different than the one configured in tools-config tool!')

    if device.revision != DEVICE_REVISION:
        print('[WARN] ************ Part Revision detected is different than the one configured in tools-config tool!')


    if not args.no_reset:
        put_target_in_maintenance_mode(isp, baud_rate, args.verbose)        

    if sys.platform == "linux" or sys.platform == "darwin":
        argList = argList.replace('\\','/')
    else:
        argList = argList.replace('/','\\')
    
    items = argList.split(' ')

    isp_start(isp)              # Start ISP Sequence

    if args.erase:
        app_mram_erase(isp,argList,ALIF_BASE_ADDRESS, ALIF_MRAM_SIZE) 
    else:
        if dynamic_baud_rate_switch:
            isp_set_baud_rate(isp,COM_BAUD_RATE_MAXIMUM) # Jack up Baud rate
            isp.setBaudRate(COM_BAUD_RATE_MAXIMUM)         # Sets the HOST baud rate

        # issue enquiry command to check if SERAM is in Maintenance Mode
        mode = isp_get_maintenance_status(isp)
        isp_show_maintenance_mode(isp, mode)

        for e in range(1, len(items),2):
            addr = items[e]
            address = int(addr,base=16)
            fileName = items[e-1]
            fileName = fileName.replace('..\\','')

            if burn_mram_isp(isp, handler, fileName, address,
                             args.verbose, args.auth_image) == False:
                
                exit_code = EXIT_WITH_ERROR
                break

        # Restore the default Baud rate
        if dynamic_baud_rate_switch:
            isp_set_baud_rate(isp,baud_rate)
            isp.setBaudRate(baud_rate) 

    isp_stop(isp)              # Stop ISP Sequence
    isp_reset(isp)

    isp.closeSerial()
    sys.exit(exit_code)

if __name__ == "__main__":
    main()

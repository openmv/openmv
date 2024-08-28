#!/usr/bin/env python3
"""
    updateSystemPackage.py
    
"""
# TODO:
# check for online updates? Download and apply?

import os
import sys
import signal
import argparse
sys.path.append("./isp")
from serialport import serialPort
from serialport import COM_BAUD_RATE_MAXIMUM
#import ispcommands
from isp_core import *
from isp_util import *

import utils.config
from utils.config import *
import device_probe

# Define Version constant for each separate tool
#  Version                  Feature
# 0.16.000     Addition of baud rate increase for bulk transfer
# 0.20.000     Removed JTAG access
# 0.21.000     Reset option set as default
# 0.21.001     Suppress maintenance mode output
# 0.21.002     Added python error exit code
# 0.22.000     Added support for multi-packages update (Alif's packages for different Part#/Revs)
#              also, removed A0 related-code
# 0.23.000     Added probe to detect the target and abort if Revision mismatches the selection
# 0.24.000     get baudrate from DBs
# 0.25.000     Added Part# and Revision Device's detection and offer to switch as defaults
# 0.26.000     Added support for MAC OS
TOOL_VERSION ="0.26.000"

EXIT_WITH_ERROR = 1

def checkTargetWithSelection(targetDescription, targetRevision):
    partIsDifferent = False
    if targetDescription != DEVICE_PART_NUMBER:
        print('Connected target is not the default Part#')
        partIsDifferent = True

    if targetRevision != DEVICE_REVISION:
        print('Connected target is not the default Revision')
        partIsDifferent = True

    if partIsDifferent:
        try:
            answer = input('Do you want to set this part as default? (y/n): ')
        except EOFError as e:
            print('\nUser aborted the process')
            sys.exit()

        if answer.lower() == 'y' or answer.lower() == 'yes':
            save_global_config(targetDescription, targetRevision)


def main():
    global DEVICE_PART_NUMBER
    global DEVICE_REVISION

    if sys.version_info.major == 2:
        print("[ERROR] You need Python 3 for this application!")
        sys.exit(EXIT_WITH_ERROR)

    parser = argparse.ArgumentParser(description='Update System Package in MRAM')
    parser.add_argument("-d" , "--discover", action='store_true', \
                        default=False, help="COM port discovery for ISP")
    parser.add_argument("-b", "--baudrate", help="(isp) serial port baud rate",
                        type=int)
    parser.add_argument("-s", "--switch",
                        help="(isp) dynamic baud rate switch toggle, default=on",
                        action="store_false")
    parser.add_argument("-nr", "--no_reset", default=False,
                        help="do not reset target before operation", action="store_true")
    parser.add_argument("-na", "--no_authentication", action='store_true',
                        help="run in non-authenticated mode",
                        default=False)
    parser.add_argument("-V" , "--version",
                        help="Display Version Number", action="store_true")
    parser.add_argument("-v" , "--verbose",
                        help="verbosity mode", action="store_true")
    args = parser.parse_args()
    if args.version:
        print(TOOL_VERSION)
        sys.exit()

    # retrieve initial params based on user selection (toold-config)
    load_global_config()
    DEVICE_PART_NUMBER = utils.config.DEVICE_PART_NUMBER
    DEVICE_REVISION = utils.config.DEVICE_REVISION
    DEVICE_REV_BAUD_RATE = utils.config.DEVICE_REV_BAUD_RATE
    HASHES_DB = utils.config.HASHES_DB

    os.system('')                   # Help MS-DOS window with ESC sequences

    print('Burning: System Package in MRAM')

    print('Selected Device:')
    print('Part# ' + DEVICE_PART_NUMBER + ' - Rev: ' + DEVICE_REVISION)


    baud_rate = DEVICE_REV_BAUD_RATE[DEVICE_REVISION]
    if args.baudrate is not None:
        baud_rate = args.baudrate

    dynamic_baud_rate_switch = args.switch

    print('\nConnecting to the target device...')

    dynamic_string = "Enabled" if dynamic_baud_rate_switch else "Disabled"
    print("[INFO] baud rate ", baud_rate)
    print("[INFO] dynamic baud rate change ", dynamic_string)

    handler = CtrlCHandler()     # handle ctrl-c key press
    isp = serialPort(baud_rate)  # Serial dabbling open up port.

    if args.discover:            # discover the COM ports if requested
        isp.discoverSerialPorts()

    errorCode = isp.openSerial()
    if errorCode is False:
        print("[ERROR] isp openSerial failed for %s" %isp.getPort())
        sys.exit(EXIT_WITH_ERROR)
    print("[INFO] %s open Serial port success" %isp.getPort())

    isp.setBaudRate(baud_rate)
    isp.setVerbose(args.verbose)

    # probe the device before update
    device = device_probe.device_get_attributes(isp)

    # check SERAM is the bootloader stage
    print('Bootloader stage: ' + device_probe.STAGE_TEXT[device.stage])
    if device.stage != device_probe.STAGE_SERAM:
        print('[ERROR] Please use Recovery option from ROM menu in Maintenance Tool')
        sys.exit(EXIT_WITH_ERROR)

    if not args.no_reset:
        put_target_in_maintenance_mode(isp, baud_rate, args.verbose)

    print('[INFO] Detected Device:')
    partDetected = device.part_number

    # Check for Blank devices
    if bytes(partDetected, 'utf-8') == b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00':
        if device.revision == 'A0':      # SPARK Device default
            partDetected = 'AB1C1F4M51920PH'
        else:                               # ENSEMBLE Device default
            partDetected = 'AE722F80F55D5LS'
        print('[WARN] No Part# was detected! Defaulting to ' + partDetected)

    print('Part# ' + partDetected + ' - Rev: ' + device.revision)

    partDescription = getPartDescription(partDetected)

    # load configuration from detected device
    load_device_config(partDescription, device.revision)

    # retrieve rest of params from the detected device
    DEVICE_PACKAGE = utils.config.DEVICE_PACKAGE
    DEVICE_REV_PACKAGE_EXT = utils.config.DEVICE_REV_PACKAGE_EXT
    DEVICE_OFFSET  = utils.config.DEVICE_OFFSET
    MRAM_BASE_ADDRESS = utils.config.MRAM_BASE_ADDRESS
    ALIF_BASE_ADDRESS = utils.config.ALIF_BASE_ADDRESS
    MRAM_SIZE = utils.config.MRAM_SIZE

    print('- MRAM Base Address: ' + hex(ALIF_BASE_ADDRESS))

    # check the default Part#/Rev in tools-config and offer to switch
    checkTargetWithSelection(partDescription, device.revision)

    env_ext = ''
    if device.env.lower() in HASHES_DB:
        if HASHES_DB[device.env] == 'DEV':
            env_ext = '-dev'

    # for devices in CM LCS, we use the DEV package
    if device.env == '00000000000000000000000000000000':
        print('[WARN] Device is not provisioned!')
        env_ext = '-dev'

    rev_ext = DEVICE_REV_PACKAGE_EXT[device.revision]

    alif_image = 'alif/' + DEVICE_PACKAGE + '-' + rev_ext + env_ext + '.bin'
    alif_offset = 'alif/' + DEVICE_OFFSET + '-' + rev_ext + env_ext + '.bin'

    if sys.platform == "linux" or sys.platform == "darwin":
        imageList = alif_image + ' ' + hex(ALIF_BASE_ADDRESS) + \
            ' ' + alif_offset + ' ' + hex(MRAM_BASE_ADDRESS + MRAM_SIZE - 16)

    else:
        imageList = '../' + alif_image + ' ' + hex(ALIF_BASE_ADDRESS) + \
            ' ../' + alif_offset + ' ' + hex(MRAM_BASE_ADDRESS + MRAM_SIZE - 16)

    # check images exist...
    if not os.path.exists(alif_image):
        print('Image ' + alif_image + ' does not exist!')
        sys.exit(EXIT_WITH_ERROR)

    if not os.path.exists(alif_offset):
        print('Image ' + alif_offset + ' does not exist!')
        sys.exit(EXIT_WITH_ERROR)

    isp_start(isp)                         # Start ISP Sequence

    if sys.platform == "linux" or sys.platform == "darwin":
        imageList = imageList.replace('\\','/')
    else:
        imageList = imageList.replace('/','\\')

    if dynamic_baud_rate_switch:
        isp_set_baud_rate(isp,COM_BAUD_RATE_MAXIMUM) # Jack up Baud rate
        isp.setBaudRate(COM_BAUD_RATE_MAXIMUM)         # Sets the HOST baud rate

    # issue enquiry command to check if SERAM is in Maintenance Mode
    mode = isp_get_maintenance_status(isp)
    isp_show_maintenance_mode(isp, mode)

    authenticate = True if not args.no_authentication else False

    items = imageList.split(' ')
    for e in range(1, len(items),2):
        addr = items[e]
        address = int(addr,base=16)
        fileName = items[e-1]
        fileName = fileName.replace('..\\','')

        if burn_mram_isp(isp, handler, fileName, address,
                         args.verbose, authenticate) == False:
            break

    # Restore the default Baud rate
    if dynamic_baud_rate_switch:
        isp_set_baud_rate(isp,baud_rate)
        isp.setBaudRate(baud_rate)

    isp_reset(isp)

    isp.closeSerial()

if __name__ == "__main__":
    main()

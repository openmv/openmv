#!/usr/bin/env python3
"""
    Perform Secure Debug unlocking

    __author__ = ""
    __copyright__ = "ALIF Seminconductor"
    __version__ = "0.1.0"
    __status__ = "Dev"
"""
# pylint: disable=unused-argument, invalid-name, bare-except
import os
import sys
import signal
import argparse
sys.path.append("./isp")
import subprocess
from serialport import serialPort
import utils.config
from utils.config import *
from utils.user_validations import validateArgList
from isp_core import *
from isp_util import *
from utils.sign_image_util import sign_image


#import gen_debug_certs

TOOL_VERSION ="0.1.000"        # Define Version constant for each separate tool



def secure_debug_alif(isp, rma):
    """
        secure_debug_alif - Secure Debug unlocking using Alif certificates
    """
    print("[INFO] Alif secure debug")

    message = isp_build_packet(isp, ISP_COMMAND_GET, [ISP_GET_SECURE_DEBUG_TOKENS])

    if rma == False:
       # DCU values - all 1's
       secure_debug_content = [0xFF] * 16
    else:
       # Magic test pattern for RMA
       secure_debug_content = [0xAA] * 16

    # append the challenge bytes received by the device
    secure_debug_content.extend(message[6:22])

    fileName = "build/alif_secure_debug.bin"
    f = open(fileName, 'wb')
    f.write(bytes(secure_debug_content))
    f.close()

    address = int.from_bytes(message[2:6],"little")
    #print("*** address: ", hex(address))
    sign_image(fileName, address, 0xFFFFFFFF, 'OEM')
    token = authenticate_image(isp, fileName)

    isp_secure_debug(isp, token, secure_debug_content)
    print("\r")

def main():
    """
        secure debug
    """
    if sys.version_info.major == 2:
        print("[ERROR] You need Python 3 for this application!")
        sys.exit(1)

    # Deal with Command Line
    parser = argparse.ArgumentParser(description=
                                     'Secure Debug tool')
    parser.add_argument("-d" , "--discover", action='store_true', \
                        default=False, help="(isp) COM port discovery")
    parser.add_argument("-b", "--baudrate", help="serial port baud rate",
                        type=int)
    parser.add_argument("-r", "--rma", default=False, 
                        help="Perform RMA transition (test)", action="store_true")
    parser.add_argument("-V" , "--version",
                        help="Display Version Number", action="store_true")
    parser.add_argument("-v" , "--verbose",
                        help="verbosity mode", action="store_true")
    args = parser.parse_args()
    if args.version:
        print(TOOL_VERSION)
        sys.exit(1)

    load_global_config()
    DEVICE_PART_NUMBER = utils.config.DEVICE_PART_NUMBER
    DEVICE_REVISION = utils.config.DEVICE_REVISION
    DEVICE_REV_BAUD_RATE = utils.config.DEVICE_REV_BAUD_RATE

    print('Secure Debug with parameters:')
    print('Device Part# ' + DEVICE_PART_NUMBER + ' - Rev: ' + DEVICE_REVISION)   

    baud_rate = DEVICE_REV_BAUD_RATE[DEVICE_REVISION]
    if args.baudrate is not None:
        baud_rate = args.baudrate
    isp = serialPort(baud_rate)  # Serial dabbling open up port.

    if args.discover:            # discover the COM ports if requested
        isp.discoverSerialPorts()

    errorCode = isp.openSerial()
    if errorCode is False:
        print("[ERROR] isp openSerial failed for %s" %isp.getPort())
        sys.exit(1)
    print("[INFO] %s open Serial port success" %isp.getPort())

    isp.setBaudRate(baud_rate)
    isp.setVerbose(args.verbose)

    isp_start(isp)    # Begin the ISP sequence

    secure_debug_alif(isp, args.rma)

    isp_stop(isp)
    isp.closeSerial()

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
    Test file for the DMPU function in SERAM

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

TOOL_VERSION ="0.1.000"        # Define Version constant for each separate tool

def run_dmpu(isp, fileName):
    """
        run_dmpu
    """
    if sys.platform == "linux" or sys.platform == "darwin":
        fileName = fileName.replace('\\','/')
    else:
        fileName = fileName.replace('/','\\')
    fileName = fileName.replace('..\\','')

    try:
        f = open(fileName, 'rb')
    except IOError as e:
        print('[ERROR] {0}'.format(e))
        sys.exit(1)

    print("[INFO] Running APP Provisioning code...")
    with f:
        fileSize = file_get_size(f)
        offset = 0
        data_size = DATA_CHUNK_SIZE
        if fileSize < data_size:   # Deal with small ones
            data_size = 16         # CHUNK_SIZE

        isp_build_packet(isp, ISP_COMMAND_DMPU)

        while offset < fileSize:
            f.seek(offset)
            data_line = f.read(data_size)
            if isp_download_data(isp, data_line) == False:
                break
            offset = offset + data_size

        isp_download_done(isp)

    print("[INFO] Done")


def main():
    """
        DMPU
    """
    if sys.version_info.major == 2:
        print("[ERROR] You need Python 3 for this application!")
        sys.exit(1)

    # Deal with Command Line
    parser = argparse.ArgumentParser(description=
                                     'APP Provision tool')
    parser.add_argument("-d" , "--discover", action='store_true', \
                        default=False, help="(isp) COM port discovery")
    parser.add_argument("-b", "--baudrate", help="serial port baud rate",
                        type=int)
    parser.add_argument("-a", "--asset", type=str,
                        help='APP Provision Assets (default assets: build/assets-app-cfg.bin)', default='build/assets-app-cfg.bin')
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

    print('APP Provision with parameters:')
    print('Device Part# ' + DEVICE_PART_NUMBER + ' - Rev: ' + DEVICE_REVISION)   
    print('Assets file: ' + args.asset + '\n')
    if args.asset == None:
        print('[ERROR] No asset file specified')
        sys.exit(1)

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

    asset_file = args.asset
    run_dmpu(isp, asset_file)

    isp_stop(isp)
    isp.closeSerial()

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
    Perform Image signing

    __author__ = ""
    __copyright__ = "ALIF Seminconductor"
    __version__ = "0.1.0"
    __status__ = "Dev"
"""
# pylint: disable=unused-argument, invalid-name, bare-except
import sys
import argparse
import utils.config
from utils.config import *
from utils.sign_image_util import sign_image

# 0.3.000 - added support for Azure Storage (Public Keys and Certs)
# 0.4.000 - split image signature functionality
# 0.5.000 - verbosity removed (it only applies to ISP-related tools)
TOOL_VERSION ="0.5.000"        # Define Version constant for each separate tool



def main():
    """
        sign an image
    """
    if sys.version_info.major == 2:
        print("[ERROR] You need Python 3 for this application!")
        sys.exit(EXIT_WITH_ERROR)

    # Deal with Command Line
    parser = argparse.ArgumentParser(description=
                                     'Image Signature tool')
    parser.add_argument("-i", "--images", type=str,
                        help='Images to sign\
               ("/path/image1.bin 0x30020000")')
    parser.add_argument("-V" , "--version",
                        help="Display Version Number", action="store_true")

    args = parser.parse_args()
    if args.version:
        print(TOOL_VERSION)
        sys.exit()

    if args.images == None:
        print('No image list provided')
        sys.exit(EXIT_WITH_ERROR)

    load_global_config()
    DEVICE_PART_NUMBER = utils.config.DEVICE_PART_NUMBER
    DEVICE_REVISION = utils.config.DEVICE_REVISION

    print('Signing images with parameters:')
    print('Device Part# ' + DEVICE_PART_NUMBER + ' - Rev: ' + DEVICE_REVISION)   

    argList = args.images

    if sys.platform == "linux":
        argList = argList.replace('\\','/')
    else:
        argList = argList.replace('/','\\')

    items = argList.split(' ')

    for e in range(1, len(items),2):
        addr = items[e]
        address = int(addr,base=16)

        fileName = items[e-1]
        fileName = fileName.replace('..\\','')
        
        sign_image(fileName, address, 0xFFFFFFFF, "OEM")

if __name__ == "__main__":
    main()

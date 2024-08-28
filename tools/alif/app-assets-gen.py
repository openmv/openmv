#!/usr/bin/env python3
import sys
import os
import struct
import argparse
from utils.config import *
from utils.toc_common import *
import json
from json.decoder import JSONDecodeError

# Define Version constant for each separate tool 
#0.01.000   Initial version
#0.02.000   added checksum for integrity
#           passed ASSET_TYPE as a flag and used this placeholder for checksum
TOOL_VERSION ="0.02.000"

EXIT_WITH_ERROR = 1

#OUTPUT_FILE = 'build/assets-rev-b0.bin'
ASSET_ID = "APPASSET"
ASSET_VER = 1
CHECK_SUM = 0x00  

HBK0_KEY = 'utils/key/hbk1.bin'
PROV_KEY = 'utils/key/oem_prov_asset.bin'
ENC_KEY = 'utils/key/oem_enc_asset.bin'

# DO NOT Alter the sequence of the following items in the list as it determines 
# the bit position in the final value
OPTIONS = ['ENCRYPTED_ASSETS','TEST_MODE'] 

OPTION_FLAGS = 0x00

def read_asset_config(cfgFile):
    f = open(cfgFile, 'r')
    try:
        cfg = json.load(f)

    except JSONDecodeError as e:
        print("ERROR in JSON file.")
        print(str(e))
        sys.exit(EXIT_WITH_ERROR)

    except ValueError as v:
        print("ERROR in JSON file:")
        print(str(v))
        sys.exit(EXIT_WITH_ERROR)

    except:
        print("ERROR: Unknown error loading JSON file")
        sys.exit(EXIT_WITH_ERROR)

    f.close()
    return cfg

def create_package(outFile):
    print('Creating Assets Package...')
    with open(outFile, 'wb') as f:
        f.write(ASSET_ID.encode('utf8'))
        f.write(struct.pack("H", ASSET_VER))
        f.write(struct.pack("H", CHECK_SUM))
        f.write(struct.pack("I", OPTION_FLAGS))
        with open(HBK0_KEY, 'rb') as i:
            f.write(i.read())
        with open(PROV_KEY, 'rb') as i:
            f.write(i.read())
        f.write(('\0' * 48).encode('utf8'))
        with open(ENC_KEY, 'rb') as i:
            f.write(i.read())
        f.write(('\0' * 48).encode('utf8'))

def decodeFlags(assetFlags):
    print('')
    print('Provisioning Options:')
    for option in OPTIONS:
        fill = 20 - len(option)
        label = option + (fill * ' ')
        if assetFlags & (1<<OPTIONS.index(option)):
            print(label + 'ON')
        else:
            print(label + '\tOFF')    

def check_package(outFile):
    print('Checking Assets Package...')
    # check integrity
    total = getChecksum(outFile)
    if total != 0:
        print('ERROR: Package integrity')
        sys.exit(EXIT_WITH_ERROR)
    else:
        print('Package integrity Ok!')

    with open(outFile, 'rb') as f:
        assetID = f.read(8).decode("utf-8").rstrip('\0')
        assetVer = int.from_bytes(f.read(2),"little", signed=False)
        assetType = int.from_bytes(f.read(2),"little", signed=False)
        assetFlags = int.from_bytes(f.read(4),"little", signed=False)
        print('AssetID: ' + assetID)
        print('Asset Version: ' + str(assetVer))
        decodeFlags(assetFlags)
        print('')

def sum_list(l):
    sum = 0
    for x in l:
        sum += x
    return sum

def getChecksum(outFile):
    with open(outFile, 'rb') as f:
        data = f.read()
        # convert byte array to list of shorts (16-bit)
        bList = [int.from_bytes(data[i:i+2], "little") for i in range(0, len(data), 2)]
        # calculate sum 
        total = sum_list(bList)&0xFFFF
        # calculate complement
        total = 0x10000 - total        
        return total&0xFFFF

def addChecksum(outFile):
    total = getChecksum(outFile)
    with open(outFile, 'r+b') as f:

        f.seek(10)
        f.write(struct.pack("H", total))    


def main():
    global OPTION_FLAGS

    if sys.version_info.major == 2:
        print("You need Python 3 for this application!")
        return 0
 
    parser = argparse.ArgumentParser(description='Generate ICV External Assets',
                        epilog="\N{COPYRIGHT SIGN} ALIF Semiconductor, 2023")
    parser.add_argument("-v" ,"--version", help="Display Version Number", action="store_true")
    parser.add_argument("-f", "--filename",
                        type=str,
                        
                        help="input file  (default: build/config/assets-app-cfg.json")
    parser.add_argument("-c" ,"--check",
                        help="Check asset package (skip generation)",
                        action="store_true")

    args = parser.parse_args()
    if args.version:     
        print(TOOL_VERSION)
        sys.exit()

    # memory defines for Alif/OEM MRAM Addresses and Sizes
    load_global_config()
    DEVICE_PART_NUMBER = utils.config.DEVICE_PART_NUMBER
    DEVICE_REVISION = utils.config.DEVICE_REVISION
    
    print('Generating APP assets with:')
    print('- Device Part# ' + DEVICE_PART_NUMBER + ' - Rev: ' + DEVICE_REVISION)
    configFile = args.filename
    if configFile == None:
        configFile = "build/config/assets-app-cfg.json"
    
    fileToCheck = configFile
    outFile = 'build/' + os.path.basename(configFile)[:-4] + 'bin'
    if args.check:
        # check binary file (check build options - NO Generation)
        if args.filename:
            outFile = args.filename
            fileToCheck = outFile
    else:
        print('- Configuration file: ' + configFile)
    
    print('- Output file: ' + outFile)
    print('')

    # verify the input file (config or binary) exists
    if not os.path.isfile(fileToCheck):
        print('File ' + fileToCheck + ' does not exist!')
        sys.exit()
    
    if args.check == False:
        cfg = read_asset_config(configFile)
        for option in cfg:
            if option not in OPTIONS:
                print('ERROR: option ' + option + ' not supported!')
                sys.exit()
            else:
                if cfg[option].upper() == "ON":
                    OPTION_FLAGS |= (1<<OPTIONS.index(option) )
                    if option == 'ENCRYPTED_ASSETS':
                        print('ENCRYPTED ASSETS Option is not supported yet')
                        sys.exit(EXIT_WITH_ERROR)

        create_package(outFile)
        addChecksum(outFile)

    check_package(outFile)
    print("Done!")
    return 0

if __name__ == "__main__":
    main()
    
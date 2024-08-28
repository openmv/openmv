#!/usr/bin/env python3
import sys
import os
import glob
import argparse
import string
import random
from utils import rsa_keygen
from utils import hbk_gen_util
from utils import cert_key_util

# Define Version constant for each separate tool 
TOOL_VERSION ="0.06.000"
PASS_FILENAME = "utils/key/oem_keys_pass.pwd"

EXIT_WITH_ERROR = 1

def createTextFormat(fileName, data):
    f = open(fileName, 'w')
    str = ''
    for i in data:
        if str != '':
            str += ','
        str += hex(i)    
    f.write(str)
    f.close()

def createBinaryFile(fileName, data):
    f = open(fileName, 'wb')
    f.write(data)
    f.close()

def convertToBinary(inputFile, outputFile):
    f = open(inputFile, 'r')
    array = f.read().split(',')
    f.close()
    print(array)
    out = open(outputFile, 'wb')
    for s in array:
        n = int(s, 16)
        b = n.to_bytes(4, byteorder='little', signed = False)
        out.write(b)
    out.close()    

def gen_AES():
    return os.urandom(16)

def confirmString_generator(size=6, chars=string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for _ in range(size))    

def cleanRoT():
    dirs = ['utils/key/','cert/']    
    for i in range(len(dirs)):
        for filename in os.listdir(dirs[i]):
            f = os.path.join(dirs[i], filename)
            # checking if it is a file
            if os.path.isfile(f):
                print(f)
                try:
                    os.remove(f)    
                except OSError as e:
                    print('Error: ' + str(e))
  
def confirm_clean():
    print("************************************************************************")
    print(" WARNING!!! This action will permanently delete the APP Root of Trust!!!\n","Please enter the following confirmation string followed by Enter to continue,\n or just press any key to abandon this action ")
    print("************************************************************************")
    confirmRequest = confirmString_generator(6)
    print("Confirmation string: ", confirmRequest)
    confirmResponse = input("> ")
    if confirmResponse == confirmRequest:
        print("The APP RoT WILL BE DELETED!!!. Enter Yes to continue (or any key to abort)")
        continueConfirmation = input('> ')
        if continueConfirmation.upper() == 'YES':
            cleanRoT()
            print('\nThe RoT was deleted!')
            return

    print('Operation aborted!')    
    sys.exit(EXIT_WITH_ERROR)       

def main():
    if sys.version_info.major == 2:
        print("You need Python 3 for this application!")
        return 0
 
    parser = argparse.ArgumentParser(description='Generate Application Root of Trust',
                        epilog="\N{COPYRIGHT SIGN} ALIF Semiconductor, 2021")
    parser.add_argument("-v" ,"--version", help="Display Version Number", action="store_true")
    parser.add_argument("-c" ,"--clean", help="Clean (delete) the APP RoT", action="store_true")
    parser.add_argument("-o" ,"--overwrite", help="Overwrite the existing APP RoT", action="store_true")

    args = parser.parse_args()
    if args.version:     
        print(TOOL_VERSION)
        sys.exit()

    if args.clean:  
        confirm_clean()
        sys.exit()

    if os.path.isfile('utils/key/OEMRoT.pem') == True and args.overwrite == False:
        print('APP Root of Trust already exist!')
        print('If you want to overwrite the existing RoT, please use the --overwrite flag ')
        sys.exit()

    # we continue generating a new RoT 
    # (but first, we delete the current one if it exists)
    if os.path.isfile('utils/key/OEMRoT.pem') == True:
        confirm_clean()

    if os.path.isfile(PASS_FILENAME) == False:
        keysPwd = input("Please enter a new password for the keys to generate:")
        f = open(PASS_FILENAME, 'w')
        f.write(keysPwd)
        f.close()
    else:
        print("File " + PASS_FILENAME + " with 'keys password' already exist and it will be used. If new password is required, delete this file and run again!")

    print("Generating APP RoT keys (to be used in Key 1 Certificate)")
    rsa_keygen.main(['utils/key/OEMRoT.pem', "-p", 'utils/key/oem_keys_pass.pwd', "-k", 'utils/key/OEMRoTPublic.pem', "-l", 'build/logs/key_gen_log.log'])

    print("Generating APP SB Key keys (to be used in Key 2 Certificate)")
    rsa_keygen.main(['utils/key/OEMSBKey.pem', "-p", 'utils/key/oem_keys_pass.pwd', "-k", 'utils/key/OEMSBKeyPublic.pem', "-l", 'build/logs/key_gen_log.log'])

    print("Generating APP SB Content keys (to be used in Content Certificates)")
    rsa_keygen.main(['utils/key/OEMSBContent.pem', "-p", 'utils/key/oem_keys_pass.pwd', "-k", 'utils/key/OEMSBContentPublic.pem', "-l", 'build/logs/key_gen_log.log'])

    print("Generating APP Hbk1")
    hbk_gen_util.main(["-f", 'SHA256_TRUNC', "-k", 'utils/key/OEMRoTPublic.pem', "-o", 'utils/key/hbk1_hash.txt', "-l", 'build/logs/gen_hbk_log.log'])

    # create Hbk1 in binary format (required by dmpu_asset_util.py)
    convertToBinary('utils/key/hbk1_hash.txt', 'utils/key/hbk1.bin')
    
    # generate Encryption key (Kce)
    # - text format: required by OEM SB Content Certificate generator to encrypt images
    # - binary format: required by dmpu_asset_util.py to create Asset Package (for key injection)    
    print("Generating OEM kce")
    kce = gen_AES()
    createTextFormat('utils/key/kce.txt', kce)
    createBinaryFile('utils/key/oem_enc_asset.bin', kce)

    # generate Provision key (Kcp)
    # - binary format: required by dmpu_asset_util.py to create Asset Package (for key injection)
    print("Generating OEM kcp")
    kcp = gen_AES()
    createBinaryFile('utils/key/oem_prov_asset.bin', kcp)

    print("Generating APP SB Key 1 Certificate")
    cert_key_util.main(['utils/cfg/OEMSBKey1.cfg', 'build/logs/OEMSBKey1.log'])

    print("Generating APP SB Key 2 Certificate")
    cert_key_util.main(['utils/cfg/OEMSBKey2.cfg', 'build/logs/OEMSBKey2.log'])


    print("\nCheck logs in build/logs/ directory")
    print("Done!")
    return 0

if __name__ == "__main__":
    main()




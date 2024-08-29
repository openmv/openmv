#!/usr/bin/env python3
#pylint: disable=invalid-name,superfluous-parens,anomalous-unicode-escape-in-string

"""
 __author__ = ""
 __copyright__ =Copyright 2021, ALIF Semiconductor"
 __version__ = "0.06.000"



where:
       OBJECT_TYPE: FIREWALL or IMAGE
       CPU_ID: A32_0, A32_1, A32_2, A32_3, M55_HP, M55_HE
       FLAGS: COMPRESS, ENCRYPT, LOAD and BOOT

"""

import os
import sys
import struct
import argparse
from pathlib import Path
import zlib
import json
from utils import toc_common
from utils.config import *
from utils.toc_common import *
from json.decoder import JSONDecodeError
from utils.device_config import gen_device_config
from utils.sign_image_util import sign_image
from printInfo import *
from utils.gen_fw_cfg import *

# Define Version constant for each separate tool
# 0.30.000      includes image signing
# 0.31.000      strip console output to a minimum (includes printInfo)
# 0.32.000      split image signature
TOOL_VERSION = "0.32.000"

EXIT_WITH_ERROR = 1


# memory defines for Alif/OEM MRAM Addresses and Sizes
#ALIF_BASE_ADDRESS = utils.mem_defs.ALIF_BASE_ADDRESS
#OEM_BASE_ADDRESS = utils.mem_defs.OEM_BASE_ADDRESS
#MRAM_BASE_ADDRESS = utils.mem_defs.MRAM_BASE_ADDRESS

# OEM TOC sizes
TOC_ENTRY_SIZE     = 32
TOC_HEADER_SIZE    = 32
TOC_TAIL_SIZE      = 16
TOC_HEADER_VERSION = 1

# Default metadata flags when there is no OEM
METADATA_FLAGS_DEFAULT = 0x3

# Supported CPU_ID for IMAGE object type
SUPPORTED_CPU_ID = ['A32_0',
                    'A32_1',
                    'A32_2',
                    'A32_3',
                    'M55_HP',
                    'M55_HE']

# Supported flags for IMAGE object type
SUPPORTED_FLAGS = ['COMPRESS',
                   'LOAD',
                   'BOOT',
                   'ENCRYPT',
                   'DEFERRED']


# Supported Attributes for JSON configuration file
SUPPORTED_ATTRIBUTES = [
        'disabled',
        'binary',
        'signed',
        'loadAddress',
        'version',
        'mramAddress',
        'cpu_id',
        'flags',
        'extensions'
]

# Swtchers: there are different options for ICV and OEM...
# switcher for getObjectType()
image_switcher = {
        'DEVICE' : 3
    }

# switcher for getCPUID()
cpuid_switcher = {
        'A32_0'  : 0,
        'A32_1'  : 1,
        'M55_HP' : 2,
        'M55_HE' : 3
    }


certPath = Path("cert/")
imagePath = Path("build/images")

# sizes of Certificates
CERT_CHAIN_SIZE = utils.toc_common.CERT_CHAIN_SIZE
CONT_CERT_SIZE  = utils.toc_common.CONT_CERT_SIZE
CERT_CHAIN_SIZE = utils.toc_common.CERT_CHAIN_SIZE

# Global Variables
numImages = 0
oemManagedAreaStartAddress = 0
oemManagedAreaSize = 0
otocStartAddress = 0
unmanaged = []


def createOemTocPackage(fwsections, metadata_flags, outputFile):
    global oemManagedAreaStartAddress
    global oemManagedAreaSize
    global otocStartAddress
    global unmanaged

    print('Creating APP TOC Package...') 
    # Start by loading the Key1/Key2 Certificates for later on
    printInfo("Loading Key1 Cert...")
    key1Cert = getBlob(certPath / 'OEMSBKey1.crt')
    printInfo("Loading Key2 Cert...")
    key2Cert = getBlob(certPath / 'OEMSBKey2.crt')

    # create the output file
    outf = open(outputFile, 'wb')

    # create a memory layout map for debugging
    mapf = open('build/app-package-map.txt', 'w')
    mapf.write('* * * User managed MRAM locations* * * \n')
    
    debugScript = ''
    for img in unmanaged:
        printInfo('Unmanaged Images:')
        imgFile = img[4]
        if img[5] == 'compressed':
            imgFile += '.lzf'

        if img[6] == 'encrypted':
            imgFile = img[4][:-4] + '_enc.bin'

        printInfo(img)
        mapf.write(f'{img[1]}\t{hex(img[2])}\t{img[2]}\t{img[3]}\t{imgFile}\n')
        debugScript += '../build/images/' + imgFile + ' ' + img[1] + ' '
        sign_image('build/images/' + imgFile, int(img[1], 16), 0xFFFFFFFF, "OEM")


    # create a pointer to keep track addresses of components (object_address in OTOC)
    mPointer = oemManagedAreaStartAddress
    #print(f'mPointer start: {mPointer - oemManagedAreaStartAddress}')
    mapf.write('\n* * * APP Package Start Address* * * \n')
    mapf.write('\n  - Certificates for User managed images\n')
    printInfo('Unmanaged images certificates')
    # start adding all Certificate Chains for Unmanaged images
    for sec in fwsections:
        # filter out disabled images
        if sec['disabled'] == True:
            continue

        # filter out Managed components (they will be included after)
        if sec['location'] == 'managed':
            continue

        # update MRAM position for object_address in OTOC
        sec['object_address'] = hex(mPointer)

        if sec['signed'] != False:
            #print('...including Key1/key2 certs')
            outf.write(key1Cert)
            outf.write(key2Cert)
            #mapf.write(f"{hostAddress(mPointer)}\t{hex(CERT_CHAIN_SIZE - CONT_CERT_SIZE)}\t{(CERT_CHAIN_SIZE - CONT_CERT_SIZE)}\tKey1/Key2 Certificates \n")
            mapf.write(f"{hex(mPointer)}\t{hex(CERT_CHAIN_SIZE - CONT_CERT_SIZE)}\t{(CERT_CHAIN_SIZE - CONT_CERT_SIZE)}\tKey1/Key2 Certificates \n")
            mPointer += (CERT_CHAIN_SIZE - CONT_CERT_SIZE)
            #print(f'mPointer KC un: {mPointer - oemManagedAreaStartAddress}')
        # copy the Content Certificate (for signed or unsigned images)
        addContentCertificate(outf, sec['binary'])
        #mapf.write(f"{hostAddress(mPointer)}\t{hex(CONT_CERT_SIZE)}\t{(CONT_CERT_SIZE)}\tSB" + sec['binary'] + ".crt\n")
        mapf.write(f"{hex(mPointer)}\t{hex(CONT_CERT_SIZE)}\t{(CONT_CERT_SIZE)}\tSB" + sec['binary'] + ".crt\n")
        mPointer += CONT_CERT_SIZE
        #print(f'mPointer CC un: {mPointer - oemManagedAreaStartAddress}')

    # add Managed components with their respective Certificates in front of them
    printInfo('Managed components')
    mapf.write('\n  - Certificates & images for Tool managed images\n')
    for sec in fwsections:
        # filter out disabled images
        if sec['disabled'] == True:
            continue

        # filter out Unmanaged components (they were already included above)
        if sec['location'] == 'unmanaged':
            continue

        # update MRAM position for object_address in OTOC
        sec['object_address'] = hex(mPointer)

        # for each managed component, copy certificates and image
        if sec['signed'] != False:
            #print('...including Key1/key2 certs')
            outf.write(key1Cert)
            outf.write(key2Cert)
            #mapf.write(f"{hostAddress(mPointer)}\t{hex(CERT_CHAIN_SIZE - CONT_CERT_SIZE)}\t{(CERT_CHAIN_SIZE - CONT_CERT_SIZE)}\tKey1/Key2 Certificates \n")
            mapf.write(f"{hex(mPointer)}\t{hex(CERT_CHAIN_SIZE - CONT_CERT_SIZE)}\t{(CERT_CHAIN_SIZE - CONT_CERT_SIZE)}\tKey1/Key2 Certificates \n")
            mPointer += (CERT_CHAIN_SIZE - CONT_CERT_SIZE)
            #print(f'mPointer - KCs: {mPointer - oemManagedAreaStartAddress}')
        # copy the Content Certificate (for signed or unsigned images)
        addContentCertificate(outf, sec['binary'])
        #mapf.write(f"{hostAddress(mPointer)}\t{hex(CONT_CERT_SIZE)}\t{(CONT_CERT_SIZE)}\tSB" + sec['binary'] + ".crt\n")
        mapf.write(f"{hex(mPointer)}\t{hex(CONT_CERT_SIZE)}\t{(CONT_CERT_SIZE)}\tSB" + sec['binary'] + ".crt\n")
        mPointer += CONT_CERT_SIZE
        #print(f'mPointer - CC: {mPointer - oemManagedAreaStartAddress}')
        # copy image (check for encryption or compression - both are not supported!)
        binFile = sec['binary']
        if 'ENCRYPT' in sec['flags']:    
            binFile = binFile[:-4] + "_enc.bin"

        if 'COMPRESS' in sec['flags']:
            binFile += '.lzf'

        #print("Copying: " + binFile)
        outf.write(getBlob(imagePath / binFile))
        #mapf.write(f"{hostAddress(mPointer)}\t{hex(sec['size'])}\t{sec['size']}\t" + binFile + "\n")
        mapf.write(f"{hex(mPointer)}\t{hex(sec['size'])}\t{sec['size']}\t" + binFile + "\n")
        mPointer += sec['size']
        #print(f'mPointer - size: {mPointer - oemManagedAreaStartAddress}')
        # padding with 0s
        if sec['padlen'] > 0:
            outf.write(('\0' * sec['padlen']).encode('utf8'))
            #mapf.write(f"{hostAddress(mPointer)}\t{hex(sec['padlen'])}\t{sec['padlen']}\tPadding\n")
            mapf.write(f"{hex(mPointer)}\t{hex(sec['padlen'])}\t{sec['padlen']}\tPadding\n")
            mPointer += sec['padlen']
            #print(f'mPointer - pad: {mPointer - oemManagedAreaStartAddress}')
        

    # finally, create the OEM TOC (verify the current pointer matches the OEM TOC Address we previously calculated)
    if mPointer != otocStartAddress:
        print("[ERROR] ATOC STARTS IS DIFFERENT THAN THE CALCULATED ONE!!!")
        #print(f"ATOC Starts at {hostAddress(mPointer)}")
        print(f"ATOC Starts at {hex(mPointer)}")
        #print(f"ATOC calculated at {hostAddress(otocStartAddress)}")
        print(f"ATOC calculated at {hex(otocStartAddress)}")
        sys.exit(EXIT_WITH_ERROR)

    #print(f"Start of ATOC: {hostAddress(otocStartAddress)}")
    printInfo(f"Start of ATOC: {hex(otocStartAddress)}")
    mapf.write('\n  - APP TOC (Table of Content)\n')
    # create the OEM TOC Header
    print("Adding ATOC...")
    #  TOC token (identifier)
    outf.write("OEMTOC01".encode('utf8'))
    # TOC Header size (in bytes)
    outf.write(struct.pack("H", TOC_HEADER_SIZE))
    # Number of TOC entries and size
    outf.write(struct.pack("H", numImages))
    outf.write(struct.pack("H", TOC_ENTRY_SIZE))
    # TOC version
    outf.write(struct.pack("H", TOC_HEADER_VERSION))
    # HFXO/LFXO presence flags
    outf.write(struct.pack("I", metadata_flags))
    # pad to 16 bytes with 00s
    outf.write(('\0' * 12).encode('utf8'))

    #mapf.write(f"{hostAddress(mPointer)}\t{hex(TOC_HEADER_SIZE)}\t{TOC_HEADER_SIZE}\tAPP TOC Header\n")
    mapf.write(f"{hex(mPointer)}\t{hex(TOC_HEADER_SIZE)}\t{TOC_HEADER_SIZE}\tAPP TOC Header\n")
    mPointer += TOC_HEADER_SIZE
    #print(f'mPointer otoc hdr: {mPointer - oemManagedAreaStartAddress}')
    # create OEM TOC (numImages entries)    
    for sec in fwsections:
        # filter out disabled images
        if sec['disabled'] == True:
            continue

        #print(sec)
        # Object_address
        outf.write(struct.pack("I", int(sec['object_address'], 16)))
        # object_length
        outf.write(struct.pack("I", sec['size']))   # do not need to declare the pad as seram doesn't need it
        # object_type
        outf.write(struct.pack("I", getObjectType(sec['identifier'].rstrip('\0'), image_switcher)))       
        # flags
        outf.write(struct.pack("I", getObjectFlags(sec['flags']) + getCPUID(sec['cpu_id'], cpuid_switcher) ))
        
        # version
        outf.write(struct.pack("I", getObjectVersion(sec['version'])))
        # image_identifier
        outf.write(sec['identifier'][:8].encode('utf8'))
        # extension_header
        outf.write(struct.pack("I", 0))

        #mapf.write(f"{hostAddress(mPointer)}\t{hex(TOC_ENTRY_SIZE)}\t{TOC_ENTRY_SIZE}\tAPP TOC entry for {sec['identifier']} obj_address {hostAddress(int(sec['object_address'],16))}\n")
        mapf.write(f"{hex(mPointer)}\t{hex(TOC_ENTRY_SIZE)}\t{TOC_ENTRY_SIZE}\tAPP TOC entry for {sec['identifier']} obj_address {sec['object_address']}\n")
        mPointer += TOC_ENTRY_SIZE
        #print(f'mPointer otoc entry: {mPointer - oemManagedAreaStartAddress}')

    # write current data to file, so we can read back and calculate CRC32
    outf.flush()

    # create Checksum (crc32)
    f = open(outputFile, "rb")
    # locate start of OTOC
    s = otocStartAddress - oemManagedAreaStartAddress
    f.seek(s)
    #f.seek((TOC_ENTRY_SIZE * numImages) + TOC_HEADER_SIZE, 2)
    otoc = f.read()
    f.close()
    crc32 = zlib.crc32(otoc)
    printInfo('Calculated CRC32: ' + hex(crc32))

    # now, in the OEM, we are adding other fields in the TOC Tail
    # OTOC crc32 + OTOC_startAddress + OEM_packageStart + OEM_packageSize
    outf.write(struct.pack("I", crc32))
    outf.write(struct.pack("I", otocStartAddress))
    outf.write(struct.pack("I", oemManagedAreaStartAddress))
    outf.write(struct.pack("I", oemManagedAreaSize))
    
    #mapf.write(f"{hostAddress(mPointer)}\t{hex(TOC_TAIL_SIZE)}\t{TOC_TAIL_SIZE}\tAPP TOC Tail\n")
    mapf.write(f"{hex(mPointer)}\t{hex(TOC_TAIL_SIZE)}\t{TOC_TAIL_SIZE}\tAPP TOC Tail\n")
    mPointer += TOC_TAIL_SIZE
    printInfo(f'mPointer atoc tail - end: {mPointer - oemManagedAreaStartAddress}')

    # check the OEM Package size is the same we calculated before
    if (mPointer - oemManagedAreaStartAddress) != oemManagedAreaSize:
        print('[ERROR] Something is wrong! Final APP Package size is different than the calculated one...')
        print(f'Calculated: {oemManagedAreaSize} bytes')
        print(f'Real size:  {mPointer - oemManagedAreaStartAddress} bytes')
        sys.exit(EXIT_WITH_ERROR)

    mapf.write("\nAPP Package Summary:\n")
    mapf.write(f" - APP Package total size: {mPointer - oemManagedAreaStartAddress} bytes\n")
    #mapf.write(f" - APP Package Start Address: {hostAddress(oemManagedAreaStartAddress)}\n")
    mapf.write(f" - APP Package Start Address: {hex(oemManagedAreaStartAddress)}\n")
    mapf.write(f" - APP TOC size: {(TOC_ENTRY_SIZE * numImages) + TOC_HEADER_SIZE + TOC_TAIL_SIZE} bytes\n")
    #mapf.write(f" - APP TOC Start Address: {hostAddress(otocStartAddress)}\n")
    mapf.write(f" - APP TOC Start Address: {hex(otocStartAddress)}\n")
    mapf.write(f" - APP CRC32: {hex(crc32)}\n")

    mapf.write('\n* * * END of APP Package * * * \n')

    debugScript += '../' + outputFile + ' ' + hex(oemManagedAreaStartAddress)

    # close the file
    outf.close()

    mapf.close()

    # generate debug script file
    dsFile = 'bin/application_package.ds'    # Was bin/oempackage.ds
    try:
        os.makedirs(os.path.dirname(dsFile), exist_ok=True)
        ds = open(dsFile, 'w')
        #f.write('reset system\n')
        ds.write('set semihosting args ' + debugScript + '\n' )
        ds.write('continue\n')
        ds.write('wait\n')
        ds.write('reset reset.hardware\n')
        ds.close()

    except:
        print('[ERROR] generating debug script file!')
        sys.exit(EXIT_WITH_ERROR)
    
    printInfo('Write the APP TOC package to ' + hex(oemManagedAreaStartAddress))

# take second element for sort
def orderByAddress(elem):
    return elem[1]
    
def calcOemManagedArea(fwsections):
    global numImages
    global oemManagedAreaStartAddress
    global oemManagedAreaSize
    global otocStartAddress
    global unmanaged

    # verify images exist and calculate size and padding
    numSignedImages = 0
    numManagedImages = 0
    sizeManagedImages = 0

    print('Calculating APP area...')    
    for sec in fwsections:
        # check if entry is disabled
        if sec['disabled'] == True:
            printInfo('Entry disabled:' + sec['identifier'])
            continue
        
        numImages += 1
              
        # determine the size of file and if padding is needed
        fsize, padlen = getImageSize('build/images/' + sec['binary'])
        # update the size and padding information
        sec['size'] = fsize
        sec['padlen'] = padlen

        # check if image requires compression
        # if so, create the file using lzf utility
        compressLabel = 'uncompressed'
        if 'COMPRESS' in sec['flags']:
            compressLabel = 'compressed'
            compressImage('build/images/' + sec['binary'])
            # we should have the (new) compressed file, so we will check if this is the case...

            # determine the size of (compressed) file and if padding is needed
            fsize, padlen = getImageSize('build/images/' + sec['binary'] + '.lzf')
            # add compressed file size and update padding information 
            # (the compressed image will be included in the package so pad should be calculated for
            # compressed file size)
            sec['uncompressedSize'] = sec['size']   # uncompressed Size will be used by the certificate creation process
            sec['size'] = fsize
            sec['padlen'] = padlen

        # check encryption
        encryptionLabel = 'unencrypted'
        if 'ENCRYPT' in sec['flags']:
            encryptionLabel = 'encrypted'


        # if image is signed
        if sec['signed'] != False:
            numSignedImages += 1

        # if image is Managed (not Fixed Address => mramAddress different than 0 or none)
        if sec['mramAddress'] == 'none' or sec['mramAddress'] == 0:
            numManagedImages += 1
            sizeManagedImages += ( sec['size'] + sec['padlen'] )
            sec['location'] = 'managed'
        else:
            sec['location'] = 'unmanaged'
            if sec['signed'] == False:
                signature = 'unsigned'
            else:
                signature = 'signed'

            printInfo(f"Unmanaged: {sec['identifier']}")
            tmp = (sec['identifier'], sec['mramAddress'], fsize + padlen, signature, sec['binary'], compressLabel, encryptionLabel)    # we normally don't pad unmanaged images, but just in case this change in the future...
            unmanaged.append(tmp)

    printInfo(f"Signed Images: {numSignedImages}")
    printInfo(f"Managed Images: {numManagedImages}")
    printInfo(f"Managed Images total size: {sizeManagedImages:,} bytes")

    # First Check: Total Managed Images should not be greater than APP MRAM SIZE
    if sizeManagedImages > APP_MRAM_SIZE:
        print('\n[ERROR] Images DO NOT FIT in available space. Please review images sizes \nconsidering this information:')
        print(f'Total APP MRAM size: {APP_MRAM_SIZE:,} bytes')
        sys.exit(EXIT_WITH_ERROR)

    # calculate size of OEM TOC
    otocSize = (TOC_ENTRY_SIZE * numImages) + TOC_HEADER_SIZE + TOC_TAIL_SIZE 
    printInfo(f"APP TOC Size: {hex(otocSize)}")
    # calculate OEM TOC Starting Address in MRAM
    otocStartAddress = ALIF_BASE_ADDRESS - otocSize
    printInfo(f"APP TOC Address: {hex(otocStartAddress)}")

    # calculate size of OEM Managed Area (total of OEM Package - don't include images with fixed addresses)
    # OEM Managed Area size = size(OTOC) + size(Managed Images with Padding) + (numSignedImages * CERT_CHAIN_SIZE) + (numUnsignedImages * CONT_CERT_SIZE) 
    oemManagedAreaSize = otocSize + sizeManagedImages + ( numSignedImages * CERT_CHAIN_SIZE) + ( (numImages - numSignedImages) * CONT_CERT_SIZE)
    printInfo(f"APP Managed Area size: {oemManagedAreaSize}")

    # calculate OEM Managed Area Starting Addresses
    oemManagedAreaStartAddress = ALIF_BASE_ADDRESS - oemManagedAreaSize
    printInfo(f"APP Package Start Address: {hex(oemManagedAreaStartAddress)}")

    # now that we know where OEM Managed Area will start (oemManagedAreaStartAddress),
    # we should verify that OEM Unmanaged Area is below above address
    boundary = 0
    # sort list with key
    unmanaged.sort(key=orderByAddress)
    printInfo("Unmanaged Images - Ordered by MRAM Address")
    for img in unmanaged:
        printInfo(img)

        startAddress = int(img[1], 16)
        size = img[2]
        # check overlap with previous image
        if startAddress < boundary:
            print('[ERROR] There is an overlap between image ' + img[0] + ' and previous image!')
            print('Please correct this to continue.')
            sys.exit(EXIT_WITH_ERROR)

        boundary = startAddress + size
        #print(hex(boundary))
   
    

    # check boundary is below OEM Managed Area Starting Address
    if boundary > oemManagedAreaStartAddress:    
        print('\n[ERROR] Images DO NOT FIT in available space. Please review layout \nconsidering this information:')
        print(f'Available space is {otocStartAddress - OEM_BASE_ADDRESS} bytes starting at MRAM address {hex(OEM_BASE_ADDRESS)}')
        sys.exit(EXIT_WITH_ERROR)
    else:
        if boundary == 0:
            boundary += MRAM_BASE_ADDRESS
        printInfo(f"Final boundary: {hex(boundary)}")
        printInfo(f"Available MRAM: {oemManagedAreaStartAddress - boundary} bytes")


    # now that we know everything fits well, we can proceed to calculate the
    # the MRAM addresses for the managed images, to update the 'mramAddress' field
    # (we need the MRAM address to generate the Content Certificates before start
    # creating the OEM Package binary)
    
    # we keep a pointer starting at OEM Managed Area address
    mPointer = oemManagedAreaStartAddress
    # we will start the OEM Managed Area layout with all certificates for Unmanaged Images
    # so we need to know How many Unmanaged images are and from those, how many are Signed 
    # and how many are Unsigned (because we need more space for signed images than for unsigned ones)

    for img in unmanaged:
        if img[3] == 'signed':
            mPointer += CERT_CHAIN_SIZE
        else:
            mPointer += CONT_CERT_SIZE
        
    for sec in fwsections:
        # check if entry is disabled
        if sec['disabled'] == True:
            continue    

        # we won't create Content Certificate for Device COnfiguration (at the moment...)
        #if sec['identifier'] == 'FIREWALL':
        #    continue  

        # if image is Managed (not Fixed Address => mramAddress different than 0 or none)
        if sec['mramAddress'] == 'none' or sec['mramAddress'] == 0:
            # for each managed image, we need to calculate the image storage address in MRAM,
            # needed to create the Content Certificate for each image
            if sec['signed'] == False:
                mPointer += CONT_CERT_SIZE
            else:
                mPointer += CERT_CHAIN_SIZE

            # save the MRAM store address for Managed images
            sec['mramAddress'] = hex(mPointer)
            #print(sec['identifier'], sec['mramAddress'])

            # move the pointer to the end of the image    
            mPointer += ( sec['size'] + sec['padlen'])


def validateVersAttr(version):
    items = version.split('.')
    if len(items)>3:
        print("[ERROR] Invalid version string. Please use 'xxx.yyy.zzz' format")
        sys.exit(EXIT_WITH_ERROR)

    for item in items:
        try:
            i = int(item)
            if i>=0 and i<=255:
                continue       
            print("[ERROR] Invalid value '" + item + "' in version string: " + version)                  
        except:
           print("[ERROR] Invalid item '" + item + "' in version string: " + version)

        sys.exit(EXIT_WITH_ERROR)

def updateDeviceConfig(file):
    #print('*** updateDeviceConfig: ', file)
    # Update the firewall configuration in the OEM DEVICE config file
    update_fw_cfg_oem()

    cfg = read_global_config('build/config/' + file)
    if 'miscellaneous' in cfg:
        for item in cfg['miscellaneous']:
            item.pop('sdesc', None)
            item.pop('ldesc', None)
            item.pop('options', None)
    with open('build/config/' + file, "w") as json_file:
        json.dump(cfg, json_file, indent=2)


def main():
    cwd_path = os.getcwd()
    os.chdir(cwd_path)
    global MRAM_BASE_ADDRESS
    global ALIF_BASE_ADDRESS
    global OEM_BASE_ADDRESS
    global APP_MRAM_SIZE

    if sys.version_info.major == 2:
        print("[ERROR] You need Python 3 for this application!")
        sys.exit(EXIT_WITH_ERROR)

    """
       Parse user arguments
    """
    parser = argparse.ArgumentParser(description='Generate APP TOC Package',
                        epilog="\N{COPYRIGHT SIGN} ALIF Semiconductor, 2023")
    parser.add_argument("-f", "--filename", type=str,
                        default="build/config/app-cfg.json",
                        help="input file  (check build/config/app-cfg.json as an example")
    parser.add_argument("-o", "--output", type=str,
                        default="build/AppTocPackage.bin",
                        help="output file (default is build/AppTocPackage.bin")
    parser.add_argument("-V", "--version", help="Display Version Number", action="store_true")
    parser.add_argument("-c" ,"--clean", help="Clean build workspace (restore initial state)", action="store_true")
    parser.add_argument("-v" , "--verbose", help="verbosity mode", action="store_true")

    args = parser.parse_args()

    if args.version:
        print(TOOL_VERSION)
        sys.exit()

    if args.clean:
        cleanBuild(['build/AppTocPackage.bin', 'build/App-package-map.txt', 'build/images/*_enc.bin', 'build/images/*.lzf', 'cert/SB*.crt'])
        sys.exit()

    # set verbose option
    verboseModeSet(args.verbose)

    # memory defines for Alif/OEM MRAM Addresses and Sizes
    load_global_config()
    DEVICE_PART_NUMBER = utils.config.DEVICE_PART_NUMBER
    DEVICE_REVISION = utils.config.DEVICE_REVISION
    ALIF_BASE_ADDRESS = utils.config.ALIF_BASE_ADDRESS
    MRAM_BASE_ADDRESS = utils.config.MRAM_BASE_ADDRESS
    OEM_BASE_ADDRESS = utils.config.APP_BASE_ADDRESS 
    APP_MRAM_SIZE = utils.config.APP_MRAM_SIZE   

    print('Generating APP Package with:')
    print('Device Part# ' + DEVICE_PART_NUMBER + ' - Rev: ' + DEVICE_REVISION)

    print('- System MRAM Base Address: ' + hex(ALIF_BASE_ADDRESS))
    print('- APP MRAM Base Address: ' + hex(OEM_BASE_ADDRESS))
    print('- APP MRAM Size: ' + str(APP_MRAM_SIZE))
    print('- Configuration file: ' + args.filename)
    print('- Output file: ' + args.output)
    print('')
    fwsections = read_json_file(args.filename, SUPPORTED_ATTRIBUTES, SUPPORTED_FLAGS, SUPPORTED_CPU_ID)
  
    validateOptions(fwsections)

    metadata_flags = METADATA_FLAGS_DEFAULT
    for sec in fwsections:
        # check if entry is disabled
        if sec['disabled'] is True:
            print('Entry disabled:' + sec['identifier'])
            continue

        # check version string (format 'xx.yy.zz' where each item should be between 0 and 255)
        validateVersAttr(sec['version'])
        
        # update the size and padding information
        if sec['identifier'].strip('\0') != 'DEVICE':
            continue

        print('Generating Device Configuration for: ' + sec['binary'])
        updateDeviceConfig(sec['binary'])
        metadata_flags = gen_device_config(sec['binary'], False)
        sec['binary'] = sec['binary'][:-5] + '.bin'

    if metadata_flags == None:
        metadata_flags = METADATA_FLAGS_DEFAULT

    #     also check unmanaged images (mramAddress != 0) are between boundaries (OEM_BASE_ADDRESS - only in Rev_A as in Rev_B will be 0)
    #     and images don't overlap... Also, advice is GAPs (big ones) exist - especially if tool can't create the layout...
    calcOemManagedArea(fwsections)
 
    printInfo(f"Images: {numImages}")
    
    #for sec in fwsections:
    #    print(sec)

    createContentCerts(fwsections, 'OEM')

    createOemTocPackage(fwsections, metadata_flags, args.output)

    # checking the OEM TOC Package size
    try:
        fsize = os.path.getsize(args.output)
        print("APP TOC Package size: " + str(fsize) + " bytes")
    except:
        print(sys.exc_info()[0])
        print("ERROR: veryfing APP TOC Package size")
        sys.exit(EXIT_WITH_ERROR)

    sign_image(args.output, oemManagedAreaStartAddress, 0xFFFFFFFF, "OEM")

    print("Done!")
    return 0

if __name__ == "__main__":
    main()

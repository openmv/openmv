#!/usr/bin/env python3
"""
    Perform Image signing

    __author__ = ""
    __copyright__ = "ALIF Seminconductor"
    __version__ = "0.1.0"
    __status__ = "Dev"
"""
# pylint: disable=unused-argument, invalid-name, bare-except
from pathlib import Path
from utils.toc_common import *
from utils.common.certificates import getKeyCertificatePath

# 0.3.000 - added support for Azure Storage (Public Keys and Certs)
# 0.4.000 - split image signature functionality 
# 0.5.000 - remove folders restriction
TOOL_VERSION ="0.5.000"        # Define Version constant for each separate tool

#EXIT_WITH_ERROR = 1

certPath = Path("cert/")
imagePath = Path("build/images")

# A simplified copy of createContentCerts (in toc_common.py) with some modified paths
def create_content_certs(sec, prefix, config_file):
    """
        create a content certificate for an image
    """
    print('Creating Signature...')
    try:
        file = open(imagePath / "images.txt", "w")
    except:
        print(sys.exc_info()[0])
        print("ERROR creating template file for Content Certificates")
        sys.exit(EXIT_WITH_ERROR)

    configFile = prefix + config_file

    fsize = sec['size']
    encrypt = '0'

    binfile = sec['binary']
    binfile = "../" + binfile
    print("Binary File: ", binfile)
    file.write(binfile                       + \
               " " + str(sec['loadAddress']) + \
               " " + str(sec['mramAddress']) + \
               " " + str(fsize)              + \
               " " + encrypt)
    file.close()

    os.chdir(os.getcwd() + '/utils/')
    cert_sb_content_util.main(["-c", 'cfg/'+configFile, "-l", '../build/logs/SBContent.log'])
    os.chdir(os.getcwd() + '/../')

    # check if file exists (and remove it) before renaming it
    #certFile = "SB" + sec['binary'] + ".crt"
    certFile = sec['binary'] + ".crt"
    if os.path.exists(certFile):
        os.remove(certFile)

    fileName = prefix + "SBContent.crt"
    os.rename(certPath / fileName, certFile)
    # save cert. name in images list
    sec['cert'] = certFile

    # remove ICVSBContent_Cert.txt file (not used)
    textFile = prefix + "SBContent_Cert.txt"
    if os.path.exists(certPath / textFile):
        os.remove(certPath / textFile)
    # remove images.txt (not used)
    textFile = "images.txt"
    if os.path.exists(imagePath / textFile):
        os.remove(imagePath / textFile)

def is_seram_address(address):
    # TODO: the end address should be configurable (for SPARK it is 0x30020000)
    if address >= 0x30000000 and address < 0x30040000: 
        return True
    else:
        return False

def sign_image(fileName, destAddress, loadAddress, signature_type):
    key1_cert = ""
    key2_cert = ""

    if signature_type == "ICV":
        #key1_cert = "cert/ICVSBKey1.crt"
        key1_cert = getKeyCertificatePath() + '/ICVSBKey1.crt'
        #key2_cert = "cert/ICVSBKey2.crt"
        key2_cert = getKeyCertificatePath() + '/ICVSBKey2.crt'
    elif signature_type == "OEM":
        key1_cert = "cert/OEMSBKey1.crt"
        key2_cert = "cert/OEMSBKey2.crt"
    else:
        print("Auth Host can only be 'ICV' or 'OEM'")
        sys.exit(EXIT_WITH_ERROR)

    sec = {}
    sec['disabled'] = False
    sec['binary'] = fileName
    fsize, padlen = getImageSize(sec['binary'])
    sec['size'] = fsize
    sec['padlen'] = padlen
    if is_seram_address(destAddress):
        sec['mramAddress'] = hex(destAddress)
    else:
        sec['mramAddress'] = globalToLocalAddress(hex(destAddress))
    sec['identifier'] = 'image'
    sec['flags'] = {}

    if loadAddress == 0xFFFFFFFF:
        # Signing content that is downloaded to 'destAddress' before verification.
        # The address is specified in the sec['mramAddress'] field.
        sec['loadAddress'] = hex(loadAddress)
        #load-verify-scheme = 1
        config_file = 'SBContent_lvs_1.cfg'
    elif is_seram_address(loadAddress):
        # signing SERAM images for packaging them as single-image STOC updates
        # SERAM images require 'load-verify-scheme = 0'
        sec['loadAddress'] = hex(loadAddress)
        #load-verify-scheme = 0
        config_file = 'SBContent_lvs_0.cfg'
    else:
        # this section is not used currently
        sec['loadAddress'] = globalToLocalAddress(hex(loadAddress))
        #load-verify-scheme = 0
        config_file = 'SBContent_lvs_0.cfg'
    create_content_certs(sec, signature_type.upper(), config_file)

    content_cert = sec["cert"]
    print("Content Certificate File: ", content_cert)
    
    sigfile = fileName + ".sign"
    print("Signature File: ", sigfile)
    with open(sigfile, "wb") as outf:
        with open(key1_cert, 'rb') as inf:
            outf.write(inf.read())
        with open(key2_cert, 'rb') as inf:
            outf.write(inf.read())
        with open(content_cert, 'rb') as inf:
            outf.write(inf.read())
        # add the 12-byte padding required for content certificate chains
        outf.write(('\0' * 12).encode('utf8'))

    if os.path.exists(content_cert):
        os.remove(content_cert)

    return sigfile
